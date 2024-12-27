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
		initAudioMappingMI();
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
		initAudioMapping();
		_musicFilename = "iMUSEClient_Music.fsb";
		indexFSBFile(_musicFilename, &_musicEntries);
		_sfxFilename = "iMUSEClient_SFX.fsb";
		indexFSBFile(_sfxFilename, &_sfxEntries);
		_speechFilename = "iMUSEClient_VO.fsb";
		indexFSBFile(_speechFilename, &_speechEntries);
		// TODO: iMUSEClient_Commentary.fsb
		break;
	case GID_FT:
		initAudioMapping();
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

	// Clear the original offset map, as we no longer need it
	_audioNameToOriginalOffsetMap.clear();
}

Audio::SeekableAudioStream *SoundSE::getXWBTrack(int track) {
	// TODO: Enable once WMA audio is implemented.
	// Also, some of the PCM music tracks are not playing correctly
	// (e.g. the act 1 track)
	return nullptr;

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

	f->seek(segments[kXWBSegmentEntryNames].offset);

	for (uint32 i = 0; i < entryCount; i++) {
		Common::String name = f->readString(0, 64);
		name.toLowercase();

		(*audioIndex)[i].name = name;

		if (!_audioNameToOriginalOffsetMap.contains(name)) {
			// warning("indexXWBFile: name %s not found in speech.info", name.c_str());
			continue;
		}

		const uint32 origOffset = _audioNameToOriginalOffsetMap[name];
		_offsetToIndex[origOffset] = i;
		// debug("indexXWBFile: %s -> offset %d, index %d", name.c_str(), origOffset, i);
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

	uint64 pos = f->pos();
	f->seek(nameOffset);
	const uint32 firstNameOffset = nameOffset + f->readUint32LE();
	f->seek(pos);

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

	f->seek(firstNameOffset);

	for (uint32 i = 0; i < sampleCount; i++) {
		Common::String name = f->readString();
		name.toLowercase();

		// Ignore sound files for the SE in-game UI
		if (name.hasPrefix("ui_") || name.hasPrefix("ft_front_end_"))
			continue;

		// Ignore seemingly duplicate audio files in FT
		if (name.hasSuffix("-copy"))
			continue;

		// TODO: Support non-English audio files
		if (name.hasPrefix("de_") || name.hasPrefix("fr_") || name.hasPrefix("it_"))
			continue;

		if (name.hasPrefix("en_") || name.hasPrefix("de_") || name.hasPrefix("fr_") || name.hasPrefix("it_"))
			name = name.substr(3);

		// Ignore classic files and use the HQ ones
		if (name.hasPrefix("classic_"))
			continue;

		if (name.hasPrefix("hq_"))
			name = name.substr(3);

		(*audioIndex)[i].name = name;

		if (!_audioNameToOriginalOffsetMap.contains(name)) {
			//warning("indexFSBFile: name %s not found in audiomapping.info", name.c_str());
			continue;
		}

		const uint32 origOffset = _audioNameToOriginalOffsetMap[name] & 0xFFFFFF00;
		_offsetToIndex[origOffset] = i;
		//debug("indexFSBFile: %s -> offset %d, index %d", name.c_str(), origOffset, i);
	}

	f->close();
	delete f;
}

#undef GET_FSB5_OFFSET
#undef WARN_AND_RETURN_FSB

void SoundSE::initAudioMappingMI() {
	Common::File *f = new Common::File();
	if (!f->open(Common::Path("speech.info"))) {
		delete f;
		return;
	}

	do {
		AudioEntryMI entry;
		entry.hash = f->readUint32LE();
		entry.room = f->readUint16LE();
		entry.script = f->readUint16LE();
		entry.localScriptOffset = f->readUint16LE();
		entry.messageIndex = f->readUint16LE();
		entry.isEgoTalking = f->readUint16LE();
		entry.wait = f->readUint16LE();

		entry.textEnglish = f->readString(0, 256);
		entry.textFrench  = f->readString(0, 256);
		entry.textItalian = f->readString(0, 256);
		entry.textGerman  = f->readString(0, 256);
		entry.textSpanish = f->readString(0, 256);

		entry.speechFile  = f->readString(0, 32);
		entry.speechFile.toLowercase();

		//debug("hash %d, room %d, script %d, localScriptOffset: %d, messageIndex %d, isEgoTalking: %d, wait: %d, textEnglish '%s', speechFile '%s'",
		//	  entry.hash, entry.room, entry.script,
		//	  entry.localScriptOffset, entry.messageIndex, entry.isEgoTalking, entry.wait,
		//	  entry.textEnglish.c_str(), entry.speechFile.c_str());

		_audioNameToOriginalOffsetMap[entry.speechFile] = getAudioOffsetForMI(
			entry.room,
			entry.script,
			entry.localScriptOffset,
			entry.messageIndex
		);

		//_audioEntriesMI.push_back(entry);
	} while (!f->eos());

	f->close();
	delete f;
}

void SoundSE::initAudioMapping() {
	ScummPAKFile *f = new ScummPAKFile(_vm);
	_vm->openFile(*f, Common::Path("audiomapping.info"));

	do {
		const uint32 origOffset = f->readUint32LE();
		Common::String name = f->readString(0, 64);
		name.toLowercase();

		if (f->eos())
			break;
		f->skip(4); // unknown flag
		if (_vm->_game.id == GID_FT)
			f->skip(4); // unknown flag

		_audioNameToOriginalOffsetMap[name] = origOffset + 10;
	} while (!f->eos());

	f->close();
	delete f;
}

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
#ifdef USE_MAD
		return Audio::makeMP3Stream(
			stream,
			DisposeAfterUse::YES
		);
#else
		warning("createSoundStream: MP3 codec is not built in");
		delete stream;
		return nullptr;
#endif
	}

	error("createSoundStream: Unknown XWB codec %d", entry.codec);
}

