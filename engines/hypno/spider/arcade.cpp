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

#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"
#include "graphics/cursorman.h"

namespace Hypno {

static const int orientationIndex[9] = {0, 1, 2, 7, 8, 3, 6, 5, 4};
static const int shootOriginIndex[9][2] = {
	{41, 3}, {51, 3}, {65, 6}, {68, 9}, {71, 22}, {57, 20}, {37, 14}, {37, 11}, {57, 20}};

void SpiderEngine::drawShoot(const Common::Point &target) {
	uint32 c = _pixelFormat.RGBToColor(255, 255, 255);
	uint32 idx = MIN(2, target.x / (_screenW / 3)) + 3 * MIN(2, target.y / (_screenH / 3));
	uint32 ox = 60  + shootOriginIndex[idx][0];
	uint32 oy = 129 + shootOriginIndex[idx][1];
	_compositeSurface->drawLine(ox, oy, target.x + 2, target.y, c);
	_compositeSurface->drawLine(ox, oy, target.x, target.y, c);
	_compositeSurface->drawLine(ox, oy, target.x - 2, target.y, c);
	playSound(_soundPath + _shootSound, 1);
}

void SpiderEngine::drawPlayer() {

	if (_arcadeMode == "YC") {
		drawImage(*_playerFrames[_playerPosition], 0, 0, true);
		return;
	}

	// if (_playerFrameSep == -1) {
	// 	Common::Point mousePos = g_system->getEventManager()->getMousePos();
	// 	drawImage(*_playerFrames[0], MIN(MAX(10, int(mousePos.x)), _screenH-10), 129, true);
	// 	return;
	// }


	if (_playerFrameIdx < _playerFrameSep) {
		Common::Point mousePos = g_system->getEventManager()->getMousePos();
		//uint32 idx = MIN(2, mousePos.x / (_screenW / 3)) + 3 * MIN(2, mousePos.y / (_screenH / 3));
		_playerFrameIdx = 4 - mousePos.x / (_screenW / 4);
		//debug("selecting index %d", _playerFrameIdx);
		//_playerFrameIdx = orientationIndex[idx];
	} else {
		_playerFrameIdx++;
		if (_playerFrameIdx >= (int)_playerFrames.size())
			_playerFrameIdx = 0;
	}
	drawImage(*_playerFrames[_playerFrameIdx], _screenW/2 - 15, _screenH-50, true);
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
