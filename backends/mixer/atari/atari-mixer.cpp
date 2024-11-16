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
#include <mint/osbind.h>
#include <mint/ostruct.h>
#include <usound.h>	// https://github.com/mikrosk/usound

#include "backends/platform/atari/atari-debug.h"
#include "common/config-manager.h"

#define DEFAULT_OUTPUT_RATE 24585
#define DEFAULT_OUTPUT_CHANNELS 2
#define DEFAULT_SAMPLES 2048	// 83ms

void AtariAudioShutdown() {
	Jdisint(MFP_TIMERA);
	AtariSoundSetupDeinitXbios();
}

static volatile bool muted;
static volatile bool endOfPlayback;
static void __attribute__((interrupt)) timerA(void)
{
	if (endOfPlayback && !muted) {
		*((volatile unsigned char *)0xFFFF8901L) &= 0xFC;	// disable playback/repeat (and triggers another interrupt)
		muted = true;
	}

	endOfPlayback = true;

	*((volatile byte *)0xFFFFFA0FL) &= ~(1<<5);	// clear in service bit
}

AtariMixerManager::AtariMixerManager() : MixerManager() {
	atari_debug("AtariMixerManager()");

	suspendAudio();

	ConfMan.registerDefault("output_rate", DEFAULT_OUTPUT_RATE);
	_outputRate = ConfMan.getInt("output_rate");
	if (_outputRate <= 0)
		_outputRate = DEFAULT_OUTPUT_RATE;

	ConfMan.registerDefault("output_channels", DEFAULT_OUTPUT_CHANNELS);
	_outputChannels = ConfMan.getInt("output_channels");
	if (_outputChannels <= 0 || _outputChannels > 2)
		_outputChannels = DEFAULT_OUTPUT_CHANNELS;

	ConfMan.registerDefault("audio_buffer_size", DEFAULT_SAMPLES);
	_samples = ConfMan.getInt("audio_buffer_size");
	if (_samples <= 0)
		_samples = DEFAULT_SAMPLES;

	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

AtariMixerManager::~AtariMixerManager() {
	atari_debug("~AtariMixerManager()");

	g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);

	AtariAudioShutdown();

	Mfree(_atariSampleBuffer);
	_atariSampleBuffer = _atariPhysicalSampleBuffer = _atariLogicalSampleBuffer = nullptr;

	delete[] _samplesBuf;
	_samplesBuf = nullptr;
}

void AtariMixerManager::init() {
	AudioSpec desired, obtained;

	desired.frequency = _outputRate;
	desired.channels = _outputChannels;
	desired.format = AudioFormatSigned16MSB;
	desired.samples = _samples;

	if (!AtariSoundSetupInitXbios(&desired, &obtained)) {
		error("Sound system is not available");
	}

	if (obtained.format != AudioFormatSigned8 && obtained.format != AudioFormatSigned16MSB) {
		error("Sound system currently supports only 8/16-bit signed big endian samples");
	}

	// don't use the recommended number of samples
	obtained.samples = desired.samples;
	obtained.size = obtained.size * desired.samples / obtained.samples;

	_outputRate = obtained.frequency;
	_outputChannels = obtained.channels;
	_samples = obtained.samples;
	_downsample = (obtained.format == AudioFormatSigned8);

	ConfMan.setInt("output_rate", _outputRate);
	ConfMan.setInt("output_channels", _outputChannels);
	ConfMan.setInt("audio_buffer_size", _samples);

	atari_debug("setting %d Hz mixing frequency (%d-bit, %s)",
		  _outputRate, obtained.format == AudioFormatSigned8 ? 8 : 16, _outputChannels == 1 ? "mono" : "stereo");
	atari_debug("sample buffer size: %d", _samples);

	ConfMan.flushToDisk();

	_atariSampleBuffer = (byte*)Mxalloc(obtained.size * 2, MX_STRAM);
	if (!_atariSampleBuffer)
		error("Failed to allocate memory in ST RAM");

	_atariPhysicalSampleBuffer = _atariSampleBuffer;
	_atariLogicalSampleBuffer = _atariSampleBuffer + obtained.size;

	Setinterrupt(SI_TIMERA, SI_PLAY);
	Xbtimer(XB_TIMERA, 1<<3, 1, timerA);	// event count mode, count to '1'
	Jenabint(MFP_TIMERA);

	_samplesBuf = new uint8[_samples * _outputChannels * 2];	// always 16-bit

	_mixer = new Audio::MixerImpl(_outputRate, _outputChannels == 2, _samples);
	_mixer->setReady(true);

	resumeAudio();
}

