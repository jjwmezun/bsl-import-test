#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "data.h"
#include "text.h"

void write_data_test( data_t data )
{
	FILE * f = fopen( "test.txt", "w" );
	if ( ! f )
	{
		printf( "Error opening file!\n" );
		exit( 1 );
	}
	for ( size_t i = 0; i < data.size; i++ ) {
		fprintf( f, "%02hhX ", data.data[ i ] );
	}
	fprintf( f, "\n" );
	fclose( f );
}

void decode_palette( data_t data )
{
	unsigned char count = *data.data++;
	data.size--;
	printf( "Palette count: %d\n\n", count );

	for ( size_t i = 0; i < count; i++ ) {
		decoded_text_data_t name_data = decode_text( data.data );
		printf( "Palette #%ld: %s\n", i + 1, name_data.text );
		data = (data_t){ name_data.remaining_data, name_data.remaining_data_size };

		for ( size_t j = 0; j < 7; j++ ) {
			uint16_t color = ( uint16_t )( data.data[ 1 ] ) | ( ( uint16_t )( data.data[ 0 ] ) << 8 );
			data.size -= 2;
			data.data += 2;
			uint8_t red = ( ( color >> 11 ) & 0x1F ) * 8;
			uint8_t green = ( ( color >> 6 ) & 0x1F ) * 8;
			uint8_t blue = ( ( color >> 1 ) & 0x1F ) * 8;
			printf( "Color #%ld: %04X = %d, %d, %d\n", j + 1, color, red, green, blue );
		}
	}
}

int main()
{
	data_t data = load_data( "assets/out.had" );
	write_data_test( data );

	decode_palette( data );

	if ( data.data ) {
		free( data.data );
	}
	return 0;
}
