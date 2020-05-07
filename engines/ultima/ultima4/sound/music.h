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

#ifndef ULTIMA4_SOUND_MUSIC_H
#define ULTIMA4_SOUND_MUSIC_H

#include "ultima/shared/std/containers.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Ultima {
namespace Ultima4 {

#define CAMP_FADE_OUT_TIME          1000
#define CAMP_FADE_IN_TIME           0
#define INN_FADE_OUT_TIME           1000
#define INN_FADE_IN_TIME            5000
#define NLOOPS -1

struct _Mix_Music;
typedef _Mix_Music OSMusicMixer;

class Music {
public:
	static bool _functional;

	enum Type {
		NONE,
		OUTSIDE,
		TOWNS,
		SHRINES,
		SHOPPING,
		RULEBRIT,
		FANFARE,
		DUNGEON,
		COMBAT,
		CASTLES,
		MAX
	};

	/*
	 * Properties
	 */
	Std::vector<Common::String> _filenames;
	Type _introMid;
	Type _current;
	Audio::AudioStream *_playing;
	Audio::SoundHandle _soundHandle;
public:
	/**
	 * Initiliaze the music
	 */
	Music();

	/**
	 * Stop playing the music and cleanup
	 */
	~Music();

	/**
	 * Returns true if the mixer is playing any audio
	 */
	static bool isPlaying();

	/**
	 * Ensures that the music is playing if it is supposed to be, or off
	 * if it is supposed to be turned off.
	 */
	static void callback(void *);

	/**
	 * Main music loop
	 */
	void play();

	/**
	 * Stop playing music
	 */
	void stop() {
		_on = false;
		stopMid();
	}

	/**
	 * Fade out the music
	 */
	void fadeOut(int msecs);

	/**
	 * Fade in the music
	 */
	void fadeIn(int msecs, bool loadFromMap);

	/**
	 * Music when you talk to Lord British
	 */
	void lordBritish() {
		playMid(RULEBRIT);
	}

	/**
	 * Music when you talk to Hawkwind
	 */
	void hawkwind() {
		playMid(SHOPPING);    
	}

	/**
	 * Music that plays while camping
	 */
	void camp() {
		fadeOut(1000);
	}

	/**
	 * Music when talking to a vendor
	 */
	void shopping() {
		playMid(SHOPPING);
	}
	void intro() {
#ifdef IOS_ULTIMA4
		_on = true; // Force iOS to turn this back on from going in the background.
#endif
		playMid(_introMid);
	}

	/**
	 * Cycle through the introduction music
	 */
	void introSwitch(int n);

	/**
	 * Toggle the music on/off (usually by pressing 'v')
	 */
	bool toggle();

private:
	void create_sys();
	void destroy_sys();

	/**
	 * Set, increase, and decrease music volume
	 */
	void setMusicVolume_sys(int volume);

	/**
	 * Set, increase, and decrease sound volume
	 */
	void setSoundVolume_sys(int volume);
	void fadeOut_sys(int msecs);
	void fadeIn_sys(int msecs, bool loadFromMap);

	/**
	 * System specific version to check if the version is still playing.
	 */
	bool isPlaying_sys();

	static Music *_instance;
	static bool _fading;
	static bool _on;


	bool load_sys(const Common::String &pathName);

	/**
	 * Play a midi file
	 */
	void playMid(Type music);

	/**
	 * Stop playing a MIDI file.
	 */
	void stopMid();

	bool load(Type music);
};

extern Music *g_music;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
