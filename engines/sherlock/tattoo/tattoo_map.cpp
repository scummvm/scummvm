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
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Tattoo {

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
	// TODO
	return 61;
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
}

void TattooMap::drwaMapIcons() {
	Screen &screen = *_vm->_screen;
	
	for (uint idx = 0; idx < _data.size(); ++idx) {
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
	Common::Point mousePos = events.mousePos();
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

			_textBuffer->fillRect(Common::Rect(0, 0, width, height), 255);
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

			// Set the position of the Text Tag
			// TODO: take current scroll into account
			int xp = (mousePos.x - width / 2);
			int yp = (mousePos.y - height / 2);

			_textBounds = Common::Rect(xp, yp, xp + width, yp + height);
		} else if (_bgFound == -1 && _oldBgFound != -1) {
			// We need to clear a currently displayed name
			delete _textBuffer;
		}
	
		_oldBgFound = _bgFound;
	} else {
		// Set the new text position
		// TODO: take current scroll into account
		int xp = (mousePos.x - _textBounds.width() / 2);
		int yp = (mousePos.y - _textBounds.height() / 2);

		_textBounds.moveTo(xp, yp);
	}

	// If the text tag was displayed, restore the graphics underneath it
	if (_oldTextBounds.width() > 0)
		screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(_oldTextBounds.left, _oldTextBounds.top), _oldTextBounds);

	// See if we need to draw a Text Tag floating with the cursor
	if (_textBuffer != nullptr)
		screen.transBlitFrom(*_textBuffer, Common::Point(_textBounds.left, _textBounds.top));

	// See if we need to flush the areas associated with the text
	if (_oldTextBounds.width() > 0) {
		if (slamIt)
			// TODO: Take into account scroll
			screen.slamRect(_oldTextBounds);

		// If there's no text to display, reset the tag and old tag bounds
		if (_textBuffer == nullptr) {
			_textBounds.left = _textBounds.top = _textBounds.right = _textBounds.bottom = 0;
			_oldTextBounds.left = _oldTextBounds.top = _oldTextBounds.right = _oldTextBounds.bottom = 0;
		}
	}

	// If there's a text to display, then copy the drawn area to the screen
	if (_textBuffer != nullptr && slamIt)
		// TODO: Handle scroll
		screen.slamRect(_textBounds);
}

void TattooMap::restoreArea(const Common::Rect &bounds) {
	Screen &screen = *_vm->_screen;

	Common::Rect r = bounds;
	r.clip(Common::Rect(0, 0, screen._backBuffer1.w(), screen._backBuffer1.h()));

	if (!r.isEmpty())
		screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(r.left, r.top), r);
}

void TattooMap::showCloseUp(int closeUpNum) {
	Screen &screen = *_vm->_screen;

	// Get the closeup images
	Common::String fname = Common::String::format("res%02d.vgs", closeUpNum + 1);
	ImageFile pic(fname);

	Common::Point closeUp(_data[closeUpNum].x * 100, _data[closeUpNum].y * 100);
	Common::Point delta((SHERLOCK_SCREEN_WIDTH / 2 - closeUp.x / 100) * 100 / CLOSEUP_STEPS,
		(SHERLOCK_SCREEN_HEIGHT / 2 - closeUp.y / 100) * 100 / CLOSEUP_STEPS);
	Common::Rect oldBounds(closeUp.x / 100, closeUp.y / 100, closeUp.x / 100 + 1, closeUp.y / 100 + 1);
	int size = 64;
	int n = 256;
	int deltaVal = 512;
	bool minimize = false;
	int scaleVal, newSize;

	do
	{
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

		// TODO: handle scrolling
		screen.slamRect(_oldTextBounds);
		screen.slamArea(pt.x, pt.y, picSize.x, picSize.y);

		oldBounds = Common::Rect(pt.x, pt.y, pt.x + picSize.x + 1, pt.y + picSize.y + 1);
		closeUp += delta;
		scaleVal += deltaScale;
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

} // End of namespace Tattoo

} // End of namespace Sherlock
