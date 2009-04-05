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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef CRUISE_SOUND_H
#define CRUISE_SOUND_H

#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "sound/mixer.h"
#include "common/serializer.h"

namespace Cruise {

class MusicPlayer {
private:
	byte _channelVolume[16];
	int _fadeVolume;
	char _musicName[33];

	bool _isPlaying;
	bool _songPlayed;
	bool _looping;
	byte _masterVolume;

	byte *_songPointer;
	// TODO: lib_SongSize 
	int _songSize;

	void patchMidi(uint32 adr, const byte *data, int size);
	byte *loadInstrument(const char *name, int i);
public:
	MusicPlayer();
	~MusicPlayer();

	void setVolume(int volume);
	int getVolume() const { return _masterVolume; }

	void stop();
	void pause();
	void resume();

	// Common public access methods
	void doSync(Common::Serializer &s);
	void loadSong(const char *name);
	void startSong();
	void stopSong();
	void removeSong();
	void fadeSong();

	bool songLoaded() const { return _songPointer != NULL; }
	bool songPlayed() const { return _songPlayed; }
	bool isPlaying() const { return _isPlaying; }
	bool looping() const { return _looping; }
	byte *songData() { return _songPointer; }
	void setPlaying(bool playing) { _isPlaying = playing; }
	void setLoop(bool loop) { _looping = loop; }
};

class SoundPlayer {
public:
	SoundPlayer() {}

	void startSound(int channelNum, const byte *ptr, int size, int speed, int volume, bool loop) {}
	void startNote(int channelNum, int speed, int volume) {}
	void stopChannel(int channelNum) {}
};

} // End of namespace Cruise

#endif
