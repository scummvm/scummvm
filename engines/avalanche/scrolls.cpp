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

#include "avalanche/avalanche.h"

#include "avalanche/scrolls.h"
#include "avalanche/gyro.h"
#include "avalanche/lucerna.h"
#include "avalanche/animation.h"
#include "avalanche/acci.h"
#include "avalanche/timer.h"

#include "common/textconsole.h"
#include "common/file.h"

namespace Avalanche {

Scrolls::Scrolls(AvalancheEngine *vm) {
	_vm = vm;
	_noError = true;
}

void Scrolls::init() {
	loadFont();
	resetScrollDriver();
}

void Scrolls::setReadyLight(byte state) {     // Sets "Ready" light to whatever
	if (_vm->_gyro->_ledStatus == state)
		return; // Already like that!

	byte color = kColorBlack;
	switch (state) {
	case 0:
		color = kColorBlack;
		break; // Off
	case 1:
	case 2:
	case 3:
		color = kColorGreen;
		break; // Hit a key
	}
	warning("STUB: Scrolls::state()");

	CursorMan.showMouse(false);

	_vm->_graphics->_surface.fillRect(Common::Rect(419, 195, 438, 197), color);

	CursorMan.showMouse(true);
	_vm->_gyro->_ledStatus = state;
}

void Scrolls::easterEgg() {
	warning("STUB: Scrolls::easterEgg()");
}

void Scrolls::say(int16 x, int16 y, Common::String z) {
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
				itw[(byte)z[xx]][yy] = _scrollFonts[_currentFont][(byte)z[xx]][yy + 2];

			// We have to draw the characters one-by-one because of the accidental font changes.
			i++;
			Common::String chr(z[xx]);
			_vm->_graphics->drawText(_vm->_graphics->_scrolls, chr, itw, 12, (x - 1) * 8 + offset * 4 + i * 8, y, kColorBlack);
			}
		}
	}
}

