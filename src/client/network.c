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

#define DIV 100000;

/*##############################################################################
 * Поток, обрабатывающий сообщения для старта игры
 *##############################################################################
 */
void* client_check(void* args)
{
  struct player* my_player = (struct player*)args;
  int net_data[7];
  float tmp;
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
        printf("[network] - Got sync msg from server\n");
        net_data[0] = SYN_REP;
        printf("[network] - net_data[0]\n");
        net_data[1] = my_id;
        printf("[network] - net_data[1]\n");
        net_data[2] = my_player[my_id].x; //  Отделяю целую часть числа
        printf("[network] - net_data[2]\n");
        tmp = my_player[my_id].x - net_data[2]; // Отделяю дробную часть
        net_data[3] = tmp * DIV;  // Умнажаю на DIV чтобы перевести в int
        printf("[network] - net_data[3]\n");
        net_data[4] = my_player[my_id].y;
        tmp = my_player[my_id].y - net_data[4];
        printf("[network] - net_data[4]\n");
        net_data[5] = tmp;
        printf("[network] - net_data[5]\n");
        net_data[6] = my_player[my_id].score;
        printf("n_d[0]:%d n_d[1]:%d n_d[2]:%d n_d[3]:%d n_d[4]:%d n_d[5]:%d\
        n_d[6]:%d\n", net_data[0], net_data[1], net_data[2], net_data[3],
                net_data[4], net_data[5], net_data[6]);
        if(send(tcp_sockfd, net_data, sizeof(net_data), 0) == -1)
        {
          perror("[network] Send");
          exit(1);
        }

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
  int net_data[7];
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
      case SYN_REP:
        /* Описание того как два целых числа собираются в одно дробное.
         * Получаем данные net_data[2] = 111; net_data[3] = 2220;
         * Записываем целую часть дроби в х: x = net_data[2]; x = 111;
         * Переводим net_data[3] в тип float путем топорного умножения на 1,0
         * Затем делим на тот-же DIV, что и использовался при первеводе в целые
         * числа, и прибавляем к х: x += (2220 * 1.0) / DIV; x += 111,2220
         */
        pthread_mutex_lock(&mutex);
        players[net_data[1]].x = net_data[2];
        players[net_data[1]].x += (net_data[3] * 1.0) / DIV;
        players[net_data[1]].y = net_data[4];
        players[net_data[1]].y += (net_data[5] * 1.0) / DIV;
        players[net_data[1]].score = net_data[6];
        pthread_mutex_unlock(&mutex);
        printf("[test sync] - id:%d x:%f y:%f score:%d\n", net_data[1],
              players[net_data[1]].x, players[net_data[1]]. y,
              players[net_data[1]].score);
    }
  }
}
