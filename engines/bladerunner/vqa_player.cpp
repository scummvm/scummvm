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

#include "audio/decoders/raw.h"

#include "common/system.h"

namespace BladeRunner {

bool VQAPlayer::open(const Common::String &name) {
	_s = _vm->getResourceStream(name);
	if (!_s) {
		return false;
	}

	if (!_decoder.loadStream(_s)) {
		delete _s;
		_s = nullptr;
		return false;
	}

	_hasAudio = _decoder.hasAudio();
	if (_hasAudio) {
		_audioStream = Audio::makeQueuingAudioStream(_decoder.frequency(), false);
	}

	if (_loopInitial >= 0) {
		setLoop(_loopInitial, _repeatsCountInitial, 2, nullptr, nullptr);
	} else {
		setBeginAndEndFrame(0, _decoder.numFrames() - 1, 0, 0, nullptr, nullptr);
	}

	return true;
}

void VQAPlayer::close() {
	_vm->_mixer->stopHandle(_soundHandle);
	delete _s;
	_s = nullptr;
}

int VQAPlayer::update() {
	uint32 now = 60 * _vm->_system->getMillis();

	if (_frameCurrent == -1) {
		_frameCurrent = 0;
		if (_frameCurrent >= 0) {
			_decoder.readPacket(_frameCurrent);
			if (_hasAudio)
				queueAudioFrame(_decoder.decodeAudioFrame());
			_surface = _decoder.decodeVideoFrame();
		}

		_frameDecoded = calcNextFrame(_frameCurrent);
		if (_frameDecoded >= 0) {
			_decoder.readPacket(_frameDecoded);
			if (_hasAudio)
				queueAudioFrame(_decoder.decodeAudioFrame());
		}

		if (_hasAudio) {
			_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, _audioStream);
			_audioStarted = true;
		}

		_nextFrameTime = now + 60000 / 15;
		return _frameCurrent;
	}

	if (now >= _nextFrameTime) {
		_frameCurrent = _frameDecoded;
		if (_frameCurrent >= 0) {
			_surface = _decoder.decodeVideoFrame();
		}

		_frameDecoded = calcNextFrame(_frameCurrent);
		if (_frameDecoded >= 0) {
			_decoder.readPacket(_frameDecoded);
			if (_hasAudio)
				queueAudioFrame(_decoder.decodeAudioFrame());
		}

		_nextFrameTime += 60000 / 15;
		return _frameCurrent;
	}

	_surface = nullptr;
	return -1;
}

const Graphics::Surface *VQAPlayer::getSurface() const {
	return _surface;
}

void VQAPlayer::updateZBuffer(ZBuffer *zbuffer) {
	_decoder.decodeZBuffer(zbuffer);
}

void VQAPlayer::updateView(View *view) {
	_decoder.decodeView(view);
}

void VQAPlayer::updateLights(Lights *lights) {
	_decoder.decodeLights(lights);
}

bool VQAPlayer::setLoop(int loop, int repeatsCount, int loopMode, void (*callback)(void *, int, int), void *callbackData) {
//	debug("VQAPlayer::setBeginAndEndFrameFromLoop(%i, %i, %i, %x, %p), streamLoaded = %i", loop, repeatsCount, loopMode, (uint)callback, callbackData, _s != nullptr);
	if (_s == nullptr) {
		_loopInitial = loop;
		_repeatsCountInitial = repeatsCount;
		return true;
	}

	int begin, end;
	if (!_decoder.getLoopBeginAndEndFrame(loop, &begin, &end)) {
		return false;
	}
	if (setBeginAndEndFrame(begin, end, repeatsCount, loopMode, callback, callbackData)) {
		_loop = loop;
		return true;
	}
	return false;
}

bool VQAPlayer::setBeginAndEndFrame(int begin, int end, int repeatsCount, int loopMode, void (*callback)(void *, int, int), void *callbackData) {
//	debug("VQAPlayer::setBeginAndEndFrame(%i, %i, %i, %i, %x, %p), streamLoaded = %i", begin, end, repeatsCount, loopMode, (uint)callback, callbackData, _s != nullptr);

	if (repeatsCount < 0) {
		repeatsCount = -1;
	}

	if (_repeatsCount == 0 && loopMode == 1) {
		loopMode = 2;
	}

	//TODO: there is code in original game which deals with changing loop at start of loop, is it nescesarry? loc_46EA04

	_frameBegin = begin;

	if (loopMode == 1) {
		_repeatsCountQueued = repeatsCount;
		_frameEndQueued = end;
	} else if (loopMode == 2) {
		_repeatsCount = repeatsCount;
		_frameEnd = end;
		_frameCurrent = begin;
		//TODO: extract this to seek function
		_decoder.readPacket(_frameCurrent);
		_frameDecoded = _frameCurrent;
		_nextFrameTime = 60 * _vm->_system->getMillis();
	} else if (loopMode == 0) {
		_repeatsCount = repeatsCount;
	}

	_callbackLoopEnded = callback;
	_callbackData = callbackData;

	return true;
}

int VQAPlayer::getLoopBeginFrame(int loop) {
	int begin, end;
	if (!_decoder.getLoopBeginAndEndFrame(loop, &begin, &end)) {
		return -1;
	}
	return begin;
}

int VQAPlayer::getLoopEndFrame(int loop) {
	int begin, end;
	if (!_decoder.getLoopBeginAndEndFrame(loop, &begin, &end)) {
		return -1;
	}
	return end;
}

int VQAPlayer::calcNextFrame(int frame) {
	//TODO: needs a slight refactoring, because it is not only calculating the next frame
	if (frame < 0) {
		return -3;
	}

	int frameNext = frame + 1;

	if ((_repeatsCount > 0 || _repeatsCount == -1) && (frameNext > _frameEnd)) {
		int loopEndQueued = _frameEndQueued;
		if (_frameEndQueued != -1) {
			_frameEnd = _frameEndQueued;
			_frameEndQueued = -1;
		}
		if (frameNext != _frameBegin) {
			frameNext = _frameBegin;
		}

		if (loopEndQueued == -1) {
			if (_repeatsCount != -1) {
				_repeatsCount--;
			}
			//callback for repeat, it is not used in the blade runner
		} else {
			_repeatsCount = _repeatsCountQueued;
			_repeatsCountQueued = -1;

			if (_callbackLoopEnded != nullptr) {
				_callbackLoopEnded(_callbackData, 0, _loop);
			}
		}
	}

	//TODO: original game is using end of loop instead of count of frames
	if (frameNext == _decoder.numFrames()) {
		return -3;
	}

	return frameNext;
}

void VQAPlayer::queueAudioFrame(Audio::AudioStream *audioStream) {
	int n = _audioStream->numQueuedStreams();
	if (n == 0)
		warning("numQueuedStreams: %d", n);
	_audioStream->queueAudioStream(audioStream, DisposeAfterUse::YES);
}

} // End of namespace BladeRunner
