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

#ifndef GLK_ALAN3_EXE
#define GLK_ALAN3_EXE

/* Header file for instruction execution unit in Alan interpreter */

/* IMPORTS */
#include "glk/alan3/sysdep.h"
#include "glk/jumps.h"
#include "glk/alan3/acode.h"
#include "glk/alan3/types.h"
#include "glk/alan3/set.h"
#include "common/stream.h"

namespace Glk {
namespace Alan3 {

/* CONSTANTS */
#define NO_JUMP_RETURN 0
#define ERROR_RETURN 1
#define UNDO_RETURN 2


/* DATA */

extern Common::SeekableReadStream *textFile;		// The text and message file
extern bool printFlag;

/* FUNCTIONS */
extern void sys(Aword fpos, Aword len);
extern void sayInteger(int val);
extern void sayString(char *str);
extern Aptr strip(bool stripFromBeginningNotEnd, int count, bool stripWordsNotChars, int id, int atr);
extern Aptr concat(Aptr s1, Aptr s2);
extern char *getStringFromFile(Aword fpos, Aword len);
extern void print(Aword fpos, Aword len);
extern void score(Aword sc);
extern void visits(Aword v);
extern void undo(CONTEXT);
extern void quitGame(CONTEXT);
extern void restartGame(CONTEXT);

extern void use(CONTEXT, int act, int scr);
extern void stop(int act);

extern void empty(CONTEXT, int cnt, int whr);
extern int getContainerMember(int container, int index, bool directly);
extern int randomInContainer(int cont);

extern void schedule(Aword evt, Aword whr, Aword aft);
extern void cancelEvent(Aword evt);

extern int randomInteger(int from, int to);
extern bool between(int val, int from, int to);
extern bool contains(Aptr string, Aptr substring);
extern bool streq(char a[], char b[]);

extern void include(int instance, int atr, Aword member);
extern void exclude(int instance, int atr, Aword member);
extern void increase(int instance, int atr, Aword step);
extern void decrease(int instance, int atr, Aword step);

extern void startTranscript(void);
extern void stopTranscript(void);

} // End of namespace Alan3
} // End of namespace Glk

#endif
