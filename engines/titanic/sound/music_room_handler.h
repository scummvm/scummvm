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

#include "titanic/sound/music_wave.h"
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
	struct Array5Entry {
		int _v1;
		int _v2;
		Array5Entry() : _v1(0), _v2(0) {}
	};
private:
	CProjectItem *_project;
	CSoundManager *_soundManager;
	CMusicWave *_musicWaves[4];
	MusicRoomInstrument _array1[4];
	MusicRoomInstrument _array2[4];
	Array5Entry _array5[4];
	bool _stopWaves;
	CWaveFile *_waveFile;
	int _soundHandle;
	int _soundVolume;
	int _field108;
public:
	CMusicRoomHandler(CProjectItem *project, CSoundManager *soundManager);
	~CMusicRoomHandler();

	/**
	 * Creates a new music wave class instance, and assigns it to a slot
	 * in the music handler
	 * @param instrument	Which instrument instance is for
	 * @param count			Number of files the new instance will contain
	 */
	CMusicWave *createMusicWave(MusicInstrument instrument, int count);

	void createWaveFile(int musicVolume);

	/**
	 * Handles regular polling the music handler
	 */
	bool poll();

	bool isBusy();

	/**
	 * Flags whether the loaded music waves will be stopped when the
	 * music handler is stopped
	 */
	void setStopWaves(bool flag) { _stopWaves = flag; }

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
};

} // End of namespace Titanic

#endif /* TITANIC_MUSIC_ROOM_HANDLER_H */
