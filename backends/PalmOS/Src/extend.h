/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef EXTEND_H
#define EXTEND_H

extern const Char *SCUMMVM_SAVEPATH;

//#define DISABLE_SCUMM
#define DISABLE_SKY
#define DISABLE_SIMON
#define DISABLE_SWORD2

// PalmOS
//#define DISABLE_TAPWAVE

int main(int argc, char **argv);

void WinDrawWarpChars(const Char *chars, Int16 len, Coord x, Coord y, Coord maxWidth);
UInt16 StrReplace(Char *ioStr, UInt16 inMaxLen, const Char *inParamStr, const Char *fndParamStr);
void PalmFatalError(const Char *err);

#endif