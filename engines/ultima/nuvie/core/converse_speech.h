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

#ifndef NUVIE_CORE_CONVERSE_SPEECH_H
#define NUVIE_CORE_CONVERSE_SPEECH_H

#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"
#include "audio/mixer.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class U6Lib_n;
class U6Lzw;
class NuvieIOBuffer;

using Std::string;

typedef struct TownsSound {
	Std::string filename;
	uint16 sample_num;
} TownsSound;

class ConverseSpeech {
	// game system objects from nuvie
	Configuration *config;
	Audio::SoundHandle handle;
	Std::list<TownsSound> list;

public:
	ConverseSpeech();
	~ConverseSpeech();
	void init(Configuration *cfg);
	void update();
	void play_speech(uint16 actor_num, uint16 sample_num);

protected:
	NuvieIOBuffer *load_speech(Std::string filename, uint16 sample_num);
	inline sint16 convert_sample(uint16 raw_sample);
	void wav_init_header(NuvieIOBuffer *wav_buffer, uint32 audio_length);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
