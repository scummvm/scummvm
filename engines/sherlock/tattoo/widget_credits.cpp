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

#include "sherlock/tattoo/widget_credits.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

WidgetCredits::WidgetCredits(SherlockEngine *vm) : _vm(vm) {
	_creditSpeed = 4;
	_creditsActive = false;
}

void WidgetCredits::initCredits() {
	Resources &res = *_vm->_res;
	Screen &screen = *_vm->_screen;
	Common::SeekableReadStream *stream = res.load("credits.txt");
	int spacing = screen.fontHeight() * 2;
	int yp = screen.height();

	_creditsActive = true;
	_creditLines.clear();

	while (stream->pos() < stream->size()) {
		Common::String line = stream->readLine();

		if (line.hasPrefix("Scroll Speed")) {
			const char *p = line.c_str() + 12;
			while ((*p < '0') || (*p > '9'))
				p++;

			_creditSpeed = atoi(p);
		} else if (line.hasPrefix("Y Spacing")) {
			const char *p = line.c_str() + 12;
			while ((*p < '0') || (*p > '9'))
				p++;

			spacing = atoi(p) + screen.fontHeight() + 1;
		} else {
			int width = screen.stringWidth(line) + 2;

			_creditLines.push_back(CreditLine(line, Common::Point((screen.width() - width) / 2 + 1, yp), width));
			yp += spacing;
		}
	}

	// Post-processing for finding split lines
	for (int l = 0; l < (int)_creditLines.size(); ++l) {
		CreditLine &cl = _creditLines[l];
		const char *p = strchr(cl._line.c_str(), '-');

		if (p != nullptr && p[1] == '>') {
			cl._line2 = Common::String(p + 3);
			cl._line = Common::String(cl._line.c_str(), p);

			int width = cl._width;
			int width1 = screen.stringWidth(cl._line);
			int width2 = screen.stringWidth(cl._line2);

			int c = 1;
			for (int l1 = l + 1; l1 < (int)_creditLines.size(); ++l1) {
				if ((p = strchr(_creditLines[l1]._line.c_str(), '-')) != nullptr) {
					if (p[1] == '>') {
						Common::String line1 = Common::String(_creditLines[l1]._line.c_str(), p);
						Common::String line2 = Common::String(p + 3);

						width1 = MAX(width1, screen.stringWidth(line1));

						if (screen.stringWidth(line2) > width2)
							width2 = screen.stringWidth(line2);
						++c;
					} else {
						break;
					}
				} else {
					break;
				}
			}

			width = width1 + width2 + screen.widestChar();
			width1 += screen.widestChar();

			for (int l1 = l; l1 < l + c; ++l1) {
				_creditLines[l1]._width = width;
				_creditLines[l1]._xOffset = width1;
			}

			l += c - 1;
		}
	}

	delete stream;
}

void WidgetCredits::close() {
	_creditsActive = false;
	_creditLines.clear();
}

