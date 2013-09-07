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

typedef void (Scrolls::*func2)();

class Scrolls {
public:
	// Constants to replace the command characters from Pascal.
	// For more information, see: https://github.com/urukgit/avalot/wiki/Scrolldrivers

	static const char kControlSpeechBubble = 2; // ^B
	static const char kControlCenter = 3; // ^C
	static const char kControlToBuffer = 4; // ^D
	static const char kControlItalic = 6; // ^F
	static const char kControlBell = 7; // ^G
	static const char kControlBackspace = 8; // ^H
	static const char kControlInsertSpaces = 9; // ^I
	static const char kControlLeftJustified = 12; // ^L
	static const char kControlNewLine = 13; // ^M
	static const char kControlParagraph = 16; // ^P
	static const char kControlQuestion = 17; // ^Q
	static const char kControlRoman = 18; // ^R
	static const char kControlRegister = 19; // ^S
	static const char kControlNegative = 21; // ^U
	static const char kControlIcon = 22; // ^V



	bool aboutscroll; // Is this the about box?



	Scrolls(AvalancheEngine *vm);

	void init();

	void state(byte x);      // Sets "Ready" light to whatever

	void drawscroll(func2 gotoit);      // This is one of the oldest funcs in the game.

	void bubble(func2 gotoit);

	void resetscroll();

	void calldrivers();

	void display(Common::String z);

	bool ask(Common::String question);

	void natural();

	Common::String lsd();

	void okay();    // Says "Okay!"

	void musical_scroll();

	FontType ch[2];

private:
	AvalancheEngine *_vm;

	static const int16 roman = 0;
	static const int16 italic = 1;

	static const int16 halficonwidth = 19; // Half the width of an icon.



	int16 dix, diy;

	byte cfont; // Current font

	Common::Point dodgeCoord;
	byte param; // For using arguments code

	byte use_icon;



	void easteregg();

	void say(int16 x, int16 y, Common::String z);

	void normscroll();

	void dialogue();

	void store_(byte what, TuneType &played);

	bool they_match(TuneType &played);

	void music_scroll();

	void resetscrolldriver();

	void dingdongbell();

	void dodgem(); // This moves the mouse pointer off the scroll so that you can read it.

	void undodgem(); // This is the opposite of Dodgem. It moves the mouse pointer back, IF you haven't moved it in the meantime.

	void geticon(int16 x, int16 y, byte which);

	void block_drop(Common::String fn, int16 xl, int16 yl, int16 y);

	void strip(Common::String &q); // Strip trailing spaces.

	void solidify(byte n); // Does the word wrapping.

	void loadfont();
};

} // End of namespace Avalanche

#endif // AVALANCHE_SCROLLS2_H
