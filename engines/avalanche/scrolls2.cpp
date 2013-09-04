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
#include "avalanche/enhanced2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/trip6.h"
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
	loadfont();
	resetscrolldriver();
}

void Scrolls::state(byte x) {     /* Sets "Ready" light to whatever */
	if (_vm->_gyro->ledstatus == x)
		return; /* Already like that! */

	byte color;
	switch (x) {
	case 0:
		color = black;
		break; /* Off */
	case 1:
	case 2:
	case 3:
		color = green;
		break; /* Hit a key */
	}

	//switch (x) {
	//case 0:
	//	setfillstyle(1, black);
	//	break; /* Off */
	//case 1:
	//	setfillstyle(9, green);
	//	break; /* Half-on (menus) */
	//case 2:
	//	setfillstyle(1, green);
	//	break; /* On (kbd) */
	//case 3:
	//	setfillstyle(6, green);
	//	break; /* Hit a key */
	//}
	warning("STUB: Scrolls::state()");

	CursorMan.showMouse(false);

	_vm->_graphics->drawBar(419, 195, 438, 197, color);

	CursorMan.showMouse(true);
	_vm->_gyro->ledstatus = x;
}

void Scrolls::easteregg() {
	warning("STUB: Scrolls::easteregg()");
}

void Scrolls::say(int16 x, int16 y, Common::String z) { /* Fancy FAST screenwriting */
	fontType itw;
	byte lz = z.size();
	
	bool offset = x % 8 == 4;
	x = x / 8;
	y++;
	int16 i = 0;
	for (byte xx = 0; xx < lz; xx++) {
		switch (z[xx]) {
		case kControlRoman: {
			cfont = roman;
			}
			break;
		case kControlItalic: {
			cfont = italic;
			}
			break;
		default: {
			for (byte yy = 0; yy < 12; yy++)
				itw[(byte)z[xx]][yy] = ch[cfont][(byte)z[xx]][yy + 2];

			// We have to draw the characters one-by-one because of the accidental font changes.
			i++;
			Common::String chr(z[xx]);
			_vm->_graphics->drawText(_vm->_graphics->_scrolls, chr, itw, 12, (x - 1) * 8 + offset * 4 + i * 8, y, black);
			}
		}
	}
}

/* Here are the procedures that Scroll calls */ /* So they must be... */ /*$F+*/

void Scrolls::normscroll() {
	Common::String egg = Common::String(kControlParagraph) + kControlLeftJustified + kControlNegative + kControlBell + kControlBackspace + "***";
	Common::String e = "(c) 1994";

	state(3);
	_vm->_gyro->seescroll = true;
	CursorMan.showMouse(true);
	_vm->_gyro->newpointer(4);
	_vm->_lucerna->mousepage(1 - _vm->_gyro->cp);



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



//	char r;
//	bool oktoexit;
//	do {
//		do {
//			_vm->_gyro->check(); /* was "checkclick;" */
//
//#ifdef RECORD slowdown(); basher::count += 1; #endif
//
//			if (_vm->_gyro->demo) {
//				if (_vm->_basher->demo_ready())
//					break;
//				if (_vm->_enhanced->keypressede())
//					return;
//			} else if (_vm->_enhanced->keypressede())
//				break;
//		} while (!((mrelease > 0) || (buttona1()) || (buttonb1())));
//
//
//		if (mrelease == 0) {
//			inkey();
//			if (aboutscroll) {
//				move(e[2 - 1], e[1 - 1], 7);
//				e[8 - 1] = inchar;
//				if (egg == e)  easteregg();
//			}
//			oktoexit = set::of('\15', '\33', '+', '#', eos).has(inchar);
//			if (! oktoexit)  errorled();
//		}
//
//	} while (!((oktoexit) || (mrelease > 0)));
//
//#ifdef RECORD record_one(); #endif
//
//	_vm->_gyro->screturn = r == '#'; /* "back door" */

	state(0);
	_vm->_gyro->seescroll = false;
	_vm->_lucerna->mousepage(_vm->_gyro->cp);
	CursorMan.showMouse(false);
	_vm->_lucerna->holdLeftMouse = false; // Used in Lucerna::checkclick().

	warning("STUB: Scrolls::normscroll()");
}

