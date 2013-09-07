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

#include "avalanche/scrolls2.h"
#include "avalanche/gyro2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/animation.h"
#include "avalanche/acci2.h"
#include "avalanche/visa2.h"
#include "avalanche/timeout2.h"

#include "common/textconsole.h"
#include "common/file.h"

//#include "avalanche/joystick2.h" - Will be implemented later, if it will be implemented at all...

namespace Avalanche {

Scrolls::Scrolls(AvalancheEngine *vm) {
	_vm = vm;
}

void Scrolls::init() {
	loadFont();
	resetScrollDriver();
}

void Scrolls::setReadyLight(byte x) {     // Sets "Ready" light to whatever
	if (_vm->_gyro->_ledStatus == x)
		return; // Already like that!

	byte color = kColorBlack;
	switch (x) {
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
	_vm->_gyro->_ledStatus = x;
}

void Scrolls::easterEgg() {
	warning("STUB: Scrolls::easteregg()");
}

void Scrolls::say(int16 x, int16 y, Common::String z) { // Fancy FAST screenwriting
	FontType itw;
	byte lz = z.size();

	bool offset = x % 8 == 4;
	x = x / 8;
	y++;
	int16 i = 0;
	for (byte xx = 0; xx < lz; xx++) {
		switch (z[xx]) {
		case kControlRoman: {
			_currentFont = kFontStyleRoman;
			}
			break;
		case kControlItalic: {
			_currentFont = kFontStyleItalic;
			}
			break;
		default: {
			for (byte yy = 0; yy < 12; yy++)
				itw[(byte)z[xx]][yy] = _scrollFonts[_currentFont][(byte)z[xx]][yy + 2];

			// We have to draw the characters one-by-one because of the accidental font changes.
			i++;
			Common::String chr(z[xx]);
			_vm->_graphics->drawText(_vm->_graphics->_scrolls, chr, itw, 12, (x - 1) * 8 + offset * 4 + i * 8, y, kColorBlack);
			}
		}
	}
}

// Here are the procedures that Scroll calls. So they must be... $F+*/

void Scrolls::scrollModeNormal() {
	Common::String egg = Common::String(kControlParagraph) + kControlLeftJustified + kControlNegative + kControlBell + kControlBackspace + "***";
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
				if (egg == e)  easteregg();
			}
			oktoexit = set::of('\15', '\33', '+', '#', eos).has(inchar);
			if (! oktoexit)  errorled();
		}

	} while (!((oktoexit) || (mrelease > 0)));

//#ifdef RECORD record_one(); #endif

	_vm->_gyro->screturn = r == '#'; // "back door"
#endif

	setReadyLight(0);
	_vm->_gyro->_seeScroll = false;
	CursorMan.showMouse(false);
	_vm->_lucerna->_holdLeftMouse = false; // Used in Lucerna::checkclick().

	warning("STUB: Scrolls::normscroll()");
}

void Scrolls::scrollModeDialogue() {
	warning("STUB: Scrolls::dialogue()");
}




void Scrolls::store(byte what, TuneType &played) {
	memcpy(played + 1, played + 2, sizeof(played) - 1);
	played[30] = what;
}

