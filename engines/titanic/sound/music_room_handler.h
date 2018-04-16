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

#ifndef TITANIC_MUSIC_ROOM_HANDLER_H
#define TITANIC_MUSIC_ROOM_HANDLER_H

#include "titanic/sound/audio_buffer.h"
#include "titanic/sound/music_room_instrument.h"
#include "titanic/sound/music_song.h"
#include "titanic/sound/wave_file.h"

namespace Titanic {

class CProjectItem;
class CSoundManager;

enum MusicInstrument { BELLS = 0, SNAKE = 1, PIANO = 2, BASS = 3 };

struct MusicRoomInstrument {
	int _pitchControl;
	int _speedControl;
	bool _directionControl;
	bool _inversionControl;
	bool _muteControl;
	MusicRoomInstrument() : _pitchControl(0), _speedControl(0), _directionControl(false),
		_inversionControl(false), _muteControl(false) {}
};

class CMusicRoomHandler {
private:
	CProjectItem *_project;
	CSoundManager *_soundManager;
	CMusicRoomInstrument *_instruments[4];
	MusicRoomInstrument _array1[4];
	MusicRoomInstrument _array2[4];
	CMusicSong *_songs[4];
	int _startPos[4];
	int _position[4];
	double _animExpiryTime[4];

	bool _active;
	CWaveFile *_waveFile;
	int _soundHandle;
	int _instrumentsActive;
	CAudioBuffer *_audioBuffer;
	bool _isPlaying;
	uint _soundStartTicks;
	uint _startTicks;
	int _volume;
private:
	/**
	 * Starts music room instruments animation
	 */
	void start();

	/**
	 * Handles updating the raw audio being played for all the instruments
	 */
	void updateAudio();

	/**
	 * Handles updating the instruments themselves, and keeping them animating
	 */
	void updateInstruments();

	/**
	 * Polls a specified instrument for any updates to see if it's still active.
	 * @returns Returns true if a given instrument is still active..
	 * that is, that there is still more data that can be read from it to play
	 */
	bool pollInstrument(MusicInstrument instrument);

	/**
	 * Gets the duration for a given fragment of an instrument to play
	 * out, so that animations of the instruments can be synchronized
	 * to the actual music
	 */
	double getAnimDuration(MusicInstrument instrument, int arrIndex);

	/**
	 * Figures out a pitch value (of some sort) for use in determining
	 * which wave file the music instruments will use.
	 */
	int getPitch(MusicInstrument instrument, int arrIndex);
public:
	CMusicRoomHandler(CProjectItem *project, CSoundManager *soundManager);
	~CMusicRoomHandler();

	/**
	 * Creates a new music room instrument class to handle the operation of one
	 * of the instruments in the music room.
	 * @param instrument	Which instrument to create for
	 * @param count			Number of Wave files the new instance will contain
	 */
	CMusicRoomInstrument *createInstrument(MusicInstrument instrument, int count);

	/**
	 * Main setup for the music room handler
	 */
	void setup(int volume);

	/**
	 * Flags whether the music handler is active
	 */
	void setActive(bool flag) { _active = flag; }

	/**
	 * Stop playing the music
	 */
	void stop();

	/**
	 * Checks the specified instrument to see if it's settings are "correct"
	 */
	bool checkInstrument(MusicInstrument instrument) const;

	/**
	 * Sets the speed control value
	 */
	void setSpeedControl2(MusicInstrument instrument, int value);

	/**
	 * Sets the pitch control value
	 */
	void setPitchControl2(MusicInstrument instrument, int value);

	/**
	 * Sets the inversion control value
	 */
	void setInversionControl2(MusicInstrument instrument, bool value);

	/**
	 * Sets the direction control value
	 */
	void setDirectionControl2(MusicInstrument instrument, bool value);

	/**
	 * Sets the pitch control value
	 */
	void setPitchControl(MusicInstrument instrument, int value);

	/**
	 * Sets the speed control value
	 */
	void setSpeedControl(MusicInstrument instrument, int value);

	/**
	 * Sets the direction control value
	 */
	void setDirectionControl(MusicInstrument instrument, bool value);

	/**
	 * Sets the inversion control value
	 */
	void setInversionControl(MusicInstrument instrument, bool value);

	/**
	 * Sets the mute control value
	 */
	void setMuteControl(MusicInstrument instrument, bool value);

	/**
	 * Handles regular updates
	 * @returns		True if the music is still playing
	 */
	bool update();
};

} // End of namespace Titanic

#endif /* TITANIC_MUSIC_ROOM_HANDLER_H */