void Scrolls::dialogue() {
	warning("STUB: Scrolls::dialogue()");
}




void Scrolls::store_(byte what, tunetype &played) {
	memcpy(played+1, played+2, sizeof(played) - 1);
	played[31] = what;
}

bool Scrolls::they_match(tunetype &played) {
	byte fv, mistakes;

	mistakes = 0;

	for (fv = 1; fv <= sizeof(played); fv ++)
		if (played[fv] != _vm->_gyro->tune[fv]) {
			mistakes += 1;
		}

	return mistakes < 5;
}

void Scrolls::music_scroll() {
	state(3);
	_vm->_gyro->seescroll = true;
	_vm->_gyro->on();
	_vm->_gyro->newpointer(4);

	// Since there are no sounds in the game yet, it's pretty pointless to implement this function further. 
	// For now we act like the player just played the right tone.
	//if (they_match(played)) {
		_vm->_gyro->screturn = true;
		_vm->_gyro->off();
		state(0);
		_vm->_gyro->seescroll = false;

		_vm->_timeout->set_up_timer(8, _vm->_timeout->procjacques_wakes_up, _vm->_timeout->reason_jacques_waking_up);
		return;
	//}
	warning("STUB: Scrolls::music_scroll()");

	_vm->_gyro->screturn = false;
	_vm->_gyro->off();
	state(0);
	_vm->_gyro->seescroll = false;
}

/* ThatsAll, so put us back to */ /*$F-*/

void Scrolls::resetscrolldriver() {   /* phew */
	_vm->_gyro->scrollbells = 0;
	cfont = roman;
	use_icon = 0;
	_vm->_gyro->interrogation = 0; /* always reset after a scroll comes up. */
}

void Scrolls::dingdongbell() {   /* Pussy's in the well. Who put her in? Little... */
	byte fv;

	for (fv = 0; fv < _vm->_gyro->scrollbells; fv++)
		_vm->_lucerna->errorled(); /* ring the bell "x" times */
}

void Scrolls::dodgem() {     
	dodgeCoord = _vm->getMousePos();
	g_system->warpMouse(dodgeCoord.x, _vm->_gyro->underscroll); // Move the pointer off the scroll.
}

void Scrolls::undodgem() {   
	Common::Point actCoord = _vm->getMousePos();
	if ((actCoord.x == dodgeCoord.x) && (actCoord.y == _vm->_gyro->underscroll))
		g_system->warpMouse(dodgeCoord.x, dodgeCoord.y); // No change, so restore the pointer's original position.
}

