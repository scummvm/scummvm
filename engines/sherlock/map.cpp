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

#include "sherlock/map.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

Map::Map(SherlockEngine *vm): _vm(vm), _topLine(SHERLOCK_SCREEN_WIDTH, 12) {
	_shapes = nullptr;
	_iconShapes = nullptr;
	_point = 0;
	_placesShown = false;
	_charPoint = -1;
	for (int idx = 0; idx < 3; ++idx)
		Common::fill(&_sequences[idx][0], &_sequences[idx][MAX_FRAME], 0);

	loadData();
}

/**
 * Loads the list of points for locations on the map for each scene
 */
void Map::loadPoints(int count, const int *xList, const int *yList) {
	for (int idx = 0; idx < count; ++idx, ++xList, ++yList) {
		_points.push_back(Common::Point(*xList, *yList));
	}
}

/**
 * Load data  needed for the map
 */
void Map::loadData() {
	// Load the list of location names
	Common::SeekableReadStream *txtStream = _vm->_res->load("chess.txt");
	char c;

	while (txtStream->pos() < txtStream->size()) {
		Common::String line;
		while ((c = txtStream->readByte()) != '\0')
			line += c;

		_locationNames.push_back(line);
	}

	delete txtStream;

	// Load the path data
	Common::SeekableReadStream *pathStream = _vm->_res->load("chess.pth");

	_paths.resize(31);
	for (uint idx = 0; idx < _paths.size(); ++idx) {
		_paths[idx].resize(_paths.size());

		for (uint idx2 = 0; idx2 < _paths.size(); ++idx2)
			_paths[idx][idx2] = pathStream->readSint16LE();
	}

	// Load in the path point information
	_pathPoints.resize(416);
	for (uint idx = 0; idx < _pathPoints.size(); ++idx)
		_pathPoints[idx] = pathStream->readSint16LE();

	delete pathStream;
}

/**
 * Show the map
 */
int Map::show() {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Common::Point lDrawn(-1, -1);
	bool changed = false, exitFlag = false;
	bool drawMap = true;

	// Set font and custom cursor for the map
	int oldFont = screen.fontNumber();
	screen.setFont(0);

	ImageFile mapCursors("omouse.vgs");
	events.setCursor(mapCursors[0]);

	// Load the entire map
	ImageFile bigMap("bigmap.vgs");

	// Load need sprites
	setupSprites();

	screen._backBuffer1.blitFrom(bigMap[1], Common::Point(-_bigPos.x, -_bigPos.y));
	screen._backBuffer1.blitFrom(bigMap[2], Common::Point(-_bigPos.x, SHERLOCK_SCREEN_HEIGHT - _bigPos.y));
	screen._backBuffer1.blitFrom(bigMap[3], Common::Point(SHERLOCK_SCREEN_WIDTH - _bigPos.x, -_bigPos.y));
	screen._backBuffer1.blitFrom(bigMap[4], Common::Point(SHERLOCK_SCREEN_WIDTH - _bigPos.x, SHERLOCK_SCREEN_HEIGHT - _bigPos.y));

	_point = -1;
	people[AL]._position = _lDrawnPos = _overPos;

	// Show place icons
	showPlaces();
	saveTopLine();
	_placesShown = true;

	// Keep looping until either a location is picked, or the game is ended
	while (!_vm->shouldQuit() && !exitFlag) {
		events.pollEventsAndWait();
		events.setButtonState();

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
		if (!drawMap) {
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

			screen._backBuffer1.blitFrom(bigMap[1], Common::Point(-_bigPos.x, -_bigPos.y));
			screen._backBuffer1.blitFrom(bigMap[2], Common::Point(-_bigPos.x, SHERLOCK_SCREEN_HEIGHT - _bigPos.y));
			screen._backBuffer1.blitFrom(bigMap[3], Common::Point(SHERLOCK_SCREEN_WIDTH - _bigPos.x, -_bigPos.y));
			screen._backBuffer1.blitFrom(bigMap[4], Common::Point(SHERLOCK_SCREEN_WIDTH - _bigPos.x, SHERLOCK_SCREEN_HEIGHT - _bigPos.y));

			showPlaces();
			_placesShown = false;

			saveTopLine();
			_savedPos.x = -1;
			updateMap(true);
		} else if (!drawMap) {
			if (!_placesShown) {
				showPlaces();
				_placesShown = true;
			}

			updateMap(false);
		}

		if ((events._released || events._rightReleased) && _point != -1) {
			if (people[AL]._walkCount == 0) {
				_charPoint = _point;
				walkTheStreets();

				events.setCursor(mapCursors[1]);
			}
		}

		// Check if a scene has beeen selected and we've finished "moving" to it
		if (people[AL]._walkCount == 0) {
			if (_charPoint >= 1 && _charPoint < (int)_points.size())
				exitFlag = true;
		}

		if (drawMap) {
			drawMap = false;

			if (screen._fadeStyle)
				screen.randomTransition();
			else
				screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
		}

		// Wait for a frame
		events.wait(1);
	}

	freeSprites();
	_overPos = people[AL]._position;

	// Reset font and cursor
	screen.setFont(oldFont);
	events.setCursor(ARROW);

	return _charPoint;
}

/**
 * Load and initialize all the sprites that are needed for the map display
 */
void Map::setupSprites() {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	typedef byte Sequences[16][MAX_FRAME];
	_savedPos.x = -1;

	_shapes = new ImageFile("mapicon.vgs");
	_iconShapes = new ImageFile("overicon.vgs");

	Person &p = people[AL];
	p._description = " ";
	p._type = CHARACTER;
	p._position = Common::Point(12400, 5000);
	p._sequenceNumber = 0;
	p._sequences = (Sequences *)&_sequences;
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
	p.setImageFrame();

	scene._bgShapes.clear();
}

/**
 * Free the sprites and data used by the map
 */
void Map::freeSprites() {
	delete _shapes;
	delete _iconShapes;
}

/**
 * Draws an icon for every place that's currently known
 */
void Map::showPlaces() {
	Screen &screen = *_vm->_screen;

	for (uint idx = 0; idx < _points.size(); ++idx) {
		const Common::Point &pt = _points[idx];

		if (pt.x != 0 && pt.y != 0) {
			if (pt.x >= _bigPos.x && (pt.x - _bigPos.x) < SHERLOCK_SCREEN_WIDTH
					&& pt.y >= _bigPos.y && (pt.y - _bigPos.y) < SHERLOCK_SCREEN_HEIGHT) {
				if (_vm->readFlags(idx)) {
					screen._backBuffer1.transBlitFrom((*_iconShapes)[idx], 
						Common::Point(pt.x - _bigPos.x - 6, pt.y - _bigPos.y - 12));
				}
			}
		}
	}
}

/**
 * Makes a copy of the top rows of the screen that are used to display location names
 */
void Map::saveTopLine() {
	_topLine.blitFrom(_vm->_screen->_backBuffer1, Common::Point(0, 0), Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH, 12));
}

/**
 * Update all on-screen sprites to account for any scrolling of the map
 */
void Map::updateMap(bool flushScreen) {
	// TODO
}

/**
 * Handle moving icon for player from their previous location on the map to a destination location
 */
void Map::walkTheStreets() {
	// TODO
}

} // End of namespace Sherlock
