/***************************************************************************/
/*                                                                         */
/*  fterrors.h                                                             */
/*                                                                         */
/*    FreeType error code handling (specification).                        */
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


/*************************************************************************/
/*                                                                       */
/* This special header file is used to define the handling of FT2        */
/* enumeration constants.  It can also be used to generate error message */
/* strings with a small macro trick explained below.                     */
/*                                                                       */
/* I - Error Formats                                                     */
/* -----------------                                                     */
/*                                                                       */
/*   Since release 2.1, the error constants have changed.  The lower     */
/*   byte of the error value gives the "generic" error code, while the   */
/*   higher byte indicates in which module the error occurred.           */
/*                                                                       */
/*   You can use the macro FT2_1_3_ERROR_BASE(x) macro to extract the generic */
/*   error code from an FT2_1_3_Error value.                                  */
/*                                                                       */
/*   The configuration macro FT2_1_3_CONFIG_OPTION_USE_MODULE_ERRORS can be   */
/*   undefined in ftoption.h in order to make the higher byte always     */
/*   zero, in case you need to be compatible with previous versions of   */
/*   FreeType 2.                                                         */
/*                                                                       */
/*                                                                       */
/* II - Error Message strings                                            */
/* --------------------------                                            */
/*                                                                       */
/*   The error definitions below are made through special macros that    */
/*   allow client applications to build a table of error message strings */
/*   if they need it.  The strings are not included in a normal build of */
/*   FreeType 2 to save space (most client applications do not use       */
/*   them).                                                              */
/*                                                                       */
/*   To do so, you have to define the following macros before including  */
/*   this file:                                                          */
/*                                                                       */
/*   FT2_1_3_ERROR_START_LIST ::                                              */
/*     This macro is called before anything else to define the start of  */
/*     the error list.  It is followed by several FT2_1_3_ERROR_DEF calls     */
/*     (see below).                                                      */
/*                                                                       */
/*   FT2_1_3_ERROR_DEF( e, v, s ) ::                                          */
/*     This macro is called to define one single error.                  */
/*     `e' is the error code identifier (e.g. FT2_1_3_Err_Invalid_Argument).  */
/*     `v' is the error numerical value.                                 */
/*     `s' is the corresponding error string.                            */
/*                                                                       */
/*   FT2_1_3_ERROR_END_LIST ::                                                */
/*     This macro ends the list.                                         */
/*                                                                       */
/*   Additionally, you have to undefine __FTERRORS_H__ before #including */
/*   this file.                                                          */
/*                                                                       */
/*   Here is a simple example:                                           */
/*                                                                       */
/*     {                                                                 */
/*       #undef __FTERRORS_H__                                           */
/*       #define FT2_1_3_ERRORDEF( e, v, s )  { e, s },                       */
/*       #define FT2_1_3_ERROR_START_LIST     {                               */
/*       #define FT2_1_3_ERROR_END_LIST       { 0, 0 } };                     */
/*                                                                       */
/*       const struct                                                    */
/*       {                                                               */
/*         int          err_code;                                        */
/*         const char*  err_msg                                          */
/*       } ft_errors[] =                                                 */
/*                                                                       */
/*       #include "engines/ags/lib/freetype-2.1.3/include/freetype/fterrors.h"                                            */
/*     }                                                                 */
/*                                                                       */
/*************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTERRORS_H
#define AGS_LIB_FREETYPE_FTERRORS_H


/* include module base error codes */
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftmoderr.h"


/*******************************************************************/
/*******************************************************************/
/*****                                                         *****/
/*****                       SETUP MACROS                      *****/
/*****                                                         *****/
/*******************************************************************/
/*******************************************************************/


#undef  FT2_1_3_NEED_EXTERN_C

#undef  FT2_1_3_ERR_XCAT
#undef  FT2_1_3_ERR_CAT

#define FT2_1_3_ERR_XCAT( x, y )  x ## y
#define FT2_1_3_ERR_CAT( x, y )   FT2_1_3_ERR_XCAT( x, y )


/* FT2_1_3_ERR_PREFIX is used as a prefix for error identifiers. */
/* By default, we use `FT2_1_3_Err_'.                            */
/*                                                          */
#ifndef FT2_1_3_ERR_PREFIX
#define FT2_1_3_ERR_PREFIX  FT2_1_3_Err_
#endif


/* FT2_1_3_ERR_BASE is used as the base for module-specific errors. */
/*                                                             */
#ifdef FT2_1_3_CONFIG_OPTION_USE_MODULE_ERRORS

#ifndef FT2_1_3_ERR_BASE
#define FT2_1_3_ERR_BASE  FT2_1_3_Mod_Err_Base
#endif

#else

#undef FT2_1_3_ERR_BASE
#define FT2_1_3_ERR_BASE  0

#endif /* FT2_1_3_CONFIG_OPTION_USE_MODULE_ERRORS */


/* If FT2_1_3_ERRORDEF is not defined, we need to define a simple */
/* enumeration type.                                         */
/*                                                           */
#ifndef FT2_1_3_ERRORDEF

#define FT2_1_3_ERRORDEF( e, v, s )  e = v,
#define FT2_1_3_ERROR_START_LIST     enum {
#define FT2_1_3_ERROR_END_LIST       FT2_1_3_ERR_CAT( FT2_1_3_ERR_PREFIX, Max ) };

#ifdef __cplusplus
#define FT2_1_3_NEED_EXTERN_C
extern "C" {
#endif

#endif /* !FT2_1_3_ERRORDEF */


/* this macro is used to define an error */
#define FT2_1_3_ERRORDEF_( e, v, s )   \
		  FT2_1_3_ERRORDEF( FT2_1_3_ERR_CAT( FT2_1_3_ERR_PREFIX, e ), v + FT2_1_3_ERR_BASE, s )

/* this is only used for FT2_1_3_Err_Ok, which must be 0! */
#define FT2_1_3_NOERRORDEF_( e, v, s ) \
		  FT2_1_3_ERRORDEF( FT2_1_3_ERR_CAT( FT2_1_3_ERR_PREFIX, e ), v, s )


#ifdef FT2_1_3_ERROR_START_LIST
FT2_1_3_ERROR_START_LIST
#endif


/* no include the error codes */
#include "engines/ags/lib/freetype-2.1.3/include/freetype/fterrdef.h"


#ifdef FT2_1_3_ERROR_END_LIST
FT2_1_3_ERROR_END_LIST
#endif


/*******************************************************************/
/*******************************************************************/
/*****                                                         *****/
/*****                      SIMPLE CLEANUP                     *****/
/*****                                                         *****/
/*******************************************************************/
/*******************************************************************/

#ifdef FT2_1_3_NEED_EXTERN_C
}
#endif

#undef FT2_1_3_ERROR_START_LIST
#undef FT2_1_3_ERROR_END_LIST

#undef FT2_1_3_ERRORDEF
#undef FT2_1_3_ERRORDEF_
#undef FT2_1_3_NOERRORDEF_

#undef FT2_1_3_NEED_EXTERN_C
#undef FT2_1_3_ERR_PREFIX
#undef FT2_1_3_ERR_BASE
#undef FT2_1_3_ERR_CONCAT

#endif /* AGS_LIB_FREETYPE_FTERRORS_H */


/* END */
