#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../net_data_defs.h"
#include "globals.h"
#include "network.h"
#include "player.h"

/*##############################################################################
 * Поток, обрабатывающий сообщения для старта игры
 *##############################################################################
 */
void* client_check(void* args)
{
  int net_data[3];
  int connected_players = 0;
  int ready_players = 0;
  printf("[network] - START\n");
  while(1)
  {
    if((recv(tcp_sockfd, net_data, sizeof(net_data), 0)) == -1) 
    {
      perror("[network] Recv init data");
      exit(1);
    }
    printf("data[0]: %d,   data[1]: %d \n", net_data[0], net_data[1]);
    switch(net_data[0])
    {
      case START:  
        printf("[network] - Got START from server\n");
        sem_post(&sem);
        break;
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
        win_id = net_data[1];
        udp_server_port = net_data[2];
        break;
      case ENDGAME:
        win_id = net_data[1];
        return (void*)0;
        break;
    }
  }
  printf("[network] - End...\n");
}
/*##############################################################################
 * Поток определяет направление движения игрока
 *##############################################################################
 */
void* net_check(void* args)
{
  int net_data[3];
  int data_size = sizeof(net_data);

  struct player* players = (struct player*)args;
  while(1)
  {
    if(recv(udp_sockfd, net_data, data_size, 0) == -1)
    {
      perror("[network] - Recv players dir");
      exit(-1);
    }
    switch(net_data[0])
    {
      case CL_DIR:
        pthread_mutex_lock(&mutex);
        players[net_data[1]].dir = net_data[2];
        pthread_mutex_unlock(&mutex);
        break;
    }
  }
}
