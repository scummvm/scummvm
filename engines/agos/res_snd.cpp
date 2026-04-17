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

#include "common/config-manager.h"
#include "common/array.h"
#include "common/compression/packice.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/textconsole.h"

#include "agos/intern.h"
#include "agos/agos.h"
#include "agos/midi.h"
#include "agos/sound.h"
#include "agos/vga.h"
#include "drivers/elvira_atarist.h"

#include "backends/audiocd/audiocd.h"

#include "audio/audiostream.h"
#include "audio/mods/protracker.h"
#include "audio/mods/desktoptracker.h"

namespace AGOS {


static Common::Array<byte> unsquashAcornDesktopTracker(const byte *data, uint32 compLen) {

	const byte mode = data[0];

	uint32 pos = 4;
	uint32 r6 = 4;

	Common::Array<byte> out;

	if (mode == 1) {
		out.resize(compLen - 4);
		if (compLen > 4)
			memcpy(out.begin(), data + 4, compLen - 4);
		return out;
	}

	uint16 bitbuf = 0;
	uint32 bitsLeft = 0;

	for (;;) {
		if (r6 == compLen)
			return out;

		if (bitsLeft == 0) {

			const byte lo = data[pos];
			const byte hi = data[pos + 1];
			pos += 2;
			r6 += 2;

			bitbuf = (uint16)(lo | (hi << 8));
			bitsLeft = 16;

			if (r6 == compLen)
				return out;
		}

		const uint16 carry = (bitbuf & 1);
		bitbuf >>= 1;
		--bitsLeft;

		if (carry == 0) {
			if (r6 == compLen)
				return out;

			out.push_back(data[pos]);
			++pos;
			++r6;
		} else {
			if (r6 == compLen)
				return out;

			const byte b1 = data[pos];
			const byte b2 = data[pos + 1];
			pos += 2;
			r6 += 2;

			const uint32 length = (uint32)((b1 & 0x0F) + 1);
			const uint32 offset = (uint32)(((b1 & 0xF0) << 4) + b2);

			for (uint32 i = 0; i < length; ++i)
				out.push_back(out[out.size() - offset]);
		}

		if (r6 >= compLen)
			return out;
	}
}

static bool isElvira1PackIcePrg(const Common::Array<byte> &data) {
	return data.size() >= 0x26 && !memcmp(data.begin() + 0x1E, "Pack-Ice", 8);
}

static bool depackElvira1PackIcePrg(const Common::Array<byte> &packedData, Common::Array<byte> &unpackedData) {
	enum {
		kPackedStreamStart = 0x021C,
		kPackedStreamEnd = 0xAFE2,
		kRawSize = 0x1694C
	};

	if (!isElvira1PackIcePrg(packedData) || packedData.size() < kPackedStreamEnd)
		return false;

	return Common::decompressPackIceStream(packedData.begin(), packedData.size(), kPackedStreamStart,
			kPackedStreamEnd, kRawSize, unpackedData, false) &&
			unpackedData.size() >= 28 && READ_BE_UINT16(unpackedData.begin()) == 0x601A;
}

static bool extractEmbeddedTosPrg(const Common::Array<byte> &containerPrg, Common::Array<byte> &innerPrg) {
	if (containerPrg.size() < 28)
		return false;

	const byte *prg = containerPrg.begin();
	if (READ_BE_UINT16(prg) != 0x601A)
		return false;

	const uint32 outerTextSize = READ_BE_UINT32(prg + 2);
	const uint32 outerRelOffset = 28 + outerTextSize + READ_BE_UINT32(prg + 6) + READ_BE_UINT32(prg + 14);
	if (containerPrg.size() < outerRelOffset + 4)
		return false;

	for (uint32 off = 30; off + 28 <= 28 + outerTextSize; off += 2) {
		if (READ_BE_UINT16(prg + off) != 0x601A)
			continue;

		const uint32 textSize = READ_BE_UINT32(prg + off + 2);
		const uint32 dataSize = READ_BE_UINT32(prg + off + 6);
		const uint32 bssSize = READ_BE_UINT32(prg + off + 10);
		const uint32 symSize = READ_BE_UINT32(prg + off + 14);
		const uint32 innerOffText = off + 28;
		const uint32 innerOffData = innerOffText + textSize;
		const uint32 innerOffSym = innerOffData + dataSize;
		const uint32 innerOffRel = innerOffSym + symSize;
		if (innerOffText < off || innerOffData < innerOffText || innerOffSym < innerOffData || innerOffRel < innerOffSym)
			continue;
		if (innerOffRel + 4 > containerPrg.size())
			continue;

		const uint32 firstRel = READ_BE_UINT32(prg + innerOffRel);
		if (firstRel >= textSize + dataSize + bssSize && firstRel != 0)
			continue;

		uint32 pos = innerOffRel + 4;
		while (pos < containerPrg.size()) {
			if (containerPrg[pos++] == 0)
				break;
		}
		if (containerPrg[pos - 1] != 0)
			continue;

		innerPrg.resize(pos - off);
		memcpy(innerPrg.begin(), prg + off, pos - off);
		debug(1, "AGOS: Found embedded Atari ST PRG at 0x%X (text=0x%X, data=0x%X, bss=0x%X)",
			off, textSize, dataSize, bssSize);
		return true;
	}

	return false;
}

static Common::SeekableReadStream *openElvira1AtariSTPrg() {
	const char *const prgNames[] = {
		"ELVIRA.PRG",
		"ELVIRA+.PRG",
		"RUNENG.PRG",
		"AUTO/RUNENG.PRG",
		"AUTO/ADEMO.PRG",
		"ADEMO.PRG"
	};

	Common::File file;
	for (uint i = 0; i < ARRAYSIZE(prgNames); ++i) {
		const char *prgName = prgNames[i];
		if (!file.open(Common::Path(prgName)))
			continue;

		Common::Array<byte> prgData;
		prgData.resize((uint32)file.size());
		if (!prgData.empty() && file.read(prgData.begin(), prgData.size()) != prgData.size()) {
			warning("playMusic: Failed to read Atari ST Elvira 1 PRG '%s'", prgName);
			return nullptr;
		}

		if (isElvira1PackIcePrg(prgData)) {
			Common::Array<byte> unpackedOuterPrg;
			if (!depackElvira1PackIcePrg(prgData, unpackedOuterPrg)) {
				warning("playMusic: Failed to depack Atari ST Elvira 1 Pack-Ice PRG '%s'", prgName);
				return nullptr;
			}
			if (!extractEmbeddedTosPrg(unpackedOuterPrg, prgData)) {
				warning("playMusic: Failed to locate embedded Atari ST PRG inside depacked Elvira 1 wrapper '%s'", prgName);
				return nullptr;
			}
		}

		byte *buf = nullptr;
		if (!prgData.empty()) {
			buf = new byte[prgData.size()];
			memcpy(buf, prgData.begin(), prgData.size());
		}
		return new Common::MemoryReadStream(buf, prgData.size(), DisposeAfterUse::YES);
	}

	return nullptr;
}

// This data is hardcoded in the executable.
const int AGOSEngine_Simon1::SIMON1_GMF_SIZE[] = {
	8900, 12166,  2848,  3442,  4034,  4508,  7064,  9730,  6014,  4742,
	3138,  6570,  5384,  8909,  6457, 16321,  2742,  8968,  4804,  8442,
	7717,  9444,  5800,  1381,  5660,  6684,  2456,  4744,  2455,  1177,
	1232, 17256,  5103,  8794,  4884,    16
};

// High nibble is the file ID (STINGSx.MUS), low nibble is the SFX number
// in the file (0 based).
const byte AGOSEngine::SIMON1_RHYTHM_SFX[] = {
	0x15, 0x16, 0x2C, 0x31, 0x37, 0x3A, 0x42, 0x43, 0x44,
	0x51, 0x55, 0x61, 0x68, 0x74, 0x78, 0x83, 0x89, 0x90
};

void AGOSEngine_Simon1::playSpeech(uint16 speech_id, uint16 vgaSpriteId) {
	if (speech_id == 9999) {
		if (_subtitles)
			return;
		if (!getBitFlag(14) && !getBitFlag(28)) {
			setBitFlag(14, true);
			_variableArray[100] = 15;
			animate(4, 1, 130, 0, 0, 0);
			waitForSync(130);
		}
		_skipVgaWait = true;
	} else {
		if (_subtitles && _scriptVar2) {
			animate(4, 2, 204, 0, 0, 0);
			waitForSync(204);
			stopAnimate(204);
		}
		if (vgaSpriteId < 100)
			stopAnimate(201 + vgaSpriteId);

		loadVoice(speech_id);

		if (vgaSpriteId < 100)
			animate(4, 2, 201 + vgaSpriteId, 0, 0, 0);
	}
}

void AGOSEngine_Simon2::playSpeech(uint16 speech_id, uint16 vgaSpriteId) {
	if (speech_id == 0xFFFF) {
		if (_subtitles)
			return;
		if (!getBitFlag(14) && !getBitFlag(28)) {
			setBitFlag(14, true);
			_variableArray[100] = 5;
			animate(4, 1, 30, 0, 0, 0);
			waitForSync(130);
		}
		_skipVgaWait = true;
	} else {
		if (getGameType() == GType_SIMON2 && _subtitles && _language != Common::HE_ISR) {
			loadVoice(speech_id);
			return;
		}

		if (_subtitles && _scriptVar2) {
			animate(4, 2, 5, 0, 0, 0);
			waitForSync(205);
			stopAnimateSimon2(2,5);
		}

		stopAnimateSimon2(2, vgaSpriteId + 2);
		loadVoice(speech_id);
		animate(4, 2, vgaSpriteId + 2, 0, 0, 0);
	}
}

void AGOSEngine::skipSpeech() {
	_sound->stopVoice();
	if (!getBitFlag(28)) {
		setBitFlag(14, true);
		if (getGameType() == GType_FF) {
			_variableArray[103] = 5;
			animate(4, 2, 13, 0, 0, 0);
			waitForSync(213);
			stopAnimateSimon2(2, 1);
		} else if (getGameType() == GType_SIMON2) {
			_variableArray[100] = 5;
			animate(4, 1, 30, 0, 0, 0);
			waitForSync(130);
			stopAnimateSimon2(2, 1);
		} else {
			_variableArray[100] = 15;
			animate(4, 1, 130, 0, 0, 0);
			waitForSync(130);
			stopAnimate(1);
		}
	}
}

void AGOSEngine::loadMusic(uint16 music, bool forceSimon2GmData, bool useSimon2Remapping) {
	stopMusic();

	debug(1, "AGOSEngine::loadMusic(music=%d, forceSimon2GmData=%d, useSimon2Remapping=%d)", music, forceSimon2GmData, useSimon2Remapping);

	uint16 indexBase = forceSimon2GmData ? MUSIC_INDEX_BASE_SIMON2_GM : _musicIndexBase;

	_gameFile->seek(_gameOffsetsPtr[indexBase + music - 1], SEEK_SET);
	_midi->load(_gameFile);

	// Activate Simon 2 GM to MT-32 remapping if we force GM, otherwise
	// deactivate it (in case it was previously activated).
	_midi->setSimon2Remapping(useSimon2Remapping);

	_lastMusicPlayed = music;
	_nextMusicToPlay = -1;
}

struct ModuleOffs {
	uint8 tune;
	uint8 fileNum;
	uint32 offs;
};

static const ModuleOffs amigaWaxworksOffs[20] = {
	// Pyramid
	{2,   2, 0,   },
	{3,   2, 50980},
	{4,   2, 56160},
	{5,   2, 62364},
	{6,   2, 73688},

	// Zombie
	{8,   8, 0},
	{11,  8, 51156},
	{12,  8, 56336},
	{13,  8, 65612},
	{14,  8, 68744},

	// Mine
	{9,   9, 0},
	{15,  9, 47244},
	{16,  9, 52424},
	{17,  9, 59652},
	{18,  9, 62784},

	// Jack
	{10, 10, 0},
	{19, 10, 42054},
	{20, 10, 47234},
	{21, 10, 49342},
	{22, 10, 51450},
};

void AGOSEngine::playModule(uint16 music) {
	char filename[15];
	Common::File f;
	uint32 offs = 0;

	if (getPlatform() == Common::kPlatformAmiga && getGameType() == GType_WW) {
		// Multiple tunes are stored in music files for main locations
		for (uint i = 0; i < 20; i++) {
			if (amigaWaxworksOffs[i].tune == music) {
				music = amigaWaxworksOffs[i].fileNum;
				offs = amigaWaxworksOffs[i].offs;
			}
		}
	}

	if (getGameType() == GType_ELVIRA1 && getFeatures() & GF_DEMO)
		Common::sprintf_s(filename, "elvira2");
	else
		Common::sprintf_s(filename, "%dtune", music);

	f.open(filename);
	if (f.isOpen() == false) {
		error("playModule: Can't load module from '%s'", filename);
	}

	Audio::AudioStream *audioStream;
	if (!(getGameType() == GType_ELVIRA1 && getFeatures() & GF_DEMO) &&
		getFeatures() & GF_CRUNCHED) {

		uint32 srcSize = f.size();
		byte *srcBuf = (byte *)malloc(srcSize);
		if (f.read(srcBuf, srcSize) != srcSize)
			error("playModule: Read failed");

		uint32 dstSize = READ_BE_UINT32(srcBuf + srcSize - 4);
		byte *dstBuf = (byte *)malloc(dstSize);
		decrunchFile(srcBuf, dstBuf, srcSize);
		free(srcBuf);

		Common::MemoryReadStream stream(dstBuf, dstSize);
		audioStream = Audio::makeProtrackerStream(&stream, offs);
		free(dstBuf);
	} else {
		audioStream = Audio::makeProtrackerStream(&f);
	}

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_modHandle, audioStream);
}

