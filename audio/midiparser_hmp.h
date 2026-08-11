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

#ifndef AUDIO_MIDIPARSER_HMP_H
#define AUDIO_MIDIPARSER_HMP_H

#include "audio/midiparser_smf.h"

/**
 * MIDI parser for the HMI SOS formats HMI and HMP.
 * 
 * Implementation is incomplete. Currently only the track chunks are read and
 * played back. Not implemented yet: device track mapping, branching, callbacks, etc.
 */
class MidiParser_HMP : public MidiParser_SMF {
protected:
	enum class HmpVersion {
		VERSION_HMI,
		VERSION_HMP_1,
		VERSION_HMP_013195
	};

	static const char HEADER_HMI[];
	static const char HEADER_HMP[];
	static const char HEADER_HMP_VERSION_1[];
	static const char HEADER_HMP_VERSION_013195[];

	uint32 readDelta(const byte *&data) override;

	HmpVersion determineVersion(const byte *pos);

public:
	MidiParser_HMP(int8 source = -1);

	bool loadMusic(const byte *data, uint32 size) override;

	int32 determineDataSize(Common::SeekableReadStream *stream) override;

protected:
	HmpVersion _version;
	uint32 _branchOffset;
	// Total track length in seconds
	uint32 _songLength;
	uint32 _channelPriorities[16];
	uint32 _deviceTrackMappings[32][5];
	uint8 _restoreControllers[128];
	uint32 _callbackPointer;
	uint32 _callbackSegment;

	// Read a word from the data with a length that is correct for the specified
	// version (i.e. 2 bytes for HMI, 4 bytes for HMP)
	uint32 readWord(const byte *&data, HmpVersion version);
	uint32 readWord(Common::SeekableReadStream *stream, HmpVersion version);
};

#endif
