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

// Allow use of stuff in <nds.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "common/scummsys.h"

#if defined(__DS__)

#include "backends/mixer/maxmod/maxmod-mixer.h"
#include "common/system.h"

#include <nds.h>
#include <maxmod9.h>

MaxModMixerManager::MaxModMixerManager(int freq, int bufSize)
	:
	_freq(freq),
	_bufSize(bufSize) {

}

MaxModMixerManager::~MaxModMixerManager() {
	// HACK: This is called during the OSystem destructor, but Audio::MixerImpl calls
	// mutex functions from OSystem during its destructor, which causes a crash.
	// _mixer->setReady(false);
	_mixer = 0;
	mmStreamClose();
}

mm_word on_stream_request( mm_word length, mm_addr dest, mm_stream_formats format ) {
	Audio::MixerImpl *mixer = (Audio::MixerImpl *)g_system->getMixer();
	assert(mixer);
	mixer->mixCallback((byte *)dest, length * 4);
	return length;
}

void MaxModMixerManager::init() {
	_mixer = new Audio::MixerImpl(_freq);
	assert(_mixer);

	mm_ds_system sys;
	sys.mod_count 			= 0;
	sys.samp_count			= 0;
	sys.mem_bank			= 0;
	sys.fifo_channel		= FIFO_MAXMOD;
	mmInit( &sys );

	_stream.sampling_rate = _freq;
	_stream.buffer_length = _bufSize / 4;
	_stream.callback = on_stream_request;
	_stream.format = MM_STREAM_16BIT_STEREO;
	_stream.timer = MM_TIMER2;
	_stream.manual = 1;

	mmStreamOpen( &_stream );

	_mixer->setReady(true);
}

void MaxModMixerManager::suspendAudio() {
	mmStreamClose();
	_audioSuspended = true;
}

int MaxModMixerManager::resumeAudio() {
	if (!_audioSuspended)
		return -2;

	mmStreamOpen( &_stream );
	_audioSuspended = false;
	return 0;
}

void MaxModMixerManager::updateAudio() {
	if (!_audioSuspended)
		mmStreamUpdate();
}

#endif
