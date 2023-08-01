/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/***************************************************************************/
/*                                                                         */
/*  tttypes.h                                                              */
/*                                                                         */
/*    Basic SFNT/TrueType type definitions and interface (specification    */
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


#ifndef AGS_LIB_FREETYPE_TTTYPES_H
#define AGS_LIB_FREETYPE_TTTYPES_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/tttables.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


/**** REQUIRED TRUETYPE/OPENTYPE TABLES DEFINITIONS ****/

typedef struct  TTC_HeaderRec_ {
	FT_ULong   tag;
	FT_Fixed   version;
	FT_Long    count;
	FT_ULong   *offsets;
} TTC_HeaderRec;

typedef struct  SFNT_HeaderRec_ {
	FT_ULong   format_tag;
	FT_UShort  num_tables;
	FT_UShort  search_range;
	FT_UShort  entry_selector;
	FT_UShort  range_shift;

	FT_ULong   offset;  /* not in file */
} SFNT_HeaderRec, *SFNT_Header;

typedef struct  TT_TableDirRec_ {
	FT_Fixed   version;        /* should be 0x10000 */
	FT_UShort  numTables;      /* number of tables  */

	FT_UShort  searchRange;    /* These parameters are only used  */
	FT_UShort  entrySelector;  /* for a dichotomy search in the   */
	FT_UShort  rangeShift;     /* directory.  We ignore them.     */
} TT_TableDirRec;

typedef struct  TT_TableRec_ {
	FT_ULong  Tag;        /*        table type */
	FT_ULong  CheckSum;   /*    table checksum */
	FT_ULong  Offset;     /* table file offset */
	FT_ULong  Length;     /*      table length */
} TT_TableRec, *TT_Table;

typedef struct  TT_CMapDirRec_ {
	FT_UShort  tableVersionNumber;
	FT_UShort  numCMaps;
} TT_CMapDirRec, *TT_CMapDir;

typedef struct  TT_CMapDirEntryRec_ {
	FT_UShort  platformID;
	FT_UShort  platformEncodingID;
	FT_Long    offset;
} TT_CMapDirEntryRec, *TT_CMapDirEntry;

typedef struct  TT_LongMetricsRec_ {
	FT_UShort  advance;
	FT_Short   bearing;
} TT_LongMetricsRec, *TT_LongMetrics;

typedef FT_Short TT_ShortMetrics;

typedef struct  TT_NameEntryRec_ {
	FT_UShort  platformID;
	FT_UShort  encodingID;
	FT_UShort  languageID;
	FT_UShort  nameID;
	FT_UShort  stringLength;
	FT_ULong   stringOffset;

	/* this last field is not defined in the spec */
	/* but used by the FreeType engine            */
	FT_Byte    *string;
} TT_NameEntryRec, *TT_NameEntry;

typedef struct  TT_NameTableRec_ {
	FT_UShort         format;
	FT_UInt           numNameRecords;
	FT_UInt           storageOffset;
	TT_NameEntryRec   *names;
	FT_Stream         stream;
} TT_NameTableRec, *TT_NameTable;


/**** OPTIONAL TRUETYPE/OPENTYPE TABLES DEFINITIONS ****/

typedef struct  TT_GaspRangeRec_ {
	FT_UShort  maxPPEM;
	FT_UShort  gaspFlag;
} TT_GaspRangeRec, *TT_GaspRange;

#define TT_GASP_GRIDFIT  0x01
#define TT_GASP_DOGRAY   0x02

typedef struct  TT_Gasp_ {
	FT_UShort     version;
	FT_UShort     numRanges;
	TT_GaspRange  gaspRanges;
} TT_GaspRec;

typedef struct  TT_HdmxEntryRec_ {
	FT_Byte   ppem;
	FT_Byte   max_width;
	FT_Byte   *widths;
} TT_HdmxEntryRec, *TT_HdmxEntry;

typedef struct  TT_HdmxRec_ {
	FT_UShort     version;
	FT_Short      num_records;
	TT_HdmxEntry  records;
} TT_HdmxRec, *TT_Hdmx;

typedef struct  TT_Kern0_PairRec_ {
	FT_UShort  left;   /* index of left  glyph in pair */
	FT_UShort  right;  /* index of right glyph in pair */
	FT_FWord   value;  /* kerning value                */
} TT_Kern0_PairRec, *TT_Kern0_Pair;


/**** EMBEDDED BITMAPS SUPPORT ****/

typedef struct  TT_SBit_MetricsRec_ {
	FT_Byte  height;
	FT_Byte  width;

	FT_Char  horiBearingX;
	FT_Char  horiBearingY;
	FT_Byte  horiAdvance;

	FT_Char  vertBearingX;
	FT_Char  vertBearingY;
	FT_Byte  vertAdvance;
} TT_SBit_MetricsRec, *TT_SBit_Metrics;

