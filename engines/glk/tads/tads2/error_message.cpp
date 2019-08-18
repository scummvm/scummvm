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

#include "glk/tads/tads2/error_handling.h"
#include "glk/tads/tads2/ltk.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

errcxdef *lerini() {
	errcxdef *errcx;                                         /* error context */
  
	// allocate an error context
	if (!(errcx = (errcxdef *)ltk_suballoc(sizeof(errcxdef)))) {
		// failure
		return((errcxdef *)0);
	}

	// initialize the error context
	errcx->errcxfp  = (osfildef *)0;                  /* no error file handle */
	errcx->errcxofs = 0;                      /* no offset in argument buffer */
	errcx->errcxlog = ltk_errlog;                    /* error logging routine */
	errcx->errcxlgc = errcx;                         /* error logging context */

	// return the new context
	return errcx;
}

void errini(errcxdef *ctx, char *arg0) {
	VARUSED(ctx);
	VARUSED(arg0);
}

void errini(errcxdef *ctx, osfildef *fp) {
	VARUSED(ctx);
	VARUSED(fp);
}

void lerfre(errcxdef *errcx) {
	// free the context
	ltk_subfree(errcx);
}

void errmsg(errcxdef *ctx, char *outbuf, uint outbufl, uint err) {
	sprintf(outbuf, "Error #%d occured.", err);
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
