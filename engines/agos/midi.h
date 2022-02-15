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
#include "audio/mididrv_ms.h"
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
	AGOSEngine *_vm;

	Common::Mutex _mutex;
	MidiDriver *_driver;
	// Multisource driver used for music. Provides access to multisource
	// methods without casting. If this is not nullptr, it points to the same
	// object as _driver.
	MidiDriver_Multisource *_driverMsMusic;
	// Multisource driver used for sound effects. Only used for Simon The
	// Sorcerer DOS floppy AdLib sound effects.
	// If AdLib is also used for music, this points to the same object as
	// _driverMsMusic and _driver.
	MidiDriver_Multisource *_driverMsSfx;
	bool _map_mt32_to_gm;
	bool _nativeMT32;

	MusicInfo _music;
	MusicInfo _sfx;
	MusicInfo *_current; // Allows us to establish current context for operations.

	MidiParser *_parserMusic;
	byte *_musicData;
	MidiParser *_parserSfx;
	byte *_sfxData;

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
	MidiPlayer(AGOSEngine *vm);
	~MidiPlayer() override;

	// Loads music data supported by the MidiParser used for the detected
	// version of the game. Specify sfx to indicate that this is a MIDI sound
	// effect.
	void loadMusic(Common::SeekableReadStream *in, int32 size = -1, bool sfx = false);

	/**
	 * Plays the currently loaded music data. If the loaded MIDI data has
	 * multiple tracks, specify track to select the track to play.
	 * 
	 * @param track The track to play. Default 0.
	 * @param sfx True if the SFX MIDI data should be played, otherwise the
	 * loaded music data will be played. Default false.
	 * @param sfxUsesRhythm True if the sound effect uses OPL rhythm
	 * instruments. Default false.
	 * @param queued True if this track was queued; false if it was played
	 * directly. Default false.
	 */
	void play(int track = 0, bool sfx = false, bool sfxUsesRhythm = false, bool queued = false);

	void loadSMF(Common::SeekableReadStream *in, int song, bool sfx = false);
	void loadMultipleSMF(Common::SeekableReadStream *in, bool sfx = false);
	void loadXMIDI(Common::SeekableReadStream *in, bool sfx = false);
	void loadS1D(Common::SeekableReadStream *in, bool sfx = false);

	// Returns true if the playback device uses MT-32 MIDI data; false it it
	// uses a different data type.
	bool usesMT32Data() const;
	bool hasAdLibSfx() const;
	void setLoop(bool loop);
	void startTrack(int track);
	void queueTrack(int track, bool loop);
	bool isPlaying(bool checkQueued = false);

	void stop();
	void stopSfx();
	void pause(bool b);
	void fadeOut();

	void setVolume(int musicVol, int sfxVol);
	void syncSoundSettings();

public:
	int open(int gameType, Common::Platform platform, bool isDemo);

	// MidiDriver_BASE interface implementation
	void send(uint32 b) override;
	void metaEvent(byte type, byte *data, uint16 length) override;

private:
	kMusicMode _musicMode;
	// The type of the music device selected for playback.
	MusicType _deviceType;
	// The type of the MIDI data of the game (MT-32 or GM).
	MusicType _dataType;

private:
	Common::SeekableReadStream *simon2SetupExtractFile(const Common::String &requestedFileName);
};

} // End of namespace AGOS

#endif
