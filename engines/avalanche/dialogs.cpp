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

#include "avalanche/avalanche.h"
#include "avalanche/dialogs.h"

#include "common/system.h"
#include "common/random.h"

namespace Avalanche {

const Dialogs::TuneType Dialogs::kTune = {
	kPitchHigher, kPitchHigher, kPitchLower, kPitchSame, kPitchHigher, kPitchHigher, kPitchLower, kPitchHigher, kPitchHigher, kPitchHigher,
	kPitchLower, kPitchHigher, kPitchHigher, kPitchSame, kPitchHigher, kPitchLower, kPitchLower, kPitchLower, kPitchLower, kPitchHigher,
	kPitchHigher, kPitchLower, kPitchLower, kPitchLower, kPitchLower, kPitchSame, kPitchLower, kPitchHigher, kPitchSame, kPitchLower, kPitchHigher
};

// A quasiped defines how people who aren't sprites talk. For example, quasiped
// "A" is Dogfood. The rooms aren't stored because I'm leaving that to context.
const QuasipedType Dialogs::kQuasipeds[16] = {
	//_whichPed, _foregroundColor,   _room,      _backgroundColor,     _who
	{1, kColorLightgray,    kRoomArgentPub,    kColorBrown,    kPeopleDogfood},   // A: Dogfood (screen 19).
	{2, kColorGreen,        kRoomArgentPub,    kColorWhite,    kPeopleIbythneth}, // B: Ibythneth (screen 19).
	{2, kColorWhite,        kRoomYours,        kColorMagenta,  kPeopleArkata},    // C: Arkata (screen 1).
	{2, kColorBlack,        kRoomLustiesRoom,  kColorRed,      kPeopleInvisible}, // D: Hawk (screen 23).
	{2, kColorLightgreen,   kRoomOutsideDucks, kColorBrown,    kPeopleTrader},    // E: Trader (screen 50).
	{5, kColorYellow,       kRoomRobins,       kColorRed,      kPeopleAvalot},    // F: Avvy, tied up (scr.42)
	{1, kColorBlue,         kRoomAylesOffice,  kColorWhite,    kPeopleAyles},     // G: Ayles (screen 16).
	{1, kColorBrown,        kRoomMusicRoom,    kColorWhite,    kPeopleJacques},   // H: Jacques (screen 7).
	{1, kColorLightgreen,   kRoomNottsPub,     kColorGreen,    kPeopleSpurge},    // I: Spurge (screen 47).
	{2, kColorYellow,       kRoomNottsPub,     kColorRed,      kPeopleAvalot},    // J: Avalot (screen 47).
	{1, kColorLightgray,    kRoomLustiesRoom,  kColorBlack,    kPeopleDuLustie},  // K: du Lustie (screen 23).
	{1, kColorYellow,       kRoomOubliette,    kColorRed,      kPeopleAvalot},    // L: Avalot (screen 27).
	{2, kColorWhite,        kRoomOubliette,    kColorRed,      kPeopleInvisible}, // M: Avaroid (screen 27).
	{3, kColorLightgray,    kRoomArgentPub,    kColorDarkgray, kPeopleMalagauche},// N: Malagauche (screen 19).
	{4, kColorLightmagenta, kRoomNottsPub,     kColorRed,      kPeoplePort},      // O: Port (screen 47).
	{1, kColorLightgreen,   kRoomDucks,        kColorDarkgray, kPeopleDrDuck}     // P: Duck (screen 51).
};

Dialogs::Dialogs(AvalancheEngine *vm) {
	_vm = vm;
	_noError = true;

	_aboutBox = false;
	_talkX = 0;
	_talkY = 0;
	_maxLineNum = 0;
	_scReturn = false;
	_currentFont = kFontStyleRoman;
	_param = 0;
	_useIcon = 0;
	_scrollBells = 0;
	_underScroll = 0;
	_shadowBoxX = 0;
	_shadowBoxY = 0;
}

void Dialogs::init() {
	loadFont();
	resetScrollDriver();
}

/**
 * Determine the color of the ready light and draw it
 * @remarks	Originally called 'state'
 */
void Dialogs::setReadyLight(byte state) {
	if (_vm->_ledStatus == state)
		return; // Already like that!

	// TODO: Implement different patterns for green color.
	Color color = kColorBlack;
	switch (state) {
	default:
	case 0:
		color = kColorBlack;
		break; // Off
	case 1:
	case 2:
	case 3:
		color = kColorGreen;
		break; // Hit a key
	}

	_vm->_graphics->drawReadyLight(color);
	CursorMan.showMouse(true);
	_vm->_ledStatus = state;
}

void Dialogs::easterEgg() {
	warning("STUB: Scrolls::easterEgg()");
}

void Dialogs::say(int16 x, int16 y, Common::String z) {
	FontType itw;
	byte lz = z.size();

	bool offset = x % 8 == 4;
	x /= 8;
	y++;
	int16 i = 0;
	for (int xx = 0; xx < lz; xx++) {
		switch (z[xx]) {
		case kControlRoman:
			_currentFont = kFontStyleRoman;
			break;
		case kControlItalic:
			_currentFont = kFontStyleItalic;
			break;
		default: {
			for (int yy = 0; yy < 12; yy++)
				itw[(byte)z[xx]][yy] = _fonts[_currentFont][(byte)z[xx]][yy + 2];

			// We have to draw the characters one-by-one because of the accidental font changes.
			i++;
			Common::String chr(z[xx]);
			_vm->_graphics->drawScrollText(chr, itw, 12, (x - 1) * 8 + offset * 4 + i * 8, y, kColorBlack);
			}
		}
	}
}

/**
 * One of the 3 "Mode" functions passed as ScrollsFunctionType parameters.
 * @remarks	Originally called 'normscroll'
 */
void Dialogs::scrollModeNormal() {
	// Original code is:
	// egg : array[1..8] of char = ^P^L^U^G^H+'***';
	// this is not using kControl characters: it's the secret code to be entered to trigger the easter egg
	// TODO: To be fixed when the Easter egg code is implemented
	Common::String egg = Common::String::format("%c%c%c%c%c***", kControlParagraph, kControlLeftJustified, kControlNegative, kControlBell, kControlBackspace);
	Common::String e = "(c) 1994";

	setReadyLight(3);
	_vm->_animationsEnabled = false;
	_vm->_graphics->loadMouse(kCurFletch);

	_vm->_graphics->saveScreen();
	_vm->_graphics->showScroll();

	Common::Event event;
	bool escape = false;
	while (!_vm->shouldQuit() && !escape) {
		_vm->_graphics->refreshScreen();
		while (_vm->getEvent(event)) {
			if ((event.type == Common::EVENT_LBUTTONUP) ||
				((event.type == Common::EVENT_KEYDOWN) && ((event.kbd.keycode == Common::KEYCODE_ESCAPE) ||
				(event.kbd.keycode == Common::KEYCODE_RETURN) || (event.kbd.keycode == Common::KEYCODE_HASH) ||
				(event.kbd.keycode == Common::KEYCODE_PLUS)))) {
				escape = true;
				break;
			} else if (event.type == Common::EVENT_KEYDOWN)
				_vm->errorLed();
		}
	}

	_vm->_graphics->restoreScreen();
	_vm->_graphics->removeBackup();

	warning("STUB: scrollModeNormal() - Check Easter Egg trigger");
#if 0
	char r;
	bool oktoexit;
	do {
		do {
			_vm->check(); // was "checkclick;"

//#ifdef RECORD slowdown(); basher::count++; #endif

			if (_vm->_enhanced->keypressede())
				break;
		} while (!((mrelease > 0) || (buttona1()) || (buttonb1())));

		if (mrelease == 0) {
			inkey();
			if (aboutscroll) {
				move(e[2 - 1], e[1 - 1], 7);
				e[8 - 1] = inchar;
				if (egg == e)
					easteregg();
			}
			oktoexit = set::of('\15', '\33', '+', '#', eos).has(inchar);
			if (!oktoexit)  errorled();
		}

	} while (!((oktoexit) || (mrelease > 0)));

//#ifdef RECORD record_one(); #endif

	_vm->screturn = r == '#'; // "back door"
#endif

	setReadyLight(0);
	_vm->_animationsEnabled = true;
	_vm->_holdLeftMouse = false; // Used in Lucerna::checkclick().
}

/**
 * One of the 3 "Mode" functions passed as ScrollsFunctionType parameters.
 * The "asking" scroll. Used indirectly in diplayQuestion().
 * @remarks	Originally called 'dialogue'
 */
void Dialogs::scrollModeDialogue() {
	_vm->_graphics->loadMouse(kCurHand);

	_vm->_graphics->saveScreen();
	_vm->_graphics->showScroll();

	Common::Event event;
	while (!_vm->shouldQuit()) {
		_vm->_graphics->refreshScreen();

		_vm->getEvent(event);

		Common::Point cursorPos = _vm->getMousePos();
		cursorPos.y /= 2;

		char inChar = 0;
		if ((event.type == Common::EVENT_KEYDOWN) && (event.kbd.ascii <= 122) && (event.kbd.ascii >= 97)) {
			inChar = (char)event.kbd.ascii;
			Common::String temp(inChar);
			temp.toUppercase();
			inChar = temp[0];
		}

		if (_vm->shouldQuit() || (event.type == Common::EVENT_LBUTTONUP) || (event.type == Common::EVENT_KEYDOWN)) {
			if (((cursorPos.x >= _shadowBoxX - 65) && (cursorPos.y >= _shadowBoxY - 24) && (cursorPos.x <= _shadowBoxX - 5) && (cursorPos.y <= _shadowBoxY - 10))
				|| (inChar == 'Y') || (inChar == 'J') || (inChar == 'O')) { // Yes, Ja, Oui
				_scReturn = true;
				break;
			} else if (((cursorPos.x >= _shadowBoxX + 5) && (cursorPos.y >= _shadowBoxY - 24) && (cursorPos.x <= _shadowBoxX + 65) && (cursorPos.y <= _shadowBoxY - 10))
						|| (inChar == 'N')){ // No, Non, Nein
				_scReturn = false;
				break;
			}
		}
	}

	_vm->_graphics->restoreScreen();
	_vm->_graphics->removeBackup();
}

void Dialogs::store(byte what, TuneType &played) {
	memmove(played, played + 1, sizeof(played) - 1);
	played[30] = what;
}

bool Dialogs::theyMatch(TuneType &played) {
	byte mistakes = 0;

	for (unsigned int i = 0; i < sizeof(played); i++) {
		if (played[i] != kTune[i])
			mistakes++;
	}

	return mistakes < 5;
}

/**
 * One of the 3 "Mode" functions passed as ScrollsFunctionType parameters.
 * Part of the harp mini-game.
 * @remarks	Originally called 'music_Scroll'
 */
void Dialogs::scrollModeMusic() {
	setReadyLight(3);
	_vm->_animationsEnabled = false;
	CursorMan.showMouse(false);
	_vm->_graphics->loadMouse(kCurFletch);

	TuneType played;
	for (unsigned int i = 0; i < sizeof(played); i++)
		played[i] = kPitchInvalid;
	int8 lastOne = -1, thisOne = -1; // Invalid values.

	_vm->_animationsEnabled = false;

	_vm->_graphics->saveScreen();
	_vm->_graphics->showScroll();

	Common::Event event;
	while (!_vm->shouldQuit()) {
		_vm->_graphics->refreshScreen();

		_vm->getEvent(event);

		// When we stop playing?
		if ((event.type == Common::EVENT_LBUTTONDOWN) ||
			((event.type == Common::EVENT_KEYDOWN) && ((event.kbd.keycode == Common::KEYCODE_RETURN) || (event.kbd.keycode == Common::KEYCODE_ESCAPE)))) {
				break;
		}

		// When we DO play:
		if ((event.type == Common::EVENT_KEYDOWN)
			&& ((event.kbd.keycode == Common::KEYCODE_q) || (event.kbd.keycode == Common::KEYCODE_w)
			|| (event.kbd.keycode == Common::KEYCODE_e) || (event.kbd.keycode == Common::KEYCODE_r)
			|| (event.kbd.keycode == Common::KEYCODE_t) || (event.kbd.keycode == Common::KEYCODE_y)
			|| (event.kbd.keycode == Common::KEYCODE_u) || (event.kbd.keycode == Common::KEYCODE_i)
			|| (event.kbd.keycode == Common::KEYCODE_o) || (event.kbd.keycode == Common::KEYCODE_p)
			|| (event.kbd.keycode == Common::KEYCODE_LEFTBRACKET) || (event.kbd.keycode == Common::KEYCODE_RIGHTBRACKET))) {
				byte value = 0;
				switch (event.kbd.keycode) {
				case Common::KEYCODE_q:
					value = 0;
					break;
				case Common::KEYCODE_w:
					value = 1;
					break;
				case Common::KEYCODE_e:
					value = 2;
					break;
				case Common::KEYCODE_r:
					value = 3;
					break;
				case Common::KEYCODE_t:
					value = 4;
					break;
				case Common::KEYCODE_y:
					value = 5;
					break;
				case Common::KEYCODE_u:
					value = 6;
					break;
				case Common::KEYCODE_i:
					value = 7;
					break;
				case Common::KEYCODE_o:
					value = 8;
					break;
				case Common::KEYCODE_p:
					value = 9;
					break;
				case Common::KEYCODE_LEFTBRACKET:
					value = 10;
					break;
				case Common::KEYCODE_RIGHTBRACKET:
					value = 11;
					break;
				default:
					error("cannot happen");
					break;
				}

				lastOne = thisOne;
				thisOne = value;

				_vm->_sound->playNote(_vm->kNotes[thisOne], 100);
				_vm->_system->delayMillis(200);

				if (!_vm->_bellsAreRinging) { // These handle playing the right tune.
					if (thisOne < lastOne)
						store(kPitchLower, played);
					else if (thisOne == lastOne)
						store(kPitchSame, played);
					else
						store(kPitchHigher, played);
				}

				if (theyMatch(played)) {
					setReadyLight(0);
					_vm->_timer->addTimer(8, Timer::kProcJacquesWakesUp, Timer::kReasonJacquesWakingUp);
					break;
				}
		}
	}

	_vm->_graphics->restoreScreen();
	_vm->_graphics->removeBackup();

	_vm->_animationsEnabled = true;
	CursorMan.showMouse(true);
}

void Dialogs::resetScrollDriver() {
	_scrollBells = 0;
	_currentFont = kFontStyleRoman;
	_useIcon = 0;
	_vm->_interrogation = 0; // Always reset after a scroll comes up.
}

/**
 * Rings the bell x times
 * @remarks	Originally called 'dingdongbell'
 */
void Dialogs::ringBell() {
	for (int i = 0; i < _scrollBells; i++)
		_vm->errorLed(); // Ring the bell "_scrollBells" times.
}

/**
 * This moves the mouse pointer off the scroll so that you can read it.
 * @remarks	Originally called 'dodgem'
 */
void Dialogs::dodgem() {
	_dodgeCoord = _vm->getMousePos();
	g_system->warpMouse(_dodgeCoord.x, _underScroll); // Move the pointer off the scroll.
}

/**
 * This is the opposite of Dodgem.
 * It moves the mouse pointer back, IF you haven't moved it in the meantime.
 * @remarks	Originally called 'undodgem'
 */
void Dialogs::unDodgem() {
	Common::Point actCoord = _vm->getMousePos();
	if ((actCoord.x == _dodgeCoord.x) && (actCoord.y == _underScroll))
		g_system->warpMouse(_dodgeCoord.x, _dodgeCoord.y); // No change, so restore the pointer's original position.
}

void Dialogs::drawScroll(DialogFunctionType modeFunc) {
	int16 lx = 0;
	int16 ly = (_maxLineNum + 1) * 6;
	int16 ex;
	for (int i = 0; i <= _maxLineNum; i++) {
		ex = _scroll[i].size() * 8;
		if (lx < ex)
			lx = ex;
	}
	int16 mx = 320;
	int16 my = 100; // Getmaxx & getmaxy div 2, both.
	lx /= 2;
	ly -= 2;

	if ((1 <= _useIcon) && (_useIcon <= 34))
		lx += kHalfIconWidth;

	CursorMan.showMouse(false);
	_vm->_graphics->drawScroll(mx, lx, my, ly);

	mx -= lx;
	my -= ly + 2;

	bool center = false;

	byte iconIndent = 0;
	switch (_useIcon) {
	default:
	case 0:
		iconIndent = 0;
		break; // No icon.
	case 34:
		_vm->_graphics->drawSign("about", 28, 76, 15);
		iconIndent = 0;
		break;
	case 35:
		_vm->_graphics->drawSign("gameover", 52, 59, 71);
		iconIndent = 0;
		break;
	}

	if ((1 <= _useIcon) && (_useIcon <= 33)) { // Standard icon.
		_vm->_graphics->drawIcon(mx, my + ly / 2, _useIcon);
		iconIndent = 53;
	}

	for (int i = 0; i <= _maxLineNum; i++) {
		if (!_scroll[i].empty())
			switch (_scroll[i][_scroll[i].size() - 1]) {
			case kControlCenter:
				center = true;
				_scroll[i].deleteLastChar();
				break;
			case kControlLeftJustified:
				center = false;
				_scroll[i].deleteLastChar();
				break;
			case kControlQuestion:
				_shadowBoxX = mx + lx;
				_shadowBoxY = my + ly;
				_scroll[i].setChar(' ', 0);
				_vm->_graphics->drawShadowBox(_shadowBoxX - 65, _shadowBoxY - 24, _shadowBoxX - 5, _shadowBoxY - 10, "Yes.");
				_vm->_graphics->drawShadowBox(_shadowBoxX + 5, _shadowBoxY - 24, _shadowBoxX + 65, _shadowBoxY - 10, "No.");
				break;
			default:
				break;
			}

		if (center)
			say(320 - _scroll[i].size() * 4 + iconIndent, my, _scroll[i]);
		else
			say(mx + iconIndent, my, _scroll[i]);

		my += 12;
	}

	_underScroll = (my + 3) * 2; // Multiplying because of the doubled screen height.
	ringBell();

	_vm->_dropsOk = false;
	dodgem();

	(this->*modeFunc)();

	unDodgem();
	_vm->_dropsOk = true;

	resetScrollDriver();
}

void Dialogs::drawBubble(DialogFunctionType modeFunc) {
	Common::Point points[3];

	CursorMan.showMouse(false);
	int16 xl = 0;
	int16 yl = (_maxLineNum + 1) * 5;
	for (int i = 0; i <= _maxLineNum; i++) {
		uint16 textWidth = _scroll[i].size() * 8;
		if (textWidth > xl)
			xl = textWidth;
	}
	xl /= 2;

	int16 xw = xl + 18;
	int16 yw = yl + 7;
	int16 my = yw * 2 - 2;
	int16 xc = 0;

	if (_talkX - xw < 0)
		xc = -(_talkX - xw);
	if (_talkX + xw > 639)
		xc = 639 - (_talkX + xw);

	// Compute triangle coords for the tail of the bubble
	points[0].x = _talkX - 10;
	points[0].y = yw;
	points[1].x = _talkX + 10;
	points[1].y = yw;
	points[2].x = _talkX;
	points[2].y = _talkY;

	_vm->_graphics->prepareBubble(xc, xw, my, points);

	// Draw the text of the bubble. The centering of the text was improved here compared to Pascal's settextjustify().
	// The font is not the same that outtextxy() uses in Pascal. I don't have that, so I used characters instead.
	// It's almost the same, only notable differences are '?', '!', etc.
	for (int i = 0; i <= _maxLineNum; i++) {
		int16 x = xc + _talkX - _scroll[i].size() / 2 * 8;
		bool offset = _scroll[i].size() % 2;
		_vm->_graphics->drawScrollText(_scroll[i], _vm->_font, 8, x - offset * 4, (i * 10) + 12, _vm->_graphics->_talkFontColor);
	}

	ringBell();
	CursorMan.showMouse(false);
	_vm->_dropsOk = false;

	// This does the actual drawing to the screen.
	(this->*modeFunc)();

	_vm->_dropsOk = true;
	CursorMan.showMouse(true); // sink;
	resetScrollDriver();
}

void Dialogs::reset() {
	_maxLineNum = 0;
	for (int i = 0; i < 15; i++) {
		if (!_scroll[i].empty())
			_scroll[i].clear();
	}
}

/**
 * Natural state of bubbles
 * @remarks	Originally called 'natural'
 */
void Dialogs::setBubbleStateNatural() {
	_talkX = 320;
	_talkY = 200;
	_vm->_graphics->setDialogColor(kColorDarkgray, kColorWhite);
}

Common::String Dialogs::displayMoney() {
	Common::String result;

	if (_vm->_money < 12) { // just pence
		result = Common::String::format("%dd", _vm->_money);
	} else if (_vm->_money < 240) { // shillings & pence
		if ((_vm->_money % 12) == 0)
			result = Common::String::format("%d/-", _vm->_money / 12);
		else
			result = Common::String::format("%d/%d", _vm->_money / 12, _vm->_money % 12);
	} else { // L, s & d
		result = Common::String::format("\x9C%d.%d.%d", _vm->_money / 240, (_vm->_money / 12) % 20,
		                _vm->_money % 12);
	}
	if (_vm->_money > 12) {
		Common::String extraStr = Common::String::format(" (that's %dd)", _vm->_money);
		result += extraStr;
	}

	return result;
}

/**
 * Strip trailing character in a string
 * @remarks	Originally called 'strip'
 */
void Dialogs::stripTrailingSpaces(Common::String &str) {
	while (str.lastChar() == ' ')
		str.deleteLastChar();
	// We don't use String::trim() here because we need the leading whitespaces.
}

/**
 * Does the word wrapping.
 */
void Dialogs::solidify(byte n) {
	if (!_scroll[n].contains(' '))
		return; // No spaces.

	// So there MUST be a space there, somewhere...
	do {
		_scroll[n + 1] = _scroll[n][_scroll[n].size() - 1] + _scroll[n + 1];
		_scroll[n].deleteLastChar();
	} while (_scroll[n][_scroll[n].size() - 1] != ' ');

	stripTrailingSpaces(_scroll[n]);
}

/**
 * @remarks	Originally called 'calldriver'
 * Display text by calling the dialog driver. It unifies the function of the original
 * 'calldriver' and 'display' by using Common::String instead of a private buffer.
 */
void Dialogs::displayText(Common::String text) {
	_vm->_sound->stopSound();

	setReadyLight(0);
	_scReturn = false;
	bool mouthnext = false;
	bool callSpriteRun = true; // Only call sprite_run the FIRST time.

	switch (text.lastChar()) {
	case kControlToBuffer:
		text.deleteLastChar();
		break; // ^D = (D)on't include pagebreak
	case kControlSpeechBubble:
	case kControlQuestion:
		break; // ^B = speech (B)ubble, ^Q = (Q)uestion in dialogue box
	default:
		text.insertChar(kControlParagraph, text.size());
	}

	for (uint16 i = 0; i < text.size(); i++) {
		if (mouthnext) {
			if (text[i] == kControlRegister)
				_param = 0;
			else if (('0' <= text[i]) && (text[i] <= '9'))
				_param = text[i] - 48;
			else if (('A' <= text[i]) && (text[i] <= 'Z'))
				_param = text[i] - 55;

			mouthnext = false;
		} else {
			switch (text[i]) {
			case kControlParagraph:
				if ((_maxLineNum == 0) && (_scroll[0].empty()))
					break;

				if (callSpriteRun)
					_vm->spriteRun();
				callSpriteRun = false;

				drawScroll(&Avalanche::Dialogs::scrollModeNormal);

				reset();

				if (_scReturn)
					return;
				break;
			case kControlBell:
				_scrollBells++;
				break;
			case kControlSpeechBubble:
				if ((_maxLineNum == 0) && (_scroll[0].empty()))
					break;

				if (callSpriteRun)
					_vm->spriteRun();
				callSpriteRun = false;

				if (_param == 0)
					setBubbleStateNatural();
				else if ((1 <= _param) && (_param <= 9)) {
					assert(_param - 1 < _vm->_animation->kSpriteNumbMax);
					AnimationType *spr = _vm->_animation->_sprites[_param - 1];
					if ((_param > _vm->_animation->kSpriteNumbMax) || (!spr->_quick)) { // Not valid.
						_vm->errorLed();
						setBubbleStateNatural();
					} else
						spr->chatter(); // Normal sprite talking routine.
				} else if ((10 <= _param) && (_param <= 36)) {
					// Quasi-peds. (This routine performs the same
					// thing with QPs as triptype.chatter does with the
					// sprites.)
					assert(_param - 10 < 16);
					PedType *quasiPed = &_vm->_peds[kQuasipeds[_param - 10]._whichPed];
					_talkX = quasiPed->_x;
					_talkY = quasiPed->_y; // Position.

					_vm->_graphics->setDialogColor(kQuasipeds[_param - 10]._backgroundColor, kQuasipeds[_param - 10]._textColor);
				} else {
					_vm->errorLed(); // Not valid.
					setBubbleStateNatural();
				}

				drawBubble(&Avalanche::Dialogs::scrollModeNormal);

				reset();

				if (_scReturn)
					return;
				break;

			// CHECKME: The whole kControlNegative block seems completely unused, as the only use (the easter egg check) is a false positive
			case kControlNegative:
				switch (_param) {
				case 1:
					displayText(displayMoney() + kControlToBuffer); // Insert cash balance. (Recursion)
					break;
				case 2: {
					int pwdId = _vm->_parser->kFirstPassword + _vm->_passwordNum;
					displayText(_vm->_parser->_vocabulary[pwdId]._word + kControlToBuffer);
					}
					break;
				case 3:
					displayText(_vm->_favoriteDrink + kControlToBuffer);
					break;
				case 4:
					displayText(_vm->_favoriteSong + kControlToBuffer);
					break;
				case 5:
					displayText(_vm->_worstPlaceOnEarth + kControlToBuffer);
					break;
				case 6:
					displayText(_vm->_spareEvening + kControlToBuffer);
					break;
				case 9: {
					Common::String tmpStr = Common::String::format("%d,%d%c",_vm->_catacombX, _vm->_catacombY, kControlToBuffer);
					displayText(tmpStr);
					}
					break;
				case 10:
					switch (_vm->_boxContent) {
					case 0: // Sixpence.
						displayScrollChain('Q', 37); // You find the sixpence.
						_vm->_money += 6;
						_vm->_boxContent = _vm->_parser->kNothing;
						_vm->incScore(2);
						return;
					case Parser::kNothing:
						displayText("nothing at all. It's completely empty.");
						break;
					default:
						displayText(_vm->getItem(_vm->_boxContent) + '.');
					}
					break;
				case 11:
					for (int j = 0; j < kObjectNum; j++) {
						if (_vm->_objects[j])
							displayText(_vm->getItem(j) + ", " + kControlToBuffer);
					}
					break;
				default:
					break;
				}
				break;
			case kControlIcon:
				_useIcon = _param;
				break;
			case kControlNewLine:
				_maxLineNum++;
				break;
			case kControlQuestion:
				if (callSpriteRun)
					_vm->spriteRun();
				callSpriteRun = false;

				_maxLineNum++;
				_scroll[_maxLineNum] = kControlQuestion;

				drawScroll(&Avalanche::Dialogs::scrollModeDialogue);
				reset();
				break;
			case kControlRegister:
				mouthnext = true;
				break;
			case kControlInsertSpaces:
				for (int j = 0; j < 9; j++)
					_scroll[_maxLineNum] += ' ';
				break;
			default: // Add new char.
				if (_scroll[_maxLineNum].size() == 50) {
					solidify(_maxLineNum);
					_maxLineNum++;
				}
				_scroll[_maxLineNum] += text[i];
				break;
			}
		}
	}

	setReadyLight(2);
}

void Dialogs::setTalkPos(int16 x, int16 y) {
	_talkX = x;
	_talkY = y;
}

int16 Dialogs::getTalkPosX() {
	return _talkX;
}

bool Dialogs::displayQuestion(Common::String question) {
	displayText(question + kControlNewLine + kControlQuestion);

	if (_scReturn && (_vm->_rnd->getRandomNumber(1) == 0)) { // Half-and-half chance.
		Common::String tmpStr = Common::String::format("...Positive about that?%cI%c%c%c", kControlRegister, kControlIcon, kControlNewLine, kControlQuestion);
		displayText(tmpStr); // Be annoying!
		if (_scReturn && (_vm->_rnd->getRandomNumber(3) == 3)) { // Another 25% chance
			// \? are used to avoid that ??! is parsed as a trigraph
			tmpStr = Common::String::format("%c100%% certain\?\?!%c%c%c%c", kControlInsertSpaces, kControlInsertSpaces, kControlIcon, kControlNewLine, kControlQuestion);
			displayText(tmpStr); // Be very annoying!
		}
	}

	return _scReturn;
}

void Dialogs::loadFont() {
	Common::File file;

	if (!file.open("avalot.fnt"))
		error("AVALANCHE: Scrolls: File not found: avalot.fnt");

	for (int16 i = 0; i < 256; i++)
		file.read(_fonts[0][i], 16);
	file.close();

	if (!file.open("avitalic.fnt"))
		error("AVALANCHE: Scrolls: File not found: avitalic.fnt");

	for (int16 i = 0; i < 256; i++)
		file.read(_fonts[1][i], 16);
	file.close();

	if (!file.open("ttsmall.fnt"))
		error("AVALANCHE: Scrolls: File not found: ttsmall.fnt");

	for (int16 i = 0; i < 256; i++)
		file.read(_vm->_font[i],16);
	file.close();
}

/**
 * Practically this one is a mini-game which called when you play the harp in the monastery.
 * @remarks	Originally called 'musical_scroll'
 */
void Dialogs::displayMusicalScroll() {
	Common::String tmpStr = Common::String::format("To play the harp...%c%cUse these keys:%c%cQ W E R T Y U I O P [ ]%c%cOr press Enter to stop playing.%c",
		        kControlNewLine, kControlNewLine, kControlNewLine, kControlInsertSpaces, kControlNewLine, kControlNewLine, kControlToBuffer);
	displayText(tmpStr);

	_vm->spriteRun();
	CursorMan.showMouse(false);
	drawScroll(&Avalanche::Dialogs::scrollModeMusic);
	CursorMan.showMouse(true);
	reset();
}

void Dialogs::unSkrimble(Common::String &text) {
	for (uint16  i = 0; i < text.size(); i++)
		text.setChar((~(text[i] - (i + 1))) % 256, i);
}

void Dialogs::doTheBubble(Common::String &text) {
	text.insertChar(kControlSpeechBubble, text.size());
	assert(text.size() < 2000);
}

/**
 * Display a string in a scroll
 * @remarks	Originally called 'dixi'
 */
void Dialogs::displayScrollChain(char block, byte point, bool report, bool bubbling) {
	Common::File indexfile;
	if (!indexfile.open("avalot.idx"))
		error("AVALANCHE: Visa: File not found: avalot.idx");

	bool error = false;

	indexfile.seek((toupper(block) - 'A') * 2);
	uint16 idx_offset = indexfile.readUint16LE();
	if (idx_offset == 0)
		error = true;

	indexfile.seek(idx_offset + point * 2);
	uint16 sez_offset = indexfile.readUint16LE();
	if (sez_offset == 0)
		error = true;

	indexfile.close();

	_noError = !error;

	if (error) {
		if (report) {
			Common::String todisplay = Common::String::format("%cError accessing scroll %c%d", kControlBell, block, point);
			displayText(todisplay);
		}
		return;
	}

	Common::File sezfile;
	if (!sezfile.open("avalot.sez"))
		::error("AVALANCHE: Visa: File not found: avalot.sez");

	sezfile.seek(sez_offset);
	uint16 _bufSize = sezfile.readUint16LE();
	assert(_bufSize < 2000);
	char *_buffer = new char[_bufSize];
	sezfile.read(_buffer, _bufSize);
	sezfile.close();
	Common::String text(_buffer, _bufSize);
	delete[] _buffer;

	unSkrimble(text);
	if (bubbling)
		doTheBubble(text);
	displayText(text);
}

/**
 * Start speech
 * @remarks	Originally called 'speech'
 */
void Dialogs::speak(byte who, byte subject) {
	if (subject == 0) { // No subject.
		displayScrollChain('S', who, false, true);
		return;
	}

	// Subject given.
	_noError = false; // Assume that until we know otherwise.

	Common::File indexfile;
	if (!indexfile.open("converse.avd"))
		error("AVALANCHE: Visa: File not found: converse.avd");

	indexfile.seek(who * 2 - 2);
	uint16 idx_offset = indexfile.readUint16LE();
	uint16 next_idx_offset = indexfile.readUint16LE();

	if ((idx_offset == 0) || ((((next_idx_offset - idx_offset) / 2) - 1) < subject))
		return;

	indexfile.seek(idx_offset + subject * 2);
	uint16 sezOffset = indexfile.readUint16LE();
	if ((sezOffset == 0) || (indexfile.err()))
		return;
	indexfile.close();

	Common::File sezfile;
	if (!sezfile.open("avalot.sez"))
		error("AVALANCHE: Visa: File not found: avalot.sez");

	sezfile.seek(sezOffset);
	uint16 _bufSize = sezfile.readUint16LE();
	assert(_bufSize < 2000);
	char *_buffer = new char[_bufSize];
	sezfile.read(_buffer, _bufSize);
	sezfile.close();
	Common::String text(_buffer, _bufSize);
	delete[] _buffer;

	unSkrimble(text);
	doTheBubble(text);
	displayText(text);

	_noError = true;
}

void Dialogs::talkTo(byte whom) {
	if (_vm->_parser->_person == kPeoplePardon) {
		_vm->_parser->_person = (People)_vm->_subjectNum;
		_vm->_subjectNum = 0;
	}

	if (_vm->_subjectNum == 0) {
		switch (whom) {
		case kPeopleSpludwick:
			if ((_vm->_lustieIsAsleep) & (!_vm->_objects[kObjectPotion - 1])) {
				displayScrollChain('Q', 68);
				_vm->_objects[kObjectPotion - 1] = true;
				_vm->refreshObjectList();
				_vm->incScore(3);
				return;
			} else if (_vm->_talkedToCrapulus) {
				// Spludwick - what does he need?
				// 0 - let it through to use normal routine.
				switch (_vm->_givenToSpludwick) {
				case 1: // Fallthrough is intended.
				case 2: {
					Common::String objStr = _vm->getItem(AvalancheEngine::kSpludwicksOrder[_vm->_givenToSpludwick]);
					Common::String tmpStr = Common::String::format("Can you get me %s, please?%c2%c",
						objStr.c_str(), kControlRegister, kControlSpeechBubble);
					displayText(tmpStr);
					}
					return;
				case 3:
					displayScrollChain('Q', 30); // Need any help with the game?
					return;
				default:
					break;
				}
			} else {
				displayScrollChain('Q', 42); // Haven't talked to Crapulus. Go and talk to him.
				return;
			}
			break;
		case kPeopleIbythneth:
			if (_vm->_givenBadgeToIby) {
				displayScrollChain('Q', 33); // Thanks a lot!
				return; // And leave the proc.
			}
			break; // Or... just continue, 'cos he hasn't got it.
		case kPeopleDogfood:
			if (_vm->_wonNim) { // We've won the game.
				displayScrollChain('Q', 6); // "I'm Not Playing!"
				return; // Zap back.
			} else
				_vm->_askedDogfoodAboutNim = true;
			break;
		case kPeopleAyles:
			if (!_vm->_aylesIsAwake) {
				displayScrollChain('Q', 43); // He's fast asleep!
				return;
			} else if (!_vm->_givenPenToAyles) {
				displayScrollChain('Q', 44); // Can you get me a pen, Avvy?
				return;
			}
			break;

		case kPeopleJacques:
			displayScrollChain('Q', 43);
			return;

		case kPeopleGeida:
			if (_vm->_givenPotionToGeida)
				_vm->_geidaFollows = true;
			else {
				displayScrollChain('U', 17);
				return;
			}
			break;
		case kPeopleSpurge:
			if (!_vm->_sittingInPub) {
				displayScrollChain('Q', 71); // Try going over and sitting down.
				return;
			} else {
				if (_vm->_spurgeTalkCount < 5)
					_vm->_spurgeTalkCount++;
				if (_vm->_spurgeTalkCount > 1) { // no. 1 falls through
					displayScrollChain('Q', 70 + _vm->_spurgeTalkCount);
					return;
				}
			}
			break;

		default:
			break;
		}
	// On a subject. Is there any reason to block it?
	} else if ((whom == kPeopleAyles) && (!_vm->_aylesIsAwake)) {
		displayScrollChain('Q', 43); // He's fast asleep!
		return;
	}

	if (whom > 149)
		whom -= 149;

	bool noMatches = true;
	for (int i = 0; i < _vm->_animation->kSpriteNumbMax; i++) {
		if (_vm->_animation->_sprites[i]->_characterId == whom) {
			Common::String tmpStr = Common::String::format("%c%c%c", kControlRegister, i + 49, kControlToBuffer);
			displayText(tmpStr);
			noMatches = false;
			break;
		}
	}

	if (noMatches) {
		Common::String tmpStr = Common::String::format("%c%c%c", kControlRegister, kControlRegister, kControlToBuffer);
		displayText(tmpStr);
	}

	speak(whom, _vm->_subjectNum);

	if (!_noError)
		displayScrollChain('N', whom); // File not found!

	if ((_vm->_subjectNum == 0) && ((whom + 149) == kPeopleCrapulus)) { // Crapulus: get the badge - first time only
		_vm->_objects[kObjectBadge - 1] = true;
		_vm->refreshObjectList();
		displayScrollChain('Q', 1); // Circular from Cardiff.
		_vm->_talkedToCrapulus = true;
		_vm->setRoom(kPeopleCrapulus, kRoomDummy); // Crapulus walks off.

		AnimationType *spr = _vm->_animation->_sprites[1];
		spr->_vanishIfStill = true;
		spr->walkTo(2); // Walks away.

		_vm->incScore(2);
	}
}

/**
 * This makes Avalot say the response.
 * @remarks	Originally called 'sayit'
 */
void Dialogs::sayIt(Common::String str) {
	Common::String x = str;
	x.setChar(toupper(x[0]), 0);
	Common::String tmpStr = Common::String::format("%c1%s.%c%c2", kControlRegister, x.c_str(), kControlSpeechBubble, kControlRegister);
	displayText(tmpStr);
}

Common::String Dialogs::personSpeaks() {
	if ((_vm->_parser->_person == kPeoplePardon) || (_vm->_parser->_person == kPeopleNone)) {
		if ((_vm->_him == kPeoplePardon) || (_vm->getRoom(_vm->_him) != _vm->_room))
			_vm->_parser->_person = _vm->_her;
		else
			_vm->_parser->_person = _vm->_him;
	}

	if (_vm->getRoom(_vm->_parser->_person) != _vm->_room) {
		return Common::String::format("%c1", kControlRegister); // Avvy himself!
	}

	bool found = false; // The _person we're looking for's code is in _person.
	Common::String tmpStr;

	for (int i = 0; i < _vm->_animation->kSpriteNumbMax; i++) {
		AnimationType *curSpr = _vm->_animation->_sprites[i];
		if (curSpr->_quick && (curSpr->_characterId + 149 == _vm->_parser->_person)) {
			tmpStr += Common::String::format("%c%c", kControlRegister, '1' + i);
			found = true;
		}
	}

	if (found)
		return tmpStr;

	for (int i = 0; i < 16; i++) {
		if ((kQuasipeds[i]._who == _vm->_parser->_person) && (kQuasipeds[i]._room == _vm->_room))
			tmpStr += Common::String::format("%c%c", kControlRegister, 'A' + i);
	}

	return tmpStr;
}

/**
 * Display a message when (uselessly) giving an object away
 * @remarks	Originally called 'heythanks'
 */
void Dialogs::sayThanks(byte thing) {
	Common::String tmpStr = personSpeaks();
	tmpStr += Common::String::format("Hey, thanks!%c(But now, you've lost it!)", kControlSpeechBubble);
	displayText(tmpStr);

	if (thing < kObjectNum)
		_vm->_objects[thing] = false;
}

/**
 * Display a 'Hello' message
 */
void Dialogs::sayHello() {
	Common::String tmpStr = personSpeaks();
	tmpStr += Common::String::format("Hello.%c", kControlSpeechBubble);
	displayText(tmpStr);
}

/**
 * Display a 'OK' message
 */
void Dialogs::sayOK() {
	Common::String tmpStr = personSpeaks();
	tmpStr += Common::String::format("That's OK.%c", kControlSpeechBubble);
	displayText(tmpStr);
}

/**
 * Display a 'Silly' message
 * @remarks	Originally called 'silly'
 */
void Dialogs::saySilly() {
	displayText("Don't be silly!");
}

} // End of namespace Avalanche
