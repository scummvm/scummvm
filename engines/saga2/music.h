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

// Music class

#ifndef SAGA2_MUSIC_H
#define SAGA2_MUSIC_H

#include "audio/mididrv.h"
#include "audio/mididrv_ms.h"
#include "audio/midiparser.h"
#include "audio/mixer.h"

namespace Saga2 {

enum MusicFlags {
	MUSIC_NORMAL = 0,
	MUSIC_LOOP = 0x0001
};

class Music {
public:

	Music(hResContext *musicRes);
	~Music();
	bool isPlaying();

	void play(uint32 resourceId, MusicFlags flags = MUSIC_NORMAL);
	void pause();
	void resume();
	void stop();

	void setVolume(int volume);
	int getVolume() { return _currentVolume; }

	bool isAdlib() const { return _driverType == MT_ADLIB; }

	void syncSoundSettings();

private:
	MidiParser *_parser;
	MidiDriver_Multisource *_driver;
	Audio::SoundHandle _musicHandle;
	uint32 _trackNumber;

	int _currentVolume;
	MusicType _musicType;
	MusicType _driverType;

	hResContext *_musicContext;

	byte *_currentMusicBuffer;

	static void timerCallback(void *refCon);
	void onTimer();
};

} // End of namespace Saga2

#endif
