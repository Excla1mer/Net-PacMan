/*
 * Основной поток контроля сетевого взаимодействия.
 *
 * Отвечает за переключение сервера между режимами принятия подключений и
 * пересылки сообщений во время игры.
 * Следит за условиями начала и конца игры.
 *
 * Созданно: 02.09.20.
 * Автор: Денис Пащенко.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <mqueue.h>
#include <semaphore.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "../server_protos.h"
#include "../server_defs.h"
#include "../../net_data_defs.h"

void *network_control()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */
  const char *section = "NET CONTROL";

  int count;
  int ret;
  int player_id;
  /* Массив сетевых данных, передаваемый между клиентом и потоком */
  int net_data[NET_DATA_SIZE];
  /* Указатели на различные данные в массиве. (для удобства обращения) */
  int *type = &net_data[0];
  int *data1 = &net_data[1];
  /*int *data2 = &net_data[2];*/

  /* Структура для прослушки дескрипторов клиентов */
  struct pollfd poll_descs[MAX_PLAYERS];

  char name[9];

  memset(name, 0, sizeof(name));
  memset(poll_descs, 0, sizeof(poll_descs));
  for (count = 0; count < NET_DATA_SIZE; count++)
  {
    net_data[count] = -1;
  }
  count = 0;
  ret = 0;
  player_id = -1;

  printf("[%s] - Started\n", section);

