#include <SFML/Graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define HEIGHT_MAP 25 // размер карты высота
#define WIDTH_MAP 34  // размер карты ширина
#define BLOCK 30      // размер блока
#define RECV_PORT 8888

char map[HEIGHT_MAP][WIDTH_MAP] = {
	"1-D------------------------------2",
	"| |                              |",
	"| 4-l                            |",
	"|                                |",
	"| 1-l r-2                        |",
	"| |     |                        |",
	"| |     |                        |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"|                                |",
	"4--------------------------------3",
};

struct player
{
  float x, y, dx, dy, speed, cur_frame;
  int dir, last_dir, w, h;
  sfSprite* sprite;
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

void init_player(struct player* p, char* f, float x, float y, int h, int w)
{
  sfIntRect rect = {1, 1, 30, 30};
  p->w = w;
  p->h = h;
  p->x = x;
  p->y = y;
  p->speed = 0;
  p->dir = -1;
  p->image = sfImage_createFromFile(f);
  p->texture = sfTexture_createFromImage(p->image, NULL);
  p->sprite = sfSprite_create();
  sfSprite_setTexture(p->sprite, p->texture, sfTrue);
  sfSprite_setTextureRect(p->sprite, rect);
}

void action_with_map(struct player* p)
{
  for(int i = (p->y) / BLOCK; i < (p->y + p->h) / (BLOCK); i++)
    for(int j = (p->x) / BLOCK; j<(p->x + p->w) / (BLOCK); j++)
    {
      if((map[i][j] != ' ') && (map[i][j] != '*'))
      {
        if(p->dy>0)
        {
          p->y = i * BLOCK - p->h;
        }
        if(p->dy<0)
        {
          p->y = i * BLOCK + BLOCK;
        }
        if(p->dx>0)
        {
          p->x = j * BLOCK - p->w;
        }
        if(p->dx < 0)
        {
          p->x = j * BLOCK + BLOCK;
        }
      }
      if(map[i][j] == ' ')
        map[i][j] = '*';
    }
}

void update(struct player* p, float time)
{
  sfIntRect rect = {0, 0, 0, 0};
  switch(p->dir)
  {
    case 0:
      p->speed = 0.1;
      p->cur_frame += 0.005*time;
      if(p->cur_frame>4)
        p->cur_frame -= 4;
      set_rect(&rect, 30*(int)p->cur_frame+(int)p->cur_frame+1, 1, 30, 30);
      sfSprite_setTextureRect(p->sprite, rect);
      p->dx = p->speed;
      p->dy = 0;
      break;
    case 1:
      p->speed = 0.1;
      p->cur_frame += 0.005*time;
      if(p->cur_frame>4)
        p->cur_frame -= 4;
      set_rect(&rect, 30*(int)p->cur_frame+(int)p->cur_frame+1, 93, 30, 30);
      sfSprite_setTextureRect(p->sprite, rect);
      p->dx = -p->speed;
      p->dy = 0;
      break;
    case 2:
      p->speed = 0.1;
      p->cur_frame += 0.005*time;
      if(p->cur_frame>4)
        p->cur_frame -= 4;
      set_rect(&rect, 30*(int)p->cur_frame+(int)p->cur_frame+1, 31, 30, 30);
      sfSprite_setTextureRect(p->sprite, rect);
      p->dx = 0;
      p->dy = p->speed;
      break;
    case 3:
      p->speed = 0.1;
      p->cur_frame += 0.005*time;
      if(p->cur_frame>4)
        p->cur_frame -= 4;
      set_rect(&rect, 30*(int)p->cur_frame+(int)p->cur_frame+1, 62, 30, 30);
      sfSprite_setTextureRect(p->sprite, rect);
      p->dx = 0;
      p->dy = -p->speed;
      break;
  }

  p->x += p->dx*time;
  p->y += p->dy*time;

  p->speed = 0;
  sfVector2f pos = {p->x, p->y};
  //printf("x: %f   y: %f\n", p->x, p->y);
  action_with_map(p);
  sfSprite_setPosition(p->sprite, pos);
}



void set_vec(sfVector2f* vec, float x, float y)
{
  vec->x = x;
  vec->y = y;
}

void* net_check(void* args)
{
  struct sockaddr_in servaddr, cliaddr;
  servaddr.sin_family = AF_INET; 
  servaddr.sin_port = htons(RECV_PORT); 
  servaddr.sin_addr.s_addr = INADDR_ANY;
 
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd<0)
  {
    perror("socket");
    exit(-1);
  }
  if(bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr))<0)
  {
    perror("bind");
    exit(-1);
  }

  socklen_t len = sizeof(struct sockaddr_in);

  struct player* player1 = calloc(sizeof(struct player), 1);
  player1 = (struct player*)args;

  char* buf = calloc(sizeof(int), 1);

  int dir;
  while(1)
  {
    recvfrom(sockfd, buf, sizeof(int), MSG_WAITALL,
        (struct sockaddr*)&cliaddr, &len);
    dir = atoi(buf);
    player1->dir = dir;
    printf("%d\n", dir);
  }

}

