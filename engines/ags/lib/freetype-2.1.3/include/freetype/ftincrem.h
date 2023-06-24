/***************************************************************************/
/*                                                                         */
/*  ftincrem.h                                                             */
/*                                                                         */
/*    FreeType incremental loading (specification).                        */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTINCREM_H__
#define __FTINCREM_H__

#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_FREETYPE_H


FT2_1_3_BEGIN_HEADER


/***************************************************************************
 *
 * @type:
 *   FT2_1_3_Incremental
 *
 * @description:
 *   An opaque type describing a user-provided object used to implement
 *   "incremental" glyph loading within FreeType.  This is used to support
 *   embedded fonts in certain environments (e.g. Postscript interpreters),
 *   where the glyph data isn't in the font file, or must be overridden by
 *   different values.
 *
 * @note:
 *   It is up to client applications to create and implement @FT2_1_3_Incremental
 *   objects, as long as they provide implementations for the methods
 *   @FT2_1_3_Incremental_GetGlyphDataFunc, @FT2_1_3_Incremental_FreeGlyphDataFunc
 *   and @FT2_1_3_Incremental_GetGlyphMetricsFunc.
 *
 *   See the description of @FT2_1_3_Incremental_InterfaceRec to understand how
 *   to use incremental objects with FreeType.
 */
typedef struct FT2_1_3_IncrementalRec_*  FT2_1_3_Incremental;


/***************************************************************************
 *
 * @struct:
 *   FT2_1_3_Incremental_Metrics
 *
 * @description:
 *   A small structure used to contain the basic glyph metrics returned
 *   by the @FT2_1_3_Incremental_GetGlyphMetricsFunc method.
 *
 * @fields:
 *   bearing_x ::
 *     Left bearing, in font units.
 *
 *   bearing_y ::
 *     Top bearing, in font units.
 *
 *   advance ::
 *     Glyph advance, in font units.
 *
 * @note:
 *   These correspond to horizontal or vertical metrics depending on the
 *   value of the 'vertical' argument to the function
 *   @FT2_1_3_Incremental_GetGlyphMetricsFunc.
 */
typedef struct  FT2_1_3_Incremental_MetricsRec_ {
	FT2_1_3_Long  bearing_x;
	FT2_1_3_Long  bearing_y;
	FT2_1_3_Long  advance;

} FT2_1_3_Incremental_MetricsRec, *FT2_1_3_Incremental_Metrics;


/***************************************************************************
 *
 * @type:
 *   FT2_1_3_Incremental_GetGlyphDataFunc
 *
 * @description:
 *   A function called by FreeType to access a given glyph's data bytes
 *   during @FT2_1_3_Load_Glyph or @FT2_1_3_Load_Char if incremental loading is
 *   enabled.
 *
 *   Note that the format of the glyph's data bytes depends on the font
 *   file format.  For TrueType, it must correspond to the raw bytes within
 *   the 'glyf' table.  For Postscript formats, it must correspond to the
 *   *unencrypted* charstring bytes, without any 'lenIV' header.  It is
 *   undefined for any other format.
 *
 * @input:
 *   incremental ::
 *     Handle to an opaque @FT2_1_3_Incremental handle provided by the client
 *     application.
 *
 *   glyph_index ::
 *     Index of relevant glyph.
 *
 * @output:
 *   adata ::
 *     A structure describing the returned glyph data bytes (which will be
 *     accessed as a read-only byte block).
 *
 * @return:
 *   FreeType error code.  0 means success.
 *
 * @note:
 *   If this function returns succesfully the method
 *   @FT2_1_3_Incremental_FreeGlyphDataFunc will be called later to release
 *   the data bytes.
 *
 *   Nested calls to @FT2_1_3_Incremental_GetGlyphDataFunc can happen for
 *   compound glyphs.
 */
typedef FT2_1_3_Error
(*FT2_1_3_Incremental_GetGlyphDataFunc)( FT2_1_3_Incremental  incremental,
                                    FT2_1_3_UInt         glyph_index,
                                    FT2_1_3_Data*        adata );


/***************************************************************************
 *
 * @type:
 *   FT2_1_3_Incremental_FreeGlyphDataFunc
 *
 * @description:
 *   A function used to release the glyph data bytes returned by a
 *   successful call to @FT2_1_3_Incremental_GetGlyphDataFunc.
 *
 * @input:
 *   incremental ::
 *     A handle to an opaque @FT2_1_3_Incremental handle provided by the client
 *     application.
 *
 *   data ::
 *     A structure describing the glyph data bytes (which will be accessed
 *     as a read-only byte block).
 */
