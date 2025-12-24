#ifndef GL_H
#define GL_H

#include <GL/glew.h>

typedef struct shader_t
{
	const char * src;
	unsigned int type;
}
shader_t;

GLuint compile_shader( const shader_t * shaders, size_t count );

#endif // GL_H
