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

TattooMap::TattooMap(SherlockEngine *vm) : Map(vm) {
	_iconImages = nullptr;
	_bgFound = _oldBgFound = 0;
	_textBuffer = nullptr;

	loadData();
}

int TattooMap::show() {
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

	if (music._midiOption) {
		// See if Holmes or Watson is the active character	
		Common::String song;
		if (_vm->readFlags(76))
			// Player is Holmes
			song = "Cue9";
		else if (_vm->readFlags(525))
			song = "Cue8";
		else
			song = "Cue7";

		if (music.loadSong(song)) {
			music.setMIDIVolume(music._musicVolume);
			if (music._musicOn)
				music.startSong();
		}
	}

	screen.initPaletteFade(1364485);
	
	// Load the custom mouse cursors for the map
	ImageFile cursors("omouse.vgs");
	events.setCursor(cursors[0]._frame);

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
	screen._backBuffer1.create(SHERLOCK_SCREEN_WIDTH * 2, SHERLOCK_SCREEN_HEIGHT * 2, _vm->getPlatform());
	screen._backBuffer1.blitFrom((*map)[0], Common::Point(0, 0));
	delete map;

	screen.clear();
	screen.setPalette(screen._cMap);
	drawMapIcons();

	// Copy the map drawn in the back buffer to the secondary back buffer
	screen._backBuffer2.create(SHERLOCK_SCREEN_WIDTH * 2, SHERLOCK_SCREEN_HEIGHT * 2, _vm->getPlatform());
	screen._backBuffer2.blitFrom(screen._backBuffer1);

	// Display the built map to the screen
	screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	// Set text display positioning and scroll position
	_oldTextBounds.left = _oldTextBounds.top = _oldTextBounds.right = _oldTextBounds.bottom = 0;
	_textBounds.left = _textBounds.top = _textBounds.right = _textBounds.bottom = 0;
	_targetScroll = _bigPos;
	_currentScroll = Common::Point(-1, -1);

	do {
		// Allow for event processing and get the current mouse position
		events.pollEventsAndWait();
		events.setButtonState();
		Common::Point mousePos = events.mousePos();

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
		if ((_targetScroll.x + SHERLOCK_SCREEN_WIDTH) > screen._backBuffer1.w())
			_targetScroll.x = screen._backBuffer1.w() - SHERLOCK_SCREEN_WIDTH;
		if (_targetScroll.y < 0)
			_targetScroll.y = 0;
		if ((_targetScroll.y + SHERLOCK_SCREEN_HEIGHT) > screen._backBuffer1.h())
			_targetScroll.y = screen._backBuffer1.h() - SHERLOCK_SCREEN_HEIGHT;

		// Check the keyboard
		if (events.kbHit()) {
			Common::KeyState keyState = events.getKey();

			switch (keyState.keycode) {
			case Common::KEYCODE_HOME:
			case Common::KEYCODE_KP7:
				_targetScroll.x = 0;
				_targetScroll.y = 0;
				break;

			case Common::KEYCODE_END:
			case Common::KEYCODE_KP1:
				_targetScroll.x = screen._backBuffer1.w() - SHERLOCK_SCREEN_WIDTH;
				_targetScroll.y = screen._backBuffer1.h() - SHERLOCK_SCREEN_HEIGHT;
				break;

			case Common::KEYCODE_PAGEUP:
			case Common::KEYCODE_KP9:
				_targetScroll.y -= SHERLOCK_SCREEN_HEIGHT;
				if (_targetScroll.y < 0)
					_targetScroll.y = 0;
				break;

			case Common::KEYCODE_PAGEDOWN:
			case Common::KEYCODE_KP3:
				_targetScroll.y += SHERLOCK_SCREEN_HEIGHT;
				if (_targetScroll.y > (screen._backBuffer1.h() - SHERLOCK_SCREEN_HEIGHT))
					_targetScroll.y = screen._backBuffer1.h() - SHERLOCK_SCREEN_HEIGHT;
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
		if (_currentScroll != _targetScroll) {
			// If there is a Text description being displayed, restore the area under it
			if (_oldTextBounds.width() > 0)
				screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(_oldTextBounds.left, 
					_oldTextBounds.top), _oldTextBounds);

			_currentScroll = _targetScroll;

			checkMapNames(false);
			slamRect(Common::Rect(_currentScroll.x, _currentScroll.y, _currentScroll.x + SHERLOCK_SCREEN_WIDTH, 
				_currentScroll.y + SHERLOCK_SCREEN_HEIGHT));
		}

		// Handling if a location has been clicked on
		if (events._released && _bgFound != -1) {
			// If there is a Text description being displayed, restore the area under it
			if (_oldTextBounds.width() > 0) {
				screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(_oldTextBounds.left,
					_oldTextBounds.top), _oldTextBounds);
				screen.slamRect(_oldTextBounds);
			}

			// Save the current scroll position on the map
			_bigPos = _currentScroll;

			showCloseUp(_bgFound);
			result = _bgFound + 1;
		}
	} while (!result && !_vm->shouldQuit());

	music.stopMusic();
	events.clearEvents();
	delete _textBuffer;
	_textBuffer = nullptr;

	// Reset the back buffers back to standard size
	screen._backBuffer1.create(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT, _vm->getPlatform());
	screen._backBuffer2.create(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT, _vm->getPlatform());

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
				return;
		} while (c < '0' || c > '9');

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
	Screen &screen = *_vm->_screen;
	
	for (uint idx = 0; idx < _data.size(); ++idx) {
		_vm->setFlagsDirect(idx + 1); //***DEBUG***
		if (_data[idx]._iconNum != -1 && _vm->readFlags(idx + 1)) {
			MapEntry &mapEntry = _data[idx];
			ImageFrame &img = (*_iconImages)[mapEntry._iconNum];
			screen._backBuffer1.transBlitFrom(img._frame, Common::Point(mapEntry.x - img._width / 2,
				mapEntry.y - img._height / 2));
		}
	}
}

