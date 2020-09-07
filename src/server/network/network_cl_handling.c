/*
 * Поток работы с клиентом.
 *
 * Извещает клиента о назначенном ему ID и ждёт сообщения о готовности.
 * По готовности, переключается на отправку сообщений от клиента в очередь
 * сообщений, где их ждёт поток пересылки (network_dist)
 *
 * Созданно: 01.09.20.
 * Автор: Денис Пащенко.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <mqueue.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "../server_defs.h"

void *network_cl_handling()
{

/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */

  /* Префикс имени секции */
  const char *section_prefix = "NET CL";
  /* Реальное имя секции. Будет собрано, после получения ID клиента. */
  char section[9];

  int client_id;
  int count;
  int net_data_int;
  unsigned int new_port;
  char net_data[50];
  char formatted_data[100];

  count = 0;

  memset(net_data, 0, sizeof(net_data));
  memset(formatted_data, 0, sizeof(formatted_data));
  memset(section, 0, sizeof(section));
  net_data_int = -1;

/*##############################################################################
 * Первичное общение (отсылка ID)
 *##############################################################################
 */
  while(1)
  {
    if(mq_receive(local_mq_desc, net_data, 50, NULL) > 0)
    {
      client_id = atoi(net_data);
      memset(net_data, 0, sizeof(net_data));

      /* Из префикса и ID собирается имя секции */
      sprintf(section, "%s#%d", section_prefix, client_id);

      printf("[%s] - Started\n", section);
      printf("[%s] - Hadling Player#%d [%s:%d]\n", section, client_id,
              inet_ntoa(net_client_addr[client_id].sin_addr),
              ntohs(net_client_addr[client_id].sin_port));
      break;
    }
  }

  /*
   * Отсылка данных в виде строки. Таким образом можно описать вначале тип
   * данных, а затем указать сами данные.
   */
  sprintf(net_data, "%d", client_id); /* "ID:%d" */
  while ((send(net_client_desc[client_id], net_data,
              sizeof(net_data), TCP_NODELAY)) == -1) {}
  memset(net_data, 0, sizeof(net_data));
  printf("[%s] - Notified client about it's ID\n", section);

/*##############################################################################
 * Ожидание начала игры
 *##############################################################################
 */

  /*
   * TCP - ожидание от клиента сообщения о готовности начать (START)
   */
  printf("[%s] - Waiting for client to get ready\n", section);
  while(1)
  {
    if(recv(net_client_desc[client_id], net_data, 50, 0) > 0)
    {
      /*printf("[%s] - (TCP) Message from (%s): %s", section,
              inet_ntoa(net_client_addr[client_id].sin_addr), net_data);*/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * для проверки с netcat используйте комбинацию Ctrl+D вместо Enter, чтобы не
 * отсылать \n
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
      if(strcmp(net_data, "1") == 0)
      {
        printf("[%s] - Client ready to start\n", section);
        /*
         * Аккуратно, через мьютекс, повышается число готовых начать клиентов
         * на единицу. За этим числом следит поток сетевого контроля
         * (network_control)
         */
        pthread_mutex_lock(&ready_count_lock);
        ready_count++;
        pthread_mutex_unlock(&ready_count_lock);

        memset(net_data, 0, sizeof(net_data));
        /* Цикл вечного ожидания обрывается */
        break;
      }
      memset(net_data, 0, sizeof(net_data));
    }
  }

