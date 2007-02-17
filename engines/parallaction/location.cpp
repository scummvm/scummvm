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
void loadExternalMaskPath(char *filename);
void switchBackground(char *name);
void parseWalkNodes(ArchivedFile *file, Node *list);
void freeAnimations();

Node helperNode = { NULL, NULL };

void Parallaction::parseLocation(const char *filename) {
//	printf("parseLocation(%s)", filename);

	char *location_src = NULL;

	uint16 _si = 1;
	_vm->_graphics->_proportionalFont = false;
	_vm->_graphics->loadExternalCnv("topazcnv", &Graphics::_font);

	char archivefile[PATH_LEN];

	if (_characterName[0] == 'm') {
		sprintf(archivefile, "%s%s", _characterName+4, _languageDir);
	} else {
		if (_characterName[0] == 'D') strcpy(archivefile, _languageDir);
		else {
			sprintf(archivefile, "%s%s", _characterName, _languageDir);
		}
	}
	strcat(archivefile, filename);
	strcat(archivefile, ".loc");

	if (strcmp(_disk, "null")) closeArchive();

	_languageDir[2] = '\0';
	openArchive(_languageDir);
	_languageDir[2] = '/';
	ArchivedFile *file = openArchivedFile(archivefile);
	if (!file) {
		sprintf(archivefile, "%s%s.loc", _languageDir, filename);
		file = openArchivedFile(archivefile);
		if (!file) errorFileNotFound(filename);
	}

	uint32 count = file->_endOffset - file->_offset;
	location_src = (char*)memAlloc(0x4000);

	parseInit(location_src);

	readArchivedFile(file, location_src, count);
	closeArchivedFile(file);
	closeArchive();

	parseFillBuffers();
	while (scumm_stricmp(_tokens[0], "ENDLOCATION")) {
//		printf("token[0] = %s", _tokens[0]);

		if (!scumm_stricmp(_tokens[0], "LOCATION")) {
			char *background = strchr(_tokens[1], '.');
			if (background) {
				background[0] = '\0';
				background++;
			} else
				background = _tokens[1];

			_currentLocationIndex = -1;
			uint16 _di = 0;
			while (_locationNames[_di][0] != '\0') {
				if (!scumm_stricmp(_locationNames[_di], filename)) {
					_currentLocationIndex = _di + 1;
				}
				_di++;
			}

			if (_currentLocationIndex  == -1) {
				strcpy(_locationNames[_numLocations], filename);
				_numLocations++;
				_currentLocationIndex = _numLocations;
				_locationNames[_numLocations][0] = '\0';
				_localFlags[_currentLocationIndex] = 0;
			} else {
				_localFlags[_currentLocationIndex] |= 1;	// 'visited'
			}

			strcpy(_location, _tokens[1]);
			switchBackground(background);

			if (_tokens[2][0] != '\0') {
				_yourself._zone.pos._position._x = atoi(_tokens[2]);
				_yourself._zone.pos._position._y = atoi(_tokens[3]);
			}

			if (_tokens[4][0] != '\0') {
				_yourself._frame = atoi(_tokens[4]);
			}
		}
		if (!scumm_stricmp(_tokens[0], "DISK")) {
			strcpy(_disk, _tokens[1]);
			strcpy(archivefile, _disk);
			openArchive(archivefile);
		}
		if (!scumm_stricmp(_tokens[0], "LOCALFLAGS")) {
			_si = 1;	// _localFlagNames[0] = 'visited'
			while (_tokens[_si][0] != '\0') {
				_localFlagNames[_si] = (char*)memAlloc(strlen(_tokens[_si])+1);
				strcpy(_localFlagNames[_si], _tokens[_si]);
				_si++;
			}
			_localFlagNames[_si] = 0;
		}
		if (!scumm_stricmp(_tokens[0], "COMMANDS")) {
			_locationCommands = parseCommands(file);
		}
		if (!scumm_stricmp(_tokens[0], "ACOMMANDS")) {
			_locationACommands = parseCommands(file);
		}
		if (!scumm_stricmp(_tokens[0], "FLAGS")) {
			if ((_localFlags[_currentLocationIndex] & 1) == 0) {
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
			_locationComment = parseComment(file);
		}
		if (!scumm_stricmp(_tokens[0], "ENDCOMMENT")) {
			_locationEndComment = parseComment(file);
		}
		if (!scumm_stricmp(_tokens[0], "ZONE")) {
			parseZone(file, &_zones, _tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "NODES")) {
			parseWalkNodes(file, &_locationWalkNodes);
		}
		if (!scumm_stricmp(_tokens[0], "ANIMATION")) {
			parseAnimation(file, &_animations, _tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "SOUND")) {
			strcpy(_soundFile, _tokens[1]);
		}
		parseFillBuffers();
	}

	resolveLocationForwards();
	_vm->_graphics->freeCnv(&Graphics::_font);
	memFree(location_src);

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
		memFree(_localFlagNames[_si]);
		_localFlagNames[_si] = NULL;
		_si++;
	}
	debugC(7, kDebugLocation, "freeLocation: localflags names freed");


	freeNodeList(_locationWalkNodes._next);
	_locationWalkNodes._next = NULL;
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

	if (_locationComment) {
		memFree(_locationComment);
	}
	_locationComment = NULL;
	debugC(7, kDebugLocation, "freeLocation: comments freed");

	if (_locationCommands) {
		freeNodeList(&_locationCommands->_node);
	}
	_locationCommands = NULL;
	debugC(7, kDebugLocation, "freeLocation: commands freed");

	if (_locationACommands) {
		freeNodeList(&_locationACommands->_node);
	}
	_locationACommands = NULL;
	debugC(7, kDebugLocation, "freeLocation: acommands freed");

	return;
}