typedef void
(*FT2_1_3_Incremental_FreeGlyphDataFunc)( FT2_1_3_Incremental  incremental,
                                     FT2_1_3_Data*        data );


/***************************************************************************
 *
 * @type:
 *   FT2_1_3_Incremental_GetGlyphMetricsFunc
 *
 * @description:
 *   A function used to retrieve the basic metrics of a given glyph index
 *   before accessing its data.  This is necessary because, in certain
 *   formats like TrueType, the metrics are stored in a different place from
 *   the glyph images proper.
 *
 * @input:
 *   incremental ::
 *     A handle to an opaque @FT2_1_3_Incremental handle provided by the client
 *     application.
 *
 *   glyph_index ::
 *     Index of relevant glyph.
 *
 *   vertical ::
 *     If true, return vertical metrics.
 *
 * @output:
 *   ametrics ::
 *     The glyph metrics in font units.
 *
 *   afound ::
 *     True if there are metrics at all.
 *
 */
typedef FT2_1_3_Error
(*FT2_1_3_Incremental_GetGlyphMetricsFunc)
( FT2_1_3_Incremental              incremental,
  FT2_1_3_UInt                     glyph_index,
  FT2_1_3_Bool                     vertical,
  FT2_1_3_Incremental_MetricsRec  *ametrics,
  FT2_1_3_Bool                    *afound );


/**************************************************************************
 *
 * @struct:
 *   FT2_1_3_Incremental_FuncsRec
 *
 * @description:
 *   A table of functions for accessing fonts that load data
 *   incrementally.  Used in @FT2_1_3_Incremental_Interface.
 *
 * @fields:
 *   get_glyph_data ::
 *     The function to get glyph data.  Must not be null.
 *
 *   free_glyph_data ::
 *     The function to release glyph data.  Must not be null.
 *
 *   get_glyph_metrics ::
 *     The function to get glyph metrics.  May be null if the font does
 *     not provide overriding glyph metrics.
 */
typedef struct  FT2_1_3_Incremental_FuncsRec_ {
	FT2_1_3_Incremental_GetGlyphDataFunc     get_glyph_data;
	FT2_1_3_Incremental_FreeGlyphDataFunc    free_glyph_data;
	FT2_1_3_Incremental_GetGlyphMetricsFunc  get_glyph_metrics;

} FT2_1_3_Incremental_FuncsRec;


/***************************************************************************
 *
 * @struct:
 *   FT2_1_3_Incremental_InterfaceRec
 *
 * @description:
 *   A structure to be used with @FT2_1_3_Open_Face to indicate that the user
 *   wants to support incremental glyph loading.  You should use it with
 *   @FT2_1_3_PARAM_TAG_INCREMENTAL as in the following example:
 *
 *     {
 *       FT2_1_3_Incremental_InterfaceRec  inc_int;
 *       FT2_1_3_Parameter                 parameter;
 *       FT2_1_3_Open_Args                 open_args;
 *
 *
 *       // set up incremental descriptor
 *       inc_int.funcs  = my_funcs;
 *       inc_int.object = my_object;
 *
 *       // set up optional parameter
 *       parameter.tag  = FT2_1_3_PARAM_TAG_INCREMENTAL;
 *       parameter.data = &inc_int;
 *
 *       // set up FT2_1_3_Open_Args structure
 *       open_args.flags      = (FT2_1_3_Open_Flags)( FT2_1_3_OPEN_PATHNAME |
 *                                               FT2_1_3_OPEN_PARAMS   );
 *       open_args.pathname   = my_font_pathname;
 *       open_args.num_params = 1;
 *       open_args.params     = &parameter; // we use one optional argument
 *
 *       // open the font
 *       error = FT2_1_3_Open_Face( library, &open_args, index, &face );
 *       ...
 *     }
 */
typedef struct  FT2_1_3_Incremental_InterfaceRec_ {
	const FT2_1_3_Incremental_FuncsRec*  funcs;
	FT2_1_3_Incremental                  object;

} FT2_1_3_Incremental_InterfaceRec;


/***************************************************************************
 *
 * @constant:
 *   FT2_1_3_PARAM_TAG_INCREMENTAL
 *
 * @description:
 *   A constant used as the tag of @FT2_1_3_Parameter structures to indicate
 *   an incremental loading object to be used by FreeType.
 *
 */
#define FT2_1_3_PARAM_TAG_INCREMENTAL  FT2_1_3_MAKE_TAG( 'i', 'n', 'c', 'r' )

/* */

FT2_1_3_END_HEADER

#endif /* __FTINCREM_H__ */


/* END */
