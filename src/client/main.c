#include <SFML/Graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "player.h"
#include "help_sets.h"
#include "map.h"

#define SERVER_PORT 1234  // порт сервера

int udp_server_port;
int udp_sockfd;

void* net_check(void* args)
{
  char buf[32];
  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET; 
  servaddr.sin_port = htons(udp_server_port); 
  servaddr.sin_addr.s_addr = inet_addr("185.255.132.26");
 
  socklen_t len = sizeof(struct sockaddr_in);

  struct player* players = (struct player*)args;
  memset(buf, '0', 32);
  while(1)
  {
    if(recvfrom(udp_sockfd, buf, 32, 0, (struct sockaddr*)&servaddr,
        &len) == -1)
    {
      perror("Recv from server");
      exit(-1);
    }
    int a = buf[0] - '0';
    int b = buf[1] - '0';
    players[a].dir = b;
  }
}

void set_netdata(int* net_data, int a, int b, int c)
{
  net_data[0] = a;
  net_data[1] = b;
  net_data[2] = c;
}

int main()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */
  int port = 7777;
  int tcp_sockfd;
  int my_id = 0;
  char score[8];
  int max_players = 4;
  struct sockaddr_in server, cliaddr;
  char *buf = calloc(5, 1);

  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;  
  server.sin_addr.s_addr = inet_addr("185.255.132.26"); 
  server.sin_port = htons(SERVER_PORT);

  cliaddr.sin_family = AF_INET;
  cliaddr.sin_addr.s_addr = INADDR_ANY;
  cliaddr.sin_port = htons(port);
