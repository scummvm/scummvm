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
	if (!_s)
		return false;

	if(!_decoder.loadStream(_s)) {
		delete _s;
		_s = nullptr;
		return false;
	}

	_hasAudio = _decoder.hasAudio();
	if (_hasAudio)
		_audioStream = Audio::makeQueuingAudioStream(_decoder.frequency(), false);

	return true;
}

void VQAPlayer::close() {
	_vm->_mixer->stopHandle(_soundHandle);
	delete _s;
	_s = nullptr;
}

int VQAPlayer::update() {
	uint32 now = 60 * _vm->_system->getMillis();

	if (_curFrame == -1) {
		_curFrame = 0;
		if (_curFrame >= 0) {
			_decoder.readNextPacket();
			if (_hasAudio)
				queueAudioFrame(_decoder.decodeAudioFrame());
			_surface = _decoder.decodeVideoFrame();
			_zBuffer = _decoder.decodeZBuffer();
			_view = _decoder.getView();
		}

		_decodedFrame = calcNextFrame(_curFrame);
		if (_decodedFrame >= 0) {
			_decoder.readNextPacket();
			if (_hasAudio)
				queueAudioFrame(_decoder.decodeAudioFrame());
		}

		if (_hasAudio) {
			_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, _audioStream);
			_audioStarted = true;
		}

		_nextFrameTime = now + 60000 / 15;
		return _curFrame;
	}

	if (now >= _nextFrameTime) {
		_curFrame = _decodedFrame;
		if (_curFrame >= 0) {
			_surface = _decoder.decodeVideoFrame();
			_zBuffer = _decoder.decodeZBuffer();
			_view = _decoder.getView();
		}

		_decodedFrame = calcNextFrame(_curFrame);
		if (_decodedFrame >= 0) {
			_decoder.readNextPacket();
			if (_hasAudio)
				queueAudioFrame(_decoder.decodeAudioFrame());
		}

		_nextFrameTime += 60000 / 15;
		return _curFrame;
	}

	_surface = nullptr;
	return -1;
}

const Graphics::Surface *VQAPlayer::getSurface() const {
	return _surface;
}

const uint16 *VQAPlayer::getZBuffer() const {
	return _zBuffer;
}

void VQAPlayer::setLoopSpecial(int loop, bool wait) {
	_loopSpecial = loop;
	if (!wait)
		_curLoop = -1;
}

void VQAPlayer::setLoopDefault(int loop) {
	_loopDefault = loop;
}

int VQAPlayer::calcNextFrame(int frame) const {
	if (frame < 0)
		return -3;

	frame += 1;

	if (frame == _decoder.numFrames())
		frame = -3;

	return frame;
}

void VQAPlayer::queueAudioFrame(Audio::AudioStream *audioStream) {
	int n = _audioStream->numQueuedStreams();
	if (n == 0)
		warning("numQueuedStreams: %d", n);
	_audioStream->queueAudioStream(audioStream, DisposeAfterUse::YES);
}

} // End of namespace BladeRunner
