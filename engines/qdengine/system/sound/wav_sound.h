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

	//! Returns sound length in seconds
	float length() const { return _length; }

	bool wav_file_load(const Common::Path fname);

	Audio::SeekableAudioStream *_audioStream = nullptr;
	Common::Path _fname;

private:
	float _length = 0.0;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_SOUND_WAV_SOUND_H
