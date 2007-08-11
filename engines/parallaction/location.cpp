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


void Parallaction::parseLocation(const char *filename) {
    debugC(1, kDebugLocation, "parseLocation('%s')", filename);

	uint16 _si = 1;
	_gfx->setFont(kFontLabel);

	Script *_locationScript = _disk->loadLocation(filename);
	_hasLocationSound = false;

	fillBuffers(*_locationScript, true);
	while (scumm_stricmp(_tokens[0], "ENDLOCATION")) {

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
				_char._ani._left = atoi(_tokens[2]);
				_char._ani._top = atoi(_tokens[3]);
			}

			if (_tokens[4][0] != '\0') {
				_char._ani._frame = atoi(_tokens[4]);
			}
		}
		if (!scumm_stricmp(_tokens[0], "DISK")) {
			_disk->selectArchive(_tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "LOCALFLAGS")) {
			_si = 1;	// _localFlagNames[0] = 'visited'
			while (_tokens[_si][0] != '\0') {
				_localFlagNames->addData(_tokens[_si]);
				_si++;
			}
		}
		if (!scumm_stricmp(_tokens[0], "COMMANDS")) {
			parseCommands(*_locationScript, _location._commands);
		}
		if (!scumm_stricmp(_tokens[0], "ACOMMANDS")) {
			parseCommands(*_locationScript, _location._aCommands);
		}
		if (!scumm_stricmp(_tokens[0], "FLAGS")) {
			if ((_localFlags[_currentLocationIndex] & kFlagsVisited) == 0) {
				// only for 1st visit
				_localFlags[_currentLocationIndex] = 0;
				_si = 1;

				do {
					byte _al = _localFlagNames->lookup(_tokens[_si]);
					_localFlags[_currentLocationIndex] |= 1 << (_al - 1);

					_si++;
					if (scumm_stricmp(_tokens[_si], "|")) break;
					_si++;
				} while (true);
			}
		}
		if (!scumm_stricmp(_tokens[0], "COMMENT")) {
			_location._comment = parseComment(*_locationScript);
			debugC(3, kDebugLocation, "Location comment: '%s'", _location._comment);
		}
		if (!scumm_stricmp(_tokens[0], "ENDCOMMENT")) {
			_location._endComment = parseComment(*_locationScript);
		}
		if (!scumm_stricmp(_tokens[0], "ZONE")) {
			parseZone(*_locationScript, _zones, _tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "NODES")) {
			parseWalkNodes(*_locationScript, _location._walkNodes);
		}
		if (!scumm_stricmp(_tokens[0], "ANIMATION")) {
			parseAnimation(*_locationScript, _animations, _tokens[1]);
		}
		if (!scumm_stricmp(_tokens[0], "SOUND")) {
			if (getPlatform() == Common::kPlatformAmiga) {
				strcpy(_locationSound, _tokens[1]);
				_hasLocationSound = true;
			}
		}
		if (!scumm_stricmp(_tokens[0], "MUSIC")) {
			if (getPlatform() == Common::kPlatformAmiga)
				_soundMan->setMusicFile(_tokens[1]);
		}
		fillBuffers(*_locationScript, true);
	}

	resolveLocationForwards();

	delete _locationScript;

	return;
}

void Parallaction::resolveLocationForwards() {

	for (uint16 _si = 0; _forwardedCommands[_si]; _si++) {
		_forwardedCommands[_si]->u._animation = findAnimation(_forwardedAnimationNames[_si]);
		_forwardedCommands[_si] = NULL;
	}

	_numForwards = 0;
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

	// TODO (LIST): helperNode should be rendered useless by the use of a Common::List<>
	// to store Zones and Animations. Right now, it holds a list of Zones to be preserved
	// but that'll pretty meaningless with a single list approach.
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
	delete _backgroundInfo;

	_backgroundInfo = 0;
	_pathBuffer = 0;

}

void Parallaction::setBackground(const char *background, const char *mask, const char *path) {

	freeBackground();

	_backgroundInfo = _disk->loadScenery(background, mask, path);

	_gfx->setPalette(_backgroundInfo->palette);
	_gfx->_palette.clone(_backgroundInfo->palette);
	_gfx->setBackground(&_backgroundInfo->bg);

	if (_backgroundInfo->mask.data)
		_gfx->setMask(&_backgroundInfo->mask);

	if (_backgroundInfo->path.data)
		_pathBuffer = &_backgroundInfo->path;

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

	BackgroundInfo *info;

	info = _disk->loadSlide(name);

	// TODO: avoid using screen buffers for displaying slides. Using a generic buffer
	// allows for positioning of graphics as needed by Big Red Adventure.
	// The main problem lies with menu, which relies on multiple buffers, mainly because
	// it is crappy code.
	_gfx->setBackground(&info->bg);
	_gfx->setPalette(info->palette);
	_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

	info->bg.free();
	info->mask.free();
	info->path.free();

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
			_gfx->setFont(kFontMenu);
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
//	THE FINAL PALETTE IS NOT THE SAME AS THE MAIN PALETTE!!!!!!
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

	_gfx->showLocationComment(_location._comment);
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
