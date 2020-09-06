/*
 * Поток сетевой рассылки
 *
 * Используется непосредственно во время игры, для рассылки данных от одного
 * клиента, всем им (включая отправителя).
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
#include <arpa/inet.h>

#include "../server_defs.h"

void *network_dist()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */
  const char *section = "NET DIST";

  int count;
  char formatted_data[100];

  memset(formatted_data, 0, sizeof(formatted_data));

  printf("[%s] - Started\n", section);

/*##############################################################################
 * Получение и пересылка данных клиентов
 *##############################################################################
 */

  while (1)
  {
    if(mq_receive(net_mq_desc, formatted_data, 100, NULL) > 0)
    {
      for(count = 0; count <= client_max_id; count++)
      {
        /* Заберите нужный блок. Другой закоментите. */
        /* Отправка по UDP */
        /* Отправка в личный сокет клиента - уточнение адреса не требуется. */
        send(udp_cl_sock_desc[count], formatted_data, 100, 0);
        /*sendto(udp_cl_sock_desc[count], formatted_data, 100, 0,
                    (struct sockaddr *)&net_client_addr[count],
                    net_client_addr_size[count]);*/
        /*perror("SEND");*/
        /* Отправка по TCP */
        /*
        send(net_client_addr[count], formatted_data, 100, 0);
        */
      }
      printf("[%s] - REsended some data to clients\n", section);
      memset(formatted_data, 0, sizeof(formatted_data));
    }
  }
}
