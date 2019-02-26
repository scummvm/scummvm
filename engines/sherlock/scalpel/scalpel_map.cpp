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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "sherlock/scalpel/scalpel_map.h"
#include "sherlock/events.h"
#include "sherlock/people.h"
#include "sherlock/screen.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Scalpel {

MapPaths::MapPaths() {
	_numLocations = 0;
}

void MapPaths::load(int numLocations, Common::SeekableReadStream &s) {
	_numLocations = numLocations;
	_paths.resize(_numLocations * _numLocations);

	for (int idx = 0; idx < (numLocations * numLocations); ++idx) {
		Common::Array<byte> &path = _paths[idx];
		int v;

		do {
			v = s.readByte();
			path.push_back(v);
		} while (v && v < 254);
	}
}

const byte *MapPaths::getPath(int srcLocation, int destLocation) {
	return &_paths[srcLocation * _numLocations + destLocation][0];
}

/*----------------------------------------------------------------*/

ScalpelMap::ScalpelMap(SherlockEngine *vm): Map(vm), _topLine(g_system->getWidth(), 12, g_system->getScreenFormat()) {
	_mapCursors = nullptr;
	_shapes = nullptr;
	_iconShapes = nullptr;
	_point = 0;
	_placesShown = false;
	_cursorIndex = -1;
	_drawMap = false;
	_overPos = Point32(130 * FIXED_INT_MULTIPLIER, 126 * FIXED_INT_MULTIPLIER);
	_frameChangeFlag = false;

	// Initialise the initial walk sequence set
	_walkSequences.resize(MAX_HOLMES_SEQUENCE);
	for (int idx = 0; idx < MAX_HOLMES_SEQUENCE; ++idx) {
		_walkSequences[idx]._sequences.resize(MAX_FRAME);
		Common::fill(&_walkSequences[idx]._sequences[0], &_walkSequences[idx]._sequences[0] + MAX_FRAME, 0);
	}

	if (!_vm->isDemo())
		loadData();
}

void ScalpelMap::loadPoints(int count, const int *xList, const int *yList, const int *transList) {
	for (int idx = 0; idx < count; ++idx, ++xList, ++yList, ++transList) {
		_points.push_back(MapEntry(*xList, *yList, *transList));
	}
}

void ScalpelMap::loadSequences(int count, const byte *seq) {
	for (int idx = 0; idx < count; ++idx, seq += MAX_FRAME)
		Common::copy(seq, seq + MAX_FRAME, &_walkSequences[idx]._sequences[0]);
}

void ScalpelMap::loadData() {
	// Load the list of location names
	Common::SeekableReadStream *txtStream = _vm->_res->load("chess.txt");

	int streamSize = txtStream->size();
	while (txtStream->pos() < streamSize) {
		Common::String line;
		char c;
		while ((c = txtStream->readByte()) != '\0')
			line += c;

		// WORKAROUND: Special fixes for faulty translations
		// Was obviously not done in the original interpreter
		if (_vm->getLanguage() == Common::ES_ESP) {
			// Spanish version
			if (line == " Alley") {
				// The "Alley" location was not translated, we do this now
				// see bug #6931
				line = " Callejon";
			} else if (line == " Alamacen") {
				// "Warehouse" location has a typo, we fix it
				// see bug #6931
				line = " Almacen";
			}
		}

		_locationNames.push_back(line);
	}

	delete txtStream;

	// Load the path data
	Common::SeekableReadStream *pathStream = _vm->_res->load("chess.pth");

	// Get routes between different locations on the map
	_paths.load(31, *pathStream);

	// Load in the co-ordinates that the paths refer to
	_pathPoints.resize(208);
	for (uint idx = 0; idx < _pathPoints.size(); ++idx) {
		_pathPoints[idx].x = pathStream->readSint16LE();
		_pathPoints[idx].y = pathStream->readSint16LE();
	}

	delete pathStream;
}

