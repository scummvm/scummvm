/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef SOUND_H
#define SOUND_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "sound/mixer.h"
#include "kyra/kyra.h"

class AudioStream;

namespace Audio {
class Mixer;
class SoundHandle;
} // end of namespace Audio

namespace Kyra {

class Sound {
public:
	Sound(KyraEngine *engine, Audio::Mixer *mixer);
	virtual ~Sound();

	virtual bool init() = 0;
	
	virtual void setVolume(int volume) = 0;
	virtual int getVolume() = 0;
	
	virtual void playMusic(const char *file) = 0;
	virtual void stopMusic() = 0;
	
	virtual void playTrack(uint8 track, bool looping = true) = 0;
	virtual void haltTrack() = 0;
	virtual void startTrack() = 0;
	
	virtual void loadSoundEffectFile(const char *file) = 0;
	virtual void stopSoundEffect() = 0;
	
	virtual void playSoundEffect(uint8 track) = 0;
	
	virtual void beginFadeOut() = 0;
	virtual bool fadeOut() = 0;
	
	void voicePlay(const char *file);
	void voiceUnload() {}
	bool voiceIsPlaying();

protected:
	KyraEngine *_engine;
	Audio::Mixer *_mixer;

private:
	AudioStream *_currentVocFile;
	Audio::SoundHandle _vocHandle;
	Common::File _compressHandle;
	
	struct SpeechCodecs {
		const char *fileext;
		AudioStream *(*streamFunc)(Common::File*, uint32);
	};
	
	static const SpeechCodecs _supportedCodes[];
};

class AdlibDriver;

class SoundAdlibPC : public Sound {
public:
	SoundAdlibPC(Audio::Mixer *mixer, KyraEngine *engine);
	~SoundAdlibPC();

	bool init();

	void setVolume(int volume);
	int getVolume();
	
	void playMusic(const char *file);
	void stopMusic();
	
	void playTrack(uint8 track, bool looping);
	void haltTrack();
	void startTrack();
	
	void loadSoundEffectFile(const char *file);
	void stopSoundEffect();
	
	void playSoundEffect(uint8 track);
	
	void beginFadeOut();
	bool fadeOut();
private:
	void loadSoundFile(const char *file);

	AdlibDriver *_driver;

	uint8 _trackEntries[120];
	uint8 *_soundDataPtr;
	int _sfxPlayingSound;
	Common::String _soundFileLoaded;

	uint8 _sfxSecondByteOfSong;
	uint8 _sfxFourthByteOfSong;
};

class SoundMidiPC : public MidiDriver, public Sound {
public:

	SoundMidiPC(MidiDriver *driver, Audio::Mixer *mixer, KyraEngine *engine);
	~SoundMidiPC();

	bool init() { return true; }

	void setVolume(int volume);
	int getVolume() { return _volume; }

	void hasNativeMT32(bool nativeMT32) { _nativeMT32 = nativeMT32; }
	bool isMT32() { return _nativeMT32; }

	void playMusic(const char *file);
	void playMusic(uint8 *data, uint32 size);
	void stopMusic();

	void playTrack(uint8 track, bool looping);
	void haltTrack() { _isPlaying = false; }
	void startTrack() { _isPlaying = true; }
	void setPassThrough(bool b)	{ _passThrough = b; }

	void loadSoundEffectFile(const char *file);
	void loadSoundEffectFile(uint8 *data, uint32 size);
	void stopSoundEffect();

	void playSoundEffect(uint8 track);

	void beginFadeOut();
	bool fadeOut() { return _fadeMusicOut; }

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

private:

	static void onTimer(void *data);

	MidiChannel *_channel[32];
	int _virChannel[16];
	uint8 _channelVolume[16];
	MidiDriver *_driver;
	bool _nativeMT32;
	bool _passThrough;
	uint8 _volume;
	bool _isPlaying;
	bool _sfxIsPlaying;
	uint32 _fadeStartTime;
	bool _fadeMusicOut;
	bool _isLooping;
	bool _eventFromMusic;
	MidiParser *_parser;
	byte *_parserSource;
	MidiParser *_soundEffect;
	byte *_soundEffectSource;
};
} // end of namespace Kyra

#endif