/*##############################################################################
* Создание личного сокета клиента
*##############################################################################
*/

  /*
   * Потребуется создать собственный сокет, идентичный глобальному, но завязаный
   * на текущего клиента
   * ПРИМЕЧАНИЕ: Эта секция сейчас очень кривая и нестабильная! Также, созданные
   * здесь сокеты не зачищаются в конце программы.
   */


  /* Создание сокета */
  if ((udp_cl_sock_desc[client_id] = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("UDP SOCKET");
    /* Будет лучше добавить какую-то обработку здесь и подобных местах далее */
  }
  else
  {
    printf("[%s] - (UDP) Socket created\n", section);
  }

  /*
   * Процесс привязки сокета:
   * Сервер начинает перебирать порт, пытаясь найти свободный
   */
  for(count = 1; count <= sizeof(int); count++)
  {
    /*
     * Новый порт опирается на тот, что уже записан в глобальной структуре.
     * Это потребует работы с мьютексом, но зато каждый следующий поток начнёт
     * перебор минуя уже опробованые порты.
     */
    pthread_mutex_lock(&new_port_lock);
    new_port = ntohs(server_addr_struct.sin_port) + count;
    server_addr_struct.sin_port = htons(new_port);

    /* Попытка привязки сокета */
    if(bind(udp_cl_sock_desc[client_id], (struct sockaddr *)&server_addr_struct,
        sizeof(server_addr_struct)) == 0)
    {
      pthread_mutex_unlock(&new_port_lock);
      printf("[%s] - (UDP) Socket binded on port [%d]\n", section, new_port);
      break;
    }
    /*else
    {
      perror("UDP BIND");
    }*/
    pthread_mutex_unlock(&new_port_lock);
    if (count == sizeof(int))
    {
      printf("[%s] - (UDP) Exeeded (int) range while serching for port! "\
              "Retrying from beginig...\n",
              section);
      count = 1;
    }
  }
  /*
   * Уведомление клиента о выбраном порте. Используется тот же TCP - сокет, что
   * и для уведомления об ID.
   */
  sprintf(net_data, "%d", new_port); /* "PORT:%d" */
  if ((send(net_client_desc[client_id], net_data,
              sizeof(net_data), TCP_NODELAY)) == -1)
  {
    perror("UDP SEND PORT");
  }
  else
  {
    printf("[%s] - Notified client about UDP connection port [%d]\n",
            section, new_port);
  }

  memset(net_data, 0, sizeof(net_data));

  /* Привязка сокета */
  if (connect(udp_cl_sock_desc[client_id],
          (struct sockaddr *)&net_client_addr[client_id],
          net_client_addr_size[client_id]) < 0)
  {
    perror("UDP CONNECT");
  }
  else
  {
    printf("[%s] - (UDP) Socket connected\n", section);
  }

/*##############################################################################
* Работа с игровыми данными
*##############################################################################
*/

/*
 * Поток переключается на ожидание данных клиента по UDP. Все эти данные он
 * будет форматировать и отправлять в очередь сообщений, где эти данные ждёт
 * поток сетевой рассылки (network_dist)
 */

  /* Можно начать слушать собственный сокет. */
  printf("[%s] - Listening client via UDP\n", section);
  while(1)
  {
    /*
     * Личный сокет клиента теперь привязан к нему, так что можно не указывать
     * адрес, а просто отсылать в сокет.
     */
    /*if(recvfrom(udp_cl_sock_desc[client_id], net_data, 50, 0,
                (struct sockaddr *)&net_client_addr[client_id],
                &net_client_addr_size[client_id]) > 0)*/
    if(recv(udp_cl_sock_desc[client_id], net_data, 50, 0) > 0)
    {
      net_data_int = atoi(net_data);
      printf("[%s] - (UDP) Message from [%s:%d]: %d\n", section,
              inet_ntoa(net_client_addr[client_id].sin_addr),
              ntohs(net_client_addr[client_id].sin_port),
              net_data_int);

      /* Проверка, было ли это сообщение, о конце игры. */
      /*if (strcmp(net_data, "9") == 0) */ /* "ENDGAME" */
      if (net_data_int == 9)
      {
        printf("[%s] - Player ends the game\n", section);

        /*
         * Нужно отправить остальным клиентам сообщение о конце игры.
         */
        sprintf(formatted_data, "%d%d", client_id, net_data_int); /*"ID:%d|%s"*/
        while (mq_send(net_mq_desc, formatted_data, strlen(formatted_data),
                      0) != 0) {}
        memset(formatted_data, 0, sizeof(formatted_data));
        memset(net_data, 0, sizeof(net_data));
        net_data_int = -1;

        /*
         * Инкрементировать(разблокировать) семафор под самый конец, чтобы не
         * закрыться из "network_control" раньше времени
         */
        sem_post(&endgame_lock);
        /*
         * Не слишком красиво, но поток будет просто спать. Его окончат в
         * "network_control" */
        while(1)
        {
          sleep(10);
        }
      }

      /*
       * Отправка данных в очередь сообщений. Считается, что из сети, от
       * клиента, было получено его направление движения. Оно отправляется в
       * очередь, дополненное ID клиента и указанием на тип данных
       */
      sprintf(formatted_data, "%d%d", client_id, net_data_int); /*"ID:%d|DIR:%s"*/
      if (mq_send(net_mq_desc, formatted_data, strlen(formatted_data), 0) != 0)
      {
        printf("[%s] - MQ unavailable. Player data dropped.\n", section);
      }
      memset(formatted_data, 0, sizeof(formatted_data));
      memset(net_data, 0, sizeof(net_data));
      net_data_int = -1;
    }
  }
}
