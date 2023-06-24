/***************************************************************************/
/*                                                                         */
/*  cidload.c                                                              */
/*                                                                         */
/*    CID-keyed Type1 font loader (body).                                  */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_INTERNAL_DEBUG_H
#include FT2_1_3_CONFIG_CONFIG_H
#include FT2_1_3_MULTIPLE_MASTERS_H
#include FT2_1_3_INTERNAL_TYPE1_TYPES_H

#include "cidload.h"

#include "ciderrs.h"


/*************************************************************************/
/*                                                                       */
/* The macro FT2_1_3_COMPONENT is used in trace mode.  It is an implicit      */
/* parameter of the FT2_1_3_TRACE() and FT2_1_3_ERROR() macros, used to print/log  */
/* messages during execution.                                            */
/*                                                                       */
#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_cidload


/* read a single offset */
FT2_1_3_LOCAL_DEF( FT2_1_3_Long )
cid_get_offset( FT2_1_3_Byte**  start,
                FT2_1_3_Byte    offsize ) {
	FT2_1_3_Long   result;
	FT2_1_3_Byte*  p = *start;


	for ( result = 0; offsize > 0; offsize-- ) {
		result <<= 8;
		result  |= *p++;
	}

	*start = p;
	return result;
}


FT2_1_3_LOCAL_DEF( void )
cid_decrypt( FT2_1_3_Byte*   buffer,
             FT2_1_3_Offset  length,
             FT2_1_3_UShort  seed ) {
	while ( length > 0 ) {
		FT2_1_3_Byte  plain;


		plain     = (FT2_1_3_Byte)( *buffer ^ ( seed >> 8 ) );
		seed      = (FT2_1_3_UShort)( ( *buffer + seed ) * 52845U + 22719 );
		*buffer++ = plain;
		length--;
	}
}


/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                    TYPE 1 SYMBOL PARSING                      *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/


