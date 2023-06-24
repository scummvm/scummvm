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


#ifndef __TTTABLES_H__
#define __TTTABLES_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_FREETYPE_H


FT2_1_3_BEGIN_HEADER

/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    truetype_tables                                                    */
/*                                                                       */
/* <Title>                                                               */
/*    TrueType Tables                                                    */
/*                                                                       */
/* <Abstract>                                                            */
/*    TrueType-specific table types and functions.                       */
/*                                                                       */
/* <Description>                                                         */
/*    This section contains the definition of TrueType-specific tables   */
/*    as well as some routines used to access and process them.          */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    TT_Header                                                          */
/*                                                                       */
/* <Description>                                                         */
/*    A structure used to model a TrueType font header table.  All       */
/*    fields follow the TrueType specification.                          */
/*                                                                       */
typedef struct  TT_Header_ {
	FT2_1_3_Fixed   Table_Version;
	FT2_1_3_Fixed   Font_Revision;

	FT2_1_3_Long    CheckSum_Adjust;
	FT2_1_3_Long    Magic_Number;

	FT2_1_3_UShort  Flags;
	FT2_1_3_UShort  Units_Per_EM;

	FT2_1_3_Long    Created [2];
	FT2_1_3_Long    Modified[2];

	FT2_1_3_Short   xMin;
	FT2_1_3_Short   yMin;
	FT2_1_3_Short   xMax;
	FT2_1_3_Short   yMax;

	FT2_1_3_UShort  Mac_Style;
	FT2_1_3_UShort  Lowest_Rec_PPEM;

	FT2_1_3_Short   Font_Direction;
	FT2_1_3_Short   Index_To_Loc_Format;
	FT2_1_3_Short   Glyph_Data_Format;

} TT_Header;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    TT_HoriHeader                                                      */
/*                                                                       */
/* <Description>                                                         */
/*    A structure used to model a TrueType horizontal header, the `hhea' */
/*    table, as well as the corresponding horizontal metrics table,      */
/*    i.e., the `hmtx' table.                                            */
/*                                                                       */
/* <Fields>                                                              */
/*    Version                :: The table version.                       */
/*                                                                       */
/*    Ascender               :: The font's ascender, i.e., the distance  */
/*                              from the baseline to the top-most of all */
/*                              glyph points found in the font.          */
/*                                                                       */
/*                              This value is invalid in many fonts, as  */
/*                              it is usually set by the font designer,  */
/*                              and often reflects only a portion of the */
/*                              glyphs found in the font (maybe ASCII).  */
/*                                                                       */
/*                              You should use the `sTypoAscender' field */
/*                              of the OS/2 table instead if you want    */
/*                              the correct one.                         */
/*                                                                       */
/*    Descender              :: The font's descender, i.e., the distance */
/*                              from the baseline to the bottom-most of  */
/*                              all glyph points found in the font.  It  */
/*                              is negative.                             */
/*                                                                       */
/*                              This value is invalid in many fonts, as  */
/*                              it is usually set by the font designer,  */
/*                              and often reflects only a portion of the */
/*                              glyphs found in the font (maybe ASCII).  */
/*                                                                       */
/*                              You should use the `sTypoDescender'      */
/*                              field of the OS/2 table instead if you   */
/*                              want the correct one.                    */
/*                                                                       */
/*    Line_Gap               :: The font's line gap, i.e., the distance  */
/*                              to add to the ascender and descender to  */
/*                              get the BTB, i.e., the                   */
/*                              baseline-to-baseline distance for the    */
/*                              font.                                    */
/*                                                                       */
/*    advance_Width_Max      :: This field is the maximum of all advance */
/*                              widths found in the font.  It can be     */
/*                              used to compute the maximum width of an  */
/*                              arbitrary string of text.                */
/*                                                                       */
/*    min_Left_Side_Bearing  :: The minimum left side bearing of all     */
/*                              glyphs within the font.                  */
/*                                                                       */
/*    min_Right_Side_Bearing :: The minimum right side bearing of all    */
/*                              glyphs within the font.                  */
/*                                                                       */
/*    xMax_Extent            :: The maximum horizontal extent (i.e., the */
/*                              `width' of a glyph's bounding box) for   */
/*                              all glyphs in the font.                  */
/*                                                                       */
/*    caret_Slope_Rise       :: The rise coefficient of the cursor's     */
/*                              slope of the cursor (slope=rise/run).    */
/*                                                                       */
/*    caret_Slope_Run        :: The run coefficient of the cursor's      */
/*                              slope.                                   */
/*                                                                       */
/*    Reserved               :: 10 reserved bytes.                       */
/*                                                                       */
/*    metric_Data_Format     :: Always 0.                                */
/*                                                                       */
/*    number_Of_HMetrics     :: Number of HMetrics entries in the `hmtx' */
/*                              table -- this value can be smaller than  */
/*                              the total number of glyphs in the font.  */
/*                                                                       */
/*    long_metrics           :: A pointer into the `hmtx' table.         */
/*                                                                       */
/*    short_metrics          :: A pointer into the `hmtx' table.         */
/*                                                                       */
/* <Note>                                                                */
/*    IMPORTANT: The TT_HoriHeader and TT_VertHeader structures should   */
/*               be identical except for the names of their fields which */
/*               are different.                                          */
/*                                                                       */
/*               This ensures that a single function in the `ttload'     */
/*               module is able to read both the horizontal and vertical */
/*               headers.                                                */
/*                                                                       */
typedef struct  TT_HoriHeader_ {
	FT2_1_3_Fixed   Version;
	FT2_1_3_Short   Ascender;
	FT2_1_3_Short   Descender;
	FT2_1_3_Short   Line_Gap;

	FT2_1_3_UShort  advance_Width_Max;      /* advance width maximum */

	FT2_1_3_Short   min_Left_Side_Bearing;  /* minimum left-sb       */
	FT2_1_3_Short   min_Right_Side_Bearing; /* minimum right-sb      */
	FT2_1_3_Short   xMax_Extent;            /* xmax extents          */
	FT2_1_3_Short   caret_Slope_Rise;
	FT2_1_3_Short   caret_Slope_Run;
	FT2_1_3_Short   caret_Offset;

	FT2_1_3_Short   Reserved[4];

	FT2_1_3_Short   metric_Data_Format;
	FT2_1_3_UShort  number_Of_HMetrics;

	/* The following fields are not defined by the TrueType specification */
	/* but they are used to connect the metrics header to the relevant    */
	/* `HMTX' table.                                                      */

	void*      long_metrics;
	void*      short_metrics;

} TT_HoriHeader;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    TT_VertHeader                                                      */
/*                                                                       */
/* <Description>                                                         */
/*    A structure used to model a TrueType vertical header, the `vhea'   */
/*    table, as well as the corresponding vertical metrics table, i.e.,  */
/*    the `vmtx' table.                                                  */
/*                                                                       */
/* <Fields>                                                              */
/*    Version                 :: The table version.                      */
/*                                                                       */
/*    Ascender                :: The font's ascender, i.e., the distance */
/*                               from the baseline to the top-most of    */
/*                               all glyph points found in the font.     */
/*                                                                       */
/*                               This value is invalid in many fonts, as */
/*                               it is usually set by the font designer, */
/*                               and often reflects only a portion of    */
/*                               the glyphs found in the font (maybe     */
/*                               ASCII).                                 */
/*                                                                       */
/*                               You should use the `sTypoAscender'      */
/*                               field of the OS/2 table instead if you  */
/*                               want the correct one.                   */
/*                                                                       */
/*    Descender               :: The font's descender, i.e., the         */
/*                               distance from the baseline to the       */
/*                               bottom-most of all glyph points found   */
/*                               in the font.  It is negative.           */
/*                                                                       */
/*                               This value is invalid in many fonts, as */
/*                               it is usually set by the font designer, */
/*                               and often reflects only a portion of    */
/*                               the glyphs found in the font (maybe     */
/*                               ASCII).                                 */
/*                                                                       */
/*                               You should use the `sTypoDescender'     */
/*                               field of the OS/2 table instead if you  */
/*                               want the correct one.                   */
/*                                                                       */
/*    Line_Gap                :: The font's line gap, i.e., the distance */
/*                               to add to the ascender and descender to */
/*                               get the BTB, i.e., the                  */
/*                               baseline-to-baseline distance for the   */
/*                               font.                                   */
/*                                                                       */
/*    advance_Height_Max      :: This field is the maximum of all        */
/*                               advance heights found in the font.  It  */
/*                               can be used to compute the maximum      */
/*                               height of an arbitrary string of text.  */
/*                                                                       */
/*    min_Top_Side_Bearing    :: The minimum top side bearing of all     */
/*                               glyphs within the font.                 */
/*                                                                       */
/*    min_Bottom_Side_Bearing :: The minimum bottom side bearing of all  */
/*                               glyphs within the font.                 */
/*                                                                       */
/*    yMax_Extent             :: The maximum vertical extent (i.e., the  */
/*                               `height' of a glyph's bounding box) for */
/*                               all glyphs in the font.                 */
/*                                                                       */
/*    caret_Slope_Rise        :: The rise coefficient of the cursor's    */
/*                               slope of the cursor (slope=rise/run).   */
/*                                                                       */
/*    caret_Slope_Run         :: The run coefficient of the cursor's     */
/*                               slope.                                  */
/*                                                                       */
/*    caret_Offset            :: The cursor's offset for slanted fonts.  */
/*                               This value is `reserved' in vmtx        */
/*                               version 1.0.                            */
/*                                                                       */
/*    Reserved                :: 8 reserved bytes.                       */
/*                                                                       */
/*    metric_Data_Format      :: Always 0.                               */
/*                                                                       */
/*    number_Of_HMetrics      :: Number of VMetrics entries in the       */
/*                               `vmtx' table -- this value can be       */
/*                               smaller than the total number of glyphs */
/*                               in the font.                            */
/*                                                                       */
/*    long_metrics           :: A pointer into the `vmtx' table.         */
/*                                                                       */
/*    short_metrics          :: A pointer into the `vmtx' table.         */
/*                                                                       */
/* <Note>                                                                */
/*    IMPORTANT: The TT_HoriHeader and TT_VertHeader structures should   */
/*               be identical except for the names of their fields which */
/*               are different.                                          */
/*                                                                       */
/*               This ensures that a single function in the `ttload'     */
/*               module is able to read both the horizontal and vertical */
/*               headers.                                                */
/*                                                                       */
typedef struct  TT_VertHeader_ {
	FT2_1_3_Fixed   Version;
	FT2_1_3_Short   Ascender;
	FT2_1_3_Short   Descender;
	FT2_1_3_Short   Line_Gap;

	FT2_1_3_UShort  advance_Height_Max;      /* advance height maximum */

	FT2_1_3_Short   min_Top_Side_Bearing;    /* minimum left-sb or top-sb       */
	FT2_1_3_Short   min_Bottom_Side_Bearing; /* minimum right-sb or bottom-sb   */
	FT2_1_3_Short   yMax_Extent;             /* xmax or ymax extents            */
	FT2_1_3_Short   caret_Slope_Rise;
	FT2_1_3_Short   caret_Slope_Run;
	FT2_1_3_Short   caret_Offset;

	FT2_1_3_Short   Reserved[4];

	FT2_1_3_Short   metric_Data_Format;
	FT2_1_3_UShort  number_Of_VMetrics;

	/* The following fields are not defined by the TrueType specification */
	/* but they're used to connect the metrics header to the relevant     */
	/* `HMTX' or `VMTX' table.                                            */

	void*      long_metrics;
	void*      short_metrics;

} TT_VertHeader;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    TT_OS2                                                             */
/*                                                                       */
/* <Description>                                                         */
/*    A structure used to model a TrueType OS/2 table. This is the long  */
/*    table version.  All fields comply to the TrueType specification.   */
/*                                                                       */
/*    Note that we now support old Mac fonts which do not include an     */
/*    OS/2 table.  In this case, the `version' field is always set to    */
/*    0xFFFF.                                                            */
/*                                                                       */
typedef struct  TT_OS2_ {
	FT2_1_3_UShort  version;                /* 0x0001 - more or 0xFFFF */
	FT2_1_3_Short   xAvgCharWidth;
	FT2_1_3_UShort  usWeightClass;
	FT2_1_3_UShort  usWidthClass;
	FT2_1_3_Short   fsType;
	FT2_1_3_Short   ySubscriptXSize;
	FT2_1_3_Short   ySubscriptYSize;
	FT2_1_3_Short   ySubscriptXOffset;
	FT2_1_3_Short   ySubscriptYOffset;
	FT2_1_3_Short   ySuperscriptXSize;
	FT2_1_3_Short   ySuperscriptYSize;
	FT2_1_3_Short   ySuperscriptXOffset;
	FT2_1_3_Short   ySuperscriptYOffset;
	FT2_1_3_Short   yStrikeoutSize;
	FT2_1_3_Short   yStrikeoutPosition;
	FT2_1_3_Short   sFamilyClass;

	FT2_1_3_Byte    panose[10];

	FT2_1_3_ULong   ulUnicodeRange1;        /* Bits 0-31   */
	FT2_1_3_ULong   ulUnicodeRange2;        /* Bits 32-63  */
	FT2_1_3_ULong   ulUnicodeRange3;        /* Bits 64-95  */
	FT2_1_3_ULong   ulUnicodeRange4;        /* Bits 96-127 */

	FT2_1_3_Char    achVendID[4];

	FT2_1_3_UShort  fsSelection;
	FT2_1_3_UShort  usFirstCharIndex;
	FT2_1_3_UShort  usLastCharIndex;
	FT2_1_3_Short   sTypoAscender;
	FT2_1_3_Short   sTypoDescender;
	FT2_1_3_Short   sTypoLineGap;
	FT2_1_3_UShort  usWinAscent;
	FT2_1_3_UShort  usWinDescent;

	/* only version 1 tables: */

	FT2_1_3_ULong   ulCodePageRange1;       /* Bits 0-31   */
	FT2_1_3_ULong   ulCodePageRange2;       /* Bits 32-63  */

	/* only version 2 tables: */

	FT2_1_3_Short   sxHeight;
	FT2_1_3_Short   sCapHeight;
	FT2_1_3_UShort  usDefaultChar;
	FT2_1_3_UShort  usBreakChar;
	FT2_1_3_UShort  usMaxContext;

} TT_OS2;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    TT_Postscript                                                      */
/*                                                                       */
/* <Description>                                                         */
/*    A structure used to model a TrueType Postscript table.  All fields */
/*    comply to the TrueType table.  This structure does not reference   */
/*    the Postscript glyph names, which can be nevertheless accessed     */
/*    with the `ttpost' module.                                          */
/*                                                                       */
typedef struct  TT_Postscript_ {
	FT2_1_3_Fixed  FormatType;
	FT2_1_3_Fixed  italicAngle;
	FT2_1_3_Short  underlinePosition;
	FT2_1_3_Short  underlineThickness;
	FT2_1_3_ULong  isFixedPitch;
	FT2_1_3_ULong  minMemType42;
	FT2_1_3_ULong  maxMemType42;
	FT2_1_3_ULong  minMemType1;
	FT2_1_3_ULong  maxMemType1;

	/* Glyph names follow in the file, but we don't   */
	/* load them by default.  See the ttpost.c file.  */

} TT_Postscript;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    TT_PCLT                                                            */
/*                                                                       */
/* <Description>                                                         */
/*    A structure used to model a TrueType PCLT table.  All fields       */
/*    comply to the TrueType table.                                      */
/*                                                                       */
typedef struct  TT_PCLT_ {
	FT2_1_3_Fixed   Version;
	FT2_1_3_ULong   FontNumber;
	FT2_1_3_UShort  Pitch;
	FT2_1_3_UShort  xHeight;
	FT2_1_3_UShort  Style;
	FT2_1_3_UShort  TypeFamily;
	FT2_1_3_UShort  CapHeight;
	FT2_1_3_UShort  SymbolSet;
	FT2_1_3_Char    TypeFace[16];
	FT2_1_3_Char    CharacterComplement[8];
	FT2_1_3_Char    FileName[6];
	FT2_1_3_Char    StrokeWeight;
	FT2_1_3_Char    WidthType;
	FT2_1_3_Byte    SerifStyle;
	FT2_1_3_Byte    Reserved;

} TT_PCLT;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    TT_MaxProfile                                                      */
/*                                                                       */
/* <Description>                                                         */
/*    The maximum profile is a table containing many max values which    */
/*    can be used to pre-allocate arrays.  This ensures that no memory   */
/*    allocation occurs during a glyph load.                             */
/*                                                                       */
/* <Fields>                                                              */
/*    version               :: The version number.                       */
/*                                                                       */
/*    numGlyphs             :: The number of glyphs in this TrueType     */
/*                             font.                                     */
/*                                                                       */
/*    maxPoints             :: The maximum number of points in a         */
/*                             non-composite TrueType glyph.  See also   */
/*                             the structure element                     */
/*                             `maxCompositePoints'.                     */
/*                                                                       */
/*    maxContours           :: The maximum number of contours in a       */
/*                             non-composite TrueType glyph.  See also   */
/*                             the structure element                     */
/*                             `maxCompositeContours'.                   */
/*                                                                       */
/*    maxCompositePoints    :: The maximum number of points in a         */
/*                             composite TrueType glyph.  See also the   */
/*                             structure element `maxPoints'.            */
/*                                                                       */
/*    maxCompositeContours  :: The maximum number of contours in a       */
/*                             composite TrueType glyph.  See also the   */
/*                             structure element `maxContours'.          */
/*                                                                       */
/*    maxZones              :: The maximum number of zones used for      */
/*                             glyph hinting.                            */
/*                                                                       */
/*    maxTwilightPoints     :: The maximum number of points in the       */
/*                             twilight zone used for glyph hinting.     */
/*                                                                       */
/*    maxStorage            :: The maximum number of elements in the     */
/*                             storage area used for glyph hinting.      */
/*                                                                       */
/*    maxFunctionDefs       :: The maximum number of function            */
/*                             definitions in the TrueType bytecode for  */
/*                             this font.                                */
/*                                                                       */
/*    maxInstructionDefs    :: The maximum number of instruction         */
/*                             definitions in the TrueType bytecode for  */
/*                             this font.                                */
/*                                                                       */
/*    maxStackElements      :: The maximum number of stack elements used */
/*                             during bytecode interpretation.           */
/*                                                                       */
/*    maxSizeOfInstructions :: The maximum number of TrueType opcodes    */
/*                             used for glyph hinting.                   */
/*                                                                       */
/*    maxComponentElements  :: An obscure value related to composite     */
/*                             glyphs definitions.                       */
/*                                                                       */
/*    maxComponentDepth     :: An obscure value related to composite     */
/*                             glyphs definitions.  Probably the maximum */
/*                             number of simple glyphs in a composite.   */
/*                                                                       */
/* <Note>                                                                */
/*    This structure is only used during font loading.                   */
/*                                                                       */
typedef struct  TT_MaxProfile_ {
	FT2_1_3_Fixed   version;
	FT2_1_3_UShort  numGlyphs;
	FT2_1_3_UShort  maxPoints;
	FT2_1_3_UShort  maxContours;
	FT2_1_3_UShort  maxCompositePoints;
	FT2_1_3_UShort  maxCompositeContours;
	FT2_1_3_UShort  maxZones;
	FT2_1_3_UShort  maxTwilightPoints;
	FT2_1_3_UShort  maxStorage;
	FT2_1_3_UShort  maxFunctionDefs;
	FT2_1_3_UShort  maxInstructionDefs;
	FT2_1_3_UShort  maxStackElements;
	FT2_1_3_UShort  maxSizeOfInstructions;
	FT2_1_3_UShort  maxComponentElements;
	FT2_1_3_UShort  maxComponentDepth;

} TT_MaxProfile;


