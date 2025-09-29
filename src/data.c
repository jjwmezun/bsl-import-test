#include <stdio.h>
#include <stdlib.h>

#include "data.h"

data_t load_data( const char *filename )
{
	data_t had = { 0, 0 };
	FILE *file = fopen( filename, "r" );
	if ( ! file ) {
		perror( "Failed to open file" );
		return had;
	}
	fseek( file, 0, SEEK_END );
	long size = ftell( file );
	if ( size < 0 ) {
		perror( "Failed to determine file size" );
		fclose( file );
		return had;
	}
	char * buffer = malloc( size );
	if ( ! buffer ) {
		perror( "Failed to allocate memory" );
		fclose( file );
		return had;
	}
	fseek( file, 0, SEEK_SET );
	size_t bytesread = fread( buffer, 1, size, file );
	if ( bytesread < size ) {
		perror( "Failed to read file" );
		free( buffer );
		fclose( file );
		return had;
	}
	fclose( file );
	had.data = buffer;
	had.size = size;
	return had;
}
