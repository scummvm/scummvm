/* ScummVM - Scumm Interpreter
 *
 * This file is licensed under both GPL and LGPL
 * Copyright (C) 2004-2006 The ScummVM project
 * Copyright (C) 2006 Torbjorn Andersson and Johannes Schickel
 *
 * GPL License
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
 * LPGL License
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

#ifndef SOUND_H
#define SOUND_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/file.h"
#include "common/mutex.h"

#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "sound/mixer.h"

#include "kyra/kyra.h"

namespace Audio {
class AudioStream;
class Mixer;
class SoundHandle;
} // end of namespace Audio

namespace Kyra {

class AUDStream;

class Sound {
public:
	Sound(KyraEngine *engine, Audio::Mixer *mixer);
	virtual ~Sound();

	virtual bool init() = 0;
	virtual void process() {}

	virtual void setVolume(int volume) = 0;
	virtual int getVolume() = 0;

	virtual void setSoundFileList(const char * const *list, uint s) { _soundFileList = list; _soundFileListSize = s; }
	virtual void loadSoundFile(uint file) = 0;

	virtual void playTrack(uint8 track) = 0;
	virtual void haltTrack() = 0;

	virtual void playSoundEffect(uint8 track) = 0;

	virtual void beginFadeOut() = 0;

	void enableMusic(bool enable) { _musicEnabled = enable; }
	bool musicEnabled() const { return _musicEnabled; }
	void enableSFX(bool enable) { _sfxEnabled = enable; }
	bool sfxEnabled() const { return _sfxEnabled; }

	void voicePlay(const char *file);
	void voiceUnload() {}
	bool voiceIsPlaying();
	void voiceStop();
	
protected:
	const char *soundFilename(uint file) { return (file < _soundFileListSize) ? _soundFileList[file] : ""; }
	bool _musicEnabled;
	bool _sfxEnabled;

	KyraEngine *_engine;
	Audio::Mixer *_mixer;
private:
	const char * const *_soundFileList;
	uint _soundFileListSize;

	Audio::AudioStream *_currentVocFile;
	Audio::SoundHandle _vocHandle;
	Common::File _compressHandle;

	struct SpeechCodecs {
		const char *fileext;
		Audio::AudioStream *(*streamFunc)(Common::File*, uint32);
	};

	static const SpeechCodecs _supportedCodes[];
};

class AdlibDriver;

class SoundAdlibPC : public Sound {
public:
	SoundAdlibPC(KyraEngine *engine, Audio::Mixer *mixer);
	~SoundAdlibPC();

	bool init();
	void process();

	void setVolume(int volume);
	int getVolume();
	
	void loadSoundFile(uint file);
	
	void playTrack(uint8 track);
	void haltTrack();
	
	void playSoundEffect(uint8 track);
	
	void beginFadeOut();
private:
	void play(uint8 track);

	void unk1();
	void unk2();

	AdlibDriver *_driver;

	uint8 _trackEntries[120];
	uint8 *_soundDataPtr;
	int _sfxPlayingSound;
	uint _soundFileLoaded;

	uint8 _sfxPriority;
	uint8 _sfxFourthByteOfSong;

	int _numSoundTriggers;
	const int *_soundTriggers;

	static const int _kyra1NumSoundTriggers;
	static const int _kyra1SoundTriggers[];
};

class SoundMidiPC : public MidiDriver, public Sound {
public:
	SoundMidiPC(KyraEngine *engine, Audio::Mixer *mixer, MidiDriver *driver);
	~SoundMidiPC();

	bool init() { return true; }

	void setVolume(int volume);
	int getVolume() { return _volume; }

	void loadSoundFile(uint file);

	void playTrack(uint8 track);
	void haltTrack();

	void playSoundEffect(uint8 track);

	void beginFadeOut();

	//MidiDriver interface implementation
	int open();
	void close();
	void send(uint32 b);
	void metaEvent(byte type, byte *data, uint16 length);

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo(void) { return _driver ? _driver->getBaseTempo() : 0; }

	//Channel allocation functions
	MidiChannel *allocateChannel()		{ return 0; }
	MidiChannel *getPercussionChannel()	{ return 0; }

	void setPassThrough(bool b)	{ _passThrough = b; }
	void setUseC55(bool b)		{ _useC55 = b; }

	void hasNativeMT32(bool nativeMT32) { _nativeMT32 = nativeMT32; }
	bool isMT32() { return _nativeMT32; }

private:
	void playMusic(uint8 *data, uint32 size);
	void stopMusic();
	void loadSoundEffectFile(uint file);
	void loadSoundEffectFile(uint8 *data, uint32 size);

	void stopSoundEffect();

	static void onTimer(void *data);

	MidiChannel *_channel[32];
	int _virChannel[16];
	uint8 _channelVolume[16];
	MidiDriver *_driver;
	bool _nativeMT32;
	bool _useC55;
	bool _passThrough;
	uint8 _volume;
	bool _isPlaying;
	bool _sfxIsPlaying;
	uint32 _fadeStartTime;
	bool _fadeMusicOut;
	bool _eventFromMusic;
	MidiParser *_parser;
	byte *_parserSource;
	MidiParser *_soundEffect;
	byte *_soundEffectSource;

	Common::Mutex _mutex;
};

class SoundTowns : public Sound {
public:
	SoundTowns(KyraEngine *engine, Audio::Mixer *mixer);
	~SoundTowns();

	bool init();
	void process();

	void setVolume(int) { /* TODO */ }
	int getVolume() { return 255; /* TODO */ }

	void loadSoundFile(uint file);

	void playTrack(uint8 track);
	void haltTrack();

	void playSoundEffect(uint8);

	void beginFadeOut() { /* TODO */ }
