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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/system.h"

#include "common/config-manager.h"

#include "parallaction/parallaction.h"
#include "parallaction/sound.h"
#include "parallaction/menu.h"


namespace Parallaction {

#define MOUSEARROW_WIDTH		16
#define MOUSEARROW_HEIGHT		16

#define MOUSECOMBO_WIDTH		32	// sizes for cursor + selected inventory item
#define MOUSECOMBO_HEIGHT		32

int Parallaction_ns::init() {

	// Detect game
	if (!detectGame()) {
		GUIErrorMessage("No valid games were found in the specified directory.");
		return -1;
	}

	_screenWidth = 320;
	_screenHeight = 200;

	if (getPlatform() == Common::kPlatformPC) {
		_disk = new DosDisk_ns(this);
	} else {
		if (getFeatures() & GF_DEMO) {
			strcpy(_location._name, "fognedemo");
		}
		_disk = new AmigaDisk_ns(this);
		_disk->selectArchive((getFeatures() & GF_DEMO) ? "disk0" : "disk1");
	}

	if (getPlatform() == Common::kPlatformPC) {
		int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
		MidiDriver *driver = MidiDriver::createMidi(midiDriver);
		_soundMan = new DosSoundMan(this, driver);
		_soundMan->setMusicVolume(ConfMan.getInt("music_volume"));
	} else {
		_soundMan = new AmigaSoundMan(this);
	}

	initResources();
	initFonts();
	initCursors();

	Parallaction::init();

	return 0;
}

Parallaction_ns::~Parallaction_ns() {
	freeFonts();

	_mouseComposedArrow->free();
	delete _mouseComposedArrow;
}


void Parallaction_ns::freeFonts() {

	delete _dialogueFont;
	delete _labelFont;
	delete _menuFont;

	return;
}

void Parallaction_ns::renderLabel(Graphics::Surface *cnv, char *text) {

	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		cnv->create(_labelFont->getStringWidth(text) + 16, 10, 1);

		_labelFont->setColor(7);
		_labelFont->drawString((byte*)cnv->pixels + 1, cnv->w, text);
		_labelFont->drawString((byte*)cnv->pixels + 1 + cnv->w * 2, cnv->w, text);
		_labelFont->drawString((byte*)cnv->pixels + cnv->w, cnv->w, text);
		_labelFont->drawString((byte*)cnv->pixels + 2 + cnv->w, cnv->w, text);
		_labelFont->setColor(1);
		_labelFont->drawString((byte*)cnv->pixels + 1 + cnv->w, cnv->w, text);
	} else {
		cnv->create(_labelFont->getStringWidth(text), _labelFont->height(), 1);
		_labelFont->drawString((byte*)cnv->pixels, cnv->w, text);
	}

}

void Parallaction_ns::initCursors() {

	_mouseComposedArrow = _disk->loadPointer("pointer");

	byte temp[MOUSEARROW_WIDTH*MOUSEARROW_HEIGHT];
	memcpy(temp, _mouseArrow, MOUSEARROW_WIDTH*MOUSEARROW_HEIGHT);

	uint16 k = 0;
	for (uint16 i = 0; i < 4; i++) {
		for (uint16 j = 0; j < 64; j++) _mouseArrow[k++] = temp[i + j * 4];
	}

	return;
}

void Parallaction_ns::setMousePointer(int16 index) {

	if (index == kCursorArrow) {		// standard mouse pointer

		_system->setMouseCursor(_mouseArrow, MOUSEARROW_WIDTH, MOUSEARROW_HEIGHT, 0, 0, 0);
		_system->showMouse(true);

	} else {
		// inventory item pointer
		byte *v8 = (byte*)_mouseComposedArrow->pixels;

		// FIXME: destination offseting is not clear
		byte* s = _char._objs->getFramePtr(getInventoryItemIndex(index));
		byte* d = v8 + 7 + MOUSECOMBO_WIDTH * 7;

		for (uint i = 0; i < INVENTORYITEM_HEIGHT; i++) {
			memcpy(d, s, INVENTORYITEM_WIDTH);

			s += INVENTORYITEM_PITCH;
			d += MOUSECOMBO_WIDTH;
		}

		_system->setMouseCursor(v8, MOUSECOMBO_WIDTH, MOUSECOMBO_HEIGHT, 0, 0, 0);
	}

	return;
}

void Parallaction_ns::callFunction(uint index, void* parm) {
	assert(index < 25);	// magic value 25 is maximum # of callables for Nippon Safes

	(this->*_callables[index])(parm);
}


int Parallaction_ns::go() {

	_menu = new Menu(this);
	_menu->start();

	char *v4 = strchr(_location._name, '.');
	if (v4) {
		*v4 = '\0';
	}

	_globalTable = _disk->loadTable("global");

	_engineFlags &= ~kEngineChangeLocation;
	changeCharacter(_characterName);

	strcpy(_saveData1, _location._name);
	parseLocation(_location._name);

	if (_location._startPosition.x != -1000) {
		_char._ani._left = _location._startPosition.x;
		_char._ani._top = _location._startPosition.y;
		_char._ani._frame = _location._startFrame;
		_location._startPosition.y = -1000;
		_location._startPosition.x = -1000;
	};

	runGame();

	delete _menu;

	return 0;
}



} // namespace Parallaction
