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
  /* Массив сетевых данных, передаваемый между клиентом и потоком */
  int net_data_int[NET_DATA_SIZE];
  /* Указатели на различные данные в массиве. (для удобства обращения) */
  int *type = &net_data_int[0];
  int *data1 = &net_data_int[1];
  /*int *data2 = &net_data_int[2];*/

  char net_data[50];
  char name[9];

  memset(net_data, 0, sizeof(net_data));
  memset(name, 0, sizeof(name));
  for (count = 0; count < NET_DATA_SIZE; count++)
  {
    net_data_int[count] = -1;
  }
  count = 0;

  printf("[%s] - Started\n", section);

/*##############################################################################
 * Запуск потока подключения клиентов (network_accept)
 *##############################################################################
 */

  if (launch_thread(&network_accept_tid, network_accept, "NET ACCEPT") != 0)
  {
    printf("[%s] - Might just end here...\n", section);
  }

/*##############################################################################
 * Ожидание подтверждения старта от всех клиентов
 *##############################################################################
 */

  while(1)
  {
    pthread_mutex_lock(&ready_count_lock);

    /*
     * Число готовых игроков должно равняться их количеству (макс.ID + 1), при
     * том хоть один игрок должен быть присоединён (макс.ID неотрицательный)
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

  printf("[%s] - All players set and ready\n", section);

  /*
   * Стоит как можно раньше закрыть поток принятия подключений. Поэтому, поток
   * закрывается прямо здесь, без вызова "close_thread"
   */
  pthread_cancel(network_accept_tid);
  pthread_join(network_accept_tid, NULL);
  network_accept_tid = 0;
  printf("[%s] - (NET ACCEPT) thread canceled and joined\n", section);

  /*
   * Небольшая пауза даёт возможность потокам клиентов отослать своим подопечным
   * порты для UDP соединения
   */
  sleep(SLEEP_TIME);

  /* Оповещение клиентов о готовности начать и количестве игроков. */
  *type = START;
  *data1 = client_max_id + 1;
  for (count = 0; count <= client_max_id; count++)
  {
    send(net_client_desc[count], net_data_int, sizeof(net_data_int),
          TCP_NODELAY);
  }
  memset(net_data, 0, sizeof(net_data));


  if (launch_thread(&network_dist_tid, network_dist, "NET DIST") != 0)
  {
    /*
     * Здесь нужно будет решить, что будет делать сервер в случае неуспешного
     * запуска потока рассылки. Вероятно, он перезапустит сессию, или закроется
     * вовсе, оставив решения пользователю.
     */
  }

/*##############################################################################
 * Конец игры
 *##############################################################################
 */

  /*
   * Поток блоируется в ожидании разблокировки семафора. Его разблокирует один
   * из клиентских потоков, когда установит конец игры.
   */
  sem_wait((sem_t *)&endgame_lock);
  printf("[%s] - Endgame reached\n", section);

  /* Закрываются ненужные теперь потоки и сокеты. */
  /* Клиентские потоки */
  for (count = 0; count <= client_max_id; count++)
  {
    sprintf(name, "NET CL#%d", count);
    close_thread(network_cl_handling_tid[count], name);
    network_cl_handling_tid[count] = 0;
  }

  /* Личные сокеты клиентов */
  for (count = 0; count <= client_max_id; count++)
  {
    sprintf(name, "UDP CL#%d", count);
    close_sock(udp_cl_sock_desc[count], name);
    memset(name, 0, sizeof(name));
    udp_cl_sock_desc[count] = 0;
  }

  /* Поток сетевой рассылки */
  close_thread(network_dist_tid, "NET DIST");
  network_dist_tid = 0;


  /* ЗАГЛУШКА */
  while (1) {sleep(10);}
}
