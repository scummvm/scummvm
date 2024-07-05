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

#include "qdengine/qd_precomp.h"
#include "qdengine/system/sound/wav_sound.h"


namespace QDEngine {

wavSound::wavSound() : data_(NULL) {
	data_length_ = 0;
	bits_per_sample_ = 0;
	channels_ = 0;
	samples_per_sec_ = 0;
}

wavSound::~wavSound() {
	free_data();
}

bool wavSound::init(int data_len, int bits, int chn, int samples) {
	free_data();

	data_length_ = data_len;
	data_ = new char[data_length_];

	channels_ = chn;
	bits_per_sample_ = bits;
	samples_per_sec_ = samples;

	return true;
}

void wavSound::free_data() {
	if (data_) {
		delete [] data_;
		data_ = NULL;
	}

	data_length_ = 0;
	bits_per_sample_ = 0;
	channels_ = 0;
	samples_per_sec_ = 0;
}
} // namespace QDEngine
