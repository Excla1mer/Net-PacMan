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
#include <semaphore.h>

#include "player.h"
#include "help_sets.h"
#include "map.h"
#include "../net_data_defs.h"
#include "globals.h"
#include "network.h"

int main()
{
/*##############################################################################
 * Объявление и определение/подготовка данных
 *##############################################################################
 */
  sem_init(&sem, 0, 0);
  int port = 7777;
  char score[8];
  struct sockaddr_in server, cliaddr;
  char input[50];
  int net_data[3];

  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
  server.sin_port = htons(SERVER_PORT);

  memset(&cliaddr, 0, sizeof(cliaddr));
  cliaddr.sin_family = AF_INET;
  cliaddr.sin_addr.s_addr = INADDR_ANY;
  cliaddr.sin_port = htons(port);
  if(pthread_mutex_init(&mutex, NULL))
  {
    printf("[main] - Error init mutex");
    exit(-1);
  }
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

  struct player* players = calloc(sizeof(struct player), 1);
  /* Поток для обработки состояний поключившихся и нажавших READY клиентов */
  pthread_t client_check_tid;
  if(pthread_create(&client_check_tid, NULL, client_check, (void*)players) != 0)
  {
    perror("[main] - Create client_check thread");
    exit(-1);
  }

  while(1)
  {
    memset(input, '\0', sizeof(input));
    fgets(input, 50, stdin);

    if(strcmp(input, "READY\n") == 0)
    {
      printf("[main] - Send READY to server\n");
      net_data[0] = READY;
      net_data[1] = -1;
      net_data[2] = -1;
      if(send(tcp_sockfd, net_data, sizeof(net_data), TCP_NODELAY) == -1)
      {
        perror("[main] Send");
        exit(1);
      }
      break;
    }
  }
  sem_wait(&sem);
  printf("[main] - Wait all players...\n");
  server.sin_port = htons(udp_server_port);
  printf("udp_server_port: %d\n", udp_server_port);
  printf("[main] - max_players: %d\n", max_players);
/*##############################################################################
 * Подготовка к началу игрового цикла
 *##############################################################################
 */
  sfIntRect rect = {0, 0, 0, 0};
  players = realloc(players, sizeof(struct player) * (max_players));
  init_players(players, max_players, &rect);
  pthread_t* threads = malloc(sizeof(max_players) * sizeof(pthread_t));

  /* Создание слушающего потока для принятия данных */
  pthread_t listen_thread;
  pthread_create(&listen_thread, NULL, net_check, (void*)players);

  /* Настройка размера окна */
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
   if(!window)
    return 1;

   /* Создание часов CSFML */
  sfClock* clock = sfClock_create();

/*##############################################################################
 * Начало игрового цикла
 *##############################################################################
 */
  while(sfRenderWindow_isOpen(window))
  {
    /* Отслеживание времени */
    sfTime time = sfClock_getElapsedTime(clock);
    ttime = sfTime_asMicroseconds(time);
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
      set_netdata(net_data, -1, -1, 1);
      sendto(udp_sockfd, net_data, sizeof(net_data), 0,
          (struct sockaddr*)&server, sizeof(server));
      printf("Send left key\n");

    }
    if(sfKeyboard_isKeyPressed(sfKeyRight) && players[my_id].last_dir != 0)
    {
      players[my_id].last_dir = 0;
      set_netdata(net_data, -1, -1, 0);
      sendto(udp_sockfd, net_data, sizeof(net_data), 0,
          (struct sockaddr*)&server, sizeof(server));
      printf("Send right key\n");
    }
    if(sfKeyboard_isKeyPressed(sfKeyUp) && players[my_id].last_dir != 3)
    {
      players[my_id].last_dir = 3;
      set_netdata(net_data, -1, -1, 3);
      sendto(udp_sockfd, net_data, sizeof(net_data), 0,
          (struct sockaddr*)&server, sizeof(server));
      printf("Send up key\n");
    }
    if(sfKeyboard_isKeyPressed(sfKeyDown) && players[my_id].last_dir != 2)
    {
      players[my_id].last_dir = 2;
      set_netdata(net_data, -1, -1, 2);
      sendto(udp_sockfd, net_data, sizeof(net_data), 0,
          (struct sockaddr*)&server, sizeof(server));
      printf("Send down key\n");
    }
    /* Определение победителя после того, как все очки собраны */
    if(dots >= MAX_DOTS)
    {
      int loose = 0;
      for(int i = 0; i < max_players; ++i)
      {
        if(players[my_id].score < players[i].score)
        {
          printf("You loose\n");
          loose = 1;
          pthread_join(client_check_tid, NULL);
          break;
        }
      }
      if(loose)
        break;
      set_netdata(net_data, ENDGAME, my_id, -1);
      if(send(tcp_sockfd, net_data, sizeof(net_data), 0) == -1)
      {
        perror("END message");
        exit(-1);
      }
      printf("You win!\n");
      pthread_join(client_check_tid, NULL);
      break;
    }
    /* Обновление данных игрока */
    pthread_mutex_lock(&mutex);
    for(int i = 0; i < max_players; ++i)
    {
      if(pthread_create(&threads[i], NULL, update, (void*)&players[i]) != 0)
      {
        perror("[main] - Create update thread");
        exit(-1);
      }
    }
    for(int i = 0; i < max_players; ++i)
    {
      pthread_join(threads[i], NULL);
    }
    pthread_mutex_unlock(&mutex);

    /* Очистка окна*/
    sfRenderWindow_clear(window, sfBlack);
    /* Отрисовка карты */
    draw_map(window, map_sprite);

    /* Отрисовка правого меню других игроков */
    /* TODO: засунуть в поток update!!! */
    int place = 1;
    for(int i = 0; i < max_players; ++i)
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
  for(int i = 0; i < max_players; ++i)
  {
    pthread_cancel(threads[i]);
  }
  sfText_destroy(score_text);
  sfFont_destroy(font);
  sfRenderWindow_destroy(window);
  free(players);
  close(tcp_sockfd);
  close(udp_sockfd);
  return 0;
}
