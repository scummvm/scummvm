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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/files/nuvie_io.h"
#include "ultima/nuvie/files/u6_lib_n.h"
#include "ultima/nuvie/files/u6_lzw.h"
#include "ultima/nuvie/core/converse_speech.h"
#include "ultima/nuvie/sound/sound_manager.h"

namespace Ultima {
namespace Nuvie {

ConverseSpeech::ConverseSpeech() {
	config = NULL;
}


/* Initialize global classes from the game.
 */
void ConverseSpeech::init(Configuration *cfg) {
	config = cfg;
}


ConverseSpeech::~ConverseSpeech() {
}

void ConverseSpeech::update() {
	TownsSound sound;
	SoundManager *sm = Game::get_game()->get_sound_manager();

	if (!sm->is_audio_enabled() || !sm->is_speech_enabled())
		return;

	if (!list.empty()) {
		if (sm->isSoundPLaying(handle) == false) {
			list.pop_front();
			if (!list.empty()) {
				sound = list.front();
				handle = sm->playTownsSound(sound.filename, sound.sample_num);
			}
		}
	}
}

void ConverseSpeech::play_speech(uint16 actor_num, uint16 sample_num) {
	Std::string sample_file;
	char filename[20]; // "/speech/charxxx.sam"
	TownsSound sound;
	SoundManager *sm = Game::get_game()->get_sound_manager();

	if (!sm->is_audio_enabled()  || !sm->is_speech_enabled())
		return;

//translate the converse sample number into the CHAR number in the SPEECH directory if required.

	if (actor_num == 202) //GUARDS
		actor_num = 228;

	if (actor_num == 201) //WISPS
		actor_num = 229;

	sample_num--;

	sprintf(filename, "speech%cchar%u.sam", U6PATH_DELIMITER, actor_num);

	config->pathFromValue("config/townsdir", filename, sample_file);

	DEBUG(0, LEVEL_DEBUGGING, "Loading Speech Sample %s:%d\n", sample_file.c_str(), sample_num);

	sound.filename = sample_file;
	sound.sample_num = sample_num;

	if (list.empty())
		handle = sm->playTownsSound(sample_file, sample_num);

	list.push_back(sound);

	return;
}

NuvieIOBuffer *ConverseSpeech::load_speech(Std::string filename, uint16 sample_num) {
	unsigned char *compressed_data, *raw_audio, *wav_data;
	sint16 *converted_audio;
	uint32 decomp_size;
	uint32 upsampled_size;
	sint16 sample = 0, prev_sample;
	U6Lib_n sam_file;
	U6Lzw lzw;
	NuvieIOBuffer *wav_buffer = 0;
	uint32 j, k;

	sam_file.open(filename, 4);

	compressed_data = sam_file.get_item(sample_num, NULL);
	raw_audio = lzw.decompress_buffer(compressed_data, sam_file.get_item_size(sample_num), decomp_size);

	free(compressed_data);

	if (raw_audio != NULL) {
		wav_buffer = new NuvieIOBuffer();
		upsampled_size = decomp_size + (int)floor((decomp_size - 1) / 4) * (2 + 2 + 2 + 1);

		switch ((decomp_size - 1) % 4) {
		case 1 :
			upsampled_size += 2;
			break;
		case 2 :
			upsampled_size += 4;
			break;
		case 3 :
			upsampled_size += 6;
			break;
		}

		DEBUG(0, LEVEL_DEBUGGING, "decomp_size %d, upsampled_size %d\n", decomp_size, upsampled_size);

		wav_data = (unsigned char *)malloc(upsampled_size * sizeof(sint16) + 44); // 44 = size of wav header

		wav_buffer->open(wav_data, upsampled_size * sizeof(sint16) + 44, false);
		wav_init_header(wav_buffer, upsampled_size);

		converted_audio = (sint16 *)&wav_data[44];

		prev_sample = convert_sample(raw_audio[0]);

		for (j = 1, k = 0; j < decomp_size; j++, k++) {
			converted_audio[k] = prev_sample;

			sample = convert_sample(raw_audio[j]);

			switch (j % 4) { // calculate the in-between samples using linear interpolation.
			case 0 :
			case 1 :
			case 2 :
				converted_audio[k + 1] = (sint16)(0.666 * (float)prev_sample + 0.333 * (float)sample);
				converted_audio[k + 2] = (sint16)(0.333 * (float)prev_sample + 0.666 * (float)sample);
				k += 2;
				break;
			case 3 :
				converted_audio[k + 1] = (sint16)(0.5 * (float)(prev_sample + sample));
				k += 1;
				break;
			}

			prev_sample = sample;
		}

		converted_audio[k] = sample;
	}

	free(raw_audio);

	return wav_buffer;
}

inline sint16 ConverseSpeech::convert_sample(uint16 raw_sample) {
	sint16 sample;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	sint16 temp_sample;
#endif

	if (raw_sample & 128)
		sample = ((sint16)(abs(128 - raw_sample) * 256) ^ 0xffff)  + 1;
	else
		sample = raw_sample * 256;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	temp_sample = sample >> 8;
	temp_sample |= (sample & 0xff) << 8;
	sample = temp_sample;
#endif

	return sample;
}

void ConverseSpeech::wav_init_header(NuvieIOBuffer *wav_buffer, uint32 audio_length) {
	wav_buffer->writeBuf((const unsigned char *)"RIFF", 4);
	wav_buffer->write4(36 + audio_length * 2); //length of RIFF chunk
	wav_buffer->writeBuf((const unsigned char *)"WAVE", 4);
	wav_buffer->writeBuf((const unsigned char *)"fmt ", 4);
	wav_buffer->write4(16); // length of format chunk
	wav_buffer->write2(1); // PCM encoding
	wav_buffer->write2(1); // mono
	wav_buffer->write4(44100); // sample frequency 16KHz
	wav_buffer->write4(44100 * 2); // sample rate
	wav_buffer->write2(2); // BlockAlign
	wav_buffer->write2(16); // Bits per sample

	wav_buffer->writeBuf((const unsigned char *)"data", 4);
	wav_buffer->write4(audio_length * 2); // length of data chunk

	return;
}

} // End of namespace Nuvie
} // End of namespace Ultima