int32 SoundSE::getSoundIndexFromOffset(uint32 offset) {
	uint32 offsetToCheck = offset;

	switch (_vm->_game.id) {
	case GID_MONKEY:
		if (_vm->_currentScriptSavedForSpeechMI < 0)
			return -1;

		offsetToCheck = getAudioOffsetForMI(
			_vm->_currentRoom,
			_vm->_currentScriptSavedForSpeechMI,
			offset,
			_vm->_currentSpeechIndexMI
		);
		break;
	case GID_TENTACLE:
		// Some of the remastered sound offsets are off compared to the
		// ones from the classic version, so we chop off the last 2 digits
		offsetToCheck = offset & 0xFFFFFF00;
		break;
	}

	if (_offsetToIndex.contains(offsetToCheck))
		return _offsetToIndex[offsetToCheck];
	else
		return -1;
}

Audio::AudioStream *SoundSE::getAudioStream(uint32 offset, SoundSEType type) {
	Common::SeekableReadStream *stream;
	Common::String audioFileName;
	AudioIndex &audioEntries = _musicEntries;
	int32 soundIndex = 0;

	switch (type) {
	case kSoundSETypeMusic:
		audioFileName = _musicFilename;
		audioEntries = _musicEntries;
		soundIndex = getSoundIndexFromOffset(offset);
		break;
	case kSoundSETypeSpeech:
		audioFileName = _speechFilename;
		audioEntries = _speechEntries;
		soundIndex = getSoundIndexFromOffset(offset);
		break;
	case kSoundSETypeSFX:
		audioFileName = _sfxFilename;
		audioEntries = _sfxEntries;
		soundIndex = getSoundIndexFromOffset(offset);
		break;
	}

	if (soundIndex == -1) {
		warning("getAudioStream: sound index not found for offset %d", offset);
		return nullptr;
	}

	if (_vm->_game.id == GID_MONKEY || _vm->_game.id == GID_MONKEY2) {
		Common::File *audioFile = new Common::File();
		stream = audioFile;
		if (!audioFile->open(Common::Path(audioFileName))) {
			delete audioFile;
			return nullptr;
		}
	} else {
		ScummPAKFile *audioFile = new ScummPAKFile(_vm);
		stream = audioFile;
		if (!_vm->openFile(*audioFile, Common::Path(audioFileName))) {
			delete audioFile;
			return nullptr;
		}
	}

	AudioEntry audioEntry = audioEntries[soundIndex];
	Common::SeekableSubReadStream *subStream = new Common::SeekableSubReadStream(
		stream,
		audioEntry.offset,
		audioEntry.offset + audioEntry.length,
		DisposeAfterUse::YES
	);

	return createSoundStream(subStream, audioEntry);
}

uint32 SoundSE::getAudioOffsetForMI(int32 room, int32 script, int32 localScriptOffset, int32 messageIndex) {
	return ((room + script + messageIndex) << 16) | (localScriptOffset & 0xFFFF);
}

} // End of namespace Scumm
