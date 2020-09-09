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
#include "../../net_data_defs.h"

void *network_dist()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */
  const char *section = "NET DIST";

  int count;

  /* Массив сетевых данных, передаваемый между клиентом и потоком */
  int net_data[NET_DATA_SIZE];
  /* Указатели на различные данные в массиве. (для удобства обращения) */
  /*int *type = &net_data[0];*/
  /*int *data1 = &net_data[1];*/
  /*int *data2 = &net_data[2];*/

  for (count = 0; count < NET_DATA_SIZE; count++)
  {
    net_data[count] = -1;
  }

  /*printf("[%s] - Started\n", section);*/

/*##############################################################################
 * Получение и пересылка данных клиентов
 *##############################################################################
 */

  while (1)
  {
    if(mq_receive(net_mq_desc, (char *)&net_data, mq_msg_size, NULL) > 0)
    {
      for(count = 0; count <= client_max_id; count++)
      {
        /*
         * Отправка в личный UDP сокет клиента - уточнение адреса не требуется.
         */
        if(send(udp_cl_sock_desc[count], net_data, sizeof(net_data), 0) < 0)
        {
          perror("DIST SEND");
        }
      }
      if(verbose_flag > 1)
      {
        printf("[%s] - Resended data to clients (%d/%d/%d)\n", section,
                net_data[0], net_data[1], net_data[2]);
      }
      for (count = 0; count < NET_DATA_SIZE; count++)
      {
        net_data[count] = -1;
      }
    }
  }
}
