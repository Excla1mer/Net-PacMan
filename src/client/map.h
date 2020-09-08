#ifndef MAP_H
#define MAP_H

#include <SFML/Graphics.h>

#define HEIGHT_MAP 31     // размер карты высота
#define WIDTH_MAP 28      // размер карты ширина
#define BLOCK 30          // размер блока
#define MAX_DOTS 10     // количетсво очков на карте

void draw_map(sfRenderWindow* window, sfSprite* map_sprite);

#endif // MAP_H
