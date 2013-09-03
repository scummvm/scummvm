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
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

 /* DROPDOWN	A customised version of Oopmenu (qv). */

#include "avalanche/avalanche.h"

#include "avalanche/dropdown2.h"

#include "avalanche/lucerna2.h"
#include "avalanche/gyro2.h"
#include "avalanche/acci2.h"
#include "avalanche/trip6.h"
#include "avalanche/enid2.h"

#include "common/textconsole.h"

namespace Avalanche {

headtype *headtype::init(char trig, char alttrig, Common::String name, byte p, func dw, func dc, Dropdown *dr) {
	_dr = dr;

	trigger = trig;
	alttrigger = alttrig;
	title = name;
	position = p;
	xpos = position * _dr->spacing + _dr->indent;
	xright = (position + 1) * _dr->spacing + _dr->indent;
	do_setup = dw;
	do_choose = dc;

	return this;
}

void headtype::display() {
	CursorMan.showMouse(false); /*MT*/
	_dr->chalk(xpos, 1, trigger, title, true);
	CursorMan.showMouse(true); /*MT*/
}

void headtype::highlight() {
	CursorMan.showMouse(false);

	//nosound();
	//setactivepage(cp);
	warning("STUB: Dropdown::headytpe::highlight()");

	_dr->hlchalk(xpos, 1, trigger, title, true);
	
	_dr->ddm_o.left = xpos;
	_dr->ddm_o.menunow = true;
	_dr->_vm->_gyro->ddmnow = true;
	_dr->ddm_o.menunum = position;
	
	_dr->_vm->_gyro->cmp = 177; /* Force redraw of cursor. */
}

bool headtype::extdparse(char c) {
	if (c != alttrigger)  
		return true;
	return false;
}






void onemenu::init(Dropdown *dr) {
	_dr = dr;
	menunow = false;
	_dr->_vm->_gyro->ddmnow = false;
	menunum = 1;
}

void onemenu::start_afresh() {
	number = 0;
	width = 0;
	firstlix = false;
	oldy = 0;
	highlightnum = 0;
}

void onemenu::opt(Common::String n, char tr, Common::String key, bool val) {
	int16 l = (n + key).size() + 3;
	if (width < l)
		width = l;

	oo[number].title = n;
	oo[number].trigger = tr;
	oo[number].shortcut = key;
	oo[number].valid = val;
	number++;
}

void onemenu::displayopt(byte y, bool highlit) {
	byte backgroundColor;
	if (highlit)
		backgroundColor = 0;
	else
		backgroundColor = 7;
	_dr->_vm->_graphics->_surface.fillRect(Common::Rect((flx1 + 1) * 8, 3 + (y + 1) * 10, (flx2 + 1) * 8, 13 + (y + 1) * 10), backgroundColor);
	//bar((flx1 + 1) * 8, 3 + y * 10, (flx2 + 1) * 8, 12 + y * 10);

	Common::String data = oo[y].title;
	while ((data + oo[y].shortcut).size() < width)
		data += ' '; /* Pad oo[y] spaces. */
	data = data + oo[y].shortcut;

	if (highlit)
		_dr->hlchalk(left, 4 + (y + 1) * 10, oo[y].trigger, data, oo[y].valid);
	else
		_dr->chalk(left, 4 + (y + 1) * 10, oo[y].trigger, data, oo[y].valid);
}

void onemenu::display() {
	CursorMan.showMouse(false);
	/*setactivepage(cp);
	setvisualpage(cp);
	setfillstyle(1, menu_b);
	setcolor(menu_border);*/
	firstlix = true;
	flx1 = left - 2;
	flx2 = left + width;
	fly = 15 + number * 10;
	menunow = true;
	_dr->_vm->_gyro->ddmnow = true;

	_dr->_vm->_graphics->_surface.fillRect(Common::Rect((flx1 + 1) * 8, 12, (flx2 + 1) * 8, fly), _dr->menu_b);
	_dr->_vm->_graphics->_surface.frameRect(Common::Rect((flx1 + 1) * 8 - 1, 11, (flx2 + 1) * 8 + 1, fly + 1), _dr->menu_border);
	/*bar((flx1 + 1) * 8, 12, (flx2 + 1) * 8, fly);
	rectangle((flx1 + 1) * 8 - 1, 11, (flx2 + 1) * 8 + 1, fly + 1);*/

	displayopt(0, true);
	for (byte y = 1; y < number; y++)
		displayopt(y, false);
	
	_dr->_vm->_gyro->defaultled = 1;
	_dr->_vm->_gyro->cmp = 177;
	//mousepage(cp);
	CursorMan.showMouse(true); // 4 = fletch
}

void onemenu::wipe() {
	//setactivepage(cp);
	CursorMan.showMouse(false); 
	
	_dr->chalk(_dr->ddm_m.ddms[_dr->ddm_o.menunum].xpos, 1, _dr->ddm_m.ddms[_dr->ddm_o.menunum].trigger, _dr->ddm_m.ddms[_dr->ddm_o.menunum].title, true);
	/*mblit(flx1, 11, flx2 + 1, fly + 1, 3, cp);
	blitfix();*/

	menunow = false;
	_dr->_vm->_gyro->ddmnow = false;
	firstlix = false;
	_dr->_vm->_gyro->defaultled = 2;

	CursorMan.showMouse(true); 
}

void onemenu::movehighlight(int8 add) {
	int8 hn;
	if (add != 0) {
		hn = highlightnum + add;
		if ((hn < 0) || (hn >= number))
			return;
		highlightnum = hn;
	}
	//setactivepage(cp);
	CursorMan.showMouse(false); 
	displayopt(oldy, false);
	displayopt(highlightnum, true);
	//setactivepage(1 - cp);
	oldy = highlightnum;
	CursorMan.showMouse(true); 
}

void onemenu::lightup(Common::Point cursorPos) { 
	if ((cursorPos.x < flx1 * 8) || (cursorPos.x > flx2 * 8) || (cursorPos.y <= 25) || (cursorPos.y > ((fly - 3) * 2 + 1)))
		return;
	highlightnum = (cursorPos.y - 26) / 20;
	if (highlightnum == oldy)
		return;
	movehighlight(0);
}

void onemenu::select(byte n) {    /* Choose which one you want. */
	if (!oo[n].valid)
		return;

	choicenum = n;
	wipe();

	if (choicenum == number)
		choicenum--; /* Off the bottom. */
	if (choicenum > number)
		choicenum = 0; /* Off the top, I suppose. */

	(_dr->*_dr->ddm_m.ddms[menunum].do_choose)();
}

void onemenu::keystroke(char c) {
	byte fv;
	bool found;
	c = toupper(c);
	found = false;
	for (fv = 0; fv < number; fv++) {
		if ((toupper(oo[fv].trigger) == c) && oo[fv].valid) {
			select(fv);
			found = true;
		}
	}
	if (!found)
		_dr->_vm->_gyro->blip();
}





void menuset::init(Dropdown *dr) {
	_dr = dr;
	howmany = 0;
}

void menuset::create(char t, Common::String n, char alttrig, func dw, func dc) {
	ddms[howmany].init(t, alttrig, n, howmany, dw, dc, _dr);
	howmany++;
}

void menuset::update() {
	const bytefield menuspace = {0, 0, 80, 9};
	byte fv, page_, savecp;

	/*setactivepage(3);
	setfillstyle(1, _dr->menu_b);
	bar(0, 0, 640, 9);*/
	_dr->_vm->_graphics->drawBar(0, 0, 640, 10, _dr->menu_b);

	savecp = _dr->_vm->_gyro->cp;
	_dr->_vm->_gyro->cp = 3;

	for (fv = 0; fv < howmany; fv++)
		ddms[fv].display();

	for (page_ = 0; page_ <= 1; page_++)
		_dr->_vm->_trip->getset[page_].remember(menuspace);

	_dr->_vm->_gyro->cp = savecp;
}

void menuset::extd(char c) {
	byte fv;
	fv = 0;
	while ((fv < howmany) && (ddms[fv].extdparse(c)))
		fv ++;
	if (fv == howmany)
		return;
	getcertain(fv);
}

void menuset::getcertain(byte fv) {
	if (_dr->ddm_o.menunow) {
		_dr->ddm_o.wipe(); // Get rid of menu.
		if (_dr->ddm_o.menunum == ddms[fv].position)
			return; // Clicked on own highlight.
	}
	ddms[fv].highlight();
	(_dr->*ddms[fv].do_setup)();
}

void menuset::getmenu(int16 x) {
	byte fv;
	fv = 0;
	do {
		if ((x > ddms[fv].xpos * 8) && (x < ddms[fv].xright * 8)) {
			getcertain(fv);
			return;
		}
		fv++;
	} while (fv < howmany);
}





Dropdown::Dropdown(AvalancheEngine *vm) {
	_vm = vm;
	ddm_o.init(this);
	ddm_m.init(this);
}

void Dropdown::find_what_you_can_do_with_it() {
	switch (_vm->_gyro->thinks) {
	case Gyro::wine:
	case Gyro::potion:
	case Gyro::ink:
		_vm->_gyro->verbstr = Common::String(_vm->_acci->kVerbCodeExam) + _vm->_acci->kVerbCodeDrink;
		break;
	case Gyro::bell:
		_vm->_gyro->verbstr = Common::String(_vm->_acci->kVerbCodeExam) + _vm->_acci->kVerbCodeRing;
		break;
	case Gyro::chastity:
		_vm->_gyro->verbstr = Common::String(_vm->_acci->kVerbCodeExam) + _vm->_acci->kVerbCodeWear;
		break;
	case Gyro::lute:
		_vm->_gyro->verbstr = Common::String(_vm->_acci->kVerbCodeExam) + _vm->_acci->kVerbCodePlay;
		break;
	case Gyro::mushroom:
	case Gyro::onion:
		_vm->_gyro->verbstr = Common::String(_vm->_acci->kVerbCodeExam) + _vm->_acci->kVerbCodeEat;
		break;
	case Gyro::clothes:
		_vm->_gyro->verbstr = Common::String(_vm->_acci->kVerbCodeExam) + _vm->_acci->kVerbCodeWear;
		break;
	default:
		_vm->_gyro->verbstr = _vm->_acci->kVerbCodeExam; /* anything else */
	}
}

void Dropdown::chalk(int16 x, int16 y, char t, Common::String z, bool valid) {
	byte ander;
	if (valid)
		ander = 255;
	else
		ander = 170;

	fontType font;
	for (byte fv = 0; fv < z.size(); fv++)
		for (byte ff = 0; ff < 8; ff++) {
			font[z[fv]][ff] = _vm->_gyro->characters[z[fv]][ff] & ander;
			for (byte i = 0; i < 8; i++)
				*(byte *)_vm->_graphics->_surface.getBasePtr(x * 8 + fv * 8 + i, y + ff) = lightgray;
		}

	_vm->_graphics->drawText(_vm->_graphics->_surface, z, font, 8, x * 8, y, black);
	
	// Underline the selected character.
	if (! z.contains(t))
		return;
	else {
		byte fv;
		for (fv = 0; z[fv] != t; fv++); // Search for the character in the string.
	
		byte pixel = ander;
		for (byte bit = 0; bit < 8; bit++) {
			byte pixelBit = (pixel >> bit) & 1;
			if (pixelBit)
				*_vm->_graphics->getPixel(x * 8 + fv * 8 + 7 - bit, y + 8) = black;
		}
	}

	_vm->_graphics->refreshScreen();
}

void Dropdown::hlchalk(int16 x, int16 y, char t, Common::String z, bool valid) {
	byte ander;
	if (valid)
		ander = 255;
	else
		ander = 170;

	fontType font;
	for (byte fv = 0; fv < z.size(); fv++)
		for (byte ff = 0; ff < 8; ff++) {
			font[z[fv]][ff] = _vm->_gyro->characters[z[fv]][ff] & ander; // Set the font.
			// And set the background of the text to black.
			for (byte i = 0; i < 8; i++)
				*(byte *)_vm->_graphics->_surface.getBasePtr(x * 8 + fv * 8 + i, y + ff) = black;
		}

	_vm->_graphics->drawText(_vm->_graphics->_surface, z, font, 8, x * 8, y, white);

	// Underline the selected character.
	if (z.contains(t)) {
		byte fv;
		for (fv = 0; z[fv] != t; fv++); // Search for the character in the string.

		byte pixel = ander;
		for (byte bit = 0; bit < 8; bit++) {
			byte pixelBit = (pixel >> bit) & 1;
			if (pixelBit)
				*_vm->_graphics->getPixel(x * 8 + fv * 8 + 7 - bit, y + 8) = white;
		}
	}

	_vm->_graphics->refreshScreen();
}

/*funcedure say(x,y:int16; t:char; z:Common::String; f,b:byte);
begin;
 settextjustify(0,2); setfillstyle(1,b); setcolor(f);
 bar(x-3,y-1,x+textwidth(z)+3,y+textheight(z));
 chalk(x,y,t,z);
end;*/

void Dropdown::bleep() {
	warning("STUB: Dropdown::bleep()");
}

void Dropdown::parsekey(char r, char re) {
	switch (r) {
	case 0:
	case 224: {
		switch (re) {
		case 'K':
			if (ddm_o.menunum > 1)  {
				ddm_o.wipe();
				ddm_m.getcertain(ddm_o.menunum - 1);
			} else {
				// Get menu on the left-hand side.
				ddm_o.wipe();
				ddm_m.getmenu((ddm_m.howmany - 1) * spacing + indent);
			}
			break;
		case 'M':
			if (ddm_o.menunum < ddm_m.howmany)  {
				ddm_o.wipe();
				ddm_m.getcertain(ddm_o.menunum + 1);
			} else {
				// Get menu on the far right-hand side.
				ddm_o.wipe();
				ddm_m.getmenu(indent);
			}
			break;
		case 'H':
			ddm_o.movehighlight(-1);
			break;
		case 'P':
			ddm_o.movehighlight(1);
			break;
		default:
			ddm_m.extd(re);
		}
	}
	break;
	case 13:
		ddm_o.select(ddm_o.highlightnum);
		break;
	default: {
		if (ddm_o.menunow)
			ddm_o.keystroke(r);
		}
	}
}

/*$F+  *** Here follow all the ddm__ and do__ funcedures for the DDM system. */

void Dropdown::ddm__game() {
	ddm_o.start_afresh();
	ddm_o.opt("Help...", 'H', "f1", true);
	ddm_o.opt("Boss Key", 'B', "alt-B", true);
	ddm_o.opt("Untrash screen", 'U', "ctrl-f7", true);
	ddm_o.opt("Score and rank", 'S', "f9", true);
	ddm_o.opt("About Avvy...", 'A', "shift-f10", true);
	ddm_o.display();
}

void Dropdown::ddm__file() {
	ddm_o.start_afresh();
	ddm_o.opt("New game", 'N', "f4", true);
	ddm_o.opt("Load...", 'L', "^f3", true);
	ddm_o.opt("Save", 'S', "^f2", _vm->_gyro->alive);
	ddm_o.opt("Save As...", 'v', "", _vm->_gyro->alive);
	ddm_o.opt("DOS Shell", 'D', _vm->_gyro->atkey + '1', true);
	ddm_o.opt("Quit", 'Q', "alt-X", true);
	ddm_o.display();
}

void Dropdown::ddm__action() {
	ddm_o.start_afresh();

	Common::String n = _vm->_gyro->f5_does();
	for (byte i = 0; i < 2; i++)
		if (!n.empty())
			n.deleteChar(0);
	if (n.empty())
		ddm_o.opt("Do something", 'D', "f5", false);
	else
		ddm_o.opt(n, n[0], "f5", true);
	ddm_o.opt("Pause game", 'P', "f6", true);
	if (_vm->_gyro->dna.room == 99)
		ddm_o.opt("Journey thither", 'J', "f7", _vm->_trip->neardoor());
	else
		ddm_o.opt("Open the door", 'O', "f7", _vm->_trip->neardoor());
	ddm_o.opt("Look around", 'L', "f8", true);
	ddm_o.opt("Inventory", 'I', "Tab", true);
	if (_vm->_trip->tr[0].xs == _vm->_gyro->walk)
		ddm_o.opt("Run fast", 'R', "^R", true);
	else
		ddm_o.opt("Walk slowly", 'W', "^W", true);

	ddm_o.display();
}

void Dropdown::ddm__people() {
	if (!people.empty())
		people.clear();
	byte here = _vm->_gyro->dna.room;

	ddm_o.start_afresh();

	for (byte fv = 150; fv <= 178; fv++)
		if (_vm->_gyro->whereis[fv - 150] == here) {
			ddm_o.opt(_vm->_gyro->getname(fv), _vm->_gyro->getnamechar(fv), "", true);
			people = people + fv;
		}

	ddm_o.display();
}

void Dropdown::ddm__objects() {
	ddm_o.start_afresh();
	for (byte fv = 0; fv < numobjs; fv++) {
		if (_vm->_gyro->dna.obj[fv])
			ddm_o.opt(_vm->_gyro->get_thing(fv + 1), _vm->_gyro->get_thingchar(fv + 1), "", true);
	}
	ddm_o.display();
}

Common::String Dropdown::himher(byte x) {
	if (x < 175)
		return "im";
	else
		return "er";
}

void Dropdown::ddm__with() {
	byte fv;
	Common::String verb;
	char vbchar;
	bool n;
	
	ddm_o.start_afresh();

	if (_vm->_gyro->thinkthing) {
		find_what_you_can_do_with_it();

		for (fv = 0; fv < _vm->_gyro->verbstr.size(); fv++) {
			_vm->_acci->verbOpt(_vm->_gyro->verbstr[fv], verb, vbchar);
			ddm_o.opt(verb, vbchar, "", true);
		}

		// We disable the "give" option if: (a), you haven't selected anybody, (b), the _person you've selected isn't in the room, or (c), the _person you've selected is YOU!
		
		if ((_vm->_gyro->last_person == _vm->_gyro->pavalot) || (_vm->_gyro->last_person == _vm->_acci->kNothing) || (_vm->_gyro->whereis[_vm->_gyro->last_person - 150] != _vm->_gyro->dna.room))
			ddm_o.opt("Give to...", 'G', "", false); /* Not here. */
		else {
			ddm_o.opt(Common::String("Give to ") + _vm->_gyro->getname(_vm->_gyro->last_person), 'G', "", true);
			_vm->_gyro->verbstr = _vm->_gyro->verbstr + _vm->_acci->kVerbCodeGive;
		}
	} else {
		ddm_o.opt("Examine", 'x', "", true);
		ddm_o.opt(Common::String("Talk to h") + himher(_vm->_gyro->thinks), 'T', "", true);
		_vm->_gyro->verbstr = Common::String(_vm->_acci->kVerbCodeExam) + _vm->_acci->kVerbCodeTalk;
		switch (_vm->_gyro->thinks) {
		case Gyro::pgeida:
		case Gyro::parkata: {
			ddm_o.opt("Kiss her", 'K', "", true);
			_vm->_gyro->verbstr = _vm->_gyro->verbstr + _vm->_acci->kVerbCodeKiss;
		}
		break;
		case Gyro::pdogfood: {
			ddm_o.opt("Play his game", 'P', "", !_vm->_gyro->dna.wonnim); /* True if you HAVEN'T won. */
			_vm->_gyro->verbstr = _vm->_gyro->verbstr + _vm->_acci->kVerbCodePlay;
		}
		break;
		case Gyro::pmalagauche: {
			n = !_vm->_gyro->dna.teetotal;
			ddm_o.opt("Buy some wine", 'w', "", !_vm->_gyro->dna.obj[_vm->_gyro->wine - 1]);
			ddm_o.opt("Buy some beer", 'b', "", n);
			ddm_o.opt("Buy some whisky", 'h', "", n);
			ddm_o.opt("Buy some cider", 'c', "", n);
			ddm_o.opt("Buy some mead", 'm', "", n);
			_vm->_gyro->verbstr = _vm->_gyro->verbstr + '\145' + '\144' + '\146' + '\147' + '\150';
		}
		break;
		case Gyro::ptrader: {
			ddm_o.opt("Buy an onion", 'o', "", !_vm->_gyro->dna.obj[_vm->_gyro->onion - 1]);
			_vm->_gyro->verbstr = _vm->_gyro->verbstr + '\151';
		}
		break;
		}
	}
	ddm_o.display();
}

/*funcedure ddm__map;
begin;
 with ddm_o do
 begin;
  ddm_o.start_afresh;
  opt('Cancel map','G','f5',true);
  opt('Pause game','P','f6',true);
  opt('Journey thither','J','f7',neardoor);
  opt('Explanation','L','f8',true);
  display;
 end;
end;

funcedure ddm__town;
begin;
 with ddm_o do
 begin;
  ddm_o.start_afresh;
  opt('Argent','A','',true);
  opt('Birmingham','B','',true);
  opt('Nottingham','N','',true);
  opt('Cardiff','C','',true);
  display;
 end;
end;*/

void Dropdown::do__game() {
	switch (ddm_o.choicenum) {
		/* Help, boss, untrash screen. */
	case 0:
		_vm->_lucerna->callverb(_vm->_acci->kVerbCodeHelp);
		break;
	case 1:
		_vm->_lucerna->callverb(_vm->_acci->kVerbCodeBoss);
		break;
	case 2:
		_vm->_lucerna->major_redraw();
		break;
	case 3:
		_vm->_lucerna->callverb(_vm->_acci->kVerbCodeScore);
		break;
	case 4:
		_vm->_lucerna->callverb(_vm->_acci->kVerbCodeInfo);
		break;
	}
}

void Dropdown::do__file() {
	switch (ddm_o.choicenum) {
		/* New game, load, save, save as, DOS shell, about, quit. */
	case 0:
		_vm->_lucerna->callverb(_vm->_acci->kVerbCodeRestart);
		break;
	case 1: {
		if (!_vm->_acci->_realWords[1].empty())
			_vm->_acci->_realWords[1].clear();
		_vm->_lucerna->callverb(_vm->_acci->kVerbCodeLoad);
	}
	break;
	case 2: {
		if (!_vm->_acci->_realWords[1].empty())
			_vm->_acci->_realWords[1].clear();
		_vm->_lucerna->callverb(_vm->_acci->kVerbCodeSave);
	}
	break;
	case 3:
		//_vm->_basher->filename_edit();
		break;
	case 4:
		_vm->_enid->back_to_bootstrap(2);
		break;
	case 5:
		_vm->_lucerna->callverb(_vm->_acci->kVerbCodeQuit);
		break;
	}
}

void Dropdown::do__action() {
	Common::String n;
	switch (ddm_o.choicenum) {
		/* Get up/pause game/open door/look/inv/walk-run */
	case 0: {
		_vm->_acci->_person = _vm->_acci->kPardon;
		_vm->_acci->_thing = _vm->_acci->kPardon;
		n = _vm->_gyro->f5_does();
		_vm->_lucerna->callverb(n[0]);
	}
	break;
	case 1:
		_vm->_lucerna->callverb(_vm->_acci->kVerbCodePause);
		break;
	case 2:
		_vm->_lucerna->callverb(_vm->_acci->kVerbCodeOpen);
		break;
	case 3:
		_vm->_lucerna->callverb(_vm->_acci->kVerbCodeLook);
		break;
	case 4:
		_vm->_lucerna->callverb(_vm->_acci->kVerbCodeInv);
		break;
	case 5: {
		if (_vm->_trip->tr[0].xs == _vm->_gyro->walk)
			_vm->_trip->tr[0].xs = _vm->_gyro->run;
		else
			_vm->_trip->tr[0].xs = _vm->_gyro->walk;
		_vm->_trip->newspeed();
	}
	break;
	}
}

void Dropdown::do__objects() {
	_vm->_lucerna->thinkabout(_vm->_gyro->objlist[ddm_o.choicenum + 1], _vm->_gyro->a_thing);
}

void Dropdown::do__people() {
	_vm->_lucerna->thinkabout(people[ddm_o.choicenum], _vm->_gyro->a_person);
	_vm->_gyro->last_person = people[ddm_o.choicenum];
}

void Dropdown::do__with() {
	_vm->_acci->_thing = _vm->_gyro->thinks;

	if (_vm->_gyro->thinkthing) {

		_vm->_acci->_thing += 49;

		if (_vm->_gyro->verbstr[ddm_o.choicenum] == _vm->_acci->kVerbCodeGive)
			_vm->_acci->_person = _vm->_gyro->last_person;
		else
			_vm->_acci->_person = 254;

	} else {
		switch (_vm->_gyro->verbstr[ddm_o.choicenum]) {
		case 100: { // Beer
			_vm->_acci->_thing = 100;
			_vm->_lucerna->callverb(_vm->_acci->kVerbCodeBuy);
			return;
		}
		break;
		case 101: { // Wine
			_vm->_acci->_thing = 50;
			_vm->_lucerna->callverb(_vm->_acci->kVerbCodeBuy);
			return;
		}
		break;
		case 102: { // Whisky
			_vm->_acci->_thing = 102;
			_vm->_lucerna->callverb(_vm->_acci->kVerbCodeBuy);
			return;
		}
		break; 
		case 103: { // Cider
			_vm->_acci->_thing = 103;
			_vm->_lucerna->callverb(_vm->_acci->kVerbCodeBuy);
			return;
		}
		break; 
		case 104: { // Mead
			_vm->_acci->_thing = 107;
			_vm->_lucerna->callverb(_vm->_acci->kVerbCodeBuy);
			return;
		}
		break; 
		case 105: { // Onion (trader)
			_vm->_acci->_thing = 67;
			_vm->_lucerna->callverb(_vm->_acci->kVerbCodeBuy);
			return;
		}
		break; 
		default: {
			_vm->_acci->_person = _vm->_acci->_thing;
			_vm->_acci->_thing = 254;
			_vm->_gyro->subjnumber = 0;
		}
		}
	}
	_vm->_lucerna->callverb(_vm->_gyro->verbstr[ddm_o.choicenum]);
}



// That's all. Now for the ...bar funcs.

void Dropdown::standard_bar() {   /* Standard menu bar */
	ddm_m.init(this);
	ddm_o.init(this);
	/* Set up menus */

	ddm_m.create('F', "File", '!', &Avalanche::Dropdown::ddm__file, &Avalanche::Dropdown::do__file); // same ones in map_bar, below,
	ddm_m.create('G', "Game", 34, &Avalanche::Dropdown::ddm__game, &Avalanche::Dropdown::do__game); // Don't forget to change the
	ddm_m.create('A', "Action", 30, &Avalanche::Dropdown::ddm__action, &Avalanche::Dropdown::do__action); // if you change them
	ddm_m.create('O', "Objects", 24, &Avalanche::Dropdown::ddm__objects, &Avalanche::Dropdown::do__objects); // here...
	ddm_m.create('P', "People", 25, &Avalanche::Dropdown::ddm__people, &Avalanche::Dropdown::do__people);
	ddm_m.create('W', "With", 17, &Avalanche::Dropdown::ddm__with, &Avalanche::Dropdown::do__with);

	ddm_m.update();
}



/*funcedure map_bar; { Special menu bar for the map (screen 99) }
begin;
 ddm_m.init; ddm_o.init;
 with ddm_m do
 begin; { Set up menus }
  create('G','Game','#',ddm__game,do__game);
  create('F','File','!',ddm__file,do__test);
  create('M','Map','2',ddm__map,do__test);
  create('T','Town',#20,ddm__town,do__test);
  update;
 end;
end;*/

void Dropdown::checkclick(Common::Point cursorPos) {
	warning("STUB: Dropdown::checkclick()");
}

void Dropdown::menu_link() { // TODO: Optimize it ASAP!!! It really needs it...
	Common::Point cursorPos = _vm->getMousePos();
	::Graphics::Surface backup;
	backup.copyFrom(_vm->_graphics->_surface);

	while (!ddm_o.menunow && (cursorPos.y <= 21) && _vm->_lucerna->holdLeftMouse) {
		ddm_m.getmenu(cursorPos.x);
		do 
			_vm->updateEvents();
		while (_vm->_lucerna->holdLeftMouse);
	

		while (!_vm->shouldQuit()) {
			do { 
				_vm->updateEvents();

				// We updadte the cursor's picture.
				cursorPos = _vm->getMousePos();
				// Change arrow...
				if ((0 <= cursorPos.y) && (cursorPos.y <= 21))
					_vm->_gyro->newpointer(1); // Up arrow
				else if ((22 <= cursorPos.y) && (cursorPos.y <= 339)) {
					if ((cursorPos.x >= ddm_o.flx1 * 8) && (cursorPos.x <= ddm_o.flx2 * 8) && (cursorPos.y > 21) && (cursorPos.y <= ddm_o.fly * 2 + 1))
						_vm->_gyro->newpointer(3); // Right-arrow
					else
						_vm->_gyro->newpointer(4); // Fletch
				} else if ((340 <= cursorPos.y) && (cursorPos.y <= 399))
					_vm->_gyro->newpointer(2); // Screwdriver

				ddm_o.lightup(cursorPos);

				_vm->_graphics->refreshScreen();
			} while (!_vm->_lucerna->holdLeftMouse);

			if (_vm->_lucerna->holdLeftMouse) {
				if (cursorPos.y > 21) {
					if (!((ddm_o.firstlix) && ((cursorPos.x >= ddm_o.flx1 * 8) && (cursorPos.x <= ddm_o.flx2 * 8)
						&& (cursorPos.y >= 24) && (cursorPos.y <= (ddm_o.fly * 2 + 1))))) {
							// Clicked OUTSIDE the menu.
							if (ddm_o.menunow) {
								ddm_o.wipe();
								_vm->_lucerna->holdLeftMouse = false;
								return;
							} // No "else"- clicking on menu has no effect (only releasing).
						} 
				} else {
					// Clicked on menu bar.
					if (ddm_o.menunow) {
						ddm_o.wipe();
						_vm->_graphics->_surface.copyFrom(backup);
						_vm->_graphics->refreshScreen();
						
						if (((ddm_o.left * 8) <= cursorPos.x) && (cursorPos.x <= (ddm_o.left * 8 + 80))) { // 80: the width of one menu item on the bar in pixels.
							// If we clicked on the same menu item (the one that is already active) on the bar...
							_vm->_lucerna->holdLeftMouse = false;
							return;
						} else {
							_vm->_lucerna->holdLeftMouse = true;
							break;
						}
					}
				}
	
				// NOT clicked button...
				if ((ddm_o.firstlix) && ((cursorPos.x >= ddm_o.flx1 * 8) && (cursorPos.x <= ddm_o.flx2 * 8)
					&& (cursorPos.y >= 12) && (cursorPos.y <= (ddm_o.fly * 2 + 1)))) {

						// We act only if the button is released over a menu item.
						while (!_vm->shouldQuit()) {
							cursorPos = _vm->getMousePos();
							ddm_o.lightup(cursorPos);
							_vm->_graphics->refreshScreen();

							_vm->updateEvents();
							if (!_vm->_lucerna->holdLeftMouse)
								break;
						}

						uint16 which = (cursorPos.y - 26) / 20;
						ddm_o.select(which);
						if (ddm_o.oo[which].valid) // If the menu item wasn't active, we do nothing.
							return;
				}
			}

		}
	}
}

} // End of namespace Avalanche.
