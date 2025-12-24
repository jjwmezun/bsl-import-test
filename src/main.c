#include "bg_gfx.h"
#include <brotli/decode.h>
#include "check_bg_gfx.h"
#include "constants.h"
#include "gl.h"
#include <GL/glew.h>
#include "main_gfx.h"
#include <math.h>
#include "matrix.h"
#include "palette.h"
#include <SDL2/SDL.h>
#include <SDL_opengl.h>
#include "selected_gfx.h"
#include "selector_gfx.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "text.h"

#define COMPRESSION_METHOD_BROTLI 1
#define COMPRESSION_METHOD_ZLIB 2
#define COMPRESSION_METHOD COMPRESSION_METHOD_BROTLI

static SDL_Window * window;
static int running = 1;
static unsigned int magnification = 4;
static unsigned int screen_width;
static unsigned int screen_height;

static struct {
	unsigned int up : 1;
	unsigned int down : 1;
	unsigned int left : 1;
	unsigned int right : 1;
	unsigned int pgup : 1;
	unsigned int pgdown : 1;
	unsigned int pgup_lock : 1;
	unsigned int pgdown_lock : 1;
	unsigned int up_lock : 1;
	unsigned int down_lock : 1;
	unsigned int left_lock : 1;
	unsigned int right_lock : 1;
} input;

static void update_screen();
static void update_viewport();

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

palette_t decode_palette( data_t * data )
{
	palette_t palette;
	palette.selected = 0;
	palette.len = *data->data++;
	palette.palettes = calloc( palette.len, sizeof( uint16_t ) * 8 );
	data->size--;
	printf( "Palette count: %d\n\n", palette.len );

	for ( size_t i = 0; i < palette.len; i++ ) {
		decoded_text_data_t name_data = decode_text( data->data );
		printf( "Palette #%ld: %s\n", i + 1, name_data.text );
		free( name_data.text );
		data->size = name_data.remaining_data_size;
		data->data = name_data.remaining_data;

		for ( size_t j = 0; j < 7; j++ ) {
			uint16_t color = ( uint16_t )( data->data[ 1 ] ) | ( ( uint16_t )( data->data[ 0 ] ) << 8 );
			palette.palettes[ i * 8 + j + 1 ] = color;
			data->size -= 2;
			data->data += 2;
			uint8_t red = ( ( color >> 11 ) & 0x1F ) * 8;
			uint8_t green = ( ( color >> 6 ) & 0x1F ) * 8;
			uint8_t blue = ( ( color >> 1 ) & 0x1F ) * 8;
			printf( "Color #%ld: 0x%04X = %d, %d, %d\n", j + 1, color, red, green, blue );
		}
	}

	return palette;
}

void get_bits_from_byte( unsigned char byte, uint8_t * bits )
{
	for ( int i = 0; i < 8; i++ ) {
		bits[ i ] = ( byte >> ( 7-i ) ) & 1;
	}
}

