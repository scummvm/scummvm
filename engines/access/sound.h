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

#ifndef ACCESS_SOUND_H
#define ACCESS_SOUND_H

#include "common/scummsys.h"
#include "access/files.h"
#include "audio/midiplayer.h"

#define MAX_SOUNDS 20

namespace Audio {
class AudioStream;
class SoundHandle;
}

namespace Access {

class AccessEngine;

class SoundManager {
	struct SoundEntry {
		Resource *_res;
		int _priority;
		int _fileNum;
		int _subFileNum;

		SoundEntry() : _res(nullptr), _priority(0), _fileNum(0), _subFileNum(0) { }
		SoundEntry(Resource *res, int priority, int fileNum = -1, int subFileNum = -1) :
			_res(res), _priority(priority), _fileNum(fileNum), _subFileNum(subFileNum) { }

		bool matches(const FileIdent &ident) const { return _fileNum == ident._fileNum && _subFileNum == ident._subFile; }
	};

	struct QueuedSound {
		Audio::AudioStream *_stream;
		int _soundId;

		QueuedSound() : _stream(nullptr), _soundId(-1) {}
		QueuedSound(Audio::AudioStream *stream, int soundIdx) : _stream(stream), _soundId(soundIdx) {}
	};
private:
	AccessEngine *_vm;
	Audio::Mixer *_mixer;
	Audio::SoundHandle *_effectsHandle;
	Common::Array<QueuedSound> _queue;
	Common::Array<SoundEntry> _soundTable;

	void clearSounds();

	void playSound(Resource *res, int priority, bool loop, int soundIndex = -1);

	bool isSoundQueued(int soundId) const;

public:
	SoundManager(AccessEngine *vm, Audio::Mixer *mixer);
	~SoundManager();

	// replace the current table entry at idx with a new one
	void loadSoundTable(int idx, int fileNum, int subfile, int priority = 1);

	// load and add a single sound resource to the table
	void loadAndAddSound(int fileNum, int subfile, int priority = 1);
	void loadAndAddSound(const FileIdent &ident, int priority = 1);

	bool hasLoadedSound(const FileIdent &ident) const;

	void playSound(int soundIndex, bool loop = false);
	void playSoundByIdent(const FileIdent &ident, bool loop = false);
	void checkSoundQueue();
	bool isSFXPlaying();

	void loadSounds(const Common::Array<RoomInfo::SoundIdent> &sounds);
	int loadRawSound(const Common::Path &path, int priority);
	void syncVolume();

	void stopSound();
	void freeSounds();
	void freeSound(int idx);
	bool hasSounds() const { return _soundTable.size() > 0 && _soundTable[0]._res; }
};

class MusicManager : public Audio::MidiPlayer {
private:
	AccessEngine *_vm;

	Resource *_tempMusic;

	// MidiDriver_BASE interface implementation
	void send(uint32 b) override;

public:
	Resource *_music;

public:
	MusicManager(AccessEngine *vm);
	~MusicManager() override;

	void midiPlay();

	bool checkMidiDone();

	void midiRepeat();

	void stopSong();

	void newMusic(int musicId, int mode);

	void freeMusic();

	void startMusicFade();

	void loadMusic(int fileNum, int subfile);
	void loadMusic(FileIdent file);

	void setLoop(bool loop);
};
} // End of namespace Access

#endif /* ACCESS_SOUND_H*/
