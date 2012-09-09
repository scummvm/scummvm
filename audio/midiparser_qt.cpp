/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "audio/midiparser_qt.h"
#include "common/debug.h"
#include "common/memstream.h"

bool MidiParser_QT::loadMusic(byte *data, uint32 size) {
	if (size < 8)
		return false;

	Common::SeekableReadStream *stream = new Common::MemoryReadStream(data, size, DisposeAfterUse::NO);

	// Attempt to detect what format we have
	bool result;
	if (READ_BE_UINT32(data + 4) == MKTAG('m', 'u', 's', 'i'))
		result = loadFromTune(stream);
	else
		result = loadFromContainerStream(stream);

	if (!result) {
		delete stream;
		return false;
	}

	return true;
}

void MidiParser_QT::unloadMusic() {
	MidiParser::unloadMusic();
	close();

	// Unlike those lesser formats, we *do* hold track data
	for (uint i = 0; i < _trackInfo.size(); i++)
		free(_trackInfo[i].data);

	_trackInfo.clear();
}

bool MidiParser_QT::loadFromTune(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	unloadMusic();

	// a tune starts off with a sample description
	stream->readUint32BE(); // header size

	if (stream->readUint32BE() != MKTAG('m', 'u', 's', 'i'))
		return false;

	stream->readUint32BE(); // reserved
	stream->readUint16BE(); // reserved
	stream->readUint16BE(); // index

	stream->readUint32BE(); // flags, ignore

	MIDITrackInfo trackInfo;
	trackInfo.size = stream->size() - stream->pos();
	assert(trackInfo.size > 0);

	trackInfo.data = (byte *)malloc(trackInfo.size);
	stream->read(trackInfo.data, trackInfo.size);

	trackInfo.timeScale = 600; // the default
	_trackInfo.push_back(trackInfo);

	initCommon();
	return true;
}

bool MidiParser_QT::loadFromContainerStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	unloadMusic();

	if (!parseStream(stream, disposeAfterUse))
		return false;

	initFromContainerTracks();
	return true;
}

bool MidiParser_QT::loadFromContainerFile(const Common::String &fileName) {
	unloadMusic();

	if (!parseFile(fileName))
		return false;

	initFromContainerTracks();
	return true;
}

void MidiParser_QT::parseNextEvent(EventInfo &info) {
	info.event = 0;

	while (info.event == 0) {
		if (_position._playPos >= _trackInfo[_activeTrack].data + _trackInfo[_activeTrack].size) {
			// Manually insert end of track when we reach the end
			info.event = 0xFF;
			info.ext.type = 0x2F;
			return;
		}

		info.delta = readNextEvent(info);
	}
}

uint32 MidiParser_QT::readNextEvent(EventInfo &info) {
	uint32 control = readUint32();

	switch (control >> 28) {
	case 0x0:
	case 0x1:
		// Rest
		// We handle this by recursively adding up all the rests into the
		// next event's delta
		return readNextEvent(info) + (control & 0xFFFFFF);
	case 0x2:
	case 0x3:
		// Note event
		info.event = 0x90 | ((control >> 24) & 0x1F);
		info.basic.param1 = ((control >> 18) & 0x3F) + 32;
		info.basic.param2 = (control >> 11) & 0x7F;
		info.length = (info.basic.param2 == 0) ? 0 : (control & 0x7FF);
		break;
	case 0x4:
	case 0x5:
		// Controller
		if (((control >> 16) & 0xFF) == 32) {
			// Pitch bend
			info.event = 0xE0 | ((control >> 24) & 0x1F);

			// Actually an 8.8 fixed point number
			int16 value = (int16)(control & 0xFFFF);

			if (value < -0x200 || value > 0x1FF) {
				warning("QuickTime MIDI pitch bend value (%d) out of range, clipping", value);
				value = CLIP<int16>(value, -0x200, 0x1FF);
			}

			// Now convert the value to 'normal' MIDI values
			value += 0x200;
			value *= 16;

			// param1 holds the low 7 bits, param2 holds the high 7 bits
			info.basic.param1 = value & 0x7F;
			info.basic.param2 = value >> 7;
		} else {
			// Regular controller
			info.event = 0xB0 | ((control >> 24) & 0x1F);
			info.basic.param1 = (control >> 16) & 0xFF;
			info.basic.param2 = (control >> 8) & 0xFF;
		}
		break;
	case 0x6:
	case 0x7:
		// Marker
		// Used for editing only, so we don't need to care about this
		break;
	case 0x9: {
		// Extended note event
		uint32 extra = readUint32();
		info.event = 0x90 | ((control >> 16) & 0xFFF);
		info.basic.param1 = (control >> 8) & 0xFF;
		info.basic.param2 = (extra >> 22) & 0x7F;
		info.length = (info.basic.param2 == 0) ? 0 : (extra & 0x3FFFFF);
		break;
	}
	case 0xA: {
		// Extended controller
		uint32 extra = readUint32();
		info.event = 0xB0 | ((control >> 16) & 0xFFF);
		info.basic.param1 = (extra >> 16) & 0x3FFF;
		info.basic.param2 = (extra >> 8) & 0xFF; // ???
		break;
	}
	case 0xB:
		// Knob
		error("Encountered knob event in QuickTime MIDI");
		break;
	case 0x8:
	case 0xC:
	case 0xD:
	case 0xE:
		// Reserved
		readUint32();
		break;
	case 0xF:
		// General
		handleGeneralEvent(info, control);
		break;
	}

	return 0;
}

