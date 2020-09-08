#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../net_data_defs.h"
#include "globals.h"
#include "network.h"
#include "player.h"

/*##############################################################################
 * Поток, обрабатывающий сообщения для старта игры
 *##############################################################################
 */
void* client_check(void* param)
{
  int net_data[3];
  int connected_players = 0;
  int ready_players = 0;
  int tcp_sockfd = *(int*)param;
  printf("[network] - START\n");
  while(1)
  {
    if((recv(tcp_sockfd, net_data, sizeof(net_data), 0)) == -1) 
    {
      perror("[network] Recv init data");
      exit(1);
    }
    switch(net_data[0])
    {
      case START:  
        printf("[network] - Got START from server\n");
        return (void*)0;
      case CL_READY:
        ready_players = net_data[2];
        max_players = ready_players;
        printf("[network] - PLAYERS IN LOBBY : %d\n", connected_players);
        printf("          - PLAYERS READY    : %d\n", ready_players);
        break;
      case CL_CONNECT:
        connected_players = net_data[2];
        printf("[network] - PLAYERS IN LOBBY : %d\n", connected_players);
        printf("          - PLAYERS READY    : %d\n", ready_players);
        break;
      case ID_PORT:
        printf("[network] - Got Id_PORT [%d:%d]\n", net_data[1], net_data[2]);
        my_id = net_data[1];
        udp_server_port = net_data[2];
        break;
    }
  }
  printf("[network] - End...\n");
  return (void*)0;
}
/*##############################################################################
 * Поток определяет направление движения игрока
 *##############################################################################
 */
void* net_check(void* args)
{
  int net_data[3];
  int data_size = sizeof(net_data);
  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET; 
  servaddr.sin_port = htons(udp_server_port); 
  servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
 
  socklen_t len = sizeof(struct sockaddr_in);

  struct player* players = (struct player*)args;
  while(1)
  {
    if(recvfrom(udp_sockfd, net_data, data_size, 0,
          (struct sockaddr*)&servaddr, &len) == -1)
    {
      perror("[network] - Recv players dir");
      exit(-1);
    }
    players[net_data[1]].dir = net_data[2];
  }
}