void AGOSEngine_Simon2::playMusic(uint16 music, uint16 track) {
	debug(1, "AGOSEngine_Simon2::loadMusic(music=%d, track=%d)", music, track);

	if (_lastMusicPlayed == 10 && getPlatform() == Common::kPlatformDOS && _midi->usesMT32Data()) {
		// WORKAROUND Simon 2 track 10 (played during the first intro scene)
		// consist of 3 subtracks. Subtracks 2 and 3 are missing from the MT-32
		// MIDI data. The original interpreter just stops playing after track 1
		// and does not restart until the next scene.
		// We fix this by loading the GM version of track 10 and remapping the
		// instruments to MT-32.
		// The 25th Anniversary Editions of the game attempted to fix this
		// problem by replacing the track 10 MT-32 data with the track 10 GM
		// data. For these versions, we can just load the MT-32 data. However,
		// we now have to remap instruments for all subtracks.

		// Reload track 10 using GM data (if necessary) and activate instrument
		// remapping.
		bool track10Fix = getFeatures() & GF_MT32_TRACK10_FIX;
		loadMusic(10, !track10Fix && track > 0, track10Fix || track > 0);
	}

#ifdef USE_VORBIS
		Common::String trackName;

		if (track)
			Common::String::format("OGG/track%02d-%d", _lastMusicPlayed, track);
		else
			Common::String::format("OGG/track%02d", _lastMusicPlayed);

		_digitalMusicStream = Audio::SeekableAudioStream::openStreamFile(trackName.c_str());
		if (_digitalMusicStream) {
			_mixer->playStream(Audio::Mixer::kMusicSoundType, &_digitalMusicHandle, _digitalMusicStream);

			debug(1, "AGOSEngine_Simon2::playMusic(): Playing %s", trackName.c_str());

			return;
		}
#endif
	_midi->play(track);
}