void Scrolls::geticon(int16 x, int16 y, byte which) {
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

void Scrolls::block_drop(Common::String fn, int16 xl, int16 yl, int16 y) {
	Common::File f;
	uint16 st;

	st = (y - 1) * 80 + (40 - xl / 2) + ((1 - _vm->_gyro->cp) * _vm->_gyro->pagetop);

	Common::String filename;
	filename = filename.format("%s.avd", fn.c_str());
	if (!f.open(filename)) {
		warning("AVALANCHE: Scrolls: File not found: %s", filename.c_str());
		return;
	}

	/*byte bit;
	for (uint16 fv = 1; fv <= yl; fv ++)
		for (bit = 0; bit <= 3; bit ++) {
			port[0x3c4] = 2;
			port[0x3ce] = 4;
			port[0x3c5] = 1 << bit;
			port[0x3cf] = bit;
			blockread(f, mem[0xa000 * st + (fv * 80)], xl);
		}
	bit = getpixel(0, 0);*/

	warning("STUB: Scrolls::block_drop()");

	f.close();
}

void Scrolls::drawscroll(func2 gotoit) { // This is one of the oldest procs in the game.
	byte b, groi;
	int16 lx, ly, mx, my, ex, ey;
	bool centre;
	byte icon_indent = 0;

	//setvisualpage(cp);
	//setactivepage(1 - cp);
	_vm->_gyro->oncandopageswap = false;  /* On can now no longer swap pages. So we can do what we want without its interference! */

	lx = 0;
	ly = (_vm->_gyro->scrolln) * 6;
	for (b = 0; b < _vm->_gyro->scrolln; b++) {
		ex = _vm->_gyro->scroll[b].size() * 8;
		if (lx < ex)
			lx = ex;
	}
	mx = 320;
	my = 100; // Getmaxx & getmaxy div 2, both.
	lx = lx / 2;
	ly -= 2;

	if ((1 <= use_icon) && (use_icon <= 34))
		lx += halficonwidth;

	_vm->_graphics->_scrolls.copyFrom(_vm->_graphics->_surface);

	CursorMan.showMouse(false);

	// The right corners of the scroll.
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, mx + lx, my - ly, 0, 90, 15, lightgray);
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, mx + lx, my + ly, 270, 360, 15, lightgray);
	_vm->_graphics->drawArc(_vm->_graphics->_scrolls, mx + lx, my - ly, 0, 90, 15, red);
	_vm->_graphics->drawArc(_vm->_graphics->_scrolls, mx + lx, my + ly, 270, 360, 15, red);
	
	// The body of the scroll.
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx - lx - 30, my + ly, mx + lx, my + ly + 6), lightgray);
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx - lx - 30, my - ly - 6, mx + lx, my - ly + 1), lightgray);
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx - lx - 15, my - ly, mx + lx + 15, my + ly + 1), lightgray);

	// The left corners of the scroll.
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, mx - lx - 31, my - ly, 0, 180, 15, darkgray);
	_vm->_graphics->drawArc(_vm->_graphics->_scrolls, mx - lx - 31, my - ly, 0, 180, 15, red);
	_vm->_graphics->_scrolls.drawLine(mx - lx - 31 - 15, my - ly, mx - lx - 31 + 15, my - ly, red);
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, mx - lx - 31, my + ly, 180, 360, 15, darkgray);
	_vm->_graphics->drawArc(_vm->_graphics->_scrolls, mx - lx - 31, my + ly, 180, 360, 15, red);
	_vm->_graphics->_scrolls.drawLine(mx - lx - 31 - 15, my + ly, mx - lx - 31 + 15, my + ly, red);

	// The rear borders of the scroll.
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx - lx - 30, my - ly - 6, mx + lx, my - ly - 5), red);
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx - lx - 30, my + ly + 6, mx + lx, my + ly + 7), red);
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx - lx - 15, my - ly, mx - lx - 14, my + ly), red);
	_vm->_graphics->_scrolls.fillRect(Common::Rect(mx + lx + 15, my - ly, mx + lx + 16, my + ly), red);

	ex = mx - lx;
	ey = my - ly;
	mx -= lx;
	my -= ly + 2;
	
	centre = false;

	switch (use_icon) {
	case 0:
		icon_indent = 0;
		break; /* No icon. */
	case 34: {
		block_drop("about", 28, 76, 15);
		icon_indent = 0;
		}
		break;
	case 35: {
		block_drop("gameover", 52, 59, 71);
		icon_indent = 0;
		}
		break;
	}

	if ((1 <= use_icon) && (use_icon <= 33)) { // Standard icon.
		geticon(mx, my + ly / 2, use_icon);
		icon_indent = 53;
	}


	for (b = 0; b < _vm->_gyro->scrolln; b++) {
		if (!_vm->_gyro->scroll[b].empty())
			switch (_vm->_gyro->scroll[b][_vm->_gyro->scroll[b].size() - 1]) {
			case kControlCenter: {
				centre = true;
				_vm->_gyro->scroll[b].deleteLastChar();
				}
				break;
			case kControlLeftJustified: {
				centre = false;
				_vm->_gyro->scroll[b].deleteLastChar();
				}
				break;
			case kControlQuestion: {
				//settextjustify(1, 1);
				dix = mx + lx;
				diy = my + ly;
				_vm->_gyro->scroll[b].setChar(' ', 0);
				groi = *_vm->_graphics->getPixel(0, 0);
				// inc(diy,14);
				_vm->_gyro->shbox(dix - 65, diy - 24, dix - 5, diy - 10, "Yes.");
				_vm->_gyro->shbox(dix + 5, diy - 24, dix + 65, diy - 10, "No.");
				}
				break;
			}

		if (centre)
			say(320 - _vm->_gyro->scroll[b].size() * 4 + icon_indent, my, _vm->_gyro->scroll[b]);
		else
			say(mx + icon_indent, my, _vm->_gyro->scroll[b]);

		my += 12;
	}

	_vm->_gyro->underscroll = my * 2 + 6; // Multiplying because of the doubled screen height.
	//setvisualpage(1 - cp);
	dingdongbell();
	//my = getpixel(0, 0);
	_vm->_gyro->dropsok = false;
	dodgem();

	(this->*gotoit)();

	undodgem();
	_vm->_gyro->dropsok = true;
	//setvisualpage(cp);
	//mousepage(cp);
	CursorMan.showMouse(false);
	/* mblit(ex-46,ey-6,ex+lx*2+15,ey+ly*2+6,3,0);*/
	//mblit((ex - 46) / 8, ey - 6, 1 + (ex + lx * 2 + 15) / 8, ey + ly * 2 + 6, cp, 1 - cp);
	//blitfix();
	_vm->_gyro->oncandopageswap = true; // Normality again.
	CursorMan.showMouse(true);
	//settextjustify(0, 0); /*sink*/
	resetscrolldriver();
	if (_vm->_gyro->mpress > 0)
		_vm->_gyro->after_the_scroll = true;

	warning("STUB: Scrolls::drawscroll()");
}

