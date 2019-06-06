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

#include "petka/petka.h"
#include "petka/screen.h"

namespace Petka {

Screen::Screen() :
	_shake(false), _shift(false),
	_shakeTime(0), _interface(nullptr) {}

void Screen::update() {
	if (_shake) {
		int width = w;
		int x =  0;

		if (_shift) {
			Graphics::Surface s;
			s.create(3, 480, g_system->getScreenFormat());
			g_system->copyRectToScreen(s.getPixels(), s.pitch, 0, 0, s.w, s.h);
			w -= 3;
			x = 3;
		}

		uint32 time = g_system->getMillis();
		if (time - _shakeTime > 30) {
			_shift = !_shift;
			_shakeTime = time;
		}

		_dirtyRects.clear();
		g_system->copyRectToScreen(getPixels(), pitch, x, 0, width, h);
		g_system->updateScreen();
	} else {
		Screen::update();
	}
}

void Screen::setShake(bool shake) {
	_shake = true;
}

void Screen::setInterface(QInterface *interface) {
	_interface = interface;
}

const Common::List<Common::Rect> &Screen::dirtyRects() const {
	return _dirtyRects;
}

}