void TattooMap::checkMapNames(bool slamIt) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Common::Point mousePos = events.mousePos() + _currentScroll;
	int dif = 10000;

	// See if the mouse is pointing at any of the map locations
	_bgFound = -1;

	for (uint idx = 0; idx < _data.size(); ++idx) {
		if (_data[idx]._iconNum != -1 && _vm->readFlags(idx + 1)) {
			MapEntry &mapEntry = _data[idx];
			ImageFrame &img = (*_iconImages)[mapEntry._iconNum];
			Common::Rect r(mapEntry.x - img._width / 2, mapEntry.y - img._height / 2,
				mapEntry.x + img._width / 2, mapEntry.y + img._height / 2);

			if (r.contains(mousePos)) {
				_bgFound = idx;
				break;
			}
		}
	}

	// Store the previous bounds that were drawn
	_oldTextBounds = _textBounds;

	// See if thay are pointing at a different location and we need to change the display
	if (_bgFound != _oldBgFound || (_bgFound != -1 && _textBuffer == nullptr)) {
		// See if there is a new image to be displayed
		if (_bgFound != -1 && (_bgFound != _oldBgFound || _textBuffer == nullptr)) {
			const Common::String &desc = _data[_bgFound]._description;
			const char *space = nullptr;
			int width = screen.stringWidth(desc) + 2;
			int height = 0;

			// See if we need to split it into two lines
			if (width > 150) {
				const char *s = desc.c_str();

				for (;;) {
					// Move to end of next word
					s = strchr(s, ' ');

					if (s == nullptr) {
						// Reached end of description
						if (space == nullptr) {
							height = screen.stringHeight(desc) + 2;
						} else {
							Common::String line1(desc.c_str(), space);
							Common::String line2(space + 1);

							height = screen.stringHeight(line1) + screen.stringHeight(line2);
						}
						break;
					} else {
						// Reached space separating words within the description
						// Get width before and after word
						int width1 = screen.stringWidth(Common::String(desc.c_str(), s));
						int width2 = screen.stringWidth(Common::String(s + 1));

						if (ABS(width1 - width2) < dif) {
							space = s;
							dif = ABS(width1 - width2);
							width = MAX(width1, width) + 2;
						}

						++s;
					}
				}
			} else {
				height = screen.stringHeight(desc) + 2;
			}

			// Delete any previous saved area
			delete _textBuffer;

			// Allocate a new surface
			_textBuffer = new Surface(width, height, _vm->getPlatform());

			_textBuffer->fillRect(Common::Rect(0, 0, width, height), TRANSPARENCY);
			if (space == nullptr) {
				// The whole text can be drawn on a single line
				_textBuffer->writeString(desc, Common::Point(0, 0), BLACK);
				_textBuffer->writeString(desc, Common::Point(1, 0), BLACK);
				_textBuffer->writeString(desc, Common::Point(2, 0), BLACK);
				_textBuffer->writeString(desc, Common::Point(0, 1), BLACK);
				_textBuffer->writeString(desc, Common::Point(2, 1), BLACK);
				_textBuffer->writeString(desc, Common::Point(0, 2), BLACK);
				_textBuffer->writeString(desc, Common::Point(1, 2), BLACK);
				_textBuffer->writeString(desc, Common::Point(2, 2), BLACK);
				_textBuffer->writeString(desc, Common::Point(1, 1), MAP_NAME_COLOR);
			} else {
				// The text needs to be split up over two lines
				Common::String line1(desc.c_str(), space);
				Common::String line2(space + 1);
				int xp, yp;

				// Draw the first line
				xp = (width - screen.stringWidth(desc)) / 2;
				_textBuffer->writeString(line1, Common::Point(xp + 0, 0), BLACK);
				_textBuffer->writeString(line1, Common::Point(xp + 1, 0), BLACK);
				_textBuffer->writeString(line1, Common::Point(xp + 2, 0), BLACK);
				_textBuffer->writeString(line1, Common::Point(xp + 0, 1), BLACK);
				_textBuffer->writeString(line1, Common::Point(xp + 2, 1), BLACK);
				_textBuffer->writeString(line1, Common::Point(xp + 0, 2), BLACK);
				_textBuffer->writeString(line1, Common::Point(xp + 1, 2), BLACK);
				_textBuffer->writeString(line1, Common::Point(xp + 2, 2), BLACK);
				_textBuffer->writeString(line1, Common::Point(xp + 1, 1), MAP_NAME_COLOR);

				yp = screen.stringHeight(line2);
				xp = (width - screen.stringWidth(line2)) / 2;
				_textBuffer->writeString(line2, Common::Point(xp + 0, 0), BLACK);
				_textBuffer->writeString(line2, Common::Point(xp + 1, 0), BLACK);
				_textBuffer->writeString(line2, Common::Point(xp + 2, 0), BLACK);
				_textBuffer->writeString(line2, Common::Point(xp + 0, 1), BLACK);
				_textBuffer->writeString(line2, Common::Point(xp + 2, 1), BLACK);
				_textBuffer->writeString(line2, Common::Point(xp + 0, 2), BLACK);
				_textBuffer->writeString(line2, Common::Point(xp + 1, 2), BLACK);
				_textBuffer->writeString(line2, Common::Point(xp + 2, 2), BLACK);
				_textBuffer->writeString(line2, Common::Point(xp + 1, 1), MAP_NAME_COLOR);
			}

			// Set the text display position
			setTextBounds();
		} else if (_bgFound == -1 && _oldBgFound != -1) {
			// We need to clear a currently displayed name
			delete _textBuffer;
			_textBuffer = nullptr;
		}
	
		_oldBgFound = _bgFound;
	} else {
		// Set the new text position
		setTextBounds();
	}

	// If the location name was displayed, restore the graphics underneath where it previously was
	if (_oldTextBounds.width() > 0)
		screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(_oldTextBounds.left, _oldTextBounds.top), _oldTextBounds);

	// See if we need to draw the currently highlighted location name
	if (_textBuffer != nullptr)
		screen._backBuffer1.transBlitFrom(*_textBuffer, Common::Point(_textBounds.left, _textBounds.top));

	// See if we need to flush the areas associated with the text
	if (_oldTextBounds.width() > 0) {
		if (slamIt)
			slamRect(_oldTextBounds);

		// If there's no text to display, reset the display bounds
		if (_textBuffer == nullptr) {
			_textBounds.left = _textBounds.top = _textBounds.right = _textBounds.bottom = 0;
			_oldTextBounds.left = _oldTextBounds.top = _oldTextBounds.right = _oldTextBounds.bottom = 0;
		}
	}

	// If there's text to display, then copy the drawn area to the screen
	if (_textBuffer != nullptr && slamIt)
		slamRect(_textBounds);
}

