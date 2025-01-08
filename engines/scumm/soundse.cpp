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
		initAudioMappingMI();
		_musicFilename = "MusicOriginal.xwb";
		//_musicFilename = "MusicNew.xwb";	// TODO: allow toggle between original and new music
		indexXWBFile(_musicFilename, &_musicEntries);
		//_sfxFilename = "SFXOriginal.xwb";
		//_sfxFilename = "SFXNew.xwb";	// TODO: allow toggle between original and new SFX
		//indexXWBFile(_sfxFilename, &_sfxEntries);
		_speechFilename = "Speech.xwb";
		indexXWBFile(_speechFilename, &_speechEntries);
		// TODO: ambience.xwb
		// TODO: roomsfx.xwb
		break;
	case GID_MONKEY2:
		initAudioMappingMI();
		_musicFilename = "MusicOriginal.xwb";
		//_musicFilename = "MusicNew.xwb";	// TODO: allow toggle between original and new music
		indexXWBFile(_musicFilename, &_musicEntries);
		indexXSBFile("musiccuesoriginal.xsb", &_musicEntries);
		//_sfxFilename = "SFXOriginal.xwb";
		//_sfxFilename = "SFXNew.xwb";	// TODO: allow toggle between original and new SFX
		// indexXWBFile(_sfxFilename, &_sfxEntries);
		_speechFilename = "Speech.xwb";
		indexXWBFile(_speechFilename, &_speechEntries);
		indexXSBFile("speechcues.xsb", &_speechEntries);
		// TODO: ambience.xwb
		// TODO: commentary.xwb
		// TODO: patch.xwb
		break;
	case GID_TENTACLE:
		initAudioMappingDOTTAndFT();
		_musicFilename = "iMUSEClient_Music.fsb";
		indexFSBFile(_musicFilename, &_musicEntries);
		_sfxFilename = "iMUSEClient_SFX.fsb";
		indexFSBFile(_sfxFilename, &_sfxEntries);
		_speechFilename = "iMUSEClient_VO.fsb";
		indexFSBFile(_speechFilename, &_speechEntries);
		// TODO: iMUSEClient_Commentary.fsb

		// Clear the original offset map, as we no longer need it
		_nameToOffsetDOTTAndFT.clear();
		break;
	case GID_FT:
		initAudioMappingDOTTAndFT();
		_musicFilename = "iMUSEClient_Music.fsb";
		indexFSBFile(_musicFilename, &_musicEntries);
		_sfxFilename = "iMUSEClient_SFX_INMEMORY.fsb";
		indexFSBFile(_sfxFilename, &_sfxEntries);
		_speechFilename = "iMUSEClient_SPEECH.fsb";
		indexFSBFile(_speechFilename, &_speechEntries);
		// TODO: iMUSEClient_SFX_STREAMING.fsb
		// TODO: iMUSEClient_UI.fsb
		// TODO: iMUSEClient_Commentary.fsb

		// Clear the original offset map, as we no longer need it
		_nameToOffsetDOTTAndFT.clear();
		break;
	default:
		error("initSoundFiles: unhandled game");
	}
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
	// Only the parts that apply to the Special Editions of
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

	const uint32 nameOffset = segments[kXWBSegmentEntryNames].offset;

	if (nameOffset) {
		f->seek(nameOffset);

		for (uint32 i = 0; i < entryCount; i++) {
			Common::String name = f->readString(0, 64);
			name.toLowercase();

			(*audioIndex)[i].name = name;
			_nameToIndex[name] = i;
		}
	}

	f->close();
	delete f;
}

#undef WARN_AND_RETURN_XWB

#define WARN_AND_RETURN_XSB(message)          \
	{                                         \
		warning("indexXSBFile: %s", message); \
		f->close();                           \
		delete f;                             \
		return;                               \
	}

void SoundSE::indexXSBFile(const Common::String &filename, AudioIndex *audioIndex) {
	Common::List<uint16> speechIndices;

	Common::File *f = new Common::File();
	f->open(Common::Path(filename));

	const uint32 magic = f->readUint32BE();
	if (magic != MKTAG('S', 'D', 'B', 'K'))
		WARN_AND_RETURN_XSB("Invalid XSB file")

	f->skip(15);
	const uint32 entryCount = f->readUint32LE();
	f->skip(19);
	const uint32 nameOffset = f->readUint32LE();
	f->skip(24);
	const uint32 entriesOffset = f->readUint32LE();

	f->seek(entriesOffset);

	for (uint32 i = 0; i < entryCount; i++) {
		f->skip(9);
		const uint16 speechIndex = f->readUint16LE();
		speechIndices.push_back(speechIndex);
		f->skip(8);
	}

	f->seek(nameOffset);

	for (auto &index : speechIndices) {
		Common::String name = f->readString(0);
		name.toLowercase();

		if (/*index >= 0 && */index < (*audioIndex).size()) {
			(*audioIndex)[index].name = name;
			_nameToIndex[name] = index;
		}
	}

	f->close();
	delete f;
}

