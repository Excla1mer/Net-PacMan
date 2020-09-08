#include "player.h"
#include "help_sets.h"
#include "map.h"
#include "../net_data_defs.h"
#include "globals.h"

extern char map[HEIGHT_MAP][WIDTH_MAP];

char colors[4][32] = {
  "textures/PacmanYellowEyes2.png\0",
  "textures/PacmanRedEyes2.png\0",
  "textures/PacmanPurpleEyes2.png\0",
  "textures/PacmanGreenEyes2.png\0"
};

/*##############################################################################
 * Инициализация игроков
 *##############################################################################
 */
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
/*##############################################################################
 * Установка положение каждого игрока
 *##############################################################################
 */
void* update(void* args)
{
  struct player* p = (struct player*)args;
  sfIntRect rect = {0, 0, 0, 0};
  switch(p->dir)
  {
    case 0:
      p->speed = 0.1;
      p->cur_frame += 0.005 * ttime;
      if(p->cur_frame > 4)
        p->cur_frame -= 4;
      set_rect(&rect, 30 * (int)p->cur_frame + 1, 1, 30, 30);
      sfSprite_setTextureRect(p->sprite, rect);
      p->dx = p->speed;
      p->dy = 0;
      break;
    case 1:
      p->speed = 0.1;
      p->cur_frame += 0.005 * ttime;
      if(p->cur_frame > 4)
        p->cur_frame -= 4;
      set_rect(&rect, 30 * (int)p->cur_frame + 1, 93, 30, 30);
      sfSprite_setTextureRect(p->sprite, rect);
      p->dx = -p->speed;
      p->dy = 0;
      break;
    case 2:
      p->speed = 0.1;
      p->cur_frame += 0.005 * ttime;
      if(p->cur_frame > 4)
        p->cur_frame -= 4;
      set_rect(&rect, 30 * (int)p->cur_frame + 1, 32, 30, 30);
      sfSprite_setTextureRect(p->sprite, rect);
      p->dx = 0;
      p->dy = p->speed;
      break;
  case 3:
    p->speed = 0.1;
    p->cur_frame += 0.005 * ttime;
    if(p->cur_frame > 4)
      p->cur_frame -= 4;
    set_rect(&rect, 30 * (int)p->cur_frame + 1, 62, 30, 30);
    sfSprite_setTextureRect(p->sprite, rect);
    p->dx = 0;
    p->dy = -p->speed;
    break;
  }
  p->x += p->dx * ttime;
  p->y += p->dy * ttime;
  p->speed = 0;
  action_with_map(p);
  sfVector2f pos = {p->x, p->y};
  sfSprite_setPosition(p->sprite, pos);
  return (void*)0;
}
/*##############################################################################
 * Взаимодействие игрока с картой
 *##############################################################################
 */
void action_with_map(struct player* p)
{
  for(int i = (p->y + 8) / BLOCK; i < (p->y + p->h) / BLOCK; i++)
    for(int j = (p->x + 8) / BLOCK; j < (p->x + p->w) / BLOCK; j++)
    {
      if(map[i][j] == ' ')
      {
        map[i][j] = '*';
        p->score += 1;
        ++dots;
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
