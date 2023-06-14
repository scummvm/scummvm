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
#include <mint/cookie.h>
#include <mint/falcon.h>
#include <mint/osbind.h>
#include <mint/ostruct.h>

#include "common/config-manager.h"
#include "common/debug.h"

#define DEFAULT_OUTPUT_RATE 24585

static bool s_audioNotAvailable = true;

void AtariAudioShutdown() {
	if (!s_audioNotAvailable) {
		Buffoper(0x00);
		Sndstatus(SND_RESET);
		Unlocksnd();
	}
}

AtariMixerManager::AtariMixerManager() : MixerManager() {
	debug("AtariMixerManager()");

	_audioSuspended = true;

	ConfMan.registerDefault("output_rate", DEFAULT_OUTPUT_RATE);

	_outputRate = ConfMan.getInt("output_rate");
	if (_outputRate <= 0)
		_outputRate = DEFAULT_OUTPUT_RATE;

	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

AtariMixerManager::~AtariMixerManager() {
	debug("~AtariMixerManager()");

	g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);

	if (s_audioNotAvailable)
		return;

	AtariAudioShutdown();

	Mfree(_atariSampleBuffer);
	_atariSampleBuffer = _atariPhysicalSampleBuffer = _atariLogicalSampleBuffer = nullptr;

	delete[] _samplesBuf;
}

void AtariMixerManager::init() {
	long cookie;
	bool useDevconnectReturnValue = false;

	if (Getcookie(C__SND, &cookie) == C_FOUND) {
		if (cookie & SND_16BIT)
			s_audioNotAvailable = false;

		useDevconnectReturnValue = (cookie & SND_EXT) != 0;
	}

	if (s_audioNotAvailable) {
		warning("Mixer manager requires 16-bit stereo mode, disabling");
	} else {
		int clk;

		if (Locksnd() < 0)
			error("Sound system is locked");

		// try XBIOS APIs which do not set SND_EXT in _SND
		useDevconnectReturnValue |= (Getcookie(C_STFA, &cookie) == C_FOUND);	// STFA
		useDevconnectReturnValue |= (Getcookie(C_McSn, &cookie) == C_FOUND);	// X-SOUND, MacSound

		// reset connection matrix (and other settings)
		Sndstatus(SND_RESET);

		int diff50, diff33, diff25, diff20, diff16, diff12, diff10, diff8, diff6;
		diff50 = abs(49170 - (int)_outputRate);
		diff33 = abs(32780 - (int)_outputRate);
		diff25 = abs(24585 - (int)_outputRate);
		diff20 = abs(19668 - (int)_outputRate);
		diff16 = abs(16390 - (int)_outputRate);
		diff12 = abs(12292 - (int)_outputRate);
		diff10 = abs(9834  - (int)_outputRate);
		diff8  = abs(8195  - (int)_outputRate);

		if (diff50 < diff33) {
			_outputRate = 49170;
			clk = CLK50K;
		} else if (diff33 < diff25) {
			_outputRate = 32780;
			clk = CLK33K;
		} else if (diff25 < diff20) {
			_outputRate = 24585;
			clk = CLK25K;
		} else if (diff20 < diff16) {
			_outputRate = 19668;
			clk = CLK20K;
		} else if (diff16 < diff12) {
			_outputRate = 16390;
			clk = CLK16K;
		} else if (diff12 < diff10) {
			_outputRate = 12292;
			clk = CLK12K;
		} else if (diff10 < diff8) {
			_outputRate = 9834;
			clk = CLK10K;
		} else {
			_outputRate = 8195;
			clk = CLK8K;
		}

		// first try to use Devconnect() with a Falcon prescaler
		if (Devconnect(DMAPLAY, DAC, CLK25M, clk, NO_SHAKE) != 0) {
			// the return value is broken on Falcon
			if (useDevconnectReturnValue) {
				if (Devconnect(DMAPLAY, DAC, CLK25M, CLKOLD, NO_SHAKE) == 0) {
					// calculate compatible prescaler
					diff50 = abs(50066 - (int)_outputRate);
					diff25 = abs(25033 - (int)_outputRate);
					diff12 = abs(12517 - (int)_outputRate);
					diff6  = abs(6258  - (int)_outputRate);

					if (diff50 < diff25) {
						_outputRate = 50066;
						clk = PRE160;
					} else if (diff25 < diff12) {
						_outputRate = 25033;
						clk = PRE320;
					} else if (diff12 < diff6) {
						_outputRate = 12517;
						clk = PRE640;
					} else {
						_outputRate = 6258;
						clk = PRE1280;
					}

					Soundcmd(SETPRESCALE, clk);
				} else {
					error("Devconnect() failed");
				}
			}
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

		_atariSampleBufferSize = _samples * 4;

		_atariSampleBuffer = (byte*)Mxalloc(_atariSampleBufferSize * 2, MX_STRAM);
		if (!_atariSampleBuffer)
			error("Failed to allocate memory in ST RAM");

		_atariPhysicalSampleBuffer = _atariSampleBuffer;
		_atariLogicalSampleBuffer = _atariSampleBuffer + _atariSampleBufferSize;

		memset(_atariSampleBuffer, 0, 2 * _atariSampleBufferSize);

		Setmode(MODE_STEREO16);
		Soundcmd(ADDERIN, MATIN);
		Setbuffer(SR_PLAY, _atariSampleBuffer, _atariSampleBuffer + 2 * _atariSampleBufferSize);
		Buffoper(SB_PLA_ENA | SB_PLA_RPT);

		_samplesBuf = new uint8[_samples * 4];
	}

	_mixer = new Audio::MixerImpl(_outputRate, _samples);
	_mixer->setReady(true);

	_audioSuspended = false;
}

void AtariMixerManager::suspendAudio() {
	if (s_audioNotAvailable)
		return;

	debug("suspendAudio");

	Buffoper(0x00);

	_audioSuspended = true;
}

int AtariMixerManager::resumeAudio() {
	if (s_audioNotAvailable)
		return 0;

	debug("resumeAudio");

	Buffoper(SB_PLA_ENA | SB_PLA_RPT);

	_audioSuspended = false;
	return 0;
}

bool AtariMixerManager::notifyEvent(const Common::Event &event) {
	if (s_audioNotAvailable)
		return false;

	switch (event.type) {
	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		debug("silencing the mixer");	// TODO: is it for long enough?
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

	if (s_audioNotAvailable) {
		static byte dummy[4];
		_mixer->mixCallback(dummy, 0);
		return;
	}

	static bool loadSampleFlag = true;
	byte *buf = nullptr;

	SndBufPtr sPtr;
	if (Buffptr(&sPtr) != 0)
		return;

	if (!loadSampleFlag) {
		if ((byte*)sPtr.play < _atariLogicalSampleBuffer) {
			buf = _atariLogicalSampleBuffer;
			loadSampleFlag = !loadSampleFlag;
		}
	} else {
		if ((byte*)sPtr.play >= _atariLogicalSampleBuffer) {
			buf = _atariPhysicalSampleBuffer;
			loadSampleFlag = !loadSampleFlag;
		}
	}

	if (buf != nullptr) {
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
