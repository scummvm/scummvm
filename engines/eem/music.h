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
 * MIDI music player. Mirrors the original `MIDI.C` source file in
 * `EEMCD.EXE` (the `_MIDIPlayFile / _MIDIPlay / _StopMIDI /
 * _IsMIDIPlaying / _StartTravelMusic` family at `20a2:00e2-05c9`).
 *
 * The original engine uses Miles Audio Interface Library (AIL):
 *
 *   - `_InitMIDI @ 20a2:013a` calls `_AIL_register_driver` against
 *     `ADLIB.ADV` / `SBFM.ADV` / `MT32MPU.ADV` and reserves a timbre
 *     cache via `_AIL_define_timbre_cache`.
 *   - `_MIDIPlayFile @ 20a2:024c` opens the .XMI, calls
 *     `_AIL_register_sequence`, then loops over the sequence's
 *     `_AIL_timbre_request` results. For every (bank, patch) pair the
 *     driver asks for, it pulls the AdLib instrument definition from
 *     **`SAMPLE.AD`** (string at `29be:14d6`) via `_load_global_timbre`
 *     and installs it through `_AIL_install_timbre`. Only after every
 *     patch is loaded does it call `_AIL_start_sequence`.
 *
 * Without those custom timbres, ScummVM's generic AdLib synth falls
 * back to its built-in default timbre table — same notes, very
 * different timbres. ScummVM ships a Miles AdLib driver
 * (`Audio::MidiDriver_Miles_AdLib_create`) that loads `SAMPLE.AD` and
 * implements the same install-on-demand workflow, so we use it for
 * AdLib output. MT-32 / GM fall back to the generic driver via
 * `Audio::MidiPlayer::createDriver`.
 *
 * Available music files in the game directory:
 *   - THEME.XMI   — opening anims (looping) + title screen
 *   - MUS00000.XMI..MUS00004.XMI — per-site travel music
 *     (`_StartTravelMusic` picks one via `_SiteNumber % 5`)
 *   - MUS00005.XMI — winner ending (`_DisplayCorrect` @ 1df2:0789)
 *   - MUS00006.XMI — loser ending (`_DisplayAlibi` @ 1df2:018a)
 */
class MusicPlayer : public Audio::MidiPlayer {
public:
	explicit MusicPlayer(bool isFloppy = false);

	/// Mirrors `_MIDIPlayFile @ 20a2:024c`. Reads the .XMI from the game
	/// directory and starts playing. `loop=true` mirrors the
	/// `_LoopMIDI = 0xFFFF` writes inside `_DoOpeningAnims` (theme music).
	void playFile(const Common::Path &xmiPath, bool loop = false);

	/// Mirrors `_MIDIPlay(num) @ 20a2:047d`. Composes the filename
	/// "MUS%05u.XMI" (CD) or maps to TRAVEL-N.XMI / FANFARE2.XMI
	/// (floppy) and plays it. Used by `_StartTravelMusic`,
	/// `_DisplayCorrect` (winner), `_DisplayAlibi` (loser).
	void playMus(uint num, bool loop = false);

	// In Miles AdLib mode the driver allocates its own AdLib voice
	// pool and consumes the XMIDI's source-channel byte directly, so we
	// must NOT route through `Audio::MidiPlayer::sendToChannel` (which
	// remaps every source channel through `allocateChannel()` and
	// breaks the timbre selection / volume scaling the Miles driver
	// performs internally). Same workaround Toltecs / SAGA use.
	void send(uint32 b) override;

private:
	bool _milesAudioMode = false;
	const bool _isFloppy;
	Common::Array<byte> _xmiData;
};

} // End of namespace EEM

#endif
