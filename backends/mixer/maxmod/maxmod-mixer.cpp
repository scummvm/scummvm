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
	_mixer(0),
	_freq(freq),
	_bufSize(bufSize) {

}

MaxModMixerManager::~MaxModMixerManager() {
	_mixer->setReady(false);
	mmStreamClose();
	delete _mixer;
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

	mm_stream mystream;
	mystream.sampling_rate = _freq;
	mystream.buffer_length = _bufSize / 4;
	mystream.callback = on_stream_request;
	mystream.format = MM_STREAM_16BIT_STEREO;
	mystream.timer = MM_TIMER2;
	mystream.manual = 0;

	mmStreamOpen( &mystream );

	_mixer->setReady(true);
}

#endif
