/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "parallaction/parallaction.h"
#include "parallaction/graphics.h"
#include "parallaction/disk.h"
#include "parallaction/parser.h"
#include "parallaction/music.h"
#include "parallaction/commands.h"
#include "parallaction/zone.h"

namespace Parallaction {

void resolveLocationForwards();
void switchBackground(const char* background, const char* mask);
void parseWalkNodes(Script &script, Node *list);
void freeAnimations();

Node helperNode = { NULL, NULL };

void Parallaction::parseLocation(const char *filename) {
//	printf("parseLocation(%s)", filename);
    debugC(1, kDebugLocation, "parseLocation('%s')", filename);

	uint16 _si = 1;
	_vm->_gfx->_proportionalFont = false;
	_vm->_gfx->setFont("topaz");

	_locationScript = _disk->loadLocation(filename);

	fillBuffers(*_locationScript, true);
	while (scumm_stricmp(_tokens[0], "ENDLOCATION")) {
//		printf("token[0] = %s", _tokens[0]);

		if (!scumm_stricmp(_tokens[0], "LOCATION")) {
			// The parameter for location is 'location.mask'.
			// If mask is not present, then it is assumed
			// that path & mask are encoded in the background
			// bitmap, otherwise a separate .msk file exists.

			char *mask = strchr(_tokens[1], '.');
			if (mask) {
				mask[0] = '\0';
				mask++;
			}

            // WORKAROUND: the original code erroneously incremented
            // _currentLocationIndex, thus producing inconsistent
            // savegames. This workaround modified the following loop
            // and if-statement, so the code exactly matches the one
            // in Big Red Adventure.
			_currentLocationIndex = -1;
			uint16 _di = 0;
			while (_locationNames[_di][0] != '\0') {
				if (!scumm_stricmp(_locationNames[_di], filename)) {
					_currentLocationIndex = _di;
				}
				_di++;
			}

			if (_currentLocationIndex  == -1) {
				strcpy(_locationNames[_numLocations], filename);
				_currentLocationIndex = _numLocations;

				_numLocations++;
				_locationNames[_numLocations][0] = '\0';
				_localFlags[_numLocations] = 0;
			} else {
				_localFlags[_currentLocationIndex] |= kFlagsVisited;	// 'visited'
			}

			strcpy(_location._name, _tokens[1]);
			switchBackground(_location._name, mask);

			if (_tokens[2][0] != '\0') {
				_vm->_char._ani._zone.pos._position._x = atoi(_tokens[2]);
				_vm->_char._ani._zone.pos._position._y = atoi(_tokens[3]);
			}

			if (_tokens[4][0] != '\0') {
				_vm->_char._ani._frame = atoi(_tokens[4]);
			}
		}
		if (!scumm_stricmp(_tokens[0], "DISK")) {
			_disk->selectArchive(_tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "LOCALFLAGS")) {
			_si = 1;	// _localFlagNames[0] = 'visited'
			while (_tokens[_si][0] != '\0') {
				_localFlagNames[_si] = (char*)malloc(strlen(_tokens[_si])+1);
				strcpy(_localFlagNames[_si], _tokens[_si]);
				_si++;
			}
			_localFlagNames[_si] = 0;
		}
		if (!scumm_stricmp(_tokens[0], "COMMANDS")) {
			_location._commands = parseCommands(*_locationScript);
		}
		if (!scumm_stricmp(_tokens[0], "ACOMMANDS")) {
			_location._aCommands = parseCommands(*_locationScript);
		}
		if (!scumm_stricmp(_tokens[0], "FLAGS")) {
			if ((_localFlags[_currentLocationIndex] & kFlagsVisited) == 0) {
				// only for 1st visit
				_localFlags[_currentLocationIndex] = 0;
				_si = 1;

				do {
					byte _al = searchTable(_tokens[_si], const_cast<const char **>(_localFlagNames));
					_localFlags[_currentLocationIndex] |= 1 << (_al - 1);

					_si++;
					if (scumm_stricmp(_tokens[_si], "|")) break;
					_si++;
				} while (true);
			}
		}
		if (!scumm_stricmp(_tokens[0], "COMMENT")) {
			_location._comment = parseComment(*_locationScript);
		}
		if (!scumm_stricmp(_tokens[0], "ENDCOMMENT")) {
			_location._endComment = parseComment(*_locationScript);
		}
		if (!scumm_stricmp(_tokens[0], "ZONE")) {
			parseZone(*_locationScript, &_zones, _tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "NODES")) {
			parseWalkNodes(*_locationScript, &_location._walkNodes);
		}
		if (!scumm_stricmp(_tokens[0], "ANIMATION")) {
			parseAnimation(*_locationScript, &_animations, _tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "SOUND")) {
			strcpy(_soundFile, _tokens[1]);
		}
		fillBuffers(*_locationScript, true);
	}

	resolveLocationForwards();

	delete _locationScript;
	_locationScript = NULL;

	return;
}

void resolveLocationForwards() {
//	printf("resolveLocationForwards()");
//	printf("# forwards: %i", _numForwards);

	for (uint16 _si = 0; _forwardedCommands[_si]; _si++) {
		_forwardedCommands[_si]->u._animation = findAnimation(_forwardedAnimationNames[_si]);
		_forwardedCommands[_si] = NULL;
	}

	_numForwards = 0;
	return;
}


void freeLocation() {
	debugC(7, kDebugLocation, "freeLocation");

	uint16 _si = 1;
	while (_localFlagNames[_si] != 0) {
		free(_localFlagNames[_si]);
		_localFlagNames[_si] = NULL;
		_si++;
	}
	debugC(7, kDebugLocation, "freeLocation: localflags names freed");


	freeNodeList(_vm->_location._walkNodes._next);
	_vm->_location._walkNodes._next = NULL;
	debugC(7, kDebugLocation, "freeLocation: walk nodes freed");

	helperNode._prev = helperNode._next = NULL;
	freeZones(_zones._next);
	freeNodeList(_zones._next);
	memcpy(&_zones, &helperNode, sizeof(Node));
	debugC(7, kDebugLocation, "freeLocation: zones freed");

	helperNode._prev = helperNode._next = NULL;
	freeZones(_animations._next);
	freeAnimations();
	freeNodeList(_animations._next);
	memcpy(&_animations, &helperNode, sizeof(Node));
	debugC(7, kDebugLocation, "freeLocation: animations freed");

	if (_vm->_location._comment) {
		free(_vm->_location._comment);
	}
	_vm->_location._comment = NULL;
	debugC(7, kDebugLocation, "freeLocation: comments freed");

	if (_vm->_location._commands) {
		freeNodeList(&_vm->_location._commands->_node);
	}
	_vm->_location._commands = NULL;
	debugC(7, kDebugLocation, "freeLocation: commands freed");

	if (_vm->_location._aCommands) {
		freeNodeList(&_vm->_location._aCommands->_node);
	}
	_vm->_location._aCommands = NULL;
	debugC(7, kDebugLocation, "freeLocation: acommands freed");

	return;
}



void parseWalkNodes(Script& script, Node *list) {

	fillBuffers(script, true);
	while (scumm_stricmp(_tokens[0], "ENDNODES")) {

		if (!scumm_stricmp(_tokens[0], "COORD")) {

			WalkNode *v4 = (WalkNode*)malloc(sizeof(WalkNode));
			v4->_x = atoi(_tokens[1]) - _vm->_char._ani._cnv._width/2;
			v4->_y = atoi(_tokens[2]) - _vm->_char._ani._cnv._height;

			addNode(list, &v4->_node);

		}

		fillBuffers(script, true);
	}

	return;

}

void switchBackground(const char* background, const char* mask) {
//	printf("switchBackground(%s)", name);

	byte palette[PALETTE_SIZE];

	uint16 v2 = 0;
	if (!scumm_stricmp(background, "final")) {
		_vm->_gfx->clearScreen(Gfx::kBitBack);
		for (uint16 _si = 0; _si <= 93; ) {
			palette[_si] = v2;
			palette[_si+1] = v2;
			palette[_si+2] = v2;
			v2 += 4;
			_si += 3;
		}

		_vm->_gfx->palUnk0(palette);
	}

	_vm->_disk->loadScenery(background, mask);

	return;
}

extern Zone     *_hoverZone;
extern Job     *_jDrawLabel;
extern Job     *_jEraseLabel;

void Parallaction::showSlide(const char *name) {

	_disk->loadSlide(name);
	_gfx->palUnk0(_vm->_gfx->_palette);
	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

	debugC(1, kDebugLocation, "changeLocation: new background set");

	_gfx->_proportionalFont = false;
	_gfx->setFont("slide");

	uint16 _ax = strlen(_slideText[0]);
	_ax <<= 3;	// text width
	uint16 _dx = (SCREEN_WIDTH - _ax) >> 1; // center text
	_gfx->displayString(_dx, 14, _slideText[0]); // displays text on screen

	waitUntilLeftClick();

	debugC(2, kDebugLocation, "changeLocation: intro text shown");

	return;
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
void Parallaction::changeLocation(char *location) {
	debugC(1, kDebugLocation, "changeLocation to '%s'", location);

	pickMusic(location);

	// WORKAROUND: this if-statement has been added to avoid crashes caused by
	// execution of label jobs after a location switch. The other workaround in
	// Parallaction::runGame should have been rendered useless by this one.
	if (_jDrawLabel != NULL) {
		removeJob(_jDrawLabel);
		removeJob(_jEraseLabel);
		_jDrawLabel = NULL;
		_jEraseLabel = NULL;
	}


	_hoverZone = NULL;
	if (_engineFlags & kEngineMouse) {
		changeCursor( kCursorArrow );
		debugC(2, kDebugLocation, "changeLocation: changed cursor");
	}

	removeNode(&_vm->_char._ani._zone._node);
	debugC(2, kDebugLocation, "changeLocation: removed character from the animation list");

	freeLocation();
	debugC(1, kDebugLocation, "changeLocation: old location free'd");

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
			list.remove_at(0);		// removes slide name
			list.remove_at(0);		// removes 'slide'
		}

		// list is now only [L].{[C]} (see above comment)
		if (list.size() == 2) {
			changeCharacter(list[1].c_str());
			strcpy(_characterName, list[1].c_str());
		}
	}