void parseWalkNodes(ArchivedFile *file, Node *list) {

	parseFillBuffers();
	while (scumm_stricmp(_tokens[0], "ENDNODES")) {

		if (!scumm_stricmp(_tokens[0], "COORD")) {

			WalkNode *v4 = (WalkNode*)memAlloc(sizeof(WalkNode));
			v4->_x = atoi(_tokens[1]) - _yourself._cnv._width/2;
			v4->_y = atoi(_tokens[2]) - _yourself._cnv._height;

			addNode(list, &v4->_node);

		}

		parseFillBuffers();
	}

	return;

}

void switchBackground(char *name) {
//	printf("switchBackground(%s)", name);

	byte palette[PALETTE_SIZE];

	uint16 v2 = 0;
	if (!scumm_stricmp(_location, "final")) {
		_vm->_graphics->clearScreen(Graphics::kBitBack);
		for (uint16 _si = 0; _si <= 93; ) {
			palette[_si] = v2;
			palette[_si+1] = v2;
			palette[_si+2] = v2;
			v2 += 4;
			_si += 3;
		}

		_vm->_graphics->palUnk0(palette);
	}

	char dest[PATH_LEN];
	strcpy(dest, _location);
	strcat(dest, ".dyn");
	_vm->_graphics->loadBackground(dest, Graphics::kBitBack);
	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBit2);

	if (!scumm_stricmp(_location, name)) return;

	// load external masks and paths only for certain locations
	sprintf(dest, "%s.msk", name);
	_vm->_graphics->loadMaskAndPath(dest);

	return;
}

extern Zone     *_hoverZone;
extern Job     *_jDrawLabel;
extern Job     *_jEraseLabel;


