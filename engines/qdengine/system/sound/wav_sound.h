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

#ifndef QDENGINE_SYSTEM_SOUND_WAV_SOUND_H
#define QDENGINE_SYSTEM_SOUND_WAV_SOUND_H

namespace Audio {
	class SeekableAudioStream;
}

namespace QDEngine {

//! Звук из WAV файла.
class wavSound {
public:
	wavSound();
	~wavSound();

	const char *data() const {
		return _data;
	}
	int data_length() const {
		return _data_length;
	}
	int bits_per_sample() const {
		return _bits_per_sample;
	}
	int channels() const {
		return _channels;
	}
	int samples_per_sec() const {
		return _samples_per_sec;
	}

	bool init(int data_len, int bits, int chn, int samples);
	void free_data();

	//! Возвращает true, если звук валиден (т.е. параметры допустимые).
	bool is_valid() const {
		if (_bits_per_sample != 8 && _bits_per_sample != 16) return false;
		if (_channels != 1 && _channels != 2) return false;
		if (!_samples_per_sec) return false;

		return true;
	}

	//! Возвращает длительность звука в секундах.
	float length() const {
		if (!is_valid()) return 0.0f;
		return float(_data_length / _channels / (_bits_per_sample >> 3)) / float(_samples_per_sec);
	}

	bool wav_file_load(const Common::Path fname);

	Audio::SeekableAudioStream *_audioStream = nullptr;
	Common::Path _fname;

private:

	//! Данные.
	char *_data;
	//! Длина данных.
	int _data_length;
	//! Количество бит на сэмпл (8/16).
	int _bits_per_sample;
	//! Количество каналов (1/2 - моно/стерео).
	int _channels;
	//! Частота дискретизации - количество сэмплов в секунду.
	/**
	Значения: 8.0, 11.025, 22.05, 44.1 x1000 Hz.
	 */
	int _samples_per_sec;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_SOUND_WAV_SOUND_H
