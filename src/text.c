#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "text.h"

typedef struct text_trie_t
{
	uint32_t c;
	uint16_t freq;
	uint16_t code;
	uint8_t child1;
	uint8_t child2;
} text_trie_t;

static text_trie_t * trie_list = 0;

static void generate_text_trie();
static void write_data_test( data_t data );

static char * unicode_to_utf8( uint32_t c ) {
	if ( c < 0x80 ) {
		unsigned char * result = malloc( 2 );
		result[ 0 ] = c;
		result[ 1 ] = '\0';
		return result;
	} else if ( c < 0x800 ) {
		unsigned char * result = malloc( 3 );
		result[ 0 ] = 0xC0 | ( c >> 6 );
		result[ 1 ] = 0x80 | ( c & 0x3F );
		result[ 2 ] = '\0';
		return result;
	} else if ( c < 0x10000 ) {
		unsigned char * result = malloc( 4 );
		result[ 0 ] = 0xE0 | ( c >> 12 );
		result[ 1 ] = 0x80 | ((c >> 6) & 0x3F);
		result[ 2 ] = 0x80 | (c & 0x3F);
		result[ 3 ] = '\0';
		return result;
	} else {
		unsigned char * result = malloc( 5 );
		result[ 0 ] = 0xF0 | (c >> 18);
		result[ 1 ] = 0x80 | ((c >> 12) & 0x3F);
		result[ 2 ] = 0x80 | ((c >> 6) & 0x3F);
		result[ 3 ] = 0x80 | (c & 0x3F);
		result[ 4 ] = '\0';
		return result;
	}
}

decoded_text_data_t decode_text( unsigned char * data )
{
	if ( ! trie_list ) {
		generate_text_trie();
	}

	unsigned char * original_data = data;
	text_trie_t * current = trie_list;
	unsigned char result[ 16 * 4 ];
	memset( result, 0, sizeof( result ) );
	size_t result_index = 0;
	for ( ; ; ) {
		uint_fast8_t bits[ 8 ] =
		{
			( data[ 0 ] >> 7 ) & 0x01,
			( data[ 0 ] >> 6 ) & 0x01,
			( data[ 0 ] >> 5 ) & 0x01,
			( data[ 0 ] >> 4 ) & 0x01,
			( data[ 0 ] >> 3 ) & 0x01,
			( data[ 0 ] >> 2 ) & 0x01,
			( data[ 0 ] >> 1 ) & 0x01,
			data[ 0 ] & 0x01
		};

		for ( size_t i = 0; i < 8; i++ ) {
			if ( current->c != 0 ) {
				if ( current->c == 0xFFFFFFFF ) {
					size_t len = strlen( result );
					char * final_result = malloc( len + 1 );
					if ( ! final_result ) {
						printf( "Memory allocation failed!\n" );
						exit( 1 );
					}
					memcpy( final_result, result, len );
					final_result[ len ] = '\0';
					decoded_text_data_t decoded_data = {
						.text = final_result,
						.remaining_data = data + 1,
						.remaining_data_size = original_data - ( data + 1 )
					};
					return decoded_data;
				}
				unsigned char * letter = unicode_to_utf8( current->c );
				while ( *letter ) {
					result[ result_index++ ] = *letter;
					letter++;
				}
				current = trie_list;
			}

			if ( bits[ i ] == 0 ) {
				if ( current->child1 == 0 ) {
					printf( "Invalid data encountered at index %zu\n", i );
					break;
				}
				current = &trie_list[ current->child1 ];
			} else {
				if ( current->child2 == 0 ) {
					printf( "Invalid data encountered at index %zu\n", i );
					break;
				}
				current = &trie_list[ current->child2 ];
			}
		}

		data++;
	}
}

static void generate_text_trie()
{
	data_t trie_data = load_data( "assets/trie.bin" );
	size_t count = trie_data.size / 10;
	trie_list = malloc( count * sizeof( text_trie_t ) );
	size_t index = 0;
	while ( trie_data.size > 0 ) {
		trie_list[ index ].c = trie_data.data[ 3 ] | ( trie_data.data[ 2 ] << 8 ) | ( trie_data.data[ 1 ] << 16 ) | ( trie_data.data[ 0 ] << 24 );
		trie_list[ index ].freq = trie_data.data[ 5 ] | ( trie_data.data[ 4 ] << 8 );
		trie_list[ index ].code = trie_data.data[ 7 ] | ( trie_data.data[ 6 ] << 8 );
		trie_list[ index ].child1 = trie_data.data[ 8 ];
		trie_list[ index ].child2 = trie_data.data[ 9 ];
		trie_data.data += 10;
		trie_data.size -= 10;
		index++;
	}
};

static void write_data_test( data_t data )
{
	FILE * f = fopen( "trietest.txt", "w" );
	if ( ! f )
	{
		printf( "Error opening file!\n" );
		exit( 1 );
	}
	for ( size_t i = 0; i < data.size; i++ ) {
		fprintf( f, "%02hhX ", data.data[ i ] );
	}
	fprintf( f, "\n" );
	fclose( f );
}