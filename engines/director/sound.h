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

#include "audio/mixer.h"

namespace Audio {
	class AudioStream;
	class SoundHandle;
	class PCSpeaker;
	class RewindableAudioStream;
}

namespace Director {

struct FadeParams {
	int startVol;
	int targetVol;
	int totalTicks;
	int startTicks;
	int lapsedTicks;
	bool fadeIn;

	FadeParams(int sv, int tv, int tt, int st, bool f) :
		startVol(sv), targetVol(tv), totalTicks(tt), startTicks(st), lapsedTicks(0), fadeIn(f) {}
};

struct SoundChannel {
	Audio::SoundHandle handle;
	int lastPlayingCast;
	byte volume;
	FadeParams *fade;

	SoundChannel(): handle(), lastPlayingCast(0), volume(255), fade(nullptr) {}
};

class DirectorSound {

private:
	DirectorEngine *_vm;
	Common::Array<SoundChannel> _channels;
	Audio::SoundHandle *_scriptSound;
	Audio::Mixer *_mixer;
	Audio::PCSpeaker *_speaker;
	Audio::SoundHandle *_pcSpeakerHandle;

public:
	DirectorSound(DirectorEngine *vm);
	~DirectorSound();

	SoundChannel *getChannel(uint8 soundChannel);
	void playFile(Common::String filename, uint8 soundChannel);
	void playMCI(Audio::AudioStream &stream, uint32 from, uint32 to);
	void playStream(Audio::AudioStream &stream, uint8 soundChannel);
	void playCastMember(int castId, uint8 soundChannel, bool allowRepeat = true);
	void systemBeep();

	void registerFade(uint8 soundChannel, bool fadeIn, int ticks);
	bool fadeChannel(uint8 soundChannel);

	bool isChannelActive(uint8 soundChannel);
	int lastPlayingCast(uint8 soundChannel);
	void stopSound(uint8 soundChannel);
	void stopSound();

private:
	bool isChannelValid(uint8 soundChannel);
	void cancelFade(uint8 soundChannel);
};

class AudioDecoder {
public:
	virtual ~AudioDecoder() {};
public:
	virtual Audio::RewindableAudioStream *getAudioStream(DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES) = 0;
	virtual Audio::AudioStream *getLoopingAudioStream();
};

class SNDDecoder : public AudioDecoder {
public:
	SNDDecoder();
	~SNDDecoder();

	bool loadStream(Common::SeekableReadStreamEndian &stream);
	bool processCommands(Common::SeekableReadStreamEndian &stream);
	bool processBufferCommand(Common::SeekableReadStreamEndian &stream);
	Audio::RewindableAudioStream *getAudioStream(DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

private:
	byte *_data;
	uint16 _channels;
	uint32 _size;
	uint16 _rate;
	byte _flags;
};

class AudioFileDecoder: public AudioDecoder {
public:
	AudioFileDecoder(Common::String &path): _path(path) {};
	~AudioFileDecoder() {};

	void setPath(Common::String &path);

	Audio::RewindableAudioStream *getAudioStream(DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

private:
	Common::String _path;
};

} // End of namespace Director

#endif
