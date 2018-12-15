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

#include "bladerunner/item_pickup.h"

#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/game_info.h"
#include "bladerunner/slice_animations.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/zbuffer.h"

namespace BladeRunner {

ItemPickup::ItemPickup(BladeRunnerEngine *vm) {
	_vm = vm;
	_facingStep = float(2.0f / 3000.0f * (2.0f * M_PI));
	reset();
}

ItemPickup::~ItemPickup() {
}

void ItemPickup::setup(int animationId, int screenX, int screenY) {
	_animationId = animationId;
	_animationFrame = 0;
	_facing = 0.0;
	_timeLeft = 3000;
	_scale = 0;
	_screenX = CLIP(screenX, 40, 600);
	_screenY = CLIP(screenY, 40, 440);
	_screenRect.left = _screenX - 40;
	_screenRect.right = _screenX + 40;
	_screenRect.top = _screenY - 40;
	_screenRect.bottom = _screenY + 40;

	int pan = (150 * _screenX - 48000) / 640;
	_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(335), 80, pan, pan, 50, 0);

	_timeLast = _vm->getTotalPlayTime(); // Original game is using system timer
}

void ItemPickup::reset() {
	_animationId = -1;
	_screenX = 0;
	_screenY = 0;
	_facing = 0.0f;
	_scale = 1.0f;
	_animationFrame = 0;
	_timeLeft = 0;
	_timeLast = 0;
}

void ItemPickup::tick() {
	if (_timeLeft == 0) {
		return;
	}

	int timeNow = _vm->getTotalPlayTime(); // Original game is using system timer
	int timeDiff = timeNow - _timeLast;
	_timeLast = timeNow;
	timeDiff = MIN(MIN(timeDiff, 67), _timeLeft);
	_timeLeft -= timeDiff;

	if (_timeLeft >= 2000) {
		_scale = 1.0f - (((2000.0f - _timeLeft) / 1000.0f) * ((2000.0f - _timeLeft) / 1000.0f));
	} else if (_timeLeft < 1000) {
		_scale = 1.0f - (((1000.0f - _timeLeft) / 1000.0f) * ((1000.0f - _timeLeft) / 1000.0f));
	} else {
		_scale = 1.0f;
	}
	_scale *= 75.0f;

	_facing += _facingStep * timeDiff;
	if (_facing > float(2.0f * M_PI)) {
		_facing -= float(2.0f * M_PI);
	}

	_animationFrame = (_animationFrame + 1) % _vm->_sliceAnimations->getFrameCount(_animationId);
}

void ItemPickup::draw() {
	if (_timeLeft == 0) {
		return;
	}

	_vm->_sliceRenderer->drawOnScreen(_animationId, _animationFrame, _screenX, _screenY, _facing, _scale, _vm->_surfaceFront);
}
} // End of namespace BladeRunner