static FT2_1_3_Error
cid_load_keyword( CID_Face        face,
                  CID_Loader*     loader,
                  const T1_Field  keyword ) {
	FT2_1_3_Error      error;
	CID_Parser*   parser = &loader->parser;
	FT2_1_3_Byte*      object;
	void*         dummy_object;
	CID_FaceInfo  cid = &face->cid;


	/* if the keyword has a dedicated callback, call it */
	if ( keyword->type == T1_FIELD_TYPE_CALLBACK ) {
		keyword->reader( (FT2_1_3_Face)face, parser );
		error = parser->root.error;
		goto Exit;
	}

	/* we must now compute the address of our target object */
	switch ( keyword->location ) {
	case T1_FIELD_LOCATION_CID_INFO:
		object = (FT2_1_3_Byte*)cid;
		break;

	case T1_FIELD_LOCATION_FONT_INFO:
		object = (FT2_1_3_Byte*)&cid->font_info;
		break;

	default: {
		CID_FaceDict  dict;


		if ( parser->num_dict < 0 ) {
			FT2_1_3_ERROR(( "cid_load_keyword: invalid use of `%s'!\n",
			           keyword->ident ));
			error = CID_Err_Syntax_Error;
			goto Exit;
		}

		dict = cid->font_dicts + parser->num_dict;
		switch ( keyword->location ) {
		case T1_FIELD_LOCATION_PRIVATE:
			object = (FT2_1_3_Byte*)&dict->private_dict;
			break;

		default:
			object = (FT2_1_3_Byte*)dict;
		}
	}
	}

	dummy_object = object;

	/* now, load the keyword data in the object's field(s) */
	if ( keyword->type == T1_FIELD_TYPE_INTEGER_ARRAY ||
	        keyword->type == T1_FIELD_TYPE_FIXED_ARRAY   )
		error = cid_parser_load_field_table( &loader->parser, keyword,
		                                     &dummy_object );
	else
		error = cid_parser_load_field( &loader->parser, keyword, &dummy_object );
Exit:
	return error;
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_Error )
parse_font_bbox( CID_Face     face,
                 CID_Parser*  parser ) {
	FT2_1_3_Fixed  temp[4];
	FT2_1_3_BBox*  bbox = &face->cid.font_bbox;


	(void)cid_parser_to_fixed_array( parser, 4, temp, 0 );
	bbox->xMin = FT2_1_3_RoundFix( temp[0] );
	bbox->yMin = FT2_1_3_RoundFix( temp[1] );
	bbox->xMax = FT2_1_3_RoundFix( temp[2] );
	bbox->yMax = FT2_1_3_RoundFix( temp[3] );

	return CID_Err_Ok;       /* this is a callback function; */
	/* we must return an error code */
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_Error )
parse_font_matrix( CID_Face     face,
                   CID_Parser*  parser ) {
	FT2_1_3_Matrix*    matrix;
	FT2_1_3_Vector*    offset;
	CID_FaceDict  dict;
	FT2_1_3_Face       root = (FT2_1_3_Face)&face->root;
	FT2_1_3_Fixed      temp[6];
	FT2_1_3_Fixed      temp_scale;


	if ( parser->num_dict >= 0 ) {
		dict   = face->cid.font_dicts + parser->num_dict;
		matrix = &dict->font_matrix;
		offset = &dict->font_offset;

		(void)cid_parser_to_fixed_array( parser, 6, temp, 3 );

		temp_scale = ABS( temp[3] );

		/* Set Units per EM based on FontMatrix values.  We set the value to */
		/* `1000/temp_scale', because temp_scale was already multiplied by   */
		/* 1000 (in t1_tofixed(), from psobjs.c).                            */
		root->units_per_EM = (FT2_1_3_UShort)( FT2_1_3_DivFix( 0x10000L,
		                                  FT2_1_3_DivFix( temp_scale, 1000 ) ) );

		/* we need to scale the values by 1.0/temp[3] */
		if ( temp_scale != 0x10000L ) {
			temp[0] = FT2_1_3_DivFix( temp[0], temp_scale );
			temp[1] = FT2_1_3_DivFix( temp[1], temp_scale );
			temp[2] = FT2_1_3_DivFix( temp[2], temp_scale );
			temp[4] = FT2_1_3_DivFix( temp[4], temp_scale );
			temp[5] = FT2_1_3_DivFix( temp[5], temp_scale );
			temp[3] = 0x10000L;
		}

		matrix->xx = temp[0];
		matrix->yx = temp[1];
		matrix->xy = temp[2];
		matrix->yy = temp[3];

		/* note that the font offsets are expressed in integer font units */
		offset->x  = temp[4] >> 16;
		offset->y  = temp[5] >> 16;
	}

	return CID_Err_Ok;       /* this is a callback function; */
	/* we must return an error code */
}


FT2_1_3_CALLBACK_DEF( FT2_1_3_Error )
parse_fd_array( CID_Face     face,
                CID_Parser*  parser ) {
	CID_FaceInfo  cid    = &face->cid;
	FT2_1_3_Memory     memory = face->root.memory;
	FT2_1_3_Error      error  = CID_Err_Ok;
	FT2_1_3_Long       num_dicts;


	num_dicts = cid_parser_to_int( parser );

	if ( !cid->font_dicts ) {
		FT2_1_3_Int  n;


		if ( FT2_1_3_NEW_ARRAY( cid->font_dicts, num_dicts ) )
			goto Exit;

		cid->num_dicts = (FT2_1_3_UInt)num_dicts;

		/* don't forget to set a few defaults */
		for ( n = 0; n < cid->num_dicts; n++ ) {
			CID_FaceDict  dict = cid->font_dicts + n;


			/* default value for lenIV */
			dict->private_dict.lenIV = 4;
		}
	}

Exit:
	return error;
}


