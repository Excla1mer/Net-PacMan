#include <SFML/Graphics.h>
#include <stdio.h>

/*##############################################################################
 * Функции для заполнения структур/массивов
 *##############################################################################
 */
void set_rect(sfIntRect* rectangle, int l, int t, int w, int h)
{
  rectangle->left = l;
  rectangle->top = t;
  rectangle->height = w;
  rectangle->width = h;
}

void set_vec(sfVector2f* vec, float x, float y)
{
  vec->x = x;
  vec->y = y;
}

void set_netdata(int* net_data, int a, int b, int c, int d,
    int e, int f, int g)
{
  net_data[0] = a;
  net_data[1] = b;
  net_data[2] = c;
  net_data[3] = d;
  net_data[4] = e;
  net_data[5] = f;
  net_data[6] = g;
}
/*##############################################################################
 * Функции для установки параметров SFML объектов
 *##############################################################################
 */
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
