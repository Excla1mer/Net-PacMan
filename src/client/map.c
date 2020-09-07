#include "help_sets.h"
#include "map.h"
#include "player.h"

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

void draw_map(sfRenderWindow* window, sfSprite* map_sprite)
{
  sfVector2f vec = {0, 0};
  sfIntRect rect = {0, 0, 0, 0};
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
}
