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

#ifndef AGOS_MIDI_H
#define AGOS_MIDI_H

#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "common/mutex.h"

namespace Common {
class File;
}

namespace AGOS {

enum kMusicMode {
	kMusicModeDisabled = 0,
	kMusicModeAccolade = 1,
	kMusicModeMilesAudio = 2,
	kMusicModeSimon1 = 3,
	kMusicModePC98 = 4
};

struct MusicInfo {
	MidiParser *parser;
	byte *data;
	byte num_songs;           // For Type 1 SMF resources
	byte *songs[16];          // For Type 1 SMF resources
	uint32 song_sizes[16];    // For Type 1 SMF resources

	MidiChannel *channel[16]; // Dynamic remapping of channels to resolve conflicts
	byte volume[16];          // Current channel volume

	MusicInfo() { clear(); }
	void clear() {
		parser = 0; data = 0; num_songs = 0;
		memset(songs, 0, sizeof(songs));
		memset(song_sizes, 0, sizeof(song_sizes));
		memset(channel, 0, sizeof(channel));
	}
};

class MidiPlayer : public MidiDriver_BASE {
protected:
	Common::Mutex _mutex;
	MidiDriver *_driver;
	bool _map_mt32_to_gm;
	bool _nativeMT32;

	MusicInfo _music;
	MusicInfo _sfx;
	MusicInfo *_current; // Allows us to establish current context for operations.

	// These are maintained for both music and SFX
	byte _masterVolume;    // 0-255
	byte _musicVolume;
	byte _sfxVolume;
	bool _paused;

	// These are only used for music.
	byte _currentTrack;
	bool _loopTrack;
	byte _queuedTrack;
	bool _loopQueuedTrack;

protected:
	static void onTimer(void *data);
	void clearConstructs();
	void clearConstructs(MusicInfo &info);
	void resetVolumeTable();

public:
	bool _adLibMusic;
	bool _enable_sfx;

public:
	MidiPlayer();
	~MidiPlayer() override;

	void loadSMF(Common::SeekableReadStream *in, int song, bool sfx = false);
	void loadMultipleSMF(Common::SeekableReadStream *in, bool sfx = false);
	void loadXMIDI(Common::SeekableReadStream *in, bool sfx = false);
	void loadS1D(Common::SeekableReadStream *in, bool sfx = false);

	bool hasNativeMT32() const { return _nativeMT32; }
	void setLoop(bool loop);
	void startTrack(int track);
	void queueTrack(int track, bool loop);
	bool isPlaying(bool check_queued = false) { return (_currentTrack != 255 && (_queuedTrack != 255 || !check_queued)); }

	void stop();
	void pause(bool b);

	int  getMusicVolume() const { return _musicVolume; }
	int  getSFXVolume() const { return _sfxVolume; }
	void setVolume(int musicVol, int sfxVol);

public:
	int open(int gameType, Common::Platform platform, bool isDemo);

	// MidiDriver_BASE interface implementation
	void send(uint32 b) override;
	void metaEvent(byte type, byte *data, uint16 length) override;

private:
	kMusicMode _musicMode;
	MusicType musicType;

private:
	Common::SeekableReadStream *simon2SetupExtractFile(const Common::String &requestedFileName);
};

} // End of namespace AGOS

#endif
