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

#include "parallaction/parallaction.h"
#include "parallaction/sound.h"

namespace Parallaction {


DECLARE_LOCATION_PARSER(invalid) {
	error("unknown keyword '%s' in location '%s'", _tokens[0], _locParseCtxt.filename);
}

DECLARE_LOCATION_PARSER(endlocation) {
	_locParseCtxt.end = true;
}


DECLARE_LOCATION_PARSER(location) {
	// The parameter for location is 'location.mask'.
	// If mask is not present, then it is assumed
	// that path & mask are encoded in the background
	// bitmap, otherwise a separate .msk file exists.

	char *mask = strchr(_tokens[1], '.');
	if (mask) {
		mask[0] = '\0';
		mask++;
	}

	strcpy(_location._name, _tokens[1]);
	switchBackground(_location._name, mask);

	if (_tokens[2][0] != '\0') {
		_char._ani._left = atoi(_tokens[2]);
		_char._ani._top = atoi(_tokens[3]);
	}

	if (_tokens[4][0] != '\0') {
		_char._ani._frame = atoi(_tokens[4]);
	}
}


DECLARE_LOCATION_PARSER(disk) {
	_disk->selectArchive(_tokens[1]);
}


DECLARE_LOCATION_PARSER(nodes) {
	parseWalkNodes(*_locParseCtxt.script, _location._walkNodes);
}


DECLARE_LOCATION_PARSER(zone) {
	parseZone(*_locParseCtxt.script, _zones, _tokens[1]);
}


DECLARE_LOCATION_PARSER(animation) {
	parseAnimation(*_locParseCtxt.script, _animations, _tokens[1]);
}


DECLARE_LOCATION_PARSER(localflags) {
	int _si = 1;	// _localFlagNames[0] = 'visited'
	while (_tokens[_si][0] != '\0') {
		_localFlagNames->addData(_tokens[_si]);
		_si++;
	}
}


DECLARE_LOCATION_PARSER(commands) {
	parseCommands(*_locParseCtxt.script, _location._commands);
}


DECLARE_LOCATION_PARSER(acommands) {
	parseCommands(*_locParseCtxt.script, _location._aCommands);
}


DECLARE_LOCATION_PARSER(flags) {
	if ((_localFlags[_currentLocationIndex] & kFlagsVisited) == 0) {
		// only for 1st visit
		_localFlags[_currentLocationIndex] = 0;
		int _si = 1;

		do {
			byte _al = _localFlagNames->lookup(_tokens[_si]);
			_localFlags[_currentLocationIndex] |= 1 << (_al - 1);

			_si++;
			if (scumm_stricmp(_tokens[_si], "|")) break;
			_si++;
		} while (true);
	}
}


DECLARE_LOCATION_PARSER(comment) {
	_location._comment = parseComment(*_locParseCtxt.script);
}


DECLARE_LOCATION_PARSER(endcomment) {
	_location._endComment = parseComment(*_locParseCtxt.script);
}


DECLARE_LOCATION_PARSER(sound) {
	if (getPlatform() == Common::kPlatformAmiga) {
		strcpy(_locationSound, _tokens[1]);
		_hasLocationSound = true;
	}
}


DECLARE_LOCATION_PARSER(music) {
	if (getPlatform() == Common::kPlatformAmiga)
		_soundMan->setMusicFile(_tokens[1]);
}

DECLARE_LOCATION_PARSER(redundant) {
	warning("redundant '%s' line found in script '%s'", _tokens[0], _locParseCtxt.filename);
}


void Parallaction::parseLocation(const char *filename) {
    debugC(1, kDebugLocation, "parseLocation('%s')", filename);

	allocateLocationSlot(filename);

	Script *script = _disk->loadLocation(filename);

	// TODO: the following two lines are specific to Nippon Safes
	// and should be moved into something like 'initializeParsing()'
	_gfx->setFont(_labelFont);
	_hasLocationSound = false;

	_locParseCtxt.end = false;;
	_locParseCtxt.script = script;
	_locParseCtxt.filename = filename;


	do {

		fillBuffers(*script, true);

		int index = _locationStmt->lookup(_tokens[0]);
		(this->*_locationParsers[index])();

	} while (!_locParseCtxt.end);


	delete script;

	finalizeLocationParsing();

	return;
}

void Parallaction::allocateLocationSlot(const char *name) {
	// WORKAROUND: the original code erroneously incremented
	// _currentLocationIndex, thus producing inconsistent
	// savegames. This workaround modified the following loop
	// and if-statement, so the code exactly matches the one
	// in Big Red Adventure.
	_currentLocationIndex = -1;
	uint16 _di = 0;
	while (_locationNames[_di][0] != '\0') {
		if (!scumm_stricmp(_locationNames[_di], name)) {
			_currentLocationIndex = _di;
		}
		_di++;
	}

	if (_di == 120)
		error("No more location slots available. Please report this immediately to ScummVM team.");

	if (_currentLocationIndex  == -1) {
		strcpy(_locationNames[_numLocations], name);
		_currentLocationIndex = _numLocations;

		_numLocations++;
		_locationNames[_numLocations][0] = '\0';
		_localFlags[_numLocations] = 0;
	} else {
		_localFlags[_currentLocationIndex] |= kFlagsVisited;	// 'visited'
	}
}

void Parallaction::finalizeLocationParsing() {

	// this resolves any forward references in the script
	for (uint16 _si = 0; _forwardedCommands[_si]; _si++) {
		_forwardedCommands[_si]->u._animation = findAnimation(_forwardedAnimationNames[_si]);
		_forwardedCommands[_si] = NULL;
	}
	_numForwards = 0;

	// this loads animation scripts
	AnimationList::iterator it = _animations.begin();
	for ( ; it != _animations.end(); it++) {
		if ((*it)->_scriptName)
			loadProgram(*it, (*it)->_scriptName);
	}

	return;
}


void Parallaction::freeLocation() {
	debugC(7, kDebugLocation, "freeLocation");

	_soundMan->stopSfx(0);
	_soundMan->stopSfx(1);
	_soundMan->stopSfx(2);
	_soundMan->stopSfx(3);

	if (_localFlagNames)
		delete _localFlagNames;

	// HACK: prevents leakage. A routine like this
	// should allocate memory at all, though.
	if ((_engineFlags & kEngineQuit) == 0) {
		_localFlagNames = new Table(120);
		_localFlagNames->addData("visited");
	}

	_location._walkNodes.clear();

	freeZones();
	freeAnimations();

	if (_location._comment) {
		free(_location._comment);
	}
	_location._comment = NULL;

	_location._commands.clear();
	_location._aCommands.clear();

	return;
}



void Parallaction::parseWalkNodes(Script& script, WalkNodeList &list) {

	fillBuffers(script, true);
	while (scumm_stricmp(_tokens[0], "ENDNODES")) {

		if (!scumm_stricmp(_tokens[0], "COORD")) {

			WalkNode *v4 = new WalkNode(
				atoi(_tokens[1]) - _char._ani.width()/2,
				atoi(_tokens[2]) - _char._ani.height()
			);

			list.push_front(v4);
		}

		fillBuffers(script, true);
	}

	return;

}

void Parallaction::freeBackground() {

	if (!_backgroundInfo)
		return;

	_backgroundInfo->bg.free();
	_backgroundInfo->mask.free();
	_backgroundInfo->path.free();

	_pathBuffer = 0;

}

void Parallaction::setBackground(const char* name, const char* mask, const char* path) {

	_disk->loadScenery(*_backgroundInfo, name, mask, path);

	_gfx->setPalette(_backgroundInfo->palette);
	_gfx->_palette.clone(_backgroundInfo->palette);
	_gfx->setBackground(&_backgroundInfo->bg);

	if (_backgroundInfo->mask.data)
		_gfx->setMask(&_backgroundInfo->mask);

	if (_backgroundInfo->path.data)
		_pathBuffer = &_backgroundInfo->path;

	return;
}

void Parallaction::showLocationComment(const char *text, bool end) {

	_gfx->setFont(_dialogueFont);

	int16 w, h;
	_gfx->getStringExtent(const_cast<char*>(text), 130, &w, &h);

	Common::Rect r(w + (end ? 5 : 10), h + 5);
	r.moveTo(5, 5);

	_gfx->floodFill(Gfx::kBitFront, r, 0);
	r.grow(-2);
	_gfx->floodFill(Gfx::kBitFront, r, 1);
	_gfx->displayWrappedString(const_cast<char*>(text), 3, 5, 0, 130);

	_gfx->updateScreen();

	return;
}

void Parallaction::switchBackground(const char* background, const char* mask) {
//	printf("switchBackground(%s)", name);

	Palette pal;

	uint16 v2 = 0;
	if (!scumm_stricmp(background, "final")) {
		_gfx->clearScreen(Gfx::kBitBack);
		for (uint16 _si = 0; _si <= 32; _si++) {
			pal.setEntry(_si, v2, v2, v2);
			v2 += 4;
		}

		g_system->delayMillis(20);
		_gfx->setPalette(pal);
		_gfx->updateScreen();
	}

	setBackground(background, mask, mask);

	return;
}

extern Zone     *_hoverZone;
extern Job     *_jDrawLabel;
extern Job     *_jEraseLabel;

void Parallaction::showSlide(const char *name) {

	BackgroundInfo info;

	_disk->loadSlide(info, name);

	// TODO: avoid using screen buffers for displaying slides. Using a generic buffer
	// allows for positioning of graphics as needed by Big Red Adventure.
	// The main problem lies with menu, which relies on multiple buffers, mainly because
	// it is crappy code.
	_gfx->setBackground(&info.bg);
	_gfx->setPalette(info.palette);
	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

	info.bg.free();
	info.mask.free();
	info.path.free();

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
	debugC(1, kDebugLocation, "changeLocation(%s)", location);

	_soundMan->playLocationMusic(location);

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

	debugC(1, kDebugLocation, "changeLocation() done");

	return;

}

//	displays transition before a new location
//
//	clears screen (in white??)
//	shows location comment (if any)
//	waits for mouse click
//	fades towards game palette
//
void Parallaction::doLocationEnterTransition() {
	debugC(1, kDebugLocation, "doLocationEnterTransition");

    if (_localFlags[_currentLocationIndex] & kFlagsVisited) {
        debugC(3, kDebugLocation, "skipping location transition");
        return; // visited
    }

	Palette pal(_gfx->_palette);
	pal.makeGrayscale();
	_gfx->setPalette(pal);

	jobRunScripts(NULL, NULL);
	jobEraseAnimations(NULL, NULL);
	jobDisplayAnimations(NULL, NULL);

	_gfx->swapBuffers();
	_gfx->copyScreen(Gfx::kBitFront, Gfx::kBitBack);

	showLocationComment(_location._comment, false);
	waitUntilLeftClick();

	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront );

	// fades maximum intensity palette towards approximation of main palette
	for (uint16 _si = 0; _si<6; _si++) {
		pal.fadeTo(_gfx->_palette, 4);
		_gfx->setPalette(pal);
		waitTime( 1 );
		_gfx->updateScreen();
	}

	debugC(1, kDebugLocation, "doLocationEnterTransition completed");

	return;
}

} // namespace Parallaction
