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
		_xwbMusicFilename = "MusicOriginal.xwb";
		//_xwbMusicFilename = "MusicNew.xwb";	// TODO: allow toggle between original and new music
		indexXWBFile(_xwbMusicFilename, &_xwbMusicEntries);
		_xwbSfxFilename = "SFXOriginal.xwb";
		//_xwbSfxFilename = "SFXNew.xwb";	// TODO: allow toggle between original and new SFX
		indexXWBFile(_xwbSfxFilename, &_xwbSfxEntries);
		_xwbSpeechFilename = "Speech.xwb";
		indexXWBFile(_xwbSpeechFilename, &_xwbSpeechEntries);
		break;
	case GID_TENTACLE:
		// TODO
		break;
	case GID_FT:
		// TODO
		break;
	default:
		error("initSoundFiles: unhandled game");
	}
}

Audio::SeekableAudioStream *SoundSE::getXWBTrack(int track) {
	Common::File *cdAudioFile = new Common::File();

	if (!cdAudioFile->open(Common::Path(_xwbMusicFilename))) {
		delete cdAudioFile;
		return nullptr;
	}

	XWBEntry entry = _xwbMusicEntries[track];

	auto subStream = new Common::SeekableSubReadStream(
		cdAudioFile,
		entry.offset,
		entry.offset + entry.length,
		DisposeAfterUse::YES
	);

	return createXWBStream(subStream, entry);
}

#define WARN_AND_RETURN_XWB(message)          \
	{                                         \
		warning("indexXWBFile: %s", message); \
		f->close();                           \
		delete f;                             \
		return;                               \
	}

void SoundSE::indexXWBFile(const Common::String &filename, XWBIndex *xwbIndex) {
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
		XWBEntry entry;
		/*uint32 flagsAndDuration = */ f->readUint32LE();
		uint32 format = f->readUint32LE();
		entry.offset = f->readUint32LE() + segments[kXWBSegmentEntryWaveData].offset;
		entry.length = f->readUint32LE();
		/*uint32 loopOffset = */ f->readUint32LE();
		/*uint32 loopLength = */ f->readUint32LE();

		entry.codec = static_cast<XWBCodec>(format & ((1 << 2) - 1));
		entry.channels = (format >> (2)) & ((1 << 3) - 1);
		entry.rate = (format >> (2 + 3)) & ((1 << 18) - 1);
		entry.align = (format >> (2 + 3 + 18)) & ((1 << 8) - 1);
		entry.bits = (format >> (2 + 3 + 18 + 8)) & ((1 << 1) - 1);

		xwbIndex->push_back(entry);
	}

	f->close();
	delete f;
}

#undef WARN_AND_RETURN_XWB

Audio::SeekableAudioStream *SoundSE::createXWBStream(Common::SeekableSubReadStream *stream, XWBEntry entry) {
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
		error("createXWBStream: XMA codec not supported");
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
		warning("createXWBStream: WMA codec not implemented");
		delete stream;
		return nullptr;
	}

	error("createXWBStream: Unknown XWB codec %d", entry.codec);
}

#if 0
void SoundSE::startMusic(int soundID) {
	int entry = -1;

	// HACK: Find the first entry with offset 8192 (MI2 theme)
	// TODO: Map soundID to entry (*.xsb files)
	for (int i = 0; i < _xwbMusicEntries.size(); i++) {
		if (_xwbMusicEntries[i].offset == 8192) {
			entry = i;
			break;
		}
	}

	if (entry == -1)
		return;

	Common::File *musicFile = new Common::File();

	if (!musicFile->open(Common::Path(_xwbMusicFilename))) {
		delete musicFile;
		return;
	}

	XWBEntry xwbEntry = _xwbMusicEntries[entry];
	Common::SeekableSubReadStream *stream = new Common::SeekableSubReadStream(
		musicFile,
		xwbEntry.offset,
		xwbEntry.offset + xwbEntry.length,
		DisposeAfterUse::YES
	);

	_mixer->playStream(
		Audio::Mixer::kMusicSoundType,
		&_musicHandle, createXWBStream(stream, xwbEntry)
	);
}
#endif

} // End of namespace Scumm