void Scrolls::bubble(func2 gotoit) {
	int16 xl, yl, my, xw, yw;
	byte fv;
	Common::Point p[3];
//	byte *rp1, *rp2; /* replace: 1=bubble, 2=pointer */
	int16 xc; /* x correction */

	/*setvisualpage(cp);
	setactivepage(1 - cp);*/
	_vm->_gyro->oncandopageswap = false;  /* On can now no longer swap pages. So we can do what we want without its interference! */
	//mousepage(1 - cp); /* Mousepage */

	CursorMan.showMouse(false);

	xl = 0;
	yl = _vm->_gyro->scrolln * 5;
	for (int8 fv = 0; fv < _vm->_gyro->scrolln; fv++) {
		uint16 textWidth = _vm->_gyro->scroll[fv].size() * 8;
		if (textWidth > xl)
			xl = textWidth;
	}
	xl = xl / 2;

	xw = xl + 18;
	yw = yl + 7;
	my = yw * 2 - 2;
	xc = 0;

	if ((_vm->_gyro->talkx - xw) < 0)
		xc = -(_vm->_gyro->talkx - xw);
	if ((_vm->_gyro->talkx + xw) > 639)
		xc = 639 - (_vm->_gyro->talkx + xw);

	p[0].x = _vm->_gyro->talkx - 10;
	p[0].y = yw;
	p[1].x = _vm->_gyro->talkx + 10;
	p[1].y = yw;
	p[2].x = _vm->_gyro->talkx;
	p[2].y = _vm->_gyro->talky;

	// Backup the screen before drawing the bubble.
	_vm->_graphics->_scrolls.copyFrom(_vm->_graphics->_surface);

	// The body of the bubble.
	_vm->_graphics->_scrolls.fillRect(Common::Rect(xc + _vm->_gyro->talkx - xw + 9, 7, _vm->_gyro->talkx + xw - 8 + xc, my + 1), _vm->_gyro->talkb);
	_vm->_graphics->_scrolls.fillRect(Common::Rect(xc + _vm->_gyro->talkx - xw - 1, 12, _vm->_gyro->talkx + xw + xc + 2, my - 4), _vm->_gyro->talkb);

	// Top right corner of the bubble.
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, xc + _vm->_gyro->talkx + xw - 10, 11, 0, 90, 9, _vm->_gyro->talkb);
	// Bottom right corner of the bubble.
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, xc + _vm->_gyro->talkx + xw - 10, my - 4, 270, 360, 9, _vm->_gyro->talkb);
	// Top left corner of the bubble.
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, xc + _vm->_gyro->talkx - xw + 10, 11, 90, 180, 9, _vm->_gyro->talkb);
	// Bottom left corner of the bubble.
	_vm->_graphics->drawPieSlice(_vm->_graphics->_scrolls, xc + _vm->_gyro->talkx - xw + 10, my - 4, 180, 270, 9, _vm->_gyro->talkb);

	// "Tail" of the speech bubble.
	_vm->_graphics->drawTriangle(_vm->_graphics->_scrolls, p, _vm->_gyro->talkb);

	
	yl -= 3;

	// Draw the text of the bubble. The centering of the text was improved here compared to Pascal's settextjustify().
	// The font is not the same that outtextxy() uses in Pascal. I don't have that, so I used Gyro::characters instead.
	// It's almost the same, only notable differences are '?', '!', etc.
	for (fv = 0; fv < _vm->_gyro->scrolln; fv++) {
		int16 x = xc + _vm->_gyro->talkx - _vm->_gyro->scroll[fv].size() / 2 * 8;
		bool offset = _vm->_gyro->scroll[fv].size() % 2;
		_vm->_graphics->drawText(_vm->_graphics->_scrolls, _vm->_gyro->scroll[fv], _vm->_gyro->characters, 8, x - offset * 4, (fv * 10) + 12, _vm->_gyro->talkf);
	}

	//setvisualpage(1 - cp);
	dingdongbell();
	_vm->_gyro->oncandopageswap = false;
	CursorMan.showMouse(true);
	_vm->_gyro->dropsok = false;

	// This does the actual drawing to the screen.
	(this->*gotoit)();

	CursorMan.showMouse(false);
	_vm->_gyro->dropsok = true;

	//setvisualpage(cp);
	CursorMan.showMouse(true); /*sink;*/
	_vm->_gyro->oncandopageswap = true;
	resetscrolldriver();
	if (_vm->_gyro->mpress > 0)
		_vm->_gyro->after_the_scroll = true;
}

