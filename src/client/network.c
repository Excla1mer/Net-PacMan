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
        return (void*)0;
      case SYN_REQ:
        /*
         * Описание того как происходит разбиение дробного числа на два целых:
         * Допустим координата x = 111,222. Записываю в net_data[2] целую часть
         * числа: net_data[2] = x; net_data[2] == 111;
         * Для отделения дробной части числа использую переменную float tmp.
         * Записываю в tmp дробную часть сисла: tmp = x - net_data[2];
         * tmp = 0.222;
         * Перевожу дробную часть числа в целую путем домножения ее на DIV
         * (DIV = 10000) и записываю в net_data[3]: net_data[3] = tmp * DIV;
         * net_data[3] = 2220;
         */
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

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
  server.sin_port = htons(udp_server_port);
  socklen_t len = sizeof(server);

  while(1)
  {
    if(recvfrom(udp_sockfd, net_data, data_size, 0,
          (struct sockaddr*)&server, &len) == -1)
    {
      perror("[NETWORK ERROR] - Recv direction");
      exit(-1);
    }
    switch(net_data[0])
    {
      case CL_DIR:
        pthread_mutex_lock(&mutex);
        players[net_data[1]].dir = net_data[2];
        pthread_mutex_unlock(&mutex);
        break;
      case SYN_REP:
        /* Описание того как два целых числа собираются в одно дробное.
         * Получаем данные net_data[2] = 111; net_data[3] = 2220;
         * Записываем целую часть дроби в х: x = net_data[2]; x = 111;
         * Переводим net_data[3] в тип float путем топорного умножения на 1,0
         * Затем делим на тот-же DIV, что и использовался при первеводе в целые
         * числа, и прибавляем к х: x += (2220 * 1.0) / DIV; x += 111,2220
         */
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
