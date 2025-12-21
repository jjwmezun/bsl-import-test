#include <brotli/decode.h>
#include <GL/glew.h>
#include <math.h>
#include "matrix.h"
#include <SDL2/SDL.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "text.h"

#define COMPRESSION_METHOD_BROTLI 1
#define COMPRESSION_METHOD_ZLIB 2
#define COMPRESSION_METHOD COMPRESSION_METHOD_BROTLI

#define SHADER_TYPE_VERTEX   0x00
#define SHADER_TYPE_FRAGMENT 0x01

#define CANVAS_WIDTH  512
#define CANVAS_HEIGHT 320

typedef struct shader_t
{
	const char * src;
	unsigned int type;
}
shader_t;

static SDL_Window * window;
static int running = 1;
static unsigned int magnification = 4;
static unsigned int screen_width;
static unsigned int screen_height;

static GLuint compile_shader( const shader_t * shaders, size_t count );
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

	GLuint bg_shader_program;
	GLuint bg_vao;
	{
		const char * vertex_shader_source =
			"#version 330 core\n"
			"layout (location = 0) in vec2 i_position;\n"
			"void main()\n"
			"{\n"
			"   gl_Position = vec4(i_position, 0.0, 1.0);\n"
			"}\n";
		const char * fragment_shader_source =
			"#version 330 core\n"
			"out vec4 f_color;\n"
			"void main()\n"
			"{\n"
			"   f_color = vec4( 1.0, 1.0, 1.0, 1.0 );\n"
			"}\n";
		shader_t shaders[] =
		{
			{ vertex_shader_source, SHADER_TYPE_VERTEX },
			{ fragment_shader_source, SHADER_TYPE_FRAGMENT }
		};
		bg_shader_program = compile_shader( shaders, 2 );
		glUseProgram( bg_shader_program );

		float vertices[] = {
			-1.0f, -1.0f,
			1.0f, -1.0f,
			1.0f, 1.0f,
			-1.0f, 1.0f
		};
		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0
		};
		glGenVertexArrays( 1, &bg_vao );
		glBindVertexArray( bg_vao );
		GLuint vbo, ebo;
		glGenBuffers( 1, &vbo );
		glGenBuffers( 1, &ebo );
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, 0 );
		glEnableVertexAttribArray( 0 );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );
	}

	GLuint check_bg_shader_program;
	GLuint check_bg_vao;
	GLuint check_bg_texture;
	{
		const char * vertex_shader_source =
			"#version 330 core\n"
			"layout (location = 0) in vec4 i_position;\n"
			"layout (location = 1) in vec2 i_texcoord;\n"
			"out vec2 v_texcoord;\n"
			"void main()\n"
			"{\n"
			"   gl_Position = i_position;\n"
			"   v_texcoord = i_texcoord;\n"
			"}\n";
		const char * fragment_shader_source =
			"#version 330 core\n"
			"out vec4 f_color;\n"
			"in vec2 v_texcoord;\n"
			"uniform sampler2D u_texture;\n"
			"void main()\n"
			"{\n"
			"	float v = texture( u_texture, v_texcoord ).r;\n"
			"   f_color = vec4( v, v, v, 0.25 );\n"
			"}\n";
		shader_t shaders[] =
		{
			{ vertex_shader_source, SHADER_TYPE_VERTEX },
			{ fragment_shader_source, SHADER_TYPE_FRAGMENT }
		};
		check_bg_shader_program = compile_shader( shaders, 2 );
		glUseProgram( check_bg_shader_program );

		float vertices[] = {
			-1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 40.0f,
			1.0f, -1.0f, 1.0f, 1.0f, 64.0f, 40.0f,
			1.0f, 1.0f, 1.0f, 1.0f, 64.0f, 0.0f,
			-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f
		};
		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0
		};
		glGenVertexArrays( 1, &check_bg_vao );
		glBindVertexArray( check_bg_vao );
		GLuint vbo, ebo;
		glGenBuffers( 1, &vbo );
		glGenBuffers( 1, &ebo );
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, 0 );
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, ( void * )( sizeof( float ) * 4 ) );
		glEnableVertexAttribArray( 1 );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );

		uint8_t pattern[ 8 * 8 ] =
		{
			192, 192, 192, 192, 64, 64, 64, 64,
			192, 192, 192, 192, 64, 64, 64, 64,
			192, 192, 192, 192, 64, 64, 64, 64,
			192, 192, 192, 192, 64, 64, 64, 64,
			64, 64, 64, 64, 192, 192, 192, 192,
			64, 64, 64, 64, 192, 192, 192, 192,
			64, 64, 64, 64, 192, 192, 192, 192,
			64, 64, 64, 64, 192, 192, 192, 192
		};
		glGenTextures( 1, &check_bg_texture );
		glActiveTexture( GL_TEXTURE1 );
		glBindTexture( GL_TEXTURE_2D, check_bg_texture );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, 8, 8, 0, GL_RED, GL_UNSIGNED_BYTE, pattern );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glUniform1i( glGetUniformLocation( check_bg_shader_program, "u_texture" ), 1 );
	}

	GLuint gfx_shader_program;
	GLuint gfx_vao;
	GLuint gfx_texture;
	{
		const char * vertex_shader_source =
			"#version 330 core\n"
			"layout (location = 0) in vec4 i_position;\n"
			"layout (location = 1) in vec2 i_texcoord;\n"
			"out vec2 v_texcoord;\n"
			"uniform mat4 u_model;\n"
			"void main()\n"
			"{\n"
			"   gl_Position = u_model * vec4( i_position.xy, 0.0, 1.0 );\n"
			"   v_texcoord = i_texcoord;\n"
			"}\n";
		const char * fragment_shader_source =
			"#version 330 core\n"
			"out vec4 f_color;\n"
			"in vec2 v_texcoord;\n"
			"uniform sampler2D u_texture;\n"
			"void main()\n"
			"{\n"
			"	float v = texture( u_texture, v_texcoord ).r;\n"
			"	if ( v <= 0.0f ) discard;\n"
			"	float v2 = v * 32.0 - ( 1.0 / 32.0 );\n"
			"   f_color = vec4( v2, v2, v2, 1.0 );\n"
			"	//f_color = vec4( 1.0, 0.0, 0.0, 1.0 );\n"
			"}\n";
		shader_t shaders[] =
		{
			{ vertex_shader_source, SHADER_TYPE_VERTEX },
			{ fragment_shader_source, SHADER_TYPE_FRAGMENT }
		};
		gfx_shader_program = compile_shader( shaders, 2 );
		glUseProgram( gfx_shader_program );

		float vertices[] = {
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f
		};
		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0
		};
		glGenVertexArrays( 1, &gfx_vao );
		glBindVertexArray( gfx_vao );
		GLuint vbo, ebo;
		glGenBuffers( 1, &vbo );
		glGenBuffers( 1, &ebo );
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, 0 );
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 6, ( void * )( sizeof( float ) * 4 ) );
		glEnableVertexAttribArray( 1 );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );

		glGenTextures( 1, &gfx_texture );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, gfx_texture );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, pixels );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glUniform1i( glGetUniformLocation( gfx_shader_program, "u_texture" ), 0 );

		float model[ 16 ];
		mat4_identity( model );
		mat4_scale( model, ( 1.0f / ( float )( CANVAS_WIDTH ) ) * 512.0f, ( 1.0f / ( float )( CANVAS_HEIGHT ) ) * 512.0f, 0.0f );
		mat4_translate( model, -1.0f + ( 512.0f / ( float )( CANVAS_WIDTH ) ), ( -1.0f + ( 512.0f / ( float )( CANVAS_HEIGHT ) ) ) * -1.0, 0.0f );
		glUniformMatrix4fv( glGetUniformLocation( gfx_shader_program, "u_model" ), 1, GL_FALSE, model );
	}

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
			}
		}

		// Clear the screen.
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		// Draw background.
		glUseProgram( bg_shader_program );
		glBindVertexArray( bg_vao );
		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );

		// Draw checkered background.
		glUseProgram( check_bg_shader_program );
		glActiveTexture( GL_TEXTURE1 );
		glBindVertexArray( check_bg_vao );
		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );

		// Draw graphics.
		glUseProgram( gfx_shader_program );
		glActiveTexture( GL_TEXTURE0 );
		glBindVertexArray( gfx_vao );
		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );

		SDL_GL_SwapWindow( window );
	}

	return 0;
}