int ScalpelMap::show() {
	Debugger &debugger = *_vm->_debugger;
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	bool changed = false, exitFlag = false;
	_active = true;

	// Set font and custom cursor for the map
	int oldFont = screen.fontNumber();
	screen.setFont(0);

	// Initial screen clear
	screen._backBuffer1.clear();
	screen.clear();

	// Load the entire map
	ImageFile *bigMap = NULL;
	if (!IS_3DO) {
		// PC
		bigMap = new ImageFile("bigmap.vgs");
		screen.setPalette(bigMap->_palette);
	} else {
		// 3DO
		bigMap = new ImageFile3DO("overland.cel", kImageFile3DOType_Cel);
	}

	// Load need sprites
	setupSprites();

	if (!IS_3DO) {
		screen._backBuffer1.SHblitFrom((*bigMap)[0], Common::Point(-_bigPos.x, -_bigPos.y));
		screen._backBuffer1.SHblitFrom((*bigMap)[1], Common::Point(-_bigPos.x, SHERLOCK_SCREEN_HEIGHT - _bigPos.y));
		screen._backBuffer1.SHblitFrom((*bigMap)[2], Common::Point(SHERLOCK_SCREEN_WIDTH - _bigPos.x, -_bigPos.y));
		screen._backBuffer1.SHblitFrom((*bigMap)[3], Common::Point(SHERLOCK_SCREEN_WIDTH - _bigPos.x, SHERLOCK_SCREEN_HEIGHT - _bigPos.y));
	} else {
		screen._backBuffer1.SHblitFrom((*bigMap)[0], Common::Point(-_bigPos.x, -_bigPos.y));
		screen.SHblitFrom((*bigMap)[0], Common::Point(-_bigPos.x, -_bigPos.y));
	}

	_drawMap = true;
	_charPoint = -1;
	_point = -1;
	people[HOLMES]._position = _lDrawnPos = _overPos;

	// Show place icons
	showPlaces();
	saveTopLine();
	_placesShown = true;

	// Keep looping until either a location is picked, or the game is ended
	while (!_vm->shouldQuit() && !exitFlag) {
		events.pollEventsAndWait();
		events.setButtonState();

		if (debugger._showAllLocations == LOC_REFRESH) {
			showPlaces();
			screen.slamArea(screen._currentScroll.x, screen._currentScroll.y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_WIDTH);
		}

		// Keyboard handling
		if (events.kbHit()) {
			Common::KeyState keyState = events.getKey();

			if (keyState.keycode == Common::KEYCODE_RETURN || keyState.keycode == Common::KEYCODE_SPACE) {
				// Both space and enter simulate a mouse release
				events._pressed = false;
				events._released = true;
				events._oldButtons = 0;
			}
		}

		// Ignore scrolling attempts until the screen is drawn
		if (!_drawMap) {
			Common::Point pt = events.mousePos();

			// Check for vertical map scrolling
			if ((pt.y > (SHERLOCK_SCREEN_HEIGHT - 10) && _bigPos.y < 200) || (pt.y < 10 && _bigPos.y > 0)) {
				if (pt.y > (SHERLOCK_SCREEN_HEIGHT - 10))
					_bigPos.y += 10;
				else
					_bigPos.y -= 10;

				changed = true;
			}

			// Check for horizontal map scrolling
			if ((pt.x > (SHERLOCK_SCREEN_WIDTH - 10) && _bigPos.x < 315) || (pt.x < 10 && _bigPos.x > 0)) {
				if (pt.x > (SHERLOCK_SCREEN_WIDTH - 10))
					_bigPos.x += 15;
				else
					_bigPos.x -= 15;

				changed = true;
			}
		}

		if (changed) {
			// Map has scrolled, so redraw new map view
			changed = false;

			if (!IS_3DO) {
				screen._backBuffer1.SHblitFrom((*bigMap)[0], Common::Point(-_bigPos.x, -_bigPos.y));
				screen._backBuffer1.SHblitFrom((*bigMap)[1], Common::Point(-_bigPos.x, SHERLOCK_SCREEN_HEIGHT - _bigPos.y));
				screen._backBuffer1.SHblitFrom((*bigMap)[2], Common::Point(SHERLOCK_SCREEN_WIDTH - _bigPos.x, -_bigPos.y));
				screen._backBuffer1.SHblitFrom((*bigMap)[3], Common::Point(SHERLOCK_SCREEN_WIDTH - _bigPos.x, SHERLOCK_SCREEN_HEIGHT - _bigPos.y));
			} else {
				screen._backBuffer1.SHblitFrom((*bigMap)[0], Common::Point(-_bigPos.x, -_bigPos.y));
			}

			showPlaces();
			_placesShown = false;

			saveTopLine();
			_savedPos.x = -1;
			updateMap(true);
		} else if (!_drawMap) {
			if (!_placesShown) {
				showPlaces();
				_placesShown = true;
			}

			if (_cursorIndex == 0) {
				Common::Point pt = events.mousePos();
				highlightIcon(Common::Point(pt.x - 4 + _bigPos.x, pt.y + _bigPos.y));
			}
			updateMap(false);
		}

		if ((events._released || events._rightReleased) && _point != -1) {
			if (people[HOLMES]._walkCount == 0) {
				people[HOLMES]._walkDest = _points[_point] + Common::Point(4, 9);
				_charPoint = _point;

				// Start walking to selected location
				walkTheStreets();

				// Show wait cursor
				_cursorIndex = 1;
				events.setCursor((*_mapCursors)[_cursorIndex]._frame);
			}
		}

		// Check if a scene has beeen selected and we've finished "moving" to it
		if (people[HOLMES]._walkCount == 0) {
			if (_charPoint >= 1 && _charPoint < (int)_points.size())
				exitFlag = true;
		}

		if (_drawMap) {
			_drawMap = false;

			if (screen._fadeStyle)
				screen.randomTransition();
			else
				screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
		}

		// Wait for a frame
		events.wait(1);
	}

	freeSprites();
	_overPos = people[HOLMES]._position;

	// Reset font
	screen.setFont(oldFont);

	// Free map graphic
	delete bigMap;

	_active = false;
	return _charPoint;
}

