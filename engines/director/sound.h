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

#ifndef DIRECTOR_SOUND_H
#define DIRECTOR_SOUND_H

namespace Audio {
	class AudioStream;
	class SoundHandle;
	class PCSpeaker;
	class SeekableAudioStream;
}

namespace Director {

class DirectorSound {

private:
	Common::Array<Audio::SoundHandle *> _channels;
	Audio::SoundHandle *_scriptSound;
	Audio::Mixer *_mixer;
	Audio::PCSpeaker *_speaker;
	Audio::SoundHandle *_pcSpeakerHandle;

public:
	DirectorSound();
	~DirectorSound();

	void playWAV(Common::String filename, uint8 soundChannel);
	void playAIFF(Common::String filename, uint8 soundChannel);
	void playFile(Common::String filename, uint8 soundChannel);
	void playMCI(Audio::AudioStream &stream, uint32 from, uint32 to);
	void playStream(Audio::AudioStream &stream, uint8 soundChannel);
	void systemBeep();
	bool isChannelActive(uint8 soundChannel);
	void stopSound(uint8 soundChannel);
	void stopSound();
};


class SNDDecoder {
public:
	SNDDecoder();
	~SNDDecoder();

	bool loadStream(Common::SeekableSubReadStreamEndian &stream);
	Audio::SeekableAudioStream *getAudioStream();
	Audio::AudioStream *getLoopingAudioStream();

private:
	byte *_data;
	uint16 _channels;
	uint32 _size;
	uint16 _rate;
	byte _flags;
};

} // End of namespace Director

#endif
