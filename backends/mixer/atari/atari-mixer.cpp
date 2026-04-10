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

#include <math.h>
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
	_atariSampleBuffer = _atariPhysicalSampleBuffer = _atariLogicalSampleBuffer = nullptr;

	delete[] _sampleBuf;
	_sampleBuf = nullptr;
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

	debug("setting %d Hz mixing frequency (%d-bit, %s)",
		_outputRate,
		obtained.format == USoundFormatSigned8 ? 8 : 16,
		_outputChannels == 2
			? "stereo"
			: _emulated16bitMono
				? "mono (emulated)"
				: "mono");
	debug("sample buffer size: %d", _samples);

	_atariSampleBuffer = (byte*)Mxalloc(obtained.size * 2, MX_STRAM);
	if (!_atariSampleBuffer)
		error("Failed to allocate memory in ST RAM");

	_atariPhysicalSampleBuffer = _atariSampleBuffer;
	_atariLogicalSampleBuffer = _atariSampleBuffer + obtained.size;

	Setinterrupt(SI_TIMERA, SI_PLAY);
	Xbtimer(XB_TIMERA, 1<<3, 1, timerA);	// event count mode, count to '1'
	Jenabint(MFP_TIMERA);

	// route both mic channels to the ADC
	Soundcmd(ADCINPUT, 0);
	// enable and mix both sources (ADC and connection matrix) to the output
	Soundcmd(ADDERIN, MATIN|ADCIN);

	_sampleBufSize = _samples * _outputChannels * 4;	// always 32-bit
	_sampleBuf = new uint8[_sampleBufSize];

	_mixer = new Audio::MixerImpl(_outputRate, _outputChannels == 2, _samples, 4, false);
	_mixer->setReady(true);

	resumeAudio();
}

void AtariMixerManager::suspendAudio() {
	debug("suspendAudio");

	Buffoper(0x00);
	muted = true;
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
		if (!muted) {
			Buffoper(0x00);
			muted = true;
			debug("silencing the mixer");
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
		processed = _mixer->mixCallback(_sampleBuf, _sampleBufSize);
	}

	if (processed > 0) {
		byte* tmp = _atariPhysicalSampleBuffer;
		_atariPhysicalSampleBuffer = _atariLogicalSampleBuffer;
		_atariLogicalSampleBuffer = tmp;

		// WARNING: loopCount, src and dst are modified by the asm code
		int loopCount = processed * _outputChannels;
		const byte *src = _sampleBuf;
		byte *dst = _atariPhysicalSampleBuffer;
		if (_downsample) {
			__asm__ volatile(
				"	move.l	#32768,%%d2\n"
				"	move.l	#65535,%%d3\n"
				"	subq.l	#1,%0\n"
				"1:	move.l	(%1)+,%%d0\n"
				"	move.l	%%d0,%%d1\n"
				"	add.l	%%d2,%%d1\n"
				"	cmp.l	%%d3,%%d1\n"
				"	bhi.b	3f\n"
				"2:	asr.l	#8,%%d0\n"	// TODO: tweak (there were reports that >> 8 is too quiet)
				"	move.b	%%d0,(%2)+\n"
				"	dbra	%0,1b\n"
				"	bra.b	4f\n"
				"3:	tst.l	%%d0\n"
				"	spl		%%d0\n"
				"	ext.w	%%d0\n"
				"	add.w	%%d2,%%d0\n"
				"	bra.b	2b\n"
				"4:\n"
				: "+d"(loopCount), "+a"(src), "+a"(dst) // outputs
				: // inputs
				: "d0", "d1", "d2", "d3", "cc" AND_MEMORY
			);
			memset(_atariPhysicalSampleBuffer + processed * _outputChannels * 2/2, 0, (_samples - processed) * _outputChannels * 2/2);
			Setbuffer(SR_PLAY, _atariPhysicalSampleBuffer, _atariPhysicalSampleBuffer + _samples * _outputChannels * 2/2);
		} else {
			int bytesPerFrame = _outputChannels * 2;

			if (!_emulated16bitMono) {
				__asm__ volatile(
					"	move.l	#32768,%%d2\n"
					"	move.l	#65535,%%d3\n"
					"	subq.l	#1,%0\n"
					"1:	move.l	(%1)+,%%d0\n"
					"	move.l	%%d0,%%d1\n"
					"	add.l	%%d2,%%d1\n"
					"	cmp.l	%%d3,%%d1\n"
					"	bhi.b	3f\n"
					"2:	move.w	%%d0,(%2)+\n"
					"	dbra	%0,1b\n"
					"	bra.b	4f\n"
					"3:	tst.l	%%d0\n"
					"	spl		%%d0\n"
					"	ext.w	%%d0\n"
					"	add.w	%%d2,%%d0\n"
					"	bra.b	2b\n"
					"4:\n"
					: "+d"(loopCount), "+a"(src), "+a"(dst) // outputs
					: // inputs
					: "d0", "d1", "d2", "d3", "cc" AND_MEMORY
				);
			} else {
				bytesPerFrame *= 2;

				__asm__ volatile(
					"	move.l	#32768,%%d2\n"
					"	move.l	#65535,%%d3\n"
					"	subq.l	#1,%0\n"
					"1:	move.l	(%1)+,%%d0\n"
					"	move.l	%%d0,%%d1\n"
					"	add.l	%%d2,%%d1\n"
					"	cmp.l	%%d3,%%d1\n"
					"	bhi.b	3f\n"
					"2:	move.w	%%d0,(%2)+\n"
					"	move.w	%%d0,(%2)+\n"
					"	dbra	%0,1b\n"
					"	bra.b	4f\n"
					"3:	tst.l	%%d0\n"
					"	spl		%%d0\n"
					"	ext.w	%%d0\n"
					"	add.w	%%d2,%%d0\n"
					"	bra.b	2b\n"
					"4:\n"
					: "+d"(loopCount), "+a"(src), "+a"(dst) // outputs
					: // inputs
					: "d0", "d1", "d2", "d3", "cc" AND_MEMORY
				);
			}
			const int bufferSize = processed * bytesPerFrame;
			memset(_atariPhysicalSampleBuffer + bufferSize, 0, (_samples - processed) * bytesPerFrame);
			Setbuffer(SR_PLAY, _atariPhysicalSampleBuffer, _atariPhysicalSampleBuffer + _samples * bytesPerFrame);
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
		warning("processed: %d, _samples: %d", processed, _samples);
	}
}
