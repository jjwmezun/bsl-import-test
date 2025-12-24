#ifndef PALETTE_H
#define PALETTE_H

typedef struct palette_t
{
	uint8_t selected;
	uint8_t len;
	uint16_t * palettes;
} palette_t;

#endif // PALETTE_H
