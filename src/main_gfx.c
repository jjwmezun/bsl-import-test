#include "constants.h"
#include "gl.h"
#include <GL/glew.h>
#include "main_gfx.h"
#include "matrix.h"

static GLuint gfx_shader_program;
static GLuint gfx_vao;
static float gfx_palette = 0;
static GLint main_gfx_palette_index_uniform_location;

void draw_main_gfx()
{
    glUseProgram( gfx_shader_program );
    glBindVertexArray( gfx_vao );
    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
}

void init_main_gfx( const unsigned char * pixels, palette_t palette )
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
        "uniform sampler2D u_palette_texture;\n"
        "uniform float u_palette_index;\n"
        "void main()\n"
        "{\n"
        "	float index = texture( u_texture, v_texcoord ).r * 32.0;\n"
        "   if ( index < 0.1 ) discard;\n"
        "   f_color = texture( u_palette_texture, vec2( index, u_palette_index ) );\n"
        "}\n";
    shader_t shaders[] =
    {
        { vertex_shader_source, SHADER_TYPE_VERTEX },
        { fragment_shader_source, SHADER_TYPE_FRAGMENT }
    };
    gfx_shader_program = compile_shader( shaders, 2 );
    glUseProgram( gfx_shader_program );

    float vertices[] = {
        -1.0f, -1.0f, 0.97f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.97f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.97f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.97f, 0.0f, 0.0f, 0.0f
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

    // Create graphics texture.
    GLuint gfx_texture;
    glGenTextures( 1, &gfx_texture );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, gfx_texture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, pixels );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glUniform1i( glGetUniformLocation( gfx_shader_program, "u_texture" ), 0 );

    // Create palette texture.
    GLuint pal_texture;
    glGenTextures( 1, &pal_texture );
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, pal_texture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB5_A1, 8, palette.len, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, palette.palettes );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glUniform1i( glGetUniformLocation( gfx_shader_program, "u_palette_texture" ), 2 );
    main_gfx_palette_index_uniform_location = glGetUniformLocation( gfx_shader_program, "u_palette_index" );
    update_main_gfx_palette( palette );

    float model[ 16 ];
    mat4_identity( model );
    mat4_scale( model, ( 1.0f / ( float )( CANVAS_WIDTH ) ) * 512.0f, ( 1.0f / ( float )( CANVAS_HEIGHT ) ) * 512.0f, 0.0f );
    mat4_translate( model, -1.0f + ( 512.0f / ( float )( CANVAS_WIDTH ) ), ( -1.0f + ( 512.0f / ( float )( CANVAS_HEIGHT ) ) ) * -1.0, 0.0f );
    glUniformMatrix4fv( glGetUniformLocation( gfx_shader_program, "u_model" ), 1, GL_FALSE, model );
}

void update_main_gfx_palette( palette_t palette )
{
    glUseProgram( gfx_shader_program );
    glUniform1f( main_gfx_palette_index_uniform_location, ( float )( palette.selected ) / ( float )( palette.len ) );
}
