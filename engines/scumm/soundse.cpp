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
		indexXWBFile(kSoundSETypeMusic);
		indexXWBFile(kSoundSETypeSFX);
		indexXWBFile(kSoundSETypeSpeech);
		indexXWBFile(kSoundSETypeAmbience);

		if (_vm->_game.id == GID_MONKEY2) {
			indexXWBFile(kSoundSETypeCommentary);
			// We need the speechcues.xsb file for MI2's speech,
			// since the file names, which are used to match the
			// speech cues with the audio files, are stored in there.
			indexSpeechXSBFile();

			// Patch audio files. Since this relies on file names,
			// it needs to be called after the file names are defined
			// from the speech cues above.
			indexXWBFile(kSoundSETypePatch);
		}
		break;

	case GID_TENTACLE:
	case GID_FT:
		initAudioMappingDOTTAndFT();
		indexFSBFile(kSoundSETypeMusic);
		indexFSBFile(kSoundSETypeSFX);
		indexFSBFile(kSoundSETypeSpeech);
		indexFSBFile(kSoundSETypeCommentary);
		// TODO: iMUSEClient_SFX_STREAMING.fsb for FT

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
		delete f;                             \
		return;                               \
	}

void SoundSE::indexXWBFile(SoundSEType type) {
	// This implementation is based off unxwb: https://github.com/mariodon/unxwb/
	// as well as xwbdump: https://raw.githubusercontent.com/wiki/Microsoft/DirectXTK/xwbdump.cpp
	// Only the parts that apply to the Special Editions of
	// MI1 and MI2 have been implemented.

	struct SegmentData {
		uint32 offset;
		uint32 length;
	};
	SegmentData segments[5] = {};

	AudioIndex *audioIndex = getAudioEntries(type);
	Common::SeekableReadStream *f = getAudioFile(type);
	if (!f)
		return;

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
		entry.isPatched = false;

		audioIndex->push_back(entry);
	}

	const uint32 nameOffset = segments[kXWBSegmentEntryNames].offset;

	if (nameOffset) {
		f->seek(nameOffset);

		for (uint32 i = 0; i < entryCount; i++) {
			Common::String name = f->readString(0, 64);
			name.toLowercase();

			if (type != kSoundSETypePatch) {
				(*audioIndex)[i].name = name;
				_nameToIndex[name] = i;
			} else {
				// Patch audio resources for MI2
				// Note: We assume that patch XWB files always contain file names

				// In Monkey Island 2, there's a gag with a phone operator from
				// the LucasArts help line, Chester, who responds to a call from
				// a phone located inside the Dinky Island jungle (room 155, boot
				// param 996). In the classic version, Chester was female, but was
				// replaced by a male operator in the Special Edition. The original
				// audio files for Chester are "chf_97_*, and the new audio files
				// are che_97_*. We patch the female voice for Chester's sound files
				// here.
				if (name.hasPrefix("chf_97_jungleb_")) {
					name.setChar('e', 2);
				}

				// Note: The original patch also contained the following entries:
				// - Fixes for audio sync during the skeleton dance / dream
				//   sequence (boot param 675). These are not needed for the
				//   classic version, and only apply to the Special Edition.
				// - Missing music files for Dinky Jungle. We don't use these
				//   yet, so we don't patch them.
				//   TODO: Process and patch music entries, once we start using
				//   the SE audio files for music.
				const int32 originalAudioIndex = _nameToIndex[name];
				if (originalAudioIndex < (int32)_speechEntries.size() && _speechEntries[originalAudioIndex].name == name) {
					_speechEntries[originalAudioIndex].isPatched = true;
					_nameToIndexPatched[name] = i;
				}
			}
		}
	}

	delete f;
}

#undef WARN_AND_RETURN_XWB

