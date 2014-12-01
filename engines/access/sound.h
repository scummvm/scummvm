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
 */

#ifndef ACCESS_SOUND_H
#define ACCESS_SOUND_H

#include "common/scummsys.h"
#include "audio/mixer.h"
#include "access/files.h"
#include "audio/midiplayer.h"
#include "audio/midiparser.h"

#define MAX_SOUNDS 20

namespace Access {

class AccessEngine;

struct SoundEntry {
	Resource *_res;
	int _priority;

	SoundEntry() { _res = nullptr; _priority = 0; }
	SoundEntry(Resource *res, int priority) { _res = res; _priority = priority; }
};

class SoundManager {
private:
	AccessEngine *_vm;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;

	void clearSounds();

	void playSound(Resource *res, int priority);
	void stopSound();

public:
	Common::Array<SoundEntry> _soundTable;
	bool _playingSound;
	bool _isVoice;
public:
	SoundManager(AccessEngine *vm, Audio::Mixer *mixer);
	~SoundManager();

	void queueSound(int idx, int fileNum, int subfile);

	void playSound(int soundIndex);

	Resource *loadSound(int fileNum, int subfile);
	void loadSounds(Common::Array<RoomInfo::SoundIdent> &sounds);

	void freeSounds();
};

class MusicManager : public Audio::MidiPlayer {
private:
	AccessEngine *_vm;
	Audio::Mixer *_mixer;

	Resource *_tempMusic;

	// MidiDriver_BASE interface implementation
	virtual void send(uint32 b);

public:
	Resource *_music;

public:
	MusicManager(AccessEngine *vm, Audio::Mixer *mixer);
	~MusicManager();

	void midiPlay();

	bool checkMidiDone();

	void midiRepeat();

	void stopSong();

	void newMusic(int musicId, int mode);

	void freeMusic();

	void loadMusic(int fileNum, int subfile);
	void loadMusic(FileIdent file);

	void setLoop(bool loop);
};
} // End of namespace Access

#endif /* ACCESS_SOUND_H*/
