#ifndef DATA_H
#define DATA_H

typedef struct data_t
{
	unsigned char * data;
	size_t size;
} data_t;

data_t load_data( const char *filename );

#endif // DATA_H