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
	// Assume that this is a Tune and not a QuickTime container
	Common::SeekableReadStream *stream = new Common::MemoryReadStream(data, size, DisposeAfterUse::NO);

	if (!loadFromTune(stream)) {
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

	MIDITrackInfo trackInfo;
	trackInfo.noteRequests = readNoteRequestList(stream);

	uint32 trackSize = stream->size() - stream->pos();
	assert(trackSize > 0);

	trackInfo.data = (byte *)malloc(trackSize);
	stream->read(trackInfo.data, trackSize);

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
	// TODO
}

Common::QuickTimeParser::SampleDesc *MidiParser_QT::readSampleDesc(Track *track, uint32 format) {
	if (track->codecType == CODEC_TYPE_MIDI) {
		debug(0, "MIDI Codec FourCC '%s'", tag2str(format));

		MIDISampleDesc *entry = new MIDISampleDesc(track, format);
		entry->_noteRequests = readNoteRequestList(_fd);
		return entry;
	}

	return 0;
}

MidiParser_QT::MIDISampleDesc::MIDISampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag) :
		Common::QuickTimeParser::SampleDesc(parentTrack, codecTag) {
}

Common::String MidiParser_QT::readString31(Common::SeekableReadStream *stream) {
	byte size = stream->readByte();
	assert(size < 32);

	Common::String string;
	for (byte i = 0; i < size; i++)
		string += (char)stream->readByte();

	stream->skip(31 - size);
	return string;
}

Common::Rational MidiParser_QT::readFixed(Common::SeekableReadStream *stream) {
	int16 integerPart = stream->readSint16BE();
	uint16 fractionalPart = stream->readUint16BE();
	return integerPart + Common::Rational(fractionalPart, 0x10000);
}

MidiParser_QT::NoteRequestList MidiParser_QT::readNoteRequestList(Common::SeekableReadStream *stream) {
	NoteRequestList requests;

	/* uint32 flags = */ stream->readUint32BE(); // always 0

	for (;;) {
		uint32 event = stream->readUint32BE();

		if (event == 0x60000000) // marker event
			break;
		else if ((event & 0xF000FFFF) != 0xF0000017) // note request event
			error("Invalid note request event");

		NoteRequest request;
		request.part = (event >> 16) & 0xFFF;
		request.info.flags = stream->readByte();
		request.info.reserved = stream->readByte();
		request.info.polyphony = stream->readUint16BE();
		request.info.typicalPolyphony = readFixed(stream);
		request.tone.synthesizerType = stream->readUint32BE();
		request.tone.synthesizerName = readString31(stream);
		request.tone.instrumentName = readString31(stream);
		request.tone.instrumentNumber = stream->readUint32BE();
		request.tone.gmNumber = stream->readUint32BE();

		if (stream->readUint32BE() != 0xC0010017) // general event note request
			error("Invalid instrument end event");

		requests.push_back(request);
	}

	return requests;
}

void MidiParser_QT::initFromContainerTracks() {
	const Common::Array<Common::QuickTimeParser::Track *> &tracks = Common::QuickTimeParser::_tracks;

	for (uint32 i = 0; i < tracks.size(); i++) {
		if (tracks[i]->codecType == CODEC_TYPE_MIDI) {
			assert(tracks[i]->sampleDescs.size() == 1);

			if (tracks[i]->editCount != 1)
				warning("Unhandled QuickTime MIDI edit lists, things may go awry");

			MIDISampleDesc *entry = (MIDISampleDesc *)tracks[i]->sampleDescs[0];

			MIDITrackInfo trackInfo;
			trackInfo.noteRequests = entry->_noteRequests;
			trackInfo.data = readWholeTrack(tracks[i]);
			_trackInfo.push_back(trackInfo);
		}
	}

	initCommon();
}

void MidiParser_QT::initCommon() {
	// Now we have all our info needed in _trackInfo from whatever container
	// form, we can fill in the MidiParser tracks.

	// TODO
}

byte *MidiParser_QT::readWholeTrack(Common::QuickTimeParser::Track *track) {
	// This just goes through all chunks and 

	Common::MemoryWriteStreamDynamic output;
	uint32 curSample = 0;

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

	return output.getData();
}

MidiParser *MidiParser::createParser_QT() {
	return new MidiParser_QT();
}
