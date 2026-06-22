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
#include "audio/decoders/mac_snd.h"
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
#include "eem/resource.h"

namespace EEM {

AudioPlayer::AudioPlayer(EEMEngine *vm) :
	_vm(vm), _mixer(g_system->getMixer()),
	_isMacintosh(vm && vm->isMacintosh()) {
}

struct MacSndResource {
	const char *name;
	uint16 id;
};

const MacSndResource kMacSndResources[] = {
	{ "B-0003SL", 7022 }, { "B-0004SL", 7023 }, { "B-0006SL", 7021 },
	{ "DING",     7000 }, { "F-0013SL", 8018 }, { "F-0016SL", 8017 },
	{ "F-0061SL", 8015 }, { "F-0067SL", 8016 }, { "F-0140SL", 8020 },
	{ "F-0159SL", 8014 }, { "F-0161SL", 8013 }, { "F-0165SL", 8024 },
	{ "F-0166SL", 8011 }, { "F-0168SL", 8010 }, { "F-0170SL", 8009 },
	{ "F-0177SL", 8008 }, { "F-0181SL", 8006 }, { "F-0184SL", 8005 },
	{ "F-0187SL", 8004 }, { "F-0191SL", 8002 }, { "F-0194SL", 8001 },
	{ "M-0012SL", 7018 }, { "M-0014SL", 7017 }, { "M-0054SL", 7016 },
	{ "M-0075SL", 7015 }, { "M-0083SL", 7001 }, { "M-0085SL", 7002 },
	{ "M-0089SL", 7004 }, { "M-0091SL", 7005 }, { "M-0092SL", 7006 },
	{ "M-0096SL", 7008 }, { "M-0102SL", 7009 }, { "M-0104SL", 7010 },
	{ "M-0107SL", 7011 }, { "M-0113SL", 7013 }, { "M-0115SL", 7014 },
	{ "M-0163SL", 7024 }, { "NEWSCAN",  7003 }, { "NEWSSHRT", 7007 },
	{ "PHONESL",  7012 }, { "SQUAK2SL", 7019 }, { "THUNDER",  7025 },
};

Common::String macSndNameFromPath(const Common::Path &path) {
	Common::String name = path.baseName();
	const size_t dot = name.findLastOf('.');
	if (dot != Common::String::npos)
		name = name.substr(0, dot);
	name.toUppercase();

	if (name == "PHONE")
		name = "PHONESL";

	return name;
}

uint16 macSndResourceIdForPath(const Common::Path &path) {
	const Common::String name = macSndNameFromPath(path);
	for (uint i = 0; i < ARRAYSIZE(kMacSndResources); i++) {
		if (name.equalsIgnoreCase(kMacSndResources[i].name))
			return kMacSndResources[i].id;
	}
	return 0;
}

AudioPlayer::~AudioPlayer() {
	stopAll();
}

void AudioPlayer::stopAll() {
	stopVoice();
	stopSpool();
	cleanMysterySounds();
}

void AudioPlayer::playVoc(const Common::Path &vocPath) {
	if (!_voiceEnabled) {
		debugC(2, kDebugSound, "AudioPlayer: voice disabled, skipping %s",
			   vocPath.toString().c_str());
		return;
	}
	stopVoice();

	if (_isMacintosh) {
		const uint16 resourceId = macSndResourceIdForPath(vocPath);
		if (resourceId == 0) {
			warning("AudioPlayer: Mac snd resource for %s missing",
					vocPath.toString().c_str());
			return;
		}
		playMacSnd(resourceId, _voiceHandle, Audio::Mixer::kSpeechSoundType);
		return;
	}

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

	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_voiceHandle,
					   stream, -1, Audio::Mixer::kMaxChannelVolume,
					   0, DisposeAfterUse::YES);
	debugC(1, kDebugSound, "AudioPlayer: playVoc(%s)",
		   vocPath.toString().c_str());
}

bool AudioPlayer::isVoicePlaying() const {
	return _mixer->isSoundHandleActive(_voiceHandle);
}

