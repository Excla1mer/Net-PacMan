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
#include "help_sets.h"

#define DIV 1000.0

/*##############################################################################
 * Поток, обрабатывающий сообщения для старта игры
 *##############################################################################
 */
void* client_check(void* args)
{
  struct player* my_player = (struct player*)args;
  int net_data[7];
  float fractx;
  float fracty;
  int connected_players = 0;
  int ready_players = 0;

  printf("[NETWORK] - Starting\n");
  while(1)
  {
    if((recv(tcp_sockfd, net_data, sizeof(net_data), 0)) == -1)
    {
      perror("[NETWORK ERROR] Recv init data");
      exit(1);
    }
    switch(net_data[0])
    {
      case START:
        printf("[NETWORK INFO] - Got START\n");
        max_players = ready_players;
        sem_post(&sem);
        break;
      case CL_READY:
        ready_players = net_data[2];
        printf("[NETWORK INFO] - Players in lobby : %d\n", connected_players);
        printf("               - Players ready    : %d\n", ready_players);
        break;
      case CL_CONNECT:
        connected_players = net_data[2];
        printf("[NETWORK INFO] - Players in lobby : %d\n", connected_players);
        printf("               - Players ready    : %d\n", ready_players);
        break;
      case ID_PORT:
        printf("[NETWORK INFO] - Got Id_PORT [%d:%d]\n", net_data[1], net_data[2]);
        my_id = net_data[1];
        udp_server_port = net_data[2];
        break;
      case ENDGAME:
        printf("[NETWORK INFO] - Recv ENDGAME message\n");
        end_game = 1;
        printf("ENDGAME FROM THREAD: %d\n", end_game);
        exit(-1);
        pthread_cancel(listen_thread_tid);
        return (void*)0;
      case SYN_REQ:
        printf("[NETWORK INFO] - Got SYN_REQ\n"); 
        pthread_mutex_lock(&mutex);
        
        /* Выделение дробной части числа */
        fractx = (my_player[my_id].x - (int)my_player[my_id].x) * DIV;
        fracty = (my_player[my_id].y - (int)my_player[my_id].y) * DIV;

        /* Заполнение сетевых данных */
        set_netdata(net_data, SYN_REP, my_id, (int)my_player[my_id].x, fractx,
            my_player[my_id].y, fracty, my_player[my_id].score);

        if(send(tcp_sockfd, net_data, sizeof(net_data), 0) == -1)
        {
          perror("[NETWORK ERROR] Send SYN_REP");
          exit(1);
        }
        pthread_mutex_unlock(&mutex);
        break;
      default: break;
    }
  }
  printf("[NETWORK] - Close\n");
}
/*##############################################################################
 * Поток определяет направление движения игрока
 *##############################################################################
 */
void* net_check(void* args)
{
  int net_data[7];
  int id;
  int data_size = sizeof(net_data);
  struct player* players = (struct player*)args;

  while(1)
  {
    if(recv(udp_sockfd, net_data, data_size, 0) == -1)
    {
      perror("[NETWORK ERROR] - Recv direction");
    }
    switch(net_data[0])
    {
      case CL_DIR:
        pthread_mutex_lock(&mutex);
        players[net_data[1]].dir = net_data[2];
        pthread_mutex_unlock(&mutex);
        break;
      case SYN_REP:
        id = net_data[1];
        pthread_mutex_lock(&mutex);
        players[id].x = net_data[2];
        players[id].x += net_data[3] / DIV;
        players[id].y = net_data[4];
        players[id].y += net_data[5] / DIV;
        players[id].score = net_data[6];
        printf("[NETWORK SYNC] - Id:%d X:%.2f Y:%.2f Score:%d\n", id,
              players[id].x, players[id]. y, players[id].score);
        pthread_mutex_unlock(&mutex);
        break;
      default:
        break;
    }
  }
}
