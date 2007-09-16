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

	initJobs();
	initResources();
	initFonts();
	initCursors();
	initOpcodes();
	initParsers();

	Parallaction::init();

	return 0;
}

Parallaction_ns::~Parallaction_ns() {
	freeFonts();

	_mouseComposedArrow->free();
	delete _mouseComposedArrow;


	delete _commandsNames;
	delete _instructionNames;
	delete _locationStmt;

}


void Parallaction_ns::freeFonts() {

	delete _dialogueFont;
	delete _labelFont;
	delete _menuFont;

	return;
}

void Parallaction_ns::renderLabel(Graphics::Surface *cnv, char *text) {

	if (getPlatform() == Common::kPlatformAmiga) {
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
		byte* s = _char._objs->getData(getInventoryItemIndex(index));
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


/*
	changeLocation handles transitions between locations, and is able to display slides
	between one and the other. The input parameter 'location' exists in some flavours:

    1 - [S].slide.[L]{.[C]}
	2 - [L]{.[C]}

    where:

	[S] is the slide to be shown
    [L] is the location to switch to (immediately in case 2, or right after slide [S] in case 1)
    [C] is the character to be selected, and is optional

    The routine tells one form from the other by searching for the '.slide.'

    NOTE: there exists one script in which [L] is not used in the case 1, but its use
		  is commented out, and would definitely crash the current implementation.
*/
void Parallaction_ns::changeLocation(char *location) {
	debugC(1, kDebugExec, "changeLocation(%s)", location);

	_soundMan->playLocationMusic(location);

	// WORKAROUND: this hideLabel has been added to avoid crashes caused by
	// execution of label jobs after a location switch. The other workaround in
	// Parallaction::runGame should have been rendered useless by this one.
	hideLabel(kPriority99);

	_hoverZone = NULL;
	if (_engineFlags & kEngineBlockInput) {
		changeCursor( kCursorArrow );
	}

	_animations.remove(&_char._ani);

	freeLocation();
	char buf[100];
	strcpy(buf, location);

	Common::StringList list;
	char *tok = strtok(location, ".");
	while (tok) {
		list.push_back(tok);
		tok = strtok(NULL, ".");
	}

	if (list.size() < 1 || list.size() > 4)
		error("changeLocation: ill-formed location string '%s'", location);

	if (list.size() > 1) {
		if (list[1] == "slide") {
			showSlide(list[0].c_str());
			_gfx->setFont(_menuFont);
			_gfx->displayCenteredString(14, _slideText[0]); // displays text on screen
			_gfx->updateScreen();
			waitUntilLeftClick();

			list.remove_at(0);		// removes slide name
			list.remove_at(0);		// removes 'slide'
		}

		// list is now only [L].{[C]} (see above comment)
		if (list.size() == 2) {
			changeCharacter(list[1].c_str());
			strcpy(_characterName, list[1].c_str());
		}
	}

	_animations.push_front(&_char._ani);

	strcpy(_saveData1, list[0].c_str());
	parseLocation(list[0].c_str());

	_char._ani._oldPos.x = -1000;
	_char._ani._oldPos.y = -1000;

	_char._ani.field_50 = 0;
	if (_location._startPosition.x != -1000) {
		_char._ani._left = _location._startPosition.x;
		_char._ani._top = _location._startPosition.y;
		_char._ani._frame = _location._startFrame;
		_location._startPosition.y = -1000;
		_location._startPosition.x = -1000;
	}


	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);
	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBit2);
	_gfx->setBlackPalette();
	_gfx->updateScreen();

	if (_location._commands.size() > 0) {
		runCommands(_location._commands);
		runJobs();
		_gfx->swapBuffers();
		runJobs();
		_gfx->swapBuffers();
	}

	if (_location._comment) {
		doLocationEnterTransition();
	}

	runJobs();
	_gfx->swapBuffers();

	_gfx->setPalette(_gfx->_palette);
	if (_location._aCommands.size() > 0) {
		runCommands(_location._aCommands);
	}

	if (_hasLocationSound)
		_soundMan->playSfx(_locationSound, 0, true);

	debugC(1, kDebugExec, "changeLocation() done");

	return;

}

void Parallaction_ns::changeCharacter(const char *name) {
	debugC(1, kDebugExec, "changeCharacter(%s)", name);

	char baseName[20];
	if (IS_MINI_CHARACTER(name)) {
		strcpy(baseName, name+4);
	} else {
		strcpy(baseName, name);
	}

	char fullName[20];
	strcpy(fullName, name);
	if (_engineFlags & kEngineTransformedDonna)
		strcat(fullName, "tras");

	if (scumm_stricmp(fullName, _characterName1)) {

		// freeCharacter takes responsibility for checking
		// character for sanity before memory is freed
		freeCharacter();

		Common::String oldArchive = _disk->selectArchive((getFeatures() & GF_LANG_MULT) ? "disk1" : "disk0");
		_char._ani._cnv = _disk->loadFrames(fullName);

		if (!IS_DUMMY_CHARACTER(name)) {
			if (getPlatform() == Common::kPlatformAmiga && (getFeatures() & GF_LANG_MULT))
				_disk->selectArchive("disk0");

			_char._head = _disk->loadHead(baseName);
			_char._talk = _disk->loadTalk(baseName);
			_char._objs = _disk->loadObjects(baseName);
			_objectsNames = _disk->loadTable(baseName);

			_soundMan->playCharacterMusic(name);

			if (!(getFeatures() & GF_DEMO))
				parseLocation("common");
		}

		if (!oldArchive.empty())
			_disk->selectArchive(oldArchive);
	}

	strcpy(_characterName1, fullName);

	debugC(1, kDebugExec, "changeCharacter() done");

	return;
}

void Parallaction_ns::initJobs() {

	static const JobFn jobs[] = {
		&Parallaction_ns::jobDisplayAnimations,
		&Parallaction_ns::jobEraseAnimations,
		&Parallaction_ns::jobDisplayDroppedItem,
		&Parallaction_ns::jobRemovePickedItem,
		&Parallaction_ns::jobRunScripts,
		&Parallaction_ns::jobWalk,
		&Parallaction_ns::jobDisplayLabel,
		&Parallaction_ns::jobEraseLabel,
		&Parallaction_ns::jobWaitRemoveJob,
		&Parallaction_ns::jobToggleDoor,
		&Parallaction_ns::jobShowInventory,
		&Parallaction_ns::jobHideInventory
	};

	_jobsFn = jobs;
}

JobOpcode* Parallaction_ns::createJobOpcode(uint functionId, Job *job) {
	return new OpcodeImpl2<Parallaction_ns>(this, _jobsFn[functionId], job);
}

} // namespace Parallaction
