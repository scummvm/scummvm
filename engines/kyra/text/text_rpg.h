/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#ifndef KYRA_TEXT_EOB_H
#define KYRA_TEXT_EOB_H

#include "common/scummsys.h"
#include "kyra/graphics/screen.h"

namespace Kyra {

class KyraRpgEngine;

class TextDisplayer_rpg {
public:
	TextDisplayer_rpg(KyraRpgEngine *engine, Screen *scr);
	virtual ~TextDisplayer_rpg();

	void setupField(int dim, bool mode);

	virtual void printDialogueText(int stringId, const char *pageBreakString, const char *pageBreakString2 = 0);
	virtual void printDialogueText(const char *str, bool wait = false);
	void printMessage(const char *str, int textColor = -1, ...);
	virtual void printShadedText(const char *str, int x = -1, int y = -1, int textColor = -1, int shadowColor = -1, int pitchW = -1, int pitchH = -1, int marginRight = 0, bool screenUpdate = true) {}

	virtual int clearDim(int dim);
	void clearCurDim();

	void resetDimTextPositions(int dim);
	void resetPageBreakString();
	void setPageBreakFlag();
	void removePageBreakFlag();

	void allowPageBreak(bool mode) { _allowPageBreak = mode; }
	void setWaitButtonMode(int mode) { _waitButtonMode = mode; }
	int lineCount() const { return _lineCount; }
	//const uint8 *colorMap() const { return _colorMap; }

	// These methods are ScummVM specific. They are supposed to make necessary modifications
	// to the text displayer for the various Japanese and Chinese versions without too much
	// hackery...
	void setColorMapping(int sd, uint8 from, uint8 to);
	void setShadowColor(int sd, int col) { applySetting(sd, kShadowColor, col); }
	void setLineSpacing(int sd, int spacing) { applySetting(sd, kLineSpacing, spacing); }
	void setVisualLineSpacingAdjust(int sd, int adj) { applySetting(sd, kVisualLineSpacingAdjust, adj); }
	void setCharSpacing(int sd, int spacing) { applySetting(sd, kCharSpacing, spacing); }

protected:
	virtual KyraRpgEngine *vm() { return _vm; }
	virtual Screen *screen() { return _screen; }

	virtual void displayText(char *str, ...);
	char parseCommand();
	void readNextPara();
	void printLine(char *str);
	virtual void textPageBreak();
	void displayWaitButton();

	void convertString(char *str);

	char *_dialogueBuffer;

	const char *_tempString1;
	const char *_tempString2;
	char *_currentLine;
	char _ctrl[3];

	uint16 _lineWidth;
	uint32 _numCharsTotal;
	uint32 _numCharsLeft;
	uint32 _numCharsPrinted;

	bool _printFlag;
	bool _twoByteLineBreakFlag;
	const bool _pc98TextMode;

	Common::String _pageBreakString;
	Common::String _scriptParaString;
	int _lineCount;

	bool _allowPageBreak;
	int _waitButtonSpace;
	int _waitButtonMode;

	static const char _pageBreakDefault[3][5];

	struct TextDimData {
		uint8 color1;
		uint8 color2;
		uint16 column;
		uint8 line;
		// These properties don't appear in the original code. The various Japanese and Chinese versions
		// just had their modifications hacked in in whatever way the devs felt like. These properties
		// help making the necessary adjustments without too much hackery...
		int lineSpacing;
		int visualLineSpacingAdjust; // LOL PC-98 has the worst hack here. The visual line spacing is different than the one that is used to measure the text field space.
		int charSpacing;
		int shadowColor;
		int noHalfWidthLineEnd;
		uint8 *colorMap;
	};

	TextDimData *_textDimData;
	const int _dimCount;
	KyraRpgEngine *_vm;

private:
	bool isTwoByteChar(uint8 c) const;
	void applySetting(int sd, int ix, int val);
	uint8 remapColor(int sd, uint8 color) const;

	enum TextFieldVar {
		kLineSpacing = 0,
		kVisualLineSpacingAdjust,
		kCharSpacing,
		kShadowColor,
		kNoHalfWidthLineEnd,
		kOutOfRange
	};

	Screen *_screen;
	char *_table1;
	char *_table2;

	Screen::FontId _waitButtonFont;

	uint8 _colorMap[256];

	bool _isChinese;
};

} // End of namespace Kyra

#endif

#endif // ENABLE_EOB || ENABLE_LOL