void AGOSEngine_Simon1::playMusic(uint16 music, uint16 track) {
	stopMusic();

	if (getPlatform() != Common::kPlatformAmiga && (getFeatures() & GF_TALKIE) && music == 35) {
		// WORKAROUND: For a script bug in the CD versions
		// We skip this music resource, as it was replaced by
		// a sound effect, and the script was never updated.
		return;
	}

	// Support for compressed music from the ScummVM Music Enhancement Project
	_system->getAudioCDManager()->stop();
	_system->getAudioCDManager()->play(music + 1, -1, 0, 0, true);
	if (_system->getAudioCDManager()->isPlaying())
		return;

	if (getPlatform() == Common::kPlatformAmiga) {
		playModule(music);
	} else if ((getPlatform() == Common::kPlatformDOS || getPlatform() == Common::kPlatformAcorn) &&
			getFeatures() & GF_TALKIE) {
		// DOS CD and Acorn CD use the same music data.

		// Data is stored in one large data file and the GMF format does not
		// have an indication of size or end of data, so the data size has to
		// be supplied from a hardcoded list.
		int size = SIMON1_GMF_SIZE[music];

		_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music], SEEK_SET);
		_midi->load(_gameFile, size);
		_midi->play();
	} else if (getPlatform() == Common::kPlatformDOS) {
		// DOS floppy version.

		// GMF music data is in separate MODxx.MUS files.
		char filename[15];
		Common::File f;
		Common::sprintf_s(filename, "MOD%d.MUS", music);
		f.open(filename);
		if (f.isOpen() == false)
			error("playMusic: Can't load music from '%s'", filename);

		_midi->load(&f, f.size());
		if (getFeatures() & GF_DEMO) {
			// Full version music data has a loop flag in the file header, but
			// the demo needs to have this set manually.
			_midi->setLoop(true);
		}

		_midi->play();
	} else if (getPlatform() == Common::kPlatformWindows) {
		// Windows version uses SMF data in one large data file.
		_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music], SEEK_SET);

		_midi->load(_gameFile);
		_midi->setLoop(true);

		_midi->play();
	} else if (getPlatform() == Common::kPlatformAcorn) {
		// Acorn floppy version.
		// Music resources are Squash-compressed Desktop Tracker modules.
		char filename[16];
		Common::File f;
		Common::sprintf_s(filename, "%dTUNE", music);
		
		f.open(filename);
		if (!f.isOpen())
			debug("playMusic(Acorn): Can't load mod from '%s'", filename);

		const uint32 compressedSize = (uint32)f.size();
		Common::Array<byte> compresedBuffer;
		compresedBuffer.resize(compressedSize);
		f.read(compresedBuffer.begin(), compressedSize);

		Common::Array<byte> moduleData;
		
		moduleData = unsquashAcornDesktopTracker(compresedBuffer.begin(), compressedSize);
		
		if (moduleData.size() < 4 || memcmp(moduleData.begin(), "DskT", 4) != 0)
			debug("playMusic(Acorn): Unsquashed mod does not begin with 'DskT'");

		byte *modBuffer = nullptr;
		if (!moduleData.empty()) {
			modBuffer = new byte[moduleData.size()];
			memcpy(modBuffer, moduleData.begin(), moduleData.size());
		}

		Common::SeekableReadStream *memStream = new Common::MemoryReadStream(modBuffer, moduleData.size(), DisposeAfterUse::YES);
		Audio::AudioStream *audioStream = Audio::makeDesktopTrackerStream(memStream, DisposeAfterUse::YES);
		_mixer->playStream(Audio::Mixer::kMusicSoundType, &_modHandle, audioStream);
	}

}