void Scrolls::scrollModeNormal() {
	// Original code is:
	// egg : array[1..8] of char = ^P^L^U^G^H+'***';
	// this is not using kControl characters: it's the secret code to be entered to trigger the easter egg
	// TODO: To be fixed when the Easter egg code is implemented
	Common::String egg = Common::String::format("%c%c%c%c%c***", kControlParagraph, kControlLeftJustified, kControlNegative, kControlBell, kControlBackspace);
	Common::String e = "(c) 1994";

	setReadyLight(3);
	_vm->_gyro->_seeScroll = true;
	CursorMan.showMouse(true);
	_vm->_gyro->newMouse(4);

	::Graphics::Surface temp;
	temp.copyFrom(_vm->_graphics->_surface);
	_vm->_graphics->_surface.copyFrom(_vm->_graphics->_scrolls); // TODO: Rework it using getSubArea !!!!!!!

	Common::Event event;
	while (!_vm->shouldQuit()) {
		_vm->_graphics->refreshScreen();

		_vm->getEvent(event);
		if ((event.type == Common::EVENT_LBUTTONUP) ||
			((event.type == Common::EVENT_KEYDOWN) && ((event.kbd.keycode == Common::KEYCODE_ESCAPE) || (event.kbd.keycode == Common::KEYCODE_RETURN) || (event.kbd.keycode == Common::KEYCODE_HASH) || (event.kbd.keycode == Common::KEYCODE_PLUS))))
			break;
	}

	_vm->_graphics->_surface.copyFrom(temp);
	temp.free();

	warning("STUB: scrollModeNormal() - Check Easter Egg trigger");
#if 0
	char r;
	bool oktoexit;
	do {
		do {
			_vm->_gyro->check(); // was "checkclick;"

//#ifdef RECORD slowdown(); basher::count += 1; #endif

			if (_vm->_gyro->demo) {
				if (_vm->_basher->demo_ready())
					break;
				if (_vm->_enhanced->keypressede())
					return;
			} else if (_vm->_enhanced->keypressede())
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

	_vm->_gyro->screturn = r == '#'; // "back door"
#endif

	setReadyLight(0);
	_vm->_gyro->_seeScroll = false;
	CursorMan.showMouse(false);
	_vm->_lucerna->_holdLeftMouse = false; // Used in Lucerna::checkclick().

	warning("STUB: Scrolls::scrollModeNormal()");
}

void Scrolls::scrollModeDialogue() {
	warning("STUB: Scrolls::scrollModeDialogue()");
}

void Scrolls::store(byte what, TuneType &played) {
	memcpy(played + 1, played + 2, sizeof(played) - 1);
	played[30] = what;
}

bool Scrolls::theyMatch(TuneType &played) {
	byte mistakes = 0;

	for (int i = 0; i < sizeof(played); i++) {
		if (played[i] != _vm->_gyro->kTune[i]) {
			mistakes += 1;
		}
	}

	return mistakes < 5;
}

void Scrolls::scrollModeMusic() {
	setReadyLight(3);
	_vm->_gyro->_seeScroll = true;
	CursorMan.showMouse(true);
	_vm->_gyro->newMouse(4);

	// Since there are no sounds in the game yet, it's pretty pointless to implement this function further.
	// For now we act like the player just played the right tone.
#if 0
	if (they_match(played)) {
#endif
		_vm->_gyro->_scReturn = true;
		CursorMan.showMouse(false);
		setReadyLight(0);
		_vm->_gyro->_seeScroll = false;

		_vm->_timer->addTimer(8, Timer::kProcJacquesWakesUp, Timer::kReasonJacquesWakingUp);
		warning("STUB: Scrolls::music_scroll()");
		return;
#if 0
	}

	_vm->_gyro->screturn = false;
	CursorMan.showMouse(false);
	state(0);
	_vm->_gyro->seescroll = false;
#endif
}

void Scrolls::resetScrollDriver() {
	_vm->_gyro->_scrollBells = 0;
	_currentFont = kFontStyleRoman;
	_useIcon = 0;
	_vm->_gyro->_interrogation = 0; // Always reset after a scroll comes up.
}

void Scrolls::ringBell() {   // Pussy's in the well. Who put her in? Little...
	for (int i = 0; i < _vm->_gyro->_scrollBells; i++)
		_vm->_lucerna->errorLed(); // Ring the bell "x" times.
}

void Scrolls::dodgem() {
	_dodgeCoord = _vm->getMousePos();
	g_system->warpMouse(_dodgeCoord.x, _vm->_gyro->_underScroll); // Move the pointer off the scroll.
}

void Scrolls::unDodgem() {
	Common::Point actCoord = _vm->getMousePos();
	if ((actCoord.x == _dodgeCoord.x) && (actCoord.y == _vm->_gyro->_underScroll))
		g_system->warpMouse(_dodgeCoord.x, _dodgeCoord.y); // No change, so restore the pointer's original position.
}

void Scrolls::getIcon(int16 x, int16 y, byte which) {
	Common::File file;

	if (!file.open("icons.avd"))
		error("AVALANCHE: Scrolls: File not found: icons.avd");

	which--;
	file.seek(which * 426);

	byte *p = new byte[426];
	file.read(p, 426);

	//putimage(x, y, p, 0);
	warning("STUB: Scrolls::getIcon()");

	delete[] p;
	file.close();
}

void Scrolls::drawSign(Common::String fn, int16 xl, int16 yl, int16 y) {
	Common::File file;
	Common::String filename = Common::String::format("%s.avd", fn.c_str());

	if (!file.open(filename))
		error("AVALANCHE: Scrolls: File not found: %s", filename.c_str());

#if 0
	uint16 st = (y - 1) * 80 + (40 - xl / 2) + ((1 - _vm->_gyro->cp) * _vm->_gyro->pagetop);
	byte bit;
	for (uint16 i = 1; i <= yl; i++)
		for (bit = 0; bit <= 3; bit++) {
			port[0x3c4] = 2;
			port[0x3ce] = 4;
			port[0x3c5] = 1 << bit;
			port[0x3cf] = bit;
			blockread(f, mem[0xa000 * st + (i * 80)], xl);
		}
	bit = getpixel(0, 0);
#endif

	warning("STUB: Scrolls::drawSign()");

	file.close();
}

void Scrolls::drawScroll(ScrollsFunctionType modeFunc) {
	int16 ex;

	//setvisualpage(cp);
	//setactivepage(1 - cp);
	_vm->_gyro->_onCanDoPageSwap = false;  // On can now no longer swap pages. So we can do what we want without its interference!

	int16 lx = 0;
	int16 ly = (_vm->_gyro->_scrollNum) * 6;
	for (int i = 0; i < _vm->_gyro->_scrollNum; i++) {
		ex = _vm->_gyro->_scroll[i].size() * 8;
		if (lx < ex)
			lx = ex;
	}
	int16 mx = 320;
	int16 my = 100; // Getmaxx & getmaxy div 2, both.
	lx = lx / 2;
	ly -= 2;

	if ((1 <= _useIcon) && (_useIcon <= 34))
		lx += kHalfIconWidth;

	_vm->_graphics->_scrolls.copyFrom(_vm->_graphics->_surface);

	CursorMan.showMouse(false);

	// The right corners of the scroll.
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, mx + lx, my - ly, 0, 90, 15, kColorLightgray);
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, mx + lx, my + ly, 270, 360, 15, kColorLightgray);
	_vm->_graphics->drawArc(_vm->_graphics->_scrolls, mx + lx, my - ly, 0, 90, 15, kColorRed);
	_vm->_graphics->drawArc(_vm->_graphics->_scrolls, mx + lx, my + ly, 270, 360, 15, kColorRed);

	// The body of the scroll.
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx - lx - 30, my + ly, mx + lx, my + ly + 6), kColorLightgray);
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx - lx - 30, my - ly - 6, mx + lx, my - ly + 1), kColorLightgray);
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx - lx - 15, my - ly, mx + lx + 15, my + ly + 1), kColorLightgray);

	// The left corners of the scroll.
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, mx - lx - 31, my - ly, 0, 180, 15, kColorDarkgray);
	_vm->_graphics->drawArc(_vm->_graphics->_scrolls, mx - lx - 31, my - ly, 0, 180, 15, kColorRed);
	_vm->_graphics->_scrolls.drawLine(mx - lx - 31 - 15, my - ly, mx - lx - 31 + 15, my - ly, kColorRed);
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, mx - lx - 31, my + ly, 180, 360, 15, kColorDarkgray);
	_vm->_graphics->drawArc(_vm->_graphics->_scrolls, mx - lx - 31, my + ly, 180, 360, 15, kColorRed);
	_vm->_graphics->_scrolls.drawLine(mx - lx - 31 - 15, my + ly, mx - lx - 31 + 15, my + ly, kColorRed);

	// The rear borders of the scroll.
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx - lx - 30, my - ly - 6, mx + lx, my - ly - 5), kColorRed);
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx - lx - 30, my + ly + 6, mx + lx, my + ly + 7), kColorRed);
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx - lx - 15, my - ly, mx - lx - 14, my + ly), kColorRed);
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx + lx + 15, my - ly, mx + lx + 16, my + ly), kColorRed);

