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

const char MidiParser_HMP::HMP_HEADER[] = "HMIMIDIP";
const char MidiParser_HMP::HMP_HEADER_VERSION_1[] = "\x00\x00\x00\x00\x00\x00";
const char MidiParser_HMP::HMP_HEADER_VERSION_013195[] = "013195";

MidiParser_HMP::MidiParser_HMP(int8 source) : MidiParser_SMF(source) {
	_version = HmpVersion::VERSION_1;
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
	if (memcmp(pos, HMP_HEADER, 8)) {
		warning("Could not find HMIMIDIP header in HMP data");
		return false;
	}
	pos += 8;
	_version = determineVersion(pos);
	// Skip version and padding bytes
	pos += 24;

	_branchOffset = READ_LE_UINT32(pos);
	// Skip 3 reserved dwords
	pos += 16;
	_numTracks = 1;
	_numSubtracks[0] = READ_LE_UINT32(pos);
	pos += 4;
	// Doesn't seem like this field is actually used...
	//uint32 ppqn = READ_LE_UINT32(pos);
	_ppqn = 60;
	pos += 4;
	uint32 bpm = READ_LE_UINT32(pos);
	setTempo(60000000 / bpm);
	pos += 4;
	_songLength = READ_LE_UINT32(pos);
	pos += 4;

	for (int i = 0; i < 16; i++) {
		_channelPriorities[i] = READ_LE_UINT32(pos);
		pos += 4;
	}
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 32; j++) {
			_deviceTrackMappings[j][i] = READ_LE_UINT32(pos);
			pos += 4;
		}
	}
	if (_version == HmpVersion::VERSION_013195) {
		Common::copy(pos, pos + 128, _restoreControllers);
		pos += 128;
	}

	_callbackPointer = READ_LE_UINT32(pos);
	pos += 4;
	_callbackSegment = READ_LE_UINT32(pos);
	pos += 4;

	// Read the tracks
	for (uint currTrack = 0; currTrack < _numSubtracks[0]; currTrack++) {
		//uint32 chunkNumber = READ_LE_UINT32(pos);
		pos += 4;
		uint32 chunkSize = READ_LE_UINT32(pos);
		pos += 4;
		//uint32 trackNumber = READ_LE_UINT32(pos);
		pos += 4;

		_tracks[0][currTrack] = pos;
		pos += chunkSize - 12;
	}

	// TODO Read branching data

	return true;
}

int32 MidiParser_HMP::determineDataSize(Common::SeekableReadStream *stream) {
	int64 startPos = stream->pos();

	// Process header
	if (strcmp(stream->readString('\x00', 8).c_str(), HMP_HEADER)) {
		return -1;
	}
	byte versionBytes[6] = {0};
	stream->readMultipleLE(*versionBytes);
	HmpVersion version = determineVersion(versionBytes);
	stream->skip(18);

	// TODO Figure out size of branching data
	//uint32 branchOffset = stream->readUint32LE();
	stream->skip(4);
	stream->skip(12);

	uint32 numTracks = stream->readUint32LE();
	stream->skip(version == HmpVersion::VERSION_013195 ? 852 : 724);

	// Read tracks
	for (uint currTrack = 0; currTrack < numTracks; currTrack++) {
		stream->skip(4);
		uint32 chunkSize = stream->readUint32LE();
		stream->skip(chunkSize - 8);
	}

	// stream should now be at the end of the HMP data
	// (other than branching data, which might be at the end)
	return stream->pos() - startPos;
}

MidiParser_HMP::HmpVersion MidiParser_HMP::determineVersion(const byte *pos) {
	if (!memcmp(pos, HMP_HEADER_VERSION_1, 6)) {
		return HmpVersion::VERSION_1;
	} else if (!memcmp(pos, HMP_HEADER_VERSION_013195, 6)) {
		return HmpVersion::VERSION_013195;
	} else {
		warning("Unknown HMP version '%c%c%c%c%c%c' - assuming version 1", pos[0], pos[1], pos[2], pos[3], pos[4], pos[5]);
		return HmpVersion::VERSION_1;
	}
}

MidiParser *MidiParser::createParser_HMP(int8 source) { return new MidiParser_HMP(source); }
