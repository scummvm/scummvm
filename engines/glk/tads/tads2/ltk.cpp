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

#include "glk/tads/tads2/ltk.h"
#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/error_handling.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/*--------------------------------- ltkini ---------------------------------*/
/*
 * ltkini - allocate and INItialize toolkit context.
 */
void ltkini(unsigned short heapsiz) {
}


/*--------------------------------- ltkfre ---------------------------------*/
/*
 * ltkfre - FREe toolkit context.
 */
void ltkfre() {
}


/*------------------------------ ltk_suballoc ------------------------------*/
/*
 * ltk_suballoc - SUB ALLOCate memory from heap segment.
 */
void *ltk_suballoc(size_t siz) {
	return ltk_alloc(siz);
}


/*---------------------------- ltk_sigsuballoc -----------------------------*/
/*
 * ltk_sigsuballoc - allocate from heap, signal failure.
 */
void *ltk_sigsuballoc(errcxdef *errcx, size_t siz) {
  void *ptr;                                     /* ptr to allocated memory */

  /* allocate the memory */
  if (!(ptr = ltk_suballoc(siz)))
  {
    /* signal an error */
    errsigf(errcx, "LTK", 0);
  }

  /* return the memory */
  return(ptr);
}


/*------------------------------ ltk_subfree -------------------------------*/
/*
 * ltk_subfree - FREe memory allocated by ltk_suballoc
 */
void ltk_subfree(void *ptr) {
	free(ptr);
}


/*------------------------------- ltk_alloc --------------------------------*/
/*
 * ltk_alloc - Allocate a block of memory
 */
void *ltk_alloc(size_t siz) {
	byte *data = (byte *)malloc(siz);
	Common::fill(data, data + siz, 0);
	return data;
}

/*------------------------------ ltk_realloc ------------------------------*/

void *ltk_realloc(void *ptr, size_t siz) {
	return realloc(ptr, siz);
}

/*------------------------------ ltk_sigalloc ------------------------------*/
/*
 * ltk_sigalloc - allocate permanent global memory, and signal on failure.
 */
void *ltk_sigalloc(errcxdef *errcx, size_t siz) {
  void *ptr;                                 /* pointer to allocated memory */

  if (!(ptr = ltk_alloc(siz))) {
    /* signal error */
    errsigf(errcx, "LTK", 0);
  }

  /* return a ptr to the allocated memory */
  return ptr;
}


/*-------------------------------- ltk_free --------------------------------*/
/*
 * ltk_free - free a block of memory allocated by ltk_alloc.  This
 * takes the memory handle stashed just behind the allocation, and frees
 * the block of memory.  
 */
void ltk_free(void *mem) {
	free(mem);
}

/*------------------------------- ltk_errlog -------------------------------*/
/*
* ltk_errlog - ERRor LOGging function.  Logs an error from the LER
* system.
*/
void ltk_errlog(void *ctx, const char *fac, int errCode, int argc, erradef *argv) {
	char buf[128];                                  /* formatted error buffer */
	char msg[128];                                          /* message buffer */

															/* $$$ filter out error #504 $$$ */
	if (errCode == 504) return;

	/* get the error message into msg */
	errmsg((errcxdef *)ctx, msg, sizeof(msg), errCode);

	/* format the error message */
	errfmt(buf, (int)sizeof(buf), msg, argc, argv);

	/* display a dialog box containing the error message */
	ltk_dlg("Error", buf);
}


/*-------------------------------- ltk_dlg ---------------------------------*/
/*
* ltk_dlg - DiaLog.  Puts the given message in a dialog box.
*/
void ltk_dlg(const char *title, const char *msg, ...) {
	va_list  argp;                                             /* printf args */
	char     inbuf[80];                                       /* input buffer */
	char     outbuf[160];                    /* allow inbuf to double in size */

											 /* clip the input message, if necessary */
	strncpy(inbuf, msg, sizeof(inbuf));
	inbuf[sizeof(inbuf) - 1] = '\0';

	/* get the printf args, build the message, and display it */
	va_start(argp, msg);
	vsprintf(outbuf, inbuf, argp);

	/* display the message */
	error("%s", outbuf);
}


/*-------------------------------- ltk_beep --------------------------------*/
/*
* ltk_beep - BEEP the PC's speaker.
*/
void ltk_beep() {
}

/*------------------------------ ltk_beg_wait ------------------------------*/
/*
* ltk_beg_wait - Put up hourglass prompt.
*/
void ltk_beg_wait() {
}


/*------------------------------ ltk_end_wait ------------------------------*/
/*
* ltk_end_wait - Put up normal prompt.
*/
void ltk_end_wait() {
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