//	CHECKME: unused?
//	ex = mx - lx;
//	int16 ey = my - ly;
	mx -= lx;
	my -= ly + 2;

	bool centre = false;

	byte iconIndent = 0;
	switch (_useIcon) {
	case 0:
		iconIndent = 0;
		break; // No icon.
	case 34:
		drawSign("about", 28, 76, 15);
		iconIndent = 0;
		break;
	case 35:
		drawSign("gameover", 52, 59, 71);
		iconIndent = 0;
		break;
	}

	if ((1 <= _useIcon) && (_useIcon <= 33)) { // Standard icon.
		getIcon(mx, my + ly / 2, _useIcon);
		iconIndent = 53;
	}


	for (int i = 0; i < _vm->_gyro->_scrollNum; i++) {
		if (!_vm->_gyro->_scroll[i].empty())
			switch (_vm->_gyro->_scroll[i][_vm->_gyro->_scroll[i].size() - 1]) {
			case kControlCenter:
				centre = true;
				_vm->_gyro->_scroll[i].deleteLastChar();
				break;
			case kControlLeftJustified:
				centre = false;
				_vm->_gyro->_scroll[i].deleteLastChar();
				break;
			case kControlQuestion:
				//settextjustify(1, 1);
				_shadowBoxX = mx + lx;
				_shadowBoxY = my + ly;
				_vm->_gyro->_scroll[i].setChar(' ', 0);
				// byte groi = *_vm->_graphics->getPixel(0, 0);
				// inc(diy,14);
				_vm->_gyro->drawShadowBox(_shadowBoxX - 65, _shadowBoxY - 24, _shadowBoxX - 5, _shadowBoxY - 10, "Yes.");
				_vm->_gyro->drawShadowBox(_shadowBoxX + 5, _shadowBoxY - 24, _shadowBoxX + 65, _shadowBoxY - 10, "No.");
				break;
			}

		if (centre)
			say(320 - _vm->_gyro->_scroll[i].size() * 4 + iconIndent, my, _vm->_gyro->_scroll[i]);
		else
			say(mx + iconIndent, my, _vm->_gyro->_scroll[i]);

		my += 12;
	}

	_vm->_gyro->_underScroll = my * 2 + 6; // Multiplying because of the doubled screen height.
	//setvisualpage(1 - cp);
	ringBell();
	//my = getpixel(0, 0);
	_vm->_gyro->_dropsOk = false;
	dodgem();

	(this->*modeFunc)();

	unDodgem();
	_vm->_gyro->_dropsOk = true;
	//setvisualpage(cp);
	//mousepage(cp);
	CursorMan.showMouse(false);
	// mblit(ex-46,ey-6,ex+lx*2+15,ey+ly*2+6,3,0);
	//mblit((ex - 46) / 8, ey - 6, 1 + (ex + lx * 2 + 15) / 8, ey + ly * 2 + 6, cp, 1 - cp);
	//blitfix();
	_vm->_gyro->_onCanDoPageSwap = true; // Normality again.
	CursorMan.showMouse(true);
	//settextjustify(0, 0); // sink
	resetScrollDriver();
	/*if (_vm->_gyro->mpress > 0)
	_vm->_gyro->after_the_scroll = true;*/

	warning("STUB: Scrolls::drawScroll()");
}

