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
 * Non-MIDI audio (digitised voice + SFX). Mirrors SOUND.C / SPOOLSND.C.
 *
 * - VOC files (THUNDER.VOC, PHONE.VOC, JEN.VOC/JAKE.VOC): _LoadSoundName +
 *   _PlayVoice @ 1ff1:0299 / 1ff1:023e via _AIL_play_VOC_file.
 * - DOS mystery spool stream: _InitMysterySounds @ 202f:05cb opens M%d.SDX/.SDB.
 *   Each SDX entry is (offset, compSize, uncompSize). Equal sizes => raw PCM
 *   (_UncompressedSound @ 202f:03e6); otherwise PKWARE DCL Implode
 *   (_DeCompressSound @ 202f:02ad). Each blob starts with SB Time Constant +
 *   AIL block count, then 8-bit unsigned PCM.
 * - Mac mystery spool streams live in M%02d.DBD/M%02d.CPD resource forks.
 *   The executable tries compressed 'csnd' by resource id, then falls back to
 *   plain 'snd '; ids are 1001 + the zero-based mystery voice slot.
 *
 * M60.SDB/SDX holds the 19 voiceovers between ANIM01..ANIM20 in
 * _DoOpeningAnims (loaded via _InitMysterySounds(0x3c)).
 * M0..M55 hold per-mystery clue voice + partner KDDigital lines.
 */
class AudioPlayer {
public:
	explicit AudioPlayer(EEMEngine *vm);
	~AudioPlayer();

	/// Setup-screen voice toggle. Mirrors DAT_2d5d_3f97 — every original
	/// `_PlayVoice` / `_SpoolSound` is wrapped in
	/// `if ((DAT_2d5d_3f97 != 0) && (_VoiceAvailable != 0))`; we pull the
	/// flag in here so callers don't duplicate the gate.
	void setVoiceEnabled(bool enabled) { _voiceEnabled = enabled; }
	bool voiceEnabled() const { return _voiceEnabled; }

	/// Loads the named .VOC from the game directory and hands it to the
	/// speech mixer channel. Mirrors _LoadSoundName + _PlayVoice
	/// @ 1ff1:0299 / 1ff1:023e. A new playVoc cancels any prior voice.
	void playVoc(const Common::Path &vocPath);

	/// _VoicePlaying @ 1ff1:01f9.
	bool isVoicePlaying() const;

	/// _WaitForVoiceDone @ 1ff1:0221. Blocks (with frame + event pumping)
	/// until the voice clip finishes; returns early on click / keypress.
	void waitForVoiceDone(uint32 maxMs = 60000);

	/// _StopTheVoice @ 1ff1:0283.
	void stopVoice();

	/// Play a floppy VOC by 0..25 slot index in the per-partner voice
	/// table. Mirrors _LoadSoundName_Floppy @ 1f4e:0305 (tables at
	/// 2608:0f0e Jake / 2608:0f76 Jenny). partner: 0=Jake, 1=Jenny.
	/// Common slots: 12 = PHONESL.VOC, 20 = partner intro, 25 = THUNDER.VOC.
	void playFloppyVoiceSlot(uint slot, uint partner);

	/// Loads the current mystery voice bundle. DOS uses M%u.SDX/M%u.SDB;
	/// Mac uses M%02u.DBD/M%02u.CPD 'csnd'/'snd ' resources.
	bool initMysterySounds(uint mysteryNum);

	/// _CleanMysterySounds @ 202f:05a5.
	void cleanMysterySounds();

	/// Reads + decompresses SDX entry `num` from the active SDB and queues
	/// it for SFX playback. Mirrors _SpoolSound @ 202f:068d. Original blocks
	/// until playback finishes; we run async — use waitForSpoolDone to opt
	/// in to the original block-then-continue semantics.
	void spoolSound(uint num);

	/// Wait loop inside _UncompressedSound / _DeCompressSound; aborts on
	/// click / keypress (same abort behaviour as the original).
	void waitForSpoolDone(uint32 maxMs = 60000);

	/// Immediate _AIL_stop_digital_playback exit.
	void stopSpool();

	bool isSpoolPlaying() const;

	/// Mirrors _SayKDDigital(kdspeak) @ 2404:0fbc. Each mystery embeds a
	/// KDDigitalIndex table 18 bytes (+0x12) after its KDTextIndex header
	/// (set up by _ReadMystery @ 2404:008f: `_KDDigitalIndex = _KDTextIndex
	/// + 0x12`). Table is two 1-based sound indices per kdspeak slot — Jen
	/// at +2, Jake at +4 from each entry start (+1 word skips an unused
	/// header slot). Pass the mystery's kdTextIndex() pointer.
	void sayKDDigital(const byte *kdTextIndex, uint kdspeak, uint partner);

	/// `_SayKDHintDigital @ 2542:187e` — EEM2/London partner chain-hint voice.
	/// Like `sayKDDigital` but indexes the table 0x3a after KDTextIndex (vs
	/// 0x12) with no +1 bias. @p slot is the 0..4 hint-chain slot.
	void sayKDHintDigital(const byte *kdTextIndex, uint slot, uint partner);

	/// _QuitSounds @ 1ff1:03c5.
	void stopAll();

private:
	struct SoundEntry {
		uint32 offset;
		uint32 compressedSize;
		uint32 uncompressedSize;
	};

	bool readSdxIndex(const Common::Path &sdxPath);
	bool initMacMysterySounds(uint mysteryNum);
	bool playMacMysterySound(uint num);
	void playMacSnd(uint16 resourceId, Audio::SoundHandle &handle,
					Audio::Mixer::SoundType type);
	void playPcmBuffer(byte *pcm, uint32 size, uint sampleRate,
					   Audio::SoundHandle &handle,
					   Audio::Mixer::SoundType type);

	EEMEngine *_vm = nullptr;
	Audio::Mixer *_mixer = nullptr;
	Audio::SoundHandle _voiceHandle;
	Audio::SoundHandle _spoolHandle;

	Common::Array<SoundEntry> _sdxIndex;
	Common::Path _sdbPath;
	Common::Path _macMysterySoundPath;
	int _currentMystery = -1;
	bool _voiceEnabled = true;
	bool _isMacintosh = false;
};

} // End of namespace EEM

#endif
