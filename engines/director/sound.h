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

class AudioDecoder;

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
	CastMemberID lastPlayingCast;
	byte volume;
	FadeParams *fade;

	// this indicate whether the sound is playing across the movie. Because the cast name may be the same while the actual sounds are changing.
	// And we will override the sound when ever the sound is changing. thus we use a flag to indicate whether the movie is changed.
	bool _movieChanged;

	SoundChannel(): handle(), volume(255), fade(nullptr), _movieChanged(false) {}
};

class DirectorSound {

private:
	DirectorEngine *_vm;
	Common::Array<SoundChannel> _channels;
	Audio::SoundHandle _scriptSound;
	Audio::Mixer *_mixer;
	Audio::PCSpeaker *_speaker;
	Audio::SoundHandle _pcSpeakerHandle;

	// these two were used in fplay xobj
	Common::Queue<Common::String> _fplayQueue;
	Common::String _currentSoundName;

	bool _enable;

public:
	DirectorSound(DirectorEngine *vm);
	~DirectorSound();

	SoundChannel *getChannel(uint8 soundChannel);
	void playFile(Common::String filename, uint8 soundChannel);
	void playMCI(Audio::AudioStream &stream, uint32 from, uint32 to);
	void playStream(Audio::AudioStream &stream, uint8 soundChannel);
	void playCastMember(CastMemberID memberID, uint8 soundChannel, bool allowRepeat = true);
	void playExternalSound(AudioDecoder *ad, uint8 soundChannel, uint8 externalSoundID);
	void playFPlaySound(const Common::Array<Common::String> &fplayList);
	void playFPlaySound();
	void setSouldLevel(int channel, uint8 soundLevel);
	uint8 getSoundLevel(uint8 soundChannel);
	void setSoundEnabled(bool enabled);
	void systemBeep();
	void changingMovie();

	void setLastPlayCast(uint8 soundChannel, CastMemberID castMemberId);
	bool checkLastPlayCast(uint8 soundChannel, const CastMemberID &castMemberId);

	bool getSoundEnabled() { return _enable; }

	Common::String getCurrentSound() { return _currentSoundName; }

	void registerFade(uint8 soundChannel, bool fadeIn, int ticks);
	bool fadeChannel(uint8 soundChannel);

	bool isChannelActive(uint8 soundChannel);
	void stopSound(uint8 soundChannel);
	void stopSound();

private:
	uint8 getChannelVolume(uint8 soundChannel);
	void setSoundLevelInternal(uint8 soundChannel, uint8 soundLevel);
	bool isChannelValid(uint8 soundChannel);
	void cancelFade(uint8 soundChannel);
};

class AudioDecoder {
public:
	AudioDecoder() {};
	virtual ~AudioDecoder() {};
public:
	virtual Audio::AudioStream *getAudioStream(bool looping = false, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES) { return nullptr; }
};

class SNDDecoder : public AudioDecoder {
public:
	SNDDecoder();
	~SNDDecoder();

	bool loadStream(Common::SeekableReadStreamEndian &stream);
	void loadExternalSoundStream(Common::SeekableReadStreamEndian &stream);
	bool processCommands(Common::SeekableReadStreamEndian &stream);
	bool processBufferCommand(Common::SeekableReadStreamEndian &stream);
	Audio::AudioStream *getAudioStream(bool looping = false, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES) override;
	bool hasLoopBounds();

private:
	byte *_data;
	uint16 _channels;
	uint32 _size;
	uint16 _rate;
	byte _flags;
	uint32 _loopStart;
	uint32 _loopEnd;
};

class AudioFileDecoder : public AudioDecoder {
public:
	AudioFileDecoder(Common::String &path);
	~AudioFileDecoder() {};

	void setPath(Common::String &path);

	Audio::AudioStream *getAudioStream(bool looping = false, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES) override;

private:
	Common::String _path;
};

} // End of namespace Director

#endif