int main()
{
	data_t orig_data = load_data( "assets/out.had" );
	data_t data = orig_data;
	write_data_test( data );

	palette_t palette = decode_palette( &data );

	printf( "\n" );
	for ( size_t i = 0; i < palette.len * 8; ++ i )
	{
		printf( "0x%04X ", palette.palettes[ i ] );
		if ( ( i + 1 ) % 8 == 0 )
		{
			printf( "\n" );
		}
	}

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

	free( decoded );

	destroy_text_system();

	if ( orig_data.data ) {
		free( orig_data.data );
	}

	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		fprintf( stderr, "Could not initialize SDL: %s\n", SDL_GetError() );
		return 0;
	}

	// Create window.
	window = SDL_CreateWindow
	(
		"Test Window",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		CANVAS_WIDTH * magnification,
		CANVAS_HEIGHT * magnification,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	if ( ! window )
	{
		fprintf( stderr, "Could not create window: %s\n", SDL_GetError() );
		SDL_Quit();
		return 0;
	}

	// Init input
	memset( &input, 0, sizeof( input ) );

	uint8_t selector_x = 0;
	uint8_t selector_y = 0;

	// Set up viewport.
	screen_width = CANVAS_WIDTH * magnification;
	screen_height = CANVAS_HEIGHT * magnification;
	update_viewport();

	// Set up OpenGL.
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GLContext context = SDL_GL_CreateContext( window );
	glewInit();

	// Enable blending.
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// Enable depth testing.
	glEnable( GL_DEPTH_TEST );

	//init_bg_gfx();
	init_checkered_bg_gfx();
	init_main_gfx( pixels, palette );
	init_selected_gfx( palette, selector_x, selector_y );
	init_selector_gfx( ( float )( selector_x ), ( float )( selector_y ) );

	SDL_Event event;
	while ( running )
	{
		while ( SDL_PollEvent( &event ) )
		{
			switch ( event.type )
			{
				case ( SDL_QUIT ):
					running = 0;
				break;
				case SDL_WINDOWEVENT:
					// On window resize.
					if ( event.window.event == SDL_WINDOWEVENT_RESIZED )
					{
						screen_width = event.window.data1;
						screen_height = event.window.data2;
						update_screen();
					}
				break;
				case SDL_KEYDOWN:
					switch ( event.key.keysym.sym )
					{
						case ( SDLK_ESCAPE ):
						{
							running = 0;
						}
						break;
						case ( SDLK_UP ):
						{
							input.up = 1;
						}
						break;
						case ( SDLK_DOWN ):
						{
							input.down = 1;
						}
						break;
						case ( SDLK_LEFT ):
						{
							input.left = 1;
						}
						break;
						case ( SDLK_RIGHT ):
						{
							input.right = 1;
						}
						break;
						case ( SDLK_PAGEUP ):
						{
							input.pgup = 1;
						}
						break;
						case ( SDLK_PAGEDOWN ):
						{
							input.pgdown = 1;
						}
						break;
					}
				break;
				case SDL_KEYUP:
					switch ( event.key.keysym.sym )
					{
						case ( SDLK_UP ):
						{
							input.up = 0;
							input.up_lock = 0;
						}
						break;
						case ( SDLK_DOWN ):
						{
							input.down = 0;
							input.down_lock = 0;
						}
						break;
						case ( SDLK_LEFT ):
						{
							input.left = 0;
							input.left_lock = 0;
						}
						break;
						case ( SDLK_RIGHT ):
						{
							input.right = 0;
							input.right_lock = 0;
						}
						break;
						case ( SDLK_PAGEUP ):
						{
							input.pgup = 0;
							input.pgup_lock = 0;
						}
						break;
						case ( SDLK_PAGEDOWN ):
						{
							input.pgdown = 0;
							input.pgdown_lock = 0;
						}
						break;
					}
				break;
			}
		}

		// Change palette on pgup or pgdown.
		if ( input.pgup && !input.pgup_lock )
		{
			palette.selected++;
			if ( palette.selected >= palette.len )
			{
				palette.selected = 0;
			}
			update_main_gfx_palette( palette );
			update_selected_gfx_palette( palette );
		}
		else if ( input.pgdown && !input.pgdown_lock )
		{
			if ( palette.selected == 0 )
			{
				palette.selected = palette.len - 1;
			}
			else
			{
				palette.selected--;
			}
			update_main_gfx_palette( palette );
			update_selected_gfx_palette( palette );
		}

		if ( input.right && !input.right_lock )
		{
			if ( selector_x < 63 )
			{
				selector_x++;
				update_selected_gfx( ( float )( selector_x ), ( float )( selector_y ) );
				update_selector_gfx( ( float )( selector_x ), ( float )( selector_y ) );
			}
		}
		else if ( input.left && !input.left_lock )
		{
			if ( selector_x > 0 )
			{
				selector_x--;
				update_selected_gfx( ( float )( selector_x ), ( float )( selector_y ) );
				update_selector_gfx( ( float )( selector_x ), ( float )( selector_y ) );
			}
		}

		if ( input.up && !input.up_lock )
		{
			if ( selector_y > 0 )
			{
				selector_y--;
				update_selected_gfx( ( float )( selector_x ), ( float )( selector_y ) );
				update_selector_gfx( ( float )( selector_x ), ( float )( selector_y ) );
			}
		}
		else if ( input.down && !input.down_lock )
		{
			if ( selector_y < 63 )
			{
				selector_y++;
				update_selected_gfx( ( float )( selector_x ), ( float )( selector_y ) );
				update_selector_gfx( ( float )( selector_x ), ( float )( selector_y ) );
			}
		}

		// Update keylock states.
		if ( input.pgup )
		{
			input.pgup_lock = 1;
		}
		if ( input.pgdown )
		{
			input.pgdown_lock = 1;
		}
		if ( input.up )
		{
			input.up_lock = 1;
		}
		if ( input.down )
		{
			input.down_lock = 1;
		}
		if ( input.left )
		{
			input.left_lock = 1;
		}
		if ( input.right )
		{
			input.right_lock = 1;
		}

		// Clear the screen.
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		draw_selector_gfx();
		draw_selected_gfx();
		draw_main_gfx();
		draw_checkered_bg_gfx();
		//draw_bg_gfx();

		SDL_GL_SwapWindow( window );
	}

	// Clean up.
	free( palette.palettes );
	SDL_GL_DeleteContext( context );
	SDL_DestroyWindow( window );
	SDL_Quit();
	return 0;
}

static void update_screen()
{
	const double screen_aspect_ratio = ( double )( CANVAS_WIDTH ) / ( double )( CANVAS_HEIGHT );
	const double monitor_aspect_ratio = ( double )( screen_width ) / ( double )( screen_height );

	// Base magnification on max that fits in window.
	magnification = 
		( unsigned int )( floor(
			( monitor_aspect_ratio > screen_aspect_ratio )
				? ( double )( screen_height ) / ( double )( CANVAS_HEIGHT )
				: ( double )( screen_width ) / ( double )( CANVAS_WIDTH )
		));

	// Clamp minimum magnification to 1.
	if ( magnification < 1 )
	{
		magnification = 1;
	}

	update_viewport();
}

static void update_viewport()
{
	float viewportw = CANVAS_WIDTH * magnification;
	float viewporth = CANVAS_HEIGHT * magnification;
	float viewportx = floor( ( double )( screen_width - viewportw ) / 2.0 );
	float viewporty = floor( ( double )( screen_height - viewporth ) / 2.0 );
	glViewport( viewportx, viewporty, viewportw, viewporth );
}
