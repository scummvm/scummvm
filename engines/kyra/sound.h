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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * LGPL License
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_SOUND_H
#define KYRA_SOUND_H

#include "kyra/kyra_v1.h"

#include "common/scummsys.h"
#include "common/file.h"
#include "common/mutex.h"
#include "common/ptr.h"

#include "sound/midiparser.h"
#include "sound/mixer.h"
#include "sound/softsynth/ym2612.h"

namespace Audio {
class AudioStream;
} // end of namespace Audio

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
		kPC98
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
	 * and sound effects from.
	 */
	virtual void loadSoundFile(Common::String file) = 0;

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
	 * Plays the specified voice file.
	 *
	 * Also before starting to play the
	 * specified voice file, it stops the
	 * current voice.
	 *
	 * TODO: add support for queueing voice
	 * files
	 *
	 * @param file	file to be played
	 * @param isSfx marks file as sfx instead of voice
	 * @return playtime of the voice file (-1 marks unknown playtime)
	 */
	virtual int32 voicePlay(const char *file, bool isSfx = false);

	/**
	 * Checks if a voice is being played.
	 *
	 * @return true when playing, else false
	 */
	bool voiceIsPlaying(const char *file = 0);

	/**
	 * Checks how long a voice has been playing
	 *
	 * @return time in milliseconds
	 */
	uint32 voicePlayedTime(const char *file);

	/**
	 * Stops playback of the current voice.
	 */
	void voiceStop(const char *file = 0);
protected:
	const char *fileListEntry(int file) const { return (_soundDataList != 0 && file >= 0 && file < _soundDataList->_fileListLen) ? _soundDataList->_fileList[file] : ""; }
	const void *cdaData() const { return _soundDataList != 0 ? _soundDataList->_cdaTracks : 0; }
	int cdaTrackNum() const { return _soundDataList != 0 ? _soundDataList->_cdaNumTracks : 0; }

	enum {
		kNumChannelHandles = 4
	};
	
	struct SoundChannel {
		Common::String file;
		Audio::SoundHandle channelHandle;
	};
	SoundChannel _soundChannels[kNumChannelHandles];

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

class AdlibDriver;

/**
 * AdLib implementation of the sound output device.
 *
 * It uses a special sound file format special to
 * Dune II, Kyrandia 1 and 2. While Dune II and
 * Kyrandia 1 are using exact the same format, the
 * one of Kyrandia 2 slightly differs.
 *
 * See AdlibDriver for more information.
 * @see AdlibDriver
 */
class SoundAdlibPC : public Sound {
public:
	SoundAdlibPC(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundAdlibPC();

	kType getMusicType() const { return kAdlib; }

	bool init();
	void process();

	void loadSoundFile(uint file);
	void loadSoundFile(Common::String file);

	void playTrack(uint8 track);
	void haltTrack();
	bool isPlaying();

	void playSoundEffect(uint8 track);

	void beginFadeOut();
private:
	void internalLoadFile(Common::String file);

	void play(uint8 track);

	void unk1();
	void unk2();

	AdlibDriver *_driver;

	bool _v2;
	uint8 _trackEntries[500];
	uint8 *_soundDataPtr;
	int _sfxPlayingSound;

	Common::String _soundFileLoaded;

	uint8 _sfxPriority;
	uint8 _sfxFourthByteOfSong;

	int _numSoundTriggers;
	const int *_soundTriggers;

	static const int _kyra1NumSoundTriggers;
	static const int _kyra1SoundTriggers[];
};

class MidiOutput;

/**
 * MIDI output device.
 *
 * This device supports both MT-32 MIDI, as used in
 * Kyrandia 1 and 2, and GM MIDI, as used in Kyrandia 2.
 */
class SoundMidiPC : public Sound {
public:
	SoundMidiPC(KyraEngine_v1 *vm, Audio::Mixer *mixer, MidiDriver *driver);
	~SoundMidiPC();

	kType getMusicType() const { return _nativeMT32 ? kMidiMT32 : kMidiGM; }

	bool init();

	void updateVolumeSettings();

