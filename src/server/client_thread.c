/*
 * Поток работы с клиентом.
 *
 * Извещает клиента о назначенном ему ID. В теории, занят получением и обработкой
 * данных о передвижении от клиента.
 * На текущий момент, просто получает сообщения от клиента.
 *
 * Созданно: 01.09.20.
 * Автор: Денис Пащенко.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server_defs.h"

void *client_thread()
{

/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */
  const char *section = "CLI THREAD";

  int client_id;
  char net_data[50];


/*##############################################################################
 * Работа с клиентом
 *##############################################################################
 */
  while(1)
  {
    if(mq_receive(mq_desc, net_data, 50, NULL) > 0)
    {
      client_id = atoi(net_data);
      memset(net_data, 0, sizeof(net_data));
      printf("[%s#%d] - Started\n", section, client_id);
      printf("[%s#%d] - Hadling Player#%d (%s)\n", section, client_id,
              client_id+1, inet_ntoa(net_client_addr[client_id].sin_addr));
      break;
    }
  }

  /*
   * Отсылка данных в виде строки. Таким образом можно описать вначале тип
   * данных, а затем указать сами данные.
   */
  sprintf(net_data, "ID:%d", client_id);
  while ((send(net_client_desc[client_id], net_data,
              sizeof(net_data), 0)) == -1) {}
  printf("[%s#%d] - Notified client about it's ID\n", section, client_id);

  /*
   * Функциональная заглушка. Поток навечно зависает в ожидании сообщений от
   * клиента
   */
  while(1)
  {
    /* Если потребуется, можно получать по TCP */
    /*if(recv(net_client_desc[client_id], net_data, 50, 0) > 0)
    {
      printf("[%s#%d] - (TCP) Message from (%s): %s", section, client_id,
              inet_ntoa(net_client_addr[client_id].sin_addr), net_data);
      memset(net_data, 0, sizeof(net_data));
    }*/
    if(recvfrom(udp_sock_desc, net_data, 50, 0,
                (struct sockaddr *)&net_client_addr[client_id],
                &net_client_addr_size[client_id]) > 0)
    {
      printf("[%s#%d] - (UDP) Message from (%s): %s", section, client_id,
              inet_ntoa(net_client_addr[client_id].sin_addr), net_data);
      memset(net_data, 0, sizeof(net_data));
    }
  }
}
