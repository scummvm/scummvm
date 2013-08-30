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

/* CELER	The unit for updating the screen pics. */

#include "avalanche/avalanche.h"

#include "avalanche/celer2.h"
#include "avalanche/trip6.h"
#include "avalanche/lucerna2.h"
#include "avalanche/gyro2.h"
#include "avalanche/roomnums.h"

#include "common/textconsole.h"

namespace Avalanche {

const int16 Celer::on_disk = -1;


Celer::Celer(AvalancheEngine *vm) {
	_vm = vm;
	num_chunks = 0;
}

Celer::~Celer() {
	for (byte i = 0; i < 40; i++)
		memory[i].free();
}

void Celer::pics_link() {
	byte xx;

	if (_vm->_gyro->ddmnow)
		return; /* No animation when the menus are up. */

	
	switch (_vm->_gyro->dna.room) {
	case r__outsideargentpub:
		if ((_vm->_gyro->roomtime % 12) == 0)
			show_one(-1, -1, 1 + (_vm->_gyro->roomtime / 12) % 4);
		break;

	case r__brummieroad:
		if ((_vm->_gyro->roomtime % 2) == 0)
			show_one(-1, -1, 1 + (_vm->_gyro->roomtime / 2) % 4);
		break;

	case r__bridge:
		if ((_vm->_gyro->roomtime % 2) == 0)
			show_one(-1, -1, 4 + (_vm->_gyro->roomtime / 2) % 4);
		break;

	case r__yours:
		if ((!_vm->_gyro->dna.avvy_is_awake) && ((_vm->_gyro->roomtime % 4) == 0))
			show_one(-1, -1, 1 + (_vm->_gyro->roomtime / 12) % 2);
		break;

	case r__argentpub:
		if (((_vm->_gyro->roomtime % 7) == 1) && (_vm->_gyro->dna.malagauche != 177)) {
			/* Malagauche cycle */
			_vm->_gyro->dna.malagauche += 1;
			switch (_vm->_gyro->dna.malagauche) {
			case 1:
			case 11:
			case 21:
				show_one(-1, -1, 12);
				break; /* Looks forwards. */
			case 8:
			case 18:
			case 28:
			case 32:
				show_one(-1, -1, 11);
				break; /* Looks at you. */
			case 30:
				show_one(-1, -1, 13);
				break; /* Winks. */
			case 33:
				_vm->_gyro->dna.malagauche = 0;
				break;
			}
		}

		switch (_vm->_gyro->roomtime % 200) {
		case 179:
		case 197:
			show_one(-1, -1, 5);
			break; /* Dogfood's drinking cycle */
		case 182:
		case 194:
			show_one(-1, -1, 6);
			break;
		case 185:
			show_one(-1, -1, 7);
			break;
		case 199:
			_vm->_gyro->dna.dogfoodpos = 177;
			break; /* Impossible value for this. */
		}

		if ((_vm->_gyro->roomtime % 200 >= 0) && (_vm->_gyro->roomtime % 200 <= 178)) { /* Normally. */
			if (((_vm->_lucerna->bearing(2) >= 1) && (_vm->_lucerna->bearing(2) <= 90)) || ((_vm->_lucerna->bearing(2) >= 358) && (_vm->_lucerna->bearing(2) <= 360)))
				xx = 3;
			else if ((_vm->_lucerna->bearing(2) >= 293) && (_vm->_lucerna->bearing(2) <= 357))
				xx = 2;
			else if ((_vm->_lucerna->bearing(2) >= 271) && (_vm->_lucerna->bearing(2) <= 292))
				xx = 4;

			if (xx != _vm->_gyro->dna.dogfoodpos) { /* Only if it's changed.*/
				show_one(-1, -1, xx);
				_vm->_gyro->dna.dogfoodpos = xx;
			}
		}
		break;

	case r__westhall:
		if ((_vm->_gyro->roomtime % 3) == 0) {
			switch ((_vm->_gyro->roomtime / int32(3)) % int32(6)) {
			case 4:
				show_one(-1, -1, 1);
				break;
			case 1:
			case 3:
			case 5:
				show_one(-1, -1, 2);
				break;
			case 0:
			case 2:
				show_one(-1, -1, 3);
				break;
			}
		}
		break;

	case r__lustiesroom:
		if (!(_vm->_gyro->dna.lustie_is_asleep)) {
			if ((_vm->_gyro->roomtime % 45) > 42)
				xx = 4; /* du Lustie blinks */

			/* Bearing of Avvy from du Lustie. */
			else if (((_vm->_lucerna->bearing(2) >= 0) && (_vm->_lucerna->bearing(2) <= 45)) || ((_vm->_lucerna->bearing(2) >= 315) && (_vm->_lucerna->bearing(2) <= 360)))
					xx = 1; /* Middle. */
			else if ((_vm->_lucerna->bearing(2) >= 45) && (_vm->_lucerna->bearing(2) <= 180))
					xx = 2; /* Left. */
			else if ((_vm->_lucerna->bearing(2) >= 181) && (_vm->_lucerna->bearing(2) <= 314))
				xx = 3; /* Right. */

			if (xx != _vm->_gyro->dna.dogfoodpos) { /* Only if it's changed.*/
				show_one(-1, -1, xx);
				_vm->_gyro->dna.dogfoodpos = xx; /* We use DogfoodPos here too- why not? */
			}
		}
		break;

	case r__aylesoffice:
		if ((!_vm->_gyro->dna.ayles_is_awake) && (_vm->_gyro->roomtime % 14 == 0)) {
			switch ((_vm->_gyro->roomtime / 14) % 2) {
			case 0:
				show_one(-1, -1, 1);
				break; /* Frame 2: EGA. */
			case 1:
				show_one(-1, -1, 3);
				break; /* Frame 1: Natural. */
			}
		}
		break;

	case r__robins:
		if (_vm->_gyro->dna.tied_up) {
			switch (_vm->_gyro->roomtime % 54) {
			case 20:
				show_one(-1, -1, 4);
				break; /* Frame 4: Avalot blinks. */
			case 23:
				show_one(-1, -1, 2);
				break; /* Frame 1: Back to normal. */
			}
		}
		break;

	case r__nottspub:
		/* Bearing of Avvy from Port. */
		if (((_vm->_lucerna->bearing(5) >= 0) && (_vm->_lucerna->bearing(5) <= 45)) || ((_vm->_lucerna->bearing(5) >= 315) && (_vm->_lucerna->bearing(5) <= 360)))
			xx = 2; /* Middle. */
		else if ((_vm->_lucerna->bearing(5) >= 45) && (_vm->_lucerna->bearing(5) <= 180))
			xx = 6; /* Left. */
		else if ((_vm->_lucerna->bearing(5) >= 181) && (_vm->_lucerna->bearing(5) <= 314))
			xx = 8; /* Right. */

		if ((_vm->_gyro->roomtime % int32(60)) > 57)
			xx--; /* Blinks */

		if (xx != _vm->_gyro->dna.dogfoodpos) { /* Only if it's changed.*/
			show_one(-1, -1, xx);
			_vm->_gyro->dna.dogfoodpos = xx; /* We use DogfoodPos here too- why not? */
		}

		switch (_vm->_gyro->roomtime % 50) {
		case 45 :
			show_one(-1, -1, 9);
			break; /* Spurge blinks */
		case 49 :
			show_one(-1, -1, 10);
			break;
		}
		break;

	case r__ducks:
		if ((_vm->_gyro->roomtime % 3) == 0) /* The fire flickers */
			show_one(-1, -1, 1 + (_vm->_gyro->roomtime / 3) % 3);

		{/* _vm->_lucerna->bearing of Avvy from Duck. */
		if (((_vm->_lucerna->bearing(2) >= 0) && (_vm->_lucerna->bearing(2) <= 45)) || ((_vm->_lucerna->bearing(2) >= 315) && (_vm->_lucerna->bearing(2) <= 360)))
			xx = 4; /* Middle. */
		else if ((_vm->_lucerna->bearing(2) >= 45) && (_vm->_lucerna->bearing(2) <= 180))
			xx = 6; /* Left. */
		else if ((_vm->_lucerna->bearing(2) >= 181) && (_vm->_lucerna->bearing(2) <= 314))
			xx = 8; /* Right. */

		if ((_vm->_gyro->roomtime % 45) > 42)
			xx += 1; /* Duck blinks */

		if (xx != _vm->_gyro->dna.dogfoodpos) { /* Only if it's changed.*/
			show_one(-1, -1, xx);
			_vm->_gyro->dna.dogfoodpos = xx; /* We use DogfoodPos here too- why not? */
		}
		break;

	}
}

	if ((_vm->_gyro->dna.ringing_bells) && (_vm->_gyro->flagset('B'))) {
		/* They're ringing the bells. */
		switch (_vm->_gyro->roomtime % 4) {
		case 1:
			if (_vm->_gyro->dna.nextbell < 5)
				_vm->_gyro->dna.nextbell = 12;
			_vm->_gyro->dna.nextbell -= 1;
			_vm->_gyro->note(_vm->_gyro->notes[_vm->_gyro->dna.nextbell]);
			break;
		case 2:
			//nosound;
			warning("STUB: Celer::pics_link()");
			break;
		}
	}
}

void Celer::load_chunks(Common::String xx) {
	chunkblocktype ch;
	byte fv;
	
	filename = filename.format("chunk%s.avd", xx.c_str());
	if (!f.open(filename)) {
		warning("AVALANCHE: Celer: File not found: %s", filename.c_str());
		return;
	}

	f.seek(44);
	num_chunks = f.readByte();
	for (byte i = 0; i < num_chunks; i++)
		offsets[i] = f.readSint32LE();

	for (fv = 0; fv < num_chunks; fv++) {
		f.seek(offsets[fv]);
		
		ch.flavour = flavourtype(f.readByte());
		ch.x = f.readSint16LE();
		ch.y = f.readSint16LE();
		ch.xl = f.readSint16LE();
		ch.yl = f.readSint16LE();
		ch.size = f.readSint32LE();
		ch.natural = f.readByte();
		ch.memorise = f.readByte();
				
		if (ch.memorise) {
			memos[fv].x = ch.x;
			memos[fv].xl = ch.xl;
			memos[fv].y = ch.y;
			memos[fv].yl = ch.yl;
			memos[fv].flavour = ch.flavour;

			if (ch.natural) {
				memos[fv].flavour = ch_natural_image; // We simply read from the screen and later, in display_it() we draw it right back.
				memos[fv].size = memos[fv].xl * 8 * memos[fv].yl + 1; 
				memory[fv].create(memos[fv].xl * 8, memos[fv].yl + 1, ::Graphics::PixelFormat::createFormatCLUT8());

				for (uint16 j = 0; j < memos[fv].yl + 1; j++)
					for (uint16 i = 0; i < memos[fv].xl * 8; i++)
						*(byte *)memory[fv].getBasePtr(i, j) = *_vm->_graphics->getPixel(memos[fv].x * 8 + i, memos[fv].y + j);
			} else {
				memos[fv].size = ch.size;
				memory[fv] = _vm->_graphics->loadPictureRow(f, memos[fv].xl * 8, memos[fv].yl + 1); // Celer::forget_chunks() deallocates it.
			}
		} else
			memos[fv].x = on_disk;
	}
	f.close();
}

void Celer::forget_chunks() {
	for (byte fv = 0; fv < num_chunks; fv ++)
		if (memos[fv].x > on_disk)
			memory[fv].free();

	memset(memos, 255, sizeof(memos)); /* x=-1, => on disk. */
}

void Celer::display_it(int16 x, int16 y, int16 xl, int16 yl, flavourtype flavour, const ::Graphics::Surface &picture) {
	r.x1 = x;
	r.y1 = y;
	r.y2 = y + yl;

	switch (flavour) {
	case ch_natural_image: // Allow fallthorugh on purpose.
	case ch_bgi : {
		r.x2 = x + xl + 1;
		}
		break;
	case ch_ega : {
		r.x2 = x + xl;
		}
		break;
	}

	// These pictures are practically parts of the background. -10 is for the drop-down menu.
	_vm->_graphics->drawPicture(_vm->_graphics->_background, picture, x, y - 10);
}

void Celer::show_one(int16 destX, int16 destY, byte which) {
	chunkblocktype ch;
	which--; // For the difference between the Pascal and C array indexes.
	//setactivepage(3);
	warning("STUB: Celer::show_one()");

	if (memos[which].x > on_disk) {
		if (destX == -1) {
			destX = memos[which].x * 8;
			destY = memos[which].y;
		}
		display_it(destX, destY, memos[which].xl, memos[which].yl, memos[which].flavour, memory[which]);
	} else {
		if (!f.open(filename)) { /* Filename was set in load_chunks() */
			warning("AVALANCHE: Celer: File not found: %s", filename.c_str());
			return;
		}

		f.seek(offsets[which]);

		ch.flavour = flavourtype(f.readByte());
		ch.x = f.readSint16LE();
		ch.y = f.readSint16LE();
		ch.xl = f.readSint16LE();
		ch.yl = f.readSint16LE();
		ch.size = f.readSint32LE();
		ch.natural = f.readByte();
		ch.memorise = f.readByte();

		::Graphics::Surface picture = _vm->_graphics->loadPictureRow(f, ch.xl * 8, ch.yl + 1);

		if (destX == -1) {
			destX = ch.x * 8;
			destY = ch.y;
		}
		display_it(destX, destY, ch.xl, ch.yl, ch.flavour, picture);

		picture.free();
		f.close();
	}

	//setactivepage(1 - cp);
	warning("STUB: Celer::show_one()");

	for (byte fv = 0; fv < 2; fv ++)
		_vm->_trip->getset[fv].remember(r);
}



} // End of namespace Avalanche.
