/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sherlock/tattoo/tattoo_map.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

#define MAP_NAME_COLOR 131
#define CLOSEUP_STEPS 30
#define SCROLL_SPEED 16

/*-------------------------------------------------------------------------*/

void MapEntry::clear() {
	_iconNum = -1;
	_description = "";
}

/*-------------------------------------------------------------------------*/

TattooMap::TattooMap(SherlockEngine *vm) : Map(vm), _mapTooltip(vm) {
	_iconImages = nullptr;
	_bgFound = _oldBgFound = 0;

	loadData();
}

int TattooMap::show() {
	Debugger &debugger = *_vm->_debugger;
	Events &events = *_vm->_events;
	Music &music = *_vm->_music;
	Resources &res = *_vm->_res;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	Screen &screen = *_vm->_screen;
	int result = 0;

	// Check if we need to keep track of how many times player has been to the map
	for (uint idx = 0; idx < scene._sceneTripCounters.size(); ++idx) {
		SceneTripEntry &entry = scene._sceneTripCounters[idx];

		if (entry._sceneNumber == OVERHEAD_MAP || entry._sceneNumber == OVERHEAD_MAP2) {
			if (--entry._numTimes == 0) {
				_vm->setFlagsDirect(entry._flag);
				scene._sceneTripCounters.remove_at(idx);
			}
		}
	}

	if (music._musicOn) {
		// See if Holmes or Watson is the active character
		Common::String song;
		if (_vm->readFlags(FLAG_PLAYER_IS_HOLMES))
			// Player is Holmes
			song = "Cue9";
		else if (_vm->readFlags(FLAG_ALT_MAP_MUSIC))
			song = "Cue8";
		else
			song = "Cue7";

		if (music.loadSong(song)) {
			music.startSong();
		}
	}

	screen.initPaletteFade(1364485);

	// Load the custom mouse cursors for the map
	ImageFile cursors("omouse.vgs");
	events.setCursor(cursors[0]._frame);
	events.warpMouse();

	// Load the data for the map
	_iconImages = new ImageFile("mapicons.vgs");
	loadData();

	// Load the palette
	Common::SeekableReadStream *stream = res.load("map.pal");
	stream->read(screen._cMap, PALETTE_SIZE);
	screen.translatePalette(screen._cMap);
	delete stream;

	// Load the map image and draw it to the back buffer
	ImageFile *map = new ImageFile("map.vgs");
	screen._backBuffer1.create(SHERLOCK_SCREEN_WIDTH * 2, SHERLOCK_SCREEN_HEIGHT * 2);
	screen._backBuffer1.SHblitFrom((*map)[0], Common::Point(0, 0));
	screen.activateBackBuffer1();
	delete map;

	screen.clear();
	screen.setPalette(screen._cMap);
	drawMapIcons();

	// Copy the map drawn in the back buffer to the secondary back buffer
	screen._backBuffer2.create(SHERLOCK_SCREEN_WIDTH * 2, SHERLOCK_SCREEN_HEIGHT * 2);
	screen._backBuffer2.SHblitFrom(screen._backBuffer1);

	// Set initial scroll position, forcing the map to be displayed
	_targetScroll = _bigPos;
	screen._currentScroll = Common::Point(-1, -1);

	do {
		// Allow for event processing and get the current mouse position
		events.pollEventsAndWait();
		events.setButtonState();
		Common::Point mousePos = events.screenMousePos();

		if (debugger._showAllLocations == LOC_REFRESH) {
			drawMapIcons();
			screen.slamArea(screen._currentScroll.x, screen._currentScroll.y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_WIDTH);
		}

		music.checkSongProgress();
		checkMapNames(true);

		if (mousePos.x < (SHERLOCK_SCREEN_WIDTH / 6))
			_targetScroll.x -= 2 * SCROLL_SPEED * (SHERLOCK_SCREEN_WIDTH / 6 - mousePos.x) / (SHERLOCK_SCREEN_WIDTH / 6);
		if (mousePos.x > (SHERLOCK_SCREEN_WIDTH * 5 / 6))
			_targetScroll.x += 2 * SCROLL_SPEED * (mousePos.x - (SHERLOCK_SCREEN_WIDTH * 5 / 6)) / (SHERLOCK_SCREEN_WIDTH / 6);
		if (mousePos.y < (SHERLOCK_SCREEN_HEIGHT / 6))
			_targetScroll.y -= 2 * SCROLL_SPEED * (SHERLOCK_SCREEN_HEIGHT / 6 - mousePos.y) / (SHERLOCK_SCREEN_HEIGHT / 6);
		if (mousePos.y > (SHERLOCK_SCREEN_HEIGHT * 5 / 6))
			_targetScroll.y += 2 * SCROLL_SPEED * (mousePos.y - SHERLOCK_SCREEN_HEIGHT * 5 / 6) / (SHERLOCK_SCREEN_HEIGHT / 6);

		if (_targetScroll.x < 0)
			_targetScroll.x = 0;
		if ((_targetScroll.x + SHERLOCK_SCREEN_WIDTH) > screen._backBuffer1.width())
			_targetScroll.x = screen._backBuffer1.width() - SHERLOCK_SCREEN_WIDTH;
		if (_targetScroll.y < 0)
			_targetScroll.y = 0;
		if ((_targetScroll.y + SHERLOCK_SCREEN_HEIGHT) > screen._backBuffer1.height())
			_targetScroll.y = screen._backBuffer1.height() - SHERLOCK_SCREEN_HEIGHT;

		// Check the keyboard
		if (events.kbHit()) {
			Common::KeyState keyState = events.getKey();

			switch (keyState.keycode) {
			case Common::KEYCODE_HOME:
				_targetScroll.x = 0;
				_targetScroll.y = 0;
				break;

			case Common::KEYCODE_END:
				_targetScroll.x = screen._backBuffer1.width() - SHERLOCK_SCREEN_WIDTH;
				_targetScroll.y = screen._backBuffer1.height() - SHERLOCK_SCREEN_HEIGHT;
				break;

			case Common::KEYCODE_PAGEUP:
				_targetScroll.y -= SHERLOCK_SCREEN_HEIGHT;
				if (_targetScroll.y < 0)
					_targetScroll.y = 0;
				break;

			case Common::KEYCODE_PAGEDOWN:
				_targetScroll.y += SHERLOCK_SCREEN_HEIGHT;
				if (_targetScroll.y > (screen._backBuffer1.height() - SHERLOCK_SCREEN_HEIGHT))
					_targetScroll.y = screen._backBuffer1.height() - SHERLOCK_SCREEN_HEIGHT;
				break;

			case Common::KEYCODE_SPACE:
				events._pressed = false;
				events._oldButtons = 0;
				events._released = true;
				break;

			default:
				break;
			}
		}

		// Handle any scrolling of the map
		if (screen._currentScroll != _targetScroll) {
			// If there is a Text description being displayed, restore the area under it
			_mapTooltip.erase();

			screen._currentScroll = _targetScroll;

			checkMapNames(false);
			screen.slamArea(_targetScroll.x, _targetScroll.y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
		}

		// Handling if a location has been clicked on
		if (events._released && _bgFound != -1) {
			// If there is a Text description being displayed, restore the area under it
			_mapTooltip.erase();

			// Save the current scroll position on the map
			_bigPos = screen._currentScroll;

			showCloseUp(_bgFound);
			result = _bgFound + 1;
		}
	} while (!result && !_vm->shouldQuit());

	music.stopMusic();
	events.clearEvents();
	_mapTooltip.banishWindow();

	// Reset the back buffers back to standard size
	screen._backBuffer1.create(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
	screen._backBuffer2.create(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
	screen.activateBackBuffer1();

	return result;
}

void TattooMap::loadData() {
	Resources &res = *_vm->_res;
	char c;

	Common::SeekableReadStream *stream = res.load("map.txt");

	_data.resize(100);
	for (uint idx = 0; idx < _data.size(); ++idx)
		_data[idx].clear();

	do
	{
		// Find the start of the number
		do {
			c = stream->readByte();
			if (stream->pos() >= stream->size())
				break;
		} while (c < '0' || c > '9');
		if (stream->pos() >= stream->size())
			break;

		// Get the scene number
		Common::String locStr;
		locStr += c;
		while ((c = stream->readByte()) != '.')
			locStr += c;
		MapEntry &mapEntry = _data[atoi(locStr.c_str()) - 1];

		// Get the location name
		while (stream->readByte() != '"')
			;

		while ((c = stream->readByte()) != '"')
			mapEntry._description += c;

		// Find the ( specifying the (X,Y) position of the Icon
		while (stream->readByte() != '(')
			;

		// Get the X Position of the icon
		Common::String numStr;
		while ((c = stream->readByte()) != ',')
			numStr += c;
		mapEntry.x = atoi(numStr.c_str());

		// Get the Y position of the icon
		numStr = "";
		while ((c = stream->readByte()) != ')')
			numStr += c;
		mapEntry.y = atoi(numStr.c_str());

		// Find and get the location's icon number
		while (stream->readByte() != '#')
			;

		Common::String iconStr;
		while (stream->pos() < stream->size() && (c = stream->readByte()) != '\r')
			iconStr += c;

		mapEntry._iconNum = atoi(iconStr.c_str()) - 1;
	} while (stream->pos() < stream->size());

	delete stream;
}

void TattooMap::drawMapIcons() {
	Debugger &debugger = *_vm->_debugger;
	Screen &screen = *_vm->_screen;

	for (uint idx = 0; idx < _data.size(); ++idx) {
		if (debugger._showAllLocations != LOC_DISABLED)
			_vm->setFlagsDirect(idx + 1);

		if (_data[idx]._iconNum != -1 && _vm->readFlags(idx + 1)) {
			MapEntry &mapEntry = _data[idx];
			ImageFrame &img = (*_iconImages)[mapEntry._iconNum];
			screen._backBuffer1.SHtransBlitFrom(img._frame, Common::Point(mapEntry.x - img._width / 2,
				mapEntry.y - img._height / 2));
		}
	}

	if (debugger._showAllLocations == LOC_REFRESH)
		debugger._showAllLocations = LOC_ALL;
}

void TattooMap::checkMapNames(bool slamIt) {
	Events &events = *_vm->_events;
	Common::Point mapPos = events.mousePos();

	// See if the mouse is pointing at any of the map locations
	_bgFound = -1;

	for (uint idx = 0; idx < _data.size(); ++idx) {
		if (_data[idx]._iconNum != -1 && _vm->readFlags(idx + 1)) {
			MapEntry &mapEntry = _data[idx];
			ImageFrame &img = (*_iconImages)[mapEntry._iconNum];
			Common::Rect r(mapEntry.x - img._width / 2, mapEntry.y - img._height / 2,
				mapEntry.x + img._width / 2, mapEntry.y + img._height / 2);

			if (r.contains(mapPos)) {
				_bgFound = idx;
				break;
			}
		}
	}

	// Handle updating the tooltip
	if (_bgFound != _oldBgFound) {
		if (_bgFound == -1) {
			_mapTooltip.setText("");
		} else {
			const Common::String &desc = _data[_bgFound]._description;
			_mapTooltip.setText(desc);
		}

		_oldBgFound = _bgFound;
	}

	_mapTooltip.handleEvents();
	if (slamIt)
		_mapTooltip.draw();
}

void TattooMap::restoreArea(const Common::Rect &bounds) {
	Screen &screen = *_vm->_screen;

	Common::Rect r = bounds;
	r.clip(Common::Rect(0, 0, screen._backBuffer1.width(), screen._backBuffer1.height()));

	if (!r.isEmpty())
		screen._backBuffer1.SHblitFrom(screen._backBuffer2, Common::Point(r.left, r.top), r);
}

void TattooMap::showCloseUp(int closeUpNum) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;

	// Hide the cursor
	events.hideCursor();

	// Get the closeup images
	Common::Path fname(Common::String::format("res%02d.vgs", closeUpNum + 1));
	ImageFile pic(fname);

	Point32 closeUp(_data[closeUpNum].x * 100, _data[closeUpNum].y * 100);
	Point32 delta((SHERLOCK_SCREEN_WIDTH / 2 - closeUp.x / 100) * 100 / CLOSEUP_STEPS,
		(SHERLOCK_SCREEN_HEIGHT / 2 - closeUp.y / 100) * 100 / CLOSEUP_STEPS);
	Common::Rect oldBounds(closeUp.x / 100, closeUp.y / 100, closeUp.x / 100 + 1, closeUp.y / 100 + 1);
	int size = 64;
	int n = 256;
	int deltaVal = 512;
	bool minimize = false;
	int scaleVal, newSize;

	do {
		scaleVal = n;
		newSize = pic[0].sDrawXSize(n);

		if (newSize > size) {
			if (minimize)
				deltaVal /= 2;
			n += deltaVal;
		} else {
			minimize = true;
			deltaVal /= 2;
			n -= deltaVal;
			if (n < 1)
				n = 1;
		}
	} while (deltaVal && size != newSize);

	int deltaScale = (SCALE_THRESHOLD - scaleVal) / CLOSEUP_STEPS;

	for (int step = 0; step < CLOSEUP_STEPS; ++step) {
		Common::Point picSize(pic[0].sDrawXSize(scaleVal), pic[0].sDrawYSize(scaleVal));
		Common::Point pt(screen._currentScroll.x + closeUp.x / 100 - picSize.x / 2,
			screen._currentScroll.y + closeUp.y / 100 - picSize.y / 2);

		restoreArea(oldBounds);
		screen._backBuffer1.SHtransBlitFrom(pic[0], pt, false, scaleVal);

		screen.slamRect(oldBounds);
		screen.slamArea(pt.x, pt.y, picSize.x, picSize.y);

		oldBounds = Common::Rect(pt.x, pt.y, pt.x + picSize.x + 1, pt.y + picSize.y + 1);
		closeUp += delta;
		scaleVal += deltaScale;

		events.wait(1);
	}

	// Handle final drawing of closeup
	Common::Rect r(screen._currentScroll.x + SHERLOCK_SCREEN_WIDTH / 2 - pic[0]._width / 2,
		screen._currentScroll.y + SHERLOCK_SCREEN_HEIGHT / 2 - pic[0]._height / 2,
		screen._currentScroll.x + SHERLOCK_SCREEN_WIDTH / 2 - pic[0]._width / 2 + pic[0]._width,
		screen._currentScroll.y + SHERLOCK_SCREEN_HEIGHT / 2 - pic[0]._height / 2 + pic[0]._height);

	restoreArea(oldBounds);
	screen._backBuffer1.SHtransBlitFrom(pic[0], Common::Point(r.left, r.top));
	screen.slamRect(oldBounds);
	screen.slamRect(r);

	events.wait(60);
	events.showCursor();
}

} // End of namespace Tattoo

} // End of namespace Sherlock
