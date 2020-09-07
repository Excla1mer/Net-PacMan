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
#include <arpa/inet.h>

#include "../server_defs.h"

/* Частота, в секундах, повтора синхронизации */
#define SYNC_FREQ 3

void *network_sync()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */
  const char *section = "NET SYNC";

  int count, count2;
  char net_data[50];
  char formatted_data[100];

  memset(net_data, 0, sizeof(net_data));
  memset(formatted_data, 0, sizeof(formatted_data));

  printf("[%s] - Started\n", section);

  /*
   * Прямо на старте запускать синхронизацию необходимости нет, поэтому поток
   * спит какое-то время.
   */
  sleep(10);

/*##############################################################################
 * Получение и пересылка данных клиентов
 *##############################################################################
 */

  printf("[%s] - Sync-ing now\n", section);
  while (1)
  {
    printf("[%s] - Sync-ing clients\n", section);
    /* Синхронизация каждого клиента по отдельности. */
    for (count = 0; count <= client_max_id; count++)
    {
      /* Отсылка запроса */
      send(net_client_desc[count], "SYN_REQ", 7, 0);
      /* Получение ответа - запрошенных данных */
      recv(net_client_desc[count], net_data, 50, 0);

      /* Отправка данных текущего клиента всем клиентам вообще*/
      sprintf(formatted_data, "ID:%d|SYN_REP:%s", count, net_data);
      for (count2 = 0; count2 <= client_max_id; count2++)
      {
        /* Отправка по UDP */
        if (send(udp_cl_sock_desc[count2], formatted_data, 100, 0) > 0)
        {
          printf("[%s] - Sync-ed client#%d\n", section, count);
        }
        else
        {
          printf("[%s] - Sync data for client#%d was not send! "\
          "Proceeding anyway...\n", section, count);
        }
      }
      memset(net_data, 0, sizeof(net_data));
      memset(formatted_data, 0, sizeof(formatted_data));
    }
    sleep(SYNC_FREQ);
  }
}
