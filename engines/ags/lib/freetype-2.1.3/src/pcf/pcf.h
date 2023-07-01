/*  pcf.h

  FreeType font driver for pcf fonts

  Copyright (C) 2000-2001, 2002 by
  Francesco Zappa Nardelli

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#ifndef __PCF_H__
#define __PCF_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftdriver.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftstream.h"


FT2_1_3_BEGIN_HEADER

typedef struct  PCF_TableRec_ {
	FT2_1_3_ULong  type;
	FT2_1_3_ULong  format;
	FT2_1_3_ULong  size;
	FT2_1_3_ULong  offset;

} PCF_TableRec, *PCF_Table;


typedef struct  PCF_TocRec_ {
	FT2_1_3_ULong   version;
	FT2_1_3_ULong   count;
	PCF_Table  tables;

} PCF_TocRec, *PCF_Toc;


typedef struct  PCF_ParsePropertyRec_ {
	FT2_1_3_Long  name;
	FT2_1_3_Byte  isString;
	FT2_1_3_Long  value;

} PCF_ParsePropertyRec, *PCF_ParseProperty;


typedef struct  PCF_PropertyRec_ {
	FT2_1_3_String*  name;
	FT2_1_3_Byte     isString;

	union {
		FT2_1_3_String*  atom;
		FT2_1_3_Long     integer;
		FT2_1_3_ULong    cardinal;

	} value;

} PCF_PropertyRec, *PCF_Property;


typedef struct  PCF_Compressed_MetricRec_ {
	FT2_1_3_Byte  leftSideBearing;
	FT2_1_3_Byte  rightSideBearing;
	FT2_1_3_Byte  characterWidth;
	FT2_1_3_Byte  ascent;
	FT2_1_3_Byte  descent;

} PCF_Compressed_MetricRec, *PCF_Compressed_Metric;


typedef struct  PCF_MetricRec_ {
	FT2_1_3_Short  leftSideBearing;
	FT2_1_3_Short  rightSideBearing;
	FT2_1_3_Short  characterWidth;
	FT2_1_3_Short  ascent;
	FT2_1_3_Short  descent;
	FT2_1_3_Short  attributes;
	FT2_1_3_ULong  bits;

} PCF_MetricRec, *PCF_Metric;


typedef struct  PCF_AccelRec_ {
	FT2_1_3_Byte        noOverlap;
	FT2_1_3_Byte        constantMetrics;
	FT2_1_3_Byte        terminalFont;
	FT2_1_3_Byte        constantWidth;
	FT2_1_3_Byte        inkInside;
	FT2_1_3_Byte        inkMetrics;
	FT2_1_3_Byte        drawDirection;
	FT2_1_3_Long        fontAscent;
	FT2_1_3_Long        fontDescent;
	FT2_1_3_Long        maxOverlap;
	PCF_MetricRec  minbounds;
	PCF_MetricRec  maxbounds;
	PCF_MetricRec  ink_minbounds;
	PCF_MetricRec  ink_maxbounds;

} PCF_AccelRec, *PCF_Accel;


typedef struct  PCD_EncodingRec_ {
	FT2_1_3_Long   enc;
	FT2_1_3_Short  glyph;

} PCF_EncodingRec, *PCF_Encoding;


typedef struct  PCF_FaceRec_ {
	FT2_1_3_FaceRec     root;

	FT2_1_3_StreamRec   gzip_stream;
	FT2_1_3_Stream      gzip_source;

	char*          charset_encoding;
	char*          charset_registry;

	PCF_TocRec     toc;
	PCF_AccelRec   accel;

	int            nprops;
	PCF_Property   properties;

	FT2_1_3_Long        nmetrics;
	PCF_Metric     metrics;
	FT2_1_3_Long        nencodings;
	PCF_Encoding   encodings;

	FT2_1_3_Short       defaultChar;

	FT2_1_3_ULong       bitmapsFormat;

	FT2_1_3_CharMap     charmap_handle;
	FT2_1_3_CharMapRec  charmap;  /* a single charmap per face */

} PCF_FaceRec, *PCF_Face;