void Scrolls::drawBubble(ScrollsFunctionType modeFunc) {
	Common::Point points[3];
//	byte *rp1, *rp2; // replace: 1=bubble, 2=pointer
	int16 xc; // x correction

	//setvisualpage(cp);
	//setactivepage(1 - cp);
	_vm->_gyro->_onCanDoPageSwap = false;  // On can now no longer swap pages. So we can do what we want without its interference!
	//mousepage(1 - cp); // Mousepage

	CursorMan.showMouse(false);

	int16 xl = 0;
	int16 yl = _vm->_gyro->_scrollNum * 5;
	for (int i = 0; i < _vm->_gyro->_scrollNum; i++) {
		uint16 textWidth = _vm->_gyro->_scroll[i].size() * 8;
		if (textWidth > xl)
			xl = textWidth;
	}
	xl /= 2;

	int16 xw = xl + 18;
	int16 yw = yl + 7;
	int16 my = yw * 2 - 2;
	xc = 0;

	if ((_vm->_gyro->_talkX - xw) < 0)
		xc = -(_vm->_gyro->_talkX - xw);
	if ((_vm->_gyro->_talkX + xw) > 639)
		xc = 639 - (_vm->_gyro->_talkX + xw);

	points[0].x = _vm->_gyro->_talkX - 10;
	points[0].y = yw;
	points[1].x = _vm->_gyro->_talkX + 10;
	points[1].y = yw;
	points[2].x = _vm->_gyro->_talkX;
	points[2].y = _vm->_gyro->_talkY;

	// Backup the screen before drawing the bubble.
	_vm->_graphics->_scrolls.copyFrom(_vm->_graphics->_surface);

	// The body of the bubble.
	_vm->_graphics->_scrolls.fillRect(Common::Rect(xc + _vm->_gyro->_talkX - xw + 9, 7, _vm->_gyro->_talkX + xw - 8 + xc, my + 1), _vm->_gyro->_talkBackgroundColor);
	_vm->_graphics->_scrolls.fillRect(Common::Rect(xc + _vm->_gyro->_talkX - xw - 1, 12, _vm->_gyro->_talkX + xw + xc + 2, my - 4), _vm->_gyro->_talkBackgroundColor);

	// Top right corner of the bubble.
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, xc + _vm->_gyro->_talkX + xw - 10, 11, 0, 90, 9, _vm->_gyro->_talkBackgroundColor);
	// Bottom right corner of the bubble.
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, xc + _vm->_gyro->_talkX + xw - 10, my - 4, 270, 360, 9, _vm->_gyro->_talkBackgroundColor);
	// Top left corner of the bubble.
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, xc + _vm->_gyro->_talkX - xw + 10, 11, 90, 180, 9, _vm->_gyro->_talkBackgroundColor);
	// Bottom left corner of the bubble.
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, xc + _vm->_gyro->_talkX - xw + 10, my - 4, 180, 270, 9, _vm->_gyro->_talkBackgroundColor);

	// "Tail" of the speech bubble.
	_vm->_graphics->drawTriangle(_vm->_graphics->_scrolls, points, _vm->_gyro->_talkBackgroundColor);


	// CHECKME: Unused?
	// yl -= 3;

	// Draw the text of the bubble. The centering of the text was improved here compared to Pascal's settextjustify().
	// The font is not the same that outtextxy() uses in Pascal. I don't have that, so I used Gyro::characters instead.
	// It's almost the same, only notable differences are '?', '!', etc.
	for (int i = 0; i < _vm->_gyro->_scrollNum; i++) {
		int16 x = xc + _vm->_gyro->_talkX - _vm->_gyro->_scroll[i].size() / 2 * 8;
		bool offset = _vm->_gyro->_scroll[i].size() % 2;
		_vm->_graphics->drawText(_vm->_graphics->_scrolls, _vm->_gyro->_scroll[i], _vm->_gyro->_font, 8, x - offset * 4, (i * 10) + 12, _vm->_gyro->_talkFontColor);
	}

	//setvisualpage(1 - cp);
	ringBell();
	_vm->_gyro->_onCanDoPageSwap = false;
	CursorMan.showMouse(true);
	_vm->_gyro->_dropsOk = false;

	// This does the actual drawing to the screen.
	(this->*modeFunc)();

	CursorMan.showMouse(false);
	_vm->_gyro->_dropsOk = true;

	//setvisualpage(cp);
	CursorMan.showMouse(true); // sink;
	_vm->_gyro->_onCanDoPageSwap = true;
	resetScrollDriver();
	/*if (_vm->_gyro->mpress > 0)
	_vm->_gyro->after_the_scroll = true;*/
}

