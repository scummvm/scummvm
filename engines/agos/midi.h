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

#include "agos/sfxparser_accolade.h"

#include "audio/mididrv.h"
#include "audio/mididrv_ms.h"
#include "audio/midiparser.h"
#include "common/mutex.h"

namespace Common {
class File;
}

namespace AGOS {

class MidiPlayer {
protected:
	// Instrument map specifically for remapping the instruments of the GM
	// version of Simon 2 track 10 subtracks 2 and 3 to MT-32.
	static const byte SIMON2_TRACK10_GM_MT32_INSTRUMENT_REMAPPING[];

	AGOSEngine *_vm;

	Common::Mutex _mutex;
	// Driver used for music. This points to the same object as _driverMsMusic,
	// except if a PC-98 driver is used (these do not implement the Multisource
	// interface).
	MidiDriver *_driver;
	// Multisource driver used for music. Provides access to multisource
	// methods without casting. If this is not nullptr, it points to the same
	// object as _driver.
	MidiDriver_Multisource *_driverMsMusic;
	// Multisource driver used for sound effects. Only used for Elvira 2,
	// Waxworks and Simon The Sorcerer DOS floppy AdLib sound effects.
	// If AdLib is also used for music, this points to the same object as
	// _driverMsMusic and _driver.
	MidiDriver_Multisource *_driverMsSfx;

	// MIDI parser and data used for music.
	MidiParser *_parserMusic;
	byte *_musicData;
	// MIDI parser and data used for SFX (Simon 1 DOS floppy).
	MidiParser *_parserSfx;
	byte *_sfxData;
	// Parser used for SFX (Elvira 2 and Waxworks DOS).
	SfxParser_Accolade *_parserSfxAccolade;

	bool _paused;

	// Queued music track data (Simon 2).
	byte _queuedTrack;
	bool _loopQueuedTrack;

protected:
	static void onTimer(void *data);

public:
	MidiPlayer(AGOSEngine *vm);
	~MidiPlayer();

	// Creates and opens the relevant parsers and drivers for the game version
	// and selected sound device.
	int open();

	// Loads music or SFX data supported by the MidiParser or SfxParser used
	// for the detected version of the game. Specify sfx to indicate that this
	// is a synthesized sound effect.
	void load(Common::SeekableReadStream *in, int32 size = -1, bool sfx = false);

	/**
	 * Plays the currently loaded music or SFX data. If the loaded data has
	 * multiple tracks, specify track to select the track to play.
	 * 
	 * @param track The track to play. Default 0.
	 * @param sfx True if the SFX data should be played, otherwise the loaded
	 * music data will be played. Default false.
	 * @param sfxUsesRhythm True if the sound effect uses OPL rhythm
	 * instruments. Default false.
	 * @param queued True if this track was queued; false if it was played
	 * directly. Default false.
	 */
	void play(int track = 0, bool sfx = false, bool sfxUsesRhythm = false, bool queued = false);
	
	// Returns true if the playback device uses MT-32 MIDI data; false it it
	// uses a different data type.
	bool usesMT32Data() const;
	// Returns true if the game version and selected sound device can use MIDI
	// (or synthesized) sound effects.
	bool hasMidiSfx() const;
	void setLoop(bool loop);
	// Activates or deactivates remapping GM to MT-32 instruments for
	// Simon 2 track 10.
	void setSimon2Remapping(bool remap);
	void queueTrack(int track, bool loop);
	bool isPlaying(bool checkQueued = false);

	void stop(bool sfx = false);
	void pause(bool b);
	void fadeOut();

	void syncSoundSettings();

private:
	bool _pc98;
	// The type of the music device selected for playback.
	MusicType _deviceType;
	// The type of the MIDI data of the game (MT-32 or GM).
	MusicType _dataType;

private:
	Common::SeekableReadStream *simon2SetupExtractFile(const Common::String &requestedFileName);
};

} // End of namespace AGOS

#endif
