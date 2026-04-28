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

#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"

#include "common/compression/dcl.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "eem/audio.h"
#include "eem/detection.h"
#include "eem/eem.h"

namespace EEM {

AudioPlayer::AudioPlayer(EEMEngine *vm) :
	_vm(vm), _mixer(g_system->getMixer()) {
}

AudioPlayer::~AudioPlayer() {
	stopAll();
}

void AudioPlayer::stopAll() {
	stopVoice();
	stopSpool();
	cleanMysterySounds();
}

// VOC playback --------------------------------------------------------

void AudioPlayer::playVoc(const Common::Path &vocPath) {
	// Voice / digital audio is gated by the `DAT_2d5d_3f97` flag in
	// the original — verified at every callsite (`_DoChoosePartner @
	// 1a35:098c`, `_DisplayClue @ 2404:0845`, `_DoOpeningAnims @
	// 2520:08a8`, etc.). Setup-screen toggle and `_NewPlayer` fresh-
	// profile init both rewrite that flag. We pull it into the audio
	// player so callers don't have to duplicate the check.
	if (!_voiceEnabled) {
		debugC(2, kDebugSound, "AudioPlayer: voice disabled, skipping %s",
			   vocPath.toString().c_str());
		return;
	}
	stopVoice();

	// Mirrors `_LoadSoundName`'s `_fopen` (1ff1:02ac).
	Common::File *f = new Common::File();
	if (!f->open(vocPath)) {
		warning("AudioPlayer: %s missing", vocPath.toString().c_str());
		delete f;
		return;
	}

	Audio::SeekableAudioStream *stream =
		Audio::makeVOCStream(f, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	if (!stream) {
		warning("AudioPlayer: %s is not a valid VOC",
				vocPath.toString().c_str());
		return;
	}

	// `_PlayVoice` (1ff1:023e) goes through `_AIL_play_VOC_file` on the
	// digital channel — we route through `kSpeechSoundType` so the
	// launcher's "Speech volume" slider applies.
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_voiceHandle,
					   stream, -1, Audio::Mixer::kMaxChannelVolume,
					   0, DisposeAfterUse::YES);
	debugC(1, kDebugSound, "AudioPlayer: playVoc(%s)",
		   vocPath.toString().c_str());
}

bool AudioPlayer::isVoicePlaying() const {
	return _mixer->isSoundHandleActive(_voiceHandle);
}

void AudioPlayer::waitForVoiceDone(uint32 maxMs) {
	// Mirrors the wait loop at `_WaitForVoiceDone @ 1ff1:0221` — pumps
	// events (so animations + abort-on-click still work) while waiting
	// for the AIL voice channel to drain.
	const uint32 startMs = g_system->getMillis();
	while (isVoicePlaying() && !_vm->shouldQuit() &&
		   g_system->getMillis() - startMs < maxMs) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT ||
				event.type == Common::EVENT_RETURN_TO_LAUNCHER ||
				event.type == Common::EVENT_LBUTTONDOWN ||
				event.type == Common::EVENT_KEYDOWN) {
				stopVoice();
				return;
			}
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void AudioPlayer::stopVoice() {
	if (_mixer->isSoundHandleActive(_voiceHandle))
		_mixer->stopHandle(_voiceHandle);
}

// Spool sound ---------------------------------------------------------

bool AudioPlayer::readSdxIndex(const Common::Path &sdxPath) {
	// Mirrors `_ReadLong(&MysterySounds, size, sdx)` at 202f:0647 —
	// reads the entire .SDX into memory; each 12-byte entry is
	// (u32 offset, u32 compressed_size, u32 uncompressed_size).
	Common::File f;
	if (!f.open(sdxPath)) {
		warning("AudioPlayer: %s missing", sdxPath.toString().c_str());
		return false;
	}
	const uint32 size = f.size();
	if (size == 0 || (size % 12) != 0) {
		warning("AudioPlayer: %s has invalid size %u",
				sdxPath.toString().c_str(), size);
		return false;
	}
	const uint count = size / 12;
	_sdxIndex.resize(count);
	for (uint i = 0; i < count; i++) {
		_sdxIndex[i].offset           = f.readUint32LE();
		_sdxIndex[i].compressedSize   = f.readUint32LE();
		_sdxIndex[i].uncompressedSize = f.readUint32LE();
	}
	return true;
}

bool AudioPlayer::initMysterySounds(uint mysteryNum) {
	// Mirrors `_InitMysterySounds @ 202f:05cb` — calls
	// `_CleanMysterySounds` first, then sprintf-opens `m%u.sdx` (string
	// at 29be:144f) and `m%u.sdb` (29be:145b).
	cleanMysterySounds();

	const Common::String sdxName = Common::String::format("M%u.SDX", mysteryNum);
	const Common::String sdbName = Common::String::format("M%u.SDB", mysteryNum);
	const Common::Path sdxPath(sdxName);
	const Common::Path sdbPath(sdbName);

	if (!readSdxIndex(sdxPath)) {
		_sdxIndex.clear();
		return false;
	}
	_sdbPath = sdbPath;
	_currentMystery = (int)mysteryNum;
	debugC(1, kDebugSound, "AudioPlayer: mystery %u — %u sounds",
		   mysteryNum, (uint)_sdxIndex.size());
	return true;
}

void AudioPlayer::cleanMysterySounds() {
	stopSpool();
	_sdxIndex.clear();
	_sdbPath = Common::Path();
	_currentMystery = -1;
}

void AudioPlayer::playPcmBuffer(byte *pcm, uint32 size, uint sampleRate,
								Audio::SoundHandle &handle,
								Audio::Mixer::SoundType type) {
	// `Audio::makeRawStream` takes ownership and `free()`s the buffer
	// on stream destruction — so the caller must `malloc` (not `new`)
	// the PCM buffer.
	Audio::SeekableAudioStream *stream =
		Audio::makeRawStream(pcm, size, sampleRate, Audio::FLAG_UNSIGNED,
							 DisposeAfterUse::YES);
	if (!stream) {
		free(pcm);
		warning("AudioPlayer: makeRawStream failed");
		return;
	}
	_mixer->playStream(type, &handle, stream, -1,
					   Audio::Mixer::kMaxChannelVolume, 0,
					   DisposeAfterUse::YES);
}

void AudioPlayer::spoolSound(uint num) {
	if (!_voiceEnabled) {
		debugC(2, kDebugSound,
			   "AudioPlayer: voice disabled, skipping spoolSound(%u)", num);
		return;
	}
	if (_currentMystery < 0) {
		// No SDB/SDX bundle is loaded — floppy install (no `M*.SDB`)
		// or pre-mystery state. Silently no-op; per-voice VOC playback
		// for floppy lives elsewhere (TODO).
		debugC(2, kDebugSound,
			   "AudioPlayer: spoolSound(%u) skipped (no mystery sounds)", num);
		return;
	}
	if (num >= _sdxIndex.size()) {
		warning("AudioPlayer: spoolSound(%u) — index out of range (%u)",
				num, (uint)_sdxIndex.size());
		return;
	}
	const SoundEntry &entry = _sdxIndex[num];

	stopSpool();

	Common::File sdb;
	if (!sdb.open(_sdbPath)) {
		warning("AudioPlayer: %s missing", _sdbPath.toString().c_str());
		return;
	}
	if (!sdb.seek(entry.offset)) {
		warning("AudioPlayer: %s seek to %u failed",
				_sdbPath.toString().c_str(), entry.offset);
		return;
	}

	// Mirrors the two `_fgetc(in)` reads at 202f:02da-e1: byte 0 =
	// Sound Blaster Time Constant, byte 1 = total AIL playback blocks.
	// Convert TC -> sample rate via the standard SB formula. The block
	// count is only used internally by the AIL DDS pipeline; ScummVM's
	// mixer doesn't need it.
	const byte tc          = sdb.readByte();
	(void)sdb.readByte(); // total blocks — unused outside AIL

	// SB Time Constant: rate = 1000000 / (256 - tc). e.g. tc=0xD2 →
	// 22 kHz. Guard against the degenerate tc=0xFF (would divide by 1
	// → 1 MHz, well above what the mixer can resample sanely).
	const uint sampleRate = (tc < 0xFF)
		? (uint)(1000000u / (256u - tc))
		: 44100u;

	byte *pcm = nullptr;
	uint32 audioSize = 0;

	if (entry.compressedSize == entry.uncompressedSize) {
		// `_UncompressedSound @ 202f:03e6` — already raw PCM. The
		// `_SpoolSound` equality check at 202f:06e6 is `comp == uncomp`;
		// in that case `len = uncompressed_size` bytes follow the
		// 2-byte header. Original reads in 16 KB chunks; we slurp the
		// lot at once.
		audioSize = entry.uncompressedSize;
		pcm = (byte *)malloc(audioSize);
		if (!pcm) {
			warning("AudioPlayer: spoolSound %u oom (%u bytes)",
					num, audioSize);
			return;
		}
		if (sdb.read(pcm, audioSize) != audioSize) {
			warning("AudioPlayer: short read on uncompressed sound %u", num);
			free(pcm);
			return;
		}
	} else {
		// `_DeCompressSound @ 202f:02ad` → `EXPLODE @ 25c6:0d01`
		// (PKWARE DCL "Implode") with READDISKSOUND/WRITESOUND
		// callbacks. EXPLODE drives both ends via its OWN end-of-stream
		// marker (length token 519); the SDX `compressed_size` /
		// `uncompressed_size` are loose hints, NOT exact lengths —
		// 202f:0332 even computes `destSize - 2` and never reads it.
		// ScummVM's fixed-size `decompressDCL` overload errors when
		// the actual output exceeds our pre-allocated buffer (which
		// we saw on every M0 clue voice). Use the dynamic-sized
		// overload instead — it lets the DCL stream terminate at its
		// own marker. Source is bounded to the rest of the SDB so
		// the bit reader can't fall off the end.
		const uint32 streamStart = (uint32)sdb.pos();
		Common::SeekableSubReadStream sub(&sdb, streamStart,
										   (uint32)sdb.size(),
										   DisposeAfterUse::NO);
		Common::SeekableReadStream *out = Common::decompressDCL(&sub);
		if (!out) {
			warning("AudioPlayer: DCL decompression failed on sound %u "
					"(comp=%u, uncomp=%u)",
					num, entry.compressedSize, entry.uncompressedSize);
			return;
		}
		audioSize = (uint32)out->size();
		pcm = (byte *)malloc(audioSize);
		if (!pcm) {
			warning("AudioPlayer: spoolSound %u oom after DCL (%u bytes)",
					num, audioSize);
			delete out;
			return;
		}
		out->read(pcm, audioSize);
		delete out;
	}

	debugC(1, kDebugSound,
		   "AudioPlayer: spoolSound(%u) tc=0x%02x rate=%u size=%u %s",
		   num, tc, sampleRate, audioSize,
		   entry.compressedSize == entry.uncompressedSize ? "raw" : "DCL");

	// `_AIL_start_digital_playback` at 202f:040c — we route through
	// `kSFXSoundType` so the launcher's "SFX volume" slider applies
	// (this is the same slider the original would've targeted via
	// `_AIL_set_digital_master_volume`). Voice clips on the spool path
	// are gameplay SFX, not the speech-only VOC stream.
	playPcmBuffer(pcm, audioSize, sampleRate, _spoolHandle,
				  Audio::Mixer::kSFXSoundType);
}

bool AudioPlayer::isSpoolPlaying() const {
	return _mixer->isSoundHandleActive(_spoolHandle);
}

void AudioPlayer::waitForSpoolDone(uint32 maxMs) {
	const uint32 startMs = g_system->getMillis();
	while (isSpoolPlaying() && !_vm->shouldQuit() &&
		   g_system->getMillis() - startMs < maxMs) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT ||
				event.type == Common::EVENT_RETURN_TO_LAUNCHER ||
				event.type == Common::EVENT_LBUTTONDOWN ||
				event.type == Common::EVENT_KEYDOWN) {
				stopSpool();
				return;
			}
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void AudioPlayer::stopSpool() {
	if (_mixer->isSoundHandleActive(_spoolHandle))
		_mixer->stopHandle(_spoolHandle);
}

void AudioPlayer::sayKDDigital(const byte *kdTextIndex, uint kdspeak,
							   uint partner) {
	if (!kdTextIndex || _currentMystery < 0)
		return;
	// `_SayKDDigital @ 2404:0fbc`:
	//   iVar1 = kdspeak * 2;
	//   if (_Partner == 0) iVar1++;            // Jake offset
	//   sound = *(u16 *)(KDDigitalIndex + (iVar1 + 1) * 2) - 1;
	//   _SpoolSound(sound);
	// KDDigitalIndex sits 18 bytes (`+ 0x12`) after KDTextIndex per
	// `_ReadMystery` 2404:0163-0167.
	const byte *digital = kdTextIndex + 0x12;
	const uint slot = (kdspeak * 2) + (partner == 0 ? 1u : 0u) + 1u;
	const uint16 raw = READ_LE_UINT16(digital + slot * 2);
	if (raw == 0 || raw == 0xFFFF)
		return;
	spoolSound((uint)(raw - 1));
}

} // End of namespace EEM
