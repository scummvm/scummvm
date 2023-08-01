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
/*  tttables.h                                                             */
/*                                                                         */
/*    Basic SFNT/TrueType tables definitions and interface                 */
/*    (specification only).                                                */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_TTTABLES_H
#define AGS_LIB_FREETYPE_TTTABLES_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


typedef struct  TT_Header_ {
	FT_Fixed   Table_Version;
	FT_Fixed   Font_Revision;

	FT_Long    CheckSum_Adjust;
	FT_Long    Magic_Number;

	FT_UShort  Flags;
	FT_UShort  Units_Per_EM;

	FT_Long    Created [2];
	FT_Long    Modified[2];

	FT_Short   xMin;
	FT_Short   yMin;
	FT_Short   xMax;
	FT_Short   yMax;

	FT_UShort  Mac_Style;
	FT_UShort  Lowest_Rec_PPEM;

	FT_Short   Font_Direction;
	FT_Short   Index_To_Loc_Format;
	FT_Short   Glyph_Data_Format;
} TT_Header;

typedef struct  TT_HoriHeader_ {
	FT_Fixed   Version;
	FT_Short   Ascender;
	FT_Short   Descender;
	FT_Short   Line_Gap;

	FT_UShort  advance_Width_Max;      /* advance width maximum */

	FT_Short   min_Left_Side_Bearing;  /* minimum left-sb       */
	FT_Short   min_Right_Side_Bearing; /* minimum right-sb      */
	FT_Short   xMax_Extent;            /* xmax extents          */
	FT_Short   caret_Slope_Rise;
	FT_Short   caret_Slope_Run;
	FT_Short   caret_Offset;

	FT_Short   Reserved[4];

	FT_Short   metric_Data_Format;
	FT_UShort  number_Of_HMetrics;

	/* The following fields are not defined by the TrueType specification */
	/* but they are used to connect the metrics header to the relevant    */
	/* `HMTX' table.                                                      */

	void*      long_metrics;
	void*      short_metrics;
} TT_HoriHeader;

typedef struct  TT_VertHeader_ {
	FT_Fixed   Version;
	FT_Short   Ascender;
	FT_Short   Descender;
	FT_Short   Line_Gap;

	FT_UShort  advance_Height_Max;      /* advance height maximum */

	FT_Short   min_Top_Side_Bearing;    /* minimum left-sb or top-sb       */
	FT_Short   min_Bottom_Side_Bearing; /* minimum right-sb or bottom-sb   */
	FT_Short   yMax_Extent;             /* xmax or ymax extents            */
	FT_Short   caret_Slope_Rise;
	FT_Short   caret_Slope_Run;
	FT_Short   caret_Offset;

	FT_Short   Reserved[4];

	FT_Short   metric_Data_Format;
	FT_UShort  number_Of_VMetrics;

	/* The following fields are not defined by the TrueType specification */
	/* but they're used to connect the metrics header to the relevant     */
	/* `HMTX' or `VMTX' table.                                            */

	void       *long_metrics;
	void       *short_metrics;
} TT_VertHeader;

typedef struct  TT_OS2_ {
	FT_UShort  version;                /* 0x0001 - more or 0xFFFF */
	FT_Short   xAvgCharWidth;
	FT_UShort  usWeightClass;
	FT_UShort  usWidthClass;
	FT_Short   fsType;
	FT_Short   ySubscriptXSize;
	FT_Short   ySubscriptYSize;
	FT_Short   ySubscriptXOffset;
	FT_Short   ySubscriptYOffset;
	FT_Short   ySuperscriptXSize;
	FT_Short   ySuperscriptYSize;
	FT_Short   ySuperscriptXOffset;
	FT_Short   ySuperscriptYOffset;
	FT_Short   yStrikeoutSize;
	FT_Short   yStrikeoutPosition;
	FT_Short   sFamilyClass;

	FT_Byte    panose[10];

	FT_ULong   ulUnicodeRange1;        /* Bits 0-31   */
	FT_ULong   ulUnicodeRange2;        /* Bits 32-63  */
	FT_ULong   ulUnicodeRange3;        /* Bits 64-95  */
	FT_ULong   ulUnicodeRange4;        /* Bits 96-127 */

	FT_Char    achVendID[4];

	FT_UShort  fsSelection;
	FT_UShort  usFirstCharIndex;
	FT_UShort  usLastCharIndex;
	FT_Short   sTypoAscender;
	FT_Short   sTypoDescender;
	FT_Short   sTypoLineGap;
	FT_UShort  usWinAscent;
	FT_UShort  usWinDescent;

	/* only version 1 tables: */

	FT_ULong   ulCodePageRange1;       /* Bits 0-31   */
	FT_ULong   ulCodePageRange2;       /* Bits 32-63  */

	/* only version 2 tables: */

	FT_Short   sxHeight;
	FT_Short   sCapHeight;
	FT_UShort  usDefaultChar;
	FT_UShort  usBreakChar;
	FT_UShort  usMaxContext;
} TT_OS2;

typedef struct  TT_Postscript_ {
	FT_Fixed  FormatType;
	FT_Fixed  italicAngle;
	FT_Short  underlinePosition;
	FT_Short  underlineThickness;
	FT_ULong  isFixedPitch;
	FT_ULong  minMemType42;
	FT_ULong  maxMemType42;
	FT_ULong  minMemType1;
	FT_ULong  maxMemType1;
} TT_Postscript;

typedef struct  TT_PCLT_ {
	FT_Fixed   Version;
	FT_ULong   FontNumber;
	FT_UShort  Pitch;
	FT_UShort  xHeight;
	FT_UShort  Style;
	FT_UShort  TypeFamily;
	FT_UShort  CapHeight;
	FT_UShort  SymbolSet;
	FT_Char    TypeFace[16];
	FT_Char    CharacterComplement[8];
	FT_Char    FileName[6];
	FT_Char    StrokeWeight;
	FT_Char    WidthType;
	FT_Byte    SerifStyle;
	FT_Byte    Reserved;
} TT_PCLT;

typedef struct  TT_MaxProfile_ {
	FT_Fixed   version;
	FT_UShort  numGlyphs;
	FT_UShort  maxPoints;
	FT_UShort  maxContours;
	FT_UShort  maxCompositePoints;
	FT_UShort  maxCompositeContours;
	FT_UShort  maxZones;
	FT_UShort  maxTwilightPoints;
	FT_UShort  maxStorage;
	FT_UShort  maxFunctionDefs;
	FT_UShort  maxInstructionDefs;
	FT_UShort  maxStackElements;
	FT_UShort  maxSizeOfInstructions;
	FT_UShort  maxComponentElements;
	FT_UShort  maxComponentDepth;
} TT_MaxProfile;

typedef enum {
	ft_sfnt_head = 0,
	ft_sfnt_maxp = 1,
	ft_sfnt_os2  = 2,
	ft_sfnt_hhea = 3,
	ft_sfnt_vhea = 4,
	ft_sfnt_post = 5,
	ft_sfnt_pclt = 6,

	sfnt_max   /* don't remove */
} FT_Sfnt_Tag;


/* internal use only */
typedef void *(*FT2_1_3_Get_Sfnt_Table_Func)(FT_Face face, FT_Sfnt_Tag tag);

FT_EXPORT(void *)
FT2_1_3_Get_Sfnt_Table(FT_Face face, FT_Sfnt_Tag tag);


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_TTTABLES_H */
