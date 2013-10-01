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
#include "avalanche/dialogs.h"
#include "avalanche/animation.h"
#include "avalanche/timer.h"

#include "common/textconsole.h"
#include "common/file.h"

namespace Avalanche {

Dialogs::Dialogs(AvalancheEngine *vm) {
	_vm = vm;
	_noError = true;
}

void Dialogs::init() {
	loadFont();
	resetScrollDriver();
}

void Dialogs::setReadyLight(byte state) {     // Sets "Ready" light to whatever
	if (_vm->_ledStatus == state)
		return; // Already like that!

	Color color = kColorBlack;
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

void Dialogs::scrollModeNormal() {
	// Original code is:
	// egg : array[1..8] of char = ^P^L^U^G^H+'***';
	// this is not using kControl characters: it's the secret code to be entered to trigger the easter egg
	// TODO: To be fixed when the Easter egg code is implemented
	Common::String egg = Common::String::format("%c%c%c%c%c***", kControlParagraph, kControlLeftJustified, kControlNegative, kControlBell, kControlBackspace);
	Common::String e = "(c) 1994";

	setReadyLight(3);
	_vm->_seeScroll = true;
	_vm->_graphics->loadMouse(kCurFletch);

	_vm->_graphics->saveScreen();
	_vm->_graphics->showScroll();

	Common::Event event;
	while (!_vm->shouldQuit()) {
		_vm->_graphics->refreshScreen();

		_vm->getEvent(event);
		if (_vm->shouldQuit() || 
			(event.type == Common::EVENT_LBUTTONUP) ||
			((event.type == Common::EVENT_KEYDOWN) && ((event.kbd.keycode == Common::KEYCODE_ESCAPE)
			|| (event.kbd.keycode == Common::KEYCODE_RETURN) || (event.kbd.keycode == Common::KEYCODE_HASH)
			|| (event.kbd.keycode == Common::KEYCODE_PLUS))))
			break;
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
	_vm->_seeScroll = false;
	_vm->_holdLeftMouse = false; // Used in Lucerna::checkclick().

	warning("STUB: Scrolls::scrollModeNormal()");
}

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
		if (event.type == Common::EVENT_KEYDOWN) {
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
	memcpy(played, played + 1, sizeof(played) - 1);
	played[30] = what;
}

bool Dialogs::theyMatch(TuneType &played) {
	byte mistakes = 0;

	for (unsigned int i = 0; i < sizeof(played); i++) {
		if (played[i] != _vm->kTune[i])
			mistakes++;
	}

	return mistakes < 5;
}

void Dialogs::scrollModeMusic() {
	setReadyLight(3);
	_vm->_seeScroll = true;
	CursorMan.showMouse(false);
	_vm->_graphics->loadMouse(kCurFletch);

	TuneType played;
	for (unsigned int i = 0; i < sizeof(played); i++)
		played[i] = kPitchInvalid;
	int8 lastOne = -1, thisOne = -1; // Invalid values.

	_vm->_seeScroll = true;

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
				byte value;
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

	_vm->_seeScroll = false;
	CursorMan.showMouse(true);
}

void Dialogs::resetScrollDriver() {
	_scrollBells = 0;
	_currentFont = kFontStyleRoman;
	_useIcon = 0;
	_vm->_interrogation = 0; // Always reset after a scroll comes up.
}

void Dialogs::ringBell() {   // Pussy's in the well. Who put her in? Little...
	for (int i = 0; i < _scrollBells; i++)
		_vm->errorLed(); // Ring the bell "x" times.
}

void Dialogs::dodgem() {
	_dodgeCoord = _vm->getMousePos();
	g_system->warpMouse(_dodgeCoord.x, _vm->_underScroll); // Move the pointer off the scroll.
}

void Dialogs::unDodgem() {
	Common::Point actCoord = _vm->getMousePos();
	if ((actCoord.x == _dodgeCoord.x) && (actCoord.y == _vm->_underScroll))
		g_system->warpMouse(_dodgeCoord.x, _dodgeCoord.y); // No change, so restore the pointer's original position.
}

void Dialogs::getIcon(int16 x, int16 y, byte which) {
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

	bool centre = false;

	byte iconIndent = 0;
	switch (_useIcon) {
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
		getIcon(mx, my + ly / 2, _useIcon);
		iconIndent = 53;
	}

	for (int i = 0; i <= _maxLineNum; i++) {
		if (!_scroll[i].empty())
			switch (_scroll[i][_scroll[i].size() - 1]) {
			case kControlCenter:
				centre = true;
				_scroll[i].deleteLastChar();
				break;
			case kControlLeftJustified:
				centre = false;
				_scroll[i].deleteLastChar();
				break;
			case kControlQuestion:
				_shadowBoxX = mx + lx;
				_shadowBoxY = my + ly;
				_scroll[i].setChar(' ', 0);
				_vm->_graphics->drawShadowBox(_shadowBoxX - 65, _shadowBoxY - 24, _shadowBoxX - 5, _shadowBoxY - 10, "Yes.");
				_vm->_graphics->drawShadowBox(_shadowBoxX + 5, _shadowBoxY - 24, _shadowBoxX + 65, _shadowBoxY - 10, "No.");
				break;
			}

		if (centre)
			say(320 - _scroll[i].size() * 4 + iconIndent, my, _scroll[i]);
		else
			say(mx + iconIndent, my, _scroll[i]);

		my += 12;
	}

	_vm->_underScroll = my * 2 + 6; // Multiplying because of the doubled screen height.
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

	if ((_vm->_talkX - xw) < 0)
		xc = -(_vm->_talkX - xw);
	if ((_vm->_talkX + xw) > 639)
		xc = 639 - (_vm->_talkX + xw);

	// Compute triangle coords for the tail of the bubble
	points[0].x = _vm->_talkX - 10;
	points[0].y = yw;
	points[1].x = _vm->_talkX + 10;
	points[1].y = yw;
	points[2].x = _vm->_talkX;
	points[2].y = _vm->_talkY;

	_vm->_graphics->prepareBubble(xc, xw, my, points);

	// Draw the text of the bubble. The centering of the text was improved here compared to Pascal's settextjustify().
	// The font is not the same that outtextxy() uses in Pascal. I don't have that, so I used characters instead.
	// It's almost the same, only notable differences are '?', '!', etc.
	for (int i = 0; i <= _maxLineNum; i++) {
		int16 x = xc + _vm->_talkX - _scroll[i].size() / 2 * 8;
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

void Dialogs::setBubbleStateNatural() {
	_vm->_talkX = 320;
	_vm->_talkY = 200;
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

void Dialogs::stripTrailingSpaces(Common::String &str) {
	while (str[str.size() - 1] == ' ')
		str.deleteLastChar();
}

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

void Dialogs::callDialogDriver() {
//	bool was_virtual; // Was the mouse cursor virtual on entry to this proc?
	warning("STUB: Scrolls::calldrivers()");

	_vm->_sound->stopSound();

	setReadyLight(0);
	_scReturn = false;
	bool mouthnext = false;
	bool callSpriteRun = true; // Only call sprite_run the FIRST time.

	switch (_vm->_buffer[_vm->_bufSize - 1]) {
	case kControlToBuffer:
		_vm->_bufSize--;
		break; // ^D = (D)on't include pagebreak
	case kControlSpeechBubble:
	case kControlQuestion:
		break; // ^B = speech (B)ubble, ^Q = (Q)uestion in dialogue box
	default:
		_vm->_buffer[_vm->_bufSize] = kControlParagraph;
		_vm->_bufSize++;
	}

	uint16 size = _vm->_bufSize;

	for (uint16 i = 0; i < size; i++) {
		if (mouthnext) {
			if (_vm->_buffer[i] == kControlRegister)
				_param = 0;
			else if (('0' <= _vm->_buffer[i]) && (_vm->_buffer[i] <= '9'))
				_param = _vm->_buffer[i] - 48;
			else if (('A' <= _vm->_buffer[i]) && (_vm->_buffer[i] <= 'Z'))
				_param = _vm->_buffer[i] - 55;

			mouthnext = false;
		} else {
			switch (_vm->_buffer[i]) {
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
					AnimationType *spr = &_vm->_animation->_sprites[_param - 1];
					if ((_param > _vm->_animation->kSpriteNumbMax) || (!spr->_quick)) { // Not valid.
						_vm->errorLed();
						setBubbleStateNatural();
					} else
						spr->chatter(); // Normal sprite talking routine.
				} else if ((10 <= _param) && (_param <= 36)) {
					// Quasi-peds. (This routine performs the same
					// thing with QPs as triptype.chatter does with the
					// sprites.)
					PedType *quasiPed = &_vm->_peds[_vm->kQuasipeds[_param - 10]._whichPed];
					_vm->_talkX = quasiPed->_x;
					_vm->_talkY = quasiPed->_y; // Position.
					
					_vm->_graphics->setDialogColor(_vm->kQuasipeds[_param - 10]._backgroundColor, _vm->kQuasipeds[_param - 10]._textColor);
				} else {
					_vm->errorLed(); // Not valid.
					setBubbleStateNatural();
				}

				drawBubble(&Avalanche::Dialogs::scrollModeNormal);

				reset();

				if (_scReturn)
					return;
				break;

			// CHECME: The whole kControlNegative block seems completely unused, as the only use (the easter egg check) is a false positive 
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
					displayText(_vm->_favouriteDrink + kControlToBuffer);
					break;
				case 4:
					displayText(_vm->_favouriteSong + kControlToBuffer);
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
						displayScrollChain('q', 37); // You find the sixpence.
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
				_scroll[_maxLineNum] += _vm->_buffer[i];
				break;
			}
		}
	}
}

void Dialogs::displayText(Common::String text) { // TODO: REPLACE BUFFER WITH A STRING!!!!!!!!!!
	_vm->_bufSize = text.size();
	memcpy(_vm->_buffer, text.c_str(), _vm->_bufSize);
	callDialogDriver();
}

bool Dialogs::displayQuestion(Common::String question) {
	displayText(question + kControlNewLine + kControlQuestion);
	warning("STUB: Dialogs::displayQuestion()");
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

// From Visa:

void Dialogs::unSkrimble() {
	for (uint16  i = 0; i < _vm->_bufSize; i++)
		_vm->_buffer[i] = (~(_vm->_buffer[i] - (i + 1))) % 256;
}

void Dialogs::doTheBubble() {
	_vm->_buffer[_vm->_bufSize] = 2;
	_vm->_bufSize++;
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
			Common::String todisplay = Common::String::format("%cError accessing scroll %c%d", Dialogs::kControlBell, block, point);
			displayText(todisplay);
		}
		return;
	}

	Common::File sezfile;
	if (!sezfile.open("avalot.sez"))
		::error("AVALANCHE: Visa: File not found: avalot.sez");

	sezfile.seek(sez_offset);
	_vm->_bufSize = sezfile.readUint16LE();
	sezfile.read(_vm->_buffer, _vm->_bufSize);
	sezfile.close();
	unSkrimble();

	if (bubbling)
		doTheBubble();

	callDialogDriver();
}

/**
 * Start speech
 * @remarks	Originally called 'speech'
 */
void Dialogs::speak(byte who, byte subject) {
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
	_vm->_bufSize = sezfile.readUint16LE();
	sezfile.read(_vm->_buffer, _vm->_bufSize);
	sezfile.close();

	unSkrimble();
	doTheBubble();

	callDialogDriver();
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
				displayScrollChain('q', 68);
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
					Common::String tmpStr = Common::String::format("Can you get me %s, please?%c2%c", objStr.c_str(), Dialogs::kControlRegister, Dialogs::kControlSpeechBubble);
					displayText(tmpStr);
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
		case kPeopleIbythneth:
			if (_vm->_givenBadgeToIby) {
				displayScrollChain('q', 33); // Thanks a lot!
				return; // And leave the proc.
			}
			break; // Or... just continue, 'cos he hasn't got it.
		case kPeopleDogfood:
			if (_vm->_wonNim) { // We've won the game.
				displayScrollChain('q', 6); // "I'm Not Playing!"
				return; // Zap back.
			} else
				_vm->_askedDogfoodAboutNim = true;
			break;
		case kPeopleAyles:
			if (!_vm->_aylesIsAwake) {
				displayScrollChain('q', 43); // He's fast asleep!
				return;
			} else if (!_vm->_givenPenToAyles) {
				displayScrollChain('q', 44); // Can you get me a pen, Avvy?
				return;
			}
			break;

		case kPeopleJacques:
			displayScrollChain('q', 43);
			return;

		case kPeopleGeida:
			if (_vm->_givenPotionToGeida)
				_vm->_geidaFollows = true;
			else {
				displayScrollChain('u', 17);
				return;
			}
			break;
		case kPeopleSpurge:
			if (!_vm->_sittingInPub) {
				displayScrollChain('q', 71); // Try going over and sitting down.
				return;
			} else {
				if (_vm->_spurgeTalkCount < 5)
					_vm->_spurgeTalkCount++;
				if (_vm->_spurgeTalkCount > 1) { // no. 1 falls through
					displayScrollChain('q', 70 + _vm->_spurgeTalkCount);
					return;
				}
			}
			break;
		}
	// On a subject. Is there any reason to block it?
	} else if ((whom == kPeopleAyles) && (!_vm->_aylesIsAwake)) {
		displayScrollChain('q', 43); // He's fast asleep!
		return;
	}

	if (whom > 149)
		whom -= 149;

	bool noMatches = true;
	for (int i = 0; i <= _vm->_animation->kSpriteNumbMax; i++) {
		if (_vm->_animation->_sprites[i]._stat._acciNum == whom) {
			Common::String tmpStr = Common::String::format("%c%c%c", Dialogs::kControlRegister, i + 49, Dialogs::kControlToBuffer);
			displayText(tmpStr);
			noMatches = false;
			break;
		}
	}

	if (noMatches) {
		Common::String tmpStr = Common::String::format("%c%c%c", Dialogs::kControlRegister, Dialogs::kControlRegister, Dialogs::kControlToBuffer);
		displayText(tmpStr);
	}

	speak(whom, _vm->_subjectNum);

	if (!_noError)
		displayScrollChain('n', whom); // File not found!

	if ((_vm->_subjectNum == 0) && ((whom + 149) == kPeopleCrapulus)) { // Crapulus: get the badge - first time only
		_vm->_objects[kObjectBadge - 1] = true;
		_vm->refreshObjectList();
		displayScrollChain('q', 1); // Circular from Cardiff.
		_vm->_talkedToCrapulus = true;
		_vm->setRoom(kPeopleCrapulus, kRoomDummy); // Crapulus walks off.

		AnimationType *spr = &_vm->_animation->_sprites[1];
		spr->_vanishIfStill = true;
		spr->walkTo(2); // Walks away.

		_vm->incScore(2);
	}
}

void Dialogs::sayIt(Common::String str) {
	Common::String x = str;
	x.setChar(toupper(x[0]), 0);
	Common::String tmpStr = Common::String::format("%c1%s.%c%c2", kControlRegister, x.c_str(), kControlSpeechBubble, kControlRegister);
	displayText(tmpStr);
}
} // End of namespace Avalanche