void AtariMixerManager::suspendAudio() {
	atari_debug("suspendAudio");

	Buffoper(0x00);
	muted = true;
	_audioSuspended = true;
}

int AtariMixerManager::resumeAudio() {
	atari_debug("resumeAudio");

	_audioSuspended = false;
	update();
	return 0;
}

bool AtariMixerManager::notifyEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		if (!muted) {
			Buffoper(0x00);
			muted = true;
			atari_debug("silencing the mixer");
		}
		return false;
	default:
		break;
	}

	return false;
}

void AtariMixerManager::update() {
	if (_audioSuspended) {
		return;
	}

	assert(_mixer);

	int processed = -1;

	if (muted || endOfPlayback) {
		endOfPlayback = false;
		processed = _mixer->mixCallback(_samplesBuf, _samples * _outputChannels * 2);
	}

	if (processed > 0) {
		byte* tmp = _atariPhysicalSampleBuffer;
		_atariPhysicalSampleBuffer = _atariLogicalSampleBuffer;
		_atariLogicalSampleBuffer = tmp;

		if (_downsample) {
			// use the trick with move.b (a7)+,dx which skips two bytes at once
			// basically supplying move.w (src)+,dx; asr.w #8,dx; move.b dx,(dst)+
			__asm__ volatile(
				"	move.l	%%a7,%%d0\n"
				"	move.l	%0,%%a7\n"
				"	moveq	#0x0f,%%d1\n"
				"	and.l	%2,%%d1\n"
				"	neg.l	%%d1\n"
				"	lsr.l	#4,%2\n"
				"	jmp		(2f,%%pc,%%d1.l*2)\n"
				"1:	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"	move.b	(%%a7)+,(%1)+\n"
				"2:	dbra	%2,1b\n"
				"	move.l	%%d0,%%a7\n"
				: // outputs
				: "g"(_samplesBuf), "a"(_atariPhysicalSampleBuffer), "d"(processed * _outputChannels * 2/2) // inputs
				: "d0", "d1", "cc" AND_MEMORY
				);
			memset(_atariPhysicalSampleBuffer + processed * _outputChannels * 2/2, 0, (_samples - processed) * _outputChannels * 2/2);
			Setbuffer(SR_PLAY, _atariPhysicalSampleBuffer, _atariPhysicalSampleBuffer + _samples * _outputChannels * 2/2);
		} else {
			memcpy(_atariPhysicalSampleBuffer, _samplesBuf, processed * _outputChannels * 2);
			memset(_atariPhysicalSampleBuffer + processed * _outputChannels * 2, 0, (_samples - processed) * _outputChannels * 2);
			Setbuffer(SR_PLAY, _atariPhysicalSampleBuffer, _atariPhysicalSampleBuffer + _samples * _outputChannels * 2);
		}

		if (muted) {
			Buffoper(SB_PLA_ENA | SB_PLA_RPT);
			endOfPlayback = true;
			muted = false;
		}
	} else if (processed == 0 && !muted) {
		Buffoper(0x00);
		muted = true;
	}

	if (processed > 0 && processed != _samples) {
		atari_warning("processed: %d, _samples: %d", processed, _samples);
	}
}
