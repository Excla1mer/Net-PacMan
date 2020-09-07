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

#include "map.h"

#define SERVER_PORT 1234  // порт сервера

char colors[4][32] = {
  "textures/PacmanYellowEyes2.png\0",
  "textures/PacmanRedEyes2.png\0",
  "textures/PacmanPurpleEyes2.png\0",
  "textures/PacmanGreenEyes2.png\0"
};

int udp_server_port;
int udp_sockfd;

struct player
{
  int dir, last_dir, w, h, score;
  float x, y, dx, dy, speed, cur_frame;
  sfSprite* sprite;
  sfSprite* icon_sprite;
  sfTexture* texture;
  sfImage* image;
};

void set_rect(sfIntRect* rectangle, int l, int t, int w, int h)
{
  rectangle->left = l;
  rectangle->top = t;
  rectangle->height = w;
  rectangle->width = h;
}

void init_players(struct player* p, int max_players, sfIntRect* rect)
{
  int w = 21;
  int h = 21;
  int place[4][2] = {{30, 30}, {26*30, 30}, {30, 29*30}, {26*30, 29*30}};

  for(int i=0; i<max_players; ++i)
  {
    /* Расстановка игроков по углам карты */
    p[i].x = place[i][0];
    p[i].y = place[i][1];

    /* Инициализация начальных переменных каждого игрока*/
    p[i].w = w;
    p[i].h = h;
    p[i].speed = 0;
    p[i].dir = -1;
    p[i].last_dir = -1;
    p[i].score = 0;
    p[i].image = sfImage_createFromFile(colors[i]);
    p[i].texture = sfTexture_createFromImage(p[i].image, NULL);
    p[i].sprite = sfSprite_create();
    p[i].icon_sprite = sfSprite_create();
    sfSprite_setTexture(p[i].sprite, p[i].texture, sfTrue);
    sfSprite_setTexture(p[i].icon_sprite, p[i].texture, sfTrue);

    /* Поворот текстур в нужную сторону */
    set_rect(rect, 1, (i % 2 ? 93 : 1), 30, 30);
    sfSprite_setTextureRect(p[i].sprite, *rect);
  }
}

void action_with_map(struct player* p)
{
  for(int i = (p->y + 8) / BLOCK; i < (p->y + p->h) / BLOCK; i++)
    for(int j = (p->x + 8) / BLOCK; j < (p->x + p->w) / BLOCK; j++)
    {
      if(map[i][j] == ' ')
      {
        map[i][j] = '*';
        p->score += 1;
        return;
      }
      if(map[i][j] == 'r')
      {
        p->x = WIDTH_MAP * BLOCK - 2 * BLOCK;
        return;
      }
      if(map[i][j] == 'l')
      {
        p->x = BLOCK;
        return;
      }
      if((map[i][j] != ' ') && (map[i][j] != '*'))
      {
        if(p->dy>0)
        {
          p->y = i * BLOCK - p->h;
          return;
        }
        if(p->dy<0)
        {
          p->y = i * BLOCK + BLOCK - 8;
          return;
        }
        if(p->dx>0)
        {
          p->x = j * BLOCK - p->w;
          return;
        }
        if(p->dx < 0)
        {
          p->x = j * BLOCK + BLOCK - 8;
          return;
        }
      }
    }
}