void AGOSEngine_Simon1::playMidiSfx(uint16 sound) {
	// The sound effects in floppy disk version of
	// Simon the Sorcerer 1 are only meant for AdLib
	if (!_midi->hasMidiSfx())
		return;

	// AdLib SFX use GMF data bundled in 9 STINGSx.MUS files.
	char filename[16];
	Common::File mus_file;

	Common::sprintf_s(filename, "STINGS%i.MUS", _soundFileId);
	mus_file.open(filename);
	if (!mus_file.isOpen())
		error("playSting: Can't load sound effect from '%s'", filename);

	// WORKAROUND Some Simon 1 DOS floppy SFX use the OPL rhythm instruments.
	// This can conflict with the music using the rhythm instruments, so the
	// original interpreter disables the music rhythm notes while a sound
	// effect is playing. However, only some sound effects use rhythm notes, so
	// in many cases this is not needed and leads to the music drums needlessly
	// being disabled.
	// To improve this, the sound effect number is checked against a list of
	// SFX using rhythm notes, and only if it is in the list the music drums
	// will be disabled while it plays.
	bool rhythmSfx = false;
	// Search for the file ID / SFX ID combination in the list of SFX that use
	// rhythm notes.
	byte sfxId = (_soundFileId << 4) | sound;
	for (int i = 0; i < ARRAYSIZE(SIMON1_RHYTHM_SFX); i++) {
		if (SIMON1_RHYTHM_SFX[i] == sfxId) {
			rhythmSfx = true;
			break;
		}
	}

	_midi->stop(true);

	_midi->load(&mus_file, mus_file.size(), true);
	_midi->play(sound, true, rhythmSfx);
}