typedef struct  TT_SBit_Small_Metrics_ {
	FT_Byte  height;
	FT_Byte  width;

	FT_Char  bearingX;
	FT_Char  bearingY;
	FT_Byte  advance;
} TT_SBit_SmallMetricsRec, *TT_SBit_SmallMetrics;

typedef struct  TT_SBit_LineMetricsRec_ {
	FT_Char  ascender;
	FT_Char  descender;
	FT_Byte  max_width;
	FT_Char  caret_slope_numerator;
	FT_Char  caret_slope_denominator;
	FT_Char  caret_offset;
	FT_Char  min_origin_SB;
	FT_Char  min_advance_SB;
	FT_Char  max_before_BL;
	FT_Char  min_after_BL;
	FT_Char  pads[2];
} TT_SBit_LineMetricsRec, *TT_SBit_LineMetrics;

typedef struct  TT_SBit_RangeRec {
	FT_UShort           first_glyph;
	FT_UShort           last_glyph;

	FT_UShort           index_format;
	FT_UShort           image_format;
	FT_ULong            image_offset;

	FT_ULong            image_size;
	TT_SBit_MetricsRec  metrics;
	FT_ULong            num_glyphs;

	FT_ULong            *glyph_offsets;
	FT_UShort           *glyph_codes;

	FT_ULong            table_offset;
} TT_SBit_RangeRec, *TT_SBit_Range;

typedef struct  TT_SBit_StrikeRec_ {
	FT_Int                  num_ranges;
	TT_SBit_Range           sbit_ranges;
	FT_ULong                ranges_offset;

	FT_ULong                color_ref;

	TT_SBit_LineMetricsRec  hori;
	TT_SBit_LineMetricsRec  vert;

	FT_UShort               start_glyph;
	FT_UShort               end_glyph;

	FT_Byte                 x_ppem;
	FT_Byte                 y_ppem;

	FT_Byte                 bit_depth;
	FT_Char                 flags;
} TT_SBit_StrikeRec, *TT_SBit_Strike;

typedef struct  TT_SBit_ComponentRec_ {
	FT_UShort  glyph_code;
	FT_Char    x_offset;
	FT_Char    y_offset;
} TT_SBit_ComponentRec, *TT_SBit_Component;

typedef struct  TT_SBit_ScaleRec_ {
	TT_SBit_LineMetricsRec  hori;
	TT_SBit_LineMetricsRec  vert;

	FT_Byte                 x_ppem;
	FT_Byte                 y_ppem;

	FT_Byte                 x_ppem_substitute;
	FT_Byte                 y_ppem_substitute;
} TT_SBit_ScaleRec, *TT_SBit_Scale;


/**** POSTSCRIPT GLYPH NAMES SUPPORT ****/

typedef struct  TT_Post_20Rec_ {
	FT_UShort   num_glyphs;
	FT_UShort   num_names;
	FT_UShort   *glyph_indices;
	FT_Char     **glyph_names;
} TT_Post_20Rec, *TT_Post_20;

typedef struct  TT_Post_25_ {
	FT_UShort  num_glyphs;
	FT_Char    *offsets;
} TT_Post_25Rec, *TT_Post_25;

typedef struct TT_Post_NamesRec_ {
	FT_Bool loaded;
	union {
		TT_Post_20Rec format_20;
		TT_Post_25Rec format_25;
	} names;
} TT_Post_NamesRec, *TT_Post_Names;


/**** TRUETYPE CHARMAPS SUPPORT ****/

/* format 0 */

typedef struct  TT_CMap0_ {
	FT_ULong  language;       /* for Mac fonts (originally ushort) */
	FT_Byte   *glyphIdArray;
} TT_CMap0Rec, *TT_CMap0;


/* format 2 */

typedef struct  TT_CMap2SubHeaderRec_ {
	FT_UShort  firstCode;      /* first valid low byte         */
	FT_UShort  entryCount;     /* number of valid low bytes    */
	FT_Short   idDelta;        /* delta value to glyphIndex    */
	FT_UShort  idRangeOffset;  /* offset from here to 1st code */
} TT_CMap2SubHeaderRec, *TT_CMap2SubHeader;

typedef struct  TT_CMap2Rec_ {
	FT_ULong            language;     /* for Mac fonts (originally ushort) */

	FT_UShort           *subHeaderKeys;  /* high byte mapping table     */
	/* value = subHeader index * 8 */
	TT_CMap2SubHeader   subHeaders;
	FT_UShort           *glyphIdArray;
	FT_UShort           numGlyphId;   /* control value */
} TT_CMap2Rec, *TT_CMap2;