	void loadSoundFile(uint file);
	void loadSoundFile(Common::String file);

	void playTrack(uint8 track);
	void haltTrack();

	void playSoundEffect(uint8 track);

	void beginFadeOut();

	void hasNativeMT32(bool nativeMT32);
private:
	static void onTimer(void *data);

	// Our channel handling
	uint8 _musicVolume;
	uint8 _sfxVolume;

	uint32 _fadeStartTime;
	bool _fadeMusicOut;

	// Midi file related
	byte *_musicFile, *_sfxFile;
	uint32 _musicFileSize, _sfxFileSize;

	MidiParser *_music;
	MidiParser *_sfx[3];

	// misc
	bool _nativeMT32;
	bool _useC55;
	MidiDriver *_driver;
	MidiOutput *_output;

	Common::Mutex _mutex;
};

class Towns_EuphonyDriver;
class TownsPC98_OpnDriver;

class SoundTowns : public MidiDriver, public Sound {
public:
	SoundTowns(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundTowns();

	kType getMusicType() const { return kTowns; }

	bool init();
	void process();

	void loadSoundFile(uint file);
	void loadSoundFile(Common::String) {}

	void playTrack(uint8 track);
	void haltTrack();

	void playSoundEffect(uint8);

	void beginFadeOut();

	//MidiDriver interface implementation
	int open();
	void close();
	void send(uint32 b);
	void metaEvent(byte type, byte *data, uint16 length) {}

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo(void);

	//Channel allocation functions
	MidiChannel *allocateChannel()		{ return 0; }
	MidiChannel *getPercussionChannel()	{ return 0; }

	static float calculatePhaseStep(int8 semiTone, int8 semiToneRootkey,
		uint32 sampleRate, uint32 outputRate, int32 pitchWheel);

private:
	bool loadInstruments();
	void playEuphonyTrack(uint32 offset, int loop);

	static void onTimer(void *data);

	int _lastTrack;
	Audio::AudioStream *_currentSFX;
	Audio::SoundHandle _sfxHandle;

	uint _sfxFileIndex;
	uint8 *_sfxFileData;

	Towns_EuphonyDriver * _driver;
	MidiParser * _parser;

	Common::Mutex _mutex;

	const uint8 *_sfxBTTable;
	const uint8 *_sfxWDTable;
};

class SoundPC98 : public Sound {
public:
	SoundPC98(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundPC98();

	virtual kType getMusicType() const { return kPC98; }

	bool init();
	
	void process() {}
	void loadSoundFile(uint file) {}
	void loadSoundFile(Common::String) {}

	void playTrack(uint8 track);
	void haltTrack();
	void beginFadeOut();

	int32 voicePlay(const char *file, bool isSfx = false) { return -1; }
	void playSoundEffect(uint8);

protected:
	int _lastTrack;
	uint8 *_musicTrackData;
	uint8 *_sfxTrackData;
	TownsPC98_OpnDriver *_driver;
};

class SoundTownsPC98_v2 : public Sound {
public:
	SoundTownsPC98_v2(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundTownsPC98_v2();

	kType getMusicType() const { return _vm->gameFlags().platform == Common::kPlatformFMTowns ? kTowns : kPC98; }

	bool init();
	void process();

	void loadSoundFile(uint file) {}
	void loadSoundFile(Common::String file);

	void playTrack(uint8 track);
	void haltTrack();
	void beginFadeOut();

	int32 voicePlay(const char *file, bool isSfx = false);
	void playSoundEffect(uint8 track);

protected:
	Audio::AudioStream *_currentSFX;
	int _lastTrack;
	bool _useFmSfx;

	uint8 *_musicTrackData;
	uint8 *_sfxTrackData;
	TownsPC98_OpnDriver *_driver;	
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

	void playTrack(uint8 track) { _music->playTrack(track); }
	void haltTrack() { _music->haltTrack(); }
	bool isPlaying() const { return _music->isPlaying() | _sfx->isPlaying(); }

	void playSoundEffect(uint8 track) { _sfx->playSoundEffect(track); }

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

} // end of namespace Kyra

#endif
