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
 
#include <PalmOS.h>
#include "extend.h"
#include "string.h"
#include "palm.h"

const Char *SCUMMVM_SAVEPATH = "/PALM/Programs/ScummVM/Saved/";

UInt16 StrReplace(Char *ioStr, UInt16 inMaxLen, const Char *inParamStr, const Char *fndParamStr)
{
	Char *found;
	Boolean quit = false;
	UInt16 occurences = 0;
	UInt16 newLength;
	UInt16 l1 = StrLen(fndParamStr);
	UInt16 l2 = 0;
	UInt16 l3 = StrLen(ioStr);
	UInt16 next = 0;
		
	if (inParamStr)
		l2 = StrLen(inParamStr); // can be null to know how many occur.
	
	while (((found = StrStr(ioStr+next, fndParamStr)) != NULL) && (!quit))
	{
		occurences++;
		newLength = (StrLen(ioStr) - l1 + l2);

		if ( newLength > inMaxLen )
		{
			quit = true;
			occurences--;
		}
		else if (inParamStr)
		{
			MemMove(found + l2, found + l1, inMaxLen-(found-ioStr+l2));
			MemMove(found, inParamStr, l2);
			next = found - ioStr + l2;
		}
		else
			next = found - ioStr + l1;
	}
	
	if (inParamStr)
		ioStr[l3 + l2*occurences - l1*occurences] = 0;

	return occurences;
}

void WinDrawWarpChars(const Char *chars, Int16 len, Coord x, Coord y, Coord maxWidth)
{
	Char *part = (Char *)chars;
	Coord x2 = x;
	Int16 next;
	
	if (part[StrLen(part)-1] == '\n')
		part[StrLen(part)-1] = 0;

	part = StrTok(part," ");

	while ( part )
	{
		next = FntLineWidth (part, StrLen(part)) + FntLineWidth (" ",1);
		if ((x2 + next - x) > maxWidth)
		{
			x2 = x;
			y += FntLineHeight();
		}
		//HRWinDrawTruncChars(gHRrefNum, part, StrLen(part), x2, y, maxWidth - (x2-x));
		WinDrawTruncChars(part, StrLen(part), x2, y, maxWidth - (x2-x));
		x2 += next;

		part = StrTok(NULL," ");
	}
}