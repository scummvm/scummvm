/***************************************************************************/
/*                                                                         */
/*  ftdebug.h                                                              */
/*                                                                         */
/*    Debugging and logging component (specification).                     */
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
/*                                                                         */
/*  IMPORTANT: A description of FreeType's debugging support can be        */
/*             found in "docs/DEBUG.TXT".  Read it if you need to use or   */
/*             understand this code.                                       */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTDEBUG_H
#define AGS_LIB_FREETYPE_FTDEBUG_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/config/ftconfig.h"


FT2_1_3_BEGIN_HEADER


/* force the definition of FT2_1_3_DEBUG_LEVEL_ERROR if FT2_1_3_DEBUG_LEVEL_TRACE */
/* is already defined; this simplifies the following #ifdefs            */
/*                                                                      */
#ifdef FT2_1_3_DEBUG_LEVEL_TRACE
#undef  FT2_1_3_DEBUG_LEVEL_ERROR
#define FT2_1_3_DEBUG_LEVEL_ERROR
#endif


/*************************************************************************/
/*                                                                       */
/* Define the trace enums as well as the trace levels array when they    */
/* are needed.                                                           */
/*                                                                       */
/*************************************************************************/

#ifdef FT2_1_3_DEBUG_LEVEL_TRACE

#define FT2_1_3_TRACE_DEF( x )  trace_ ## x ,

/* defining the enumeration */
typedef enum {
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/fttrace.h"
	trace_count

} FT2_1_3_Trace;


/* defining the array of trace levels, provided by `src/base/ftdebug.c' */
extern int  FT2_1_3_trace_levels[trace_count];

#undef FT2_1_3_TRACE_DEF

#endif /* FT2_1_3_DEBUG_LEVEL_TRACE */


/*************************************************************************/
/*                                                                       */
/* Define the FT2_1_3_TRACE macro                                             */
/*                                                                       */
/* IMPORTANT!                                                            */
/*                                                                       */
/* Each component must define the macro FT2_1_3_COMPONENT to a valid FT2_1_3_Trace */
/* value before using any TRACE macro.                                   */
/*                                                                       */
/*************************************************************************/

#ifdef FT2_1_3_DEBUG_LEVEL_TRACE

#define FT2_1_3_TRACE(level, varformat)                       \
	do {                                                      \
		if (FT2_1_3_trace_levels[FT2_1_3_COMPONENT] >= level) \
			FT_Message varformat;                        \
	} while (0)

#else /* !FT2_1_3_DEBUG_LEVEL_TRACE */

#define FT2_1_3_TRACE( level, varformat )  do ; while ( 0 )      /* nothing */

#endif /* !FT2_1_3_DEBUG_LEVEL_TRACE */


/*************************************************************************/
/*                                                                       */
/* You need two opening resp. closing parentheses!                       */
/*                                                                       */
/* Example: FT2_1_3_TRACE0(( "Value is %i", foo ))                            */
/*                                                                       */
/*************************************************************************/

#define FT2_1_3_TRACE0( varformat )  FT2_1_3_TRACE( 0, varformat )
#define FT2_1_3_TRACE1( varformat )  FT2_1_3_TRACE( 1, varformat )
#define FT2_1_3_TRACE2( varformat )  FT2_1_3_TRACE( 2, varformat )
#define FT2_1_3_TRACE3( varformat )  FT2_1_3_TRACE( 3, varformat )
#define FT2_1_3_TRACE4( varformat )  FT2_1_3_TRACE( 4, varformat )
#define FT2_1_3_TRACE5( varformat )  FT2_1_3_TRACE( 5, varformat )
#define FT2_1_3_TRACE6( varformat )  FT2_1_3_TRACE( 6, varformat )
#define FT2_1_3_TRACE7( varformat )  FT2_1_3_TRACE( 7, varformat )


/*************************************************************************/
/*                                                                       */
/*  Define the FT2_1_3_ERROR macro                                            */
/*                                                                       */
/*************************************************************************/

#ifdef FT2_1_3_DEBUG_LEVEL_ERROR

#define FT2_1_3_ERROR( varformat )  FT_Message  varformat

#else  /* !FT2_1_3_DEBUG_LEVEL_ERROR */

#define FT2_1_3_ERROR( varformat )  do ; while ( 0 )      /* nothing */

#endif /* !FT2_1_3_DEBUG_LEVEL_ERROR */


/*************************************************************************/
/*                                                                       */
/* Define the FT2_1_3_ASSERT macro                                            */
/*                                                                       */
/*************************************************************************/

#ifdef FT2_1_3_DEBUG_LEVEL_ERROR

#define FT2_1_3_ASSERT(condition)                                     \
	do {                                                              \
		if (!(condition))                                             \
			FT_Panic("assertion failed on line %d of file %s\n", \
						  __LINE__, __FILE__);                        \
	} while (0)

#else /* !FT2_1_3_DEBUG_LEVEL_ERROR */

#define FT2_1_3_ASSERT( condition )  do ; while ( 0 )

#endif /* !FT2_1_3_DEBUG_LEVEL_ERROR */


/*************************************************************************/
/*                                                                       */
/*  Define 'FT_Message' and 'FT_Panic' when needed                       */
/*                                                                       */
/*************************************************************************/

#ifdef FT2_1_3_DEBUG_LEVEL_ERROR

#include "stdio.h"  /* for vprintf() */

/* print a message */
FT2_1_3_EXPORT( void )
FT_Message( const char*  fmt, ... );

/* print a message and exit */
FT2_1_3_EXPORT( void )
FT_Panic( const char*  fmt, ... );

#endif /* FT2_1_3_DEBUG_LEVEL_ERROR */


FT2_1_3_BASE( void )
ft_debug_init( void );


#if defined( _MSC_VER )      /* Visual C++ (and Intel C++) */

/* we disable the warning `conditional expression is constant' here */
/* in order to compile cleanly with the maximum level of warnings   */
#pragma warning( disable : 4127 )

#endif /* _MSC_VER */


FT2_1_3_END_HEADER

#endif /* AGS_LIB_FREETYPE_FTDEBUG_H */


/* END */