bool Scrolls::displayQuestion(Common::String question) {
	warning("STUB: Scrolls::displayQuestion()");
	return true;
}

void Scrolls::resetScroll() {
	_vm->_gyro->_scrollNum = 1;
	for (int i = 0; i < 15; i++)
		if (!_vm->_gyro->_scroll[i].empty())
			_vm->_gyro->_scroll[i].clear();
}

void Scrolls::setBubbleStateNatural() {
	_vm->_gyro->_talkX = 320;
	_vm->_gyro->_talkY = 200;
	_vm->_gyro->_talkBackgroundColor = 8;
	_vm->_gyro->_talkFontColor = 15;
}

Common::String Scrolls::displayMoney() {
	Common::String result;

	if (_vm->_gyro->_money < 12) { // just pence
		result = Common::String::format("%dd", _vm->_gyro->_money);
	} else if (_vm->_gyro->_money < 240) { // shillings & pence
		if ((_vm->_gyro->_money % 12) == 0)
			result = Common::String::format("%d/-", _vm->_gyro->_money / 12);
		else
			result = Common::String::format("%d/%d", _vm->_gyro->_money / 12, _vm->_gyro->_money % 12);
	} else { // L, s & d
		result = Common::String::format("\x9C%d.%d.%d", _vm->_gyro->_money / 240, (_vm->_gyro->_money / 12) % 20, 
		                _vm->_gyro->_money % 12);
	}
	if (_vm->_gyro->_money > 12) {
		Common::String extraStr = Common::String::format(" (that's %dd)", _vm->_gyro->_money);
		result += extraStr;
	}

	return result;
}

void Scrolls::stripTrailingSpaces(Common::String &str) {
	while (str[str.size() - 1] == ' ') {
		str.deleteLastChar();
	}
}

void Scrolls::solidify(byte n) {
	if (!_vm->_gyro->_scroll[n].contains(' '))
		return; // No spaces.

	// So there MUST be a space there, somewhere...
	do {
		_vm->_gyro->_scroll[n + 1] = _vm->_gyro->_scroll[n][_vm->_gyro->_scroll[n].size() - 1] + _vm->_gyro->_scroll[n + 1];
		_vm->_gyro->_scroll[n].deleteLastChar();
	} while (_vm->_gyro->_scroll[n][_vm->_gyro->_scroll[n].size() - 1] != ' ');

	stripTrailingSpaces(_vm->_gyro->_scroll[n]);
}

