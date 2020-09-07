#ifndef HELP_SETS_H
#define HELP_SETS_H

#include <SFML/Graphics.h>

void set_rect(sfIntRect* rectangle, int l, int t, int w, int h);
void set_vec(sfVector2f* vec, float x, float y);
void set_text(sfText* text, const char* frmt, char* buf, float x, float y);
void set_icon(sfSprite* icon, float x, float y);

#endif // HELP_SETS_H
