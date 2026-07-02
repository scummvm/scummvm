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

#include "audio/midiparser_hmp.h"

#include "common/algorithm.h"
#include "common/textconsole.h"

const char MidiParser_HMP::HEADER_HMI[] = "HMIMIDIR";
const char MidiParser_HMP::HEADER_HMP[] = "HMIMIDIP";
const char MidiParser_HMP::HEADER_HMP_VERSION_1[] = "\x00\x00\x00\x00\x00\x00";
const char MidiParser_HMP::HEADER_HMP_VERSION_013195[] = "013195";

MidiParser_HMP::MidiParser_HMP(int8 source) : MidiParser_SMF(source) {
	_version = HmpVersion::VERSION_HMP_1;
	_branchOffset = 0;
	_songLength = 0;
	memset(_channelPriorities, 0, sizeof(_channelPriorities));
	memset(_deviceTrackMappings, 0, sizeof(_deviceTrackMappings));
	memset(_restoreControllers, 0, sizeof(_restoreControllers));
	_callbackPointer = 0;
	_callbackSegment = 0;
}

uint32 MidiParser_HMP::readDelta(const byte*& data) {
	byte str;
	uint32 value = 0;

	for (int i = 0; i < 4; ++i) {
		str = *data++;
		value |= ((str & 0x7F) << (7 * i));
		if (str & 0x80)
			break;
	}
	return value;
}

bool MidiParser_HMP::loadMusic(const byte *data, uint32 size) {
	unloadMusic();
	const byte *pos = data;

	// Process header
	if (memcmp(pos, HEADER_HMI, 7)) {
		warning("Could not find HMIMIDI header in HMI/HMP data");
		return false;
	}
	_version = determineVersion(pos);
	bool isHmi = _version == HmpVersion::VERSION_HMI;

	// Skip signature, version, length and padding bytes
	pos += isHmi ? 26 : 32;

	if (isHmi) {
		_branchOffset = 0;
	} else {
		_branchOffset = readWord(pos, _version);
		// Skip 3 reserved dwords
		pos += 12;
	}

	_numTracks = 1;
	_numSubtracks[0] = readWord(pos, _version);
	// Doesn't seem like this field is actually used...
	//uint32 ppqn = readWord(pos, _version);
	_ppqn = 60;
	pos += isHmi ? 2 : 4;
	uint32 bpm = readWord(pos, _version);
	setTempo(60000000 / bpm);
	_songLength = readWord(pos, _version);

	for (int i = 0; i < 16; i++) {
		_channelPriorities[i] = readWord(pos, _version);
	}
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 32; j++) {
			_deviceTrackMappings[j][i] = readWord(pos, _version);
		}
	}
	if (_version == HmpVersion::VERSION_HMP_013195) {
		Common::copy(pos, pos + 128, _restoreControllers);
		pos += 128;
	}

	_callbackPointer = readWord(pos, _version);
	_callbackSegment = readWord(pos, _version);

	// Read the tracks
	for (uint currTrack = 0; currTrack < _numSubtracks[0]; currTrack++) {
		//uint32 chunkNumber = readWord(pos, _version);
		pos += isHmi ? 2 : 4;
		uint32 chunkSize = readWord(pos, _version);
		//uint32 trackNumber = readWord(pos, _version);
		pos += isHmi ? 2 : 4;

		_tracks[0][currTrack] = pos;
		pos += (chunkSize - (isHmi ? 6 : 12));
	}

	// TODO Read branching data

	return true;
}

int32 MidiParser_HMP::determineDataSize(Common::SeekableReadStream *stream) {
	int64 startPos = stream->pos();

	// Process header
	byte signatureBytes[14] = {0};
	stream->readMultipleLE(*signatureBytes);
	if (memcmp(signatureBytes, HEADER_HMI, 7)) {
		return -1;
	}
	HmpVersion version = determineVersion(signatureBytes);
	bool isHmi = version == HmpVersion::VERSION_HMI;
	if (isHmi) {
		stream->skip(12);
	} else {
		stream->skip(18);
		// TODO Figure out size of branching data
		//uint32 branchOffset = stream->readUint32LE();
		stream->skip(4);
		stream->skip(12);
	}

	uint32 numTracks = readWord(stream, version);
	// Skip over the rest of the header
	switch (version) {
		case HmpVersion::VERSION_HMI:
			stream->skip(362);
			break;
		case HmpVersion::VERSION_HMP_1:
			stream->skip(724);
			break;
		case HmpVersion::VERSION_HMP_013195:
			stream->skip(852);
			break;
	}

	// Read tracks
	for (uint currTrack = 0; currTrack < numTracks; currTrack++) {
		stream->skip(isHmi ? 2 : 4);
		uint32 chunkSize = readWord(stream, version);
		stream->skip(chunkSize - (isHmi ? 4 : 8));
	}

	// stream should now be at the end of the HMP data
	// (other than branching data, which might be at the end)
	return stream->pos() - startPos;
}

MidiParser_HMP::HmpVersion MidiParser_HMP::determineVersion(const byte *pos) {
	if (!memcmp(pos, HEADER_HMI, 8)) {
		return HmpVersion::VERSION_HMI;
	} else if (memcmp(pos, HEADER_HMP, 8)) {
		warning("Unknown HMI/HMP signature '%c%c%c%c%c%c%c%c' - assuming HMP version 1", pos[0], pos[1], pos[2], pos[3], pos[4], pos[5], pos[6], pos[7]);
		return HmpVersion::VERSION_HMP_1;
	}
	pos += 8;

	if (!memcmp(pos, HEADER_HMP_VERSION_1, 6)) {
		return HmpVersion::VERSION_HMP_1;
	} else if (!memcmp(pos, HEADER_HMP_VERSION_013195, 6)) {
		return HmpVersion::VERSION_HMP_013195;
	} else {
		warning("Unknown HMP version '%c%c%c%c%c%c' - assuming version 1", pos[0], pos[1], pos[2], pos[3], pos[4], pos[5]);
		return HmpVersion::VERSION_HMP_1;
	}
}

uint32 MidiParser_HMP::readWord(const byte*& data, MidiParser_HMP::HmpVersion version) {
	uint32 result;
	if (version == HmpVersion::VERSION_HMI) {
		result = READ_LE_UINT16(data);
		data += 2;
	} else {
		result = READ_LE_UINT32(data);
		data += 4;
	}
	return result;
}

uint32 MidiParser_HMP::readWord(Common::SeekableReadStream *stream, MidiParser_HMP::HmpVersion version) {
	if (version == HmpVersion::VERSION_HMI)
		return stream->readUint16LE();
	return stream->readUint32LE();
}

MidiParser *MidiParser::createParser_HMP(int8 source) { return new MidiParser_HMP(source); }