	addNode(&_animations, &_vm->_char._ani._zone._node);
	debugC(2, kDebugLocation, "changeLocation: new character added to the animation list");

	strcpy(_saveData1, list[0].c_str());
	parseLocation(list[0].c_str());

	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBit2);
	debugC(1, kDebugLocation, "changeLocation: new location '%s' parsed", _saveData1);

	_vm->_char._ani._zone.pos._oldposition._x = -1000;
	_vm->_char._ani._zone.pos._oldposition._y = -1000;

	_vm->_char._ani.field_50 = 0;
	if (_location._startPosition._x != -1000) {
		_vm->_char._ani._zone.pos._position._x = _location._startPosition._x;
		_vm->_char._ani._zone.pos._position._y = _location._startPosition._y;
		_vm->_char._ani._frame = _location._startFrame;
		_location._startPosition._y = -1000;
		_location._startPosition._x = -1000;

		debugC(2, kDebugLocation, "changeLocation: initial position set to x: %i, y: %i, f: %i", _location._startPosition._x, _location._startPosition._y, _location._startFrame);
	}

	byte palette[PALETTE_SIZE];
	for (uint16 _si = 0; _si < PALETTE_SIZE; _si++) palette[_si] = 0;
	_gfx->palUnk0(palette);
	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);
	if (_location._commands) {
		runCommands(_location._commands);
		runJobs();
		_gfx->swapBuffers();
		runJobs();
		_gfx->swapBuffers();
	}

	if (_location._comment) {
		doLocationEnterTransition();
		debugC(2, kDebugLocation, "changeLocation: shown location comment");
	}

	runJobs();
	_gfx->swapBuffers();

	_gfx->palUnk0(_vm->_gfx->_palette);
	if (_location._aCommands) {
		runCommands(_location._aCommands);
		debugC(1, kDebugLocation, "changeLocation: location acommands run");
	}

	debugC(1, kDebugLocation, "changeLocation completed");

	return;

}