// _WaitForVoiceDone @ 1ff1:0221 — pumps events while the AIL voice channel
// drains, so animations + abort-on-click keep working during the wait.
void AudioPlayer::waitForVoiceDone(uint32 maxMs) {
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

// _ReadLong(&MysterySounds, size, sdx) @ 202f:0647.
// 12-byte entries: (u32 offset, u32 compressed_size, u32 uncompressed_size).
bool AudioPlayer::readSdxIndex(const Common::Path &sdxPath) {
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

// _InitMysterySounds @ 202f:05cb. Strings "m%u.sdx" @ 29be:144f, "m%u.sdb" @ 29be:145b.
bool AudioPlayer::initMysterySounds(uint mysteryNum) {
	const Common::String sdxName = Common::String::format("M%u.SDX", mysteryNum);
	const Common::String sdbName = Common::String::format("M%u.SDB", mysteryNum);
	const Common::Path sdxPath(sdxName);
	const Common::Path sdbPath(sdbName);

	cleanMysterySounds();

	if (_isMacintosh) {
		debugC(2, kDebugSound,
			   "AudioPlayer: Mac release has no SDB/SDX bundle for mystery %u",
			   mysteryNum);
		return true;
	}

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

void AudioPlayer::playMacSnd(uint16 resourceId, Audio::SoundHandle &handle,
							 Audio::Mixer::SoundType type) {
	Common::SeekableReadStream *stream =
		openMacResource(Common::Path("EEM Sound&Music"),
						MKTAG('s', 'n', 'd', ' '), resourceId);
	if (!stream) {
		warning("AudioPlayer: Mac snd resource %u missing", resourceId);
		return;
	}

	Audio::SeekableAudioStream *audioStream =
		Audio::makeMacSndStream(stream, DisposeAfterUse::YES);
	if (!audioStream) {
		delete stream;
		warning("AudioPlayer: Mac snd resource %u is not playable", resourceId);
		return;
	}

	_mixer->playStream(type, &handle, audioStream, -1,
					   Audio::Mixer::kMaxChannelVolume, 0,
					   DisposeAfterUse::YES);
	debugC(1, kDebugSound, "AudioPlayer: playMacSnd(%u)", resourceId);
}

// pcm must be allocated with malloc(): Audio::makeRawStream takes ownership
// and frees it via free() on stream destruction (NOT delete/delete[]).
void AudioPlayer::playPcmBuffer(byte *pcm, uint32 size, uint sampleRate,
								Audio::SoundHandle &handle,
								Audio::Mixer::SoundType type) {
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

// Floppy per-partner voice tables, indexed by _LoadSoundName_Floppy @ 1f4e:0305.
// Filename FAR-ptr arrays at 2608:0f0e (Jake) and 2608:0f76 (Jenny), each
// 26 * FAR-ptr to a NUL-terminated `*.voc` name. Slots align across partners,
// e.g. 12 = PHONESL.VOC, 20 = partner intro, 25 = THUNDER.VOC.
const char *const kFloppyJakeVoiceTable[26] = {
	"DING.VOC",       "M-0083SL.VOC", "M-0085SL.VOC", "NEWSCAN.VOC",
	"M-0089SL.VOC",   "M-0091SL.VOC", "M-0092SL.VOC", "NEWSSHRT.VOC",
	"M-0096SL.VOC",   "M-0102SL.VOC", "M-0104SL.VOC", "M-0107SL.VOC",
	"PHONESL.VOC",    "M-0113SL.VOC", "DING.VOC",     "DING.VOC",
	"M-0054SL.VOC",   "M-0014SL.VOC", "M-0012SL.VOC", "SQUAK2SL.VOC",
	"M-0113SL.VOC",   "B-0006SL.VOC", "B-0003SL.VOC", "B-0004SL.VOC",
	"M-0163SL.VOC",   "THUNDER.VOC",
};
const char *const kFloppyJennyVoiceTable[26] = {
	"DING.VOC",       "F-0194SL.VOC", "F-0191SL.VOC", "NEWSCAN.VOC",
	"F-0187SL.VOC",   "F-0184SL.VOC", "F-0181SL.VOC", "NEWSSHRT.VOC",
	"F-0177SL.VOC",   "F-0170SL.VOC", "F-0168SL.VOC", "F-0166SL.VOC",
	"PHONESL.VOC",    "F-0161SL.VOC", "DING.VOC",     "DING.VOC",
	"F-0067SL.VOC",   "F-0016SL.VOC", "F-0013SL.VOC", "SQUAK2SL.VOC",
	"F-0140SL.VOC",   "B-0006SL.VOC", "B-0003SL.VOC", "B-0004SL.VOC",
	"F-0165SL.VOC",   "THUNDER.VOC",
};

void AudioPlayer::playFloppyVoiceSlot(uint slot, uint partner) {
	if (slot >= 26)
		slot = 0;  // _LoadSoundName_Floppy: if (0x19 < slot) slot = 0;
	const char *name = (partner == 0)
		? kFloppyJakeVoiceTable[slot]
		: kFloppyJennyVoiceTable[slot];
	playVoc(Common::Path(name));
}

void AudioPlayer::spoolSound(uint num) {
	if (!_voiceEnabled) {
		debugC(2, kDebugSound,
			   "AudioPlayer: voice disabled, skipping spoolSound(%u)", num);
		return;
	}
	if (_currentMystery < 0) {
		// No SDB/SDX bundle loaded (floppy install or pre-mystery state).
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

	// _UncompressedSound 2-byte header @ 202f:02da-e1:
	//   byte 0 = Sound Blaster Time Constant
	//   byte 1 = total AIL playback blocks (internal to AIL DDS; unused here)
	const byte tc          = sdb.readByte();
	sdb.skip(1); // AIL block count, unused outside AIL

	// SB Time Constant formula: rate = 1000000 / (256 - tc).
	// e.g. tc=0xD2 -> 22 kHz. tc=0xFF would divide by 1 (1 MHz, nonsense); clamp.
	const uint sampleRate = (tc < 0xFF)
		? (uint)(1000000u / (256u - tc))
		: 44100u;

	byte *pcm = nullptr;
	uint32 audioSize = 0;

	if (entry.compressedSize == entry.uncompressedSize) {
		// _UncompressedSound @ 202f:03e6 — raw PCM follows the 2-byte header.
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
		// _DeCompressSound @ 202f:02ad -> EXPLODE @ 25c6:0d01 (PKWARE DCL Implode),
		// driven by READDISKSOUND/WRITESOUND callbacks. EXPLODE terminates on its
		// own length token 519 (end-of-stream marker), not on the SDX sizes:
		// 202f:0332 even computes `destSize - 2` and never reads it. The SDX
		// `compressed_size` / `uncompressed_size` fields are loose hints.
		// ScummVM's fixed-size `decompressDCL` overload errors when actual output
		// exceeds the pre-allocated buffer (observed on M0 clue voices), so use
		// the dynamic-sized overload and let DCL terminate at its own marker.
		// Source is bounded to the rest of the SDB so the bit reader stays in range.
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

	// _AIL_start_digital_playback @ 202f:040c. Spool clips are gameplay SFX.
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

// _SayKDDigital @ EEM1 2404:0fbc / EEM2 2542:1845. KDDigitalIndex =
// KDTextIndex + 0x12. EEM1 reads index `kdspeak*2 + (Jake?1:0) + 1`; EEM2
// drops the trailing +1 (`kdspeak*2 + (Jake?1:0)`) — a real per-variant
// difference, so using EEM1's +1 for London shifts every KD line by one slot.
void AudioPlayer::sayKDDigital(const byte *kdTextIndex, uint kdspeak,
							   uint partner) {
	if (!kdTextIndex || _currentMystery < 0)
		return;
	const byte *digital = kdTextIndex + 0x12;
	const uint slot = (kdspeak * 2) + (partner == 0 ? 1u : 0u) +
					  (_vm && _vm->isLondon() ? 0u : 1u);
	const uint16 raw = READ_LE_UINT16(digital + slot * 2);
	if (raw == 0 || raw == 0xFFFF)
		return;
	spoolSound((uint)(raw - 1));
}

// _SayKDHintDigital @ 2542:187e — EEM2/London partner chain-hint voice.
// Identical to EEM2 `_SayKDDigital` but indexes the table 0x3a bytes after
// KDTextIndex (vs 0x12); `slot` is the 0..4 hint-chain slot. EEM1 voices its
// chain hints through `sayKDDigital(slot + 10)` instead.
void AudioPlayer::sayKDHintDigital(const byte *kdTextIndex, uint slot,
								   uint partner) {
	if (!kdTextIndex || _currentMystery < 0)
		return;
	const byte *table = kdTextIndex + 0x3a;
	const uint idx = (slot * 2) + (partner == 0 ? 1u : 0u);
	const uint16 raw = READ_LE_UINT16(table + idx * 2);
	if (raw == 0 || raw == 0xFFFF)
		return;
	spoolSound((uint)(raw - 1));
}

} // End of namespace EEM
