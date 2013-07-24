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
#include "avalanche/basher2.h"

#include "common/textconsole.h"

namespace Avalanche {

headtype *headtype::init(char trig, char alttrig, Common::String name, byte p, func dw, func dc, Dropdown *dr) {
	_dr = dr;

	trigger = trig;
	alttrigger = alttrig;
	title = name;
	position = p;
	xpos = (position - 1) * _dr->spacing + _dr->indent;
	xright = position * _dr->spacing + _dr->indent;
	do_setup = dw;
	do_choose = dc;

	return this;
}

void headtype::display() {
	_dr->_vm->_gyro.off(); /*MT*/
	_dr->chalk(xpos, 1, trigger, title, true);
	_dr->_vm->_gyro.on(); /*MT*/
}

void headtype::highlight() {
	_dr->_vm->_gyro.off();
	_dr->_vm->_gyro.off_virtual();

	//nosound();
	//setactivepage(cp);
	warning("STUB: Dropdown::headytpe::highlight()");

	_dr->hlchalk(xpos, 1, trigger, title, true);
	{
		_dr->ddm_o.left = xpos;
		_dr->ddm_o.menunow = true;
		_dr->_vm->_gyro.ddmnow = true;
		_dr->ddm_o.menunum = position;
	}
	_dr->_vm->_gyro.cmp = 177; /* Force redraw of cursor. */
}

bool headtype::extdparse(char c) {
	if (c != alttrigger)  
		return true;
	return false;
}






onemenu *onemenu::init(Dropdown *dr) {
	_dr = dr;
	menunow = false;
	_dr->_vm->_gyro.ddmnow = false;
	menunum = 1;
	return this;
}

void onemenu::start_afresh() {
	number = 0;
	width = 0;
	firstlix = false;
	oldy = 0;
	highlightnum = 0;
}

void onemenu::opt(Common::String n, char tr, Common::String key, bool val) {
	int16 l;
	number += 1;
	l = (n + key).size() + 3;
	if (width < l)  width = l;
	{
		optiontype &with = oo[number];
		with.title = n;
		with.trigger = tr;
		with.shortcut = key;
		with.valid = val;
	}
}

void onemenu::displayopt(byte y, bool highlit) {
	warning("STUB: Dropdown::onemenu::displayopt()");
}

void onemenu::display() {
	warning("STUB: Dropdown::onemenu::display()");
}

void onemenu::wipe() {
	warning("STUB: Dropdown::onemenu::wipe()");
}

void onemenu::movehighlight(int8 add) {
	warning("STUB: Dropdown::onemenu::movehighlight()");
}

void onemenu::lightup() {  /* This makes the menu highlight follow the mouse.*/
	warning("STUB: Dropdown::onemenu::lightup()");
	/*if ((_dr->_vm->_gyro.mx < flx1 * 8) || (_dr->_vm->_gyro.mx > flx2 * 8) ||
		(_dr->_vm->_gyro.my <= 12) || (_dr->_vm->_gyro.my > fly - 3))
			return;
	highlightnum = (_dr->_vm->_gyro.my - 13) / 10;
	if (highlightnum == oldy)  return;
	movehighlight(0);*/
	
}

void onemenu::select(byte n) {    /* Choose which one you want. */
	if (! oo[n + 1].valid)  return;
	choicenum = n;
	wipe();

	if (choicenum == number)  choicenum -= 1; /* Off the bottom. */
	if (choicenum > number)  choicenum = 0; /* Off the top, I suppose. */

	_dr->ddm_m.ddms[menunum].do_choose;
}

void onemenu::keystroke(char c) {
	byte fv;
	bool found;
	c = toupper(c);
	found = false;
	for (fv = 1; fv <= number; fv ++) {
		if ((toupper(oo[fv].trigger) == c) && oo[fv].valid) {
			select(fv - 1);
			found = true;
		}
	}
	if (! found)
		_dr->_vm->_gyro.blip();
}





menuset *menuset::init(Dropdown *dr) {
	_dr = dr;
	howmany = 0;
	return this;
}

void menuset::create(char t, Common::String n, char alttrig, func dw, func dc) {
	howmany ++;
	ddms[howmany].init(t, alttrig, n, howmany, dw, dc, _dr);
}

void menuset::update() {
	const bytefield menuspace = {0, 0, 80, 9};
	byte fv, page_, savecp;

	/*setactivepage(3);
	setfillstyle(1, _dr->menu_b);
	bar(0, 0, 640, 9);*/
	_dr->_vm->_graphics->drawBar(0, 0, 640, 10, _dr->menu_b);

	savecp = _dr->_vm->_gyro.cp;
	_dr->_vm->_gyro.cp = 3;

	for (fv = 1; fv <= howmany; fv ++)
		ddms[fv].display();

	for (page_ = 0; page_ <= 1; page_ ++)
		_dr->_vm->_trip.getset[page_].remember(menuspace);

	_dr->_vm->_gyro.cp = savecp;
}

void menuset::extd(char c) {
	byte fv;
	fv = 1;
	while ((fv <= howmany) && (ddms[fv].extdparse(c)))  fv += 1;
	if (fv > howmany)  return;
	getcertain(fv);
}

void menuset::getcertain(byte fv) {
	{
		if (_dr->ddm_o.menunow) {
			_dr->ddm_o.wipe(); /* get rid of menu */
			if (_dr->ddm_o.menunum == ddms[fv].position)  return; /* clicked on own highlight */
		}
		ddms[fv].highlight();
		ddms[fv].do_setup;
	}
}

void menuset::getmenu(int16 x) {
	byte fv;
	fv = 0;
	do {
		fv += 1;
		if ((x > ddms[fv].xpos * 8) && (x < ddms[fv].xright * 8)) {
			getcertain(fv);
			return;
		}
	} while (!(fv > howmany));
}





void Dropdown::setParent(AvalancheEngine *vm) {
	_vm = vm;
}

void Dropdown::find_what_you_can_do_with_it() {
	switch (_vm->_gyro.thinks) {
	case _vm->_gyro.wine:
	case _vm->_gyro.potion:
	case _vm->_gyro.ink:
		_vm->_gyro.verbstr = Common::String(_vm->_acci.vb_exam) + _vm->_acci.vb_drink;
		break;
	case _vm->_gyro.bell:
		_vm->_gyro.verbstr = Common::String(_vm->_acci.vb_exam) + _vm->_acci.vb_ring;
		break;
	case _vm->_gyro.chastity:
		_vm->_gyro.verbstr = Common::String(_vm->_acci.vb_exam) + _vm->_acci.vb_wear;
		break;
	case _vm->_gyro.lute:
		_vm->_gyro.verbstr = Common::String(_vm->_acci.vb_exam) + _vm->_acci.vb_play;
		break;
	case _vm->_gyro.mushroom:
	case _vm->_gyro.onion:
		_vm->_gyro.verbstr = Common::String(_vm->_acci.vb_exam) + _vm->_acci.vb_eat;
		break;
	case _vm->_gyro.clothes:
		_vm->_gyro.verbstr = Common::String(_vm->_acci.vb_exam) + _vm->_acci.vb_wear;
		break;
	default:
		_vm->_gyro.verbstr = _vm->_acci.vb_exam; /* anything else */
	}
}

void Dropdown::chalk(int16 x, int16 y, char t, Common::String z, bool valid) {
	byte ander;
	if (valid)
		ander = 255;
	else
		ander = 170;

	for (byte fv = 0; fv < z.size(); fv++)
		for (byte ff = 0; ff < 8; ff++) {
			byte pixel = ~(_vm->_gyro.little[z[fv]][ff] & ander); // Note that it's the bitwise NOT operator!
			for (byte bit = 0; bit < 8; bit++) {
				byte pixelBit = (pixel >> bit) & 1;
				*_vm->_graphics->getPixel(x * 8 + fv * 8 + 7 - bit, y + ff) = pixelBit + (pixelBit << 1) + (pixelBit << 2);
				// We don't have to bother with the planes, since they all have the same value. See the original.
				// Note that it's the bitwise OR operator!
			}
		}

	if (! z.contains(t))
		return;
	else {
		byte fv;
		for (fv = 0; z[fv] != t; fv++); // Search for the character in the string.
	
		// Similar to the cycle before.
		byte pixel = ~ ander;
		for (byte bit = 0; bit < 8; bit++) {
			byte pixelBit = (pixel >> bit) & 1;
			*_vm->_graphics->getPixel(x * 8 + fv * 8 + 7 - bit, y + 8) = pixelBit | (pixelBit << 1) | (pixelBit << 2);
		}
	}

	_vm->_lucerna.blitfix();
}

void Dropdown::hlchalk(int16 x, int16 y, char t, Common::String z, bool valid) {
/* Highlighted. */
	byte fv, ff, p;
	uint16 pageseg;
	byte ander;

	warning("STUB: Dropdown::hlchalk()");
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
				;     /* Get menu on the left-hand side */
				ddm_o.wipe();
				ddm_m.getmenu((ddm_m.howmany - 1)*spacing + indent);
			}
			break;
		case 'M':
			if (ddm_o.menunum < ddm_m.howmany)  {
				ddm_o.wipe();
				ddm_m.getcertain(ddm_o.menunum + 1);
			} else {
				;     /* Get menu on the far right-hand side */
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
	{
		ddm_o.start_afresh();
		ddm_o.opt("Help...", 'H', "f1", true);
		ddm_o.opt("Boss Key", 'B', "alt-B", true);
		ddm_o.opt("Untrash screen", 'U', "ctrl-f7", true);
		ddm_o.opt("Score and rank", 'S', "f9", true);
		ddm_o.opt("About Avvy...", 'A', "shift-f10", true);
		ddm_o.display();
	}
}

void Dropdown::ddm__file() {
	{
		ddm_o.start_afresh();
		ddm_o.opt("New game", 'N', "f4", true);
		ddm_o.opt("Load...", 'L', "^f3", true);
		ddm_o.opt("Save", 'S', "^f2", _vm->_gyro.alive);
		ddm_o.opt("Save As...", 'v', "", _vm->_gyro.alive);
		ddm_o.opt("DOS Shell", 'D', _vm->_gyro.atkey + '1', true);
		ddm_o.opt("Quit", 'Q', "alt-X", true);
		ddm_o.display();
	}
}

void Dropdown::ddm__action() {
	Common::String n;
	n = Common::String(_vm->_gyro.f5_does().c_str() + 2, 253);

	{
		ddm_o.start_afresh();
		if (n == "")
			ddm_o.opt("Do something", 'D', "f5", false);
		else
			ddm_o.opt(Common::String(n.c_str() + 2, 253), n[1], "f5", true);
		ddm_o.opt("Pause game", 'P', "f6", true);
		if (_vm->_gyro.dna.room == 99)
			ddm_o.opt("Journey thither", 'J', "f7", _vm->_trip.neardoor());
		else
			ddm_o.opt("Open the door", 'O', "f7", _vm->_trip.neardoor());
		ddm_o.opt("Look around", 'L', "f8", true);
		ddm_o.opt("Inventory", 'I', "Tab", true);
		if (_vm->_trip.tr[1].xs == _vm->_gyro.walk)
			ddm_o.opt("Run fast", 'R', "^R", true);
		else
			ddm_o.opt("Walk slowly", 'W', "^W", true);
		ddm_o.display();
	}
}

void Dropdown::ddm__people() {
	byte here;
	char fv;

	people = "";
	here = _vm->_gyro.dna.room;

	{
		ddm_o.start_afresh();
		for (fv = '\226'; fv <= '\262'; fv ++)
			if (_vm->_gyro.whereis[fv] == here) {
				ddm_o.opt(_vm->_gyro.getname(fv), _vm->_gyro.getnamechar(fv), "", true);
				people = people + fv;
			}
		ddm_o.display();
	}
}

void Dropdown::ddm__objects() {
	char fv;
	{
		ddm_o.start_afresh();
		for (fv = '\1'; fv <= numobjs; fv ++)
			if (_vm->_gyro.dna.obj[fv])
				ddm_o.opt(_vm->_gyro.get_thing(fv), _vm->_gyro.get_thingchar(fv), "", true);
		ddm_o.display();
	}
}

Common::String Dropdown::himher(char x) {         /* Returns "im" for boys, and "er" for girls.*/
	if (x < 175)
		return "im";
	else
		return "er";
}

void Dropdown::ddm__with() {
	warning("STUB: Dropdown::ddm__with()");
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
		_vm->_lucerna.callverb(_vm->_acci.vb_help);
		break;
	case 1:
		_vm->_lucerna.callverb(_vm->_acci.vb_boss);
		break;
	case 2:
		_vm->_lucerna.major_redraw();
		break;
	case 3:
		_vm->_lucerna.callverb(_vm->_acci.vb_score);
		break;
	case 4:
		_vm->_lucerna.callverb(_vm->_acci.vb_info);
		break;
	}
}

