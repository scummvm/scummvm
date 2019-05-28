/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*  Library porting Tool Kit
 *
 * These are generic definitions which should be applicable to any system.
 */

#ifndef GLK_TADS_TADS2_LTK
#define GLK_TADS_TADS2_LTK

#include "glk/tads/tads2/error_handling.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/*
 * ltkini - allocate and INItialize ltk context.  'heapsiz' is the
 * requested size for the local heap. Returns 0 if the request cannot be
 * satisfied.  
 */
extern void ltkini(unsigned short heapsiz);


/*
 * ltkfre - FREe ltk context.  
 */
extern void ltkfre();


/*
 * ltk_dlg - DiaLoG.  Present user with informational dialog message.
 * 'title' specifies the title to use in the dialog box, 'msg' is the
 * text message, which may contain printf-style formatting.
 * printf-style arguments must be passed in also, if the message
 * requires them.  
 */
extern void ltk_dlg(const char *title, const char *msg, ...);


/*
 * ltk_errlog - Error logging function for LER routines.
 */
extern void ltk_errlog(void *ctx, const char *fac, int errCode, int agrc, erradef *argv);


/*
 * ltk_alloc - ALLOCate permanent global memory.  Returns 0 if the
 * request cannot be satisfied.  
 */
extern void *ltk_alloc(size_t siz);

/* ltk_realloc - reallocate memory; analogous to realloc() */
extern void *ltk_realloc(void *ptr, size_t siz);


/*
 * ltk_sigalloc - ALLOCate permanent global memory, signals error on
 * failure.  
 */
extern void *ltk_sigalloc(struct errcxdef *errcx, size_t siz);


/*
 * ltk_free - FREE memory allocated using ltk_alloc.
 */
extern void ltk_free(void *ptr);


/*
 * ltk_suballoc - SUB-ALLOCate memory from user heap.  Returns 0 if the
 * request cannot be satisfied.  
 */
extern void *ltk_suballoc(size_t siz);


/*
 * ltk_sigsuballoc - SUB-ALLOCate memory from user heap, signals error
 * on failure. 
 */
extern void *ltk_sigsuballoc(struct errcxdef *errcx, size_t siz);


/*
 * ltk_subfree - SUBsegment FREE.  Frees memory allocated by
 * ltk_suballoc.  
 */
extern void ltk_subfree(void *ptr);


/*
 * ltk_beep - BEEP the user. 
 */
extern void ltk_beep(void);


/*
 * ltk_beg_wait - signal that the user needs to wait.  
 */
extern void ltk_beg_wait(void);


/*
 * ltk_end_wait - end the waiting period .
 */
extern void ltk_end_wait(void);

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