void Scrolls::callScrollDriver() {
//	bool was_virtual; // Was the mouse cursor virtual on entry to this proc?


	//nosound();
	warning("STUB: Scrolls::calldrivers()");

	setReadyLight(0);
	_vm->_gyro->_scReturn = false;
	bool mouthnext = false;
	bool call_spriterun = true; // Only call sprite_run the FIRST time.

	switch (_vm->_gyro->_buffer[_vm->_gyro->_bufSize - 1]) {
	case kControlToBuffer:
		_vm->_gyro->_bufSize--;
		break; // ^D = (D)on't include pagebreak
	case kControlSpeechBubble:
	case kControlQuestion:
		break; // ^B = speech (B)ubble, ^Q = (Q)uestion in dialogue box
	default:
		_vm->_gyro->_buffer[_vm->_gyro->_bufSize] = kControlParagraph;
		_vm->_gyro->_bufSize++;
	}

	uint16 size = _vm->_gyro->_bufSize;

	for (uint16 i = 0; i < size; i++) {
		if (mouthnext) {
			if (_vm->_gyro->_buffer[i] == kControlRegister)
				_param = 0;
			else if (('0' <= _vm->_gyro->_buffer[i]) && (_vm->_gyro->_buffer[i] <= '9'))
				_param = _vm->_gyro->_buffer[i] - 48;
			else if (('A' <= _vm->_gyro->_buffer[i]) && (_vm->_gyro->_buffer[i] <= 'Z'))
				_param = _vm->_gyro->_buffer[i] - 55;

			mouthnext = false;
		} else {
			switch (_vm->_gyro->_buffer[i]) {
			case kControlParagraph:
				if ((_vm->_gyro->_scrollNum == 1) && (_vm->_gyro->_scroll[0].empty()))
					break;

				if (call_spriterun)
					_vm->_lucerna->spriteRun();
				call_spriterun = false;

				drawScroll(&Avalanche::Scrolls::scrollModeNormal);

				resetScroll();

				if (_vm->_gyro->_scReturn)
					return;
				break;
			case kControlBell:
				_vm->_gyro->_scrollBells++;
				break; // #7 = "Bel"
			case kControlSpeechBubble:
				if ((_vm->_gyro->_scrollNum == 1) && (_vm->_gyro->_scroll[0].empty()))
					break;

				if (call_spriterun)
					_vm->_lucerna->spriteRun();
				call_spriterun = false;

				if (_param == 0)
					setBubbleStateNatural();
				else if ((1 <= _param) && (_param <= 9)) {
					AnimationType *spr = &_vm->_animation->_sprites[_param - 1];
					if ((_param > _vm->_animation->kSpriteNumbMax) || (!spr->_quick)) { // Not valid.
						_vm->_lucerna->errorLed();
						setBubbleStateNatural();
					} else
						spr->chatter(); // Normal sprite talking routine.
				} else if ((10 <= _param) && (_param <= 36)) {
					// Quasi-peds. (This routine performs the same
					// thing with QPs as triptype.chatter does with the
					// sprites.)
					PedType *quasiPed = &_vm->_gyro->_peds[_vm->_gyro->kQuasipeds[_param - 10]._whichPed];
					_vm->_gyro->_talkX = quasiPed->_x;
					_vm->_gyro->_talkY = quasiPed->_y; // Position.

					_vm->_gyro->_talkFontColor = _vm->_gyro->kQuasipeds[_param - 10]._foregroundColor;
					_vm->_gyro->_talkBackgroundColor = _vm->_gyro->kQuasipeds[_param - 10]._backgroundColor; // Colors.
				} else {
					_vm->_lucerna->errorLed(); // Not valid.
					setBubbleStateNatural();
				}

				drawBubble(&Avalanche::Scrolls::scrollModeNormal);

				resetScroll();

				if (_vm->_gyro->_scReturn)
					return;
				break;

			// CHECME: The whole kControlNegative block seems completely unused, as the only use (the easter egg check) is a false positive 
			case kControlNegative:
				switch (_param) {
				case 1:
					displayText(displayMoney() + kControlToBuffer); // Insert cash balance. (Recursion)
					break;
				case 2: {
					int pwdId = _vm->_acci->kFirstPassword + _vm->_gyro->_passwordNum;
					displayText(_vm->_acci->_vocabulary[pwdId]._word + kControlToBuffer);
					}
					break;
				case 3:
					displayText(_vm->_gyro->_favouriteDrink + kControlToBuffer);
					break;
				case 4:
					displayText(_vm->_gyro->_favouriteSong + kControlToBuffer);
					break;
				case 5:
					displayText(_vm->_gyro->_worstPlaceOnEarth + kControlToBuffer);
					break;
				case 6:
					displayText(_vm->_gyro->_spareEvening + kControlToBuffer);
					break;
				case 9: {
					Common::String tmpStr = Common::String::format("%d,%d%c",_vm->_gyro->_catacombX, _vm->_gyro->_catacombY, kControlToBuffer);
					displayText(tmpStr);
					}
					break;
				case 10:
					switch (_vm->_gyro->_boxContent) {
					case 0: // Sixpence.
						_vm->_scrolls->displayScrollChain('q', 37); // You find the sixpence.
						_vm->_gyro->_money += 6;
						_vm->_gyro->_boxContent = _vm->_acci->kNothing;
						_vm->_lucerna->incScore(2);
						return;
					case Acci::kNothing:
						displayText("nothing at all. It's completely empty.");
						break;
					default:
						displayText(_vm->_gyro->getItem(_vm->_gyro->_boxContent) + '.');
					}
					break;
				case 11:
					for (int j = 0; j < kObjectNum; j++) {
						if (_vm->_gyro->_objects[j]) 
							displayText(_vm->_gyro->getItem(j) + ", " + kControlToBuffer);
					}
					break;
				}
				break;
			case kControlIcon:
				_useIcon = _param;
				break;
			case kControlNewLine:
				_vm->_gyro->_scrollNum++;
				break;
			case kControlQuestion:
				if (call_spriterun)
					_vm->_lucerna->spriteRun();
				call_spriterun = false;

				_vm->_gyro->_scroll[_vm->_gyro->_scrollNum] = kControlQuestion;
				_vm->_gyro->_scrollNum++;

				drawScroll(&Avalanche::Scrolls::scrollModeDialogue);
				resetScroll();
				break;
			case kControlRegister:
				mouthnext = true;
				break;
			case kControlInsertSpaces:
				for (int j = 0; j < 9; j++)
					_vm->_gyro->_scroll[_vm->_gyro->_scrollNum - 1] += ' ';
				break;
			default: // Add new char.
				if (_vm->_gyro->_scroll[_vm->_gyro->_scrollNum - 1].size() == 50) {
					solidify(_vm->_gyro->_scrollNum - 1);
					_vm->_gyro->_scrollNum++;
				}
				_vm->_gyro->_scroll[_vm->_gyro->_scrollNum - 1] += _vm->_gyro->_buffer[i];
				break;
			}
		}
	}
}

