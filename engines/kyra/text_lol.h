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

#ifdef ENABLE_LOL

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

	void setupField(bool mode);
	void expandField();

	int clearDim(int dim);
	void resetDimTextPositions(int dim);

	void printDialogueText(int dim, char *str, EMCState *script, const uint16 *paramList, int16 paramIndex);
	void printMessage(uint16 type, const char *str, ...) GCC_PRINTF(3, 4);

	int16 _scriptTextParameter;

private:
	void displayText(char *str, ...);
	char parseCommand();
	void readNextPara();
	void printLine(char *str);
	void preprocessString(char *str, EMCState *script, const uint16 *paramList, int16 paramIndex);
	void textPageBreak();

	void clearCurDim();

	char *_stringParameters[15];
	char *_buffer;
	char *_dialogueBuffer;
	char *_tempString1;
	char *_tempString2;
	char *_currentLine;
	char _ctrl[3];

	char _scriptParaString[11];

	uint16 _lineWidth;
	int _lineCount;
	uint32 _numCharsTotal;
	uint32 _numCharsLeft;
	uint32 _numCharsPrinted;

	bool _printFlag;
	bool _sjisLineBreakFlag;

	LoLEngine *_vm;
	Screen_LoL *_screen;

	struct TextDimData {
		uint8 color1;
		uint8 color2;
		uint16 column;
		uint8 line;
	};

	TextDimData _textDimData[14];
};

} // end of namespace Kyra

#endif

#endif // ENABLE_LOL

