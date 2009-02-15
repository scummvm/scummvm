/***************************************************************************
 gfx_crossblit.c Copyright (C) 2000 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

/* This file isn't used directly; rather, it's included by a different file. */
/* Note that memcpy() is assumed to be an inlineable built-in. If it isn't,
** performance will suck... badly.
*/
/* Config parameters:
** FUNCTION_NAME: Name of the blitter function
** USE_PRIORITY: Whether to care about the priority buffer
** BYTESPP: Bytes per pixel
*/

#include "common/scummsys.h"

/* set optimisations for Win32: */
/* g on: enable global optimizations */
/* t on: use fast code */
/* y on: suppress creation of frame pointers on stack */
/* s off: disable minimize size code */
#ifdef WIN32
#	include <memory.h>
#	ifndef SATISFY_PURIFY
#		pragma optimize( "s", off )
#		pragma optimize( "gty", on )
#		pragma intrinsic( memcpy, memset )
#	endif
#endif

static void FUNCTION_NAME(byte *dest, byte *src, int bytes_per_dest_line, int bytes_per_src_line,
			  int xl, int yl, byte *alpha, int bytes_per_alpha_line, int bytes_per_alpha_pixel,
			  unsigned int alpha_test_mask, unsigned int alpha_min
#ifdef USE_PRIORITY
			  , byte *priority_buffer, int bytes_per_priority_line, int bytes_per_priority_pixel, int priority
#endif /* USE_PRIORITY */
		   )
{
	int x, y;
	int alpha_end = xl * bytes_per_alpha_pixel;

	for (y = 0; y < yl; y++) {
		int pixel_offset = 0;
		int alpha_offset = 0;
#ifdef USE_PRIORITY
		int priority_offset = 0;
#endif /* USE_PRIORITY */

		for (x = 0; x < alpha_end; x += bytes_per_alpha_pixel) {
			if ((alpha_test_mask & alpha[x])
#ifdef REVERSE_ALPHA
			    >=
#else
			    <
#endif
			    alpha_min)
#ifdef USE_PRIORITY
				if (priority_buffer[priority_offset] <= priority) {
					priority_buffer[priority_offset] = priority;
#endif /* USE_PRIORITY */
					memcpy(dest + pixel_offset, src + pixel_offset, BYTESPP);
#ifdef USE_PRIORITY
				}
#endif /* USE_PRIORITY */

			pixel_offset += BYTESPP;
			alpha_offset += bytes_per_alpha_pixel;
#ifdef USE_PRIORITY
			priority_offset += bytes_per_priority_pixel;
#endif /* USE_PRIORITY */
		}

		dest += bytes_per_dest_line;
		src += bytes_per_src_line;
		alpha += bytes_per_alpha_line;
#ifdef USE_PRIORITY
		priority_buffer += bytes_per_priority_line;
#endif /* USE_PRIORITY */
	}
}

/* reset to original optimisations for Win32: */
/* (does not reset intrinsics) */
#ifdef WIN32
#  pragma optimize( "", on )
#endif