void Dropdown::do__file() {
	switch (ddm_o.choicenum) {
		/* New game, load, save, save as, DOS shell, about, quit. */
	case 0:
		_vm->_lucerna.callverb(_vm->_acci.vb_restart);
		break;
	case 1: {
		_vm->_acci.realwords[2] = "";
		_vm->_lucerna.callverb(_vm->_acci.vb_load);
	}
	break;
	case 2: {
		_vm->_acci.realwords[2] = "";
		_vm->_lucerna.callverb(_vm->_acci.vb_save);
	}
	break;
	case 3:
		_vm->_basher.filename_edit();
		break;
	case 4:
		_vm->_enid.back_to_bootstrap(2);
		break;
	case 5:
		_vm->_lucerna.callverb(_vm->_acci.vb_quit);
		break;
	}
}

void Dropdown::do__action() {
	Common::String n;
	switch (ddm_o.choicenum) {
		/* Get up/pause game/open door/look/inv/walk-run */
	case 0: {
		_vm->_acci.person = _vm->_acci.pardon;
		_vm->_acci.thing = _vm->_acci.pardon;
		n = _vm->_gyro.f5_does();
		_vm->_lucerna.callverb(n[1]);
	}
	break;
	case 1:
		_vm->_lucerna.callverb(_vm->_acci.vb_pause);
		break;
	case 2:
		_vm->_lucerna.callverb(_vm->_acci.vb_open);
		break;
	case 3:
		_vm->_lucerna.callverb(_vm->_acci.vb_look);
		break;
	case 4:
		_vm->_lucerna.callverb(_vm->_acci.vb_inv);
		break;
	case 5: {
		if (_vm->_trip.tr[1].xs == _vm->_gyro.walk)  _vm->_trip.tr[1].xs = _vm->_gyro.run;
		else _vm->_trip.tr[1].xs = _vm->_gyro.walk;
		_vm->_trip.newspeed();
	}
	break;
	}
}

