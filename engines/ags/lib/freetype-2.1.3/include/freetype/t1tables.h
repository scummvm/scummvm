/***************************************************************************/
/*                                                                         */
/*  t1tables.h                                                             */
/*                                                                         */
/*    Basic Type 1/Type 2 tables definitions and interface (specification  */
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


#ifndef __T1TABLES_H__
#define __T1TABLES_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_FREETYPE_H


FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    type1_tables                                                       */
/*                                                                       */
/* <Title>                                                               */
/*    Type 1 Tables                                                      */
/*                                                                       */
/* <Abstract>                                                            */
/*    Type 1 (PostScript) specific font tables.                          */
/*                                                                       */
/* <Description>                                                         */
/*    This section contains the definition of Type 1-specific tables,    */
/*    including structures related to other PostScript font formats.     */
/*                                                                       */
/*************************************************************************/


/* Note that we separate font data in PS_FontInfoRec and PS_PrivateRec */
/* structures in order to support Multiple Master fonts.               */


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    PS_FontInfoRec                                                     */
/*                                                                       */
/* <Description>                                                         */
/*    A structure used to model a Type1/Type2 FontInfo dictionary.  Note */
/*    that for Multiple Master fonts, each instance has its own          */
/*    FontInfo.                                                          */
/*                                                                       */
typedef struct  PS_FontInfoRec {
	FT2_1_3_String*  version;
	FT2_1_3_String*  notice;
	FT2_1_3_String*  full_name;
	FT2_1_3_String*  family_name;
	FT2_1_3_String*  weight;
	FT2_1_3_Long     italic_angle;
	FT2_1_3_Bool     is_fixed_pitch;
	FT2_1_3_Short    underline_position;
	FT2_1_3_UShort   underline_thickness;

} PS_FontInfoRec, *PS_FontInfo;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    T1_FontInfo                                                        */
/*                                                                       */
/* <Description>                                                         */
/*    This type is equivalent to @PS_FontInfoRec.  It is deprecated but  */
/*    kept to maintain source compatibility between various versions of  */
/*    FreeType.                                                          */
/*                                                                       */
typedef PS_FontInfoRec  T1_FontInfo;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    PS_PrivateRec                                                      */
/*                                                                       */
/* <Description>                                                         */
/*    A structure used to model a Type1/Type2 private dictionary.  Note  */
/*    that for Multiple Master fonts, each instance has its own Private  */
/*    dictionary.                                                        */
/*                                                                       */
typedef struct  PS_PrivateRec_ {
	FT2_1_3_Int     unique_id;
	FT2_1_3_Int     lenIV;

	FT2_1_3_Byte    num_blue_values;
	FT2_1_3_Byte    num_other_blues;
	FT2_1_3_Byte    num_family_blues;
	FT2_1_3_Byte    num_family_other_blues;

	FT2_1_3_Short   blue_values[14];
	FT2_1_3_Short   other_blues[10];

	FT2_1_3_Short   family_blues      [14];
	FT2_1_3_Short   family_other_blues[10];

	FT2_1_3_Fixed   blue_scale;
	FT2_1_3_Int     blue_shift;
	FT2_1_3_Int     blue_fuzz;

	FT2_1_3_UShort  standard_width[1];
	FT2_1_3_UShort  standard_height[1];

	FT2_1_3_Byte    num_snap_widths;
	FT2_1_3_Byte    num_snap_heights;
	FT2_1_3_Bool    force_bold;
	FT2_1_3_Bool    round_stem_up;

	FT2_1_3_Short   snap_widths [13];  /* including std width  */
	FT2_1_3_Short   snap_heights[13];  /* including std height */

	FT2_1_3_Long    language_group;
	FT2_1_3_Long    password;

	FT2_1_3_Short   min_feature[2];

} PS_PrivateRec, *PS_Private;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    T1_Private                                                         */
/*                                                                       */
/* <Description>                                                         */
/*   This type is equivalent to @PS_PrivateRec.  It is deprecated but    */
/*   kept to maintain source compatibility between various versions of   */
/*   FreeType.                                                           */
/*                                                                       */
typedef PS_PrivateRec  T1_Private;


