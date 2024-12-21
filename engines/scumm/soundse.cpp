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
#include "common/timer.h"
#include "common/util.h"
#include "common/substream.h"

#include "scumm/file.h"
#include "scumm/scumm.h"
#include "scumm/soundse.h"

#include "audio/audiostream.h"
#include "audio/decoders/adpcm.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"

namespace Scumm {

SoundSE::SoundSE(ScummEngine *parent, Audio::Mixer *mixer)
	: _vm(parent),
	_mixer(mixer) {

	initSoundFiles();
}

void SoundSE::initSoundFiles() {
	switch (_vm->_game.id) {
	case GID_MONKEY:
	case GID_MONKEY2:
		_musicFilename = "MusicOriginal.xwb";
		//_musicFilename = "MusicNew.xwb";	// TODO: allow toggle between original and new music
		indexXWBFile(_musicFilename, &_musicEntries);
		_sfxFilename = "SFXOriginal.xwb";
		//_sfxFilename = "SFXNew.xwb";	// TODO: allow toggle between original and new SFX
		indexXWBFile(_sfxFilename, &_sfxEntries);
		_speechFilename = "Speech.xwb";
		indexXWBFile(_speechFilename, &_speechEntries);
		// TODO: iMUSEClient_Commentary.fsb
		break;
	case GID_TENTACLE:
		_musicFilename = "iMUSEClient_Music.fsb";
		indexFSBFile(_musicFilename, &_musicEntries);
		_sfxFilename = "iMUSEClient_SFX.fsb";
		indexFSBFile(_sfxFilename, &_sfxEntries);
		_speechFilename = "iMUSEClient_VO.fsb";
		indexFSBFile(_speechFilename, &_speechEntries);
		// TODO: iMUSEClient_Commentary.fsb
		break;
	case GID_FT:
		_musicFilename = "iMUSEClient_Music.fsb";
		indexFSBFile(_musicFilename, &_musicEntries);
		_sfxFilename = "iMUSEClient_SFX_INMEMORY.fsb";
		indexFSBFile(_sfxFilename, &_sfxEntries);
		_speechFilename = "iMUSEClient_SPEECH.fsb";
		indexFSBFile(_speechFilename, &_speechEntries);
		// TODO: iMUSEClient_SFX_STREAMING.fsb
		// TODO: iMUSEClient_UI.fsb
		// TODO: iMUSEClient_Commentary.fsb
		break;
	default:
		error("initSoundFiles: unhandled game");
	}
}

Audio::SeekableAudioStream *SoundSE::getXWBTrack(int track) {
	Common::File *cdAudioFile = new Common::File();

	if (!cdAudioFile->open(Common::Path(_musicFilename))) {
		delete cdAudioFile;
		return nullptr;
	}

	AudioEntry entry = _musicEntries[track];

	auto subStream = new Common::SeekableSubReadStream(
		cdAudioFile,
		entry.offset,
		entry.offset + entry.length,
		DisposeAfterUse::YES
	);

	return createSoundStream(subStream, entry);
}

#define WARN_AND_RETURN_XWB(message)          \
	{                                         \
		warning("indexXWBFile: %s", message); \
		f->close();                           \
		delete f;                             \
		return;                               \
	}

void SoundSE::indexXWBFile(const Common::String &filename, AudioIndex *audioIndex) {
	// This implementation is based off unxwb: https://github.com/mariodon/unxwb/
	// as well as xwbdump: https://raw.githubusercontent.com/wiki/Microsoft/DirectXTK/xwbdump.cpp
	// Only the parts that apply to the Doublefine releases of
	// MI1 and MI2 have been implemented.

	struct SegmentData {
		uint32 offset;
		uint32 length;
	};
	SegmentData segments[5] = {};

	Common::File *f = new Common::File();
	f->open(Common::Path(filename));

	const uint32 magic = f->readUint32BE();
	const uint32 version = f->readUint32LE();
	f->skip(4); // skip dwHeaderVersion

	if (magic != MKTAG('W', 'B', 'N', 'D'))
		WARN_AND_RETURN_XWB("Invalid XWB file")

	if (version < 42)
		WARN_AND_RETURN_XWB("Unsupported XWB version")

	for (uint32 i = 0; i < 5; i++) {
		segments[i].offset = f->readUint32LE();
		segments[i].length = f->readUint32LE();
	}

	f->seek(segments[kXWBSegmentBankData].offset);
	const uint32 flags = f->readUint32LE();
	const uint32 entryCount = f->readUint32LE();
	f->skip(64); // skip bank name
	const uint32 entrySize = f->readUint32LE();
	if (entrySize < 24)
		WARN_AND_RETURN_XWB("Unsupported XWB entry size")

	if (flags & 0x00020000)
		WARN_AND_RETURN_XWB("XWB compact format is not supported")

	f->seek(segments[kXWBSegmentEntryMetaData].offset);

	for (uint32 i = 0; i < entryCount; i++) {
		AudioEntry entry;
		/*uint32 flagsAndDuration = */ f->readUint32LE();
		uint32 format = f->readUint32LE();
		entry.offset = f->readUint32LE() + segments[kXWBSegmentEntryWaveData].offset;
		entry.length = f->readUint32LE();
		/*uint32 loopOffset = */ f->readUint32LE();
		/*uint32 loopLength = */ f->readUint32LE();

		entry.codec = static_cast<AudioCodec>(format & ((1 << 2) - 1));
		entry.channels = (format >> (2)) & ((1 << 3) - 1);
		entry.rate = (format >> (2 + 3)) & ((1 << 18) - 1);
		entry.align = (format >> (2 + 3 + 18)) & ((1 << 8) - 1);
		entry.bits = (format >> (2 + 3 + 18 + 8)) & ((1 << 1) - 1);

		audioIndex->push_back(entry);
	}

	f->close();
	delete f;
}

#undef WARN_AND_RETURN_XWB

#define WARN_AND_RETURN_FSB(message)          \
	{                                         \
		warning("indexFSBFile: %s", message); \
		f->close();                           \
		delete f;                             \
		return;                               \
	}

#define GET_FSB5_OFFSET(X) ((((X) >> (uint64)7) << (uint64)5) & (((uint64)1 << (uint64)32) - 1))

void SoundSE::indexFSBFile(const Common::String &filename, AudioIndex *audioIndex) {
	// Based off DoubleFine Explorer: https://github.com/bgbennyboy/DoubleFine-Explorer/blob/master/uDFExplorer_FSBManager.pas
	// and fsbext: https://aluigi.altervista.org/search.php?src=fsbext
	ScummPAKFile *f = new ScummPAKFile(_vm);
	_vm->openFile(*f, Common::Path(filename));

	const uint32 headerSize = 60; // 4 * 7 + 8 + 16 + 8
	const uint32 magic = f->readUint32BE();
	if (magic != MKTAG('F', 'S', 'B', '5'))
		WARN_AND_RETURN_FSB("Invalid FSB file")

	/*const uint32 version = */f->readUint32LE();
	const uint32 sampleCount = f->readUint32LE();
	const uint32 sampleHeaderSize = f->readUint32LE();
	const uint32 nameSize = f->readUint32LE();
	const uint32 dataSize = f->readUint32LE();
	/*const uint32 mode = */f->readUint32LE();
	f->skip(8);	// skip zero
	f->skip(16);	// skip hash
	f->skip(8);	// skip dummy
	const uint32 nameOffset = sampleHeaderSize + headerSize;
	const uint32 baseOffset = headerSize + sampleHeaderSize + nameSize;

	for (uint32 i = 0; i < sampleCount; i++) {
		const uint32 origOffset = f->readUint32LE();
		f->skip(4); // samples, used in XMA
		uint32 type = origOffset & ((1 << 7) - 1);
		const uint32 fileOffset = nameOffset + nameSize + GET_FSB5_OFFSET(origOffset);
		uint32 size;

		// Meta data, skip it
		while (type & 1) {
			const uint32 t = f->readUint32LE();
			type = t & 1;
			const uint32 metaDataSize = (t & 0xffffff) >> 1;
			f->skip(metaDataSize);
		}

		if (f->pos() < nameOffset) {
			size = f->readUint32LE();
			f->seek(-4, SEEK_CUR);
			if (!size) {
				size = dataSize + baseOffset;
			} else {
				size = GET_FSB5_OFFSET(size) + baseOffset;
			}
		} else {
			size = dataSize + baseOffset;
		}

		size -= fileOffset;

		AudioEntry entry;
		entry.length = size;
		entry.offset = fileOffset;
		// The following are all unused - they'll
		// be read from the MP3 streams
		entry.rate = 48000;
		entry.channels = 2;
		entry.codec = kFSBCodecMP3;
		entry.align = 0;
		entry.bits = 16;

		audioIndex->push_back(entry);
	}

	f->close();
	delete f;
}

#undef GET_FSB5_OFFSET
#undef WARN_AND_RETURN_FSB

Audio::SeekableAudioStream *SoundSE::createSoundStream(Common::SeekableSubReadStream *stream, AudioEntry entry) {
	switch (entry.codec) {
	case kXWBCodecPCM: {
		byte flags = Audio::FLAG_LITTLE_ENDIAN;
		if (entry.bits == 1)	// 0: 8 bits, 1: 16 bits
			flags |= Audio::FLAG_16BITS;
		if (entry.channels == 2)
			flags |= Audio::FLAG_STEREO;
		return Audio::makeRawStream(stream, entry.rate, flags, DisposeAfterUse::YES);
	}
	case kXWBCodecXMA:
		// Unused in MI1SE and MI2SE
		error("createSoundStream: XMA codec not supported");
	case kXWBCodecADPCM: {
		const uint32 blockAlign = (entry.align + 22) * entry.channels;
		return Audio::makeADPCMStream(
			stream,
			DisposeAfterUse::YES,
			entry.length,
			Audio::kADPCMMS,
			entry.rate,
			entry.channels,
			blockAlign
		);
	}
	case kXWBCodecWMA:
		// TODO: Implement WMA codec
		/*return new Audio::WMACodec(
			2,
			entry.rate,
			entry.channels,
			entry.bits,
			entry.align,
			stream
		);*/
		warning("createSoundStream: WMA codec not implemented");
		delete stream;
		return nullptr;
	case kFSBCodecMP3:
		return Audio::makeMP3Stream(
			stream,
			DisposeAfterUse::YES
		);
	}

	error("createSoundStream: Unknown XWB codec %d", entry.codec);
}

void SoundSE::startSoundEntry(int soundIndex, SoundSEType type) {
	Common::SeekableReadStream *stream = nullptr;
	Audio::SoundHandle *handle = nullptr;
	Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType;
	Common::String audioFileName;
	AudioIndex &audioEntries = _musicEntries;

	switch (type) {
	case kSoundSETypeMusic:
		handle = &_musicHandle;
		soundType = Audio::Mixer::kMusicSoundType;
		audioFileName = _musicFilename;
		audioEntries = _musicEntries;
		break;
	case kSoundSETypeSpeech:
		handle = &_speechHandle;
		soundType = Audio::Mixer::kSpeechSoundType;
		audioFileName = _speechFilename;
		audioEntries = _speechEntries;
		break;
	case kSoundSETypeSFX:
		handle = &_sfxHandle;
		soundType = Audio::Mixer::kSFXSoundType;
		audioFileName = _sfxFilename;
		audioEntries = _sfxEntries;
		break;
	}

	if (_vm->_game.id == GID_MONKEY || _vm->_game.id == GID_MONKEY2) {
		Common::File *audioFile = new Common::File();
		stream = audioFile;
		if (!audioFile->open(Common::Path(audioFileName))) {
			delete audioFile;
			return;
		}
	} else {
		ScummPAKFile *audioFile = new ScummPAKFile(_vm);
		stream = audioFile;
		if (!_vm->openFile(*audioFile, Common::Path(audioFileName))) {
			delete audioFile;
			return;
		}
	}

	AudioEntry audioEntry = audioEntries[soundIndex];
	Common::SeekableSubReadStream *subStream = new Common::SeekableSubReadStream(
		stream,
		audioEntry.offset,
		audioEntry.offset + audioEntry.length,
		DisposeAfterUse::YES
	);

	_mixer->playStream(
		soundType,
		handle,
		createSoundStream(subStream, audioEntry)
	);
}

} // End of namespace Scumm