void ScalpelMap::setupSprites() {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	_savedPos.x = -1;

	if (!IS_3DO) {
		// PC
		_mapCursors = new ImageFile("omouse.vgs");
		_shapes = new ImageFile("mapicon.vgs");
		_iconShapes = new ImageFile("overicon.vgs");
	} else {
		// 3DO
		_mapCursors = new ImageFile3DO("omouse.vgs", kImageFile3DOType_RoomFormat);
		_shapes = new ImageFile3DO("mapicon.vgs", kImageFile3DOType_RoomFormat);
		_iconShapes = new ImageFile3DO("overicon.vgs", kImageFile3DOType_RoomFormat);
	}

	_cursorIndex = 0;
	events.setCursor((*_mapCursors)[_cursorIndex]._frame);

	_iconSave.create((*_shapes)[4]._width, (*_shapes)[4]._height, g_system->getScreenFormat());
	Person &p = people[HOLMES];
	p._description = " ";
	p._type = CHARACTER;
	p._position = Common::Point(12400, 5000);
	p._sequenceNumber = 0;
	p._images = _shapes;
	p._imageFrame = nullptr;
	p._frameNumber = 0;
	p._delta = Common::Point(0, 0);
	p._oldSize = Common::Point(0, 0);
	p._oldSize = Common::Point(0, 0);
	p._misc = 0;
	p._walkCount = 0;
	p._allow = 0;
	p._noShapeSize = Common::Point(0, 0);
	p._goto = Common::Point(28000, 15000);
	p._status = 0;
	p._walkSequences = _walkSequences;
	p.setImageFrame();
	scene._bgShapes.clear();
}

void ScalpelMap::freeSprites() {
	delete _mapCursors;
	delete _shapes;
	delete _iconShapes;
}

void ScalpelMap::showPlaces() {
	Debugger &debugger = *_vm->_debugger;
	Screen &screen = *_vm->_screen;

	for (uint idx = 0; idx < _points.size(); ++idx) {
		const MapEntry &pt = _points[idx];

		if (pt.x != 0 && pt.y != 0) {
			if (debugger._showAllLocations != LOC_DISABLED)
				_vm->setFlagsDirect(idx);

			if (pt.x >= _bigPos.x && (pt.x - _bigPos.x) < SHERLOCK_SCREEN_WIDTH
					&& pt.y >= _bigPos.y && (pt.y - _bigPos.y) < SHERLOCK_SCREEN_HEIGHT) {
				if (_vm->readFlags(idx)) {
					screen._backBuffer1.SHtransBlitFrom((*_iconShapes)[pt._translate],
						Common::Point(pt.x - _bigPos.x - 6, pt.y - _bigPos.y - 12));
				}
			}
		}
	}

	if (debugger._showAllLocations == LOC_REFRESH)
		debugger._showAllLocations = LOC_ALL;
}

void ScalpelMap::saveTopLine() {
	_topLine.SHblitFrom(_vm->_screen->_backBuffer1, Common::Point(0, 0), Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, 12));
}

