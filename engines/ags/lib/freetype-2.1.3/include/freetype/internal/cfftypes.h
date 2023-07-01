/***************************************************************************/
/*                                                                         */
/*  cfftypes.h                                                             */
/*                                                                         */
/*    Basic OpenType/CFF type definitions and interface (specification     */
/*    only).                                                               */
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


#ifndef __CFFTYPES_H__
#define __CFFTYPES_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"


FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    CFF_IndexRec                                                       */
/*                                                                       */
/* <Description>                                                         */
/*    A structure used to model a CFF Index table.                       */
/*                                                                       */
/* <Fields>                                                              */
/*    stream      :: The source input stream.                            */
/*                                                                       */
/*    count       :: The number of elements in the index.                */
/*                                                                       */
/*    off_size    :: The size in bytes of object offsets in index.       */
/*                                                                       */
/*    data_offset :: The position of first data byte in the index's      */
/*                   bytes.                                              */
/*                                                                       */
/*    offsets     :: A table of element offsets in the index.            */
/*                                                                       */
/*    bytes       :: If the index is loaded in memory, its bytes.        */
/*                                                                       */
typedef struct  CFF_IndexRec_ {
	FT2_1_3_Stream  stream;
	FT2_1_3_UInt    count;
	FT2_1_3_Byte    off_size;
	FT2_1_3_ULong   data_offset;

	FT2_1_3_ULong*  offsets;
	FT2_1_3_Byte*   bytes;

} CFF_IndexRec, *CFF_Index;


typedef struct  CFF_EncodingRec_ {
	FT2_1_3_UInt     format;
	FT2_1_3_ULong    offset;

	FT2_1_3_UInt     count;
	FT2_1_3_UShort   sids [256];  /* avoid dynamic allocations */
	FT2_1_3_UShort   codes[256];

} CFF_EncodingRec, *CFF_Encoding;


typedef struct  CFF_CharsetRec_ {

	FT2_1_3_UInt     format;
	FT2_1_3_ULong    offset;

	FT2_1_3_UShort*  sids;

} CFF_CharsetRec, *CFF_Charset;


typedef struct  CFF_FontRecDictRec_ {
	FT2_1_3_UInt    version;
	FT2_1_3_UInt    notice;
	FT2_1_3_UInt    copyright;
	FT2_1_3_UInt    full_name;
	FT2_1_3_UInt    family_name;
	FT2_1_3_UInt    weight;
	FT2_1_3_Bool    is_fixed_pitch;
	FT2_1_3_Fixed   italic_angle;
	FT2_1_3_Pos     underline_position;
	FT2_1_3_Pos     underline_thickness;
	FT2_1_3_Int     paint_type;
	FT2_1_3_Int     charstring_type;
	FT2_1_3_Matrix  font_matrix;
	FT2_1_3_UShort  units_per_em;
	FT2_1_3_Vector  font_offset;
	FT2_1_3_ULong   unique_id;
	FT2_1_3_BBox    font_bbox;
	FT2_1_3_Pos     stroke_width;
	FT2_1_3_ULong   charset_offset;
	FT2_1_3_ULong   encoding_offset;
	FT2_1_3_ULong   charstrings_offset;
	FT2_1_3_ULong   private_offset;
	FT2_1_3_ULong   private_size;
	FT2_1_3_Long    synthetic_base;
	FT2_1_3_UInt    embedded_postscript;
	FT2_1_3_UInt    base_font_name;
	FT2_1_3_UInt    postscript;

	/* these should only be used for the top-level font dictionary */
	FT2_1_3_UInt    cid_registry;
	FT2_1_3_UInt    cid_ordering;
	FT2_1_3_ULong   cid_supplement;

	FT2_1_3_Long    cid_font_version;
	FT2_1_3_Long    cid_font_revision;
	FT2_1_3_Long    cid_font_type;
	FT2_1_3_Long    cid_count;
	FT2_1_3_ULong   cid_uid_base;
	FT2_1_3_ULong   cid_fd_array_offset;
	FT2_1_3_ULong   cid_fd_select_offset;
	FT2_1_3_UInt    cid_font_name;

} CFF_FontRecDictRec, *CFF_FontRecDict;