void AGOSEngine::playMusic(uint16 music, uint16 track) {
	stopMusic();

	if (getPlatform() == Common::kPlatformAmiga) {
		playModule(music);
	} else if (getPlatform() == Common::kPlatformAtariST) {
		if (getGameType() == GType_ELVIRA2) {
			Common::File *file = new Common::File();
			if (!file->open(Common::Path(Common::String::format("%dTUNE.PKD", music))))
				error("playMusic: Can't load music from '%dTUNE.PKD'", music);

			delete _elviraAtariSTPlayer;
			_elviraAtariSTPlayer = nullptr;

			_elviraAtariSTPlayer = new ElviraAtariSTPlayer(file);
		} else if (getGameType() == GType_ELVIRA1) {
			// Elvira 1 Atari ST scripts do not pass direct driver tune numbers.
			// The original prg remaps the script music IDs to PRG subtunes:
			//   1 -> 4
			//   4 -> 2
			//   7 -> 5
			//   8 -> 7
			//   9 -> 7
			//  10 -> 6
			//  14 -> 7
			// 1 and 3 appear to be unused by the
			// game's script-level music requests.
			uint16 prgTune = 0;
			switch (music) {
			case 1:
				prgTune = 4;
				break;
			case 4:
				prgTune = 2;
				break;
			case 7:
				prgTune = 5;
				break;
			case 8:
			case 9:
			case 14:
				prgTune = 7;
				break;
			case 10:
				prgTune = 6;
				break;
			default:
				warning("playMusic: unsupported Elvira 1 Atari ST music id %d", music);
				return;
			}

			Common::SeekableReadStream *stream = openElvira1AtariSTPrg();
			if (!stream) {
				warning("playMusic: Can't load Atari ST Elvira 1 PRG for music id %d", music);
				return;
			}

			delete _elviraAtariSTPlayer;
			_elviraAtariSTPlayer = nullptr;

			_elviraAtariSTPlayer = new ElviraAtariSTPlayer(stream, prgTune);
			if (!_elviraAtariSTPlayer->isValid()) {
				warning("playMusic: Unsupported or unreadable Atari ST Elvira 1 PRG, skipping music id %d", music);
				delete _elviraAtariSTPlayer;
				_elviraAtariSTPlayer = nullptr;
				return;
			}
		}
	} else {
		_midi->setLoop(true); // Must do this BEFORE loading music.

		Common::SeekableReadStream *str = nullptr;
		if (getPlatform() == Common::kPlatformPC98) {
			str = createPak98FileStream(Common::String::format("MOD%d.PAK", music).c_str());
			if (!str)
				error("playMusic: Can't load music from 'MOD%d.PAK'", music);
		} else {
			Common::File *file = new Common::File();
			if (!file->open(Common::Path(Common::String::format("MOD%d.MUS", music))))
				error("playMusic: Can't load music from 'MOD%d.MUS'", music);
			str = file;
		}

		//warning("Playing track %d", music);
		_midi->load(str);
		_midi->play();
		delete str;
	}
}

