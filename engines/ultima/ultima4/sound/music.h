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
#include "audio/midiplayer.h"
#include "audio/mixer.h"

namespace Ultima {
namespace Ultima4 {

#define CAMP_FADE_OUT_TIME          1000
#define CAMP_FADE_IN_TIME           0
#define INN_FADE_OUT_TIME           1000
#define INN_FADE_IN_TIME            5000
#define NLOOPS -1


class Music : public Audio::MidiPlayer {
public:
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

	Type _introMid;
private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	Std::vector<Common::String> _filenames;

	/**
	 * Play a given music file if is exists
	 */
	bool startMusic(const Common::String &filename);
protected:
	// Overload Audio::MidiPlayer method
	void sendToChannel(byte channel, uint32 b) override;
public:
	Music(Audio::Mixer *mixer);
	~Music() override;

	/**
	 * Play music
	 */
	void playMusic(const Common::String &filename);

	/**
	 * Play music of a given type
	 */
	void playMusic(Type music);

	/**
	 * Play the designated music for the current map
	 */
	void playMapMusic();

	void stop() override;


	/**
	 * Fade out the music
	 */
	void fadeOut(int msecs) {
		// TODO
	}

	/**
	 * Fade in the music
	 */
	void fadeIn(int msecs, bool loadFromMap) {
		// TODO
	}

	/**
	 * Music when you talk to Lord British
	 */
	void lordBritish() {
		playMusic(RULEBRIT);
	}

	/**
	 * Music when you talk to Hawkwind
	 */
	void hawkwind() {
		playMusic(SHOPPING);
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
		playMusic(SHOPPING);
	}

	void intro() {
#ifdef IOS_ULTIMA4
		_on = true; // Force iOS to turn this back on from going in the background.
#endif
		playMusic(_introMid);
	}

	/**
	 * Cycle through the introduction music
	 */
	void introSwitch(int n);
};

extern Music *g_music;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
