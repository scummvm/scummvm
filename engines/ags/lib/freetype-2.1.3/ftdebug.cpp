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
/*  ftdebug.c                                                              */
/*    Debugging and logging component (body).                              */
/*                                                                         */
/***************************************************************************/


/*************************************************************************/
/*                                                                       */
/* This component contains various macros and functions used to ease the */
/* debugging of the FreeType engine.  Its main purpose is in assertion   */
/* checking, tracing, and error detection.                               */
/*                                                                       */
/* There are now three debugging modes:                                  */
/*                                                                       */
/* - trace mode                                                          */
/*                                                                       */
/*   Error and trace messages are sent to the log file (which can be the */
/*   standard error output).                                             */
/*                                                                       */
/* - error mode                                                          */
/*                                                                       */
/*   Only error messages are generated.                                  */
/*                                                                       */
/* - release mode:                                                       */
/*                                                                       */
/*   No error message is sent or generated.  The code is free from any   */
/*   debugging parts.                                                    */
/*                                                                       */
/*************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"
#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"


#if defined(FT_DEBUG_LEVEL_ERROR)

FT_EXPORT_DEF(void)
FT_Message(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

FT_EXPORT_DEF(void)
FT_Panic(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	exit(EXIT_FAILURE);
}

#endif /* FT_DEBUG_LEVEL_ERROR */


#ifdef FT_DEBUG_LEVEL_TRACE

/* array of trace levels, initialized to 0 */
int ft_trace_levels[trace_count];

/* define array of trace toggle names */
#define FT_TRACE_DEF(x) #x,

static const char *ft_trace_toggles[trace_count + 1] = {
#include "engines/ags/lib/freetype-2.1.3/fttrace.h"
	NULL
};

#undef FT_TRACE_DEF


/*************************************************************************/
/*                                                                       */
/* Initialize the tracing sub-system.  This is done by retrieving the    */
/* value of the "FT2_DEBUG" environment variable.  It must be a list of  */
/* toggles, separated by spaces, `;' or `,'.  Example:                   */
/*                                                                       */
/*    "any:3 memory:6 stream:5"                                          */
/*                                                                       */
/* This will request that all levels be set to 3, except the trace level */
/* for the memory and stream components which are set to 6 and 5,        */
/* respectively.                                                         */
/*                                                                       */
/* See the file <freetype/internal/fttrace.h> for details of the         */
/* available toggle names.                                               */
/*                                                                       */
/* The level must be between 0 and 6; 0 means quiet (except for serious  */
/* runtime errors), and 6 means _very_ verbose.                          */
/*                                                                       */
FT_BASE_DEF(void)
ft_debug_init(void) {
	const char *ft2_debug = getenv("FT2_DEBUG");

	if (ft2_debug) {
		const char *p = ft2_debug;
		const char *q;

		for (; *p; p++) {
			/* skip leading whitespace and separators */
			if (*p == ' ' || *p == '\t' || *p == ',' || *p == ';' || *p == '=')
				continue;

			/* read toggle name, followed by ':' */
			q = p;
			while (*p && *p != ':')
				p++;

			if (*p == ':' && p > q) {
				FT_Int n, i, len = (FT_Int)(p - q);
				FT_Int level = -1, found = -1;

				for (n = 0; n < trace_count; n++) {
					const char *toggle = ft_trace_toggles[n];

					for (i = 0; i < len; i++) {
						if (toggle[i] != q[i])
							break;
					}

					if (i == len && toggle[i] == 0) {
						found = n;
						break;
					}
				}

				/* read level */
				p++;
				if (*p) {
					level = *p++ - '0';
					if (level < 0 || level > 6)
						level = -1;
				}

				if (found >= 0 && level >= 0) {
					if (found == trace_any) {
						/* special case for "any" */
						for (n = 0; n < trace_count; n++)
							ft_trace_levels[n] = level;
					} else
						ft_trace_levels[found] = level;
				}
			}
		}
	}
}

#else /* !FT_DEBUG_LEVEL_TRACE */

FT_BASE_DEF(void)
ft_debug_init(void) {
	/* nothing */
}

#endif /* !FT_DEBUG_LEVEL_TRACE */
