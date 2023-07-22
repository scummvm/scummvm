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
/*  fterrors.h                                                             */
/*    FreeType error code handling (specification).                        */
/*                                                                         */
/***************************************************************************/

#ifndef AGS_LIB_FREETYPE_FTERRORS_H
#define AGS_LIB_FREETYPE_FTERRORS_H

/* include module base error codes */
#include "engines/ags/lib/freetype-2.1.3/ftmoderr.h"


/**** SETUP MACROS ****/

#undef FT2_1_3_NEED_EXTERN_C

#undef FT_ERR_XCAT
#undef FT_ERR_CAT

#define FT_ERR_XCAT(x, y) x##y
#define FT_ERR_CAT(x, y) FT_ERR_XCAT(x, y)

/* FT_ERR_PREFIX is used as a prefix for error identifiers. */
/* By default, we use `FT2_1_3_Err_'.                            */
#ifndef FT_ERR_PREFIX
#define FT_ERR_PREFIX FT2_1_3_Err_
#endif

/* FT_ERR_BASE is used as the base for module-specific errors. */
#ifdef FT_CONFIG_OPTION_USE_MODULE_ERRORS

#ifndef FT_ERR_BASE
#define FT_ERR_BASE FT2_1_3_Mod_Err_Base
#endif

#else

#undef FT_ERR_BASE
#define FT_ERR_BASE 0

#endif /* FT_CONFIG_OPTION_USE_MODULE_ERRORS */

/* If FT_ERRORDEF is not defined, we need to define a simple */
#ifndef FT_ERRORDEF

#define FT_ERRORDEF(e, v, s) e = v,
#define FT_ERROR_START_LIST enum {

#define FT_ERROR_END_LIST       FT_ERR_CAT( FT_ERR_PREFIX, Max ) };

#ifdef __cplusplus
#define FT2_1_3_NEED_EXTERN_C
extern "C" {
#endif

#endif /* !FT_ERRORDEF */

/* this macro is used to define an error */
#define FT_ERRORDEF_(e, v, s) \
	FT_ERRORDEF(FT_ERR_CAT(FT_ERR_PREFIX, e), v + FT_ERR_BASE, s)

/* this is only used for FT2_1_3_Err_Ok, which must be 0! */
#define FT2_1_3_NOERRORDEF_(e, v, s) \
	FT_ERRORDEF(FT_ERR_CAT(FT_ERR_PREFIX, e), v, s)

#ifdef FT_ERROR_START_LIST
FT_ERROR_START_LIST
#endif

/* now include the error codes */
#include "engines/ags/lib/freetype-2.1.3/fterrdef.h"

#ifdef FT_ERROR_END_LIST
FT_ERROR_END_LIST
#endif


/* SIMPLE CLEANUP */

#ifdef FT2_1_3_NEED_EXTERN_C
}
#endif

#undef FT_ERROR_START_LIST
#undef FT_ERROR_END_LIST

#undef FT_ERRORDEF
#undef FT_ERRORDEF_
#undef FT2_1_3_NOERRORDEF_

#undef FT2_1_3_NEED_EXTERN_C
#undef FT_ERR_PREFIX
#undef FT_ERR_BASE
#undef FT_ERR_CONCAT

#endif /* AGS_LIB_FREETYPE_FTERRORS_H */
