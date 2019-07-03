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

#include "glk/alan3/syserr.h"
#include "glk/alan3/current.h"
#include "glk/alan3/debug.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/output.h"
#include "glk/alan3/utils.h"
#include "common/textconsole.h"

namespace Glk {
namespace Alan3 {

static void (*handler)(const char *);

/*----------------------------------------------------------------------*/
static void runtimeError(CONTEXT, const char *errorClassification, const char *errorDescription, const char *blurb) {
	output("$n$nAs you enter the twilight zone of Adventures, you stumble \
and fall to your knees. In front of you, you can vaguely see the outlines \
of an Adventure that never was.$n$n");
	output(errorClassification);
	output(errorDescription);
	newline();

	if (current.sourceLine != 0) {
		printf("At source line %d in '%s':\n", current.sourceLine, sourceFileName(current.sourceFile));
		printf("%s", readSourceLine(current.sourceFile, current.sourceLine));
	}

	newline();
	output(blurb);

	terminate(context, 2);
}

static void runtimeError(const char *errorClassification, const char *errorDescription, const char *blurb) {
	::error("%s%s %s", errorClassification, errorDescription, blurb);
}


/*======================================================================*/
void setSyserrHandler(void (*f)(const char *)) {
	handler = f;
}


/*======================================================================*/
// TODO Make syserr() use ... as printf()
void syserr(const char *description) {
	lin = 0;
	if (handler == NULL) {
		const char *blurb = "<If you are the creator of this piece of Interactive Fiction, \
please help debug this Alan system error. Collect *all* the sources, and, if possible, an \
exact transcript of the commands that led to this error, in a zip-file and send \
it to support@alanif.se. Thank you!>";
		runtimeError("SYSTEM ERROR: ", description, blurb);
	} else
		handler(description);
}


/*======================================================================*/
void apperr(const char *description) {
	if (handler == NULL) {
		const char *blurb = "<If you are playing this piece of Interactive Fiction, \
please help the author to debug this programming error. Send an exact \
transcript of the commands that led to this error to the author. Thank you! \
If you *are* the author, then you have to figure this out before releasing the game.>";
		runtimeError("APPLICATION ERROR: ", description, blurb);
	} else
		handler(description);
}

/*======================================================================*/
void playererr(CONTEXT, const char *description) {
	if (handler == NULL) {
		const char *blurb = "<You have probably done something that is not exactly right.>";
		runtimeError(context, "PLAYER ERROR: ", description, blurb);
	} else
		handler(description);
}

} // End of namespace Alan3
} // End of namespace Glk