//	displays transition before a new location
//
//	clears screen (in white??)
//	shows location comment (if any)
//	waits for mouse click
//	fades towards game palette
//
//	THE FINAL PALETTE IS NOT THE SAME AS THE MAIN PALETTE!!!!!!
//
void Parallaction::doLocationEnterTransition() {
	debugC(1, kDebugLocation, "doLocationEnterTransition");

	byte v60[PALETTE_SIZE];
	if (_localFlags[_currentLocationIndex] & kFlagsVisited) return; // visited

	_vm->_gfx->buildBWPalette(v60);
	_vm->_gfx->setPalette(v60);

	jobRunScripts(NULL, NULL);
	jobEraseAnimations(NULL, NULL);
	jobDisplayAnimations(NULL, NULL);

	_vm->_gfx->setFont("comic");
	_vm->_gfx->swapBuffers();
	_vm->_gfx->copyScreen(Gfx::kBitFront, Gfx::kBitBack);

	int16 v7C, v7A;
	_vm->_gfx->getStringExtent(_vm->_location._comment, 130, &v7C, &v7A);

	Common::Rect r(10 + v7C, 5 + v7A);
	r.moveTo(5, 5);
	_vm->_gfx->floodFill(Gfx::kBitFront, r, 0);
	r.grow(-1);
	_vm->_gfx->floodFill(Gfx::kBitFront, r, 1);
	_vm->_gfx->displayWrappedString(_vm->_location._comment, 3, 5, 130, 0);

	// FIXME: ???
#if 0
	do {
		mouseFunc1();
	} while (_mouseButtons != kMouseLeftUp);
#endif

	waitUntilLeftClick();

	_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront );

	// fades maximum intensity palette towards approximation of main palette
	for (uint16 _si = 0; _si<6; _si++) {
		waitTime( 1 );
		_vm->_gfx->quickFadePalette(v60);
		_vm->_gfx->setPalette(v60);
	}

	debugC(1, kDebugLocation, "doLocationEnterTransition completed");

	return;
}

void mouseFunc1() {
	// FIXME: implement this
}

} // namespace Parallaction