/*************************************************************************/
/*                                                                       */
/* <Enum>                                                                */
/*    T1_Blend_Flags                                                     */
/*                                                                       */
/* <Description>                                                         */
/*    A set of flags used to indicate which fields are present in a      */
/*    given blen dictionary (font info or private).  Used to support     */
/*    Multiple Masters fonts.                                            */
/*                                                                       */
typedef enum {
	/*# required fields in a FontInfo blend dictionary */
	T1_BLEND_UNDERLINE_POSITION = 0,
	T1_BLEND_UNDERLINE_THICKNESS,
	T1_BLEND_ITALIC_ANGLE,

	/*# required fields in a Private blend dictionary */
	T1_BLEND_BLUE_VALUES,
	T1_BLEND_OTHER_BLUES,
	T1_BLEND_STANDARD_WIDTH,
	T1_BLEND_STANDARD_HEIGHT,
	T1_BLEND_STEM_SNAP_WIDTHS,
	T1_BLEND_STEM_SNAP_HEIGHTS,
	T1_BLEND_BLUE_SCALE,
	T1_BLEND_BLUE_SHIFT,
	T1_BLEND_FAMILY_BLUES,
	T1_BLEND_FAMILY_OTHER_BLUES,
	T1_BLEND_FORCE_BOLD,

	/*# never remove */
	T1_BLEND_MAX

} T1_Blend_Flags;


/*# backwards compatible definitions */
#define t1_blend_underline_position   T1_BLEND_UNDERLINE_POSITION
#define t1_blend_underline_thickness  T1_BLEND_UNDERLINE_THICKNESS
#define t1_blend_italic_angle         T1_BLEND_ITALIC_ANGLE
#define t1_blend_blue_values          T1_BLEND_BLUE_VALUES
#define t1_blend_other_blues          T1_BLEND_OTHER_BLUES
#define t1_blend_standard_widths      T1_BLEND_STANDARD_WIDTH
#define t1_blend_standard_height      T1_BLEND_STANDARD_HEIGHT
#define t1_blend_stem_snap_widths     T1_BLEND_STEM_SNAP_WIDTHS
#define t1_blend_stem_snap_heights    T1_BLEND_STEM_SNAP_HEIGHTS
#define t1_blend_blue_scale           T1_BLEND_BLUE_SCALE
#define t1_blend_blue_shift           T1_BLEND_BLUE_SHIFT
#define t1_blend_family_blues         T1_BLEND_FAMILY_BLUES
#define t1_blend_family_other_blues   T1_BLEND_FAMILY_OTHER_BLUES
#define t1_blend_force_bold           T1_BLEND_FORCE_BOLD
#define t1_blend_max                  T1_BLEND_MAX


/* maximum number of Multiple Masters designs, as defined in the spec */
#define T1_MAX_MM_DESIGNS     16

/* maximum number of Multiple Masters axes, as defined in the spec */
#define T1_MAX_MM_AXIS        4

/* maximum number of elements in a design map */
#define T1_MAX_MM_MAP_POINTS  20


/* this structure is used to store the BlendDesignMap entry for an axis */
typedef struct  PS_DesignMap_ {
	FT2_1_3_Byte    num_points;
	FT2_1_3_Fixed*  design_points;
	FT2_1_3_Fixed*  blend_points;

} PS_DesignMapRec, *PS_DesignMap;

/* backwards-compatible definition */
typedef PS_DesignMapRec  T1_DesignMap;


typedef struct  PS_BlendRec_ {
	FT2_1_3_UInt          num_designs;
	FT2_1_3_UInt          num_axis;

	FT2_1_3_String*       axis_names[T1_MAX_MM_AXIS];
	FT2_1_3_Fixed*        design_pos[T1_MAX_MM_DESIGNS];
	PS_DesignMapRec  design_map[T1_MAX_MM_AXIS];

	FT2_1_3_Fixed*        weight_vector;
	FT2_1_3_Fixed*        default_weight_vector;

	PS_FontInfo      font_infos[T1_MAX_MM_DESIGNS + 1];
	PS_Private       privates  [T1_MAX_MM_DESIGNS + 1];

	FT2_1_3_ULong         blend_bitflags;

	FT2_1_3_BBox*         bboxes    [T1_MAX_MM_DESIGNS + 1];

} PS_BlendRec, *PS_Blend;


