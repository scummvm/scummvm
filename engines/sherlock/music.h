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

#ifndef SHERLOCK_MUSIC_H
#define SHERLOCK_MUSIC_H

#include "audio/midiplayer.h"
#include "audio/midiparser.h"
//#include "audio/mididrv.h"
#include "sherlock/scalpel/drivers/mididriver.h"
// for 3DO digital music
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/mutex.h"

namespace Sherlock {

class SherlockEngine;

class MidiParser_SH : public MidiParser {
public:
	MidiParser_SH();
	~MidiParser_SH();

protected:
	Common::Mutex _mutex;
	void parseNextEvent(EventInfo &info);

	uint8 _beats;
	uint8 _lastEvent;
	byte *_data;
	byte *_trackEnd;

public:
	bool loadMusic(byte *musData, uint32 musSize);
	virtual void unloadMusic();

private:
	byte  *_musData;
	uint32 _musDataSize;
};

class Music {
private:
	SherlockEngine *_vm;
	Audio::Mixer *_mixer;
	MidiParser *_midiParser;
	MidiDriver *_midiDriver;
	Audio::SoundHandle _digitalMusicHandle;
	MusicType _musicType;
	
	/**
	 * Play the specified music resource
	 */
	bool playMusic(const Common::String &name);
public:
	bool _musicPlaying;
	bool _musicOn;
	int _musicVolume;
	bool _midiOption;
	Common::String _currentSongName, _nextSongName;
public:
	Music(SherlockEngine *vm, Audio::Mixer *mixer);
	~Music();

	/**
	 * Saves sound-related settings
	 */
	void syncMusicSettings();

	/**
	 * Load a specified song
	 */
	bool loadSong(int songNumber);

	/**
	 * Load a specified song
	 */
	bool loadSong(const Common::String &songName);

	/**
	 * Start playing a song
	 */
	void startSong();
	
	/**
	 * Free any currently loaded song
	 */
	void freeSong();

	/**
	 * Stop playing the music
	 */
	void stopMusic();
	
	bool isPlaying();
	uint32 getCurrentPosition();

	bool waitUntilMSec(uint32 msecTarget, uint32 maxMSec, uint32 additionalDelay, uint32 noMusicDelay);

	/**
	 * Sets the volume of the MIDI music with a value ranging from 0 to 127
	 */
	void setMusicVolume(int volume);
};

} // End of namespace Sherlock

#endif

