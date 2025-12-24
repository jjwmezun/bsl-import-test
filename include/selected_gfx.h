#ifndef SELECTED_GFX_H
#define SELECTED_GFX_H

#include "palette.h"

void draw_selected_gfx();
void init_selected_gfx( palette_t palette, float selector_x, float selector_y );
void update_selected_gfx( float selector_x, float selector_y );
void update_selected_gfx_palette( palette_t palette );

#endif // SELECTED_GFX_H
