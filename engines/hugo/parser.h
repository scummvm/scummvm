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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_PARSER_H
#define HUGO_PARSER_H
namespace Hugo {

enum seqTextParser {
	kTBExit     = 0,
	kTBMaze     = 1,
	kTBNoPoint  = 2,
	kTBNoun     = 3,
	kTBVerb     = 4,
	kTBEh       = 5,
	kTBUnusual  = 6,
	kTBHave     = 7,
	kTBNoUse    = 8,
	kTBDontHave = 9,
	kTBNeed     = 10,
	kTBOk       = 11,
	kCmtAny1    = 12,
	kCmtAny2    = 13,
	kCmtAny3    = 14,
	kCmtClose   = 15
};

class Parser {
public:
	Parser(HugoEngine &vm);

	bool  isWordPresent(char **wordArr);

	void  charHandler();
	void  command(const char *format, ...);
	void  drawStatusText();
	void  keyHandler(uint16 nChar, uint16 nFlags);
	void  lineHandler();

private:
	HugoEngine &_vm;

	char   _ringBuffer[32];                         // Ring buffer
	uint16 _putIndex;
	uint16 _getIndex;                               // Index into ring buffer

	command_t _statusLine;
	command_t _scoreLine;

	bool  isBackgroundWord(objectList_t obj, char *line);
	bool  isCarrying(uint16 wordIndex);
	bool  isCatchallVerb(objectList_t obj, char *line);
	bool  isGenericVerb(object_t *obj, char *line, char *comment);
	bool  isNear(object_t *obj, char *verb, char *comment);
	bool  isObjectVerb(object_t *obj, char *line, char *comment);

	char *findNoun(char *line);
	char *findVerb(char *line);
	char *strlwr(char *buffer);

	void  dropObject(object_t *obj);
	void  showTakeables();
	void  takeObject(object_t *obj);
};

} // end of namespace Hugo
#endif //HUGO_PARSER_H