bool Scrolls::theyMatch(TuneType &played) {
	byte fv, mistakes;

	mistakes = 0;

	for (fv = 1; fv <= sizeof(played); fv++) {
		if (played[fv] != _vm->_gyro->kTune[fv]) {
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

		_vm->_timeout->set_up_timer(8, _vm->_timeout->procjacques_wakes_up, _vm->_timeout->reason_jacques_waking_up);
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

// ThatsAll, so put us back to $F-

void Scrolls::resetScrollDriver() {   // phew
	_vm->_gyro->_scrollBells = 0;
	_currentFont = kFontStyleRoman;
	_useIcon = 0;
	_vm->_gyro->_interrogation = 0; // always reset after a scroll comes up.
}

void Scrolls::ringBell() {   // Pussy's in the well. Who put her in? Little...
	for (byte fv = 0; fv < _vm->_gyro->_scrollBells; fv++)
		_vm->_lucerna->errorLed(); // ring the bell "x" times
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
	Common::File f;
	byte *p;

	if (!f.open("icons.avd")) {
		warning("AVALANCHE: Scrolls: File not found: icons.avd");
		return;
	}

	which--;
	f.seek(which * 426);

	p = new byte[426];
	f.read(p, 426);

	//putimage(x, y, p, 0);
	warning("STUB: Scrolls::geticon()");

	delete[] p;
	f.close();
}

void Scrolls::drawSign(Common::String fn, int16 xl, int16 yl, int16 y) {
	Common::File f;

	Common::String filename;
	filename = filename.format("%s.avd", fn.c_str());
	if (!f.open(filename)) {
		warning("AVALANCHE: Scrolls: File not found: %s", filename.c_str());
		return;
	}

#if 0
	uint16 st = (y - 1) * 80 + (40 - xl / 2) + ((1 - _vm->_gyro->cp) * _vm->_gyro->pagetop);
	byte bit;
	for (uint16 fv = 1; fv <= yl; fv++)
		for (bit = 0; bit <= 3; bit++) {
			port[0x3c4] = 2;
			port[0x3ce] = 4;
			port[0x3c5] = 1 << bit;
			port[0x3cf] = bit;
			blockread(f, mem[0xa000 * st + (fv * 80)], xl);
		}
	bit = getpixel(0, 0);
#endif

	warning("STUB: Scrolls::block_drop()");

	f.close();
}

void Scrolls::drawScroll(ScrollsFunctionType gotoit) { // This is one of the oldest procs in the game.
	byte b;
	int16 lx, ly, mx, my, ex;
	bool centre;
	byte icon_indent = 0;

	//setvisualpage(cp);
	//setactivepage(1 - cp);
	_vm->_gyro->_onCanDoPageSwap = false;  // On can now no longer swap pages. So we can do what we want without its interference!

	lx = 0;
	ly = (_vm->_gyro->_scrollNum) * 6;
	for (b = 0; b < _vm->_gyro->_scrollNum; b++) {
		ex = _vm->_gyro->_scroll[b].size() * 8;
		if (lx < ex)
			lx = ex;
	}
	mx = 320;
	my = 100; // Getmaxx & getmaxy div 2, both.
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

	ex = mx - lx;
//	int16 ey = my - ly;
	mx -= lx;
	my -= ly + 2;

	centre = false;

	switch (_useIcon) {
	case 0:
		icon_indent = 0;
		break; // No icon.
	case 34: {
		drawSign("about", 28, 76, 15);
		icon_indent = 0;
		}
		break;
	case 35: {
		drawSign("gameover", 52, 59, 71);
		icon_indent = 0;
		}
		break;
	}

	if ((1 <= _useIcon) && (_useIcon <= 33)) { // Standard icon.
		getIcon(mx, my + ly / 2, _useIcon);
		icon_indent = 53;
	}


	for (b = 0; b < _vm->_gyro->_scrollNum; b++) {
		if (!_vm->_gyro->_scroll[b].empty())
			switch (_vm->_gyro->_scroll[b][_vm->_gyro->_scroll[b].size() - 1]) {
			case kControlCenter: {
				centre = true;
				_vm->_gyro->_scroll[b].deleteLastChar();
				}
				break;
			case kControlLeftJustified: {
				centre = false;
				_vm->_gyro->_scroll[b].deleteLastChar();
				}
				break;
			case kControlQuestion: {
				//settextjustify(1, 1);
				_shadowBoxX = mx + lx;
				_shadowBoxY = my + ly;
				_vm->_gyro->_scroll[b].setChar(' ', 0);
				// byte groi = *_vm->_graphics->getPixel(0, 0);
				// inc(diy,14);
				_vm->_gyro->drawShadowBox(_shadowBoxX - 65, _shadowBoxY - 24, _shadowBoxX - 5, _shadowBoxY - 10, "Yes.");
				_vm->_gyro->drawShadowBox(_shadowBoxX + 5, _shadowBoxY - 24, _shadowBoxX + 65, _shadowBoxY - 10, "No.");
				}
				break;
			}

		if (centre)
			say(320 - _vm->_gyro->_scroll[b].size() * 4 + icon_indent, my, _vm->_gyro->_scroll[b]);
		else
			say(mx + icon_indent, my, _vm->_gyro->_scroll[b]);

		my += 12;
	}

	_vm->_gyro->_underScroll = my * 2 + 6; // Multiplying because of the doubled screen height.
	//setvisualpage(1 - cp);
	ringBell();
	//my = getpixel(0, 0);
	_vm->_gyro->_dropsOk = false;
	dodgem();

	(this->*gotoit)();

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

	warning("STUB: Scrolls::drawscroll()");
}

void Scrolls::drawBubble(ScrollsFunctionType gotoit) {
	int16 xl, yl, my, xw, yw;
	Common::Point p[3];
//	byte *rp1, *rp2; // replace: 1=bubble, 2=pointer
	int16 xc; // x correction

	//setvisualpage(cp);
	//setactivepage(1 - cp);
	_vm->_gyro->_onCanDoPageSwap = false;  // On can now no longer swap pages. So we can do what we want without its interference!
	//mousepage(1 - cp); // Mousepage

	CursorMan.showMouse(false);

	xl = 0;
	yl = _vm->_gyro->_scrollNum * 5;
	for (byte fv = 0; fv < _vm->_gyro->_scrollNum; fv++) {
		uint16 textWidth = _vm->_gyro->_scroll[fv].size() * 8;
		if (textWidth > xl)
			xl = textWidth;
	}
	xl = xl / 2;

	xw = xl + 18;
	yw = yl + 7;
	my = yw * 2 - 2;
	xc = 0;

	if ((_vm->_gyro->_talkX - xw) < 0)
		xc = -(_vm->_gyro->_talkX - xw);
	if ((_vm->_gyro->_talkX + xw) > 639)
		xc = 639 - (_vm->_gyro->_talkX + xw);

	p[0].x = _vm->_gyro->_talkX - 10;
	p[0].y = yw;
	p[1].x = _vm->_gyro->_talkX + 10;
	p[1].y = yw;
	p[2].x = _vm->_gyro->_talkX;
	p[2].y = _vm->_gyro->_talkY;

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
	_vm->_graphics->drawTriangle(_vm->_graphics->_scrolls, p, _vm->_gyro->_talkBackgroundColor);


	yl -= 3;

	// Draw the text of the bubble. The centering of the text was improved here compared to Pascal's settextjustify().
	// The font is not the same that outtextxy() uses in Pascal. I don't have that, so I used Gyro::characters instead.
	// It's almost the same, only notable differences are '?', '!', etc.
	for (byte fv = 0; fv < _vm->_gyro->_scrollNum; fv++) {
		int16 x = xc + _vm->_gyro->_talkX - _vm->_gyro->_scroll[fv].size() / 2 * 8;
		bool offset = _vm->_gyro->_scroll[fv].size() % 2;
		_vm->_graphics->drawText(_vm->_graphics->_scrolls, _vm->_gyro->_scroll[fv], _vm->_gyro->_font, 8, x - offset * 4, (fv * 10) + 12, _vm->_gyro->_talkFontColor);
	}

	//setvisualpage(1 - cp);
	ringBell();
	_vm->_gyro->_onCanDoPageSwap = false;
	CursorMan.showMouse(true);
	_vm->_gyro->_dropsOk = false;

	// This does the actual drawing to the screen.
	(this->*gotoit)();

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
	warning("STUB: Scrolls::ask()");
	return true;
}

void Scrolls::resetScroll() {
	_vm->_gyro->_scrollNum = 1;
	for (int j = 0; j < 15; j++)
		if (!_vm->_gyro->_scroll[j].empty())
			_vm->_gyro->_scroll[j].clear();
}

void Scrolls::setBubbleStateNatural() {
	_vm->_gyro->_talkX = 320;
	_vm->_gyro->_talkY = 200;
	_vm->_gyro->_talkBackgroundColor = 8;
	_vm->_gyro->_talkFontColor = 15;
}

Common::String Scrolls::displayMoney() {
	Common::String x;

	if (_vm->_gyro->_dna._money < 12) { // just pence
		x = _vm->_gyro->intToStr(_vm->_gyro->_dna._money) + 'd';
	} else if (_vm->_gyro->_dna._money < 240) { // shillings & pence
		x = _vm->_gyro->intToStr(_vm->_gyro->_dna._money / 12) + '/';
		if ((_vm->_gyro->_dna._money % 12) == 0)
			x = x + '-';
		else
			x = x + _vm->_gyro->intToStr(_vm->_gyro->_dna._money % 12);
	} else // L, s & d
		x = Common::String('\x9C') + _vm->_gyro->intToStr(_vm->_gyro->_dna._money / 240) + '.' + _vm->_gyro->intToStr((_vm->_gyro->_dna._money / 12) % 20)
			+ '.' + _vm->_gyro->intToStr(_vm->_gyro->_dna._money % 12);
	if (_vm->_gyro->_dna._money > 12)
		x = x + " (that's " + _vm->_gyro->intToStr(_vm->_gyro->_dna._money) + "d)";

	return x;
}




void Scrolls::stripTrailingSpaces(Common::String &q) {
	while (q[q.size() - 1] == ' ') {
		q.deleteLastChar();
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
	uint16 fv;
	byte nn;
	bool mouthnext;
	bool call_spriterun; // Only call sprite_run the FIRST time.
//	bool was_virtual; // Was the mouse cursor virtual on entry to this proc?


	//nosound();
	warning("STUB: Scrolls::calldrivers()");

	setReadyLight(0);
	_vm->_gyro->_scReturn = false;
	mouthnext = false;
	call_spriterun = true;

	switch (_vm->_gyro->_buffer[_vm->_gyro->_bufSize - 1]) {
	case kControlToBuffer:
		_vm->_gyro->_bufSize--;
		break; // ^D = (D)on't include pagebreak
	case kControlSpeechBubble:
	case kControlQuestion:
		break; // ^B = speech (B)ubble, ^Q = (Q)uestion in dialogue box
	default: {
		_vm->_gyro->_bufSize++;
		_vm->_gyro->_buffer[_vm->_gyro->_bufSize - 1] = kControlParagraph;
		}
	}

	uint16 size = _vm->_gyro->_bufSize;

	for (fv = 0; fv < size; fv++) {
		if (mouthnext) {
			if (_vm->_gyro->_buffer[fv] == kControlRegister)
				_param = 0;
			else if (('0' <= _vm->_gyro->_buffer[fv]) && (_vm->_gyro->_buffer[fv] <= '9'))
				_param = _vm->_gyro->_buffer[fv] - 48;
			else if (('A' <= _vm->_gyro->_buffer[fv]) && (_vm->_gyro->_buffer[fv] <= 'Z'))
				_param = _vm->_gyro->_buffer[fv] - 55;

			mouthnext = false;
		} else {
			switch (_vm->_gyro->_buffer[fv]) {
			case kControlParagraph: {
				if ((_vm->_gyro->_scrollNum == 1) && (_vm->_gyro->_scroll[0].empty()))
					break;

				if (call_spriterun)
					_vm->_lucerna->spriteRun();
				call_spriterun = false;

				drawScroll(&Avalanche::Scrolls::scrollModeNormal);

				resetScroll();

				if (_vm->_gyro->_scReturn)
					return;
			}
			break;
			case kControlBell:
				_vm->_gyro->_scrollBells++;
				break; // #7 = "Bel"
			case kControlSpeechBubble: {
				if ((_vm->_gyro->_scrollNum == 1) && (_vm->_gyro->_scroll[0].empty()))
					break;

				if (call_spriterun)
					_vm->_lucerna->spriteRun();
				call_spriterun = false;

				if (_param == 0)
					setBubbleStateNatural();
				else if ((1 <= _param) && (_param <= 9)) {
					if ((_param > _vm->_animation->kSpriteNumbMax) || (!_vm->_animation->tr[_param - 1].quick)) { // Not valid.
						_vm->_lucerna->errorLed();
						setBubbleStateNatural();
					} else
						_vm->_animation->tr[_param - 1].chatter(); // Normal sprite talking routine.
				} else if ((10 <= _param) && (_param <= 36)) {
					// Quasi-peds. (This routine performs the same
					// thing with QPs as triptype.chatter does with the
					// sprites.)
					_vm->_gyro->_talkX = _vm->_gyro->_peds[_vm->_gyro->kQuasipeds[_param - 10]._whichPed - 1]._x;
					_vm->_gyro->_talkY = _vm->_gyro->_peds[_vm->_gyro->kQuasipeds[_param - 10]._whichPed - 1]._y; // Position.

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
			}
			break;
			case kControlNegative:
				switch (_param) {
				case 1:
					displayText(displayMoney() + kControlToBuffer); // Insert cash balance. (Recursion)
					break;
				case 2:
					displayText(_vm->_acci->kVocabulary[_vm->_acci->kFirstPassword + _vm->_gyro->_dna._passwordNum]._word + kControlToBuffer);
					break;
				case 3:
					displayText(_vm->_gyro->_dna._favouriteDrink + kControlToBuffer);
					break;
				case 4:
					displayText(_vm->_gyro->_dna._favouriteSong + kControlToBuffer);
					break;
				case 5:
					displayText(_vm->_gyro->_dna._worstPlaceOnEarth + kControlToBuffer);
					break;
				case 6:
					displayText(_vm->_gyro->_dna._spareEvening + kControlToBuffer);
					break;
				case 9:
					displayText(_vm->_gyro->intToStr(_vm->_gyro->_dna._catacombX) + ',' + _vm->_gyro->intToStr(_vm->_gyro->_dna._catacombY) + kControlToBuffer);
					break;
				case 10:
					switch (_vm->_gyro->_dna._boxContent) {
					case 0: { // Sixpence.
						_vm->_visa->dixi('q', 37); // You find the sixpence.
						_vm->_gyro->_dna._money += 6;
						_vm->_gyro->_dna._boxContent = _vm->_acci->kNothing;
						_vm->_lucerna->incScore(2);
						return;
					}
					break;
					case Acci::kNothing:
						displayText("nothing at all. It's completely empty.");
						break;
					default:
						displayText(_vm->_gyro->getItem(_vm->_gyro->_dna._boxContent) + '.');
					}
					break;
				case 11:
					nn = 1;
					for (byte nnn = 0; nnn < kObjectNum; nnn++) {
						if (_vm->_gyro->_dna._objects[nnn]) {
							nn++;
							displayText(_vm->_gyro->getItem(nnn) + ", " + kControlToBuffer);
						}
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
			case kControlQuestion: {
				if (call_spriterun)
					_vm->_lucerna->spriteRun();
				call_spriterun = false;

				_vm->_gyro->_scrollNum++;
				_vm->_gyro->_scroll[_vm->_gyro->_scrollNum - 1] = kControlQuestion;

				drawScroll(&Avalanche::Scrolls::scrollModeDialogue);

				resetScroll();
				}
				break;
			case kControlRegister:
				mouthnext = true;
				break;
			case kControlInsertSpaces:
				for (nn = 0; nn < 9; nn++)
					_vm->_gyro->_scroll[_vm->_gyro->_scrollNum - 1] += ' ';
				break;
			default: { // Add new char.
				if (_vm->_gyro->_scroll[_vm->_gyro->_scrollNum - 1].size() == 50) {
					solidify(_vm->_gyro->_scrollNum - 1);
					_vm->_gyro->_scrollNum++;
				}
				_vm->_gyro->_scroll[_vm->_gyro->_scrollNum - 1] += _vm->_gyro->_buffer[fv];
				}
			}
		}
	}
}

void Scrolls::displayText(Common::String z) { // TODO: REPLACE BUFFER WITH A STRING!!!!!!!!!!
	_vm->_gyro->_bufSize = z.size();
	memcpy(_vm->_gyro->_buffer, z.c_str(), _vm->_gyro->_bufSize);
	callScrollDriver();
}

void Scrolls::loadFont() {
	Common::File f;

	if (!f.open("avalot.fnt")) {
		warning("AVALANCHE: Scrolls: File not found: avalot.fnt");
		return;
	}
	for (int16 i = 0; i < 256; i++)
		f.read(_scrollFonts[0][i], 16);
	f.close();

	if (!f.open("avitalic.fnt")) {
		warning("AVALANCHE: Scrolls: File not found: avitalic.fnt");
		return;
	}
	for (int16 i = 0; i < 256; i++)
		f.read(_scrollFonts[1][i], 16);
	f.close();

	if (!f.open("ttsmall.fnt")) {
		warning("AVALANCHE: Scrolls: File not found: ttsmall.fnt");
		return;
	}
	for (int16 i = 0; i < 256; i++)
		f.read(_vm->_gyro->_font[i],16);
	f.close();
}

void Scrolls::musicalScroll() {
	displayText(Common::String("To play the harp...") + kControlNewLine + kControlNewLine + "Use these keys:" + + kControlNewLine
		+ kControlInsertSpaces + "Q W E R T Y U I O P [ ]" + kControlNewLine + kControlNewLine + "Or press Enter to stop playing."
		+ kControlToBuffer);

	_vm->_lucerna->spriteRun();

	drawScroll(&Avalanche::Scrolls::scrollModeMusic);

	resetScroll();
}

} // End of namespace Avalanche
