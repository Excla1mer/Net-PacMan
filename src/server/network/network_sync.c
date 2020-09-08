/*
 * Поток сетевой синхронизации (ОТЛОЖЕН)
 *
 * Используется непосредственно во время игры, для запроса данных от клиентов и
 * рассылки этих же им всем.
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

/* Частота, в секундах, повтора синхронизации */
#define SYNC_FREQ 3
/* Пауза, в секундах, перед стартом синхронизации, после запуска потока */
#define INIT_SLEEP 5

/* Сетевые константы. Будут перенесенны в основной файл после согласования */
#define SYN_REQ 6
#define SYN_REP 7

void *network_sync()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */
  const char *section = "NET SYNC";

  int count, count2;

  /* Массив сетевых данных, передаваемый между клиентом и потоком */
  int net_data[NET_DATA_SIZE];
  /* Указатели на различные данные в массиве. (для удобства обращения) */
  int *type = &net_data[0];
  int *data1 = &net_data[1];
  /*int *data2 = &net_data[2];*/

  for (count = 0; count < NET_DATA_SIZE; count++)
  {
    net_data[count] = -1;
  }

  printf("[%s] - Started\n", section);

  /*
   * Прямо на старте запускать синхронизацию необходимости нет, поэтому поток
   * спит какое-то время.
   */
  sleep(INIT_SLEEP);

/*##############################################################################
 * Получение и пересылка данных клиентов
 *##############################################################################
 */

  while (1)
  {
    printf("[%s] - Sync-ing clients\n", section);
    /* Синхронизация каждого клиента по отдельности. */
    for (count = 0; count <= client_max_id; count++)
    {
      /* Отсылка запроса */
      *type = SYN_REQ;
      send(net_client_desc[count], net_data, sizeof(net_data), TCP_NODELAY);

      /* Получение ответа - запрошенных данных */
      if (recv(net_client_desc[count], net_data, sizeof(net_data), 0) > 0)
      {
        if (*type == SYN_REP)
        {
          *data1 = count;
          /* Отправка данных текущего клиента всем клиентам вообще*/
          for (count2 = 0; count2 <= client_max_id; count2++)
          {
            /* Отправка по TCP (по TCP ли? возможно, клиенту будет накладно)*/
            if (send(net_client_desc[count2], net_data, sizeof(net_data),
                    TCP_NODELAY) <= 0)
            {
              printf("[%s] - Sync data for client#%d was not send! "\
              "Proceeding anyway...\n", section, count);
            }
          }
        }
      }
    }
    printf("[%s] - Sync finished\n", section);
    sleep(SYNC_FREQ);
  }
}
