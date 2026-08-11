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

#ifndef GOT_MUSICPARSER_H
#define GOT_MUSICPARSER_H

#include "got/musicdriver_adlib.h"

#include "common/scummsys.h"

namespace Got {

class MusicParser_Got {
public:
	MusicParser_Got();
	~MusicParser_Got();

	bool startPlaying();
	void pausePlaying();
	void resumePlaying();
	void stopPlaying();
	bool isPlaying();

	void setMusicDriver(MusicDriver_Got *driver);

	bool loadMusic(byte *data, uint32 size);
	void unloadMusic();

	void onTimer();
	static void timerCallback(void *data);

protected:
	void allNotesOff();
	void sendToDriver(uint16 b);
	void sendToDriver(byte reg, byte value);
	void resetTracking();

	bool _pause;
	bool _abortParse;
	// Number of ticks to the next event to process.
	uint16 _currentDelta;

	MusicDriver_Got *_driver;

	// Points to start of current track data, or nullptr if no track is loaded.
	byte *_track;
	// The length of the track data.
	uint32 _trackLength;
	// The current play position in the track data, or nullptr if the parser is
	// not playing.
	byte *_playPos;
	// The offset from the start of _track where playback should restart if the
	// track has reached the end. -1 if there is no (valid) loop point defined.
	int32 _loopStart;
};

} // namespace Got

#endif