void MidiParser_QT::handleGeneralEvent(EventInfo &info, uint32 control) {
	uint32 part = (control >> 16) & 0xFFF;
	uint32 dataSize = ((control & 0xFFFF) - 2) * 4;
	byte subType = READ_BE_UINT16(_position._playPos + dataSize) & 0x3FFF;

	switch (subType) {
	case 1:
		// Note Request
		// Currently we're only using the GM number from the request
		assert(dataSize == 84);
		info.event = 0xC0 | part;
		info.basic.param1 = READ_BE_UINT32(_position._playPos + 80);
		break;
	case 5: // Tune Difference
	case 8: // MIDI Channel
	case 10: // No-op
	case 11: // Used Notes
		// Should be safe to skip these
		break;
	default:
		warning("Unhandled general event %d", subType);
	}

	_position._playPos += dataSize + 4;
}

Common::QuickTimeParser::SampleDesc *MidiParser_QT::readSampleDesc(Track *track, uint32 format, uint32 descSize) {
	if (track->codecType == CODEC_TYPE_MIDI) {
		debug(0, "MIDI Codec FourCC '%s'", tag2str(format));

		_fd->readUint32BE(); // flags, ignore
		descSize -= 4;

		MIDISampleDesc *entry = new MIDISampleDesc(track, format);
		entry->_requestSize = descSize;
		entry->_requestData = (byte *)malloc(descSize);
		_fd->read(entry->_requestData, descSize);
		return entry;
	}

	return 0;
}

MidiParser_QT::MIDISampleDesc::MIDISampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag) :
		Common::QuickTimeParser::SampleDesc(parentTrack, codecTag) {
}

void MidiParser_QT::initFromContainerTracks() {
	const Common::Array<Common::QuickTimeParser::Track *> &tracks = Common::QuickTimeParser::_tracks;

	for (uint32 i = 0; i < tracks.size(); i++) {
		if (tracks[i]->codecType == CODEC_TYPE_MIDI) {
			assert(tracks[i]->sampleDescs.size() == 1);

			if (tracks[i]->editCount != 1)
				warning("Unhandled QuickTime MIDI edit lists, things may go awry");

			MIDITrackInfo trackInfo;
			trackInfo.data = readWholeTrack(tracks[i], trackInfo.size);
			trackInfo.timeScale = tracks[i]->timeScale;
			_trackInfo.push_back(trackInfo);
		}
	}

	initCommon();
}

void MidiParser_QT::initCommon() {
	// Now we have all our info needed in _trackInfo from whatever container
	// form, we can fill in the MidiParser tracks.

	_numTracks = _trackInfo.size();
	assert(_numTracks > 0);

	for (uint32 i = 0; i < _trackInfo.size(); i++)
		MidiParser::_tracks[i] = _trackInfo[i].data;

	_ppqn = _trackInfo[0].timeScale;
	resetTracking();
	setTempo(1000000);
	setTrack(0);
}

byte *MidiParser_QT::readWholeTrack(Common::QuickTimeParser::Track *track, uint32 &trackSize) {
	// This just goes through all chunks and 

	Common::MemoryWriteStreamDynamic output;
	uint32 curSample = 0;

	// Read in the note request data first
	MIDISampleDesc *entry = (MIDISampleDesc *)track->sampleDescs[0];
	output.write(entry->_requestData, entry->_requestSize);

	for (uint i = 0; i < track->chunkCount; i++) {
		_fd->seek(track->chunkOffsets[i]);

		uint32 sampleCount = 0;

		for (uint32 j = 0; j < track->sampleToChunkCount; j++)
			if (i >= track->sampleToChunk[j].first)
				sampleCount = track->sampleToChunk[j].count;

		for (uint32 j = 0; j < sampleCount; j++, curSample++) {
			uint32 size = (track->sampleSize != 0) ? track->sampleSize : track->sampleSizes[curSample];

			byte *data = new byte[size];
			_fd->read(data, size);
			output.write(data, size);
			delete[] data;
		}
	}

	trackSize = output.size();
	return output.getData();
}

uint32 MidiParser_QT::readUint32() {
	uint32 value = READ_BE_UINT32(_position._playPos);
	_position._playPos += 4;
	return value;
}

MidiParser *MidiParser::createParser_QT() {
	return new MidiParser_QT();
}
