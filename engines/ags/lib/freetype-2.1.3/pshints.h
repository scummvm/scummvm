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
/*  pshints.h                                                              */
/*    Interface to Postscript-specific (Type 1 and Type 2) hints           */
/*    recorders (specification only).  These are used to support native    */
/*    T1/T2 hints in the "type1", "cid" and "cff" font drivers.            */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_PSHINTS_H
#define AGS_LIB_FREETYPE_PSHINTS_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"
#include "engines/ags/lib/freetype-2.1.3/t1tables.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/**** INTERNAL REPRESENTATION OF GLOBALS ****/

typedef struct PSH_GlobalsRec_ *PSH_Globals;

typedef FT_Error (*PSH_Globals_NewFunc)(FT_Memory memory, T1_Private *private_dict, PSH_Globals *aglobals);

typedef FT_Error (*PSH_Globals_SetScaleFunc)(PSH_Globals globals, FT_Fixed x_scale, FT_Fixed y_scale, FT_Fixed x_delta, FT_Fixed y_delta);

typedef void (*PSH_Globals_DestroyFunc)(PSH_Globals globals);

typedef struct PSH_Globals_FuncsRec_ {
	PSH_Globals_NewFunc 	 create;
	PSH_Globals_SetScaleFunc set_scale;
	PSH_Globals_DestroyFunc  destroy;
} PSH_Globals_FuncsRec, *PSH_Globals_Funcs;


/**** PUBLIC TYPE 1 HINTS RECORDER ****/

typedef struct T1_HintsRec_*  T1_Hints;
typedef const struct T1_Hints_FuncsRec_*  T1_Hints_Funcs;

typedef void (*T1_Hints_OpenFunc)(T1_Hints hints);
typedef void (*T1_Hints_SetStemFunc)(T1_Hints hints, FT_UInt dimension, FT_Long *coords);
typedef void (*T1_Hints_SetStem3Func)(T1_Hints hints, FT_UInt dimension, FT_Long *coords);
typedef void (*T1_Hints_ResetFunc)(T1_Hints hints, FT_UInt end_point);

typedef FT_Error (*T1_Hints_CloseFunc)(T1_Hints hints, FT_UInt end_point);
typedef FT_Error (*T1_Hints_ApplyFunc)(T1_Hints hints, FT_Outline *outline, PSH_Globals globals, FT_Render_Mode hint_mode);

typedef struct  T1_Hints_FuncsRec_ {
	T1_Hints               hints;
	T1_Hints_OpenFunc      open;
	T1_Hints_CloseFunc     close;
	T1_Hints_SetStemFunc   stem;
	T1_Hints_SetStem3Func  stem3;
	T1_Hints_ResetFunc     reset;
	T1_Hints_ApplyFunc     apply;
} T1_Hints_FuncsRec;


/**** PUBLIC TYPE 2 HINTS RECORDER ****/

typedef struct T2_HintsRec_*  T2_Hints;
typedef const struct T2_Hints_FuncsRec_*  T2_Hints_Funcs;

typedef void (*T2_Hints_OpenFunc) (T2_Hints hints);
typedef void (*T2_Hints_StemsFunc)(T2_Hints hints, FT_UInt dimension, FT_UInt count, FT_Fixed *coordinates);
typedef void (*T2_Hints_MaskFunc)(T2_Hints hints, FT_UInt end_point, FT_UInt bit_count, const FT_Byte *bytes);
typedef void (*T2_Hints_CounterFunc)(T2_Hints hints, FT_UInt bit_count, const FT_Byte *bytes);

typedef FT_Error (*T2_Hints_CloseFunc)(T2_Hints hints, FT_UInt end_point);
typedef FT_Error (*T2_Hints_ApplyFunc)(T2_Hints hints, FT_Outline *outline, PSH_Globals globals, FT_Render_Mode hint_mode);

typedef struct  T2_Hints_FuncsRec_ {
	T2_Hints              hints;
	T2_Hints_OpenFunc     open;
	T2_Hints_CloseFunc    close;
	T2_Hints_StemsFunc    stems;
	T2_Hints_MaskFunc     hintmask;
	T2_Hints_CounterFunc  counter;
	T2_Hints_ApplyFunc    apply;
} T2_Hints_FuncsRec;

typedef struct PSHinter_Interface_ {
	PSH_Globals_Funcs (*get_globals_funcs)(FT_Module module);
	T1_Hints_Funcs 	  (*get_t1_funcs)(FT_Module module);
	T2_Hints_Funcs 	  (*get_t2_funcs)(FT_Module module);
} PSHinter_Interface;

typedef PSHinter_Interface *PSHinter_Service;


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_PSHINTS_H */
