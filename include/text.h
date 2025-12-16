#ifndef TEXT_H
#define TEXT_H

typedef struct decoded_text_data_t
{
	unsigned char * text;
	unsigned char * remaining_data;
	size_t remaining_data_size;
} decoded_text_data_t;

decoded_text_data_t decode_text( unsigned char * data );
void destroy_text_system();

#endif // TEXT_H
