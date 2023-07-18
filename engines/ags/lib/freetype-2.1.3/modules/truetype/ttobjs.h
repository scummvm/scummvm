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
/*  ttobjs.h                                                               */
/*    Objects manager (specification).                                     */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_TTOBJS_H
#define AGS_LIB_FREETYPE_TTOBJS_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"
#include "engines/ags/lib/freetype-2.1.3/tttypes.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


typedef struct TT_DriverRec_ *TT_Driver;
typedef struct TT_SizeRec_ *TT_Size;

typedef FT_GlyphSlot TT_GlyphSlot;

typedef struct TT_GraphicsState_ {
	FT_UShort      rp0;
	FT_UShort      rp1;
	FT_UShort      rp2;

	FT_UnitVector  dualVector;
	FT_UnitVector  projVector;
	FT_UnitVector  freeVector;

	FT_Long        loop;
	FT_F26Dot6     minimum_distance;
	FT_Int         round_state;

	FT_Bool        auto_flip;
	FT_F26Dot6     control_value_cutin;
	FT_F26Dot6     single_width_cutin;
	FT_F26Dot6     single_width_value;
	FT_Short       delta_base;
	FT_Short       delta_shift;

	FT_Byte        instruct_control;
	FT_Bool        scan_control;
	FT_Int         scan_type;

	FT_UShort      gep0;
	FT_UShort      gep1;
	FT_UShort      gep2;
} TT_GraphicsState;


#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER

FT2_1_3_LOCAL(void)
tt_glyphzone_done(TT_GlyphZone zone);

FT2_1_3_LOCAL(FT_Error)
tt_glyphzone_new(FT_Memory memory, FT_UShort maxPoints, FT_Short maxContours, TT_GlyphZone zone);

#endif /* TT_CONFIG_OPTION_BYTECODE_INTERPRETER */


/**** EXECUTION SUBTABLES ****/

#define TT_MAX_CODE_RANGES 3

typedef enum TT_CodeRange_Tag_ {
	tt_coderange_none = 0,
	tt_coderange_font,
	tt_coderange_cvt,
	tt_coderange_glyph
} TT_CodeRange_Tag;

typedef struct TT_CodeRange_ {
	FT_Byte *base;
	FT_ULong size;
} TT_CodeRange;

typedef TT_CodeRange TT_CodeRangeTable[TT_MAX_CODE_RANGES];

typedef struct TT_DefRecord_ {
	FT_Int   range;      /* in which code range is it located? */
	FT_Long  start;      /* where does it start?               */
	FT_UInt  opc;        /* function #, or instruction code    */
	FT_Bool  active;     /* is it active?                      */
} TT_DefRecord, *TT_DefArray;

typedef struct TT_Transform_ {
	FT_Fixed    xx, xy;     /* transformation matrix coefficients */
	FT_Fixed    yx, yy;
	FT_F26Dot6  ox, oy;     /* offsets */
} TT_Transform;

typedef struct  TT_SubglyphRec_ {
	FT_Long          index;        /* subglyph index; initialized with -1 */
	FT_Bool          is_scaled;    /* is the subglyph scaled?             */
	FT_Bool          is_hinted;    /* should it be hinted?                */
	FT_Bool          preserve_pps; /* preserve phantom points?            */

	FT_Long          file_offset;

	FT_BBox          bbox;
	FT_Pos           left_bearing;
	FT_Pos           advance;

	TT_GlyphZoneRec  zone;

	FT_Long          arg1;         /* first argument                      */
	FT_Long          arg2;         /* second argument                     */

	FT_UShort        element_flag; /* current load element flag           */

	TT_Transform     transform;    /* transformation matrix               */

	FT_Vector        pp1, pp2;     /* phantom points                      */
} TT_SubGlyphRec, *TT_SubGlyph_Stack;

typedef struct  TT_Size_Metrics_ {
	/* for non-square pixels */
	FT_Long     x_ratio;
	FT_Long     y_ratio;

	FT_UShort   ppem;               /* maximum ppem size              */
	FT_Long     ratio;              /* current ratio                  */
	FT_Fixed    scale;

	FT_F26Dot6  compensations[4];   /* device-specific compensations  */

	FT_Bool     valid;

	FT_Bool     rotated;            /* `is the glyph rotated?'-flag   */
	FT_Bool     stretched;          /* `is the glyph stretched?'-flag */
} TT_Size_Metrics;

typedef struct  TT_SizeRec_ {
	FT_SizeRec         root;

	TT_Size_Metrics    ttmetrics;

#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS

	FT_UInt            strike_index;    /* 0xFFFF to indicate invalid */
	FT_Size_Metrics    strike_metrics;  /* current strike's metrics   */

#endif

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER

	FT_UInt            num_function_defs; /* number of function definitions */
	FT_UInt            max_function_defs;
	TT_DefArray        function_defs;     /* table of function definitions  */

	FT_UInt            num_instruction_defs;  /* number of ins. definitions */
	FT_UInt            max_instruction_defs;
	TT_DefArray        instruction_defs;      /* table of ins. definitions  */

	FT_UInt            max_func;
	FT_UInt            max_ins;

	TT_CodeRangeTable  codeRangeTable;

	TT_GraphicsState   GS;

	FT_ULong           cvt_size;      /* the scaled control value table */
	FT_Long            *cvt;

	FT_UShort          storage_size; /* The storage area is now part of */
	FT_Long            *storage;      /* the instance                    */

	TT_GlyphZoneRec    twilight;     /* The instance's twilight zone    */

	/* debugging variables */

	/* When using the debugger, we must keep the */
	/* execution context tied to the instance    */
	/* object rather than asking it on demand.   */

	FT_Bool            debug;
	TT_ExecContext     context;

#endif /* TT_CONFIG_OPTION_BYTECODE_INTERPRETER */
} TT_SizeRec;

typedef struct  TT_DriverRec_ {
	FT_DriverRec     root;
	TT_ExecContext   context;  /* execution context        */
	TT_GlyphZoneRec  zone;     /* glyph loader points zone */

	void             *extension_component;
} TT_DriverRec;


/* Face functions */

FT2_1_3_LOCAL(FT_Error)
tt_face_init(FT_Stream stream, TT_Face face, FT_Int face_index, FT_Int num_params, FT_Parameter *params);

FT2_1_3_LOCAL(void)
tt_face_done(TT_Face face);


/* Size functions */

FT2_1_3_LOCAL(FT_Error)
tt_size_init(TT_Size size);

FT2_1_3_LOCAL(void)
tt_size_done(TT_Size size);

FT2_1_3_LOCAL(FT_Error)
tt_size_reset(TT_Size size);


/* Driver functions */

FT2_1_3_LOCAL(FT_Error)
tt_driver_init(TT_Driver driver);

FT2_1_3_LOCAL(void)
tt_driver_done(TT_Driver driver);


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_TTOBJS_H */