void ScalpelMap::eraseTopLine() {
	Screen &screen = *_vm->_screen;
	screen._backBuffer1.SHblitFrom(_topLine, Common::Point(0, 0));
	screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, _topLine.height());
}

void ScalpelMap::showPlaceName(int idx, bool highlighted) {
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;

	Common::String name = _locationNames[idx];
	int width = screen.stringWidth(name);

	if (!_cursorIndex) {
		saveIcon(people[HOLMES]._imageFrame, _lDrawnPos);

		bool flipped = people[HOLMES]._sequenceNumber == MAP_DOWNLEFT || people[HOLMES]._sequenceNumber == MAP_LEFT
			|| people[HOLMES]._sequenceNumber == MAP_UPLEFT;
		screen._backBuffer1.SHtransBlitFrom(*people[HOLMES]._imageFrame, _lDrawnPos, flipped);
	}

	if (highlighted) {
		int xp = (SHERLOCK_SCREEN_WIDTH - screen.stringWidth(name)) / 2;
		screen.gPrint(Common::Point(xp + 2, 2), BLACK, "%s", name.c_str());
		screen.gPrint(Common::Point(xp + 1, 1), BLACK, "%s", name.c_str());
		screen.gPrint(Common::Point(xp, 0), 12, "%s", name.c_str());

		screen.slamArea(xp, 0, width + 2, 15);
	}
}

void ScalpelMap::updateMap(bool flushScreen) {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	Common::Point osPos = _savedPos;
	Common::Point osSize = _savedSize;
	Common::Point hPos;

	if (_cursorIndex >= 1) {
		if (++_cursorIndex > (1 + 8))
			_cursorIndex = 1;

		events.setCursor((*_mapCursors)[(_cursorIndex + 1) / 2]._frame);
	}

	if (!_drawMap && !flushScreen)
		restoreIcon();
	else
		_savedPos.x = -1;

	people[HOLMES].adjustSprite();

	_lDrawnPos.x = hPos.x = people[HOLMES]._position.x / FIXED_INT_MULTIPLIER - _bigPos.x;
	_lDrawnPos.y = hPos.y = people[HOLMES]._position.y / FIXED_INT_MULTIPLIER - people[HOLMES].frameHeight() - _bigPos.y;

	// Draw the person icon
	saveIcon(people[HOLMES]._imageFrame, hPos);
	if (people[HOLMES]._sequenceNumber == MAP_DOWNLEFT || people[HOLMES]._sequenceNumber == MAP_LEFT
			|| people[HOLMES]._sequenceNumber == MAP_UPLEFT)
		screen._backBuffer1.SHtransBlitFrom(*people[HOLMES]._imageFrame, hPos, true);
	else
		screen._backBuffer1.SHtransBlitFrom(*people[HOLMES]._imageFrame, hPos, false);

	if (flushScreen) {
		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
	} else if (!_drawMap) {
		if (hPos.x > 0 && hPos.y >= 0 && hPos.x < SHERLOCK_SCREEN_WIDTH && hPos.y < SHERLOCK_SCREEN_HEIGHT)
			screen.flushImage(people[HOLMES]._imageFrame, Common::Point(people[HOLMES]._position.x / FIXED_INT_MULTIPLIER - _bigPos.x,
			people[HOLMES]._position.y / FIXED_INT_MULTIPLIER - people[HOLMES].frameHeight() - _bigPos.y),
			&people[HOLMES]._oldPosition.x, &people[HOLMES]._oldPosition.y, &people[HOLMES]._oldSize.x, &people[HOLMES]._oldSize.y);

		if (osPos.x != -1)
			screen.slamArea(osPos.x, osPos.y, osSize.x, osSize.y);
	}
}

