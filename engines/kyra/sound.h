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
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_SOUND_H
#define KYRA_SOUND_H

#include "kyra/kyra_v1.h"

#include "common/scummsys.h"
#include "common/str.h"

#include "sound/mixer.h"

namespace Audio {
class AudioStream;
} // End of namespace Audio

namespace Kyra {

/**
 * Analog audio output device API for Kyrandia games.
 * It contains functionality to play music tracks,
 * sound effects and voices.
 */
class Sound {
public:
	Sound(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	virtual ~Sound();

	enum kType {
		kAdlib,
		kMidiMT32,
		kMidiGM,
		kTowns,
		kPC98,
		kPCSpkr,
		kAmiga
	};

	virtual kType getMusicType() const = 0;
	virtual kType getSfxType() const { return getMusicType(); }

	/**
	 * Initializes the output device.
	 *
	 * @return true on success, else false
	 */
	virtual bool init() = 0;

	/**
	 * Updates the device, this is needed for some devices.
	 */
	virtual void process() {}

	/**
	 * Updates internal volume settings according to ConfigManager.
	 */
	virtual void updateVolumeSettings() {}

	/**
	 * Sets the soundfiles the output device will use
	 * when playing a track and/or sound effect.
	 *
	 * @param list	soundfile list
	 */
	virtual void setSoundList(const AudioDataStruct *list) { _soundDataList = list; }

	/**
	 * Checks if a given sound file is present.
	 *
	 * @param track	track number
	 * @return true if available, false otherwise
	 */
	virtual bool hasSoundFile(uint file) { return (fileListEntry(file) != 0); }

	/**
	 * Load a specifc sound file for use of
	 * playing music and sound effects.
	 */
	virtual void loadSoundFile(uint file) = 0;

	/**
	 * Load a sound file for playing music
	 * (and somtimes sound effects) from.
	 */
	virtual void loadSoundFile(Common::String file) = 0;

	/**
	 * Load a sound file for playing sound
	 * effects from.
	 */
	virtual void loadSfxFile(Common::String file) {}

	/**
	 * Plays the specified track.
	 *
	 * @param track	track number
	 */
	virtual void playTrack(uint8 track) = 0;

	/**
	 * Stop playback of the current track.
	 */
	virtual void haltTrack() = 0;

	/**
	 * Plays the specified sound effect.
	 *
	 * @param track	sound effect id
	 */
	virtual void playSoundEffect(uint8 track) = 0;

	/**
	 * Stop playback of all sfx tracks.
	 */
	virtual void stopAllSoundEffects() {}

	/**
	 * Checks if the sound driver plays any sound.
	 *
	 * @return true if playing, false otherwise
	 */
	virtual bool isPlaying() const { return false; }

	/**
	 * Starts fading out the volume.
	 *
	 * This keeps fading out the output until
	 * it is silenced, but does not change
	 * the volume set by setVolume! It will
	 * automatically reset the volume when
	 * playing a new track or sound effect.
	 */
	virtual void beginFadeOut() = 0;

	void enableMusic(int enable) { _musicEnabled = enable; }
	int musicEnabled() const { return _musicEnabled; }
	void enableSFX(bool enable) { _sfxEnabled = enable; }
	bool sfxEnabled() const { return _sfxEnabled; }

	virtual bool voiceFileIsPresent(const char *file);

	/**
	 * Checks whether a voice file with the given name is present
	 *
	 * @param file		file name
	 * @return true if available, false otherwise
	 */
	bool isVoicePresent(const char *file);

	/**
	 * Plays the specified voice file.
	 *
	 * Also before starting to play the
	 * specified voice file, it stops the
	 * current voice.
	 *
	 * @param file		file to be played
	 * @param volume	volume of the voice file
	 * @param isSfx		marks file as sfx instead of voice
	 * @param handle	store a copy of the sound handle
	 * @return playtime of the voice file (-1 marks unknown playtime)
	 */
	virtual int32 voicePlay(const char *file, Audio::SoundHandle *handle = 0, uint8 volume = 255, bool isSfx = false);

	Audio::AudioStream *getVoiceStream(const char *file);

	bool playVoiceStream(Audio::AudioStream *stream, Audio::SoundHandle *handle = 0, uint8 volume = 255, bool isSfx = false);

	/**
	 * Checks if a voice is being played.
	 *
	 * @return true when playing, else false
	 */
	bool voiceIsPlaying(const Audio::SoundHandle *handle = 0);

	/**
	 * Checks if all voice handles are used.
	 *
	 * @return false when a handle is free, else true
	 */
	bool allVoiceChannelsPlaying();

	/**
	 * Checks how long a voice has been playing
	 *
	 * @return time in milliseconds
	 */
	uint32 voicePlayedTime(const Audio::SoundHandle &handle) {
		return _mixer->getSoundElapsedTime(handle);
	}

	/**
	 * Stops playback of the current voice.
	 */
	void voiceStop(const Audio::SoundHandle *handle = 0);
protected:
	const char *fileListEntry(int file) const { return (_soundDataList != 0 && file >= 0 && file < _soundDataList->fileListLen) ? _soundDataList->fileList[file] : ""; }
	int fileListLen() const { return _soundDataList->fileListLen; }
	const void *cdaData() const { return _soundDataList != 0 ? _soundDataList->cdaTracks : 0; }
	int cdaTrackNum() const { return _soundDataList != 0 ? _soundDataList->cdaNumTracks : 0; }
	int extraOffset() const { return _soundDataList != 0 ? _soundDataList->extraOffset : 0; }

	enum {
		kNumChannelHandles = 4
	};

	Audio::SoundHandle _soundChannels[kNumChannelHandles];

	int _musicEnabled;
	bool _sfxEnabled;

	KyraEngine_v1 *_vm;
	Audio::Mixer *_mixer;

private:
	const AudioDataStruct *_soundDataList;

	struct SpeechCodecs {
		const char *fileext;
		Audio::AudioStream *(*streamFunc)(
			Common::SeekableReadStream *stream,
			bool disposeAfterUse,
			uint32 startTime,
			uint32 duration,
			uint numLoops);
	};

	static const SpeechCodecs _supportedCodecs[];
};

class MixedSoundDriver : public Sound {
public:
	MixedSoundDriver(KyraEngine_v1 *vm, Audio::Mixer *mixer, Sound *music, Sound *sfx) : Sound(vm, mixer), _music(music), _sfx(sfx) {}
	~MixedSoundDriver() { delete _music; delete _sfx; }

