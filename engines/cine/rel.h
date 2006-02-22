/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef CINE_REL_H_
#define CINE_REL_H_

typedef struct relData {
	char *ptr0;
	u16 var4;
	u16 var6;
	u16 var8;
	u16 varA;
} relStruct;

#define NUM_MAX_REL 255

extern relStruct relTable[NUM_MAX_REL];

void releaseObjectScripts(void);
void resetObjectScriptHead(void);

void loadRel(char *pRelName);

#endif