static GLuint compile_shader( const shader_t * shaders, size_t count )
{
	GLuint program = glCreateProgram();
	GLuint * shader_ids = malloc( sizeof( GLuint ) * count );
	char info_log[ 512 ];

	for ( size_t i = 0; i < count; i++ )
	{
		GLenum shader_type = shaders[i].type == SHADER_TYPE_FRAGMENT
			? GL_FRAGMENT_SHADER
			: GL_VERTEX_SHADER;
		GLuint shader = glCreateShader( shader_type );

		glShaderSource( shader, 1, &shaders[i].src, NULL );
		glCompileShader( shader );

		GLint success;
		glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
		if ( ! success )
		{
			glGetShaderInfoLog( shader, 512, NULL, info_log );
			fprintf( stderr, "Shader compilation failed: %s\n", info_log );
			glDeleteShader( shader );
			continue;
		}

		glAttachShader( program, shader );
		shader_ids[i] = shader;
	}

	glLinkProgram( program );

	GLint success;
	glGetProgramiv( program, GL_LINK_STATUS, &success );
	if ( ! success )
	{
		glGetProgramInfoLog( program, 512, NULL, info_log );
		fprintf( stderr, "Program linking failed: %s\n", info_log );
		glDeleteProgram( program );
		program = 0;
	}

	for ( size_t i = 0; i < count; i++ )
	{
		glDetachShader( program, shader_ids[i] );
		glDeleteShader( shader_ids[i] );
	}
	free( shader_ids );

	return program;
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