void Scrolls::displayText(Common::String text) { // TODO: REPLACE BUFFER WITH A STRING!!!!!!!!!!
	_vm->_gyro->_bufSize = text.size();
	memcpy(_vm->_gyro->_buffer, text.c_str(), _vm->_gyro->_bufSize);
	callScrollDriver();
}

void Scrolls::loadFont() {
	Common::File file;

	if (!file.open("avalot.fnt"))
		error("AVALANCHE: Scrolls: File not found: avalot.fnt");

	for (int16 i = 0; i < 256; i++)
		file.read(_scrollFonts[0][i], 16);
	file.close();

	if (!file.open("avitalic.fnt"))
		error("AVALANCHE: Scrolls: File not found: avitalic.fnt");

	for (int16 i = 0; i < 256; i++)
		file.read(_scrollFonts[1][i], 16);
	file.close();

	if (!file.open("ttsmall.fnt"))
		error("AVALANCHE: Scrolls: File not found: ttsmall.fnt");

	for (int16 i = 0; i < 256; i++)
		file.read(_vm->_gyro->_font[i],16);
	file.close();
}

void Scrolls::musicalScroll() {
	Common::String tmpStr = Common::String::format("To play the harp...%c%cUse these keys:%c%cQ W E R T Y U I O P [ ]%c%cOr press Enter to stop playing.%c", 
		        kControlNewLine, kControlNewLine, kControlNewLine, kControlInsertSpaces, kControlNewLine, kControlNewLine, kControlToBuffer);
	displayText(tmpStr);

	_vm->_lucerna->spriteRun();
	drawScroll(&Avalanche::Scrolls::scrollModeMusic);
	resetScroll();
}

// From Visa:

void Scrolls::unSkrimble() {
	for (uint16  i = 0; i < _vm->_gyro->_bufSize; i++)
		_vm->_gyro->_buffer[i] = (~(_vm->_gyro->_buffer[i] - (i + 1))) % 256;
}

void Scrolls::doTheBubble() {
	_vm->_gyro->_buffer[_vm->_gyro->_bufSize] = 2;
	_vm->_gyro->_bufSize++;
}

/**
 * Display a string in a scroll
 * @remarks	Originally called 'dixi'
 */
void Scrolls::displayScrollChain(char block, byte point, bool report, bool bubbling) {
	Common::File indexfile;
	if (!indexfile.open("avalot.idx"))
		error("AVALANCHE: Visa: File not found: avalot.idx");

	bool error = false;

	indexfile.seek((toupper(block) - 65) * 2);
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
			Common::String todisplay = Common::String::format("%cError accessing scroll %c%d", Scrolls::kControlBell, block, point);
			_vm->_scrolls->displayText(todisplay);
		}
		return;
	}

	Common::File sezfile;
	if (!sezfile.open("avalot.sez"))
		::error("AVALANCHE: Visa: File not found: avalot.sez");

	sezfile.seek(sez_offset);
	_vm->_gyro->_bufSize = sezfile.readUint16LE();
	sezfile.read(_vm->_gyro->_buffer, _vm->_gyro->_bufSize);
	sezfile.close();
	unSkrimble();

	if (bubbling)
		doTheBubble();

	_vm->_scrolls->callScrollDriver();
}

/**
 * Start speech
 * @remarks	Originally called 'speech'
 */
