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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

 /* SCROLLS		The scroll driver. */

#ifndef AVALANCHE_SCROLLS2_H
#define AVALANCHE_SCROLLS2_H

#include "common/system.h"

namespace Avalanche {
class AvalancheEngine;

class Scrolls;

typedef void (Scrolls::*ScrollsFunctionType)();

class Scrolls {
public:
	// Constants to replace the command characters from Pascal.
	// For more information, see: https://github.com/urukgit/avalot/wiki/Scrolldrivers
	enum ControlCharacter {
		kControlSpeechBubble = 2, // ^B
		kControlCenter = 3, // ^C
		kControlToBuffer = 4, // ^D
		kControlItalic = 6, // ^F
		kControlBell = 7, // ^G
		kControlBackspace = 8, // ^H
		kControlInsertSpaces = 9, // ^I
		kControlLeftJustified = 12, // ^L
		kControlNewLine = 13, // ^M
		kControlParagraph = 16, // ^P
		kControlQuestion = 17, // ^Q
		kControlRoman = 18, // ^R
		kControlRegister = 19, // ^S
		kControlNegative = 21, // ^U
		kControlIcon = 22 // ^V
	};

	bool _aboutScroll; // Is this the about box?
	FontType _scrollFonts[2];

	Scrolls(AvalancheEngine *vm);

	void init();
	void setReadyLight(byte state); // Sets "Ready" light to whatever.
	void drawScroll(ScrollsFunctionType modeFunc);
	void drawBubble(ScrollsFunctionType modeFunc);
	void resetScroll();
	void callScrollDriver();
	void displayText(Common::String text);
	bool displayQuestion(Common::String question);
	void setBubbleStateNatural(); // Natural state of bubbles
	Common::String displayMoney();
	void musicalScroll(); // Practically this one is a mini-game which called when you play the harp in the monastery.

private:
	AvalancheEngine *_vm;

	enum FontStyle {
		kFontStyleRoman,
		kFontStyleItalic
	};

	static const int16 kHalfIconWidth = 19; // Half the width of an icon.

	int16 _shadowBoxX, _shadowBoxY;
	byte _currentFont; // Current font
	Common::Point _dodgeCoord;
	byte _param; // For using arguments code
	byte _useIcon;

	// These 3 functions are always passed as ScrollsFunctionType parameters.
	void scrollModeNormal();
	void scrollModeDialogue();
	void scrollModeMusic();

	// These 2 are used only in musicalScroll().
	void store(byte what, TuneType &played);
	bool theyMatch(TuneType &played);

	void stripTrailingSpaces(Common::String &str); // Original: strip.
	void solidify(byte n); // Does the word wrapping.

	void dodgem(); // This moves the mouse pointer off the scroll so that you can read it.
	void unDodgem(); // This is the opposite of Dodgem. It moves the mouse pointer back, IF you haven't moved it in the meantime.

	void easterEgg();
	void say(int16 x, int16 y, Common::String text);
	void resetScrollDriver();
	void ringBell(); // Original: dingdongbell
	void getIcon(int16 x, int16 y, byte which);
	void drawSign(Common::String name, int16 xl, int16 yl, int16 y); // This is for drawing a big "about" or "gameover" picture loaded from a file into an empty scroll.
	void loadFont();
};

} // End of namespace Avalanche

#endif // AVALANCHE_SCROLLS2_H
