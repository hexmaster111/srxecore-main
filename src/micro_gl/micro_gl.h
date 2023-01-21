/* MicroGL - A small ui library
 * Auther: Hailey Gruszynski
 * Date: 2023-JAN-02
 *
 * Version: 0.0.0 -- Gray scale only
 */

#ifndef MICRO_GL_H
#define MICRO_GL_H

//----------------FUNCTIONS THE USER MUST IMPLEMENT----------------//

// -- USER INCLUDES -- //
#include "../lcdbase.h"

void _MGL_draw_pixel(int x, int y, int fg, int bg);
void _MGL_fill_rect(int x, int y, int w, int h, int color);
void _MGL_put_string(int x, int y, char *str, int color);
void _MGL_set_color(int fg, int bg);
int _MGL_get_active_font_height();
int _MGL_get_active_font_width();
//-----------------------------------------------------------------//

//-------------------------EXPOSED API ----------------------------//
//-----------------------------------------------------------------//

#endif