int main()
{
  /* Инициализировать массив структур player размером, равному числу игроков,
   * полученному от сервера при коннекте к нему.
   * Отрисовка будет происходить в главном потоке.
   * В слушающем потоке будут приниматься ID игрока, что соответсвует номеру
   * в массиве структур, и направление его движения.
   * */
  struct player* player1 = calloc(sizeof(struct player), 1);
  struct player* player2 = calloc(sizeof(struct player), 1);
  init_player(player1, "textures/PacmanYellowEyes2.png", 30, 30, 18, 18);
  init_player(player2, "textures/PacmanRedEyes2.png", 130, 310, 18, 18);
  
  pthread_t listen_thread;
  pthread_create(&listen_thread, NULL, net_check, (void*)player2);

  sfVideoMode mode = {1024, 800, 32};
  sfCircleShape* circle;
  sfRenderWindow* window;
  sfEvent event;
  sfVector2f vec = {0, 0};

  sfSprite* map_sprite = sfSprite_create();
  sfImage* map_image = sfImage_createFromFile("textures/Walls.png");
  sfTexture* map_texture = sfTexture_createFromImage(map_image, NULL);
  sfSprite_setTexture(map_sprite, map_texture, sfTrue);

  sfRenderStates* states;
  window = sfRenderWindow_create(mode, "PAC-MAN", sfResize | sfClose, NULL);
  
  sfIntRect rect = {0, 0, 0, 0};
  sfClock* clock = sfClock_create();
  if(!window)
    return 1;
  /* Start the game loop */
  while(sfRenderWindow_isOpen(window))
  {
    sfTime time = sfClock_getElapsedTime(clock);
    float ttime = sfTime_asMicroseconds(time);
    sfClock_restart(clock);
    ttime /= 800;
    /* Process events */
    while(sfRenderWindow_pollEvent(window, &event))
    {
      /* Close window : exit */
      if(event.type == sfEvtClosed)
        sfRenderWindow_close(window);
    }
    if(sfKeyboard_isKeyPressed(sfKeyRight))
    {
      player1->dir = 0;
    }
    if(sfKeyboard_isKeyPressed(sfKeyLeft))
    {
      player1->dir = 1;

    }
    if(sfKeyboard_isKeyPressed(sfKeyUp))
    {
      player1->dir = 3;

    }
    if(sfKeyboard_isKeyPressed(sfKeyDown))
    {
      player1->dir = 2;
    }

    update(player1, ttime);
    update(player2, ttime);
    /* Clear the screen */
    sfRenderWindow_clear(window, sfBlack);

    for(int i = 0; i < HEIGHT_MAP; i++)
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
        if(map[i][j] == 'R')
        {
          set_rect(&rect, 90, 0, 30, 30);
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
        if(map[i][j] == 'p')
        {
          set_rect(&rect, 30, 30, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect);
        }
        if(map[i][j] == '-')
        {
          set_rect(&rect, 60, 30, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect);
        }
        if(map[i][j] == 'D')
        {
          set_rect(&rect, 90, 30, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect);
        }
        if(map[i][j] == 'U')
        {
          set_rect(&rect, 120, 30, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect);
        }
        if(map[i][j] == 'L')
        {
          set_rect(&rect, 150, 30, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect);
        }
        if(map[i][j] == 'l')
        {
          set_rect(&rect, 60, 60, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect);
        }
        if(map[i][j] == 'r')
        {
          set_rect(&rect, 90, 60, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect);
        }
        if(map[i][j] == 'd')
        {
          set_rect(&rect, 30, 60, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect);
        }
        if(map[i][j] == 'u')
        {
          set_rect(&rect, 0, 60, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect);
        }
        if(map[i][j] == '*')
        {
          set_rect(&rect, 0, 0, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect);
        }
        //printf("MAP: x:%d    y:%d\n", i*7, j*7);
        set_vec(&vec, j*30, i*30);
        sfSprite_setPosition(map_sprite, vec);
        sfRenderWindow_drawSprite(window, map_sprite, states);
    }
    sfRenderWindow_drawSprite(window, player1->sprite, states);
    sfRenderWindow_drawSprite(window, player2->sprite, states);
    sfRenderWindow_display(window);
  }
  /* Cleanup resources */
  sfRenderWindow_destroy(window);
  return 0;
}