void TattooMap::setTextBounds() {
	Events &events = *_vm->_events;
	Common::Point mousePos = events.mousePos();

	if (_textBuffer == nullptr) {
		_textBounds = Common::Rect(0, 0, 0, 0);
	} else {
		int xp = (mousePos.x - _textBounds.width() / 2) + _currentScroll.x;
		int yp = (mousePos.y - _textBounds.height() / 2) + _currentScroll.y;
		if (xp < _currentScroll.x)
			xp = _currentScroll.x;
		if ((xp + _textBounds.width()) >(_currentScroll.x + SHERLOCK_SCREEN_WIDTH))
			xp = _currentScroll.x + SHERLOCK_SCREEN_WIDTH - _textBounds.width();
		if (yp < _currentScroll.y)
			yp = _currentScroll.y;
		if ((yp + _textBounds.height()) >(_currentScroll.y + SHERLOCK_SCREEN_HEIGHT))
			yp = _currentScroll.y + SHERLOCK_SCREEN_HEIGHT - _textBounds.height();

		_textBounds = Common::Rect(xp, yp, xp + _textBuffer->w(), yp + _textBuffer->h());
	}
}

void TattooMap::restoreArea(const Common::Rect &bounds) {
	Screen &screen = *_vm->_screen;

	Common::Rect r = bounds;
	r.clip(Common::Rect(0, 0, screen._backBuffer1.w(), screen._backBuffer1.h()));

	if (!r.isEmpty())
		screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(r.left, r.top), r);
}