void AGOSEngine::stopMusic() {
	if (_midiEnabled) {
		_midi->stop();
	}
	_mixer->stopHandle(_modHandle);
	_mixer->stopHandle(_digitalMusicHandle);

	delete _elviraAtariSTPlayer;
	_elviraAtariSTPlayer = nullptr;

	debug(1, "AGOSEngine::stopMusic()");
}

static const byte elvira1_soundTable[100] = {
	0, 2, 0, 1, 0, 0, 0, 0, 0, 3,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 6, 4, 0, 0, 9, 0,
	0, 2, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 8, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 5, 0, 6, 6, 0, 0,
	0, 5, 0, 0, 6, 0, 0, 0, 0, 8,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

bool AGOSEngine::loadVGASoundFile(uint16 id, uint8 type) {
	Common::File in;
	char filename[15];
	byte *dst;
	uint32 srcSize, dstSize;

	if (getPlatform() == Common::kPlatformAmiga || getPlatform() == Common::kPlatformAtariST) {
		if (getGameType() == GType_ELVIRA1 && (getFeatures() & GF_DEMO) &&
			getPlatform() == Common::kPlatformAmiga) {
			Common::sprintf_s(filename, "%c%d.out", 48 + id, type);
		} else if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2) {
			Common::sprintf_s(filename, "%.2d%d.out", id, type);
		} else if (getGameType() == GType_PN) {
			Common::sprintf_s(filename, "%c%d.in", id + 48, type);
		} else {
			Common::sprintf_s(filename, "%.3d%d.out", id, type);
		}
	} else {
		if (getGameType() == GType_ELVIRA1) {
			if (elvira1_soundTable[id] == 0)
				return false;

			Common::sprintf_s(filename, "%.2d.SND", elvira1_soundTable[id]);
		} else if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
			Common::sprintf_s(filename, "%.2d%d.VGA", id, type);
		} else if (getGameType() == GType_PN) {
			Common::sprintf_s(filename, "%c%d.out", id + 48, type);
		} else {
			Common::sprintf_s(filename, "%.3d%d.VGA", id, type);
		}
	}

	in.open(filename);
	if (in.isOpen() == false || in.size() == 0) {
		return false;
	}

	dstSize = srcSize = in.size();
	if (getGameType() == GType_PN && (getFeatures() & GF_CRUNCHED)) {
		Common::Stack<uint32> data;
		byte *dataOut = nullptr;
		int dataOutSize = 0;

		for (uint i = 0; i < srcSize / 4; ++i)
			data.push(in.readUint32BE());

		decompressPN(data, dataOut, dataOutSize);
		dst = allocBlock (dataOutSize);
		memcpy(dst, dataOut, dataOutSize);
		delete[] dataOut;
	} else if (getGameType() == GType_ELVIRA1 && getFeatures() & GF_DEMO) {
		byte *srcBuffer = (byte *)malloc(srcSize);
		if (in.read(srcBuffer, srcSize) != srcSize)
			error("loadVGASoundFile: Read failed");

		dstSize = READ_BE_UINT32(srcBuffer + srcSize - 4);
		dst = allocBlock (dstSize);
		decrunchFile(srcBuffer, dst, srcSize);
		free(srcBuffer);
	} else {
		dst = allocBlock(dstSize);
		if (in.read(dst, dstSize) != dstSize)
			error("loadVGASoundFile: Read failed");
	}
	in.close();

	return true;
}

