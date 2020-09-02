#include <SFML/Graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define HEIGHT_MAP 31 // размер карты высота
#define WIDTH_MAP 28  // размер карты ширина
#define BLOCK 30      // размер блока
#define RECV_PORT 8888
#define PORT 1234  		// TCP порт
char map[HEIGHT_MAP][WIDTH_MAP] = {
	"1------------21------------2",
	"|            ||            |",
	"| 1--2 1---2 || 1---2 1--2 |",
	"| |**| |***| || |***| |**| |",
	"| 4--3 4---3 43 4---3 4--3 |",
	"|                          |",
	"| 1--2 12 1------2 12 1--2 |",
	"| 4--3 || 4--21--3 || 4--3 |",
	"|      ||    ||    ||      |",
	"4----2 |4--2 || 1--3| 1----3",
	"*****| |1--3 43 4--2| |*****",
	"*****| ||          || |*****",
	"*****| || 1------2 || |*****",
	"d----3 43 |******| 43 4----u",
	"r*****    |******|    *****l",
	"d----2 12 |******| 12 1----u",
	"*****| || 4------3 || |*****",
	"*****| ||          || |*****",
	"*****| || 1------2 || |*****",
	"1----3 43 4--21--3 43 4----2",
	"|            ||            |",
	"| 1--2 1---2 || 1---2 1--2 |",
	"| 4-2| 4---3 43 4---3 |1-3 |",
	"|   ||                ||   |",
	"4-2 || 12 1------2 12 || 1-3",
	"1-3 43 || 4--21--3 || 43 4-2",
	"|      ||    ||    ||      |",
	"| 1----34--2 || 1--34----2 |",
	"| 4--------3 43 4--------3 |",
	"|                          |",
	"4--------------------------3",
};

struct player
{
  float x, y, dx, dy, speed, cur_frame;
  int dir, last_dir, w, h;
  sfSprite* sprite;
  sfTexture* texture;
  sfImage* image;
};

struct player_stat {
	float x;
	float y;
	int score;
};

void set_rect(sfIntRect* rectangle, int l, int t, int w, int h)
{
  rectangle->left = l;
  rectangle->top = t;
  rectangle->height = w;
  rectangle->width = h;
}

void init_player(struct player* p, char* f, float x, float y, int h, int w,
    sfIntRect rect)
{
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
      if(map[i][j] == ' ')
      {
        map[i][j] = '*';
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
          p->y = i * BLOCK + BLOCK;
          return;
        }
        if(p->dx>0)
        {
          p->x = j * BLOCK - p->w;
          return;
        }
        if(p->dx < 0)
        {
          p->x = j * BLOCK + BLOCK;
          return;
        }
      }
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
  //printf("x: %f   y: %f\n", p->x, p->y);
  action_with_map(p);
  sfVector2f pos = {p->x, p->y};
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
/*
 * Синхронизирующий поток, работает по TCP. Получает запрос от клиента на 
 * синхронизацию, в ответ отправляет структуру player_stat с координатами 
 * пакмена и его набранными очками.
 */
void *sinc_thread(void  *param){
	struct player *player1 = (struct player*)param;
	struct player_stat player_sinc;
	struct sockaddr_in server;
	int n;
	int sign;
	int pt_fd;
	if((pt_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket:");
    exit(1);
  }
        
	memset(&server, 0, sizeof(server));
  server.sin_family    = AF_INET;  
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(PORT);
	if(connect(pt_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
		perror("connect");
		exit(1);
	}
	printf("[sinc_thread] - connecting to server\n");
	
	while(1){
		printf("[sinc_thread] - waiting signal\n");
		if((n = recv(pt_fd, &sign, sizeof(int), 0)) == -1) {
			perror("sinc_thread Recv");
			exit(1);
		}
		player_sinc.x = player1->x;
		player_sinc.y = player1->y;
		player_sinc.score = player1->speed; /* Надо будет узнать какая переменная 
		* отвечает за очки
		*/
		printf("[sinc_thread] - Got signal, sending struct: x=%f y=%f score=%d\n", 
				player_sinc.x, player_sinc.y, player_sinc.score);
		
		if(send(pt_fd, &player_sinc, sizeof(struct player_stat), 0) == -1) {
			perror("sinc_thread Send");
			exit(1);
		}
	

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
	// Файловый дескриптор для подключения к серверу TCP
	int fd;
	int n;
	char player_id[50]="hello\0";
	struct sockaddr_in server;
  sfIntRect rect = {0, 0, 0, 0};
  struct player* player1 = calloc(sizeof(struct player), 1);
  struct player* player2 = calloc(sizeof(struct player), 1);
  set_rect(&rect, 1, 1, 30, 30);
  init_player(player1, "textures/PacmanYellowEyes2.png", 30, 30, 19, 19, rect);
  set_rect(&rect, 1, 93, 30, 30);
  init_player(player2, "textures/PacmanRedEyes2.png", 26*30, 30, 19, 19, rect);
  
  pthread_t listen_thread;
  pthread_create(&listen_thread, NULL, net_check, (void*)player2);

  pthread_t sinc_thread_tid; 

  if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("Socket:");
      exit(1);
  }
        
	memset(&server, 0, sizeof(server));
  server.sin_family    = AF_INET;  
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(PORT);
  
	if(connect(fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
		perror("[main] connect");
		exit(1);
	}
	
	printf("[main] connecting to server\n");
	/*if(send(fd, player_id, strlen(player_id), 0) == -1) 
	{
		perror("[main] Send");
		exit(1);
	}*/
	printf("[main] wait player_id\n");
	if((n = recv(fd, player_id, sizeof(player_id), 0)) == -1) 
	{
		perror("[main] Recv");
		exit(1);
	}
	printf("[main] - Got player_id from server: %s \n", player_id);
	pthread_create(&sinc_thread_tid, NULL, sinc_thread, &player1);

  sfVideoMode mode = {1150, 950, 32};
  sfRenderWindow* window;
  sfEvent event;
  sfVector2f vec = {0, 0};

  sfSprite* map_sprite = sfSprite_create();
  sfImage* map_image = sfImage_createFromFile("textures/Walls.png");
  sfTexture* map_texture = sfTexture_createFromImage(map_image, NULL);
  sfSprite_setTexture(map_sprite, map_texture, sfTrue);

  window = sfRenderWindow_create(mode, "PAC-MAN", sfResize | sfClose, NULL);
  
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
          set_rect(&rect, 60, 60, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect);
        }
        if((map[i][j] == '*') || (map[i][j] == 't') || (map[i][j] == 'r'))
        {
          set_rect(&rect, 0, 0, 30, 30);
          sfSprite_setTextureRect(map_sprite, rect);
        }
        //printf("MAP: x:%d    y:%d\n", i*7, j*7);
        set_vec(&vec, j*30, i*30);
        sfSprite_setPosition(map_sprite, vec);
        sfRenderWindow_drawSprite(window, map_sprite, NULL);
    }
    sfRenderWindow_drawSprite(window, player1->sprite, NULL);
    sfRenderWindow_drawSprite(window, player2->sprite, NULL);
    sfRenderWindow_display(window);
  }
  /* Cleanup resources */
  sfRenderWindow_destroy(window);
  return 0;
}
