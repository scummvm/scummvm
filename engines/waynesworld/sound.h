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

#ifndef WAYNESWORLD_SOUND_H
#define WAYNESWORLD_SOUND_H

#include "common/scummsys.h"
#include "audio/midiplayer.h"

#define MAX_SOUNDS 20

namespace Audio {
class AudioStream;
class SoundHandle;
}

namespace WaynesWorld {

class WaynesWorldEngine;

class SoundManager {
	Audio::AudioStream *_stream;
	Common::String _filename;

private:
	WaynesWorldEngine *_vm;
	Audio::Mixer *_mixer;
	Audio::SoundHandle *_effectsHandle;
	char _abtLookupTable[256] = {0};

	byte decompDelta1(int16 *curVal, byte **src, byte **dst, byte count);
	byte decompDelta2(int16 *curVal, byte **src, byte **dst, byte count);
	byte decompDelta4(int16 *curVal, byte **src, byte **dst, byte count);
	
	byte* abtDecomp(Common::File *fd, int *size, int *freq);

public:
	SoundManager(WaynesWorldEngine *vm, Audio::Mixer *mixer);
	~SoundManager();

	void playSound(const char *filename, bool flag);
	bool isSFXPlaying();

	void syncVolume();

	void stopSound();
};

class MusicManager : public Audio::MidiPlayer {
private:
	//WaynesWorldEngine *_vm;
	byte *_music;

	// MidiDriver_BASE interface implementation
	void send(uint32 b) override;

public:
	MusicManager(WaynesWorldEngine *vm);
	~MusicManager() override;

	bool checkMidiDone();

	void midiRepeat();
	void stopSong();

	void playMusic(const char *filename, bool loop = false);

	void setLoop(bool loop);
};
} // End of namespace WaynesWorld

#endif /* WAYNESWORLD_SOUND_H*/
