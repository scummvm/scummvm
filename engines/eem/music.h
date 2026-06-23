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

#ifndef EEM_MUSIC_H
#define EEM_MUSIC_H

#include "audio/midiplayer.h"

#include "common/array.h"
#include "common/path.h"
#include "common/scummsys.h"

namespace EEM {

/**
 * MIDI music player. Mirrors MIDI.C in EEMCD.EXE
 * (_MIDIPlayFile / _MIDIPlay / _StopMIDI / _IsMIDIPlaying /
 * _StartTravelMusic family at 20a2:00e2-05c9).
 *
 * Original uses Miles AIL: _InitMIDI @ 20a2:013a registers
 * ADLIB.ADV / SBFM.ADV / MT32MPU.ADV; _MIDIPlayFile @ 20a2:024c
 * installs AdLib timbres from SAMPLE.AD (29be:14d6) via
 * _AIL_install_timbre before _AIL_start_sequence. We use
 * Audio::MidiDriver_Miles_AdLib_create (loads SAMPLE.AD) for AdLib
 * and the Miles MT-32 driver for MT-32.
 *
 * Music files:
 *   THEME.XMI — opening anims + title.
 *   MUS00000..MUS00004 — travel music (siteNumber % 5).
 *   MUS00005 — winner (_DisplayCorrect @ 1df2:0789).
 *   MUS00006 — loser  (_DisplayAlibi  @ 1df2:018a).
 */
class MusicPlayer : public Audio::MidiPlayer {
public:
	explicit MusicPlayer(bool isFloppy = false, bool isMacintosh = false);

	/// _MIDIPlayFile @ 20a2:024c. loop=true mirrors
	void playFile(const Common::Path &xmiPath, bool loop = false);

	/// _MIDIPlay(num) @ 20a2:047d. CD: "MUS%05u.XMI";
	/// floppy: TRAVEL-N.XMI / FANFARE2.XMI.
	void playMus(uint num, bool loop = false);

	// WORKAROUND: Miles drivers handle source-channel routing themselves;
	// bypass Audio::MidiPlayer::sendToChannel. Same as Toltecs / SAGA.
	void send(uint32 b) override;

private:
	void playMacMidiResource(uint16 resourceId, bool loop);
	void playMacSongResource(uint16 resourceId, bool loop);
	void startLoadedMusic(const Common::String &name, bool loop, bool smf);
	bool loadMacSong(uint16 resourceId, uint16 &midiId);
	void clearMacInstrumentMap();
	byte mapMacInstrumentToGM(byte inst, byte channel) const;

	bool _milesAudioMode = false;
	const bool _isFloppy;
	const bool _isMacintosh;
	Common::Array<byte> _xmiData;
	byte _macChannelInstrument[16] = {};
};

} // End of namespace EEM

#endif
