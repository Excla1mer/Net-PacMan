/*
 * Поток ожидания и подключения клиентов.
 * По подключению, заполняет данные клиентов.
 *
 * Созданно: 03.09.20.
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

void *network_accept()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */
  const char *section = "NET ACPT";


  int count;
  /* Массив сетевых данных, передаваемый между клиентом и потоком */
  int net_data[NET_DATA_SIZE];
  /* Указатели на различные данные в массиве. (для удобства обращения) */
  int *type = &net_data[0];
  int *data1 = &net_data[1];
  int *data2 = &net_data[2];

  char thread_name[9];

  memset(thread_name, 0, sizeof(thread_name));
  for (count = 0; count < NET_DATA_SIZE; count++)
  {
    net_data[count] = -1;
  }
  count = 0;

  /*printf("[%s] - Started\n", section);*/

/*##############################################################################
 * Подключение и подготовка игроков
 *##############################################################################
 */

  /*
   * Ожидание до MAX_PLAYERS подключений.
   */
  while (client_max_id < MAX_PLAYERS)
  {
    /* Ожидание подключения */
    net_client_addr_size[client_max_id + 1] =
                  sizeof(net_client_addr[client_max_id + 1]);
    net_client_desc[client_max_id + 1] =
                  accept(tcp_sock_desc,
                  (struct sockaddr *)&net_client_addr[client_max_id + 1],
                  &net_client_addr_size[client_max_id + 1]);

      /* Полученное подключение трактуется как новый клиент */
      if (net_client_desc[client_max_id + 1] > 0)
      {
        /*
         * Теперь максимальный ID можно повысить на единицу.
         */
        client_max_id++;
        printf("[%s] - (TCP) Player#%d [%s:%d] connected\n", section,
                client_max_id,
                inet_ntoa(net_client_addr[client_max_id].sin_addr),
                ntohs(net_client_addr[client_max_id].sin_port));

        /*
         * Присвоенный здесь же ID отправляется в очередь сообщений.
         * Там его подберёт созданный далее поток клиента.
         */
        while (mq_send(local_mq_desc, (char *)&client_max_id,
                      sizeof(client_max_id), 0) != 0) {}

        /* Создание потока для клиента */
        sprintf(thread_name, "NET CL#%d", client_max_id);
        launch_thread(&network_cl_handling_tid[client_max_id],
                      network_cl_handling, thread_name);
        memset(thread_name, 0, sizeof(thread_name));

        /* Отправка остальным клиентам информации о новом подключении. */
        *type = CL_CONNECT;
        *data1 = client_max_id;
        *data2 = client_max_id + 1;
        for(count = 0; count <= client_max_id; count++)
        {
          if ((send(net_client_desc[count], net_data,
                      sizeof(net_data), TCP_NODELAY)) == -1)
          {
            perror("TCP SEND NEW CONNECT");
          }
        }
        if(verbose_flag != 0)
        {
          printf("[%s] - Notified clients about newly connected client\n",
                  section);
        }
        for (count = 0; count < NET_DATA_SIZE; count++)
        {
          net_data[count] = -1;
        }
      }
  }

  /* Если достигнут предел числинности игроков, поток выйдет сюда */
  printf("[%s] - Reached maximum player capacity of (%d)\n", section,
          MAX_PLAYERS);

  /* Пока что, поток уйдёт спать. Стоит придумать что-то получше. */
  while (1) {sleep(10);}
}