#undef WARN_AND_RETURN_XSB

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

		if (!_nameToOffsetDOTTAndFT.contains(name)) {
			//warning("indexFSBFile: name %s not found in audiomapping.info", name.c_str());
			continue;
		}

		const uint32 origOffset = _nameToOffsetDOTTAndFT[name];
		_offsetToIndexDOTTAndFT[origOffset] = i;
		//debug("indexFSBFile: %s -> offset %d, index %d", name.c_str(), origOffset, i);
	}

	f->close();
	delete f;
}

#undef GET_FSB5_OFFSET
#undef WARN_AND_RETURN_FSB

static int32 calculateStringHash(const char *input) {
	int32 hash = 0;
	int32 multiplier = 0x1EDD;

	for (const char *i = input; *i != '\0'; i++) {
		char current = *i;

		// Convert lowercase to uppercase...
		if (current >= 'a' && current <= 'z') {
			current -= 32;
		}

		// Process alphanumeric characters only...
		if ((current >= '0' && current <= '9') ||
			(current >= 'A' && current <= 'Z')) {
			multiplier++;
			hash ^= multiplier * current;
		}
	}

	return hash;
}

static int32 calculate4CharStringHash(const char *str) {
	int32 hash;
	int charCount;
	const char *i;
	char current;

	hash = 0;
	charCount = 0;

	// Process until the string terminator or 4 valid characters are found...
	for (i = str; *i; ++i) {
		if (charCount >= 4)
			break;

		current = *i;

		if ((current >= 'A' && current <= 'Z') || (current >= 'a' && current <= 'z')) {
			// Take the lower nibble of the char and incorporate it into the hash...
			hash = (16 * hash) | (current & 0xF);
			++charCount;
		}
	}

	return hash;
}

static int32 calculateStringSimilarity(const char *str1, const char *str2) {
	// This function is responsible for calculating a similarity score between
	// the two input strings; the closer the score is to zero, the closer the
	// two strings are. Taken from disasm.

	int str1Len = strlen(str1);
	int str2Len = strlen(str2);
	int totalPenalty = 0;
	int lastMatchOffset = 0;

	// Return 0 if first string is empty...
	if (str1Len <= 0)
		return 0;

	// Scan through first string with a sliding window...
	for (int windowPos = 3; windowPos - 3 < str1Len; windowPos++) {
		char currentChar = str1[windowPos - 3];

		// Check if the current character is alphanumeric...
		if ((currentChar >= 'a' && currentChar <= 'z') || (currentChar >= 'A' && currentChar <= 'Z') ||
			(currentChar >= '0' && currentChar <= '9')) {

			// Normalize character to 5-bit value (so that it's case insensitive)
			char normalizedChar = currentChar & 0x1F;
			int penalty = 9; // Default penalty

			// Calculate the search window bounds in the second string...
			int searchStart = (windowPos - 6 <= 0) ? 0 : windowPos - 6;
			int searchEnd = windowPos;

			// Look for matching character in second string...
			if (searchStart <= searchEnd) {
				while (searchStart < str2Len) {
					if ((str2[searchStart] & 0x1F) == normalizedChar) {
						int positionDiff = windowPos - searchStart - 3;

						// If character found at same relative position as last match...
						if (lastMatchOffset == positionDiff) {
							penalty = 0; // No penalty for consistent positioning!
						} else {
							// Penalty based on square of position difference!
							penalty = positionDiff * positionDiff;
							lastMatchOffset = positionDiff;
						}

						break;
					}

					if (++searchStart > searchEnd)
						break;
				}
			}

			totalPenalty -= penalty; // Subtract penalty from total score...
		}
	}

	return totalPenalty;
}

