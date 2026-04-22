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

#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "common/memstream.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/pack.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {

bool speech_system_active = false;
bool speech_on = false;
char global_speech_resource[16] = "*PHAN009.DSR";
int  global_speech_ready = -1;
Audio::AudioStream *speech_stream;


struct SpeechDir {
	int16 field0 = 0;
	int16 compression = 0;
	int16 field4 = 0, field6 = 0, field8 = 0;
	int32 size = 0;
	int32 offset = 0;

	static constexpr int SIZE = 2 + 2 + 2 + 2 + 2 + 4 + 4;
	void load(Common::SeekableReadStream *src);
};


void SpeechDir::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(field0, compression, field4, field6, field8, size, offset);
}


void speech_init() {
	speech_system_active = true;
}

void speech_shutdown() {
	if (speech_stream) {
		delete speech_stream;
		speech_stream = nullptr;
	}

	speech_system_active = false;
}

Audio::AudioStream *speech_load(const char *resName, int id, bool useMainMemory) {
	Common::MemoryReadStream *memStream;
	Audio::AudioStream *audioStream = nullptr;
	uint filePos;
	int count;
	SpeechDir speechDir;
	byte *load_buf = nullptr;
	int packing_flag;

	// Always use main memory in ScummVM
	useMainMemory = true;

	// Open the sound resource for access
	Common::SeekableReadStream *handle = env_open(resName);
	if (!handle) goto done;
	filePos = handle->pos();

	// Get the number of voice samples in the file
	count = handle->readUint16LE();

	// Validate the speech Id specified is within range
	--id;
	if (id < 0 || id >= count) goto done;

	// Seek to the correct offset and read the index entry
	if (id > 0)
		handle->seek(id * SpeechDir::SIZE, SEEK_CUR);
	speechDir.load(handle);

	// Seek to the start of the voice content
	filePos += speechDir.offset;
	handle->seek(filePos);

	// Get the buffer space
	load_buf = (byte *)malloc(speechDir.size);

	// Decompress the data
	pack_strategy = speechDir.compression;
	packing_flag = (speechDir.compression != PACK_NONE) ? PACK_EXPLODE : PACK_RAW_COPY;

	if (pack_data(packing_flag, speechDir.size, FROM_DISK, handle, TO_MEMORY, load_buf) != speechDir.size) goto done;

	// At this point we have valid data
	memStream = new Common::MemoryReadStream(load_buf, speechDir.size, DisposeAfterUse::YES);
	audioStream = Audio::makeRawStream(memStream, 11025, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);

done:
	delete handle;

	delete speech_stream;
	speech_stream = audioStream;

	return audioStream;
}

void speech_play(const char *resName, int id) {
	Audio::AudioStream *speech = speech_load(resName, id);

	if (speech)
		g_engine->playSpeech(speech);

	speech_stream = nullptr;
}

void speech_all_off() {
	g_engine->stopSpeech();
}

void speech_sample_rate(int rate) {
	// TODO: implement speech_sample_rate
}

void speech_go() {
	if (speech_stream) {
		g_engine->playSpeech(speech_stream);
		speech_stream = nullptr;
	}
}

void global_speech(int id) {
	if (speech_system_active && speech_on) {
		speech_play(global_speech_resource, id);
	}
}

void global_speech_load(int id) {
	Audio::AudioStream *chunk;

	if (speech_system_active && speech_on) {
		speech_all_off();
		chunk = speech_load(global_speech_resource, id, false);
		if (chunk != NULL) {
			global_speech_ready = id;
		} else {
			global_speech_ready = -1;
		}
	} else {
		global_speech_ready = -1;
	}
}

void global_speech_go(int id) {
	if (speech_system_active && speech_on) {
		if (global_speech_ready == id) {
			speech_all_off();
			//speech_sample_rate(speech_main_buffer.sample_rate);
			speech_go();
		} else {
			global_speech(id);
		}
	}
}

} // namespace MADSV2
} // namespace MADS
