#ifndef MAIN_GFX_H
#define MAIN_GFX_H

#include "palette.h"

void draw_main_gfx();
void init_main_gfx( const unsigned char * pixels, palette_t palette );
void update_main_gfx_palette( palette_t palette );

#endif // MAIN_GFX_H