void Dropdown::do__objects() {
	_vm->_lucerna.thinkabout(_vm->_gyro.objlist[ddm_o.choicenum + 1], _vm->_gyro.a_thing);
}

void Dropdown::do__people() {
	_vm->_lucerna.thinkabout(people[ddm_o.choicenum + 1], _vm->_gyro.a_person);
	_vm->_gyro.last_person = people[ddm_o.choicenum + 1];
}

void Dropdown::do__with() {
	_vm->_acci.thing = _vm->_gyro.thinks;

	if (_vm->_gyro.thinkthing) {

		_vm->_acci.thing += 49;

		if (_vm->_gyro.verbstr[ddm_o.choicenum + 1] == _vm->_acci.vb_give)
			_vm->_acci.person = _vm->_gyro.last_person;
		else
			_vm->_acci.person = '\376';

	} else {
		switch (_vm->_gyro.verbstr[ddm_o.choicenum + 1]) {
		case '\144': {
			_vm->_acci.thing = '\144';
			_vm->_lucerna.callverb(_vm->_acci.vb_buy);
			return;
		}
		break; /* Beer */
		case '\145': {
			_vm->_acci.thing = '\62';
			_vm->_lucerna.callverb(_vm->_acci.vb_buy);
			return;
		}
		break; /* Wine */
		case '\146': {
			_vm->_acci.thing = '\146';
			_vm->_lucerna.callverb(_vm->_acci.vb_buy);
			return;
		}
		break; /* Whisky */
		case '\147': {
			_vm->_acci.thing = '\147';
			_vm->_lucerna.callverb(_vm->_acci.vb_buy);
			return;
		}
		break; /* Cider */
		case '\150': {
			_vm->_acci.thing = '\153';
			_vm->_lucerna.callverb(_vm->_acci.vb_buy);
			return;
		}
		break; /* Mead */
		case '\151': {
			_vm->_acci.thing = '\103';
			_vm->_lucerna.callverb(_vm->_acci.vb_buy);
			return;
		}
		break; /* Onion (trader) */
		default: {
			_vm->_acci.person = _vm->_acci.thing;
			_vm->_acci.thing = '\376';
		}
		}
	}
	_vm->_lucerna.callverb(_vm->_gyro.verbstr[ddm_o.choicenum + 1]);
}

