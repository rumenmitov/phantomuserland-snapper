/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2011 Dmitry Zavalishin, dz@dz.ru
 *
 * JSON parser interface.
 *
 **/


#include <kernel/json.h>
// #include <jsmn.h>
#include <errno.h>
#include <ph_malloc.h>
#include <ph_string.h>
#include <phantom_assert.h>

/*

//! tokens allocated, must be freed by caller
errno_t json_parse(const char *js, jsmntok_t **tokens, size_t *o_count )
{
	assert(js);

	int jslen = ph_strlen(js);

	return json_parse_len( js, jslen, tokens, o_count );
}


//! tokens allocated, must be freed by caller
errno_t json_parse_len(const char *js, int jslen, jsmntok_t **tokens, size_t *o_count )
{
	jsmn_parser parser;

	assert(tokens);
	assert(js);
	assert( o_count );

	jsmn_init( &parser );

	int count = jsmn_parse( &parser, js, jslen, 0, 0 );
	if( count < 0 )
		return EINVAL;

	*tokens = ph_calloc( count, sizeof(jsmntok_t) );
	if( !*tokens )
		return ENOMEM;

	jsmn_init( &parser );


	if( count != jsmn_parse( &parser, js, jslen, *tokens, count ) )
	{
		ph_free( *tokens );
		*tokens = 0;
		return EINVAL;
	}

	*o_count = count;

	return 0;
}

*/