void Scrolls::speak(byte who, byte subject) {
	if (subject == 0) { // No subject.
		displayScrollChain('s', who, false, true);
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
	uint16 sez_offset = indexfile.readUint16LE();
	if ((sez_offset == 0) || (indexfile.err()))
		return;
	indexfile.close();

	Common::File sezfile;
	if (!sezfile.open("avalot.sez"))
		error("AVALANCHE: Visa: File not found: avalot.sez");

	sezfile.seek(sez_offset);
	_vm->_gyro->_bufSize = sezfile.readUint16LE();
	sezfile.read(_vm->_gyro->_buffer, _vm->_gyro->_bufSize);
	sezfile.close();

	unSkrimble();
	doTheBubble();

	_vm->_scrolls->callScrollDriver();
	_noError = true;
}

void Scrolls::talkTo(byte whom) {
	if (_vm->_acci->_person == _vm->_acci->kPardon) {
		_vm->_acci->_person = _vm->_gyro->_subjectNum;
		_vm->_gyro->_subjectNum = 0;
	}

	if (_vm->_gyro->_subjectNum == 0) {
		switch (whom) {
		case Gyro::kPeopleSpludwick:
			if ((_vm->_gyro->_lustieIsAsleep) & (!_vm->_gyro->_objects[Gyro::kObjectPotion - 1])) {
				displayScrollChain('q', 68);
				_vm->_gyro->_objects[Gyro::kObjectPotion - 1] = true;
				_vm->_lucerna->refreshObjectList();
				_vm->_lucerna->incScore(3);
				return;
			} else if (_vm->_gyro->_talkedToCrapulus) {
				// Spludwick - what does he need?
				// 0 - let it through to use normal routine.
				switch (_vm->_gyro->_givenToSpludwick) {
				case 1: // Fallthrough is intended.
				case 2: {
					Common::String objStr = _vm->_gyro->getItem(Gyro::kSpludwicksOrder[_vm->_gyro->_givenToSpludwick]);
					Common::String tmpStr = Common::String::format("Can you get me %s, please?%c2%c", objStr.c_str(), Scrolls::kControlRegister, Scrolls::kControlSpeechBubble);
					_vm->_scrolls->displayText(tmpStr);
					}
					return;
				case 3:
					displayScrollChain('q', 30); // Need any help with the game?
					return;
				}
			} else {
				displayScrollChain('q', 42); // Haven't talked to Crapulus. Go and talk to him.
				return;
			}
			break;
		case Gyro::kPeopleIbythneth:
			if (_vm->_gyro->_givenBadgeToIby) {
				displayScrollChain('q', 33); // Thanks a lot!
				return; // And leave the proc.
			}
			break; // Or... just continue, 'cos he hasn't got it.
		case Gyro::kPeopleDogfood:
			if (_vm->_gyro->_wonNim) { // We've won the game.
				displayScrollChain('q', 6); // "I'm Not Playing!"
				return; // Zap back.
			} else
				_vm->_gyro->_askedDogfoodAboutNim = true;
			break;
		case Gyro::kPeopleAyles:
			if (!_vm->_gyro->_aylesIsAwake) {
				displayScrollChain('q', 43); // He's fast asleep!
				return;
			} else if (!_vm->_gyro->_givenPenToAyles) {
				displayScrollChain('q', 44); // Can you get me a pen, Avvy?
				return;
			}
			break;

		case Gyro::kPeopleJacques:
			displayScrollChain('q', 43);
			return;

		case Gyro::kPeopleGeida:
			if (_vm->_gyro->_givenPotionToGeida)
				_vm->_gyro->_geidaFollows = true;
			else {
				displayScrollChain('u', 17);
				return;
			}
			break;
		case Gyro::kPeopleSpurge:
			if (!_vm->_gyro->_sittingInPub) {
				displayScrollChain('q', 71); // Try going over and sitting down.
				return;
			} else {
				if (_vm->_gyro->_spurgeTalkCount < 5)
					_vm->_gyro->_spurgeTalkCount++;
				if (_vm->_gyro->_spurgeTalkCount > 1) { // no. 1 falls through
					displayScrollChain('q', 70 + _vm->_gyro->_spurgeTalkCount);
					return;
				}
			}
			break;
		}
	// On a subject. Is there any reason to block it?
	} else if ((whom == Gyro::kPeopleAyles) && (!_vm->_gyro->_aylesIsAwake)) {
		displayScrollChain('q', 43); // He's fast asleep!
		return;
	}

	if (whom > 149)
		whom -= 149;

	bool noMatches = true;
	for (int i = 0; i <= _vm->_animation->kSpriteNumbMax; i++) {
		if (_vm->_animation->_sprites[i]._stat._acciNum == whom) {
			Common::String tmpStr = Common::String::format("%c%c%c", Scrolls::kControlRegister, i + 49, Scrolls::kControlToBuffer);
			_vm->_scrolls->displayText(tmpStr);
			noMatches = false;
			break;
		}
	}

	if (noMatches) {
		Common::String tmpStr = Common::String::format("%c%c%c", Scrolls::kControlRegister, Scrolls::kControlRegister, Scrolls::kControlToBuffer);
		_vm->_scrolls->displayText(tmpStr);
	}

	speak(whom, _vm->_gyro->_subjectNum);

	if (!_noError)
		displayScrollChain('n', whom); // File not found!

	if ((_vm->_gyro->_subjectNum == 0) && ((whom + 149) == Gyro::kPeopleCrapulus)) { // Crapulus: get the badge - first time only
		_vm->_gyro->_objects[Gyro::kObjectBadge - 1] = true;
		_vm->_lucerna->refreshObjectList();
		displayScrollChain('q', 1); // Circular from Cardiff.
		_vm->_gyro->_talkedToCrapulus = true;
		_vm->_gyro->_whereIs[Gyro::kPeopleCrapulus - 150] = kRoomDummy; // Crapulus walks off.

		AnimationType *spr = &_vm->_animation->_sprites[1];
		spr->_vanishIfStill = true;
		spr->walkTo(2); // Walks away.

		_vm->_lucerna->incScore(2);
	}
}

} // End of namespace Avalanche