void SoundSE::indexSpeechXSBFile() {
	Common::List<uint16> speechIndices;

	AudioIndex *audioIndex = getAudioEntries(kSoundSETypeSpeech);
	Common::SeekableReadStream *f = getAudioFile("speechcues.xsb");
	if (!f)
		return;

	const uint32 magic = f->readUint32BE();
	if (magic != MKTAG('S', 'D', 'B', 'K')) {
		warning("Invalid XSB file");
		delete f;
		return;
	}

	f->skip(15);
	const uint32 entryCount = f->readUint32LE();
	f->skip(19);
	const uint32 nameOffset = f->readUint32LE();
	f->skip(24);
	const uint32 entriesOffset = f->readUint32LE();

	f->seek(entriesOffset);

	for (uint32 i = 0; i < entryCount; i++) {
		uint16 entryTag = f->readUint16LE();
		bool isSpeech = (entryTag == 0x0410);
		f->skip(7);
		const uint16 speechIndex = f->readUint16LE();
		speechIndices.push_back(speechIndex);
		//debug("indexSpeechXSBFile: speech cue %d -> index %d, offset %d", i, speechIndex, f->pos());
		f->skip(isSpeech ? 8 : 1);
	}

	f->seek(nameOffset);

	for (auto &index : speechIndices) {
		Common::String name = f->readString(0);
		name.toLowercase();

		if (index < (*audioIndex).size()) {
			(*audioIndex)[index].name = name;
			_nameToIndex[name] = index;
			//debug("indexSpeechXSBFile: %s -> index %d", name.c_str(), index);
		}
	}

	delete f;
}

#define GET_FSB5_OFFSET(X) ((((X) >> (uint64)7) << (uint64)5) & (((uint64)1 << (uint64)32) - 1))

void SoundSE::indexFSBFile(SoundSEType type) {
	// Based off DoubleFine Explorer: https://github.com/bgbennyboy/DoubleFine-Explorer/blob/master/uDFExplorer_FSBManager.pas
	// and fsbext: https://aluigi.altervista.org/search.php?src=fsbext

	AudioIndex *audioIndex = getAudioEntries(type);
	Common::SeekableReadStream *f = getAudioFile(type);
	if (!f)
		return;

	const uint32 headerSize = 60; // 4 * 7 + 8 + 16 + 8
	const uint32 magic = f->readUint32BE();
	if (magic != MKTAG('F', 'S', 'B', '5')) {
		warning("Invalid FSB file");
		delete f;
		return;
	}

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
		uint32 sampleType = origOffset & ((1 << 7) - 1);
		const uint32 fileOffset = nameOffset + nameSize + GET_FSB5_OFFSET(origOffset);
		uint32 size;

		// Meta data, skip it
		while (sampleType & 1) {
			const uint32 t = f->readUint32LE();
			sampleType = t & 1;
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
		entry.isPatched = false;

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

	delete f;
}

#undef GET_FSB5_OFFSET

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
	Common::SeekableReadStream *f = getAudioFile("speech.info");
	if (!f)
		return;

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

	delete f;
}

void SoundSE::initAudioMappingDOTTAndFT() {
	Common::SeekableReadStream *f = getAudioFile("audiomapping.info");
	if (!f)
		return;

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

	delete f;
}

Common::String SoundSE::getAudioFilename(SoundSEType type) {
	const bool isMonkey = _vm->_game.id == GID_MONKEY || _vm->_game.id == GID_MONKEY2;
	const bool isTentacle = _vm->_game.id == GID_TENTACLE;
	const bool isFT = _vm->_game.id == GID_FT;

	switch (type) {
	case kSoundSETypeMusic:
	case kSoundSETypeCDAudio:
		return isMonkey ? "MusicOriginal.xwb" : "iMUSEClient_Music.fsb";
	case kSoundSETypeSpeech:
		if (isMonkey)
			return "Speech.xwb";
		else if (isTentacle)
			return "iMUSEClient_VO.fsb";
		else if (isFT)
			return "iMUSEClient_SPEECH.fsb";
		else
			error("getAudioFilename: unknown game type in SoundSEType %d", type);
		break;
	case kSoundSETypeSFX:
		if (isMonkey)
			return "SFXOriginal.xwb";
		else if (isTentacle)
			return "iMUSEClient_SFX.fsb";
		else if (isFT)
			return "iMUSEClient_SFX_INMEMORY.fsb";
		else
			error("getAudioFilename: unknown game type in SoundSEType %d", type);
		break;
	case kSoundSETypeAmbience:
		return "Ambience.xwb";
	case kSoundSETypeCommentary:
		return isMonkey ? "commentary.xwb" : "iMUSEClient_Commentary.fsb";
	case kSoundSETypePatch:
		return "patch.xwb";
	default:
		error("getAudioFilename: unknown SoundSEType %d", type);
		break;
	}
}

