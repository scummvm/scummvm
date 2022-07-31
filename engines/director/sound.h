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

#ifndef DIRECTOR_SOUND_H
#define DIRECTOR_SOUND_H

#include "audio/mixer.h"

namespace Audio {
	class AudioStream;
	class SoundHandle;
	class PCSpeaker;
	class RewindableAudioStream;
	class LoopableAudioStream;
}

namespace Common {
	class MacResManager;
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

const uint16 kMinSampledMenu = 10;
const uint16 kMaxSampledMenu = 15;
const uint16 kNumSampledMenus = kMaxSampledMenu - kMinSampledMenu + 1;

struct ExternalSoundID {
	uint16 menu;
	uint16 submenu;

	ExternalSoundID() : menu(0), submenu(0) {}
	ExternalSoundID(uint16 menuID, uint16 submenuID)
		: menu(menuID), submenu(submenuID) {}

	bool operator==(const ExternalSoundID &b) {
		return menu == b.menu && submenu == b.submenu;
	}
	bool operator!=(const ExternalSoundID &b) {
		return !(*this == b);
	}
};

enum SoundIDType {
	kSoundCast,
	kSoundExternal
};

struct SoundID {
	SoundIDType type;
	union {
		struct {
			int member;
			int castLib;
		} cast;
		struct {
			uint16 menu;
			uint16 submenu;
		} external;
	} u;

	SoundID() {
		type = kSoundCast;
		u.cast.member = 0;
		u.cast.castLib = 0;
	}
	SoundID(SoundIDType type_, int a, int b) {
		type = type_;
		switch (type) {
		case kSoundCast:
			u.cast.member = a;
			u.cast.castLib = b;
			break;
		case kSoundExternal:
			u.external.menu = a;
			u.external.submenu = b;
		}
	}
	SoundID(CastMemberID memberID) {
		type = kSoundCast;
		u.cast.member = memberID.member;
		u.cast.castLib = memberID.castLib;
	}

	bool operator==(const SoundID &b) {
		if (type != b.type)
			return false;

		switch (type) {
		case kSoundCast:
			return u.cast.member == b.u.cast.member && u.cast.castLib == b.u.cast.castLib;
		case kSoundExternal:
			return u.external.menu == b.u.external.menu && u.external.submenu == b.u.external.submenu;
		}

		return false;
	}
	bool operator!=(const SoundID &b) {
		return !(*this == b);
	}
};

struct SoundChannel {
	Audio::SoundHandle handle;
	SoundID lastPlayedSound;
	bool stopOnZero; // Should the sound be stopped when the channel contains cast member 0?
	byte volume;
	FadeParams *fade;

	// a non-zero sound ID if the channel is a puppet. i.e. it's controlled by lingo
	SoundID puppet;
	bool newPuppet;

	// this indicate whether the sound is playing across the movie. Because the cast name may be the same while the actual sounds are changing.
	// And we will override the sound when ever the sound is changing. thus we use a flag to indicate whether the movie is changed.
	bool movieChanged;

	// Pointer for keeping track of a looping sound, used to signal
	// a stop at the end of a loop.
	Audio::LoopableAudioStream *loopPtr;

	SoundChannel(): handle(), lastPlayedSound(SoundID()), stopOnZero(true), volume(255), fade(nullptr), puppet(SoundID()), newPuppet(false), movieChanged(false), loopPtr(nullptr) {}
};

class DirectorSound {

private:
	Window *_window;
	Common::Array<SoundChannel> _channels;
	Audio::SoundHandle _scriptSound;
	Audio::Mixer *_mixer;
	Audio::PCSpeaker *_speaker;
	Audio::SoundHandle _pcSpeakerHandle;

	// these two were used in fplay xobj
	Common::Queue<Common::String> _fplayQueue;
	Common::String _currentSoundName;

	bool _enable;

	Common::Array<AudioDecoder *> _sampleSounds[kNumSampledMenus];

public:
	DirectorSound(Window *window);
	~DirectorSound();

	SoundChannel *getChannel(uint8 soundChannel);
	void playFile(Common::String filename, uint8 soundChannel);
	void playMCI(Audio::AudioStream &stream, uint32 from, uint32 to);
	void playStream(Audio::AudioStream &stream, uint8 soundChannel);
	void playSound(SoundID soundId, uint8 soundChannel, bool forPuppet = false);
	void playCastMember(CastMemberID memberID, uint8 soundChannel, bool forPuppet = false);
	void playExternalSound(uint16 menu, uint16 submenu, uint8 soundChannel);
	void playFPlaySound(const Common::Array<Common::String> &fplayList);
	void playFPlaySound();
	void setSoundLevel(int channel, uint8 soundLevel);
	uint8 getSoundLevel(uint8 soundChannel);
	void setSoundEnabled(bool enabled);
	void systemBeep();
	void changingMovie();

	void loadSampleSounds(uint type);
	void unloadSampleSounds();

	void setLastPlayedSound(uint8 soundChannel, SoundID soundId, bool stopOnZero = true);
	bool isLastPlayedSound(uint8 soundChannel, const SoundID &soundId);
	bool shouldStopOnZero(uint8 soundChannel);

	bool isChannelPuppet(uint8 soundChannel);
	void setPuppetSound(SoundID soundId, uint8 soundChannel);
	void playPuppetSound(uint8 soundChannel);

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
	virtual Audio::AudioStream *getAudioStream(bool looping = false, bool forPuppet = false, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES) { return nullptr; }
};

class SNDDecoder : public AudioDecoder {
public:
	SNDDecoder();
	~SNDDecoder();

	bool loadStream(Common::SeekableReadStreamEndian &stream);
	void loadExternalSoundStream(Common::SeekableReadStreamEndian &stream);
	bool processCommands(Common::SeekableReadStreamEndian &stream);
	bool processBufferCommand(Common::SeekableReadStreamEndian &stream);
	Audio::AudioStream *getAudioStream(bool looping = false, bool forPuppet = false, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES) override;
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
	~AudioFileDecoder();

	void setPath(Common::String &path);

	Audio::AudioStream *getAudioStream(bool looping = false, bool forPuppet = false, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES) override;

private:
	Common::String _path;
	Common::MacResManager *_macresman;
};

} // End of namespace Director

#endif
