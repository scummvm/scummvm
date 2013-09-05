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

/* LUCERNA		The screen, [keyboard] and mouse handler.*/

#include "avalanche/avalanche.h"

#include "avalanche/lucerna2.h"
#include "avalanche/gyro2.h"
#include "avalanche/scrolls2.h"
#include "avalanche/visa2.h"
#include "avalanche/timeout2.h"
#include "avalanche/trip6.h"
#include "avalanche/enid2.h"
#include "avalanche/celer2.h"
#include "avalanche/pingo2.h"
#include "avalanche/sequence2.h"
#include "avalanche/acci2.h"
#include "avalanche/roomnums.h"

#include "common/rect.h"
#include "common/system.h"

#include "graphics/palette.h"

//#include "dropdown.h"
//#include "basher.h"


namespace Avalanche {

Lucerna::Lucerna(AvalancheEngine *vm) : fxhidden(false) {
	_vm = vm;
}

Lucerna::~Lucerna() {
	for (byte i = 0; i < 31; i++)
		for (byte j = 0; j < 2; j++)
			if (_vm->_gyro->also[i][j] != 0)  {
				delete _vm->_gyro->also[i][j];
				_vm->_gyro->also[i][j] = 0;
			}
}

void Lucerna::init() {
	_vm->_gyro->oh = _vm->_gyro->onh = _vm->_gyro->om = 17717;

	for (byte i = 0; i < 31; i++)
		for (byte j = 0; j < 2; j++)
			_vm->_gyro->also[i][j] = 0;

#if 0
	if (_vm->_enhanced->atbios)
		_vm->_gyro->atkey = "f1";
	else
		_vm->_gyro->atkey = "alt-";
#endif
}
	
void Lucerna::callverb(byte n) {
	if (n == _vm->_acci->kPardon) {
		_vm->_scrolls->display(Common::String("The f5 key lets you do a particular action in certain ") +
			"situations. However, at the moment there is nothing " +
			"assigned to it. You may press alt-A to see what the " +
			"current setting of this key is.");
	} else {
		_vm->_gyro->weirdword = false;
		_vm->_acci->_polite = true;
		_vm->_acci->_verb = n;
		_vm->_acci->doThat();
	}
}

void Lucerna::draw_also_lines() {
	byte ff;
	
	CursorMan.showMouse(false);

	_vm->_graphics->_magics.fillRect(Common::Rect(0, 0, 640, 200), 0);
	_vm->_graphics->_magics.frameRect(Common::Rect(0, 45, 640, 161), 15);

	for (ff = 0; ff < _vm->_gyro->lineNum; ff++) {
		// We had to check if the lines are within the borders of the screen.
		if ((_vm->_gyro->lines[ff].x1 >= 0) && (_vm->_gyro->lines[ff].x1 < _vm->_graphics->kScreenWidth) && (_vm->_gyro->lines[ff].y1 >= 0) && (_vm->_gyro->lines[ff].y1 < _vm->_graphics->kScreenHeight)
		 && (_vm->_gyro->lines[ff].x2 >= 0) && (_vm->_gyro->lines[ff].x2 < _vm->_graphics->kScreenWidth) && (_vm->_gyro->lines[ff].y2 >= 0) && (_vm->_gyro->lines[ff].y2 < _vm->_graphics->kScreenHeight))
			_vm->_graphics->_magics.drawLine(_vm->_gyro->lines[ff].x1, _vm->_gyro->lines[ff].y1, _vm->_gyro->lines[ff].x2, _vm->_gyro->lines[ff].y2, _vm->_gyro->lines[ff].col);
	}

	CursorMan.showMouse(true);
}



// nextstring, scram1 and unscrable are only used in load_also 

Common::String Lucerna::nextstring() {
	Common::String str;
	byte length = f.readByte();
	for (int i = 0; i < length; i++)
		str += f.readByte();
	return str;
}

void Lucerna::scram1(Common::String &x) {
	for (uint fz = 0; fz < x.size(); fz++)
		x.setChar(x[fz] ^ 177, fz);
}

void Lucerna::unscramble() {
	for (byte fv = 0; fv < 31; fv++)
		for (byte ff = 0; ff < 2; ff++)
			if (_vm->_gyro->also[fv][ff] != 0)
				scram1(*_vm->_gyro->also[fv][ff]);
	scram1(_vm->_gyro->listen);
	scram1(_vm->_gyro->flags);
}

void Lucerna::load_also(Common::String n) {
	byte ff, fv;
	
	for (fv = 0; fv < 31; fv++)
		for (ff = 0; ff < 2; ff++)
			if (_vm->_gyro->also[fv][ff] != 0)  {
				delete _vm->_gyro->also[fv][ff];
				_vm->_gyro->also[fv][ff] = 0;
			}

	Common::String filename;
	filename = filename.format("also%s.avd", n.c_str());
	if (!f.open(filename)) {
		warning("AVALANCHE: Lucerna: File not found: %s", filename.c_str());
		return;
	}

	f.seek(128);

	byte minnames = f.readByte();
	for (fv = 0; fv <= minnames; fv++) {
		for (ff = 0; ff < 2; ff++) {
			_vm->_gyro->also[fv][ff] = new Common::String;
			*_vm->_gyro->also[fv][ff] = nextstring();
		}
		*_vm->_gyro->also[fv][0] = Common::String('\x9D') + *_vm->_gyro->also[fv][0] + Common::String('\x9D');
	}
	
	memset(_vm->_gyro->lines, 0xFF, sizeof(_vm->_gyro->lines));
	
	//fv = getpixel(0, 0);
	_vm->_gyro->lineNum = f.readByte();
	for (byte i = 0; i < _vm->_gyro->lineNum; i++) {
		_vm->_gyro->lines[i].x1 = f.readSint16LE();
		_vm->_gyro->lines[i].y1 = f.readSint16LE();
		_vm->_gyro->lines[i].x2 = f.readSint16LE();
		_vm->_gyro->lines[i].y2 = f.readSint16LE();
		_vm->_gyro->lines[i].col = f.readByte();
	}

	memset(_vm->_gyro->peds, 177, sizeof(_vm->_gyro->peds));
	fv = f.readByte();
	for (byte i = 0; i < fv; i++) {
		_vm->_gyro->peds[i].x = f.readSint16LE();
		_vm->_gyro->peds[i].y = f.readSint16LE();
		_vm->_gyro->peds[i].dir = f.readByte();
	}

	_vm->_gyro->numfields = f.readByte();
	for (byte i = 0; i < _vm->_gyro->numfields; i++) {
		_vm->_gyro->fields[i].x1 = f.readSint16LE();
		_vm->_gyro->fields[i].y1 = f.readSint16LE();
		_vm->_gyro->fields[i].x2 = f.readSint16LE();
		_vm->_gyro->fields[i].y2 = f.readSint16LE();
	}

	for (byte i = 0; i < 15; i++) {
		_vm->_gyro->magics[i].op = f.readByte();
		_vm->_gyro->magics[i].data = f.readUint16LE();
	}

	for (byte i = 0; i < 7; i++) {
		_vm->_gyro->portals[i].op = f.readByte();
		_vm->_gyro->portals[i].data = f.readUint16LE();
	}

	_vm->_gyro->flags.clear();
	for (byte i = 0;  i < 26; i++)
		_vm->_gyro->flags += f.readByte();

	int16 listen_length = f.readByte();
	_vm->_gyro->listen.clear();
	for (byte i = 0; i < listen_length; i++)
		_vm->_gyro->listen += f.readByte();
	
	draw_also_lines();

	//setactivepage(1);
	warning("STUB: Lucerna::load_also()");

	f.close();
	unscramble();
	for (fv = 0; fv <= minnames; fv++)
		*_vm->_gyro->also[fv][0] = Common::String(',') + *_vm->_gyro->also[fv][0] + ',';
}

void Lucerna::load(byte n) {     // Load2, actually
	Common::String xx;
	
	CursorMan.showMouse(false);
	
	_vm->_graphics->fleshColors();

	xx = _vm->_gyro->strf(n);
	Common::String filename;
	filename = filename.format("place%s.avd", xx.c_str());
	if (!f.open(filename)) {
		warning("AVALANCHE: Lucerna: File not found: %s", filename.c_str());
		return;
	}

	f.seek(146);
	if (!_vm->_gyro->roomname.empty())
		_vm->_gyro->roomname.clear();
	for (byte i = 0; i < 30; i++) {
		char actChar = f.readByte();
		if ((32 <= actChar) && (actChar <= 126))
			_vm->_gyro->roomname += actChar;
	}
	// Compression method byte follows this...

	f.seek(177);

	_vm->_graphics->_background = _vm->_graphics->loadPictureRow(f, _vm->_graphics->kBackgroundWidth, _vm->_graphics->kBackgroundHeight);
	_vm->_graphics->refreshBackground();

	f.close();

	load_also(xx);
	_vm->_celer->loadBackgroundSprites(n);
	// byte bit = *_vm->_graphics->getPixel(0,0);
	CursorMan.showMouse(true);
}



void Lucerna::zoomout(int16 x, int16 y) {
//	int16 x1, y1, x2, y2;
//	byte fv;

	warning("STUB: Lucerna::zoomout()");
}

void Lucerna::find_people(byte room) {
	for (byte fv = 1; fv < 29; fv++)
		if (_vm->_gyro->whereis[fv] == room) {
			if (fv < 25)
				_vm->_gyro->him = fv + 150;
			else
				_vm->_gyro->her = fv + 150;
		}
}

void Lucerna::exitroom(byte x) {
	//nosound();
	_vm->_celer->forgetBackgroundSprites();
	_vm->_gyro->seescroll = true;  // This stops the trippancy system working over the length of this procedure.

	switch (x) {
	case r__spludwicks:
		_vm->_timeout->lose_timer(_vm->_timeout->reason_avariciustalks);
		 _vm->_gyro->dna.avaricius_talk = 0;
		// He doesn't HAVE to be talking for this to work. It just deletes it IF it exists.
		break;
	case r__bridge:
		if (_vm->_gyro->dna.drawbridge_open > 0) {
			_vm->_gyro->dna.drawbridge_open = 4; // Fully open.
			_vm->_timeout->lose_timer(_vm->_timeout->reason_drawbridgefalls);
		}
		break;
	case r__outsidecardiffcastle:
		_vm->_timeout->lose_timer(_vm->_timeout->reason_cardiffsurvey);
		break;
	case r__robins:
		_vm->_timeout->lose_timer(_vm->_timeout->reason_getting_tied_up);
		break;
	}

	_vm->_gyro->interrogation = 0; // Leaving the room cancels all the questions automatically.

	_vm->_gyro->seescroll = false; // Now it can work again!

	_vm->_gyro->dna.last_room = _vm->_gyro->dna.room;
	if (_vm->_gyro->dna.room != r__map)
		_vm->_gyro->dna.last_room_not_map = _vm->_gyro->dna.room;
}

void Lucerna::new_town() {   // You've just entered a town from the map.
	_vm->_dropdown->setupMenu();

	switch (_vm->_gyro->dna.room) {
	case r__outsidenottspub: // Entry into Nottingham.
		if ((_vm->_gyro->dna.rooms[r__robins] > 0) && (_vm->_gyro->dna.been_tied_up) && (!_vm->_gyro->dna.taken_mushroom))
			_vm->_gyro->dna.mushroom_growing = true;
		break;
	case r__wisewomans: { // Entry into Argent.
		if (_vm->_gyro->dna.talked_to_crapulus && (! _vm->_gyro->dna.lustie_is_asleep)) {
			_vm->_gyro->dna.spludwicks_here = !((_vm->_gyro->dna.rooms[r__wisewomans] % 3) == 1);
			_vm->_gyro->dna.crapulus_will_tell = ! _vm->_gyro->dna.spludwicks_here;
		} else {
			_vm->_gyro->dna.spludwicks_here = true;
			_vm->_gyro->dna.crapulus_will_tell = false;
		}
		if (_vm->_gyro->dna.box_contents == _vm->_gyro->wine)
			_vm->_gyro->dna.winestate = 3; // Vinegar
	}
	break;
	}

	if (_vm->_gyro->dna.room != r__outsideducks) {
		if ((_vm->_gyro->dna.obj[_vm->_gyro->onion - 1]) && !(_vm->_gyro->dna.onion_in_vinegar))
			_vm->_gyro->dna.rotten_onion = true; // You're holding the onion
	}
}



void Lucerna::put_geida_at(byte whichped, byte &ped) {
	if (ped == 0)
		return;
	_vm->_trip->tr[1].init(5, false, _vm->_trip); // load Geida
	_vm->_trip->apped(2, whichped);
	_vm->_trip->tr[1].call_eachstep = true;
	_vm->_trip->tr[1].eachstep = _vm->_trip->procgeida_procs;
}

void Lucerna::enterroom(byte x, byte ped) {
	_vm->_gyro->seescroll = true;  // This stops the trippancy system working over the length of this procedure.

	find_people(x);
	_vm->_gyro->dna.room = x;
	if (ped != 0)
		_vm->_gyro->dna.rooms[x]++;

	load(x);

	if ((_vm->_gyro->dna.rooms[x] == 0) && (! _vm->_gyro->flagset('S')))
		points(1);

	_vm->_gyro->whereis[_vm->_gyro->pavalot - 150] = _vm->_gyro->dna.room;

	if (_vm->_gyro->dna.geida_follows)
		_vm->_gyro->whereis[_vm->_gyro->pgeida - 150] = x;

	_vm->_gyro->roomtime = 0;

	
	if ((_vm->_gyro->dna.last_room == r__map) && (_vm->_gyro->dna.last_room_not_map != _vm->_gyro->dna.room))
		new_town();
	

	switch (x) {
	case r__yours:
		if (_vm->_gyro->dna.avvy_in_bed) {
			_vm->_celer->drawBackgroundSprite(-1, -1, 3);
			_vm->_graphics->refreshBackground();
			_vm->_timeout->set_up_timer(100, _vm->_timeout->procarkata_shouts, _vm->_timeout->reason_arkata_shouts);
		}
		break;

	case r__outsideyours:
		if (ped > 0) {
			if (! _vm->_gyro->dna.talked_to_crapulus) {

				_vm->_gyro->whereis[_vm->_gyro->pcrapulus - 150] = r__outsideyours;
				_vm->_trip->tr[1].init(8, false, _vm->_trip); // load Crapulus

				if (_vm->_gyro->dna.rooms[r__outsideyours] == 1) {
					_vm->_trip->apped(2, 4); // Start on the right-hand side of the screen.
					_vm->_trip->tr[1].walkto(5); // Walks up to greet you.
				} else {
					_vm->_trip->apped(2, 5); // Starts where he was before.
					_vm->_trip->tr[1].face = 3;
				}

				_vm->_trip->tr[1].call_eachstep = true;
				_vm->_trip->tr[1].eachstep = _vm->_trip->procface_avvy; // He always faces Avvy.

			} else _vm->_gyro->whereis[_vm->_gyro->pcrapulus - 150] = r__nowhere;

			if (_vm->_gyro->dna.crapulus_will_tell) {
				_vm->_trip->tr[1].init(8, false, _vm->_trip);
				_vm->_trip->apped(2, 2);
				_vm->_trip->tr[1].walkto(4);
				_vm->_timeout->set_up_timer(20, _vm->_timeout->proccrapulus_splud_out, _vm->_timeout->reason_crapulus_says_spludwick_out);
				_vm->_gyro->dna.crapulus_will_tell = false;
			}
		}
		break;

	case r__outsidespludwicks:
		if ((_vm->_gyro->dna.rooms[r__outsidespludwicks] == 1) && (ped == 1)) {
			_vm->_timeout->set_up_timer(20, _vm->_timeout->procbang, _vm->_timeout->reason_explosion);
			_vm->_gyro->dna.spludwicks_here = true;
		}
		break;

	case r__spludwicks:
		if (_vm->_gyro->dna.spludwicks_here) {
			if (ped > 0) {
				_vm->_trip->tr[1].init(2, false, _vm->_trip); // load Spludwick
				_vm->_trip->apped(2, 2);
				_vm->_gyro->whereis[1] = r__spludwicks;
			}

			_vm->_gyro->dna.dogfoodpos = 0;  // _vm->_gyro->also Spludwick pos.

			_vm->_trip->tr[1].call_eachstep = true;
			_vm->_trip->tr[1].eachstep = _vm->_trip->procgeida_procs;
		} else
			_vm->_gyro->whereis[1] = r__nowhere;
		break;

	case r__brummieroad: {
		if (_vm->_gyro->dna.geida_follows)
			put_geida_at(5, ped);
		if (_vm->_gyro->dna.cwytalot_gone) {
			_vm->_gyro->magics[kColorLightred - 1].op = _vm->_gyro->nix;
			_vm->_gyro->whereis[_vm->_gyro->pcwytalot - 150] = r__nowhere;
		} else {
			if (ped > 0) {
				_vm->_trip->tr[1].init(4, false, _vm->_trip); // 4 = Cwytalot
				_vm->_trip->tr[1].call_eachstep = true;
				_vm->_trip->tr[1].eachstep = _vm->_trip->procfollow_avvy_y;
				_vm->_gyro->whereis[_vm->_gyro->pcwytalot - 150] = r__brummieroad;

				if (_vm->_gyro->dna.rooms[r__brummieroad] == 1) { // First time here...
					_vm->_trip->apped(2, 2); // He appears on the right of the screen...
					_vm->_trip->tr[1].walkto(4); // ...and he walks up...
				} else {
					// You've been here before.
					_vm->_trip->apped(2, 4); // He's standing in your way straight away...
					_vm->_trip->tr[1].face = _vm->_trip->left;
				}
			}
		}
	}
	break;

	case r__argentroad: {
		if ((_vm->_gyro->dna.cwytalot_gone) && (! _vm->_gyro->dna.cwytalot_in_herts) && (ped == 2) &&
		        (_vm->_gyro->dna.rooms[r__argentroad] > 3)) {
			_vm->_trip->tr[1].init(4, false, _vm->_trip); // 4 = Cwytalot again
			_vm->_trip->apped(2, 1);
			_vm->_trip->tr[1].walkto(2);
			_vm->_trip->tr[1].vanishifstill = true;
			_vm->_gyro->dna.cwytalot_in_herts = true;
			// _vm->_gyro->whereis[#157] = r__Nowhere; // can we fit this in?
			_vm->_timeout->set_up_timer(20, _vm->_timeout->proc_cwytalot_in_herts, _vm->_timeout->reason_cwytalot_in_herts);
		}
	}
	break;

	case r__bridge: {
		if (_vm->_gyro->dna.drawbridge_open == 4) { // open
			_vm->_celer->drawBackgroundSprite(-1, -1, 3); // Position of drawbridge
			_vm->_graphics->refreshBackground();
			_vm->_gyro->magics[kColorGreen - 1].op = _vm->_gyro->nix; // You may enter the drawbridge.
		}
		if (_vm->_gyro->dna.geida_follows)
			put_geida_at(ped + 3, ped); // load Geida
	}
	break;

	case r__robins: {
		if (ped > 0) {
			if (! _vm->_gyro->dna.been_tied_up) {
				// A welcome party... or maybe not...
				_vm->_trip->tr[1].init(6, false, _vm->_trip);
				_vm->_trip->apped(2, 2);
				_vm->_trip->tr[1].walkto(3);
				_vm->_timeout->set_up_timer(36, _vm->_timeout->procget_tied_up, _vm->_timeout->reason_getting_tied_up);
			}
		}

		if (_vm->_gyro->dna.been_tied_up) {
			_vm->_gyro->whereis[_vm->_gyro->probinhood - 150] = 0;
			_vm->_gyro->whereis[_vm->_gyro->pfriartuck - 150] = 0;
		}

		if (_vm->_gyro->dna.tied_up)
			_vm->_celer->drawBackgroundSprite(-1, -1, 2);

		if (!_vm->_gyro->dna.mushroom_growing) 
			_vm->_celer->drawBackgroundSprite(-1, -1, 3);
		_vm->_graphics->refreshBackground();
	}
	break;

	case r__outsidecardiffcastle: {
		if (ped > 0)
			switch (_vm->_gyro->dna.cardiff_things) {
			case 0 : { // You've answered NONE of his questions.
				_vm->_trip->tr[1].init(9, false, _vm->_trip);
				_vm->_trip->apped(2, 2);
				_vm->_trip->tr[1].walkto(3);
				_vm->_timeout->set_up_timer(47, _vm->_timeout->proccardiffsurvey, _vm->_timeout->reason_cardiffsurvey);
			}
			break;
			case 5 :
				_vm->_gyro->magics[1].op = _vm->_gyro->nix;
				break; // You've answered ALL his questions. => nothing happens.
			default: { // You've answered SOME of his questions.
				_vm->_trip->tr[1].init(9, false, _vm->_trip);
				_vm->_trip->apped(2, 3);
				_vm->_trip->tr[1].face = _vm->_trip->right;
				_vm->_timeout->set_up_timer(3, _vm->_timeout->proccardiff_return, _vm->_timeout->reason_cardiffsurvey);
			}
			}
		if (_vm->_gyro->dna.cardiff_things < 5)
			_vm->_gyro->interrogation = _vm->_gyro->dna.cardiff_things;
		else _vm->_gyro->interrogation = 0;
	}
	break;

	case r__map: {
		// You're entering the map.
		dawn();
		//setactivepage(cp);
		if (ped > 0)
			zoomout(_vm->_gyro->peds[ped - 1].x, _vm->_gyro->peds[ped - 1].y);
		//setactivepage(1 - cp);

		{
			if ((_vm->_gyro->dna.obj[_vm->_gyro->wine - 1]) && (_vm->_gyro->dna.winestate != 3)) {
				_vm->_visa->dixi('q', 9); // Don't want to waste the wine!
				_vm->_gyro->dna.obj[_vm->_gyro->wine - 1] = false;
				objectlist();
			}
		}

		_vm->_visa->dixi('q', 69);
	}
	break;

	case r__catacombs: {
		if ((ped == 0) || (ped == 3) || (ped == 5) || (ped == 6)) {

			switch (ped) {
			case 3: { // Enter from oubliette
				_vm->_gyro->dna.cat_x = 8;
				_vm->_gyro->dna.cat_y = 4;
			}
			break; 
			case 5: { // Enter from du Lustie's
				_vm->_gyro->dna.cat_x = 8;
				_vm->_gyro->dna.cat_y = 7;
			}
			break; 
			case 6: { // Enter from Geida's
				_vm->_gyro->dna.cat_x = 4;
				_vm->_gyro->dna.cat_y = 1;
			}
			break; 
			}

			_vm->_gyro->dna.enter_catacombs_from_lusties_room = true;
			_vm->_trip->catamove(ped);
			_vm->_gyro->dna.enter_catacombs_from_lusties_room = false;
		}
	}
	break;

	case r__argentpub: {
		if (_vm->_gyro->dna.wonnim)
			_vm->_celer->drawBackgroundSprite(-1, -1, 1);   // No lute by the settle.
		_vm->_gyro->dna.malagauche = 0; // Ready to boot Malagauche
		if (_vm->_gyro->dna.givenbadgetoiby) {
			_vm->_celer->drawBackgroundSprite(-1, -1, 8);
			_vm->_celer->drawBackgroundSprite(-1, -1, 9);
		}
		_vm->_graphics->refreshBackground();
	}
	break;

	case r__lustiesroom: {
		_vm->_gyro->dna.dogfoodpos = 1; // Actually, du Lustie pos.
		if (_vm->_trip->tr[0].whichsprite == 0) // Avvy in his normal clothes
			_vm->_timeout->set_up_timer(3, _vm->_timeout->proccallsguards, _vm->_timeout->reason_du_lustie_talks);
		else if (! _vm->_gyro->dna.entered_lusties_room_as_monk) // already
			// Presumably, Avvy dressed as a monk.
			_vm->_timeout->set_up_timer(3, _vm->_timeout->procgreetsmonk, _vm->_timeout->reason_du_lustie_talks);

		if (_vm->_gyro->dna.geida_follows) {
			put_geida_at(5, ped);
			if (_vm->_gyro->dna.lustie_is_asleep) {
				_vm->_celer->drawBackgroundSprite(-1, -1, 5);
				_vm->_graphics->refreshBackground();
			}
		}
	}
	break;

	case r__musicroom: {
		if (_vm->_gyro->dna.jacques_awake > 0) {
			_vm->_gyro->dna.jacques_awake = 5;
			_vm->_celer->drawBackgroundSprite(-1, -1, 2);
			_vm->_graphics->refreshBackground();
			_vm->_celer->drawBackgroundSprite(-1, -1, 4);
			_vm->_gyro->magics[kColorBrown - 1].op = _vm->_gyro->nix;
			_vm->_gyro->whereis[_vm->_gyro->pjacques - 150] = 0;
		}
		if (ped != 0) {
			_vm->_celer->drawBackgroundSprite(-1, -1, 6);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(6);
			_vm->_sequence->then_show(5);
			_vm->_sequence->then_show(7);
			_vm->_sequence->start_to_close();
		}
	}
	break;

	case r__outsidenottspub:
		if (ped == 2) {
			_vm->_celer->drawBackgroundSprite(-1, -1, 3);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(3);
			_vm->_sequence->then_show(2);
			_vm->_sequence->then_show(1);
			_vm->_sequence->then_show(4);
			_vm->_sequence->start_to_close();
		}
		break;

	case r__outsideargentpub:
		if (ped == 2)  {
			_vm->_celer->drawBackgroundSprite(-1, -1, 6);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(6);
			_vm->_sequence->then_show(5);
			_vm->_sequence->then_show(7);
			_vm->_sequence->start_to_close();
		}
		break;

	case r__wisewomans: {
		_vm->_trip->tr[1].init(11, false, _vm->_trip);
		if ((_vm->_gyro->dna.rooms[r__wisewomans] == 1) && (ped > 0)) {
			_vm->_trip->apped(2, 2); // Start on the right-hand side of the screen.
			_vm->_trip->tr[1].walkto(4); // Walks up to greet you.
		} else {
			_vm->_trip->apped(2, 4); // Starts where she was before.
			_vm->_trip->tr[1].face = 3;
		}

		_vm->_trip->tr[1].call_eachstep = true;
		_vm->_trip->tr[1].eachstep = _vm->_trip->procface_avvy; // She always faces Avvy.
	}
	break;

	case r__insidecardiffcastle:
		if (ped > 0) {
			_vm->_trip->tr[1].init(10, false, _vm->_trip); // Define the dart.
			_vm->_celer->drawBackgroundSprite(-1, -1, 1);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(1);
			if (_vm->_gyro->dna.arrow_in_the_door)
				_vm->_sequence->then_show(3);
			else
				_vm->_sequence->then_show(2);

			if (_vm->_gyro->dna.taken_pen)
				_vm->_celer->drawBackgroundSprite(-1, -1, 4);

			_vm->_sequence->start_to_close();
		} else {
			_vm->_celer->drawBackgroundSprite(-1, -1, 1);
			if (_vm->_gyro->dna.arrow_in_the_door)
				_vm->_celer->drawBackgroundSprite(-1, -1, 3);
			else
				_vm->_celer->drawBackgroundSprite(-1, -1, 2);
			_vm->_graphics->refreshBackground();
		}
		break;

	case r__avvysgarden:
		if (ped == 1)  {
			_vm->_celer->drawBackgroundSprite(-1, -1, 2);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(2);
			_vm->_sequence->then_show(1);
			_vm->_sequence->then_show(3);
			_vm->_sequence->start_to_close();
		}
		break;
	
	case r__entrancehall:
	case r__insideabbey:
	case r__yourhall:
		if (ped == 2)  {
#if 0
			// It was the original:
			_vm->_celer->show_one(-1, -1, 2);
			_vm->_sequence->first_show(1);
			_vm->_sequence->then_show(3);
			_vm->_sequence->start_to_close();
#endif

			_vm->_celer->drawBackgroundSprite(-1, -1, 2);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(2);
			_vm->_sequence->then_show(1);
			_vm->_sequence->then_show(3);
			_vm->_sequence->start_to_close();
		}
		break;

	case r__aylesoffice:
		if (_vm->_gyro->dna.ayles_is_awake)
			_vm->_celer->drawBackgroundSprite(-1, -1, 2);
		_vm->_graphics->refreshBackground();
		break; // Ayles awake.

	case r__geidas:
		put_geida_at(2, ped);
		break; // load Geida

	case r__easthall:
	case r__westhall:
		if (_vm->_gyro->dna.geida_follows)
			put_geida_at(ped + 2, ped);
		break;

	case r__lusties:
		if (_vm->_gyro->dna.geida_follows)
			put_geida_at(ped + 6, ped);
		break;

	case r__nottspub: {
		if (_vm->_gyro->dna.sitting_in_pub)  _vm->_celer->drawBackgroundSprite(-1, -1, 3);
		_vm->_gyro->dna.dogfoodpos = 1; // Actually, du Lustie pos.
	}
	break;

	case r__outsideducks:
		if (ped == 2) {
			// Shut the door
			_vm->_celer->drawBackgroundSprite(-1, -1, 3);
			_vm->_graphics->refreshBackground();
			_vm->_sequence->first_show(3);
			_vm->_sequence->first_show(2);
			_vm->_sequence->then_show(1);
			_vm->_sequence->then_show(4);
			_vm->_sequence->start_to_close();
		}
		break;

	case r__ducks:
		_vm->_gyro->dna.dogfoodpos = 1;
		break; // Actually, Duck pos.
	}

	_vm->_gyro->seescroll = false; // Now it can work again!

	if (_vm->_gyro->isLoaded)
		_vm->_gyro->isLoaded = false;
}

void Lucerna::thinkabout(byte z, bool th) {     // Hey!!! Get it and put it!!!
	const int16 picsize = 966;
	const bytefield thinkspace = {25, 170, 32, 200};

	_vm->_gyro->thinks = z;
	z--;

	_vm->_gyro->wait();

	if (th) {
		if (!f.open("thinks.avd")) {
			warning("AVALANCHE: Lucerna: File not found: thinks.avd");
			return;
		}
	} else {
		if (!f.open("folk.avd")) {
			warning("AVALANCHE: Lucerna: File not found: folk.avd");
			return;
		}

		z = z - 149;
		if (z >= 25)
			z -= 8;
		if (z == 20) 
			z--; // Last time...

	}

	f.seek(z * picsize + 65);
	
	::Graphics::Surface picture = _vm->_graphics->loadPictureGraphic(f);

	_vm->_graphics->drawPicture(_vm->_graphics->_surface, picture, 205, 170);

	picture.free();
	
	f.close();

	CursorMan.showMouse(false);

#if 0
	setactivepage(3);
	putimage(x, y, p, 0);
	setactivepage(1 - cp);
#endif

	for (byte fv = 0; fv <= 1; fv++)
		_vm->_trip->getset[fv].remember(thinkspace);
	
	CursorMan.showMouse(true);
	_vm->_gyro->thinkthing = th;
}

void Lucerna::load_digits() {   // Load the scoring digits & rwlites
	const byte digitsize = 134;
	const byte rwlitesize = 126;

	if (!f.open("digit.avd")) {
		warning("AVALANCHE: Lucerna: File not found: digit.avd");
		return;
	}

	for (byte fv = 0; fv < 10; fv++) {
		f.seek(fv * digitsize);
		
#if 0
		_vm->_gyro->digit[fv] = new byte[digitsize];
		f.read(_vm->_gyro->digit[fv], digitsize);
#endif

		_vm->_gyro->digit[fv] = _vm->_graphics->loadPictureGraphic(f);
	}

	for (byte ff = 0; ff < 9; ff++) {
		f.seek(10 * digitsize + ff * rwlitesize);
		
#if 0
		_vm->_gyro->rwlite[ff] = new byte[rwlitesize];
		f.read(_vm->_gyro->rwlite[ff], rwlitesize);
#endif

		_vm->_gyro->rwlite[ff] = _vm->_graphics->loadPictureGraphic(f);
	}

	f.close();
}

void Lucerna::toolbar() {
	if (!f.open("useful.avd")) {
		warning("AVALANCHE: Lucerna: File not found: useful.avd");
		return;
	}

	f.seek(40);
	
	// off;

	::Graphics::Surface picture = _vm->_graphics->loadPictureGraphic(f);

	_vm->_graphics->drawPicture(_vm->_graphics->_surface, picture, 5, 169);

	picture.free();

	f.close();

	// on;

	_vm->_gyro->oldrw = 177;
	showrw();
}

void Lucerna::showscore() {

	const bytefield scorespace = {33, 177, 39, 200};

	if (_vm->_gyro->demo)
		return;

	uint16 score = _vm->_gyro->dna.score;
	int8 numbers[3] = {0, 0, 0};
	for (byte i = 0; i < 2; i++) {
		byte divisor = 1;
		for (byte j = 0; j < (2 - i); j++)
			divisor *= 10;
		numbers[i] = score / divisor;
		score -= numbers[i] * divisor;
	}
	numbers[2] = score;

	CursorMan.showMouse(false);

	//setactivepage(3);

	for (byte fv = 0; fv < 3; fv++)
		if (_vm->_gyro->lastscore[fv] != numbers[fv]) 
			_vm->_graphics->drawPicture(_vm->_graphics->_surface, _vm->_gyro->digit[numbers[fv]], 250 + (fv + 1) * 15, 177);

	for (byte fv = 0; fv < 2; fv++)
		_vm->_trip->getset[fv].remember(scorespace);

	//setactivepage(1 - cp);

	CursorMan.showMouse(true);
	for (byte i = 0; i < 3; i++)
		_vm->_gyro->lastscore[i] = numbers[i];
}

void Lucerna::points(byte num) {     // Add on no. of points
	for (byte q = 1; q <= num; q++) {
		_vm->_gyro->dna.score++;

#if 0
		if (soundfx) {
			for (byte fv = 1; fv <= 97; fv++)
				sound(177 + dna.score * 3);
		}
		nosound;
#endif
	}
	warning("STUB: Lucerna::points()");

	showscore();
}

void Lucerna::topcheck(Common::Point cursorPos) {
	_vm->_dropdown->_menuBar.chooseMenuItem(cursorPos.x);
}

void Lucerna::mouseway(const Common::Point &cursorPos) {
	byte col = *(byte *)_vm->_graphics->_surface.getBasePtr(cursorPos.x, cursorPos.y / 2);

	switch (col) {
	case kColorGreen: {
			_vm->_gyro->dna.rw = _vm->_trip->up;
			_vm->_trip->rwsp(0, _vm->_trip->up);
			showrw();
		}
		break;
	case kColorBrown: {
			_vm->_gyro->dna.rw = _vm->_trip->down;
			_vm->_trip->rwsp(0, _vm->_trip->down);
			showrw();
		}
		break;
	case kColorCyan: {
			_vm->_gyro->dna.rw = _vm->_trip->left;
			_vm->_trip->rwsp(0, _vm->_trip->left);
			showrw();
		}
		break;
	case kColorLightmagenta: {
			_vm->_gyro->dna.rw = _vm->_trip->right;
			_vm->_trip->rwsp(0, _vm->_trip->right);
			showrw();
		}
		break;
	case kColorRed:
	case kColorWhite:
	case kColorLightcyan:
	case kColorYellow: { // Fall-throughs are intended.
			_vm->_trip->stopwalking();
			showrw();
		}
		break;
	}
}

void Lucerna::posxy() {
	warning("STUB: Lucerna::posxy()");
}

void Lucerna::fxtoggle() {
	warning("STUB: Lucerna::fxtoggle()");
}

void Lucerna::objectlist() {
	_vm->_gyro->dna.carrying = 0;
	if (_vm->_gyro->thinkthing && !_vm->_gyro->dna.obj[_vm->_gyro->thinks - 1])
		thinkabout(_vm->_gyro->money, _vm->_gyro->a_thing); // you always have money
	for (byte fv = 0; fv < numobjs; fv++)
		if (_vm->_gyro->dna.obj[fv]) {
			_vm->_gyro->dna.carrying++;
			_vm->_gyro->objlist[_vm->_gyro->dna.carrying] = fv + 1;
		}
}

void Lucerna::verte(Common::Point cursorPos) {
	if (! _vm->_gyro->dna.user_moves_avvy) 
		return;

	cursorPos.y /= 2;
	byte what;

	// _vm->_trip->tr[0] : that's the only one we're interested in here. (It's Avalot.)
	if (cursorPos.x < _vm->_trip->tr[0].x) 
		what = 1;
	else if (cursorPos.x > (_vm->_trip->tr[0].x + _vm->_trip->tr[0]._info._xLength)) 
		what = 2;
	else
		what = 0; // On top

	if (cursorPos.y < _vm->_trip->tr[0].y)
		what += 3;
	else if (cursorPos.y > (_vm->_trip->tr[0].y + _vm->_trip->tr[0]._info._yLength))
		what += 6;

	switch (what) {
	case 0:
		_vm->_trip->stopwalking();
		break; // Clicked on Avvy: no movement.
	case 1:
		_vm->_trip->rwsp(0, _vm->_trip->left);
		break;
	case 2:
		_vm->_trip->rwsp(0, _vm->_trip->right);
		break;
	case 3:
		_vm->_trip->rwsp(0, _vm->_trip->up);
		break;
	case 4:
		_vm->_trip->rwsp(0, _vm->_trip->ul);
		break;
	case 5:
		_vm->_trip->rwsp(0, _vm->_trip->ur);
		break;
	case 6:
		_vm->_trip->rwsp(0, _vm->_trip->down);
		break;
	case 7:
		_vm->_trip->rwsp(0, _vm->_trip->dl);
		break;
	case 8:
		_vm->_trip->rwsp(0, _vm->_trip->dr);
		break;
	}    // No other values are possible.

	showrw();
}

void Lucerna::checkclick() {
	Common::Point cursorPos = _vm->getMousePos();
	_vm->_gyro->ontoolbar = _vm->_gyro->slow_computer && ((cursorPos.y >= 169) || (cursorPos.y <= 10));
	
	/*if (_vm->_gyro->mrelease > 0)
	_vm->_gyro->after_the_scroll = false;*/

	if ((0 <= cursorPos.y) && (cursorPos.y <= 21))
		_vm->_gyro->newpointer(1); // up arrow
	else if ((317 <= cursorPos.y) && (cursorPos.y <= 339))
		_vm->_gyro->newpointer(8); //I-beam
	else if ((340 <= cursorPos.y) && (cursorPos.y <= 399))
		_vm->_gyro->newpointer(2); // screwdriver
	else if (!_vm->_gyro->ddmnow) { // Dropdown can handle its own pointers.
		if (holdLeftMouse) {
			_vm->_gyro->newpointer(7); // Mark's crosshairs
			verte(cursorPos); // Normally, if you click on the picture, you're guiding Avvy around.
		} else
			_vm->_gyro->newpointer(4); // fletch
	}
	
	if (holdLeftMouse) {
		if ((0 <= cursorPos.y) && (cursorPos.y <= 21)) { // Click on the dropdown menu.
			if (_vm->_gyro->dropsok)
				_vm->_dropdown->updateMenu();
		} else if ((317 <= cursorPos.y) && (cursorPos.y <= 339)) { // Click on the command line.
			_vm->_parser->_inputTextPos = (cursorPos.x - 23) / 8;
			if (_vm->_parser->_inputTextPos > _vm->_parser->_inputText.size() + 1)
				_vm->_parser->_inputTextPos = _vm->_parser->_inputText.size() + 1;
			if (_vm->_parser->_inputTextPos < 1)
				_vm->_parser->_inputTextPos = 1;
			_vm->_parser->_inputTextPos--;
			_vm->_parser->plotText();
		} else if ((340 <= cursorPos.y) && (cursorPos.y <= 399)) { // Check the toolbar.
			if ((137 <= cursorPos.x) && (cursorPos.x <= 207)) { // Control Avvy with the compass.
				if (_vm->_gyro->alive && _vm->_gyro->dna.avvy_is_awake)
					mouseway(cursorPos);
			} else if ((208 <= cursorPos.x) && (cursorPos.x <= 260)) { // Examine the _thing.
				do {
					_vm->updateEvents();
				} while (holdLeftMouse);

				if (_vm->_gyro->thinkthing) {
					_vm->_acci->_thing = _vm->_gyro->thinks;
					_vm->_acci->_thing += 49;
					_vm->_acci->_person = _vm->_acci->kPardon;
				} else {
					_vm->_acci->_person = _vm->_gyro->thinks;
					_vm->_acci->_thing = _vm->_acci->kPardon;
				}
				callverb(_vm->_acci->kVerbCodeExam);
			} else if ((261 <= cursorPos.x) && (cursorPos.x <= 319)) { // Display the score.
				do {
					_vm->updateEvents();
				} while (holdLeftMouse);

				callverb(_vm->_acci->kVerbCodeScore);
			} else if ((320 <= cursorPos.x) && (cursorPos.x <= 357)) { // Change speed.
				_vm->_trip->tr[0].xs = _vm->_gyro->walk;
				_vm->_trip->newspeed();
			} else if ((358 <= cursorPos.x) && (cursorPos.x <= 395)) { // Change speed.
				_vm->_trip->tr[0].xs = _vm->_gyro->run;
				_vm->_trip->newspeed();
			} else if ((396 <= cursorPos.x) && (cursorPos.x <= 483))
				fxtoggle();
			else if ((535 <= cursorPos.x) && (cursorPos.x <= 640))
				_vm->_gyro->mousetext = Common::String(13) + _vm->_gyro->mousetext;
		} else if (!_vm->_gyro->dropsok)
			_vm->_gyro->mousetext = Common::String(13) + _vm->_gyro->mousetext;
	}
}

void Lucerna::mouse_init() {
	_vm->_gyro->wait();
}

void Lucerna::mousepage(uint16 page_) {
	warning("STUB: Lucerna::mousepage()");
}

void Lucerna::errorled() {
	warning("STUB: Lucerna::errorled()");
}

int8 Lucerna::fades(int8 x) {
	byte r = x / 16;
	x = x % 16;
	byte g = x / 4;
	byte b = x % 4;
	if (r > 0)
		r--;
	if (g > 0)
		g--;
	if (b > 0)
		b--;

	return (16 * r + 4 * g + b);
	// fades = x - 1;
}




void Lucerna::fadeout(byte n) {
	warning("STUB: Lucerna::fadeout()"); // I'll bother with colors later.
}

void Lucerna::dusk() {
	warning("STUB: Lucerna::dusk()"); // I'll bother with colors later.
}




void Lucerna::fadein(byte n) {
	warning("STUB: Lucerna::fadein()");
}

void Lucerna::dawn() {
	warning("STUB: Lucerna::dawn()");
}



void Lucerna::showrw() { // It's data is loaded in load_digits().
	if (_vm->_gyro->oldrw == _vm->_gyro->dna.rw) 
		return;
	_vm->_gyro->oldrw = _vm->_gyro->dna.rw;
	CursorMan.showMouse(false);
	
#if 0
	for (byte page_ = 0; page_ <= 1; page_++) {
		setactivepage(page_);
		putimage(0, 161, rwlite[with.rw], 0);
	}
#endif

	_vm->_graphics->drawPicture(_vm->_graphics->_surface, _vm->_gyro->rwlite[_vm->_gyro->dna.rw], 0, 161);

	CursorMan.showMouse(true);
	//setactivepage(1 - cp);
	
	warning("STUB: Lucerna::showrw()");
}






void Lucerna::calchand(uint16 ang, uint16 length, Common::Point &a, byte c) {
	if (ang > 900) {
		a.x = 177;
		return;
	}

	a = _vm->_graphics->drawArc(_vm->_graphics->_surface, xm, ym, 449 - ang, 450 - ang, length, c);
}

void Lucerna::hand(const Common::Point &a, byte c) {
	if (a.x == 177)
		return;

	_vm->_graphics->_surface.drawLine(xm, ym, a.x, a.y, c);
}

void Lucerna::refresh_hands() {
	const bytefield clockspace = {61, 166, 66, 200};

	for (byte page_ = 0; page_ < 2; page_++)
		_vm->_trip->getset[page_].remember(clockspace);
}

void Lucerna::plothands() {
	//   off;
	//setactivepage(3);
	calchand(_vm->_gyro->onh, 14, ah, kColorYellow);
	calchand(_vm->_gyro->om * 6, 17, am, kColorYellow);
	hand(ah, kColorBrown);
	hand(am, kColorBrown);

	calchand(nh, 14, ah, kColorBrown);
	calchand(_vm->_gyro->minutes * 6, 17, am, kColorBrown);
	hand(ah, kColorYellow);
	hand(am, kColorYellow);

	//setactivepage(1 - cp);

	refresh_hands();

	//   on;
}

void Lucerna::chime() {
	warning("STUB: Lucerna::chime()");
}

void Lucerna::clock_lucerna() {
	// ...Clock.
	TimeDate t;
	_vm->_system->getTimeAndDate(t);
	_vm->_gyro->hour = t.tm_hour;
	_vm->_gyro->minutes = t.tm_min;
	_vm->_gyro->seconds = t.tm_sec;

	nh = (_vm->_gyro->hour % 12) * 30 + _vm->_gyro->minutes / 2;

	if (_vm->_gyro->oh != _vm->_gyro->hour)  {
		plothands();
		chime();
	}

	if (_vm->_gyro->om != _vm->_gyro->minutes)
		plothands();

	if ((_vm->_gyro->hour == 0) && (_vm->_gyro->oh != 0) && (_vm->_gyro->oh != 17717))
		_vm->_scrolls->display(Common::String("Good morning!") + 13 + 13 + "Yes, it's just past midnight. Are you having an all-night Avvy session? Glad you like the game that much!");
	
	_vm->_gyro->oh = _vm->_gyro->hour;
	_vm->_gyro->onh = nh;
	_vm->_gyro->om = _vm->_gyro->minutes;
}



void Lucerna::delavvy() {
	CursorMan.showMouse(false);
	
//	triptype &with = _vm->_trip->tr[0];
//	for (byte page_ = 0; page_ <= 1; page_++)
//		mblit(with.x / 8, with.y, (with.x + with._info.xl) / 8 + 1, with.y + with._info.yl, 3, page_);
	
	CursorMan.showMouse(true);
}

void Lucerna::gameover() {
	_vm->_gyro->dna.user_moves_avvy = false;

	int16 sx = _vm->_trip->tr[0].x;
	int16 sy = _vm->_trip->tr[0].y;
	
	_vm->_trip->tr[0].done();
	_vm->_trip->tr[0].init(12, true, _vm->_trip); // 12 = Avalot falls
	_vm->_trip->tr[0].step = 0;
	_vm->_trip->tr[0].appear(sx, sy, 0);
	
	_vm->_timeout->set_up_timer(3, _vm->_timeout->procavalot_falls, _vm->_timeout->reason_falling_over);

	_vm->_gyro->alive = false;
}

/* OK. There are two kinds of redraw: Major and Minor. Minor is what happens
  when you load a game, etc. Major redraws EVERYTHING. */

void Lucerna::minor_redraw() {
	dusk();

	enterroom(_vm->_gyro->dna.room, 0); // Ped unknown or non-existant.

	for (byte fv = 0; fv <= 1; fv++) {
		_vm->_gyro->cp = 1 - _vm->_gyro->cp;
		_vm->_trip->getback();
	}
	
	for (byte i = 0; i < 3; i++)
		_vm->_gyro->lastscore[i] = -1; // impossible digits
	showscore();
	
	dawn();
}

void Lucerna::major_redraw() {
	warning("STUB: Lucerna::major_redraw()");
}

uint16 Lucerna::bearing(byte whichped) {
	whichped--; // Different array indexes in Pascal and C.

	const double rad2deg = 180 / 3.14; // Pi
		
	if (_vm->_trip->tr[0].x == _vm->_gyro->peds[whichped].x)
		return 0;
	else {
		if (_vm->_trip->tr[0].x < _vm->_gyro->peds[whichped].x) {
			return (atan(double((_vm->_trip->tr[0].y - _vm->_gyro->peds[whichped].y))
					/ (_vm->_trip->tr[0].x - _vm->_gyro->peds[whichped].x)) * rad2deg) + 90;
		} else {
			return (atan(double((_vm->_trip->tr[0].y - _vm->_gyro->peds[whichped].y))
					/ (_vm->_trip->tr[0].x - _vm->_gyro->peds[whichped].x)) * rad2deg) + 270;
		}
	}
}

void Lucerna::sprite_run() {
/* A sprite run is performed before displaying a scroll, if not all the
  sprites are still. It performs two fast cycles, only using a few of
  the links usually used, and without any extra animation. This should
  make the sprites the same on both pages. */
	_vm->_gyro->doing_sprite_run = true;

	_vm->_trip->get_back_loretta();
	_vm->_trip->trippancy_link();

	_vm->_gyro->doing_sprite_run = false;

}

void Lucerna::fix_flashers() {
	_vm->_gyro->ledstatus = 177;
	_vm->_gyro->oldrw = 177;
	_vm->_scrolls->state(2);
	showrw();
}

} // End of namespace Avalanche
