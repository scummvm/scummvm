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

#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"
#include "graphics/cursorman.h"

namespace Hypno {

bool WetEngine::clickedSecondaryShoot(const Common::Point &mousePos) {
	return clickedPrimaryShoot(mousePos);
}

void WetEngine::hitPlayer() {
	assert( _playerFrameSep < (int)_playerFrames.size());
	if (_playerFrameIdx < _playerFrameSep)
		_playerFrameIdx = _playerFrameSep;
}

void WetEngine::drawShoot(const Common::Point &mousePos) {
	uint32 c = 253;
	_compositeSurface->drawLine(0, _screenH, mousePos.x, mousePos.y, c);
	_compositeSurface->drawLine(0, _screenH, mousePos.x - 1, mousePos.y, c);
	_compositeSurface->drawLine(0, _screenH, mousePos.x - 2, mousePos.y, c);

	_compositeSurface->drawLine(_screenW, _screenH, mousePos.x, mousePos.y, c);
	_compositeSurface->drawLine(_screenW, _screenH, mousePos.x - 1, mousePos.y, c);
	_compositeSurface->drawLine(_screenW, _screenH, mousePos.x - 2, mousePos.y, c);
	playSound(_soundPath + _shootSound, 1);
}

void WetEngine::drawPlayer() {

	if (_playerFrameIdx < _playerFrameSep) {
		// TARGET ACQUIRED frame
		uint32 c = 251;
		_compositeSurface->drawLine(113, 1, 119, 1, c);
		_compositeSurface->drawLine(200, 1, 206, 1, c);

		_compositeSurface->drawLine(113, 1, 113, 9, c);
		_compositeSurface->drawLine(206, 1, 206, 9, c);

		_compositeSurface->drawLine(113, 9, 119, 9, c);
		_compositeSurface->drawLine(200, 9, 206, 9, c);

		c = _pixelFormat.RGBToColor(255, 0, 0);
		Common::Point mousePos = g_system->getEventManager()->getMousePos();
		int i = detectTarget(mousePos);
		if (i > 0)
			drawString("TARGET  ACQUIRED", 120, 1, 80, c);

		_playerFrameIdx++;
		_playerFrameIdx = _playerFrameIdx % _playerFrameSep;
	} else {
		_playerFrameIdx++;
		if (_playerFrameIdx >= (int)_playerFrames.size())
			_playerFrameIdx = 0;
	}

	drawImage(*_playerFrames[_playerFrameIdx], 0, 200 - _playerFrames[_playerFrameIdx]->h + 1, true);
}

void WetEngine::drawHealth() {
	uint32 c = 253; //_pixelFormat.RGBToColor(252, 252, 0);
	int p = (100 * _health) / _maxHealth;
	int s = _score;
	if (_playerFrameIdx < _playerFrameSep) {
		const chapterEntry *entry = _chapterTable[_levelId];
		//uint32 id = _levelId;
		drawString(Common::String::format("ENERGY   %d%%", p), entry->energyPos[0], entry->energyPos[1], 65, c);
		drawString(Common::String::format("SCORE    %04d", s), entry->scorePos[0], entry->scorePos[1], 72, c);
		// Objectives are always in the zero in the demo
		//drawString(Common::String::format("M.O.     0/0"), uiPos[id][2][0], uiPos[id][2][1], 60, c);
	}
}

} // End of namespace Hypno
