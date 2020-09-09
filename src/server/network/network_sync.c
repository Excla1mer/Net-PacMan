/*
 * Поток сетевой синхронизации (ОТЛОЖЕН)
 *
 * Используется непосредственно во время игры, для запроса данных от клиентов и
 * рассылки этих же данных им всем.
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

#include "../server_defs.h"
#include "../../net_data_defs.h"

void *network_sync()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */
  const char *section = "NET SYNC";

  int count, count2;
  short ret;

  /* Массив сетевых данных, передаваемый между клиентом и потоком */
  int net_data[NET_DATA_SIZE];
  /* Указатели на различные данные в массиве. (для удобства обращения) */
  int *type = &net_data[0];
  /*int *data1 = &net_data[1];*/
  /*int *data2 = &net_data[2];*/

  ret = -1;
  for (count = 0; count < NET_DATA_SIZE; count++)
  {
    net_data[count] = -1;
  }

  /*printf("[%s] - Started\n", section);*/

  /*
   * Прямо на старте запускать синхронизацию необходимости нет, поэтому поток
   * спит какое-то время.
   */
  sleep(SYNC_DELAY);

/*##############################################################################
 * Получение и пересылка данных клиентов
 *##############################################################################
 */

  while (1)
  {
    if(verbose_flag != 0)
    {
      printf("[%s] - Attempting to sync clients\n", section);
    }
    /* Синхронизация каждого клиента по отдельности. */
    for (count = 0; count <= client_max_id; count++)
    {
      /* Отсылка запроса */
      *type = SYN_REQ;
      ret = send(net_client_desc[count], net_data, sizeof(net_data),
                  TCP_NODELAY);
      for (count2 = 0; count2 < NET_DATA_SIZE; count2++)
      {
        net_data[count2] = -1;
      }

      /*
       * Получение ответа - запрошенных данных. Их получил поток сетевого
       * контроля (network_control).
       */
      if(ret > 0)
      {
        if (mq_receive(local_mq_desc, (char *)&net_data, mq_msg_size,
                        NULL) > 0)
        {
          if (*type == SYN_REP)
          {
            /* Отправка данных текущего клиента всем клиентам вообще*/
            for (count2 = 0; count2 <= client_max_id; count2++)
            {
              /* Отправка по UDP*/
              if (send(udp_cl_sock_desc[count2], net_data, sizeof(net_data),
                        0) <= 0)
              {
                printf("[%s] - Sync data for client#%d was not send! "\
                        "Proceeding anyway...\n", section, count2);
              }
              else if(verbose_flag > 0)
              {
                printf("[%s] - Sync-ed Player#%d data with other players "\
                        "(%d/%d/%d/%d/%d/%d/%d)\n", section, count,
                        net_data[0],
                        net_data[1],
                        net_data[2],
                        net_data[3],
                        net_data[4],
                        net_data[5],
                        net_data[6]);
              }
            }
          }
          for (count2 = 0; count2 < NET_DATA_SIZE; count2++)
          {
            net_data[count2] = -1;
          }
        }
      }
      else
      {
        printf("[%s] - Unable to send sync request to client#%d! "\
                "Proceeding anyway...\n", section, count);
      }
      ret = -1;
    }
    if(verbose_flag != 0)
    {
      printf("[%s] - Sync finished\n", section);
    }
    sleep(SYNC_FREQ);
  }
}
