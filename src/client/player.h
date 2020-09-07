#ifndef PLAYER_H
#define PLAUER_H

#include <SFML/Graphics.h>

struct player
{
  int dir, last_dir, w, h, score;
  float x, y, dx, dy, speed, cur_frame;
  sfSprite* sprite;
  sfSprite* icon_sprite;
  sfTexture* texture;
  sfImage* image;
};

void init_players(struct player* p, int max_players, sfIntRect* rect);
void update(struct player* p, float time, int max_players);
void action_with_map(struct player* p);

#endif // PLAYER_H
