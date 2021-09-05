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

#include <algorithm>

#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"
#include "graphics/cursorman.h"

namespace Hypno {

static const int frame_idx[9] = {0, 1, 2, 7, 8, 3, 6, 5, 4};

void SpiderEngine::drawShoot(Common::Point target) {
	uint32 c = _pixelFormat.RGBToColor(255, 255, 255);
	_compositeSurface->drawLine(80, 155, target.x, target.y + 1, c);
	_compositeSurface->drawLine(80, 155, target.x, target.y, c);
	_compositeSurface->drawLine(80, 155, target.x, target.y - 1, c);
	playSound(_soundPath + _shootSound, 1);
}

void SpiderEngine::drawPlayer() {

	if (_playerFrameIdx < _playerFrameSep) {
		Common::Point mousePos = g_system->getEventManager()->getMousePos();
		_playerFrameIdx = frame_idx[std::min(2, mousePos.x / (_screenW / 3)) + 3 * std::min(2, mousePos.y / (_screenH / 3))];
	} else {
		_playerFrameIdx++;
		if (_playerFrameIdx >= _playerFrames.size())
			_playerFrameIdx = 0;
	}
	drawImage(*_playerFrames[_playerFrameIdx], 60, 129, true);
}

void SpiderEngine::drawHealth() {
	Common::Rect r;
	uint32 c;
	int d = (22 * (_maxHealth - _health) / _maxHealth);

	r = Common::Rect(256, 152 + d, 272, 174);
	if (d >= 11)
		c = _pixelFormat.RGBToColor(255, 0, 0);
	else
		c = _pixelFormat.RGBToColor(32, 208, 32);

	_compositeSurface->fillRect(r, c);

	r = Common::Rect(256, 152, 272, 174);
	c = _pixelFormat.RGBToColor(0, 0, 255);
	_compositeSurface->frameRect(r, c);

	_font->drawString(_compositeSurface, "ENERGY", 248, 180, 38, c);
}

} // End of namespace Hypno