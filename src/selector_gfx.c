#include "constants.h"
#include "gl.h"
#include <GL/glew.h>
#include "matrix.h"
#include "selector_gfx.h"
#include <string.h>

static GLuint selector_shader_program;
static GLuint selector_vao;
static GLint selector_model_uniform_location;

void draw_selector_gfx()
{
    glUseProgram( selector_shader_program );
    glBindVertexArray( selector_vao );
    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
}

void init_selector_gfx( float selector_x, float selector_y )
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
        "   if ( texture( u_texture, v_texcoord ).a < 0.1 ) discard;\n"
        "   f_color = texture( u_texture, v_texcoord );\n"
        "}\n";
    shader_t shaders[] =
    {
        { vertex_shader_source, SHADER_TYPE_VERTEX },
        { fragment_shader_source, SHADER_TYPE_FRAGMENT }
    };
    selector_shader_program = compile_shader( shaders, 2 );
    glUseProgram( selector_shader_program );

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
    glGenVertexArrays( 1, &selector_vao );
    glBindVertexArray( selector_vao );
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

    selector_model_uniform_location = glGetUniformLocation( selector_shader_program, "u_model" );
    update_selector_gfx( selector_x, selector_y );

    uint16_t pattern[ 10 * 10 ];
    memset( pattern, 0x0000, sizeof( pattern ) );
    for ( size_t x = 0; x < 10; x++ )
    {
        pattern[ x ] = 0xFC01;
        pattern[ x + ( 10 * 9 ) ] = 0xFC01;
    }
    for ( size_t y = 0; y < 10; y++ )
    {
        pattern[ y * 10 ] = 0xFC01;
        pattern[ y * 10 + 9 ] = 0xFC01;
    }
    GLuint texture;
    glGenTextures( 1, &texture );
    glActiveTexture( GL_TEXTURE4 );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB5_A1, 10, 10, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, pattern );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glUniform1i( glGetUniformLocation( selector_shader_program, "u_texture" ), 4 );
}

void update_selector_gfx( float selector_x, float selector_y )
{
    glUseProgram( selector_shader_program );
    float model[ 16 ];
    mat4_identity( model );
    mat4_scale( model, 0.01953125f, 0.03125f, 0.0f );
    mat4_translate( model, -0.984375f + ( 0.03125f * selector_x ), 0.975f - ( 0.05f * selector_y ), 0.0f );
    glUniformMatrix4fv( selector_model_uniform_location, 1, GL_FALSE, model );
}
