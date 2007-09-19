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

#ifndef CINE_TEXTE_H
#define CINE_TEXTE_H


#include "common/scummsys.h"

namespace Cine {

typedef char commandeType[20];

extern byte *textDataPtr;
extern byte textTable[256][2][16 * 8];

extern const char **failureMessages;
extern const commandeType *defaultActionCommand;
extern const commandeType *systemMenu;
extern const commandeType *confirmMenu;
extern const char **otherMessages;
extern const char *commandPrepositionOn;

struct CharacterEntry {
	byte characterIdx;
	byte characterWidth;
};

extern const CharacterEntry *fontParamTable;

void loadTextData(const char *pFileName, byte *pDestinationBuffer);
void loadErrmessDat(const char *fname);
void freeErrmessDat(void);
void loadPoldatDat(const char *fname);
void freePoldatDat(void);

uint16 computeMessageLength(const byte *ptr, uint16 width, uint16 *numWords, uint16 *messageWidth, uint16 *lineResult);

} // End of namespace Cine

#endif
