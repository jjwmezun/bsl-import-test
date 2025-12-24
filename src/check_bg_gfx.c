#include "check_bg_gfx.h"
#include "constants.h"
#include "gl.h"
#include <GL/glew.h>

static GLuint check_bg_shader_program;
static GLuint check_bg_vao;

void draw_checkered_bg_gfx()
{
    glUseProgram( check_bg_shader_program );
    glBindVertexArray( check_bg_vao );
    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
}

void init_checkered_bg_gfx()
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
        "   f_color = vec4( v, v, v, 1.0 );\n"
        "}\n";
    shader_t shaders[] =
    {
        { vertex_shader_source, SHADER_TYPE_VERTEX },
        { fragment_shader_source, SHADER_TYPE_FRAGMENT }
    };
    check_bg_shader_program = compile_shader( shaders, 2 );
    glUseProgram( check_bg_shader_program );

    float vertices[] = {
        -1.0f, -1.0f, 0.98f, 1.0f, 0.0f, 40.0f,
        1.0f, -1.0f, 0.98f, 1.0f, 64.0f, 40.0f,
        1.0f, 1.0f, 0.98f, 1.0f, 64.0f, 0.0f,
        -1.0f, 1.0f, 0.98f, 1.0f, 0.0f, 0.0f
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
        241, 241, 241, 241, 226, 226, 226, 226,
        241, 241, 241, 241, 226, 226, 226, 226,
        241, 241, 241, 241, 226, 226, 226, 226,
        241, 241, 241, 241, 226, 226, 226, 226,
        226, 226, 226, 226, 241, 241, 241, 241,
        226, 226, 226, 226, 241, 241, 241, 241,
        226, 226, 226, 226, 241, 241, 241, 241,
        226, 226, 226, 226, 241, 241, 241, 241
    };
    GLuint check_bg_texture;
    glGenTextures( 1, &check_bg_texture );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, check_bg_texture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, 8, 8, 0, GL_RED, GL_UNSIGNED_BYTE, pattern );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glUniform1i( glGetUniformLocation( check_bg_shader_program, "u_texture" ), 1 );
}