/* backwards-compatible definition */
typedef PS_BlendRec  T1_Blend;


typedef struct  CID_FaceDictRec_ {
	PS_PrivateRec  private_dict;

	FT2_1_3_UInt        len_buildchar;
	FT2_1_3_Fixed       forcebold_threshold;
	FT2_1_3_Pos         stroke_width;
	FT2_1_3_Fixed       expansion_factor;

	FT2_1_3_Byte        paint_type;
	FT2_1_3_Byte        font_type;
	FT2_1_3_Matrix      font_matrix;
	FT2_1_3_Vector      font_offset;

	FT2_1_3_UInt        num_subrs;
	FT2_1_3_ULong       subrmap_offset;
	FT2_1_3_Int         sd_bytes;

} CID_FaceDictRec, *CID_FaceDict;


/* backwards-compatible definition */
typedef CID_FaceDictRec  CID_FontDict;


typedef struct  CID_FaceInfoRec_ {
	FT2_1_3_String*      cid_font_name;
	FT2_1_3_Fixed        cid_version;
	FT2_1_3_Int          cid_font_type;

	FT2_1_3_String*      registry;
	FT2_1_3_String*      ordering;
	FT2_1_3_Int          supplement;

	PS_FontInfoRec  font_info;
	FT2_1_3_BBox         font_bbox;
	FT2_1_3_ULong        uid_base;

	FT2_1_3_Int          num_xuid;
	FT2_1_3_ULong        xuid[16];

	FT2_1_3_ULong        cidmap_offset;
	FT2_1_3_Int          fd_bytes;
	FT2_1_3_Int          gd_bytes;
	FT2_1_3_ULong        cid_count;

	FT2_1_3_Int          num_dicts;
	CID_FaceDict    font_dicts;

	FT2_1_3_ULong        data_offset;

} CID_FaceInfoRec, *CID_FaceInfo;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    CID_Info                                                           */
/*                                                                       */
/* <Description>                                                         */
/*   This type is equivalent to @CID_FaceInfoRec. It is deprecated but   */
/*   kept to maintain source compatibility between various versions of   */
/*   FreeType.                                                           */
/*                                                                       */
typedef CID_FaceInfoRec  CID_Info;

/* */


/************************************************************************
 *
 * @function:
 *    FT2_1_3_Has_PS_Glyph_Names
 *
 * @description:
 *    Return true if a given face provides reliable Postscript glyph
 *    names.  This is similar to using the @FT2_1_3_HAS_GLYPH_NAMES macro,
 *    except that certain fonts (mostly TrueType) contain incorrect
 *    glyph name tables.
 *
 *    When this function returns true, the caller is sure that the glyph
 *    names returned by @FT2_1_3_Get_Glyph_Name are reliable.
 *
 * @input:
 *    face ::
 *       face handle
 *
 * @return:
 *    Boolean.  True if glyph names are reliable.
 */
FT2_1_3_EXPORT( FT2_1_3_Int )
FT2_1_3_Has_PS_Glyph_Names( FT2_1_3_Face  face );


/************************************************************************
 *
 * @function:
 *    FT2_1_3_Get_PS_Font_Info
 *
 * @description:
 *    Retrieve the @PS_FontInfoRec structure corresponding to a given
 *    Postscript font.
 *
 * @input:
 *    face ::
 *       Postscript face handle.
 *
 * @output:
 *    afont_info ::
 *       Output font info structure pointer.
 *
 * @return:
 *    FreeType error code.  0 means success.
 *
 * @note:
 *    The string pointers within the font info structure are owned by
 *    the face and don't need to be freed by the caller.
 *
 *    If the font's format is not Postscript-based, this function will
 *    return the @FT2_1_3_Err_Invalid_Argument error code.
 */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_Get_PS_Font_Info( FT2_1_3_Face          face,
                     PS_FontInfoRec  *afont_info );

/* */



FT2_1_3_END_HEADER

#endif /* __T1TABLES_H__ */


/* END */
