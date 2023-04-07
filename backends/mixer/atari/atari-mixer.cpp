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

#include "backends/mixer/atari/atari-mixer.h"

#include <math.h>

#include <mint/falcon.h>

#include "common/config-manager.h"
#include "common/debug.h"

#define DEFAULT_OUTPUT_RATE 24585

AtariMixerManager::AtariMixerManager() : MixerManager() {
	ConfMan.registerDefault("output_rate", DEFAULT_OUTPUT_RATE);

	_outputRate = ConfMan.getInt("output_rate");
	if (_outputRate <= 0)
		_outputRate = DEFAULT_OUTPUT_RATE;

	int diff50, diff33, diff25, diff20, diff16, diff12, diff10, diff8;
	diff50 = abs(49170 - (int)_outputRate);
	diff33 = abs(32780 - (int)_outputRate);
	diff25 = abs(24585 - (int)_outputRate);
	diff20 = abs(19668 - (int)_outputRate);
	diff16 = abs(16390 - (int)_outputRate);
	diff12 = abs(12292 - (int)_outputRate);
	diff10 = abs(9834 - (int)_outputRate);
	diff8  = abs(8195 - (int)_outputRate);

	if (diff50 < diff33) {
		_outputRate = 49170;
		_clk = CLK50K;
	} else if (diff33 < diff25) {
		_outputRate = 32780;
		_clk = CLK33K;
	} else if (diff25 < diff20) {
		_outputRate = 24585;
		_clk = CLK25K;
	} else if (diff20 < diff16) {
		_outputRate = 19668;
		_clk = CLK20K;
	} else if (diff16 < diff12) {
		_outputRate = 16390;
		_clk = CLK16K;
	} else if (diff12 < diff10) {
		_outputRate = 12292;
		_clk = CLK12K;
	} else if (diff10 < diff8) {
		_outputRate = 9834;
		_clk = CLK10K;
	} else {
		_outputRate = 8195;
		_clk = CLK8K;
	}

	ConfMan.setInt("output_rate", _outputRate);
	debug("setting %d Hz mixing frequency", _outputRate);

	_samples = 8192;
	while (_samples * 16 > _outputRate * 2)
		_samples >>= 1;

	ConfMan.registerDefault("audio_buffer_size", (int)_samples);

	int samples = ConfMan.getInt("audio_buffer_size");
	if (samples > 0)
		_samples = samples;

	ConfMan.setInt("audio_buffer_size", (int)_samples);
	debug("sample buffer size: %d", _samples);

	ConfMan.flushToDisk();

	_samplesBuf = new uint8[_samples * 4];

	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

AtariMixerManager::~AtariMixerManager() {
	g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);

	Buffoper(0x00);

	Mfree(_atariSampleBuffer);
	_atariSampleBuffer = _atariPhysicalSampleBuffer = _atariLogicalSampleBuffer = nullptr;

	Sndstatus(SND_RESET);

	Unlocksnd();

	_atariInitialized = false;

	delete[] _samplesBuf;
}

void AtariMixerManager::init() {
	_mixer = new Audio::MixerImpl(_outputRate, _samples);
	_mixer->setReady(true);

	_atariSampleBufferSize = _samples * 4;

	_atariSampleBuffer = (byte*)Mxalloc(_atariSampleBufferSize * 2, MX_STRAM);
	if (!_atariSampleBuffer)
		return;

	_atariPhysicalSampleBuffer = _atariSampleBuffer;
	_atariLogicalSampleBuffer = _atariSampleBuffer + _atariSampleBufferSize;

	memset(_atariSampleBuffer, 0, 2 * _atariSampleBufferSize);

	if (Locksnd() < 0)
		return;

	Sndstatus(SND_RESET);
	Setmode(MODE_STEREO16);
	Devconnect(DMAPLAY, DAC, CLK25M, _clk, NO_SHAKE);
	Soundcmd(ADDERIN, MATIN);
	Setbuffer(SR_PLAY, _atariSampleBuffer, _atariSampleBuffer + 2 * _atariSampleBufferSize);
	Buffoper(SB_PLA_ENA | SB_PLA_RPT);

	_atariInitialized = true;
}

void AtariMixerManager::suspendAudio() {
	debug("suspendAudio");

	Buffoper(0x00);

	_audioSuspended = true;
}

int AtariMixerManager::resumeAudio() {
	debug("resumeAudio 1");

	if (!_audioSuspended || !_atariInitialized) {
		return -2;
	}

	debug("resumeAudio 2");

	Buffoper(SB_PLA_ENA | SB_PLA_RPT);

	_audioSuspended = false;
	return 0;
}

bool AtariMixerManager::notifyEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		debug("silencing the mixer");
		memset(_atariSampleBuffer, 0, 2 * _atariSampleBufferSize);
		return false;
	case Common::EVENT_MUTE:
		_muted = !_muted;
		debug("audio %s", _muted ? "off" : "on");
		return false;
	default:
		[[fallthrough]];
	}

	return false;
}

void AtariMixerManager::update() {
	if (_audioSuspended) {
		return;
	}

	static bool loadSampleFlag = true;
	byte *buf = nullptr;

	SndBufPtr sPtr;
	if (Buffptr(&sPtr) != 0)
		return;

	if (!loadSampleFlag) {
		// we play from _atariPhysicalSampleBuffer (1st buffer)
		if ((byte*)sPtr.play < _atariLogicalSampleBuffer) {
			buf = _atariLogicalSampleBuffer;
			loadSampleFlag = !loadSampleFlag;
		}
	} else {
		// we play from _atariLogicalSampleBuffer (2nd buffer)
		if ((byte*)sPtr.play >= _atariLogicalSampleBuffer) {
			buf = _atariPhysicalSampleBuffer;
			loadSampleFlag = !loadSampleFlag;
		}
	}

	if (_atariInitialized && buf != nullptr) {
		assert(_mixer);
		// generates stereo 16-bit samples
		int processed = _mixer->mixCallback(_samplesBuf, _muted ? 0 : _samples * 4);
		if (processed > 0) {
			memcpy(buf, _samplesBuf, processed * 4);
		} else {
			memset(buf, 0, _atariSampleBufferSize);
		}
	}
}
