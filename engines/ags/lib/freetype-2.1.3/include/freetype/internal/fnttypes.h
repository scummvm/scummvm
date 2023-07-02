/***************************************************************************/
/*                                                                         */
/*  fnttypes.h                                                             */
/*                                                                         */
/*    Basic Windows FNT/FON type definitions and interface (specification  */
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


#ifndef AGS_LIB_FREETYPE_FNTTYPES_H
#define AGS_LIB_FREETYPE_FNTTYPES_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


typedef struct  WinMZ_HeaderRec_ {
	FT2_1_3_UShort  magic;
	/* skipped content */
	FT2_1_3_UShort  lfanew;

} WinMZ_HeaderRec;


typedef struct  WinNE_HeaderRec_ {
	FT2_1_3_UShort  magic;
	/* skipped content */
	FT2_1_3_UShort  resource_tab_offset;
	FT2_1_3_UShort  rname_tab_offset;

} WinNE_HeaderRec;


typedef struct  WinNameInfoRec_ {
	FT2_1_3_UShort  offset;
	FT2_1_3_UShort  length;
	FT2_1_3_UShort  flags;
	FT2_1_3_UShort  id;
	FT2_1_3_UShort  handle;
	FT2_1_3_UShort  usage;

} WinNameInfoRec;


typedef struct  WinResourceInfoRec_ {
	FT2_1_3_UShort  type_id;
	FT2_1_3_UShort  count;

} WinResourceInfoRec;


#define WINFNT_MZ_MAGIC  0x5A4D
#define WINFNT_NE_MAGIC  0x454E


typedef struct  WinFNT_HeaderRec_ {
	FT2_1_3_UShort  version;
	FT2_1_3_ULong   file_size;
	FT2_1_3_Byte    copyright[60];
	FT2_1_3_UShort  file_type;
	FT2_1_3_UShort  nominal_point_size;
	FT2_1_3_UShort  vertical_resolution;
	FT2_1_3_UShort  horizontal_resolution;
	FT2_1_3_UShort  ascent;
	FT2_1_3_UShort  internal_leading;
	FT2_1_3_UShort  external_leading;
	FT2_1_3_Byte    italic;
	FT2_1_3_Byte    underline;
	FT2_1_3_Byte    strike_out;
	FT2_1_3_UShort  weight;
	FT2_1_3_Byte    charset;
	FT2_1_3_UShort  pixel_width;
	FT2_1_3_UShort  pixel_height;
	FT2_1_3_Byte    pitch_and_family;
	FT2_1_3_UShort  avg_width;
	FT2_1_3_UShort  max_width;
	FT2_1_3_Byte    first_char;
	FT2_1_3_Byte    last_char;
	FT2_1_3_Byte    default_char;
	FT2_1_3_Byte    break_char;
	FT2_1_3_UShort  bytes_per_row;
	FT2_1_3_ULong   device_offset;
	FT2_1_3_ULong   face_name_offset;
	FT2_1_3_ULong   bits_pointer;
	FT2_1_3_ULong   bits_offset;
	FT2_1_3_Byte    reserved;
	FT2_1_3_ULong   flags;
	FT2_1_3_UShort  A_space;
	FT2_1_3_UShort  B_space;
	FT2_1_3_UShort  C_space;
	FT2_1_3_UShort  color_table_offset;
	FT2_1_3_Byte    reserved2[4];

} WinFNT_HeaderRec, *WinFNT_Header;


typedef struct  FNT_FontRec_ {
	FT2_1_3_ULong          offset;
	FT2_1_3_Int            size_shift;

	WinFNT_HeaderRec  header;

	FT2_1_3_Byte*          fnt_frame;
	FT2_1_3_ULong          fnt_size;

} FNT_FontRec, *FNT_Font;


typedef struct  FNT_SizeRec_ {
	FT2_1_3_SizeRec  root;
	FNT_Font    font;

} FNT_SizeRec, *FNT_Size;


typedef struct  FNT_FaceRec_ {
	FT2_1_3_FaceRec     root;

	FT2_1_3_UInt        num_fonts;
	FNT_Font       fonts;

	FT2_1_3_CharMap     charmap_handle;
	FT2_1_3_CharMapRec  charmap;  /* a single charmap per face */

} FNT_FaceRec, *FNT_Face;


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FNTTYPES_H */


/* END */