Common::SeekableReadStream *SoundSE::getAudioFile(SoundSEType type) {
	Common::String audioFileName = getAudioFilename(type);
	return getAudioFile(audioFileName);
}

Common::SeekableReadStream *SoundSE::getAudioFile(const Common::String &filename) {
	if (_vm->_game.id == GID_MONKEY || _vm->_game.id == GID_MONKEY2) {
		Common::File *audioFile = new Common::File();
		if (!audioFile->open(Common::Path(filename))) {
			warning("getAudioFile: failed to open %s", filename.c_str());
			delete audioFile;
			return nullptr;
		}
		return audioFile;
	} else {
		ScummPAKFile *audioFile = new ScummPAKFile(_vm);
		if (!_vm->openFile(*audioFile, Common::Path(filename))) {
			warning("getAudioFile: failed to open %s", filename.c_str());
			delete audioFile;
			return nullptr;
		}
		return audioFile;
	}
}

SoundSE::AudioIndex *SoundSE::getAudioEntries(SoundSEType type) {
	switch (type) {
	case kSoundSETypeMusic:
	case kSoundSETypeCDAudio:
		return &_musicEntries;
	case kSoundSETypeSpeech:
		return &_speechEntries;
	case kSoundSETypeSFX:
		return &_sfxEntries;
	case kSoundSETypeAmbience:
		return &_ambienceEntries;
	case kSoundSETypeCommentary:
		return &_commentaryEntries;
	case kSoundSETypePatch:
		return &_patchEntries;
	default:
		error("getAudioEntries: unknown SoundSEType %d", type);
	}
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
	} else if (_vm->_game.id == GID_TENTACLE || _vm->_game.id == GID_FT) {
		return (_offsetToIndexDOTTAndFT.contains(offset)) ? (int32)_offsetToIndexDOTTAndFT[offset] : -1;
	}

	return -1;
}

int32 SoundSE::getAppropriateSpeechCue(const char *msgString, const char *speechFilenameSubstitution,
											   uint16 roomNumber, uint16 actorTalking, uint16 scriptNum, uint16 scriptOffset, uint16 numWaits) {
	uint32 hash = calculateStringHash(msgString);
	uint32 tmpHash = hash;
	AudioEntryMI *curAudioEntry;
	uint16 script;
	int32 currentScore;
	int32 bestScore = 0x40000000; // This is the score that we have to minimize...
	int32 bestScoreIdx = -1;

	if (!hash || _audioEntriesMI.empty())
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

Audio::SeekableAudioStream *SoundSE::getAudioStreamFromOffset(uint32 offset, SoundSEType type) {
	int32 index = getSoundIndexFromOffset(offset);

	if (index < 0) {
		warning("getAudioStreamFromOffset: sound index not found for offset %d", offset);
		return nullptr;
	}

	return getAudioStreamFromIndex(index, type);
}

Audio::SeekableAudioStream *SoundSE::getAudioStreamFromIndex(int32 index, SoundSEType type) {
	AudioIndex *audioIndex = getAudioEntries(type);
	AudioEntry audioEntry = {};

	if (index < 0 || index >= (int32)(*audioIndex).size())
		return nullptr;

	audioEntry = (*audioIndex)[index];

	// Load patched audio files, if present
	if (audioEntry.isPatched && _nameToIndexPatched.contains(audioEntry.name)) {
		int32 patchedEntry = _nameToIndexPatched[audioEntry.name];
		type = kSoundSETypePatch;
		audioIndex = getAudioEntries(type);
		audioEntry = (*audioIndex)[patchedEntry];
	}

	Common::SeekableReadStream *f = getAudioFile(type);
	if (!f)
		return nullptr;

	Common::SeekableSubReadStream *subStream = new Common::SeekableSubReadStream(
		f,
		audioEntry.offset,
		audioEntry.offset + audioEntry.length,
		DisposeAfterUse::YES
	);

	return createSoundStream(subStream, audioEntry);
}

Common::String calculateCurrentString(const char *msgString) {
	char currentChar = *msgString;
	bool shouldContinue = true;
	char messageBuffer[512];
	char *outMsgBuffer = messageBuffer;

	memset(messageBuffer, 0, sizeof(messageBuffer));

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
		return _nameToIndex.contains(entry->speechFile) ? _nameToIndex[entry->speechFile] : -1;
	}

	return -1;
}

} // End of namespace Scumm
