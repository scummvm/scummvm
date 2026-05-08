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

#define FORCE_TEXT_CONSOLE

#include "backends/mixer/atari/atari-mixer.h"

#include <mint/falcon.h>
#include <mint/osbind.h>
#include <mint/ostruct.h>
// https://github.com/mikrosk/usound
// Use usound_compat.h until SDL 1.2 + uSound have been upgraded in the build image.
// Replace with #include <usound.h> once ihe image ships usound.h >= 2; it will #error in such case.
#include "usound_compat.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/textconsole.h"

#ifdef DISABLE_FANCY_THEMES
#define DEFAULT_OUTPUT_RATE			11025
#define DEFAULT_OUTPUT_CHANNELS		1
#define DEFAULT_SAMPLES				512		// 2 * 46ms (42ms at 12292 Hz) latency
#else
#define DEFAULT_OUTPUT_RATE			22050
#define DEFAULT_OUTPUT_CHANNELS		2
#define DEFAULT_SAMPLES				1024	// 2 * 46ms (42ms at 24585 Hz) latency
#endif

static USoundContext usoundContext;

void AtariAudioShutdown() {
	Jdisint(MFP_TIMERA);
	USoundDeinitXbios(&usoundContext);
}

static volatile enum {
	kPlaybackStopped,	// DMA not playing (initial or after starvation)
	kPlay1stHalf,		// DMA looping [Beg, Mid)
	kPlay2ndHalf		// DMA looping [Mid, End)
} s_playbackState = kPlaybackStopped;

static volatile uint32 s_updatePulse;
static volatile bool s_dmaWrapped;
static volatile bool s_isrStoppedDma;

static void __attribute__((interrupt)) timerA(void) {
	static uint32 s_lastPulseSeen;

	if (s_updatePulse == s_lastPulseSeen) {
		// update() didn't run since the previous wrap: stop the playback
		*((volatile byte *)0xFFFF8901L) = 0;
		s_isrStoppedDma = true;
	} else {
		s_dmaWrapped = true;
	}
	s_lastPulseSeen = s_updatePulse;

	// clear in-service bit
	*((volatile byte *)0xFFFFFA0FL) = ~(1 << 5);
}

AtariMixerManager::AtariMixerManager() : MixerManager() {
	debug("AtariMixerManager()");

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
	debug("~AtariMixerManager()");

	g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);

	AtariAudioShutdown();

	Mfree(_atariSampleBuffer);
	_atariSampleBuffer = nullptr;

	delete[] _sampleBuffer;
	_sampleBuffer = nullptr;
}

void AtariMixerManager::init() {
	USoundSpec desired, obtained;

	desired.frequency = _outputRate;
	desired.channels = _outputChannels;
	desired.format = USoundFormatSigned16MSB;
	desired.samples = _samples;

	if (!USoundInitXbios(&desired, &obtained, &usoundContext)) {
		error("Sound system is not available");
	}

	if (obtained.format != USoundFormatSigned8 && obtained.format != USoundFormatSigned16MSB) {
		error("Sound system currently supports only 8/16-bit signed big endian samples");
	}

	// don't use the recommended number of samples
	obtained.size = obtained.size * desired.samples / obtained.samples;
	obtained.samples = desired.samples;

	_outputRate = obtained.frequency;
	if (desired.channels == 1 && obtained.channels == 2 && obtained.format == USoundFormatSigned16MSB) {
		_outputChannels = 1;
		_emulated16bitMono = true;
	} else {
		_outputChannels = obtained.channels;
		_emulated16bitMono = false;
	}
	_downsample = (obtained.format == USoundFormatSigned8);
	_samples = obtained.samples;

	debug("setting %d Hz mixing frequency, %d-bit, %s",
		_outputRate,
		obtained.format == USoundFormatSigned8 ? 8 : 16,
		_outputChannels == 2
			? "stereo"
			: _emulated16bitMono
				? "mono (emulated)"
				: "mono");
	debug("audio buffer size: %d", _samples);

	_atariSampleBufferSize = obtained.size * 2;	// two buffers
	_atariSampleBuffer = (byte *)Mxalloc(_atariSampleBufferSize, MX_STRAM);
	if (!_atariSampleBuffer) {
		_atariSampleBufferSize = 0;
		error("Failed to allocate memory in ST RAM");
	}

	Setinterrupt(SI_TIMERA, SI_PLAY);
	Xbtimer(XB_TIMERA, 1<<3, 1, timerA);	// event count mode, count to '1'
	Jenabint(MFP_TIMERA);

	// route both mic channels to the ADC
	Soundcmd(ADCINPUT, 0);
	// enable and mix both sources (ADC and connection matrix) to the output
	Soundcmd(ADDERIN, MATIN|ADCIN);

	_sampleBufferSize = _samples * _outputChannels * 4;	// always 32-bit
	_sampleBuffer = new uint8[_sampleBufferSize];

	_mixer = new Audio::MixerImpl(_outputRate, _outputChannels == 2, _samples, 4, false);
	_mixer->setReady(true);

	resumeAudio();
}

