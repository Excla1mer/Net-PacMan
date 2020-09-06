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
#include <arpa/inet.h>

#include "../server_defs.h"
#include "../server_protos.h"

void *network_accept()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */
  const char *section = "NET ACCEPT";

  char net_data[50];
  char thread_name[9];

  memset(net_data, 0, sizeof(net_data));
  memset(thread_name, 0, sizeof(thread_name));

  printf("[%s] - Started\n", section);

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
        printf("[%s] - (TCP) Client [%s:%d] connected\n", section,
                inet_ntoa(net_client_addr[client_max_id].sin_addr),
                ntohs(net_client_addr[client_max_id].sin_port));

        /*
         * Присвоенный здесь же ID отправляется в очередь сообщений.
         * Там его подберёт созданный далее поток клиента.
         */
        sprintf(net_data, "%d", client_max_id);
        while (mq_send(local_mq_desc, net_data, strlen(net_data),
                      0) != 0) {}
        memset(net_data, 0, sizeof(net_data));

        /* Создание потока для клиента */
        sprintf(thread_name, "NET CL#%d", client_max_id);
        launch_thread(&network_cl_handling_tid[client_max_id],
                      network_cl_handling, thread_name);
        memset(thread_name, 0, sizeof(thread_name));
      }
  }

  /* ЗАГЛУШКА */
  while (1) {sleep(10);}
}