static const char *const dimpSoundList[32] = {
	"Beep",
	"Birth",
	"Boiling",
	"Burp",
	"Cough",
	"Die1",
	"Die2",
	"Fart",
	"Inject",
	"Killchik",
	"Puke",
	"Lights",
	"Shock",
	"Snore",
	"Snotty",
	"Whip",
	"Whistle",
	"Work1",
	"Work2",
	"Yawn",
	"And0w",
	"And0x",
	"And0y",
	"And0z",
	"And10",
	"And11",
	"And12",
	"And13",
	"And14",
	"And15",
	"And16",
	"And17",
};


void AGOSEngine::loadSoundFile(const char* filename) {
	Common::File in;
	if (!in.open(filename))
		error("loadSound: Can't load %s", filename);

	uint32 dstSize = in.size();
	byte *dst = (byte *)malloc(dstSize);
	if (in.read(dst, dstSize) != dstSize)
		error("loadSound: Read failed");

	_sound->playSfxData(dst, 0, 0, 0);
}

void AGOSEngine::loadSound(uint16 sound, int16 pan, int16 vol, uint16 type) {
	byte *dst;

	if (getGameId() == GID_DIMP) {
		Common::File in;
		char filename[15];

		assert(sound >= 1 && sound <= 32);
		Common::sprintf_s(filename, "%s.wav", dimpSoundList[sound - 1]);

		if (!in.open(filename))
			error("loadSound: Can't load %s", filename);

		uint32 dstSize = in.size();
		dst = (byte *)malloc(dstSize);
		if (in.read(dst, dstSize) != dstSize)
			error("loadSound: Read failed");
	} else if (getFeatures() & GF_ZLIBCOMP) {
		char filename[15];

		uint32 file, offset, srcSize, dstSize;
		if (getPlatform() == Common::kPlatformAmiga) {
			loadOffsets((const char*)"sfxindex.dat", _zoneNumber * 22 + sound, file, offset, srcSize, dstSize);
		} else {
			loadOffsets((const char*)"effects.wav", _zoneNumber * 22 + sound, file, offset, srcSize, dstSize);
		}

		if (getPlatform() == Common::kPlatformAmiga)
			Common::sprintf_s(filename, "sfx%u.wav", file);
		else
			Common::sprintf_s(filename, "effects.wav");

		dst = (byte *)malloc(dstSize);
		decompressData(filename, dst, offset, srcSize, dstSize);
	} else {
		if (_curSfxFile == nullptr)
			return;

		dst = _curSfxFile + READ_LE_UINT32(_curSfxFile + sound * 4);
	}

	if (type == Sound::TYPE_AMBIENT)
		_sound->playAmbientData(dst, sound, pan, vol);
	else if (type == Sound::TYPE_SFX)
		_sound->playSfxData(dst, sound, pan, vol);
	else if (type == Sound::TYPE_SFX5)
		_sound->playSfx5Data(dst, sound, pan, vol);
}