	kType getMusicType() const { return _music->getMusicType(); }
	kType getSfxType() const { return _sfx->getSfxType(); }

	bool init() { return (_music->init() && _sfx->init()); }
	void process() { _music->process(); _sfx->process(); }

	void updateVolumeSettings() { _music->updateVolumeSettings(); _sfx->updateVolumeSettings(); }

	void setSoundList(const AudioDataStruct * list) { _music->setSoundList(list); _sfx->setSoundList(list); }
	bool hasSoundFile(uint file) const { return _music->hasSoundFile(file) && _sfx->hasSoundFile(file); }
	void loadSoundFile(uint file) { _music->loadSoundFile(file); _sfx->loadSoundFile(file); }
	void loadSoundFile(Common::String file) { _music->loadSoundFile(file); _sfx->loadSoundFile(file); }
	
	void loadSfxFile(Common::String file) { _sfx->loadSoundFile(file); }

	void playTrack(uint8 track) { _music->playTrack(track); }
	void haltTrack() { _music->haltTrack(); }
	bool isPlaying() const { return _music->isPlaying() | _sfx->isPlaying(); }

	void playSoundEffect(uint8 track) { _sfx->playSoundEffect(track); }

	void stopAllSoundEffects() { _sfx->stopAllSoundEffects(); }

	void beginFadeOut() { _music->beginFadeOut(); }
private:
	Sound *_music, *_sfx;
};

// Digital Audio
class AUDStream;
class KyraAudioStream;
class KyraEngine_MR;

/**
 * Digital audio output device.
 *
 * This is just used for Kyrandia 3.
 */
class SoundDigital {
public:
	SoundDigital(KyraEngine_MR *vm, Audio::Mixer *mixer);
	~SoundDigital();

	bool init() { return true; }

	/**
	 * Plays a sound.
	 *
	 * @param filename		file to be played
	 * @param priority		priority of the sound
	 * @param type			type
	 * @param volume		channel volume
	 * @param loop			true if the sound should loop (endlessly)
	 * @param channel		tell the sound player to use a specific channel for playback
	 *
	 * @return channel playing the sound
	 */
	int playSound(const char *filename, uint8 priority, Audio::Mixer::SoundType type, int volume = 255, bool loop = false, int channel = -1);

	/**
	 * Checks if a given channel is playing a sound.
	 *
	 * @param channel	channel number to check
	 * @return true if playing, else false
	 */
	bool isPlaying(int channel);

	/**
	 * Stop the playback of a sound in the given
	 * channel.
	 *
	 * @param channel	channel number
	 */
	void stopSound(int channel);

	/**
	 * Stops playback of all sounds.
	 */
	void stopAllSounds();

	/**
	 * Makes the sound in a given channel
	 * fading out.
	 *
	 * @param channel	channel number
	 * @param ticks		fadeout time
	 */
	void beginFadeOut(int channel, int ticks);
private:
	KyraEngine_MR *_vm;
	Audio::Mixer *_mixer;

	struct Sound {
		Audio::SoundHandle handle;

		char filename[16];
		uint8 priority;
		KyraAudioStream *stream;
	} _sounds[4];

	struct AudioCodecs {
		const char *fileext;
		Audio::AudioStream *(*streamFunc)(
			Common::SeekableReadStream *stream,
			bool disposeAfterUse,
			uint32 startTime,
			uint32 duration,
			uint numLoops);
	};

	static const AudioCodecs _supportedCodecs[];
};

} // End of namespace Kyra

#endif
