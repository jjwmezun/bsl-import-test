#include <brotli/decode.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "data.h"
#include "text.h"

#define COMPRESSION_METHOD_BROTLI 1
#define COMPRESSION_METHOD_ZLIB 2
#define COMPRESSION_METHOD COMPRESSION_METHOD_BROTLI

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

void write_gfx_test( uint8_t * pixels, size_t w, size_t h )
{
	FILE * f = fopen( "gfx.txt", "w" );
	if ( ! f )
	{
		printf( "Error opening file!\n" );
		exit( 1 );
	}
	for ( size_t i = 0; i < h; i++ ) {
		for ( size_t j = 0; j < w; j++ ) {
			if ( j % 8 == 0 ) {
				fprintf( f, " " );
			}
			size_t index = i * w + j;
			fprintf( f, "%d", pixels[ index ] );
		}
		if ( i % 8 == 7 ) {
			fprintf( f, "\n" );
		}
		fprintf( f, "\n" );
	}
	fprintf( f, "\n" );
	fclose( f );
}

void write_gfx_test2( uint8_t * pixels, size_t w, size_t h )
{
	FILE * f = fopen( "gfx.txt", "w" );
	if ( ! f )
	{
		printf( "Error opening file!\n" );
		exit( 1 );
	}
	for ( size_t i = 0; i < h; i++ ) {
		for ( size_t j = 0; j < w; j++ ) {
			if ( j % 3 == 0 ) {
				if ( j % 24 == 0 ) {
					fprintf( f, " |" );
				}
				fprintf( f, " " );
			}
			size_t index = i * w + j;
			fprintf( f, "%d", pixels[ index ] );
		}
		if ( i % 8 == 7 ) {
			fprintf( f, "\n" );
		}
		fprintf( f, "\n" );
	}
	fprintf( f, "\n" );
	fclose( f );
}

void write_gfx_test3( uint8_t * pixels, size_t len )
{
	FILE * f = fopen( "gfx.txt", "w" );
	if ( ! f )
	{
		printf( "Error opening file!\n" );
		exit( 1 );
	}
	for ( size_t i = 0; i < len; i++ ) {
		fprintf( f, "%02hhX ", pixels[ i ] );
	}
	fprintf( f, "\n" );
	fclose( f );
}

data_t decode_palette( data_t data )
{
	unsigned char count = *data.data++;
	data.size--;
	printf( "Palette count: %d\n\n", count );

	for ( size_t i = 0; i < count; i++ ) {
		decoded_text_data_t name_data = decode_text( data.data );
		printf( "Palette #%ld: %s\n", i + 1, name_data.text );
		free( name_data.text );
		data = ( data_t ){ name_data.remaining_data, name_data.remaining_data_size };

		for ( size_t j = 0; j < 7; j++ ) {
			uint16_t color = ( uint16_t )( data.data[ 1 ] ) | ( ( uint16_t )( data.data[ 0 ] ) << 8 );
			data.size -= 2;
			data.data += 2;
			uint8_t red = ( ( color >> 11 ) & 0x1F ) * 8;
			uint8_t green = ( ( color >> 6 ) & 0x1F ) * 8;
			uint8_t blue = ( ( color >> 1 ) & 0x1F ) * 8;
			printf( "Color #%ld: 0x%04X = %d, %d, %d\n", j + 1, color, red, green, blue );
		}
	}

	return data;
}

void get_bits_from_byte( unsigned char byte, uint8_t * bits )
{
	/*
	for ( int i = 7; i >= 0; i-- ) {
		bits[ i ] = ( byte >> ( 7 - i ) ) & 1;
	}*/
	/*
	for ( int i = 0; i < 8; i++ ) {
		bits[ i ] = ( byte >> ( 7 - i ) ) & 1;
	}*/
	for ( int i = 0; i < 8; i++ ) {
		bits[ i ] = ( byte >> ( 7-i ) ) & 1;
	}
}

int main()
{
	data_t orig_data = load_data( "assets/out.had" );
	data_t data = orig_data;
	write_data_test( data );

	data = decode_palette( data );

	size_t block_gfx_decompressed_len = ( size_t )( ceil( 512.0 * 512.0 * ( 3.0 / 8.0 ) ) );
	size_t block_gfx_compressed_len = ( uint32_t )( data.data[ 3 ] ) | ( ( uint32_t )( data.data[ 2 ] ) << 8 ) | ( ( uint32_t )( data.data[ 1 ] ) << 16 ) | ( ( uint32_t )( data.data[ 0 ] ) << 24 );
	data.size -= 4;
	data.data += 4;
	printf( "\nBlock GFX length: %zu -> %zu\n", block_gfx_decompressed_len, block_gfx_compressed_len );


	BrotliDecoderState* state = BrotliDecoderCreateInstance( NULL, NULL, NULL );
	unsigned char* decoded = ( unsigned char * )( malloc( block_gfx_decompressed_len) );
	BrotliDecoderResult result = BrotliDecoderDecompress( block_gfx_compressed_len, data.data, &block_gfx_decompressed_len, decoded );
	if ( result != BROTLI_DECODER_RESULT_SUCCESS ) {
		printf( "Brotli decompression failed\n" );
		free( decoded );
		return 1;
	}
	BrotliDecoderDestroyInstance( state );

	uint8_t total_bits[ 512 * 512 * 3 ];
	memset( total_bits, 0, block_gfx_decompressed_len );
	size_t total_bits_used = 0;
	uint8_t pixels[ 512 * 512 ];
	memset( pixels, 0, 512 * 512 );
	uint8_t bits[ 10 ];
	memset( bits, 0, 10 );
	size_t pixels_used = 0;
	size_t bits_used = 0;
	size_t bits_index = 0;

	for ( size_t i = 0; i < block_gfx_decompressed_len; i++ ) {
		get_bits_from_byte( decoded[ i ], &bits[ bits_used ] );
		bits_used += 8;
		while ( bits_used >= 3 ) {
			total_bits[ total_bits_used++ ] = bits[ bits_index ];
			total_bits[ total_bits_used++ ] = bits[ bits_index+1 ];
			total_bits[ total_bits_used++ ] = bits[ bits_index+2 ];
			pixels[ pixels_used++ ] = ( bits[ bits_index++ ] << 2 ) | ( bits[ bits_index++ ] << 1 ) | bits[ bits_index++ ];
			bits_used -= 3;
		}
		for ( size_t j = 0; j < bits_used; j++ ) {
			bits[ j ] = bits[ bits_index++ ];
		}
		bits_index = 0;
	}

	if ( bits_used > 0 ) {
		printf( "Warning: %zu unused bits remaining after decoding\n", bits_used );
	}

	printf( "Decoded %zu pixels / %u\n", pixels_used, 512 * 512 );

	write_gfx_test( pixels, 512, 512 );

	free( decoded );

	destroy_text_system();

	if ( orig_data.data ) {
		free( orig_data.data );
	}

	return 0;
}
