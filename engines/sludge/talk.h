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
#ifndef SLUDGE_TALK_H
#define SLUDGE_TALK_H

#include "sprites.h"

namespace Sludge {

struct speechLine {
	char *textLine;
	speechLine *next;
	int x;
};


struct speechStruct {
	onScreenPerson *currentTalker;
	speechLine *allSpeech;
	int speechY, lastFile, lookWhosTalking;
#if 0
	spritePalette talkCol;
#endif
};

int wrapSpeech(char *theText, int objT, int sampleFile, bool);
void viewSpeech();
void killAllSpeech();
int isThereAnySpeechGoingOn();
void initSpeech();
#if ALLOW_FILE
void saveSpeech(speechStruct *sS, FILE *fp);
bool loadSpeech(speechStruct *sS, FILE *fp);
#endif

} // End of namespace Sludge

#endif
