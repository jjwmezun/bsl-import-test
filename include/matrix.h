#ifndef mat2_H
#define mat2_H

void mat2_identity( float * matrix );
void mat2_rotate_x( float * matrix, float angle );
void mat2_rotate_y( float * matrix, float angle );
void mat2_rotate_z( float * matrix, float angle );
void mat2_scale( float * matrix, float x, float y );
void mat2_translate( float * matrix, float x, float y );

void mat3_identity( float * matrix );
void mat3_rotate_x( float * matrix, float angle );
void mat3_rotate_y( float * matrix, float angle );
void mat3_rotate_z( float * matrix, float angle );
void mat3_scale( float * matrix, float x, float y );
void mat3_translate( float * matrix, float x, float y );

void mat4_identity( float * matrix );
void mat4_rotate_x( float * matrix, float angle );
void mat4_rotate_y( float * matrix, float angle );
void mat4_rotate_z( float * matrix, float angle );
void mat4_scale( float * matrix, float x, float y, float z );
void mat4_translate( float * matrix, float x, float y, float z );
void mat4_debug_print( float * matrix );

#endif // mat2_H