/* macros for pcf font format */

#define LSBFirst  0
#define MSBFirst  1

#define PCF_FILE_VERSION        ( ( 'p' << 24 ) | \
                                  ( 'c' << 16 ) | \
                                  ( 'f' <<  8 ) | 1 )
#define PCF_FORMAT_MASK         0xFFFFFF00L

#define PCF_DEFAULT_FORMAT      0x00000000L
#define PCF_INKBOUNDS           0x00000200L
#define PCF_ACCEL_W_INKBOUNDS   0x00000100L
#define PCF_COMPRESSED_METRICS  0x00000100L

#define PCF_FORMAT_MATCH( a, b ) \
          ( ( (a) & PCF_FORMAT_MASK ) == ( (b) & PCF_FORMAT_MASK ) )

#define PCF_GLYPH_PAD_MASK  ( 3 << 0 )
#define PCF_BYTE_MASK       ( 1 << 2 )
#define PCF_BIT_MASK        ( 1 << 3 )
#define PCF_SCAN_UNIT_MASK  ( 3 << 4 )

#define PCF_BYTE_ORDER( f ) \
          ( ( (f) & PCF_BYTE_MASK ) ? MSBFirst : LSBFirst )
#define PCF_BIT_ORDER( f ) \
          ( ( (f) & PCF_BIT_MASK ) ? MSBFirst : LSBFirst )
#define PCF_GLYPH_PAD_INDEX( f ) \
          ( (f) & PCF_GLYPH_PAD_MASK )
#define PCF_GLYPH_PAD( f ) \
          ( 1 << PCF_GLYPH_PAD_INDEX( f ) )
#define PCF_SCAN_UNIT_INDEX( f ) \
          ( ( (f) & PCF_SCAN_UNIT_MASK ) >> 4 )
#define PCF_SCAN_UNIT( f ) \
          ( 1 << PCF_SCAN_UNIT_INDEX( f ) )
#define PCF_FORMAT_BITS( f )             \
          ( (f) & ( PCF_GLYPH_PAD_MASK | \
                    PCF_BYTE_MASK      | \
                    PCF_BIT_MASK       | \
                    PCF_SCAN_UNIT_MASK ) )

#define PCF_SIZE_TO_INDEX( s )  ( (s) == 4 ? 2 : (s) == 2 ? 1 : 0 )
#define PCF_INDEX_TO_SIZE( b )  ( 1 << b )

#define PCF_FORMAT( bit, byte, glyph, scan )          \
          ( ( PCF_SIZE_TO_INDEX( scan )      << 4 ) | \
            ( ( (bit)  == MSBFirst ? 1 : 0 ) << 3 ) | \
            ( ( (byte) == MSBFirst ? 1 : 0 ) << 2 ) | \
            ( PCF_SIZE_TO_INDEX( glyph )     << 0 ) )

#define PCF_PROPERTIES        ( 1 << 0 )
#define PCF_ACCELERATORS      ( 1 << 1 )
#define PCF_METRICS           ( 1 << 2 )
#define PCF_BITMAPS           ( 1 << 3 )
#define PCF_INK_METRICS       ( 1 << 4 )
#define PCF_BDF_ENCODINGS     ( 1 << 5 )
#define PCF_SWIDTHS           ( 1 << 6 )
#define PCF_GLYPH_NAMES       ( 1 << 7 )
#define PCF_BDF_ACCELERATORS  ( 1 << 8 )

#define GLYPHPADOPTIONS  4 /* I'm not sure about this */

FT2_1_3_LOCAL( FT2_1_3_Error )
pcf_load_font( FT2_1_3_Stream,
               PCF_Face );


FT2_1_3_END_HEADER

#endif /* __PCF_H__ */


/* END */
