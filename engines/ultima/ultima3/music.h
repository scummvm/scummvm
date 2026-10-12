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

#ifndef ULTIMA3_MUSIC_H
#define ULTIMA3_MUSIC_H

#include "audio/midiplayer.h"

namespace Ultima {
namespace Ultima3 {

/**
 * The songs of the Ultima III Upgrade's MIDI music, numbered as in the game
 */
enum Song {
	SONG_NONE = -1,
	SONG_WANDER = 0,
	SONG_CASTLES = 1,
	SONG_RULEBRIT = 2,
	SONG_TOWNS = 3,
	SONG_SHOPPING = 4,
	SONG_COMBAT = 5,
	SONG_DUNGEON = 6,
	SONG_ALIVE = 7,
	SONG_FANFARE = 8,
	SONG_EXODUS = 9,
	SONG_COUNT = 10
};

// The kinds of place that have a song, other than the ones in the savegame
constexpr byte MUSIC_LOCATION_SCENE = 4;
constexpr byte MUSIC_LOCATION_GEM = 0x11;
constexpr byte MUSIC_LOCATION_COMBAT = 0x80;

/**
 * Plays the XMIDI songs that the Upgrade adds. A song carries on until
 * another is chosen, and a jingle is played once on top of it
 */
class Music : public Audio::MidiPlayer {
private:
	static constexpr int UNSELECTED = -2;

	bool _enabled = false;
	bool _driverOpened = false;
	bool _miles = false;
	int _song = SONG_NONE;
	int _current = SONG_NONE;
	int _jingle = SONG_NONE;
	int _selected = UNSELECTED;

	/**
	 * Creates the device the music is played on
	 */
	void openDriver();

	/**
	 * Starts a song from the beginning
	 */
	void start(int song, bool loop);

	/**
	 * Starts a song file from the beginning
	 * @returns		False if it couldn't be played
	 */
	bool startFile(const Common::String &name, bool loop);

public:
	Music() {}
	~Music() override {}

	/**
	 * Sets whether music plays at all
	 */
	void setEnabled(bool enabled);

	/**
	 * Plays a song until another is chosen. Nothing happens if it's already playing
	 */
	void play(int song);

	/**
	 * Plays a song once, after which the one that was playing carries on
	 */
	void jingle(int song);

	/**
	 * Plays one of the song files, whether it's one of the game's or not
	 * @param name		The name of the file, without its extension
	 * @returns			False if it couldn't be played
	 */
	bool playFile(const Common::String &name);

	/**
	 * Stops all the music
	 */
	void silence();

	/**
	 * Plays the song that suits where the party is, if that's changed
	 * @param location		The kind of place, as in the savegame, or 0x80 for a fight
	 * @param inExodus		True if in the castle of Exodus
	 */
	void autoSelect(byte location, bool inExodus);

	/**
	 * Makes the next autoSelect play its song even if it hasn't changed
	 */
	void resync() {
		_selected = UNSELECTED;
	}

	/**
	 * Called every frame to carry on from a jingle
	 */
	void update();

	// MidiDriver_BASE API
	void send(uint32 b) override;
};

} // namespace Ultima3
} // namespace Ultima

#endif
