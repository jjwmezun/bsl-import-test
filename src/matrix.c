#include <math.h>
#include "matrix.h"
#include <stdio.h>
#include <string.h>

static void mat2_multiply( float * a, float * b );
static void mat3_multiply( float * a, float * b );
static void mat4_multiply( float * a, float * b );

static float identity_mat2[ 6 ] =
{
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f
};

static float identity_mat3[ 9 ] =
{
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f
};

static float identity_mat4[ 16 ] =
{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

void mat2_identity( float * matrix )
{
	memcpy( matrix, identity_mat2, sizeof( float ) * 6 );
}

void mat2_rotate_x( float * matrix, float angle )
{
	float c = cosf( angle );
	float rotate_matrix[ 6 ] =
	{
		c, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};
	mat2_multiply( matrix, rotate_matrix );
}

void mat2_rotate_y( float * matrix, float angle )
{
	float c = cosf( angle );
	float rotate_matrix[ 6 ] =
	{
		1.0f, 0.0f, 0.0f,
		0.0f, c, 0.0f
	};
	mat2_multiply( matrix, rotate_matrix );
}

void mat2_rotate_z( float * matrix, float angle )
{
	float c = cosf( angle );
	float s = sinf( angle );
	float rotate_matrix[ 6 ] =
	{
		c, -s, 0.0f,
		s, c, 0.0f
	};
	mat2_multiply( matrix, rotate_matrix );
}

void mat2_scale( float * matrix, float x, float y )
{
	float scale_matrix[ 6 ] =
	{
		x, 0.0f, 0.0f,
		0.0f, y, 0.0f
	};
	mat2_multiply( matrix, scale_matrix );
}

void mat2_translate( float * matrix, float x, float y )
{
	float translate_matrix[ 6 ] =
	{
		1.0f, 0.0f, x,
		0.0f, 1.0f, y
	};
	mat2_multiply( matrix, translate_matrix );
}

void mat3_identity( float * matrix )
{
	memcpy( matrix, identity_mat3, sizeof( float ) * 9 );
}

void mat3_rotate_x( float * matrix, float angle )
{
	float c = cosf( angle );
	float s = sinf( angle );
	float rotate_matrix[ 9 ] =
	{
		1.0f, 0.0f, 0.0f,
		0.0f, c, -s,
		0.0f, s, c
	};
	mat3_multiply( matrix, rotate_matrix );
}

void mat3_rotate_y( float * matrix, float angle )
{
	float c = cosf( angle );
	float s = sinf( angle );
	float rotate_matrix[ 9 ] =
	{
		c, 0.0f, s,
		0.0f, 1.0f, 0.0f,
		-s, 0.0f, c
	};
	mat3_multiply( matrix, rotate_matrix );
}

void mat3_rotate_z( float * matrix, float angle )
{
	float c = cosf( angle );
	float s = sinf( angle );
	float rotate_matrix[ 9 ] =
	{
		c, -s, 0.0f,
		s, c, 0.0f,
		0.0f, 0.0f, 1.0f
	};
	mat3_multiply( matrix, rotate_matrix );
}

void mat3_scale( float * matrix, float x, float y )
{
	float scale_matrix[ 9 ] =
	{
		x, 0.0f, 0.0f,
		0.0f, y, 0.0f,
		0.0f, 0.0f, 1.0f
	};
	mat3_multiply( matrix, scale_matrix );
}

void mat3_translate( float * matrix, float x, float y )
{
	float translate_matrix[ 9 ] =
	{
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		x, y, 1.0f
	};
	mat3_multiply( matrix, translate_matrix );
}

void mat4_identity( float * matrix )
{
	memcpy( matrix, identity_mat4, sizeof( float ) * 16 );
}

void mat4_rotate_x( float * matrix, float angle )
{
	float c = cosf( angle );
	float s = sinf( angle );
	float rotate_matrix[ 16 ] =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, c, -s, 0.0f,
		0.0f, s, c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	mat4_multiply( matrix, rotate_matrix );
}

void mat4_rotate_y( float * matrix, float angle )
{
	float c = cosf( angle );
	float s = sinf( angle );
	float rotate_matrix[ 16 ] =
	{
		c, 0.0f, s, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-s, 0.0f, c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	mat4_multiply( matrix, rotate_matrix );
}

void mat4_rotate_z( float * matrix, float angle )
{
	float c = cosf( angle );
	float s = sinf( angle );
	float rotate_matrix[ 16 ] =
	{
		c, -s, 0.0f, 0.0f,
		s, c, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	mat4_multiply( matrix, rotate_matrix );
}

void mat4_scale( float * matrix, float x, float y, float z )
{
	float scale_matrix[ 16 ] =
	{
		x, 0.0f, 0.0f, 0.0f,
		0.0f, y, 0.0f, 0.0f,
		0.0f, 0.0f, z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	mat4_multiply( matrix, scale_matrix );
}

void mat4_translate( float * matrix, float x, float y, float z )
{
	float translate_matrix[ 16 ] =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		x, y, z, 1.0f
	};
	mat4_multiply( matrix, translate_matrix );
}

void mat4_debug_print( float * matrix )
{
	for ( int row = 0; row < 4; row++ )
	{
		for ( int col = 0; col < 4; col++ )
		{
			printf( "%f ", matrix[ col + row * 4 ] );
		}
		printf( "\n" );
	}
}

static void mat2_multiply( float * a, float * b )
{
	float result[ 6 ] =
	{
		a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 3 ],
		a[ 0 ] * b[ 1 ] + a[ 1 ] * b[ 4 ],
		a[ 0 ] * b[ 2 ] + a[ 1 ] * b[ 5 ] + a[ 2 ],

		a[ 3 ] * b[ 0 ] + a[ 4 ] * b[ 3 ],
		a[ 3 ] * b[ 1 ] + a[ 4 ] * b[ 4 ],
		a[ 3 ] * b[ 2 ] + a[ 4 ] * b[ 5 ] + a[ 5 ]
	};
	memcpy( a, result, sizeof( float ) * 6 );
}

static void mat3_multiply( float * a, float * b )
{
	float result[ 9 ] =
	{
		a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 3 ] + a[ 2 ] * b[ 6 ],
		a[ 0 ] * b[ 1 ] + a[ 1 ] * b[ 4 ] + a[ 2 ] * b[ 7 ],
		a[ 0 ] * b[ 2 ] + a[ 1 ] * b[ 5 ] + a[ 2 ] * b[ 8 ],

		a[ 3 ] * b[ 0 ] + a[ 4 ] * b[ 3 ] + a[ 5 ] * b[ 6 ],
		a[ 3 ] * b[ 1 ] + a[ 4 ] * b[ 4 ] + a[ 5 ] * b[ 7 ],
		a[ 3 ] * b[ 2 ] + a[ 4 ] * b[ 5 ] + a[ 5 ] * b[ 8 ],

		a[ 6 ] * b[ 0 ] + a[ 7 ] * b[ 3 ] + a[ 8 ] * b[ 6 ],
		a[ 6 ] * b[ 1 ] + a[ 7 ] * b[ 4 ] + a[ 8 ] * b[ 7 ],
		a[ 6 ] * b[ 2 ] + a[ 7 ] * b[ 5 ] + a[ 8 ] * b[ 8 ]
	};
	memcpy( a, result, sizeof( float ) * 9 );
}

static void mat4_multiply( float * a, float * b )
{
	float result[ 16 ] =
	{
		a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 4 ] + a[ 2 ] * b[ 8 ] + a[ 3 ] * b[ 12 ],
		a[ 0 ] * b[ 1 ] + a[ 1 ] * b[ 5 ] + a[ 2 ] * b[ 9 ] + a[ 3 ] * b[ 13 ],
		a[ 0 ] * b[ 2 ] + a[ 1 ] * b[ 6 ] + a[ 2 ] * b[ 10 ] + a[ 3 ] * b[ 14 ],
		a[ 0 ] * b[ 3 ] + a[ 1 ] * b[ 7 ] + a[ 2 ] * b[ 11 ] + a[ 3 ] * b[ 15 ],

		a[ 4 ] * b[ 0 ] + a[ 5 ] * b[ 4 ] + a[ 6 ] * b[ 8 ] + a[ 7 ] * b[ 12 ],
		a[ 4 ] * b[ 1 ] + a[ 5 ] * b[ 5 ] + a[ 6 ] * b[ 9 ] + a[ 7 ] * b[ 13 ],
		a[ 4 ] * b[ 2 ] + a[ 5 ] * b[ 6 ] + a[ 6 ] * b[ 10 ] + a[ 7 ] * b[ 14 ],
		a[ 4 ] * b[ 3 ] + a[ 5 ] * b[ 7 ] + a[ 6 ] * b[ 11 ] + a[ 7 ] * b[ 15 ],

		a[ 8 ] * b[ 0 ] + a[ 9 ] * b[ 4 ] + a[ 10 ] * b[ 8 ] + a[ 11 ] * b[ 12 ],
		a[ 8 ] * b[ 1 ] + a[ 9 ] * b[ 5 ] + a[ 10 ] * b[ 9 ] + a[ 11 ] * b[ 13 ],
		a[ 8 ] * b[ 2 ] + a[ 9 ] * b[ 6 ] + a[ 10 ] * b[ 10 ] + a[ 11 ] * b[ 14 ],
		a[ 8 ] * b[ 3 ] + a[ 9 ] * b[ 7 ] + a[ 10 ] * b[ 11 ] + a[ 11 ] * b[ 15	 ],

		a[ 12 ] * b[ 0 ] + a[ 13 ] * b[ 4 ] + a[ 14 ] * b[ 8 ] + a[ 15 ] * b[ 12 ],
		a[ 12 ] * b[ 1 ] + a[ 13 ] * b[ 5 ] + a[ 14 ] * b[ 9 ] + a[ 15 ] * b[ 13 ],
		a[ 12 ] * b[ 2 ] + a[ 13 ] * b[ 6 ] + a[ 14 ] * b[ 10 ] + a[ 15 ] * b[ 14 ],
		a[ 12 ] * b[ 3 ] + a[ 13 ] * b[ 7 ] + a[ 14 ] * b[ 11 ] + a[ 15 ] * b[ 15 ]
	};
	memcpy( a, result, sizeof( float ) * 16 );
}