void AGOSEngine::playSfx(uint16 sound, uint16 freq, uint16 flags, bool digitalOnly, bool midiOnly) {
	if (_useDigitalSfx && !midiOnly) {
		loadSound(sound, freq, flags);
	} else if (!_useDigitalSfx && !digitalOnly) {
		playMidiSfx(sound);
	}
}

void AGOSEngine::loadSound(uint16 sound, uint16 freq, uint16 flags) {
	byte *dst;
	uint32 offs, size = 0;
	uint32 rate = 8000;

	if (_curSfxFile == nullptr)
		return;

	dst = _curSfxFile;
	if (getGameType() == GType_WW) {
		uint16 tmp = sound;

		while (tmp--) {
			size += READ_LE_UINT16(dst) + 4;
			dst += READ_LE_UINT16(dst) + 4;

			if (size > _curSfxFileSize)
				error("loadSound: Reading beyond EOF (%d, %d)", size, _curSfxFileSize);
		}

		size = READ_LE_UINT16(dst);
		offs = 4;
	} else if (getGameType() == GType_ELVIRA2) {
		while (READ_BE_UINT32(dst + 4) != sound) {
			size += 12;
			dst += 12;

			if (size > _curSfxFileSize)
				error("loadSound: Reading beyond EOF (%d, %d)", size, _curSfxFileSize);
		}

		size = READ_BE_UINT32(dst);
		offs = READ_BE_UINT32(dst + 8);
	} else {
		while (READ_BE_UINT16(dst + 6) != sound) {
			size += 12;
			dst += 12;

			if (size > _curSfxFileSize)
				error("loadSound: Reading beyond EOF (%d, %d)", size, _curSfxFileSize);
		}

		size = READ_BE_UINT16(dst + 2);
		offs = READ_BE_UINT32(dst + 8);
	}

	if (getGameType() == GType_PN) {
		if (freq == 0) {
			rate = 4600;
		} else if (freq == 1) {
			rate = 7400;
		} else {
			rate = 9400;
		}
	}

	// TODO: Handle other sound flags in Amiga/AtariST versions
	if (flags == 2 && _sound->isSfxActive()) {
		_sound->queueSound(dst + offs, sound, size, rate);
	} else {
		if (flags == 0)
			_sound->stopSfx();
		_sound->playRawData(dst + offs, sound, size, rate);
	}
}

void AGOSEngine::loadMidiSfx() {
	if (!_midi->hasMidiSfx())
		return;

	Common::File fxb_file;

	Common::String filename = getGameType() == GType_ELVIRA2 ? "MYLIB.FXB" : "WAX.FXB";
	fxb_file.open(Common::Path(filename));
	if (!fxb_file.isOpen())
		error("loadMidiSfx: Can't open sound effect bank '%s'", filename.c_str());

	_midi->load(&fxb_file, fxb_file.size(), true);

	fxb_file.close();
}

void AGOSEngine::playMidiSfx(uint16 sound) {
	if (!_midi->hasMidiSfx())
		return;

	_midi->play(sound, true);
}

void AGOSEngine::loadVoice(uint speechId) {
	if (getGameType() == GType_PP && speechId == 99) {
		_sound->stopVoice();
		return;
	}

	if (getFeatures() & GF_ZLIBCOMP) {
		char filename[15];

		uint32 file, offset, srcSize, dstSize;
		if (getPlatform() == Common::kPlatformAmiga) {
			loadOffsets((const char*)"spindex.dat", speechId, file, offset, srcSize, dstSize);
		} else {
			loadOffsets((const char*)"speech.wav", speechId, file, offset, srcSize, dstSize);
		}

		// Voice segment doesn't exist
		if (offset == 0xFFFFFFFF && srcSize == 0xFFFFFFFF && dstSize == 0xFFFFFFFF) {
			debug(0, "loadVoice: speechId %d removed", speechId);
			return;
		}

		if (getPlatform() == Common::kPlatformAmiga)
			Common::sprintf_s(filename, "sp%u.wav", file);
		else
			Common::sprintf_s(filename, "speech.wav");

		byte *dst = (byte *)malloc(dstSize);
		decompressData(filename, dst, offset, srcSize, dstSize);
		_sound->playVoiceData(dst, speechId);
	} else {
		_sound->playVoice(speechId);
	}
}

void AGOSEngine::stopAllSfx() {
	_sound->stopAllSfx();
	if (_midi->hasMidiSfx())
		_midi->stop(true);
}

} // End of namespace AGOS