/*##############################################################################
 * Получение данных для настройки от сервера
 *##############################################################################
 */
  if((tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[main] TCP socket");
    exit(1);
  }
  if((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    perror("[main] UDP socket");
    exit(1);
  }
  
  while(bind(tcp_sockfd, (struct sockaddr*)&cliaddr, sizeof(cliaddr)) == -1)
  {
    cliaddr.sin_port = htons(++port);
  }
  printf("[main] - Your port: %d\n", port);
  if(bind(udp_sockfd, (struct sockaddr*)&cliaddr, sizeof(cliaddr)) == -1)
  {
    perror("[main] Bind udp scoket");
    exit(-1);
  }
  printf("[main] - Connecting to server...\n");
  if(connect(tcp_sockfd, (struct sockaddr*)&server, sizeof(server)) == -1)
  {
    perror("[main] connect");
    exit(1);
  }
  printf("[main] - Successful connect\n");
  printf("[main] - Wait data from server...\n");
  bzero(buf, 5);
  if((recv(tcp_sockfd, buf, 5, 0)) == -1) 
  {
    perror("[main] Recv");
    exit(1);
  }
  my_id = atoi(buf);
  printf("[main] - Id: %d\n", my_id);
  if(send(tcp_sockfd, "0001", 5, TCP_NODELAY) == -1) 
  {
    perror("[main] Send");
    exit(1);
  }
  printf("[main] - Wait all players...\n");
  bzero(buf, 5);

  while((recv(tcp_sockfd, buf, 5, 0)) == 0){}
  server.sin_port = htons(atoi(buf));
  udp_server_port = atoi(buf);
  printf("[main] - Port: %s\n", buf);

  while((recv(tcp_sockfd, buf, 5, 0)) == 0){}
  printf("[main] - Some data: %s\n", buf);
/*##############################################################################
 * Подготовка к началу игрового цикла 
 *##############################################################################
 */
  /* Инициализация игроков */
  sfIntRect rect = {0, 0, 0, 0};
  struct player* players = calloc(sizeof(struct player), max_players);
  init_players(players, max_players, &rect);
  
  /* Создание слушающего потока для принятия данных */
  pthread_t listen_thread;
  pthread_create(&listen_thread, NULL, net_check, (void*)players);

  sfVideoMode mode = {1150, 950, 32};
  sfRenderWindow* window;
  sfEvent event;

  /* Инициализация и создание карты */
  sfSprite* map_sprite = sfSprite_create();
  sfImage* map_image = sfImage_createFromFile("textures/Walls.png");
  sfTexture* map_texture = sfTexture_createFromImage(map_image, NULL);
  sfSprite_setTexture(map_sprite, map_texture, sfTrue);

  /* Инициализация и создание текстовых данных */
  sfText* score_text = sfText_create();
  sfText_setColor(score_text, sfWhite);
  sfText_setCharacterSize(score_text, 67);
  sfFont* font = sfFont_createFromFile("textures/Font.otf");
  sfText_setFont(score_text, font);

  /* Создание окна */
  window = sfRenderWindow_create(mode, "PAC-MAN", sfResize | sfClose, NULL);
  
  /* Создание часов CSFML */
  sfClock* clock = sfClock_create();
  if(!window)
    return 1;
/*##############################################################################
 * Начало игрового цикла
 *##############################################################################
 */
  while(sfRenderWindow_isOpen(window))
  {
    /* Отслеживание времени */
    sfTime time = sfClock_getElapsedTime(clock);
    float ttime = sfTime_asMicroseconds(time);
    sfClock_restart(clock);
    ttime /= 800;

    /* Отслеживание события окна */
    while(sfRenderWindow_pollEvent(window, &event))
    {
      if(event.type == sfEvtClosed)
        sfRenderWindow_close(window);
    }

    /* Обработка нажатых клавиш */
    
    if(sfKeyboard_isKeyPressed(sfKeyLeft) && players[my_id].last_dir != 1)
    {
      players[my_id].last_dir = 1;
      sendto(udp_sockfd, "1", 1, 0, (struct sockaddr*)&server, sizeof(server));
      printf("Send left key\n");

    }
    if(sfKeyboard_isKeyPressed(sfKeyRight) && players[my_id].last_dir != 0)
    {
      players[my_id].last_dir = 0;
      sendto(udp_sockfd, "0", 1, 0, (struct sockaddr*)&server, sizeof(server));
      printf("Send right key\n");
    }
    if(sfKeyboard_isKeyPressed(sfKeyUp) && players[my_id].last_dir != 3)
    {
      players[my_id].last_dir = 3;
      sendto(udp_sockfd, "3", 1, 0, (struct sockaddr*)&server, sizeof(server));
      printf("Send up key\n");

    }
    if(sfKeyboard_isKeyPressed(sfKeyDown) && players[my_id].last_dir != 2)
    {
      players[my_id].last_dir = 2;
      sendto(udp_sockfd, "2", 1, 0, (struct sockaddr*)&server, sizeof(server));
      printf("Send down key\n");

    }
    /* Обновление данных игрока */
    update(players, ttime, max_players);
    /* Очистка окна*/
    sfRenderWindow_clear(window, sfBlack);
    /* Отрисовка карты */
    draw_map(window, map_sprite);

    /* Отрисовка правого меню других игроков */
    int place = 1;
    for(int i=0; i < max_players; ++i)
    {
      sfRenderWindow_drawSprite(window, players[i].sprite, NULL);
      if(i != my_id)
      {
        sprintf(score, "%d", players[i].score);
        set_text(score_text, "Score:  %s\n", score, 900, 660 + (place * 40));
        set_icon(players[i].icon_sprite, 850, 695 + (place * 40));
        sfRenderWindow_drawSprite(window, players[i].sprite, NULL);
        sfRenderWindow_drawSprite(window, players[i].icon_sprite, NULL);
        sfRenderWindow_drawText(window, score_text, NULL);
        ++place;
      }
    }

    /* Отрисовка правого меню текущего игрока */
    sprintf(score, "%d", players[my_id].score);
    set_text(score_text, "You: \nScore:  %s\n", score, 850, 20);
    set_icon(players[my_id].icon_sprite, 935, 61);
    sfRenderWindow_drawSprite(window, players[my_id].icon_sprite, NULL);
    sfRenderWindow_drawText(window, score_text, NULL);
    sfRenderWindow_display(window);
  }
/*##############################################################################
 * Освобождение ресурсов
 *##############################################################################
 */
  sfText_destroy(score_text);
  sfFont_destroy(font);
  sfRenderWindow_destroy(window);
  free(players);
  close(tcp_sockfd);
  close(udp_sockfd);
  return 0;
}