typedef struct  CFF_PrivateRec_ {
	FT2_1_3_Byte   num_blue_values;
	FT2_1_3_Byte   num_other_blues;
	FT2_1_3_Byte   num_family_blues;
	FT2_1_3_Byte   num_family_other_blues;

	FT2_1_3_Pos    blue_values[14];
	FT2_1_3_Pos    other_blues[10];
	FT2_1_3_Pos    family_blues[14];
	FT2_1_3_Pos    family_other_blues[10];

	FT2_1_3_Fixed  blue_scale;
	FT2_1_3_Pos    blue_shift;
	FT2_1_3_Pos    blue_fuzz;
	FT2_1_3_Pos    standard_width;
	FT2_1_3_Pos    standard_height;

	FT2_1_3_Byte   num_snap_widths;
	FT2_1_3_Byte   num_snap_heights;
	FT2_1_3_Pos    snap_widths[13];
	FT2_1_3_Pos    snap_heights[13];
	FT2_1_3_Bool   force_bold;
	FT2_1_3_Fixed  force_bold_threshold;
	FT2_1_3_Int    lenIV;
	FT2_1_3_Int    language_group;
	FT2_1_3_Fixed  expansion_factor;
	FT2_1_3_Long   initial_random_seed;
	FT2_1_3_ULong  local_subrs_offset;
	FT2_1_3_Pos    default_width;
	FT2_1_3_Pos    nominal_width;

} CFF_PrivateRec, *CFF_Private;


typedef struct  CFF_FDSelectRec_ {
	FT2_1_3_Byte   format;
	FT2_1_3_UInt   range_count;

	/* that's the table, taken from the file `as is' */
	FT2_1_3_Byte*  data;
	FT2_1_3_UInt   data_size;

	/* small cache for format 3 only */
	FT2_1_3_UInt   cache_first;
	FT2_1_3_UInt   cache_count;
	FT2_1_3_Byte   cache_fd;

} CFF_FDSelectRec, *CFF_FDSelect;


/* A SubFont packs a font dict and a private dict together.  They are */
/* needed to support CID-keyed CFF fonts.                             */
typedef struct  CFF_SubFontRec_ {
	CFF_FontRecDictRec  font_dict;
	CFF_PrivateRec      private_dict;

	CFF_IndexRec        local_subrs_index;
	FT2_1_3_UInt             num_local_subrs;
	FT2_1_3_Byte**           local_subrs;

} CFF_SubFontRec, *CFF_SubFont;


/* maximum number of sub-fonts in a CID-keyed file */
#define CFF_MAX_CID_FONTS  16


typedef struct  CFF_FontRec_ {
	FT2_1_3_Stream        stream;
	FT2_1_3_Memory        memory;
	FT2_1_3_UInt          num_faces;
	FT2_1_3_UInt          num_glyphs;

	FT2_1_3_Byte          version_major;
	FT2_1_3_Byte          version_minor;
	FT2_1_3_Byte          header_size;
	FT2_1_3_Byte          absolute_offsize;


	CFF_IndexRec     name_index;
	CFF_IndexRec     top_dict_index;
	CFF_IndexRec     string_index;
	CFF_IndexRec     global_subrs_index;

	CFF_EncodingRec  encoding;
	CFF_CharsetRec   charset;

	CFF_IndexRec     charstrings_index;
	CFF_IndexRec     font_dict_index;
	CFF_IndexRec     private_index;
	CFF_IndexRec     local_subrs_index;

	FT2_1_3_String*       font_name;
	FT2_1_3_UInt          num_global_subrs;
	FT2_1_3_Byte**        global_subrs;

	CFF_SubFontRec   top_font;
	FT2_1_3_UInt          num_subfonts;
	CFF_SubFont      subfonts[CFF_MAX_CID_FONTS];

	CFF_FDSelectRec  fd_select;

	/* interface to PostScript hinter */
	void*            pshinter;

	/* interface to Postscript Names service */
	void*            psnames;

} CFF_FontRec, *CFF_Font;


FT2_1_3_END_HEADER

#endif /* __CFFTYPES_H__ */


/* END */