/* */

typedef enum {
	ft_sfnt_head = 0,
	ft_sfnt_maxp = 1,
	ft_sfnt_os2  = 2,
	ft_sfnt_hhea = 3,
	ft_sfnt_vhea = 4,
	ft_sfnt_post = 5,
	ft_sfnt_pclt = 6,

	sfnt_max   /* don't remove */

} FT2_1_3_Sfnt_Tag;


/* internal use only */
typedef void*
(*FT2_1_3_Get_Sfnt_Table_Func)( FT2_1_3_Face      face,
                           FT2_1_3_Sfnt_Tag  tag );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_Get_Sfnt_Table                                                  */
/*                                                                       */
/* <Description>                                                         */
/*    Returns a pointer to a given SFNT table within a face.             */
/*                                                                       */
/* <Input>                                                               */
/*    face :: A handle to the source.                                    */
/*                                                                       */
/*    tag  :: The index of the SFNT table.                               */
/*                                                                       */
/* <Return>                                                              */
/*    A type-less pointer to the table.  This will be 0 in case of       */
/*    error, or if the corresponding table was not found *OR* loaded     */
/*    from the file.                                                     */
/*                                                                       */
/* <Note>                                                                */
/*    The table is owned by the face object and disappears with it.      */
/*                                                                       */
/*    This function is only useful to access SFNT tables that are loaded */
/*    by the sfnt/truetype/opentype drivers.  See FT2_1_3_Sfnt_Tag for a      */
/*    list.                                                              */
/*                                                                       */
FT2_1_3_EXPORT( void* )
FT2_1_3_Get_Sfnt_Table( FT2_1_3_Face      face,
                   FT2_1_3_Sfnt_Tag  tag );


/* */


FT2_1_3_END_HEADER

#endif /* __TTTABLES_H__ */


/* END */