bool Scrolls::ask(Common::String question) {
	warning("STUB: Scrolls::ask()");
	return true;
}

void Scrolls::resetscroll() {
	_vm->_gyro->scrolln = 1;
	for (int j = 0; j < 15; j ++)
		if (!_vm->_gyro->scroll[j].empty())
			_vm->_gyro->scroll[j].clear();
}

void Scrolls::natural() {   /* Natural state of bubbles */
	_vm->_gyro->talkx = 320;
	_vm->_gyro->talky = 200;
	_vm->_gyro->talkb = 8;
	_vm->_gyro->talkf = 15;
}

Common::String Scrolls::lsd() {
	Common::String x;

	if (_vm->_gyro->dna.pence < 12) { // just pence
		x = _vm->_gyro->strf(_vm->_gyro->dna.pence) + 'd';
	} else if (_vm->_gyro->dna.pence < 240) { // shillings & pence
		x = _vm->_gyro->strf(_vm->_gyro->dna.pence / 12) + '/';
		if ((_vm->_gyro->dna.pence % 12) == 0)
			x = x + '-';
		else
			x = x + _vm->_gyro->strf(_vm->_gyro->dna.pence % 12);
	} else // L, s & d
		x = Common::String('œ') + _vm->_gyro->strf(_vm->_gyro->dna.pence / 240) + '.' + _vm->_gyro->strf((_vm->_gyro->dna.pence / 12) % 20)
			+ '.' + _vm->_gyro->strf(_vm->_gyro->dna.pence % 12);
	if (_vm->_gyro->dna.pence > 12)
		x = x + " (that's " + _vm->_gyro->strf(_vm->_gyro->dna.pence) + "d)";
	
	return x;
}




void Scrolls::strip(Common::String &q) {
	while (q[q.size() - 1] == ' ') {
		q.deleteLastChar();
	}
}

void Scrolls::solidify(byte n) {
	if (!_vm->_gyro->scroll[n].contains(' '))
		return; // No spaces.

	// So there MUST be a space there, somewhere...
	do {
		_vm->_gyro->scroll[n + 1] = _vm->_gyro->scroll[n][_vm->_gyro->scroll[n].size() - 1] + _vm->_gyro->scroll[n + 1];
		_vm->_gyro->scroll[n].deleteLastChar();
	} while (_vm->_gyro->scroll[n][_vm->_gyro->scroll[n].size() - 1] != ' ');

	strip(_vm->_gyro->scroll[n]);
}

