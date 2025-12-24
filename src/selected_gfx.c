#include "constants.h"
#include "gl.h"
#include <GL/glew.h>
#include "matrix.h"
#include "selected_gfx.h"
#include <string.h>

static GLuint selected_gfx_shader_program;
static GLuint selected_gfx_vao;
static GLint selected_gfx_text_model_uniform_location;
static GLint selected_gfx_palette_index_uniform_location;

static GLuint selected_border_gfx_shader_program;
static GLuint selected_border_gfx_vao;

static void init_selected_border_gfx();

void draw_selected_gfx()
{
    // Draw border.
    glUseProgram( selected_border_gfx_shader_program );
    glBindVertexArray( selected_border_gfx_vao );
    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );

    // Draw GFX.
    glUseProgram( selected_gfx_shader_program );
    glBindVertexArray( selected_gfx_vao );
    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
}

void init_selected_gfx( palette_t palette, float selector_x, float selector_y )
{
    const char * vertex_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec4 i_position;\n"
        "layout (location = 1) in vec2 i_texcoord;\n"
        "out vec2 v_texcoord;\n"
        "uniform mat4 u_model;\n"
        "uniform mat3 u_text_model;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = u_model * vec4( i_position.xy, 0.0, 1.0 );\n"
        "   v_texcoord = ( u_text_model * vec3( i_texcoord, 1.0 ) ).xy;\n"
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
    selected_gfx_shader_program = compile_shader( shaders, 2 );
    glUseProgram( selected_gfx_shader_program );

    float vertices[] = {
        -1.0f, -1.0f, 0.96f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.96f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.96f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.96f, 0.0f, 0.0f, 0.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    glGenVertexArrays( 1, &selected_gfx_vao );
    glBindVertexArray( selected_gfx_vao );
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

    glUniform1i( glGetUniformLocation( selected_gfx_shader_program, "u_texture" ), 0 );
    glUniform1i( glGetUniformLocation( selected_gfx_shader_program, "u_palette_texture" ), 2 );
    selected_gfx_palette_index_uniform_location = glGetUniformLocation( selected_gfx_shader_program, "u_palette_index" );
    update_selected_gfx_palette( palette );

    float model[ 16 ];
    mat4_identity( model );
    mat4_scale( model, ( 1.0f / ( float )( CANVAS_WIDTH ) ) * 32.0f, ( 1.0f / ( float )( CANVAS_HEIGHT ) ) * 32.0f, 0.0f );
    mat4_translate( model, -0.90625, -0.85f, 0.0f );
    glUniformMatrix4fv( glGetUniformLocation( selected_gfx_shader_program, "u_model" ), 1, GL_FALSE, model );

    selected_gfx_text_model_uniform_location = glGetUniformLocation( selected_gfx_shader_program, "u_text_model" );
    update_selected_gfx( selector_x, selector_y );

    init_selected_border_gfx();
}

void update_selected_gfx( float selector_x, float selector_y )
{
    glUseProgram( selected_gfx_shader_program );
    float text_model[ 9 ];
    mat3_identity( text_model );
    mat3_scale( text_model, 0.015625f, 0.015625f );
    mat3_translate( text_model, 0.015625f * selector_x, 0.015625f * selector_y );
    glUniformMatrix3fv( selected_gfx_text_model_uniform_location, 1, GL_FALSE, text_model );
}

void update_selected_gfx_palette( palette_t palette )
{
    glUseProgram( selected_gfx_shader_program );
    glUniform1f( selected_gfx_palette_index_uniform_location, ( float )( palette.selected ) / ( float )( palette.len ) );
}

static void init_selected_border_gfx()
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
        "   vec4 color = texture( u_texture, v_texcoord );\n"
        "   if ( color.a < 0.1 ) discard;\n"
        "   f_color = color;\n"
        "}\n";
    shader_t shaders[] =
    {
        { vertex_shader_source, SHADER_TYPE_VERTEX },
        { fragment_shader_source, SHADER_TYPE_FRAGMENT }
    };
    selected_border_gfx_shader_program = compile_shader( shaders, 2 );
    glUseProgram( selected_border_gfx_shader_program );

    float vertices[] = {
        -1.0f, -1.0f, 0.95f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.95f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.95f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.95f, 0.0f, 0.0f, 0.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    glGenVertexArrays( 1, &selected_border_gfx_vao );
    glBindVertexArray( selected_border_gfx_vao );
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

    uint16_t pattern[ 18 * 18 ];
    memset( pattern, 0, sizeof( pattern ) );
    for ( int y = 0; y < 18; y++ )
    {
        pattern[ y * 18 + 0 ] = 0x0001;
        pattern[ y * 18 + 17 ] = 0x0001;
    }
    for ( int x = 0; x < 18; x++ )
    {
        pattern[ 0 * 18 + x ] = 0x0001;
        pattern[ 17 * 18 + x ] = 0x0001;
    }
    GLuint texture;
    glGenTextures( 1, &texture );
    glActiveTexture( GL_TEXTURE5 );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB5_A1, 18, 18, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, pattern );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glUniform1i( glGetUniformLocation( selected_border_gfx_shader_program, "u_texture" ), 5 );

    float model[ 16 ];
    mat4_identity( model );
    mat4_scale( model, ( 1.0f / ( float )( CANVAS_WIDTH ) ) * 36.0f, ( 1.0f / ( float )( CANVAS_HEIGHT ) ) * 36.0f, 0.0f );
    mat4_translate( model, -0.90625, -0.85f, 0.0f );
    glUniformMatrix4fv( glGetUniformLocation( selected_border_gfx_shader_program, "u_model" ), 1, GL_FALSE, model );
}
