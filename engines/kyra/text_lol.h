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

#ifndef KYRA_TEXT_LOL_H
#define KYRA_TEXT_LOL_H

namespace Kyra {

class Screen_v2;
class LoLEngine;
struct EMCState;

class TextDisplayer_LoL {
friend class LoLEngine;
public:
	TextDisplayer_LoL(LoLEngine *vm, Screen_LoL *screen);
	~TextDisplayer_LoL();

	void setAnimParameters(const char *str, int x, uint8 col1, uint8 col2);
	void setAnimFlag(bool flag) { _animFlag = flag; }
	
	void setupField(bool mode);
	void expandField();

	void printDialogueText(int dim, char *str, EMCState *script, const uint16 *paramList, int16 paramIndex);
	void printMessage(uint16 type, char *str, ...);
	
	int16 _scriptParameter;

private:
	void displayText(char *str, ...);
	char parseCommand();
	void readNextPara();
	void printLine(char *str);
	void preprocessString(char *str, EMCState *script, const uint16 *paramList, int16 paramIndex);
	
	//typedef void (LoLEngine::*DialogueAnimCallback)(const char *str, uint16 lineWidth, uint8 col1, uint8 col2);
	//DialogueAnimCallback _dlgAnimCallback;
	//void portraitAnimation1(const char *str);
	void portraitAnimation2();
	

	char *_stringParameters[15];
	char *_buffer;
	char *_dialogueBuffer;
	char *_tempString1;
	char *_tempString2;
	char *_currentLine;
	char _ctrl[3];

	char _scriptParaString[11];
	uint32 _stringLength;

	uint16 _lineWidth;
	uint32 _numChars;
	uint32 _numCharsPrinted;
	
	const char *_animString;
	int16 _animWidth;
	uint8 _animColour1;
	uint8 _animColour2;

	bool _animFlag;
	bool _printFlag;

	uint8 _posX;
	uint8 _posY;
	uint8 _colour1;
	uint8 _colour2;
	bool _colour1prot;

	LoLEngine *_vm;
	Screen_LoL *_screen;
};

} // end of namespace Kyra

#endif