void Scrolls::calldrivers() {
	uint16 fv;
	byte nn;
	char nnn;
	bool mouthnext;
	bool call_spriterun; // Only call sprite_run the FIRST time.
//	bool was_virtual; // Was the mouse cursor virtual on entry to this proc?


	//nosound();
	warning("STUB: Scrolls::calldrivers()");

	state(0);
	_vm->_gyro->screturn = false;
	mouthnext = false;
	call_spriterun = true;

	switch (_vm->_gyro->buffer[_vm->_gyro->bufsize - 1]) {
	case kControlToBuffer:
		_vm->_gyro->bufsize--;
		break; // ^D = (D)on't include pagebreak
	case kControlSpeechBubble:
	case kControlQuestion:
		break; // ^B = speech (B)ubble, ^Q = (Q)uestion in dialogue box
	default: {
		_vm->_gyro->bufsize++;
		_vm->_gyro->buffer[_vm->_gyro->bufsize - 1] = kControlParagraph;
		}
	}

	uint16 size = _vm->_gyro->bufsize;

	for (fv = 0; fv < size; fv++)
		if (mouthnext) {
			if (_vm->_gyro->buffer[fv] == kControlRegister)
				param = 0;
			else
				if (('0' <= _vm->_gyro->buffer[fv]) && (_vm->_gyro->buffer[fv] <= '9'))
					param = _vm->_gyro->buffer[fv] - 48;
				else if (('A' <= _vm->_gyro->buffer[fv]) && (_vm->_gyro->buffer[fv] <= 'Z'))
					param = _vm->_gyro->buffer[fv] - 55;

			mouthnext = false;
		} else
			switch (_vm->_gyro->buffer[fv]) {
			case kControlParagraph: {
				if ((_vm->_gyro->scrolln == 1) && (_vm->_gyro->scroll[0].empty()))
					break;

				if (call_spriterun)
					_vm->_lucerna->sprite_run();
				call_spriterun = false;

				drawscroll(&Avalanche::Scrolls::normscroll);

				resetscroll();

				if (_vm->_gyro->screturn)
					return;
			}
			break;
			case kControlBell:
				_vm->_gyro->scrollbells++;
				break; // #7 = "Bel"
			case kControlSpeechBubble: {
				if ((_vm->_gyro->scrolln == 1) && (_vm->_gyro->scroll[0].empty()))
					break;

				if (call_spriterun)
					_vm->_lucerna->sprite_run();
				call_spriterun = false;

				if (param == 0)
					natural();
				else if ((1 <= param) && (param <= 9))
					if ((param > _vm->_trip->numtr) || (!_vm->_trip->tr[param - 1].quick)) { // Not valid.
						_vm->_lucerna->errorled();
						natural();
					} else
						_vm->_trip->tr[param - 1].chatter(); // Normal sprite talking routine.
				else if ((10 <= param) && (param <= 36)) {
					/* Quasi-peds. (This routine performs the same
					thing with QPs as triptype.chatter does with the
					sprites.) */
					_vm->_gyro->talkx = _vm->_gyro->peds[_vm->_gyro->quasipeds[param - 10].whichped - 1].x;
					_vm->_gyro->talky = _vm->_gyro->peds[_vm->_gyro->quasipeds[param - 10].whichped - 1].y; // Position.
		
					_vm->_gyro->talkf = _vm->_gyro->quasipeds[param - 10].fgc;
					_vm->_gyro->talkb = _vm->_gyro->quasipeds[param - 10].bgc; // Colors.
				} else {
					_vm->_lucerna->errorled(); // Not valid.
					natural();
				}

				bubble(&Avalanche::Scrolls::normscroll);

				resetscroll();

				if (_vm->_gyro->screturn)
					return;
			}
			break;
			case kControlNegative: {
				switch (param) {
				case 1:
					display(lsd() + kControlToBuffer); // Insert cash balance. (Recursion)
					break; 
				case 2:
					display(_vm->_acci->kVocabulary[_vm->_acci->kFirstPassword + _vm->_gyro->dna.pass_num]._word + kControlToBuffer);
					break;
				case 3:
					display(_vm->_gyro->dna.like2drink + kControlToBuffer);
					break;
				case 4:
					display(_vm->_gyro->dna.favourite_song + kControlToBuffer);
					break;
				case 5:
					display(_vm->_gyro->dna.worst_place_on_earth + kControlToBuffer);
					break;
				case 6:
					display(_vm->_gyro->dna.spare_evening + kControlToBuffer);
					break;
				case 9:
					display(_vm->_gyro->strf(_vm->_gyro->dna.cat_x) + ',' + _vm->_gyro->strf(_vm->_gyro->dna.cat_y) + kControlToBuffer);
					break;
				case 10:
					switch (_vm->_gyro->dna.box_contents) {
					case 0: { // Sixpence.
						_vm->_visa->dixi('q', 37); // You find the sixpence.
						_vm->_gyro->dna.pence += 6;
						_vm->_gyro->dna.box_contents = _vm->_acci->kNothing;
						_vm->_lucerna->points(2);
						return;
					}
					break;
					case Acci::kNothing:
						display("nothing at all. It's completely empty.");
						break;
					default:
						display(_vm->_gyro->get_better(_vm->_gyro->dna.box_contents) + '.');
					}
					break;
				case 11: {
					nn = 1;
					for (nnn = 0; nnn < numobjs; nnn++)
						if (_vm->_gyro->dna.obj[nnn]) {
							nn ++;
							display(_vm->_gyro->get_better(nnn) + ", " + kControlToBuffer);
						}
					}
					break;
				}
				
				}
				break;
			case kControlIcon:
				use_icon = param;
				break;
			case kControlNewLine:
				_vm->_gyro->scrolln++;
				break;
			case kControlQuestion: {
				if (call_spriterun)
					_vm->_lucerna->sprite_run();
				call_spriterun = false;

				_vm->_gyro->scrolln++;
				_vm->_gyro->scroll[_vm->_gyro->scrolln - 1] = kControlQuestion;

				drawscroll(&Avalanche::Scrolls::dialogue);

				resetscroll();
				}
				break;
			case kControlRegister:
				mouthnext = true;
				break;
			case kControlInsertSpaces:
				for (nn = 0; nn < 9; nn++)
					_vm->_gyro->scroll[_vm->_gyro->scrolln - 1] += ' ';
				break;
			default: { // Add new char.
				if (_vm->_gyro->scroll[_vm->_gyro->scrolln - 1].size() == 50) {
					solidify(_vm->_gyro->scrolln - 1);
					_vm->_gyro->scrolln++;
				}
				_vm->_gyro->scroll[_vm->_gyro->scrolln - 1] += _vm->_gyro->buffer[fv];
				}
			}
}