void AtariMixerManager::suspendAudio() {
	debug("suspendAudio");

	Buffoper(0x00);
	s_playbackState = kPlaybackStopped;
	_audioSuspended = true;
}

int AtariMixerManager::resumeAudio() {
	debug("resumeAudio");

	_audioSuspended = false;
	update();
	return 0;
}

bool AtariMixerManager::notifyEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		if (s_playbackState != kPlaybackStopped) {
			debug("silencing the mixer");
			suspendAudio();
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

	s_updatePulse++;

	// Translate ISR's starvation signal into a state transition. Done
	// here so that update() is the only writer of s_playbackState.
	if (s_isrStoppedDma) {
		s_isrStoppedDma = false;
		s_playbackState = kPlaybackStopped;
	}

	byte *atariSampleBuffer1stHalf = _atariSampleBuffer;
	byte *atariSampleBuffer2ndHalf = _atariSampleBuffer + _atariSampleBufferSize/2;
	byte *atariSampleBufferEnd     = _atariSampleBuffer + _atariSampleBufferSize;

	bool needsMix = false;

	if (s_playbackState == kPlaybackStopped) {
		memset(_atariSampleBuffer, 0, _atariSampleBufferSize);
		Setbuffer(SR_PLAY, atariSampleBuffer1stHalf, atariSampleBuffer2ndHalf);
		Buffoper(SB_PLA_ENA | SB_PLA_RPT);
		s_playbackState = kPlay1stHalf;
		// Buffoper's 0->ENA transition can fire a spurious SI_PLAY which
		// would set s_dmaWrapped here. The resulting extra state toggle
		// is benign — it just shuffles which physical half holds the next
		// mix. Audio output is continuous either way.
		needsMix = true;
	}

	if (s_dmaWrapped) {
		s_dmaWrapped = false;
		if (s_playbackState == kPlay1stHalf)
			s_playbackState = kPlay2ndHalf;
		else if (s_playbackState == kPlay2ndHalf)
			s_playbackState = kPlay1stHalf;
		needsMix = true;
	}

	if (!needsMix)
		return;

	// Mix into the half DMA is NOT currently playing, and Setbuffer to
	// it so DMA wraps there at the next frame boundary.
	byte *buf;
	if (s_playbackState == kPlay1stHalf) {
		buf = atariSampleBuffer2ndHalf;
		Setbuffer(SR_PLAY, atariSampleBuffer2ndHalf, atariSampleBufferEnd);
	} else {
		buf = atariSampleBuffer1stHalf;
		Setbuffer(SR_PLAY, atariSampleBuffer1stHalf, atariSampleBuffer2ndHalf);
	}

	int processed = _mixer->mixCallback(_sampleBuffer, _sampleBufferSize);

	// WARNING: loopCount, src and dst are modified by the asm code
	int loopCount = processed * _outputChannels;
	const byte *src = _sampleBuffer;
	byte *dst = buf;

	if (_downsample) {
		__asm__ volatile(
			"	subq.l	#1,%0\n"
			"	bmi.b	3f\n"
			"	move.l	#32768,%%d2\n"
			"	move.l	#65535,%%d3\n"
			"	moveq	#31,%%d4\n"
			"	moveq	#0x7f,%%d5\n"
			"1:	move.l	(%1)+,%%d0\n"
			"	move.l	%%d0,%%d1\n"
			"	add.l	%%d2,%%d1\n"
			"	cmp.l	%%d3,%%d1\n"
			"	bhi.b	2f\n"
			"	asr.l	#8,%%d0\n"	// TODO: tweak (there were reports that >> 8 is too quiet)
			"	move.b	%%d0,(%2)+\n"
			"	dbra	%0,1b\n"
			"	bra.b	3f\n"
			"2:	asr.l	%%d4,%%d0\n"
			"	eor.b	%%d5,%%d0\n"
			"	move.b	%%d0,(%2)+\n"
			"	dbra	%0,1b\n"
			"3:\n"
			: "+d"(loopCount), "+a"(src), "+a"(dst) // outputs
			: // inputs
			: "d0", "d1", "d2", "d3", "d4", "d5", "cc" AND_MEMORY
		);
		memset(buf + processed * _outputChannels *2/2, 0, (_samples - processed) * _outputChannels * 2/2);
	} else {
		if (!_emulated16bitMono) {
			__asm__ volatile(
				"	subq.l	#1,%0\n"
				"	bmi.b	3f\n"
				"	move.l	#32768,%%d2\n"
				"	move.l	#65535,%%d3\n"
				"	moveq	#31,%%d4\n"
				"	move.w	#0x7fff,%%d5\n"
				"1:	move.l	(%1)+,%%d0\n"
				"	move.l	%%d0,%%d1\n"
				"	add.l	%%d2,%%d1\n"
				"	cmp.l	%%d3,%%d1\n"
				"	bhi.b	2f\n"
				"	move.w	%%d0,(%2)+\n"
				"	dbra	%0,1b\n"
				"	bra.b	3f\n"
				"2:	asr.l	%%d4,%%d0\n"
				"	eor.w	%%d5,%%d0\n"
				"	move.w	%%d0,(%2)+\n"
				"	dbra	%0,1b\n"
				"3:\n"
				: "+d"(loopCount), "+a"(src), "+a"(dst) // outputs
				: // inputs
				: "d0", "d1", "d2", "d3", "d4", "d5", "cc" AND_MEMORY
			);
			memset(buf + processed * _outputChannels * 2, 0, (_samples - processed) * _outputChannels * 2);
		} else {
			__asm__ volatile(
				"	subq.l	#1,%0\n"
				"	bmi.b	3f\n"
				"	move.l	#32768,%%d2\n"
				"	move.l	#65535,%%d3\n"
				"	moveq	#31,%%d4\n"
				"	move.w	#0x7fff,%%d5\n"
				"1:	move.l	(%1)+,%%d0\n"
				"	move.l	%%d0,%%d1\n"
				"	add.l	%%d2,%%d1\n"
				"	cmp.l	%%d3,%%d1\n"
				"	bhi.b	2f\n"
				"	move.w	%%d0,(%2)+\n"
				"	move.w	%%d0,(%2)+\n"
				"	dbra	%0,1b\n"
				"	bra.b	3f\n"
				"2:	asr.l	%%d4,%%d0\n"
				"	eor.w	%%d5,%%d0\n"
				"	move.w	%%d0,(%2)+\n"
				"	move.w	%%d0,(%2)+\n"
				"	dbra	%0,1b\n"
				"3:\n"
				: "+d"(loopCount), "+a"(src), "+a"(dst) // outputs
				: // inputs
				: "d0", "d1", "d2", "d3", "d4", "d5", "cc" AND_MEMORY
			);
			memset(buf + processed * _outputChannels * 2*2, 0, (_samples - processed) * _outputChannels * 2*2);
		}
	}

	if (processed > 0 && processed != _samples) {
		warning("processed: %d, _samples: %d", processed, _samples);
	}
}