void TattooMap::showCloseUp(int closeUpNum) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;

	// Get the closeup images
	Common::String fname = Common::String::format("res%02d.vgs", closeUpNum + 1);
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
		Common::Point pt(closeUp.x / 100 - picSize.x, closeUp.y / 100 - picSize.y);

		restoreArea(oldBounds);
		screen._backBuffer1.transBlitFrom(pic[0], pt, false, 0, scaleVal);

		screen.slamRect(oldBounds);
		screen.slamArea(pt.x, pt.y, picSize.x, picSize.y);

		oldBounds = Common::Rect(pt.x, pt.y, pt.x + picSize.x + 1, pt.y + picSize.y + 1);
		closeUp += delta;
		scaleVal += deltaScale;

		events.wait(1);
	}

	// Handle final drawing of closeup
	// TODO: Handle scrolling
	Common::Rect r(SHERLOCK_SCREEN_WIDTH / 2 - pic[0]._width / 2, SHERLOCK_SCREEN_HEIGHT / 2 - pic[0]._height / 2,
		SHERLOCK_SCREEN_WIDTH / 2 + pic[0]._width / 2 + pic[0]._width,
		SHERLOCK_SCREEN_HEIGHT / 2 + pic[0]._height / 2 + pic[0]._height);

	restoreArea(oldBounds);
	screen._backBuffer1.transBlitFrom(pic[0], Common::Point(r.left, r.top));
	screen.slamRect(oldBounds);
	screen.slamRect(r);
}

void TattooMap::slamRect(const Common::Rect &bounds) {
	Screen &screen = *_vm->_screen;
	Common::Rect r = bounds;
	r.translate(-_currentScroll.x, -_currentScroll.y);

	screen.blitFrom(screen._backBuffer1, Common::Point(r.left, r.top), bounds);
}

} // End of namespace Tattoo

} // End of namespace Sherlock
