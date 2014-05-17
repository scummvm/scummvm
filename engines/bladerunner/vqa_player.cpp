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

#include "bladerunner/vqa_player.h"

#include "bladerunner/bladerunner.h"

#include "common/system.h"

namespace BladeRunner {

bool VQAPlayer::open(const Common::String &name) {
	_s = _vm->getResourceStream(name);
	if (!_s)
		return false;

	if(!_decoder.open(_s)) {
		delete _s;
		_s = nullptr;
		return false;
	}

	return true;
}

int VQAPlayer::update() {
	uint32 now = _vm->_system->getMillis();

	if (_nextFrameTime == 0)
		_nextFrameTime = now;

	if (now < _nextFrameTime)
		return -1;

	_nextFrameTime += 1000 / _decoder._header.frameRate;

	if (_decoder._loopInfo.loopCount) {
		if (_loopSpecial >= 0) {
			_curLoop = _loopSpecial;
			_loopSpecial = -3;

			_curFrame = _decoder._loopInfo.loops[_curLoop].begin;
			_decoder.seekToFrame(_curFrame);
		} else if (_curLoop == -1 && _loopDefault >= 0) {
			_curLoop = _loopDefault;
			_curFrame = _decoder._loopInfo.loops[_curLoop].begin;
			_decoder.seekToFrame(_curFrame);
		} else if (_curLoop >= 0 && _curFrame == _decoder._loopInfo.loops[_curLoop].end) {
			if (_loopDefault == -1)
				return -3;

			_curLoop = _loopDefault;
			_curFrame = _decoder._loopInfo.loops[_curLoop].begin;
			_decoder.seekToFrame(_curFrame);
		}
		else
			++_curFrame;
	}
	else
		++_curFrame;

	if (_curFrame >= _decoder._header.numFrames)
		return -3;

	_decoder.readFrame();
	_decoder.decodeFrame((uint16*)_surface->getPixels());

	return _curFrame;
}

void VQAPlayer::setLoopSpecial(int loop, bool wait)
{
	_loopSpecial = loop;
	if (!wait)
		_curLoop = -1;
}

void VQAPlayer::setLoopDefault(int loop)
{
	_loopDefault = loop;
}

}; // End of namespace BladeRunner
