/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $Header$
 *
 */

#ifndef QUEENSOUND_H
#define QUEENSOUND_H

#include "common/util.h"
#include "sound/mixer.h"
#include "queen/defs.h"

namespace Queen {

class Input;
class Resource;

struct songData {
	int16 tuneList[5];
	int16 volume;
	int16 tempo;
	int16 reverb;
	int16 override;
	int16 ignore;
};

struct tuneData {
	int16 tuneNum[9];
	int16 sfx[2];
	int16 mode;
	int16 delay;
};

class QueenEngine;

class Sound {
public:
	Sound(Audio::Mixer *mixer, QueenEngine *vm);
	virtual ~Sound();
	virtual bool sfxPlay(const char *name, Audio::SoundHandle *soundHandle) = 0;
	static Sound *giveSound(Audio::Mixer *mixer, QueenEngine *vm, uint8 compression);
	void playSfx(uint16 sfx, bool isSpeech);
	void playSfx(const char *base, bool isSpeech);
	void playSong(int16 songNum);
	void playLastSong()		{ playSong(_lastOverride); }
	void stopSpeech()		{ _mixer->stopHandle(_speechHandle); }
	void stopSfx()			{ _mixer->stopHandle(_sfxHandle); }

	bool sfxOn() const			{ return _sfxToggle; }
	void sfxToggle(bool val)	{ _sfxToggle = val; }
	void toggleSfx()			{ _sfxToggle ^= true; }

	bool speechOn()	const		{ return _speechToggle; }
	void speechToggle(bool val)	{ _speechToggle = val; }
	void toggleSpeech()			{ _speechToggle ^= true; }

	bool musicOn() const		{ return _musicToggle; }
	void musicToggle(bool val)	{ _musicToggle = val; }
	void toggleMusic()			{ _musicToggle ^= true; }

	bool isSpeechActive() const	{ return _mixer->isSoundHandleActive(_speechHandle); }
	bool isSfxActive() const 	{ return _mixer->isSoundHandleActive(_sfxHandle); }

	bool speechSfxExists() const	{ return _speechSfxExists; }

	int16 lastOverride() const	{ return _lastOverride; }

	void saveState(byte *&ptr);
	void loadState(uint32 ver, byte *&ptr);

#ifndef PALMOS_68K
	static const songData _songDemo[];
	static const songData _song[];
	static const tuneData _tuneDemo[];
	static const tuneData _tune[];
	static const char *_sfxName[];
	static const int16 _jungleList[];
#else
	static const songData *_songDemo;
	static const songData *_song;
	static const tuneData *_tuneDemo;
	static const tuneData *_tune;
	static const char *_sfxName;
	static const int16 *_jungleList;
#endif

protected:
	void waitFinished(bool isSpeech);

	Audio::Mixer *_mixer;
	QueenEngine *_vm;

	bool _sfxToggle;
	bool _speechToggle;
	bool _musicToggle;
	bool _speechSfxExists;

	int16 _lastOverride;
	Audio::SoundHandle _sfxHandle;
	Audio::SoundHandle _speechHandle;
};

class SilentSound : public Sound {
public:
	SilentSound(Audio::Mixer *mixer, QueenEngine *vm) : Sound(mixer, vm) {};
	bool sfxPlay(const char *name, Audio::SoundHandle *soundHandle);
};

class SBSound : public Sound {
public:
	SBSound(Audio::Mixer *mixer, QueenEngine *vm) : Sound(mixer, vm) {};
	bool sfxPlay(const char *name, Audio::SoundHandle *soundHandle);
};

#ifdef USE_MAD
class MP3Sound : public Sound {
public:
	MP3Sound(Audio::Mixer *mixer, QueenEngine *vm) : Sound(mixer, vm) {};
	bool sfxPlay(const char *name, Audio::SoundHandle *soundHandle);
};
#endif

#ifdef USE_VORBIS
class OGGSound : public Sound {
public:
	OGGSound(Audio::Mixer *mixer, QueenEngine *vm) : Sound(mixer, vm) {};
	bool sfxPlay(const char *name, Audio::SoundHandle *soundHandle);
};
#endif

#ifdef USE_FLAC
class FLACSound : public Sound {
public:
	FLACSound(Audio::Mixer *mixer, QueenEngine *vm) : Sound(mixer, vm) {};
	bool sfxPlay(const char *name, Audio::SoundHandle *soundHandle);
};
#endif // #ifdef USE_FLAC

} // End of namespace Queen

#endif