void SoundSE::initAudioMappingMI() {
	Common::File *f = new Common::File();
	if (!f->open(Common::Path("speech.info"))) {
		delete f;
		return;
	}

	_audioEntriesMI.clear();

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

		entry.hashFourCharString = calculate4CharStringHash(entry.textEnglish.c_str()); // From disasm

		//debug("hash %d, room %d, script %d, localScriptOffset: %d, messageIndex %d, isEgoTalking: %d, wait: %d, textEnglish '%s', speechFile '%s'",
		//	  entry.hash, entry.room, entry.script,
		//	  entry.localScriptOffset, entry.messageIndex, entry.isEgoTalking, entry.wait,
		//	  entry.textEnglish.c_str(), entry.speechFile.c_str());

		_audioEntriesMI.emplace_back(entry);
	} while (!f->eos());

	f->close();
	delete f;
}

void SoundSE::initAudioMappingDOTTAndFT() {
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

		_nameToOffsetDOTTAndFT[name] = origOffset;
	} while (!f->eos());

	f->close();
	delete f;
}

Audio::SeekableAudioStream *SoundSE::createSoundStream(Common::SeekableSubReadStream *stream, AudioEntry entry, DisposeAfterUse::Flag disposeAfterUse) {
	switch (entry.codec) {
	case kXWBCodecPCM: {
		byte flags = Audio::FLAG_LITTLE_ENDIAN;
		if (entry.bits == 1)	// 0: 8 bits, 1: 16 bits
			flags |= Audio::FLAG_16BITS;
		if (entry.channels == 2)
			flags |= Audio::FLAG_STEREO;
		return Audio::makeRawStream(stream, entry.rate, flags, disposeAfterUse);
	}
	case kXWBCodecXMA:
		// Unused in MI1SE and MI2SE
		error("createSoundStream: XMA codec not supported");
	case kXWBCodecADPCM: {
		const uint32 blockAlign = (entry.align + 22) * entry.channels;
		return Audio::makeADPCMStream(
			stream,
			disposeAfterUse,
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
			disposeAfterUse
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
	if (_vm->_game.id == GID_MONKEY || _vm->_game.id == GID_MONKEY2) {
		return offset;
	} else if (_vm->_game.id == GID_TENTACLE) {
		if (_offsetToIndexDOTTAndFT.contains(offset))
			return _offsetToIndexDOTTAndFT[offset];
		else
			return -1;
	}

	return -1;
}

int32 SoundSE::getAppropriateSpeechCue(const char *msgString, const char *speechFilenameSubstitution,
											   uint16 roomNumber, uint16 actorTalking, uint16 scriptNum, uint16 scriptOffset, uint16 numWaits) {
	uint32 hash;
	AudioEntryMI *curAudioEntry;
	uint16 script;
	int32 currentScore;
	int32 bestScore;
	int32 bestScoreIdx;
	uint32 tmpHash;

	hash = calculateStringHash(msgString);

	tmpHash = hash;
	if (!hash)
		return -1;

	bestScore = 0x40000000; // This is the score that we have to minimize...
	bestScoreIdx = -1;

	if (_audioEntriesMI.empty())
		return -1;

	for (uint curEntryIdx = 0; curEntryIdx < _audioEntriesMI.size(); curEntryIdx++) {
		curAudioEntry = &_audioEntriesMI[curEntryIdx];

		if (curAudioEntry->hash == hash &&
			curAudioEntry->messageIndex == numWaits &&
			calculate4CharStringHash(msgString) == curAudioEntry->hashFourCharString) {

			currentScore = ABS(scriptOffset - curAudioEntry->localScriptOffset - 7);
			if (curAudioEntry->room == roomNumber) {
				script = curAudioEntry->script;
				if (script && script != scriptNum)
					currentScore = 10000;
			} else {
				currentScore += 10000;
			}

			currentScore -= 10 * calculateStringSimilarity(curAudioEntry->textEnglish.c_str(), msgString);

			if (actorTalking == 255) {
				if (curAudioEntry->isEgoTalking == 1)
					currentScore += 2000;
			} else if ((actorTalking == 1) != curAudioEntry->isEgoTalking) {
				currentScore += 20000;
			}

			if (speechFilenameSubstitution &&
				scumm_strnicmp(curAudioEntry->speechFile.c_str(), speechFilenameSubstitution, strlen(speechFilenameSubstitution))) {
				currentScore += 100000;
			}
			if (currentScore < bestScore) {
				bestScore = currentScore;
				bestScoreIdx = (int32)curEntryIdx;
			}
		}

		hash = tmpHash;
	}

	return bestScoreIdx;
}

Audio::SeekableAudioStream *SoundSE::getAudioStream(uint32 offset, SoundSEType type) {
	Common::SeekableReadStream *stream;
	Common::String audioFileName;
	AudioEntry audioEntry = {};
	int32 soundIndex = (type != kSoundSETypeCDAudio) ? getSoundIndexFromOffset(offset) : (int32)offset;

	if (soundIndex == -1) {
		warning("getAudioStream: sound index not found for offset %d", offset);
		return nullptr;
	}

	switch (type) {
	case kSoundSETypeMusic:
	case kSoundSETypeCDAudio:
		audioFileName = _musicFilename;
		audioEntry = _musicEntries[soundIndex];
		break;
	case kSoundSETypeSpeech:
		audioFileName = _speechFilename;
		audioEntry = _speechEntries[soundIndex];
		break;
	case kSoundSETypeSFX:
		audioFileName = _sfxFilename;
		audioEntry = _sfxEntries[soundIndex];
		break;
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

	Common::SeekableSubReadStream *subStream = new Common::SeekableSubReadStream(
		stream,
		audioEntry.offset,
		audioEntry.offset + audioEntry.length,
		DisposeAfterUse::YES
	);

	return createSoundStream(subStream, audioEntry);
}

Common::String calculateCurrentString(const char *msgString) {
	char currentChar;
	bool shouldContinue = true;
	char messageBuffer[512];
	char *outMsgBuffer = messageBuffer;

	memset(messageBuffer, 0, sizeof(messageBuffer));

	currentChar = *msgString;

	// Handle empty string case
	if (msgString[0] == '\0') {
		messageBuffer[0] = 0;
		// TODO: This case sets a variable msgType equal to 1,
		// it's not clear where this is used in the executable...
	} else {
		// Process each character to find the control codes...
		while (shouldContinue && *msgString) {
			currentChar = *msgString;

			// If there are no control codes...
			if (currentChar != (char)0xFF && currentChar != (char)0xFE) {
				// Handle normal characters...
				switch (currentChar) {
				case '\r':
					*outMsgBuffer = '\n';
					outMsgBuffer++;
					msgString++;
					break;

				case '@':
				case 8: // "Verb next line" marker...
					msgString++;
					break;

				default: // Normal character, copy it over...
					*outMsgBuffer = *msgString;
					outMsgBuffer++;
					msgString++;
					break;
				}
			} else {
				// Handle special character sequences
				msgString++;
				currentChar = *msgString;

				switch (currentChar) {
				case 1: // "Next line" marker
					*outMsgBuffer = '\n';
					outMsgBuffer++;
					msgString++;
					break;

				case 2: // "No crlf" marker
					shouldContinue = false;
					// TODO: This case sets a variable msgType equal to 2,
					// it's not clear where this is used in the executable...
					*outMsgBuffer = '\0';
					break;

				case 3: // "Wait" marker
					*outMsgBuffer = '\0';
					// TODO: This case sets a variable msgType equal to 1,
					// it's not clear where this is used in the executable...
					shouldContinue = false;
					break;

				default:
					break; // Do nothing
				}
			}
		}

		// Handle end of string if we haven't already
		if (shouldContinue) {
			*outMsgBuffer = '\0';
			// TODO: This case sets a variable msgType equal to 1,
			// it's not clear where this is used in the executable...
		}
	}

	Common::String result(messageBuffer);

	return result;
}

int32 SoundSE::handleMISESpeech(const char *msgString, const char *speechFilenameSubstitution,
								     uint16 roomNumber, uint16 actorTalking, uint16 numWaits) {

	// Get the string without the various control codes and special characters...
	Common::String currentString = calculateCurrentString(msgString);
	const int32 entryIndex = getAppropriateSpeechCue(
		currentString.c_str(),
		speechFilenameSubstitution,
		roomNumber, actorTalking,
		(uint16)_currentScriptSavedForSpeechMI,
		(uint16)_currentScriptOffsetSavedForSpeechMI,
		numWaits
	);

	if (entryIndex >= 0 && entryIndex < (int32)_audioEntriesMI.size()) {
		const AudioEntryMI *entry = &_audioEntriesMI[entryIndex];
		//debug("Selected entry: %s (%s)", entry->textEnglish.c_str(), entry->speechFile.c_str());
		return _nameToIndex[entry->speechFile];
	}

	return -1;
}

} // End of namespace Scumm