void Parallaction::changeLocation(char *location) {
	debugC(1, kDebugLocation, "changeLocation to '%s'", location);

//	printf("changeLocation('%s')", location);
	if (_musicData1 != 0) {
		if (!scumm_stricmp(_characterName, "dino"))
			loadMusic("dino");
		else
		if (!scumm_stricmp(_characterName, "donna"))
			loadMusic("donna");
		else
			loadMusic("nuts");

		playMusic();
		_musicData1 = 0;

		debugC(2, kDebugLocation, "changeLocation: started character specific music");
	}

	if (!scumm_stricmp(location, "night") || !scumm_stricmp(location, "intsushi")) {
		stopMusic();
		loadMusic("soft");
		playMusic();

		debugC(2, kDebugLocation, "changeLocation: started music 'soft'");
	}

	if (!scumm_stricmp(location, "museo") ||
		!scumm_stricmp(location, "caveau") ||
		!scumm_strnicmp(location, "plaza1", 6) ||
		!scumm_stricmp(location, "estgrotta") ||
		!scumm_stricmp(location, "intgrottadopo") ||
		!scumm_stricmp(location, "endtgz") ||
		!scumm_stricmp(location, "common")) {

		stopMusic();
		_musicData1 = 1;

		debugC(2, kDebugLocation, "changeLocation: music stopped");
	}

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

	strcpy(_newLocation, location);

	removeNode(&_yourself._zone._node);
	debugC(2, kDebugLocation, "changeLocation: removed character from the animation list");

	freeLocation();
	debugC(1, kDebugLocation, "changeLocation: old location free'd");


	char *tmp = strchr(_newLocation, '.');
	if (tmp) {
		*tmp = '\0';

		if (!scumm_strnicmp(tmp+1, "slide", 5)) {
			char filename[200];
			sprintf(filename, "%s.slide", _newLocation);

			_vm->_graphics->loadBackground(filename, Graphics::kBitBack);
			_vm->_graphics->palUnk0(_palette);
			_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront);

			debugC(1, kDebugLocation, "changeLocation: new background set");

			_vm->_graphics->_proportionalFont = false;
			_vm->_graphics->loadExternalCnv("slidecnv", &Graphics::_font);

			uint16 _ax = strlen(_slideText[0]);
			_ax <<= 3;	// text width
			uint16 _dx = (SCREEN_WIDTH - _ax) >> 1; // center text
			_vm->_graphics->displayString(_dx, 14, _slideText[0]); // displays text on screen

			_vm->_graphics->freeCnv(&Graphics::_font);
			waitUntilLeftClick();

			debugC(2, kDebugLocation, "changeLocation: intro text shown");

			tmp = strchr(tmp+1, '.');
			strcpy(_newLocation, tmp+1);
			tmp = strchr(_newLocation, '.');

			if (tmp) {
				*tmp = '\0';
				changeCharacter(tmp+1);
				strcpy(_characterName, tmp+1);
			}

		} else {
			changeCharacter(tmp+1);
			strcpy(_characterName, tmp+1);
		}

		debugC(2, kDebugLocation, "changeLocation: character changed to '%s'", _characterName);
	}

	addNode(&_animations, &_yourself._zone._node);
	debugC(2, kDebugLocation, "changeLocation: new character added to the animation list");

	strcpy(_saveData1, _newLocation);

	parseLocation(_newLocation);
	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBit2);
	debugC(1, kDebugLocation, "changeLocation: new location '%s' parsed", _newLocation);

	_yourself._zone.pos._oldposition._x = -1000;
	_yourself._zone.pos._oldposition._y = -1000;

	_yourself.field_50 = 0;
	if (_firstPosition._x != -1000) {
		_yourself._zone.pos._position._x = _firstPosition._x;
		_yourself._zone.pos._position._y = _firstPosition._y;
		_yourself._frame = _firstFrame;
		_firstPosition._y = -1000;
		_firstPosition._x = -1000;

		debugC(2, kDebugLocation, "changeLocation: initial position set to x: %i, y: %i, f: %i", _firstPosition._x, _firstPosition._y, _firstFrame);
	}

	byte palette[PALETTE_SIZE];
	for (uint16 _si = 0; _si < PALETTE_SIZE; _si++) palette[_si] = 0;
	_vm->_graphics->palUnk0(palette);
	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront);
	if (_locationCommands) {
		runCommands(_locationCommands);
		runJobs();
		_vm->_graphics->swapBuffers();
		runJobs();
		_vm->_graphics->swapBuffers();
	}

	if (_locationComment) {
		doLocationEnterTransition();
		debugC(2, kDebugLocation, "changeLocation: shown location comment");
	}

	runJobs();
	_vm->_graphics->swapBuffers();

	_vm->_graphics->palUnk0(_palette);
	if (_locationACommands) {
		runCommands(_locationACommands);
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
	if (_localFlags[_currentLocationIndex] & 1) return; // visited

	_vm->_graphics->buildBWPalette(v60);
	_vm->_graphics->setPalette(v60);

	jobRunScripts(NULL, NULL);
	jobEraseAnimations(NULL, NULL);
	jobDisplayAnimations(NULL, NULL);

	_vm->_graphics->loadExternalCnv("comiccnv", &Graphics::_font);
	_vm->_graphics->swapBuffers();
	_vm->_graphics->copyScreen(Graphics::kBitFront, Graphics::kBitBack);

	int16 v7C, v7A;
	_vm->_graphics->getStringExtent(_locationComment, 130, &v7C, &v7A);
	_vm->_graphics->floodFill(0, 5, 5, 10 + v7C, 5 + v7A, Graphics::kBitFront);
	_vm->_graphics->floodFill(1, 6, 6, 9 + v7C, 4 + v7A, Graphics::kBitFront);
	_vm->_graphics->displayWrappedString(_locationComment, 3, 5, 130, 0);

	_vm->_graphics->freeCnv( &Graphics::_font );

	// FIXME: ???
#if 0
	do {
		mouseFunc1();
	} while (_mouseButtons != kMouseLeftUp);
#endif

	waitUntilLeftClick();

	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront );

	// fades maximum intensity palette towards approximation of main palette
	for (uint16 _si = 0; _si<6; _si++) {
		waitTime( 1 );
		_vm->_graphics->quickFadePalette(v60);
		_vm->_graphics->setPalette(v60);
	}

	debugC(1, kDebugLocation, "doLocationEnterTransition completed");

	return;
}

void mouseFunc1() {
	// FIXME: implement this
}

} // namespace Parallaction
