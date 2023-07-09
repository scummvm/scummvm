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

void AtariAudioShutdown() {
	Buffoper(0x00);
	Sndstatus(SND_RESET);
	Unlocksnd();
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

	AtariAudioShutdown();

	Mfree(_atariSampleBuffer);
	_atariSampleBuffer = _atariPhysicalSampleBuffer = _atariLogicalSampleBuffer = nullptr;

	delete[] _samplesBuf;
}

void AtariMixerManager::init() {
    long cookie, stfa = 0;
	bool useDevconnectReturnValue = Getcookie(C__SND, &cookie) == C_FOUND && (cookie & SND_EXT) != 0;

	int clk;

	if (Locksnd() < 0)
		error("Sound system is locked");

	// try XBIOS APIs which do not set SND_EXT in _SND
    useDevconnectReturnValue |= (Getcookie(C_STFA, &stfa) == C_FOUND);	// STFA
	useDevconnectReturnValue |= (Getcookie(C_McSn, &cookie) == C_FOUND);	// X-SOUND, MacSound

    bool forceSoundCmd = false;
    if (stfa) {
        // see http://removers.free.fr/softs/stfa.php#STFA
        struct STFA_control {
            uint16 sound_enable;
            uint16 sound_control;
            uint16 sound_output;
            uint32 sound_start;
            uint32 sound_current;
            uint32 sound_end;
            uint16 version;
            uint32 old_vbl;
            uint32 old_timerA;
            uint32 old_mfp_status;
            uint32 stfa_vbl;
            uint32 drivers_list;
            uint32 play_stop;
            uint16 timer_a_setting;
            uint32 set_frequency;
            uint16 frequency_treshold;
            uint32 custom_freq_table;
            int16 stfa_on_off;
            uint32 new_drivers_list;
            uint32 old_bit_2_of_cookie_snd;
            uint32 it;
        } __attribute__((packed));

        STFA_control *stfaControl = (STFA_control *)stfa;
        if (stfaControl->version < 0x0200) {
            error("Your STFA version is too old, please upgrade to at least 2.00");
        }
        if (stfaControl->stfa_on_off == -1) {
            // emulating 16-bit playback, force TT frequencies
            enum {
                MCH_ST = 0,
                MCH_STE,
                MCH_TT,
                MCH_FALCON,
                MCH_CLONE,
                MCH_ARANYM
            };

            long mch = MCH_ST<<16;
            Getcookie(C__MCH, &mch);
            mch >>= 16;

            if (mch == MCH_TT) {
                debug("Forcing STE/TT compatible frequency");
                forceSoundCmd = true;
            }
        }
    }

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
    if (forceSoundCmd || Devconnect(DMAPLAY, DAC, CLK25M, clk, NO_SHAKE) != 0) {
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

	Setmode(MODE_STEREO16);
	Soundcmd(ADDERIN, MATIN);
	Setbuffer(SR_PLAY, _atariSampleBuffer, _atariSampleBuffer + 2 * _atariSampleBufferSize);

	_samplesBuf = new uint8[_samples * 4];

	_mixer = new Audio::MixerImpl(_outputRate, _samples);
	_mixer->setReady(true);

	_audioSuspended = false;
}

void AtariMixerManager::suspendAudio() {
	debug("suspendAudio");

	stopPlayback(kPlaybackStopped);

	_audioSuspended = true;
}

int AtariMixerManager::resumeAudio() {
	debug("resumeAudio");

	update();

	_audioSuspended = false;
	return 0;
}

bool AtariMixerManager::notifyEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		stopPlayback(kPlaybackStopped);
		debug("silencing the mixer");
		return false;
	default:
		[[fallthrough]];
	}

	return false;
}

void AtariMixerManager::startPlayback(PlaybackState playbackState) {
	Buffoper(SB_PLA_ENA | SB_PLA_RPT);
	_playbackState = playbackState;
	debug("playback started");
}

void AtariMixerManager::stopPlayback(PlaybackState playbackState) {
	Buffoper(0x00);
	_playbackState = playbackState;
	debug("playback stopped");
}

void AtariMixerManager::update() {
	if (_audioSuspended) {
		return;
	}

	assert(_mixer);

	uint32 processed = 0;
	if (_playbackState == kPlaybackStopped) {
		// playback stopped means that we are not playing anything (DMA
		// pointer is not updating) but the mixer may have something available
		processed = _mixer->mixCallback(_samplesBuf, _samples * 4);

		if (processed > 0) {
			memcpy(_atariPhysicalSampleBuffer, _samplesBuf, processed * 4);
			startPlayback(kPlayingFromPhysicalBuffer);
		}
	} else {
		SndBufPtr sPtr;
		if (Buffptr(&sPtr) != 0) {
			warning("Buffptr() failed");
			return;
		}

		byte *buf = nullptr;

		if (_playbackState == kPlayingFromPhysicalBuffer) {
			if ((byte*)sPtr.play < _atariLogicalSampleBuffer) {
				buf = _atariLogicalSampleBuffer;
				_playbackState = kPlayingFromLogicalBuffer;
			}
		} else if (_playbackState == kPlayingFromLogicalBuffer) {
			if ((byte*)sPtr.play >= _atariLogicalSampleBuffer) {
				buf = _atariPhysicalSampleBuffer;
				_playbackState = kPlayingFromPhysicalBuffer;
			}
		}

		if (buf) {
			processed = _mixer->mixCallback(_samplesBuf, _samples * 4);
			if (processed > 0) {
				memcpy(buf, _samplesBuf, processed * 4);
			} else {
				stopPlayback(kPlaybackStopped);
			}
		}
	}

	if (processed > 0 && processed != _samples) {
		warning("processed: %d, _samples: %d", processed, _samples);
	}
}