void update(struct player* p, float time, int max_players)
{
  sfIntRect rect = {0, 0, 0, 0};
  for(int i = 0; i < max_players; ++i)
  {
    switch(p[i].dir)
    {
      case 0:
        p[i].speed = 0.1;
        p[i].cur_frame += 0.005 * time;
        if(p[i].cur_frame > 4)
          p[i].cur_frame -= 4;
        set_rect(&rect, 30 * (int)p[i].cur_frame + 1, 1, 30, 30);
        sfSprite_setTextureRect(p[i].sprite, rect);
        p[i].dx = p[i].speed;
        p[i].dy = 0;
        break;
      case 1:
        p[i].speed = 0.1;
        p[i].cur_frame += 0.005 * time;
        if(p[i].cur_frame > 4)
          p[i].cur_frame -= 4;
        set_rect(&rect, 30 * (int)p[i].cur_frame + 1, 93, 30, 30);
        sfSprite_setTextureRect(p[i].sprite, rect);
        p[i].dx = -p[i].speed;
        p[i].dy = 0;
        break;
      case 2:
        p[i].speed = 0.1;
        p[i].cur_frame += 0.005 * time;
        if(p[i].cur_frame > 4)
          p[i].cur_frame -= 4;
        set_rect(&rect, 30 * (int)p[i].cur_frame + 1, 32, 30, 30);
        sfSprite_setTextureRect(p[i].sprite, rect);
        p[i].dx = 0;
        p[i].dy = p[i].speed;
        break;
      case 3:
        p[i].speed = 0.1;
        p[i].cur_frame += 0.005 * time;
        if(p[i].cur_frame > 4)
          p[i].cur_frame -= 4;
        set_rect(&rect, 30 * (int)p[i].cur_frame + 1, 62, 30, 30);
        sfSprite_setTextureRect(p[i].sprite, rect);
        p[i].dx = 0;
        p[i].dy = -p[i].speed;
        break;
    }
    p[i].x += p[i].dx * time;
    p[i].y += p[i].dy * time;
    p[i].speed = 0;
    action_with_map(&p[i]);
    sfVector2f pos = {p[i].x, p[i].y};
    sfSprite_setPosition(p[i].sprite, pos);
  }
}

void set_vec(sfVector2f* vec, float x, float y)
{
  vec->x = x;
  vec->y = y;
}

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

void set_text(sfText* text, const char* frmt, char* buf, float x, float y)
{
  sfVector2f vec = {0, 0};
	char str[32];
  sprintf(str, frmt, buf);
  sfText_setString(text, str);
  set_vec(&vec, x, y);
  sfText_setPosition(text, vec);
}