/* format 4 */

typedef struct  TT_CMap4Segment_ {
	FT_UShort  endCount;
	FT_UShort  startCount;
	FT_Short   idDelta;
	FT_UShort  idRangeOffset;
} TT_CMap4SegmentRec, *TT_CMap4Segment;

typedef struct  TT_CMap4Rec_ {
	FT_ULong         language;       /* for Mac fonts (originally ushort) */

	FT_UShort        segCountX2;     /* number of segments * 2            */
	FT_UShort        searchRange;    /* these parameters can be used      */
	FT_UShort        entrySelector;  /* for a binary search               */
	FT_UShort        rangeShift;

	TT_CMap4Segment  segments;
	FT_UShort        *glyphIdArray;
	FT_UShort        numGlyphId;     /* control value */

	TT_CMap4Segment  last_segment;   /* last used segment; this is a small  */
	/* cache to potentially increase speed */
} TT_CMap4Rec, *TT_CMap4;


/* format 6 */

typedef struct  TT_CMap6_ {
	FT_ULong    language;       /* for Mac fonts (originally ushort)     */

	FT_UShort   firstCode;      /* first character code of subrange      */
	FT_UShort   entryCount;     /* number of character codes in subrange */

	FT_UShort   *glyphIdArray;
} TT_CMap6Rec, *TT_CMap6;


/* auxiliary table for format 8 and 12 */

typedef struct  TT_CMapGroupRec_ {
	FT_ULong  startCharCode;
	FT_ULong  endCharCode;
	FT_ULong  startGlyphID;
} TT_CMapGroupRec, *TT_CMapGroup;

typedef struct  TT_CMap8_12Rec_ {
	FT_ULong      language;        /* for Mac fonts */

	FT_ULong      nGroups;
	TT_CMapGroup  groups;

	TT_CMapGroup  last_group;      /* last used group; this is a small    */
	/* cache to potentially increase speed */
} TT_CMap8_12Rec, *TT_CMap8_12;


/* format 10 */

typedef struct  TT_CMap10Rec_ {
	FT_ULong    language;           /* for Mac fonts */

	FT_ULong    startCharCode;      /* first character covered */
	FT_ULong    numChars;           /* number of characters covered */

	FT_UShort   *glyphs;
} TT_CMap10Rec, *TT_CMap10;


typedef struct TT_CMapTableRec_ *TT_CMapTable;

typedef FT_UInt  (*TT_CharMap_Func)(TT_CMapTable charmap, FT_ULong char_code);
typedef FT_ULong (*TT_CharNext_Func)(TT_CMapTable charmap, FT_ULong char_code);

/* charmap table */
typedef struct  TT_CMapTableRec_ {
	FT_UShort  platformID;
	FT_UShort  platformEncodingID;
	FT_UShort  format;
	FT_ULong   length;          /* must be ulong for formats 8, 10, and 12 */

	FT_Bool    loaded;
	FT_ULong   offset;

	union {
		TT_CMap0Rec     cmap0;
		TT_CMap2Rec     cmap2;
		TT_CMap4Rec     cmap4;
		TT_CMap6Rec     cmap6;
		TT_CMap8_12Rec  cmap8_12;
		TT_CMap10Rec    cmap10;
	} c;

	TT_CharMap_Func   get_index;
	TT_CharNext_Func  get_next_char;
} TT_CMapTableRec;

typedef struct  TT_CharMapRec_ {
	FT_CharMapRec    root;
	TT_CMapTableRec  cmap;
} TT_CharMapRec;


/**** ORIGINAL TT_FACE CLASS DEFINITION ****/

typedef struct TT_FaceRec_ *TT_Face;
typedef struct TT_CharMapRec_ *TT_CharMap;

/* a function type used for the truetype bytecode interpreter hooks */
typedef FT_Error (*TT_Interpreter)(void *exec_context);

/* forward declaration */
typedef struct TT_LoaderRec_ *TT_Loader;

typedef FT_Error (*TT_Loader_GotoTableFunc)(TT_Face face, FT_ULong tag, FT_Stream stream, FT_ULong *length);
typedef FT_Error (*TT_Loader_StartGlyphFunc)(TT_Loader loader, FT_UInt glyph_index, FT_ULong offset, FT_UInt byte_count);
typedef FT_Error (*TT_Loader_ReadGlyphFunc)(TT_Loader loader);
typedef void 	 (*TT_Loader_EndGlyphFunc)(TT_Loader loader);


/**** TrueType Face Type ****/

