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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

 /* SCROLLS		The scroll driver. */

#ifndef AVALANCHE_DIALOGS_H
#define AVALANCHE_DIALOGS_H

namespace Avalanche {
class AvalancheEngine;

class Dialogs;

typedef void (Dialogs::*DialogFunctionType)();

class Dialogs {
public:
	bool _aboutBox; // Is this the about box? - Used in scrollModeNormal(), not yet fully implemented
	FontType _fonts[2];

	Dialogs(AvalancheEngine *vm);

	void init();
	void reset();
	void setReadyLight(byte state);
	void displayText(Common::String text);
	bool displayQuestion(Common::String question);
	void setTalkPos(int16 x, int16 y);
	int16 getTalkPosX();
	void setBubbleStateNatural();
	void displayMusicalScroll();
	void displayScrollChain(char block, byte point, bool report = true, bool bubbling = false);
	void talkTo(byte whom);
	void sayIt(Common::String str);
	Common::String personSpeaks();
	void sayThanks(byte thing);
	void sayHello();
	void sayOK();
	void saySilly();
private:
	AvalancheEngine *_vm;
	int16 _talkX, _talkY;

	enum FontStyle {
		kFontStyleRoman,
		kFontStyleItalic
	};

	typedef int8 TuneType[31];

	static const TuneType kTune;
	static const int16 kHalfIconWidth = 19;
	static const QuasipedType kQuasipeds[16];

	Common::String _scroll[15];
	Common::Point _dodgeCoord;
	byte _maxLineNum;
	bool _scReturn;
	bool _noError;
	byte _currentFont;
	byte _param; // For using arguments code
	byte _useIcon;
	byte _scrollBells; // no. of times to ring the bell
	int16 _underScroll; // Y-coord of just under the scroll text.
	int16 _shadowBoxX, _shadowBoxY;

	void drawBubble(DialogFunctionType modeFunc);
	void drawScroll(DialogFunctionType modeFunc);
	void scrollModeNormal();
	void scrollModeDialogue();
	void scrollModeMusic();

	// These 2 are used only in musicalScroll().
	void store(byte what, TuneType &played);
	bool theyMatch(TuneType &played);
	void stripTrailingSpaces(Common::String &str);
	void solidify(byte n);
	void dodgem();
	void unDodgem();

	Common::String displayMoney();
	void easterEgg();
	void say(int16 x, int16 y, Common::String text);
	void resetScrollDriver();
	void ringBell();
	void loadFont();

	void unSkrimble(Common::String &text);
	void doTheBubble(Common::String &text);
	void speak(byte who, byte subject);
};

} // End of namespace Avalanche

#endif // AVALANCHE_DIALOGS_H
