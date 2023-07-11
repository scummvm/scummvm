/***************************************************************************/
/*                                                                         */
/*  ftmoderr.h                                                             */
/*                                                                         */
/*    FreeType module error offsets (specification).                       */
/*                                                                         */
/*  Copyright 2001, 2002 by                                                */
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
/* This file is used to define the FreeType module error offsets.        */
/*                                                                       */
/* The lower byte gives the error code, the higher byte gives the        */
/* module.  The base module has error offset 0.  For example, the error  */
/* `FT2_1_3_Err_Invalid_File_Format' has value 0x003, the error               */
/* `TT_Err_Invalid_File_Format' has value 0xB03, the error               */
/* `T1_Err_Invalid_File_Format' has value 0xC03, etc.                    */
/*                                                                       */
/* Undefine the macro FT2_1_3_CONFIG_OPTION_USE_MODULE_ERRORS in ftoption.h   */
/* to make the higher byte always zero (disabling the module error       */
/* mechanism).                                                           */
/*                                                                       */
/* It can also be used to create a module error message table easily     */
/* with something like                                                   */
/*                                                                       */
/*   {                                                                   */
/*     #undef __FTMODERR_H__                                             */
/*     #define FT2_1_3_MODERRDEF( e, v, s )  { FT2_1_3_Mod_Err_ ## e, s },         */
/*     #define FT2_1_3_MODERR_START_LIST     {                                */
/*     #define FT2_1_3_MODERR_END_LIST       { 0, 0 } };                      */
/*                                                                       */
/*     const struct                                                      */
/*     {                                                                 */
/*       int          mod_err_offset;                                    */
/*       const char*  mod_err_msg                                        */
/*     } ft_mod_errors[] =                                               */
/*                                                                       */
/*     #include "engines/ags/lib/freetype-2.1.3/ftmoderr.h"                                       */
/*   }                                                                   */
/*                                                                       */
/* To use such a table, all errors must be ANDed with 0xFF00 to remove   */
/* the error code.                                                       */
/*                                                                       */
/*************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTMODERR_H
#define AGS_LIB_FREETYPE_FTMODERR_H


/*******************************************************************/
/*******************************************************************/
/*****                                                         *****/
/*****                       SETUP MACROS                      *****/
/*****                                                         *****/
/*******************************************************************/
/*******************************************************************/


#undef  FT2_1_3_NEED_EXTERN_C

#ifndef FT2_1_3_MODERRDEF

#ifdef FT2_1_3_CONFIG_OPTION_USE_MODULE_ERRORS
#define FT2_1_3_MODERRDEF( e, v, s )  FT2_1_3_Mod_Err_ ## e = v,
#else
#define FT2_1_3_MODERRDEF( e, v, s )  FT2_1_3_Mod_Err_ ## e = 0,
#endif

#define FT2_1_3_MODERR_START_LIST  enum {
#define FT2_1_3_MODERR_END_LIST    FT_Mod_Err_Max };

#ifdef __cplusplus
#define FT2_1_3_NEED_EXTERN_C
extern "C" {
#endif

#endif /* !FT2_1_3_MODERRDEF */


/*******************************************************************/
/*******************************************************************/
/*****                                                         *****/
/*****               LIST MODULE ERROR BASES                   *****/
/*****                                                         *****/
/*******************************************************************/
/*******************************************************************/


#ifdef FT2_1_3_MODERR_START_LIST
FT2_1_3_MODERR_START_LIST
#endif


FT2_1_3_MODERRDEF( Base,     0x000, "base module" )
FT2_1_3_MODERRDEF( Autohint, 0x100, "autohinter module" )
FT2_1_3_MODERRDEF( Cache,    0x200, "cache module" )
FT2_1_3_MODERRDEF( CFF,      0x300, "CFF module" )
FT2_1_3_MODERRDEF( CID,      0x400, "CID module" )
FT2_1_3_MODERRDEF( PCF,      0x500, "PCF module" )
FT2_1_3_MODERRDEF( PSaux,    0x600, "PS auxiliary module" )
FT2_1_3_MODERRDEF( PSnames,  0x700, "PS names module" )
FT2_1_3_MODERRDEF( Raster,   0x800, "raster module" )
FT2_1_3_MODERRDEF( SFNT,     0x900, "SFNT module" )
FT2_1_3_MODERRDEF( Smooth,   0xA00, "smooth raster module" )
FT2_1_3_MODERRDEF( TrueType, 0xB00, "TrueType module" )
FT2_1_3_MODERRDEF( Type1,    0xC00, "Type 1 module" )
FT2_1_3_MODERRDEF( Winfonts, 0xD00, "Windows FON/FNT module" )
FT2_1_3_MODERRDEF( PFR,      0xE00, "PFR module" )


#ifdef FT2_1_3_MODERR_END_LIST
FT2_1_3_MODERR_END_LIST
#endif


/*******************************************************************/
/*******************************************************************/
/*****                                                         *****/
/*****                      CLEANUP                            *****/
/*****                                                         *****/
/*******************************************************************/
/*******************************************************************/


#ifdef FT2_1_3_NEED_EXTERN_C
}
#endif

#undef FT2_1_3_MODERR_START_LIST
#undef FT2_1_3_MODERR_END_LIST
#undef FT2_1_3_MODERRDEF
#undef FT2_1_3_NEED_EXTERN_C


#endif /* AGS_LIB_FREETYPE_FTMODERR_H */


/* END */
