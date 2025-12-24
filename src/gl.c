#include "constants.h"
#include "gl.h"
#include <stdlib.h>
#include <stdio.h>

GLuint compile_shader( const shader_t * shaders, size_t count )
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