void Scrolls::display(Common::String z) { // TODO: REPLACE BUFFER WITH A STRING!!!!!!!!!!
	_vm->_gyro->bufsize = z.size();
	memcpy(_vm->_gyro->buffer, z.c_str(), _vm->_gyro->bufsize);
	calldrivers();
}

void Scrolls::loadfont() {
	Common::File f;

	if (!f.open("avalot.fnt")) {
		warning("AVALANCHE: Scrolls: File not found: avalot.fnt");
		return;
	}
	for (int16 i = 0; i < 256; i++)
		f.read(ch[0][i], 16);
	f.close();

	if (!f.open("avitalic.fnt")) {
		warning("AVALANCHE: Scrolls: File not found: avitalic.fnt");
		return;
	}
	for (int16 i = 0; i < 256; i++)
		f.read(ch[1][i], 16);
	f.close();
	
	if (!f.open("ttsmall.fnt")) {
		warning("AVALANCHE: Scrolls: File not found: ttsmall.fnt");
		return;
	}
	for (int16 i = 0; i < 256; i++)
		f.read(_vm->_gyro->characters[i],16);
	f.close();
}

void Scrolls::okay() {
	display("Okay!");
}

void Scrolls::musical_scroll() {
	display(Common::String("To play the harp...") + kControlNewLine + kControlNewLine + "Use these keys:" + + kControlNewLine
		+ kControlInsertSpaces + "Q W E R T Y U I O P [ ]" + kControlNewLine + kControlNewLine + "Or press Enter to stop playing."
		+ kControlToBuffer);

	_vm->_lucerna->sprite_run();

	drawscroll(&Avalanche::Scrolls::music_scroll);

	resetscroll();
}

} // End of namespace Avalanche