private:
	void stopSoundEffect();
	void setPitch(uint8 *&data, uint32 &size, int8 sourcePitch, int8 targetPitch);

	int _lastTrack;
	Audio::AudioStream *_currentSFX;
	Audio::SoundHandle _sfxHandle;
	int _currentTrackTable;
	bool _sfxIsPlaying;
	uint _sfxFileIndex;
	uint8 *_sfxFileData;
	uint8 *_sfxPlaybackBuffer;

	static const char *_sfxFiles[];
	static const int _sfxFilenum;
	static const uint8 _sfxBTTable[256];
	const uint8 *_sfxWDTable;	
};

class MixedSoundDriver : public Sound {
public:
	MixedSoundDriver(KyraEngine *engine, Audio::Mixer *mixer, Sound *music, Sound *sfx) : Sound(engine, mixer), _music(music), _sfx(sfx) {}
	~MixedSoundDriver() { delete _music; delete _sfx; }

	bool init() { return _music->init() | _sfx->init(); }
	void process() { _music->process(); _sfx->process(); }

	void setVolume(int volume) { _music->setVolume(volume); _sfx->setVolume(volume); }
	int getVolume() { return _music->getVolume(); }

	void setSoundFileList(const char * const*list, uint s) { _music->setSoundFileList(list, s); _sfx->setSoundFileList(list, s); }
	void loadSoundFile(uint file) { _music->loadSoundFile(file); _sfx->loadSoundFile(file); }

	void playTrack(uint8 track) { _music->playTrack(track); }
	void haltTrack() { _music->haltTrack(); }

	void playSoundEffect(uint8 track) { _sfx->playSoundEffect(track); }

	void beginFadeOut() { _music->beginFadeOut(); }
private:
	Sound *_music, *_sfx;
};

#define SOUND_STREAMS 4

class SoundDigital {
public:
	SoundDigital(KyraEngine *vm, Audio::Mixer *mixer);
	~SoundDigital();

	bool init();
	
	int playSound(Common::File *fileHandle, bool loop = false, bool fadeIn = false, int channel = -1);
	bool isPlaying(int channel);
	void stopSound(int channel);
	void beginFadeOut(int channel);
private:
	KyraEngine *_vm;
	Audio::Mixer *_mixer;

	struct Sound {
		Common::File *fileHandle;
		Audio::SoundHandle handle;
		AUDStream *stream;
	} _sounds[SOUND_STREAMS];
};

} // end of namespace Kyra

#endif
