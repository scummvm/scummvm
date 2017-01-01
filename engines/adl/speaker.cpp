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

#include "common/system.h"
#include "common/events.h"

#include "engines/engine.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "adl/speaker.h"
#include "adl/adl.h"

namespace Adl {

// Number of times to duplicate each sample
#define SAMPLE_DUP 4
// Bell frequency in Hz
#define BELL_FREQ 1000
// Sample rate
#define SAMPLE_RATE (BELL_FREQ * SAMPLE_DUP * 2)
// Number of waves per 0.1 seconds (bell length)
#define BELL_WAVE_COUNT (SAMPLE_RATE / 10 / SAMPLE_DUP / 2)
// Length of bell in samples
#define BELL_LEN (BELL_WAVE_COUNT * SAMPLE_DUP * 2)
// Length of silence in samples
#define SILENCE_LEN (SAMPLE_RATE / 80)

Speaker::~Speaker() {
	delete[] _bell;
	delete[] _silence;
}

Speaker::Speaker() {
	_bell = new byte[BELL_LEN];

	byte *buf = _bell;
	for (uint i = 0; i < BELL_WAVE_COUNT; ++i) {
		for (uint j = 0; j < SAMPLE_DUP; ++j)
			*buf++ = 0x00;
		for (uint j = 0; j < SAMPLE_DUP; ++j)
			*buf++ = 0xff;
	}

	_silence = new byte[SILENCE_LEN];

	buf = _silence;
	for (uint i = 0; i < SILENCE_LEN; ++i)
		*buf++ = 0x80;
}

void Speaker::bell(uint count) {
	Audio::QueuingAudioStream *stream = Audio::makeQueuingAudioStream(SAMPLE_RATE, false);
	Audio::SoundHandle handle;

	stream->queueBuffer(_bell, BELL_LEN, DisposeAfterUse::NO, Audio::FLAG_UNSIGNED);

	for (uint i = 1; i < count; ++i) {
		stream->queueBuffer(_silence, SILENCE_LEN, DisposeAfterUse::NO, Audio::FLAG_UNSIGNED);
		stream->queueBuffer(_bell, BELL_LEN, DisposeAfterUse::NO, Audio::FLAG_UNSIGNED);
	}

	stream->finish();

	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &handle, stream);

	while (!g_engine->shouldQuit() && g_system->getMixer()->isSoundHandleActive(handle)) {
		Common::Event event;
		static_cast<AdlEngine *>(g_engine)->pollEvent(event);
		g_system->delayMillis(16);
	}
}

} // End of namespace Adl