/*##############################################################################
 * Бесконечный цикл игры.
 *##############################################################################
 */

  /*
   * Вся последующая секция потока - это бесконечный цикл.
   * Его начало совпадает с началом новой игры, а конец - с окончанием текущей.
   * Цикл перезапускается, после окончания игры, давая, тем самым, старт новой.
   */

  while(1) /* Общий бесконечный цикл игры */
  {
    printf("\n-------------------------------------------------------\n"\
            "[%s] - New game session started\n", section);
    /*
     * Цикл, в котором игра начинается и идёт. Вне него - она завершается. Если
     * оборвать этот цикл, текущая игровая сессия сначала завершится, а потом
     * начнётся новая, с этого места.
     */
    while(1 && restart_flag == 0) /* Цикл текущей игровой сессии */
    {
/*##############################################################################
* Запуск потока подключения клиентов (network_accept)
*##############################################################################
*/
      if(network_accept_tid == 0)
      {
        if (launch_thread(&network_accept_tid, network_accept, "NET ACCEPT")
                          != 0)
        {
          printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"\
          "[%s] - Failed on crucial part here.\n"\
          "        Server will now close current game sesssion "\
          "and start new one.\n", section);
          /* Провал здесь приводит к перезапуску цикла. */
          break;
        }
      }

/*##############################################################################
* Ожидание подтверждения старта от всех клиентов
*##############################################################################
*/

      while(1  && restart_flag == 0)
      {
        pthread_mutex_lock(&ready_count_lock);

        /*
        * Число готовых игроков должно равняться их количеству (макс.ID + 1),
        * при том хоть один игрок должен быть присоединён
        * (макс.ID неотрицательный)
        */
        if((ready_count == client_max_id + 1) && (client_max_id != -1))
        {
          pthread_mutex_unlock(&ready_count_lock);
          break;
        }
        pthread_mutex_unlock(&ready_count_lock);
        /*
        * Если не повесить на поток хоть какую-то задачу в этом цикле, то при
        * попытке присоединить его в этот момент pthread_join зависает. По всей
        * видимости, это вызвано тем, что поток никогда и не доходит до
        * возможности завершиться. Даже самая незначительная пауза решает эту
        * проблему.
        *
        * Можно попробовать переписать этот момент с использованием семафоров,
        * чтобы поток пробуждался только при изменениях в "ready_count".
        */
        sleep(0.01); /* 100 миллисекунд */
      }

/*##############################################################################
 * Переключение с принятия подключений на перессылку данных клиентов
 *##############################################################################
 */

      if(restart_flag == 0)
      {
        printf("[%s] - All players set and ready\n", section);

        /*
        * Стоит как можно раньше закрыть поток принятия подключений. Поэтому,
        * поток закрывается прямо здесь, без вызова "close_thread"
        */
        pthread_cancel(network_accept_tid);
        pthread_join(network_accept_tid, NULL);
        network_accept_tid = 0;
        printf("[%s] - (NET ACCEPT) thread canceled and joined\n", section);

        /*
        * Небольшая пауза даёт возможность потокам клиентов отослать своим
        * подопечным порты для UDP соединения
        */
        sleep(SLEEP_TIME);

        /* Оповещение клиентов о готовности начать и количестве игроков. */
        *type = START;
        *data1 = client_max_id + 1;
        for (count = 0; count <= client_max_id; count++)
        {
          send(net_client_desc[count], net_data, sizeof(net_data),
                TCP_NODELAY);
        }

        /*
         * Запуск потока сетевой рассылки.
         *
         * Провал здесь приводит к перезапуску цикла игры.
         */
        if (launch_thread(&network_dist_tid, network_dist, "NET DIST") != 0)
        {
          printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"\
                "[%s] - Failed on crucial part here.\n"\
                "        Server will now close current game sesssion "\
                "and start new one.\n", section);
          break;
        }

        /*
         * Запуск потока сетевой синхронизации.
         *
         * Провал здесь приводит к перезапуску цикла игры.
         */
        if (launch_thread(&network_sync_tid, network_sync, "NET SYNC") != 0)
        {
          printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n"\
                  "[%s] - Failed on crucial part here.\n"\
                  "        Server will now close current game sesssion "\
                  "and start new one.\n", section);
          break;
        }
      }

/*##############################################################################
 * Ожидание конца игры
 *##############################################################################
 */

      if(restart_flag == 0)
      {
        /* Заполнение poll структуры дескрипторами клиентов */
        for(count = 0; count <= client_max_id; count++)
        {
          poll_descs[count].fd = net_client_desc[count];
          /* Событие для проверки - есть данные для чтения */
          poll_descs[count].events = POLLIN;
        }
      }

      /*
      * Бесконечная прослушка дескрипторов. Обрывается только, если из одного из
      * них, от клиента, было полученно сообщение о конце игры.
      */
      while(ret == 0  && restart_flag == 0)
      {
        /* Блокирующееся ожидание событий в дескрипторах */
        poll(poll_descs, client_max_id + 1, -1);

        /* Поиск сработавшего дескиптора */
        for(count = 0; count <= client_max_id; count++)
        {
          /*Сравнение события в дескрипторе с событием, которое окончило poll*/
          if(poll_descs[count].revents & POLLIN)
          {
            if(recv(net_client_desc[count], net_data, sizeof(net_data), 0) > 0)
            {

              /*
              * Проверка, было ли это сообщение, о конце игры.
              * Если да - сообщить об этом клиентам и оборвать цикл.
              */
              if (*type == ENDGAME)
              {
                /* Отправка остальным клиентам сообщения о конце игры. */
                *type = ENDGAME; /* Конец игры*/
                *data1 = count; /* Номер клиента */
                player_id = count;
                for(count = 0; count <= client_max_id; count++)
                {
                  if ((send(net_client_desc[count], net_data,
                            sizeof(net_data), TCP_NODELAY)) == -1)
                  {
                    perror("TCP SEND ENDGAME");
                  }
                }
                /* За ret следит цикл while. */
                ret = 1;
                /* break оборвёт лишь for */
                break;
              }
                /*
                 * Если это не конец игры, вероятно, это ответ от клиента на
                 * запрос о синхронизации от клиента.
                 */
                else if(*type == SYN_REP)
                {
                  *data1 = count; /* Номер клиента */
                  if (mq_send(local_mq_desc, (char *)&net_data,
                              sizeof(net_data), 0) != 0)
                  {
                    printf("[%s] - MQ unavailable. Player data dropped.\n",
                            section);
                  }
                }
                for (count = 0; count < NET_DATA_SIZE; count++)
                {
                  net_data[count] = -1;
                }
              }
            }
          }
        }
        /* Разорвать цикл текущей игровой сессии. */
        break;
    } /* Конец цикла текущей игровой сессии */

/*##############################################################################
* Конец игры (очистка данных и запуск новой игровой сессии)
*##############################################################################
*/

    /*
     * После шага с прослушкой сетевых дескрипторов, поток знает, кто именно
     * окончил игру. Сообщение об этом отправляется в вывод.
     */
    if(restart_flag == 0 && player_id >= 0)
    {
      printf("-------------------------------------------------------\n"\
              "[%s] - Endgame reached\n", section);
      printf("[%s] - Player#%d [%s:%d] ends the game\n", section,
              player_id,
              inet_ntoa(net_client_addr[player_id].sin_addr),
              ntohs(net_client_addr[player_id].sin_port));
    }
    else
    {
      printf("-------------------------------------------------------\n"\
              "[%s] - Forced restart\n", section);
    }

    /* Закрываются ненужные теперь потоки и сокеты. */
    /* Поток сетевой синхронизации */
    if(network_sync_tid != 0)
    {
      close_thread(network_sync_tid, "NET SYNC");
      network_sync_tid = 0;
    }

    /* Поток сетевой рассылки */
    if(network_dist_tid != 0)
    {
      close_thread(network_dist_tid, "NET DIST");
      network_dist_tid = 0;
    }

    /* Различные клиентские данные */
    for (count = 0; count <= client_max_id; count++)
    {
      /* Клиентские потоки */
      if(network_cl_handling_tid[count] != 0)
      {
        sprintf(name, "NET CL#%d", count);
        close_thread(network_cl_handling_tid[count], name);
        memset(name, 0, sizeof(name));
        network_cl_handling_tid[count] = 0;
      }

      /* Личные сокеты клиентов */
      if(udp_cl_sock_desc[count] != 0)
      {
        sprintf(name, "UDP CL#%d", count);
        close_sock(udp_cl_sock_desc[count], name);
        memset(name, 0, sizeof(name));
        udp_cl_sock_desc[count] = 0;
      }

      /* Прочие данные клиентов или о клиентах */
      memset(&net_client_addr[count], 0, sizeof(net_client_addr[count]));
    }

    /* Счётчики */
    ready_count = 0;
    client_max_id = -1;
    ret = 0;
    restart_flag = 0;
    player_id = -1;

    /* Сетевые poll-дескрипторы клиентов */
    memset(poll_descs, 0, sizeof(poll_descs));

    /* Сброс порта. Теперь перебор порта новых сокетов вновь начнётся сначала */
    server_addr_struct.sin_port = htons(SERVER_PORT);

    /*
    * Очистка очередей сообщений.
    * Какой-либо из потоков мог успеть записать сообщение в очередь, но другие
    * потоки могли не успеть изъять его. Чтобы старые данные не участвовали в
    * новой сессии игры, сообщения из очередей изымаются, если они там есть.
    */
    clear_mq(local_mq_desc, "LOCAL");
    clear_mq(net_mq_desc, "NET");

    printf("[%s] - Restarting...\n"\
          "-------------------------------------------------------\n",
          section);
  } /* Конец общего бесконечного цикла игры */
}