static
const T1_FieldRec  cid_field_records[] = {

#include "cidtoken.h"

	T1_FIELD_CALLBACK( "FontBBox", parse_font_bbox )
	T1_FIELD_CALLBACK( "FDArray", parse_fd_array )
	T1_FIELD_CALLBACK( "FontMatrix", parse_font_matrix ) {
		0, T1_FIELD_LOCATION_CID_INFO, T1_FIELD_TYPE_NONE, 0, 0, 0, 0, 0
	}
};


static int
is_alpha( char  c ) {
	return ( ft_isalnum( (int)c ) ||
	         c == '.'             ||
	         c == '_'             );
}


static FT2_1_3_Error
cid_parse_dict( CID_Face     face,
                CID_Loader*  loader,
                FT2_1_3_Byte*     base,
                FT2_1_3_Long      size ) {
	CID_Parser*  parser = &loader->parser;


	parser->root.cursor = base;
	parser->root.limit  = base + size;
	parser->root.error  = 0;

	{
		FT2_1_3_Byte*  cur   = base;
		FT2_1_3_Byte*  limit = cur + size;


		for ( ; cur < limit; cur++ ) {
			/* look for `%ADOBeginFontDict' */
			if ( *cur == '%' && cur + 20 < limit &&
			        ft_strncmp( (char*)cur, "%ADOBeginFontDict", 17 ) == 0 ) {
				cur += 17;

				/* if /FDArray was found, then cid->num_dicts is > 0, and */
				/* we can start increasing parser->num_dict               */
				if ( face->cid.num_dicts > 0 )
					parser->num_dict++;
			}
			/* look for immediates */
			else if ( *cur == '/' && cur + 2 < limit ) {
				FT2_1_3_Byte*  cur2;
				FT2_1_3_Int    len;


				cur++;

				cur2 = cur;
				while ( cur2 < limit && is_alpha( *cur2 ) )
					cur2++;

				len = (FT2_1_3_Int)( cur2 - cur );
				if ( len > 0 && len < 22 ) {
					/* now compare the immediate name to the keyword table */
					T1_Field  keyword = (T1_Field) cid_field_records;


					for (;;) {
						FT2_1_3_Byte*  name;


						name = (FT2_1_3_Byte*)keyword->ident;
						if ( !name )
							break;

						if ( cur[0] == name[0]                          &&
						        len == (FT2_1_3_Int)ft_strlen( (const char*)name ) ) {
							FT2_1_3_Int  n;


							for ( n = 1; n < len; n++ )
								if ( cur[n] != name[n] )
									break;

							if ( n >= len ) {
								/* we found it - run the parsing callback */
								parser->root.cursor = cur2;
								cid_parser_skip_spaces( parser );
								parser->root.error = cid_load_keyword( face,
								                                       loader,
								                                       keyword );
								if ( parser->root.error )
									return parser->root.error;

								cur = parser->root.cursor;
								break;
							}
						}
						keyword++;
					}
				}
			}
		}
	}
	return parser->root.error;
}