/*$F- That's all. Now for the ...bar funcs. */


void Dropdown::standard_bar() {   /* Standard menu bar */
	ddm_m.init(this);
	ddm_o.init(this);
	/* Set up menus */

	ddm_m.create('F', "File", '!', &Avalanche::Dropdown::ddm__file, &Avalanche::Dropdown::do__file); /* same ones in map_bar, below, */
	ddm_m.create('G', "Game", '\42', &Avalanche::Dropdown::ddm__game, &Avalanche::Dropdown::do__game); /* Don't forget to change the */
	ddm_m.create('A', "Action", '\36', &Avalanche::Dropdown::ddm__action, &Avalanche::Dropdown::do__action); /* if you change them */
	ddm_m.create('O', "Objects", '\30', &Avalanche::Dropdown::ddm__objects, &Avalanche::Dropdown::do__objects); /* here... */
	ddm_m.create('P', "People", '\31', &Avalanche::Dropdown::ddm__people, &Avalanche::Dropdown::do__people);
	ddm_m.create('W', "With", '\21', &Avalanche::Dropdown::ddm__with, &Avalanche::Dropdown::do__with);

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

void Dropdown::checkclick() {   /* only for when the menu's displayed */
	if (_vm->_gyro.mpress > 0) {
		if (_vm->_gyro.mpy > 10) {
			if (!((ddm_o.firstlix) &&
			   ((_vm->_gyro.mpx >= ddm_o.flx1 * 8) && (_vm->_gyro.mpx <= ddm_o.flx2 * 8) &&
			   (_vm->_gyro.mpy >= 12) && (_vm->_gyro.mpy <= ddm_o.fly)))) {
			     /* Clicked OUTSIDE the menu. */
				if (ddm_o.menunow)
					ddm_o.wipe();
			}    /* No "else"- clicking on menu has no effect (only releasing) */
		} else {
			 /* Clicked on menu bar */
			ddm_m.getmenu(_vm->_gyro.mpx);
		}
	} else {
		;     /* NOT clicked button... */
		if (_vm->_gyro.mrelease > 0) {
			if ((ddm_o.firstlix) &&
			   ((_vm->_gyro.mrx >= ddm_o.flx1 * 8) && (_vm->_gyro.mrx <= ddm_o.flx2 * 8) &&
			   (_vm->_gyro.mry >= 12) && (_vm->_gyro.mry <= ddm_o.fly)))
					ddm_o.select((_vm->_gyro.mry - 13) / 10);
		}
	}
}

void Dropdown::menu_link() {
	if (! ddm_o.menunow)  return;

	_vm->_gyro.check(); /* find mouse coords & click information */
	checkclick(); /* work out click codes */

	/* Change arrow... */

		
	if ((_vm->_gyro.my >= 0) && (_vm->_gyro.my <= 10))
		_vm->_gyro.newpointer(1); /* up-arrow */
	else if ((_vm->_gyro.my >= 11) && (_vm->_gyro.my <= 169)) {
		if ((_vm->_gyro.mx >= ddm_o.flx1 * 8) && (_vm->_gyro.mx <= ddm_o.flx2 * 8)
			&& (_vm->_gyro.my > 10) && (_vm->_gyro.my <= ddm_o.fly))
			_vm->_gyro.newpointer(3); /* right-arrow */
		else
			_vm->_gyro.newpointer(4); /* fletch */
	} else if ((_vm->_gyro.my >= 169) && (_vm->_gyro.my <= 200))
		_vm->_gyro.newpointer(2); /* screwdriver */

	if (! ddm_o.menunow)
		return;

	ddm_o.lightup();
}


} // End of namespace Avalanche.