void ScalpelMap::walkTheStreets() {
	People &people = *_vm->_people;
	Common::Array<Common::Point> tempPath;

	// Get indexes into the path lists for the start and destination scenes
	int start = _points[_oldCharPoint]._translate;
	int dest = _points[_charPoint]._translate;

	// Get pointer to start of path
	const byte *path = _paths.getPath(start, dest);

	// Add in destination position
	people[HOLMES]._walkTo.clear();
	Common::Point destPos = people[HOLMES]._walkDest;

	// Check for any intermediate points between the two locations
	if (path[0] || _charPoint > 50 || _oldCharPoint > 50) {
		people[HOLMES]._sequenceNumber = -1;

		if (_charPoint == 51 || _oldCharPoint == 51) {
			people[HOLMES].setWalking();
		} else {
			bool reversePath = false;

			// Check for moving the path backwards or forwards
			if (path[0] == 255) {
				reversePath = true;
				SWAP(start, dest);
				path = _paths.getPath(start, dest);
			}

			do {
				int idx = *path++;
				tempPath.push_back(_pathPoints[idx - 1] + Common::Point(4, 4));
			} while (*path != 254);

			// Load up the path to use
			people[HOLMES]._walkTo.clear();

			if (reversePath) {
				for (int idx = (int)tempPath.size() - 1; idx >= 0; --idx)
					people[HOLMES]._walkTo.push(tempPath[idx]);
			} else {
				for (int idx = 0; idx < (int)tempPath.size(); ++idx)
					people[HOLMES]._walkTo.push(tempPath[idx]);
			}

			people[HOLMES]._walkDest = people[HOLMES]._walkTo.pop() + Common::Point(12, 6);
			people[HOLMES].setWalking();
		}
	} else {
		people[HOLMES]._walkCount = 0;
	}

	// Store the final destination icon position
	people[HOLMES]._walkTo.push(destPos);
}

void ScalpelMap::saveIcon(ImageFrame *src, const Common::Point &pt) {
	Screen &screen = *_vm->_screen;
	Common::Point size(src->_width, src->_height);
	Common::Point pos = pt;

	if (pos.x < 0) {
		size.x += pos.x;
		pos.x = 0;
	}

	if (pos.y < 0) {
		size.y += pos.y;
		pos.y = 0;
	}

	if ((pos.x + size.x) > SHERLOCK_SCREEN_WIDTH)
		size.x -= (pos.x + size.x) - SHERLOCK_SCREEN_WIDTH;

	if ((pos.y + size.y) > SHERLOCK_SCREEN_HEIGHT)
		size.y -= (pos.y + size.y) - SHERLOCK_SCREEN_HEIGHT;

	if (size.x < 1 || size.y < 1 || pos.x >= SHERLOCK_SCREEN_WIDTH || pos.y >= SHERLOCK_SCREEN_HEIGHT || _drawMap) {
		// Flag as the area not needing to be saved
		_savedPos.x = -1;
		return;
	}

	assert(size.x <= _iconSave.width() && size.y <= _iconSave.height());
	_iconSave.SHblitFrom(screen._backBuffer1, Common::Point(0, 0),
		Common::Rect(pos.x, pos.y, pos.x + size.x, pos.y + size.y));
	_savedPos = pos;
	_savedSize = size;
}

void ScalpelMap::restoreIcon() {
	Screen &screen = *_vm->_screen;

	if (_savedPos.x >= 0 && _savedPos.y >= 0 && _savedPos.x <= SHERLOCK_SCREEN_WIDTH
			&& _savedPos.y < SHERLOCK_SCREEN_HEIGHT)
		screen._backBuffer1.SHblitFrom(_iconSave, _savedPos, Common::Rect(0, 0, _savedSize.x, _savedSize.y));
}

void ScalpelMap::highlightIcon(const Common::Point &pt) {
	int oldPoint = _point;

	// Iterate through the icon list
	bool done = false;
	for (int idx = 0; idx < (int)_points.size(); ++idx) {
		const MapEntry &entry = _points[idx];

		// Check whether the mouse is over a given icon
		if (entry.x != 0 && entry.y != 0) {
			if (Common::Rect(entry.x - 8, entry.y - 8, entry.x + 9, entry.y + 9).contains(pt)) {
				done = true;

				if (_point != idx && _vm->readFlags(idx)) {
					// Changed to a new valid (visible) location
					eraseTopLine();
					showPlaceName(idx, true);
					_point = idx;
				}
			}
		}
	}

	if (!done) {
		// No icon was highlighted
		if (_point != -1) {
			// No longer highlighting previously highlighted icon, so erase it
			showPlaceName(_point, false);
			eraseTopLine();
		}

		_point = -1;
	} else if (oldPoint != -1 && oldPoint != _point) {
		showPlaceName(oldPoint, false);
		eraseTopLine();
	}
}

} // End of namespace Scalpel

} // End of namespace Sherlock