/* read the subrmap and the subrs of each font dict */
static FT2_1_3_Error
cid_read_subrs( CID_Face  face ) {
	CID_FaceInfo  cid    = &face->cid;
	FT2_1_3_Memory     memory = face->root.memory;
	FT2_1_3_Stream     stream = face->root.stream;
	FT2_1_3_Error      error;
	FT2_1_3_Int        n;
	CID_Subrs     subr;
	FT2_1_3_UInt       max_offsets = 0;
	FT2_1_3_ULong*     offsets = 0;


	if ( FT2_1_3_NEW_ARRAY( face->subrs, cid->num_dicts ) )
		goto Exit;

	subr = face->subrs;
	for ( n = 0; n < cid->num_dicts; n++, subr++ ) {
		CID_FaceDict  dict  = cid->font_dicts + n;
		FT2_1_3_Int        lenIV = dict->private_dict.lenIV;
		FT2_1_3_UInt       count, num_subrs = dict->num_subrs;
		FT2_1_3_ULong      data_len;
		FT2_1_3_Byte*      p;


		/* reallocate offsets array if needed */
		if ( num_subrs + 1 > max_offsets ) {
			FT2_1_3_UInt  new_max = ( num_subrs + 1 + 3 ) & -4;


			if ( FT2_1_3_RENEW_ARRAY( offsets, max_offsets, new_max ) )
				goto Fail;

			max_offsets = new_max;
		}

		/* read the subrmap's offsets */
		if ( FT2_1_3_STREAM_SEEK( cid->data_offset + dict->subrmap_offset ) ||
		        FT2_1_3_FRAME_ENTER( ( num_subrs + 1 ) * dict->sd_bytes )   )
			goto Fail;

		p = (FT2_1_3_Byte*)stream->cursor;
		for ( count = 0; count <= num_subrs; count++ )
			offsets[count] = cid_get_offset( &p, (FT2_1_3_Byte)dict->sd_bytes );

		FT2_1_3_FRAME_EXIT();

		/* now, compute the size of subrs charstrings, */
		/* allocate, and read them                     */
		data_len = offsets[num_subrs] - offsets[0];

		if ( FT2_1_3_NEW_ARRAY( subr->code, num_subrs + 1 ) ||
		        FT2_1_3_ALLOC( subr->code[0], data_len )   )
			goto Fail;

		if ( FT2_1_3_STREAM_SEEK( cid->data_offset + offsets[0] ) ||
		        FT2_1_3_STREAM_READ( subr->code[0], data_len )  )
			goto Fail;

		/* set up pointers */
		for ( count = 1; count <= num_subrs; count++ ) {
			FT2_1_3_ULong  len;


			len               = offsets[count] - offsets[count - 1];
			subr->code[count] = subr->code[count - 1] + len;
		}

		/* decrypt subroutines, but only if lenIV >= 0 */
		if ( lenIV >= 0 ) {
			for ( count = 0; count < num_subrs; count++ ) {
				FT2_1_3_ULong  len;


				len = offsets[count + 1] - offsets[count];
				cid_decrypt( subr->code[count], len, 4330 );
			}
		}

		subr->num_subrs = num_subrs;
	}

Exit:
	FT2_1_3_FREE( offsets );
	return error;

Fail:
	if ( face->subrs ) {
		for ( n = 0; n < cid->num_dicts; n++ ) {
			if ( face->subrs[n].code )
				FT2_1_3_FREE( face->subrs[n].code[0] );

			FT2_1_3_FREE( face->subrs[n].code );
		}
		FT2_1_3_FREE( face->subrs );
	}
	goto Exit;
}


static void
t1_init_loader( CID_Loader*  loader,
                CID_Face     face ) {
	FT2_1_3_UNUSED( face );

	FT2_1_3_MEM_ZERO( loader, sizeof ( *loader ) );
}


static void
t1_done_loader( CID_Loader*  loader ) {
	CID_Parser*  parser = &loader->parser;


	/* finalize parser */
	cid_parser_done( parser );
}


FT2_1_3_LOCAL_DEF( FT2_1_3_Error )
cid_face_open( CID_Face  face ) {
	CID_Loader   loader;
	CID_Parser*  parser;
	FT2_1_3_Error     error;


	t1_init_loader( &loader, face );

	parser = &loader.parser;
	error = cid_parser_new( parser, face->root.stream, face->root.memory,
	                        (PSAux_Service)face->psaux );
	if ( error )
		goto Exit;

	error = cid_parse_dict( face, &loader,
	                        parser->postscript,
	                        parser->postscript_len );
	if ( error )
		goto Exit;

	face->cid.data_offset = loader.parser.data_offset;
	error = cid_read_subrs( face );

Exit:
	t1_done_loader( &loader );
	return error;
}


/* END */