void WidgetCredits::drawCredits() {
	Screen &screen = *_vm->_screen;
	Common::Rect screenRect(0, 0, screen.width(), screen.height());
	Surface &bb1 = screen._backBuffer1;

	for (uint idx = 0; idx < _creditLines.size() && _creditLines[idx]._position.y < screen.height(); ++idx) {
		if (screenRect.contains(_creditLines[idx]._position)) {
			if (!_creditLines[idx]._line2.empty()) {
				int x1 = _creditLines[idx]._position.x;
				int x2 = x1 + _creditLines[idx]._xOffset;
				const Common::String &line1 = _creditLines[idx]._line;
				const Common::String &line2 = _creditLines[idx]._line2;

				bb1.writeString(line1, Common::Point(x1 - 1, _creditLines[idx]._position.y - 1), 0);
				bb1.writeString(line1, Common::Point(x1, _creditLines[idx]._position.y - 1), 0);
				bb1.writeString(line1, Common::Point(x1 + 1, _creditLines[idx]._position.y - 1), 0);

				bb1.writeString(line1, Common::Point(x1 - 1, _creditLines[idx]._position.y), 0);
				bb1.writeString(line1, Common::Point(x1 + 1, _creditLines[idx]._position.y), 0);

				bb1.writeString(line1, Common::Point(x1 - 1, _creditLines[idx]._position.y + 1), 0);
				bb1.writeString(line1, Common::Point(x1, _creditLines[idx]._position.y + 1), 0);
				bb1.writeString(line1, Common::Point(x1 + 1, _creditLines[idx]._position.y + 1), 0);

				bb1.writeString(line1, Common::Point(x1, _creditLines[idx]._position.y), INFO_TOP);

				bb1.writeString(line2, Common::Point(x2 - 1, _creditLines[idx]._position.y - 1), 0);
				bb1.writeString(line2, Common::Point(x2, _creditLines[idx]._position.y - 1), 0);
				bb1.writeString(line2, Common::Point(x2 + 1, _creditLines[idx]._position.y - 1), 0);

				bb1.writeString(line2, Common::Point(x2 - 1, _creditLines[idx]._position.y), 0);
				bb1.writeString(line2, Common::Point(x2 + 1, _creditLines[idx]._position.y), 0);

				bb1.writeString(line2, Common::Point(x2 - 1, _creditLines[idx]._position.y + 1), 0);
				bb1.writeString(line2, Common::Point(x2, _creditLines[idx]._position.y + 1), 0);
				bb1.writeString(line2, Common::Point(x2 + 1, _creditLines[idx]._position.y + 1), 0);

				bb1.writeString(line2, Common::Point(x2, _creditLines[idx]._position.y), INFO_TOP);
			} else {
				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x - 1, _creditLines[idx]._position.y - 1), 0);
				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x, _creditLines[idx]._position.y - 1), 0);
				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x + 1, _creditLines[idx]._position.y - 1), 0);

				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x - 1, _creditLines[idx]._position.y), 0);
				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x + 1, _creditLines[idx]._position.y), 0);

				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x - 1, _creditLines[idx]._position.y + 1), 0);
				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x, _creditLines[idx]._position.y + 1), 0);
				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x + 1, _creditLines[idx]._position.y + 1), 0);

				bb1.writeString(_creditLines[idx]._line, Common::Point(_creditLines[idx]._position.x, _creditLines[idx]._position.y), INFO_TOP);
			}
		}
	}
}

void WidgetCredits::blitCredits() {
	Screen &screen = *_vm->_screen;
	Common::Rect screenRect(0, -_creditSpeed, screen.width(), screen.height() + _creditSpeed);

	for (uint idx = 0; idx < _creditLines.size(); ++idx) {
		if (screenRect.contains(_creditLines[idx]._position)) {
			Common::Rect r(_creditLines[idx]._width, screen.fontHeight() + 2);
			r.moveTo(_creditLines[idx]._position.x, _creditLines[idx]._position.y - 1);

			screen.slamRect(r);
		}
	}
}

void WidgetCredits::eraseCredits() {
	Screen &screen = *_vm->_screen;
	Common::Rect screenRect(0, -_creditSpeed, screen.width(), screen.height() + _creditSpeed);

	for (uint idx = 0; idx < _creditLines.size(); ++idx) {
		if (screenRect.contains(_creditLines[idx]._position)) {
			Common::Rect r(_creditLines[idx]._width, screen.fontHeight() + 3);
			r.moveTo(_creditLines[idx]._position.x, _creditLines[idx]._position.y - 1 + _creditSpeed);

			screen.restoreBackground(r);
			screen.slamRect(r);
		}

		_creditLines[idx]._position.y -= _creditSpeed;
	}

	if (_creditLines[_creditLines.size() - 1]._position.y < -_creditSpeed) {
		// Completely finished credits display. Note that the credits will still remain flagged as active,
		// so that the user interface knows not to allow and standard scene interaction
		_creditLines.clear();
	}
}

} // End of namespace Tattoo

} // End of namespace Sherlock