void set_icon(sfSprite* icon, float x, float y)
{
  sfVector2f vec = {0, 0};
  sfIntRect rect = {0, 0, 0, 0};
  set_vec(&vec, x, y);
  set_rect(&rect, 93, 1, 30, 30);
  sfSprite_setTextureRect(icon, rect);
  sfSprite_setPosition(icon, vec);
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
  int tcp_sockfd;
  int my_id = 0;
  char score[8];
  int max_players = 1;
  struct sockaddr_in server, cliaddr;
  char buf[32];

  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;  
  server.sin_addr.s_addr = inet_addr("185.255.132.26"); 
  server.sin_port = htons(SERVER_PORT);

  cliaddr.sin_family = AF_INET;
  cliaddr.sin_addr.s_addr = INADDR_ANY;
  cliaddr.sin_port = htons(9956);
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
  
  bind(tcp_sockfd, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
  perror("bind1");
  bind(udp_sockfd, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
  perror("bind2");
  
  printf("[main] - Connecting to server...\n");
  if(connect(tcp_sockfd, (struct sockaddr*)&server, sizeof(server)) == -1)
  {
    perror("[main] connect");
    exit(1);
  }
  printf("[main] - Successful connect\n");
  printf("[main] - Wait data from server...\n");
  memset(buf, '0', 32);
  if((recv(tcp_sockfd, buf, 32, 0)) == -1) 
  {
    perror("[main] Recv");
    exit(1);
  }
  my_id = atoi(buf);
  printf("[main] - Id: %d\n", my_id);
  if(send(tcp_sockfd, "1", 1, 0) == -1) 
  {
    perror("[main] Send");
    exit(1);
  }
  printf("[main] - Wait all players...\n");
  memset(buf, '0', 32);

  while((recv(tcp_sockfd, buf, 32, 0)) == 0){}
  while((recv(tcp_sockfd, buf, 32, 0)) == 0){} 

  server.sin_port = htons(atoi(buf));
  udp_server_port = atoi(buf);
  printf("[main] - Port: %s\n", buf);

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
  sfVector2f vec = {0, 0};

  /* Инициализация и создание карты */
  sfSprite* map_sprite = sfSprite_create();
  sfImage* map_image = sfImage_createFromFile("textures/Walls.png");
  sfTexture* map_texture = sfTexture_createFromImage(map_image, NULL);
  sfSprite_setTexture(map_sprite, map_texture, sfTrue);

  /* Инициализация и создание текстовых данных */
  sfText* score_text = sfText_create();
  sfText_setColor(score_text, sfWhite);
  sfText_setCharacterSize(score_text, 50);
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
  server.sin_port = htons(udp_server_port);
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
    
    if(sfKeyboard_isKeyPressed(sfKeyLeft)&& players[my_id].last_dir != 1)
    {
      players[my_id].last_dir = 1;
      sendto(udp_sockfd, "1", 1, 0, (struct sockaddr*)&server, sizeof(server));
      printf("Send left key\n");

    }
    if(sfKeyboard_isKeyPressed(sfKeyRight)&& players[my_id].last_dir != 0)
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
    if(sfKeyboard_isKeyPressed(sfKeyDown)&& players[my_id].last_dir != 2)
    {
      players[my_id].last_dir = 2;
      sendto(udp_sockfd, "2", 1, 0, (struct sockaddr*)&server, sizeof(server));
      printf("Send down key\n");

    }

    update(players, ttime, max_players);
    sfRenderWindow_clear(window, sfBlack);

    /* Отрисовка карты */
    for(int i = 0; i < HEIGHT_MAP; i++)
    {
      for(int j = 0; j < WIDTH_MAP; j++)
      {
        if(map[i][j] == ' ')
        {
          set_rect(&rect, 0, 90, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect);
        }
        if(map[i][j] == '2')
        {
          set_rect(&rect, 30, 0, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect); 
        }
        if(map[i][j] == '1')
        {
          set_rect(&rect, 60, 0, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect); 
        }
        if(map[i][j] == '3')
        {
          set_rect(&rect, 120, 0, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect); 
        }
        if(map[i][j] == '4')
        {
          set_rect(&rect, 150, 0, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect); 
        }
        if(map[i][j] == '|')
        {
          set_rect(&rect, 0, 30, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect); 
        }
        if(map[i][j] == '-')
        {
          set_rect(&rect, 60, 30, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect); 
        }
        if(map[i][j] == 'd')
        {
          set_rect(&rect, 90, 60, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect); 
        }
        if(map[i][j] == 'u')
        {
          sfSprite_setTextureRect(map_sprite, rect); 
        }
        if((map[i][j] == '*') || (map[i][j] == 't') || (map[i][j] == 'r'))
        {
          set_rect(&rect, 0, 0, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect); 
        }
        set_vec(&vec, j*30, i*30);
        sfSprite_setPosition(map_sprite, vec);
        sfRenderWindow_drawSprite(window, map_sprite, NULL);
      }
    }

    /* Отрисовка правого меню других игроков */
    int place = 1;
    for(int i=0; i < max_players; ++i)
    {
      sfRenderWindow_drawSprite(window, players[i].sprite, NULL);
      if(i != my_id)
      {
        sprintf(score, "%d", players[i].score);
        set_text(score_text, "Score:  %s\n", score, 900, 650 + (place * 40));
        set_icon(players[i].icon_sprite, 850, 675 + (place * 40));
        sfRenderWindow_drawSprite(window, players[i].sprite, NULL);
        sfRenderWindow_drawSprite(window, players[i].icon_sprite, NULL);
        sfRenderWindow_drawText(window, score_text, NULL);
        ++place;
      }
    }

    /* Отрисовка правого меню текущего игрока */
    sprintf(score, "%d", players[my_id].score);
    set_text(score_text, "You: \nScore:  %s\n", score, 850, 20);
    set_icon(players[my_id].icon_sprite, 915, 48);
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