typedef struct  TT_FaceRec_ {
	FT_FaceRec            root;

	TTC_HeaderRec         ttc_header;

	FT_ULong              format_tag;
	FT_UShort             num_tables;
	TT_Table              dir_tables;

	TT_Header             header;       /* TrueType header table          */
	TT_HoriHeader         horizontal;   /* TrueType horizontal header     */

	TT_MaxProfile         max_profile;
	FT_ULong              max_components;

	FT_Bool               vertical_info;
	TT_VertHeader         vertical;     /* TT Vertical header, if present */

	FT_UShort             num_names;    /* number of name records  */
	TT_NameTableRec       name_table;   /* name table              */

	TT_OS2                os2;          /* TrueType OS/2 table            */
	TT_Postscript         postscript;   /* TrueType Postscript table      */

	FT_Byte               *cmap_table;   /* extracted 'cmap' table */
	FT_ULong              cmap_size;

	TT_Loader_GotoTableFunc   goto_table;

	TT_Loader_StartGlyphFunc  access_glyph_frame;
	TT_Loader_EndGlyphFunc    forget_glyph_frame;
	TT_Loader_ReadGlyphFunc   read_glyph_header;
	TT_Loader_ReadGlyphFunc   read_simple_glyph;
	TT_Loader_ReadGlyphFunc   read_composite_glyph;

	/* a typeless pointer to the SFNT_Interface table used to load     */
	/* the basic TrueType tables in the face object                    */
	void                  *sfnt;

	/* a typeless pointer to the PSNames_Interface table used to       */
	/* handle glyph names <-> unicode & Mac values                     */
	void                  *psnames;


	/* Optional TrueType/OpenType tables */

	/* horizontal device metrics */
	TT_HdmxRec            hdmx;

	/* grid-fitting and scaling table */
	TT_GaspRec            gasp;                 /* the `gasp' table */

	/* PCL 5 table */
	TT_PCLT               pclt;

	/* embedded bitmaps support */
	FT_ULong              num_sbit_strikes;
	TT_SBit_Strike        sbit_strikes;

	FT_ULong              num_sbit_scales;
	TT_SBit_Scale         sbit_scales;

	/* postscript names table */
	TT_Post_NamesRec      postscript_names;


	/* TrueType-specific fields (ignored by the OTF-Type2 driver) */

	/* the glyph locations */
	FT_UShort             num_locations;
	FT_Long               *glyph_locations;

	/* the font program, if any */
	FT_ULong              font_program_size;
	FT_Byte               *font_program;

	/* the cvt program, if any */
	FT_ULong              cvt_program_size;
	FT_Byte               *cvt_program;

	/* the original, unscaled, control value table */
	FT_ULong              cvt_size;
	FT_Short              *cvt;

	/* the format 0 kerning table, if any */
	FT_Int                num_kern_pairs;
	FT_Int                kern_table_index;
	TT_Kern0_Pair         kern_pairs;

	/* A pointer to the bytecode interpreter to use.  This is also */
	/* used to hook the debugger for the `ttdebug' utility.        */
	TT_Interpreter        interpreter;


	/* Other tables or fields */
	FT_Generic            extra;
} TT_FaceRec;


typedef struct  TT_GlyphZoneRec_ {
	FT_Memory   memory;
	FT_UShort   max_points;
	FT_UShort   max_contours;
	FT_UShort   n_points;   /* number of points in zone    */
	FT_Short    n_contours; /* number of contours          */

	FT_Vector   *org;       /* original point coordinates  */
	FT_Vector   *cur;       /* current point coordinates   */

	FT_Byte     *tags;      /* current touch flags         */
	FT_UShort   *contours;  /* contour end points          */
} TT_GlyphZoneRec, *TT_GlyphZone;


/* handle to execution context */
typedef struct TT_ExecContextRec_ *TT_ExecContext;

/* glyph loader structure */
typedef struct  TT_LoaderRec_ {
	FT_Face          face;
	FT_Size          size;
	FT_GlyphSlot     glyph;
	FT_GlyphLoader   gloader;

	FT_ULong         load_flags;
	FT_UInt          glyph_index;

	FT_Stream        stream;
	FT_Int           byte_len;

	FT_Short         n_contours;
	FT_BBox          bbox;
	FT_Int           left_bearing;
	FT_Int           advance;
	FT_Int           linear;
	FT_Bool          linear_def;
	FT_Bool          preserve_pps;
	FT_Vector        pp1;
	FT_Vector        pp2;

	FT_ULong         glyf_offset;

	/* the zone where we load our glyphs */
	TT_GlyphZoneRec  base;
	TT_GlyphZoneRec  zone;

	TT_ExecContext   exec;
	FT_Byte          *instructions;
	FT_ULong         ins_pos;

	/* for possible extensibility in other formats */
	void             *other;
} TT_LoaderRec;


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_TTTYPES_H */
