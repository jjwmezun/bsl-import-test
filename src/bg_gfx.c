#include "bg_gfx.h"
#include "constants.h"
#include "gl.h"
#include <GL/glew.h>

static GLuint bg_shader_program;
static GLuint bg_vao;

void init_bg_gfx()
{
    const char * vertex_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec2 i_position;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4( i_position, 0.99, 1.0 );\n"
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

void draw_bg_gfx()
{
	glUseProgram( bg_shader_program );
	glBindVertexArray( bg_vao );
	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
}
