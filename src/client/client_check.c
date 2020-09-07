/* Данный поток служит для удобства клинтов. Он отображает количество 
 * подключенных и готовых клиентов. При готовности всех клиентов сервер должен
 * отправить сообщение START, служащий для данного потока концом его работы. 
 */
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>

#include "../net_data_defs.h"

void *client_check(void *param)
{
  int net_data[3];
  int connected_players = 0;
  int ready_players = 0;
  int tcp_sockfd = *(int  *)param;
  printf("[client_check] - START\n");
  while(1)
  {
    if((recv(tcp_sockfd, net_data, sizeof(net_data), 0)) == -1) 
    {
      perror("[main] Recv");
      exit(1);
    }
    if(net_data[0] == START)
    {
      printf("[client_check] - Got START from server\n");
      break;
    }
    if(net_data[0] == CL_READY)
    {
      ready_players = net_data[1];
      printf("[client_check] - PLAYERS IN LOBBY : %d\n", connected_players);
      printf("               - PLAYERS READY    : %d\n", ready_players);
    }
    if(net_data[0] == CL_CONNECT)
    {
      connected_players = net_data[1];
      printf("[client_check] - PLAYERS IN LOBBY : %d\n", connected_players);
      printf("               - PLAYERS READY    : %d\n", ready_players);
    }
  }
  printf("[client_check] - End...\n");
  return 0;
}