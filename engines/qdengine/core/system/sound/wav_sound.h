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

#ifndef QDENGINE_CORE_SYSTEM_SOUND_WAV_SOUND_H
#define QDENGINE_CORE_SYSTEM_SOUND_WAV_SOUND_H

#include "audio/decoders/wave.h"

namespace QDEngine {

//! Звук из WAV файла.
class wavSound {
public:
	wavSound();
	~wavSound();

	const char *data() const {
		return data_;
	}
	int data_length() const {
		return data_length_;
	}
	int bits_per_sample() const {
		return bits_per_sample_;
	}
	int channels() const {
		return channels_;
	}
	int samples_per_sec() const {
		return samples_per_sec_;
	}

	bool init(int data_len, int bits, int chn, int samples);
	void free_data();

	//! Возвращает true, если звук валиден (т.е. параметры допустимые).
	bool is_valid() const {
		if (bits_per_sample_ != 8 && bits_per_sample_ != 16) return false;
		if (channels_ != 1 && channels_ != 2) return false;
		if (!samples_per_sec_) return false;

		return true;
	}

	//! Возвращает длительность звука в секундах.
	float length() const {
		if (!is_valid()) return 0.0f;
		return float(data_length_ / channels_ / (bits_per_sample_ >> 3)) / float(samples_per_sec_);
	}

	Audio::SeekableAudioStream *audioStream() const {
		return _audiostream;
	}

private:

	//! Данные.
	char *data_;
	//! Длина данных.
	int data_length_;
	//! Количество бит на сэмпл (8/16).
	int bits_per_sample_;
	//! Количество каналов (1/2 - моно/стерео).
	int channels_;
	//! Частота дискретизации - количество сэмплов в секунду.
	/**
	Значения: 8.0, 11.025, 22.05, 44.1 x1000 Hz.
	 */
	int samples_per_sec_;

	Audio::SeekableAudioStream *_audiostream;

	friend bool wav_file_load(const char *fname, class wavSound *snd);

};

} // namespace QDEngine

#endif // QDENGINE_CORE_SYSTEM_SOUND_WAV_SOUND_H
