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

namespace Sherlock {

class SherlockEngine;

class MidiParser_SH : public MidiParser {
public:
	MidiParser_SH();
	~MidiParser_SH();

protected:
	virtual void parseNextEvent(EventInfo &info);

	uint8 _beats;
	uint8 _lastEvent;
	byte *_data;
	byte *_trackEnd;

public:
	virtual bool loadMusic(byte *data, uint32 size);
};

class Music {
public:
	Music(SherlockEngine *vm, Audio::Mixer *mixer);
	~Music();

private:
	SherlockEngine *_vm;
	Audio::Mixer *_mixer;
	MidiParser_SH *_midiParser;
	MidiDriver *_driver;

public:
	bool _musicPlaying;
	bool _musicOn;

private:
	MusicType _musicType;

public:
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
	 * Play the specified music resource
	 */
	bool playMusic(const Common::String &name);

	/**
	 * Stop playing the music
	 */
	void stopMusic();
	
	void waitTimerRoland(uint time);

	bool waitUntilTick(uint32 tick, uint32 maxTick, uint32 additionalDelay, uint32 noMusicDelay);
};

} // End of namespace Sherlock

#endif

