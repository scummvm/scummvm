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

#ifndef EEM_AUDIO_H
#define EEM_AUDIO_H

#include "audio/mixer.h"
#include "audio/audiostream.h"

#include "common/array.h"
#include "common/file.h"
#include "common/path.h"
#include "common/scummsys.h"

namespace EEM {

class EEMEngine;

/**
 * Non-MIDI audio (digitised voice + sound effects). Mirrors the
 * original `SOUND.C` / `SPOOLSND.C` source files in `EEMCD.EXE` —
 * the AIL digital playback path used for both standalone .VOC files
 * and the per-mystery `M%d.SDB` spool stream.
 *
 * Two pathways:
 *
 * 1. **VOC playback** — `_LoadSoundName @ 1ff1:0299` reads a Creative
 *    Voice File into memory and `_PlayVoice @ 1ff1:023e` hands it to
 *    `_AIL_play_VOC_file`. Used for THUNDER.VOC (Storm logo),
 *    PHONE.VOC (briefing), JEN.VOC / JAKE.VOC (partner choose).
 *
 * 2. **Spool stream** — `_InitMysterySounds @ 202f:05cb` opens
 *    `m%d.sdx` (29be:144f) and `m%d.sdb` (29be:145b) for the active
 *    mystery. `_SpoolSound(num) @ 202f:068d` indexes into the SDX
 *    table (12 bytes per entry: u32 file_offset, u32 compressed_size,
 *    u32 uncompressed_size). If sizes match it streams via
 *    `_UncompressedSound @ 202f:03e6`; otherwise it EXPLODE-decompresses
 *    via `_DeCompressSound @ 202f:02ad`.
 *
 *    Each entry's data starts with 2 metadata bytes — Sound Blaster
 *    Time Constant (`rate = 1000000 / (256 - tc)`) and the AIL block
 *    count — followed by the (optionally) PKWARE-DCL-compressed
 *    8-bit unsigned PCM stream. We use ScummVM's `Common::decompressDCL`
 *    + `Audio::makeRawStream` to reach the same audio at the same
 *    sample rate.
 *
 * Mystery 60 (`M60.SDB/SDX`) holds the 19 voiceovers played between
 * ANIM01..ANIM20 in `_DoOpeningAnims` (it loads `_InitMysterySounds(0x3c)`
 * before the loop and `_SpoolSound(uVar3 - 1)` between every clip).
 * Mysteries 0..55 hold each case's per-clue voice plus the partner's
 * digital lines (`_KDDigitalIndex` table within the .SD blob).
 */
class AudioPlayer {
public:
	explicit AudioPlayer(EEMEngine *vm);
	~AudioPlayer();

	// VOC playback ----------------------------------------------------

	/// Mirrors `_LoadSoundName` + `_PlayVoice`. Loads the named .VOC
	/// from the game directory and hands it to the speech mixer
	/// channel. A new `playVoc` cancels any prior voice.
	void playVoc(const Common::Path &vocPath);

	/// Mirrors `_VoicePlaying @ 1ff1:01f9`.
	bool isVoicePlaying() const;

	/// Mirrors `_WaitForVoiceDone @ 1ff1:0221`. Blocks (with frame /
	/// event pumping, like the rest of the engine's busy-loops) until
	/// the voice clip finishes. Returns early if the user clicks /
	/// presses a key — same abort behaviour the original
	/// `_AIL_stop_digital_playback` callback installed.
	void waitForVoiceDone(uint32 maxMs = 60000);

	/// Mirrors `_StopTheVoice @ 1ff1:0283`.
	void stopVoice();

	// Mystery sound spool ---------------------------------------------

	/// Mirrors `_InitMysterySounds @ 202f:05cb`. Loads `M%u.SDX` into
	/// memory and remembers the corresponding `M%u.SDB` path.
	bool initMysterySounds(uint mysteryNum);

	/// Mirrors `_CleanMysterySounds @ 202f:05a5`.
	void cleanMysterySounds();

	/// Mirrors `_SpoolSound @ 202f:068d`. Reads + decompresses entry
	/// `num` from the active SDB and queues it for SFX playback. The
	/// original blocks until playback finishes — for ScummVM we let
	/// the mixer run asynchronously and expose `waitForSpoolDone` so
	/// callers that need the original "block-then-continue" semantics
	/// can opt in.
	void spoolSound(uint num);

	/// Mirrors the abort-on-input wait loop inside `_UncompressedSound`
	/// / `_DeCompressSound`. Returns when the spool clip finishes or
	/// the user clicks / presses a key.
	void waitForSpoolDone(uint32 maxMs = 60000);

	/// Mirrors the immediate `_AIL_stop_digital_playback` exit.
	void stopSpool();

	bool isSpoolPlaying() const;

	/// Mirrors `_SayKDDigital(kdspeak) @ 2404:0fbc`. Each mystery
	/// embeds a `KDDigitalIndex` table immediately after the 18-byte
	/// `KDTextIndex` header (set up by `_ReadMystery @ 2404:008f`:
	/// `_KDDigitalIndex = _KDTextIndex + 0x12`). The table is two
	/// 1-based sound indices per `kdspeak` slot — Jen at +2, Jake at
	/// +4 from each entry's start (+1 word for the unused header
	/// slot). Pass the mystery's `kdTextIndex()` pointer.
	void sayKDDigital(const byte *kdTextIndex, uint kdspeak, uint partner);

	/// Mirrors `_QuitSounds @ 1ff1:03c5`.
	void stopAll();

private:
	struct SoundEntry {
		uint32 offset;
		uint32 compressedSize;
		uint32 uncompressedSize;
	};

	bool readSdxIndex(const Common::Path &sdxPath);
	void playPcmBuffer(byte *pcm, uint32 size, uint sampleRate,
					   Audio::SoundHandle &handle,
					   Audio::Mixer::SoundType type);

	EEMEngine *_vm = nullptr;
	Audio::Mixer *_mixer = nullptr;
	Audio::SoundHandle _voiceHandle;
	Audio::SoundHandle _spoolHandle;

	Common::Array<SoundEntry> _sdxIndex;
	Common::Path _sdbPath;
	int _currentMystery = -1;
};

} // End of namespace EEM

#endif
