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

#include "audio/midiparser.h"
#include "common/debug.h"
#include "common/quicktime.h"

class MidiParser_QT : /* public MidiParser, */ public Common::QuickTimeParser {
public:
	MidiParser_QT() {}
	~MidiParser_QT() {}

protected:
	SampleDesc *readSampleDesc(Track *track, uint32 format);

private:
	struct NoteRequestInfo {
		byte flags;
		byte reserved;
		uint16 polyphony;
		Common::Rational typicalPolyphony;
	};

	struct ToneDescription {
		uint32 synthesizerType;
		Common::String synthesizerName;
		Common::String instrumentName;
		uint32 instrumentNumber;
		uint32 gmNumber;
	};

	struct NoteRequest {
		uint16 part;
		NoteRequestInfo info;
		ToneDescription tone;
	};

	typedef Common::Array<NoteRequest> NoteRequestList;

	class MIDISampleDesc : public SampleDesc {
	public:
		MIDISampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag);
		~MIDISampleDesc() {}

		NoteRequestList _noteRequests;
	};

	Common::String readString31();
	Common::Rational readFixed();
	NoteRequestList readNoteRequestList();
};

Common::QuickTimeParser::SampleDesc *MidiParser_QT::readSampleDesc(Track *track, uint32 format) {
	if (track->codecType == CODEC_TYPE_MIDI) {
		debug(0, "MIDI Codec FourCC '%s'", tag2str(format));

		MIDISampleDesc *entry = new MIDISampleDesc(track, format);
		entry->_noteRequests = readNoteRequestList();
		return entry;
	}

	return 0;
}

MidiParser_QT::MIDISampleDesc::MIDISampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag) :
		Common::QuickTimeParser::SampleDesc(parentTrack, codecTag) {
}

Common::String MidiParser_QT::readString31() {
	byte size = _fd->readByte();
	assert(size < 32);

	Common::String string;
	for (byte i = 0; i < size; i++)
		string += (char)_fd->readByte();

	_fd->skip(31 - size);
	return string;
}

Common::Rational MidiParser_QT::readFixed() {
	int16 integerPart = _fd->readSint16BE();
	uint16 fractionalPart = _fd->readUint16BE();
	return integerPart + Common::Rational(fractionalPart, 0x10000);
}

MidiParser_QT::NoteRequestList MidiParser_QT::readNoteRequestList() {
	NoteRequestList requests;

	/* uint32 flags = */ _fd->readUint32BE(); // always 0

	for (;;) {
		uint32 event = _fd->readUint32BE();

		if (event == 0x60000000) // marker event
			break;
		else if ((event & 0xF000FFFF) != 0xF0000017) // note request event
			error("Invalid note request event");

		NoteRequest request;
		request.part = (event >> 16) & 0xFFF;
		request.info.flags = _fd->readByte();
		request.info.reserved = _fd->readByte();
		request.info.polyphony = _fd->readUint16BE();
		request.info.typicalPolyphony = readFixed();
		request.tone.synthesizerType = _fd->readUint32BE();
		request.tone.synthesizerName = readString31();
		request.tone.instrumentName = readString31();
		request.tone.instrumentNumber = _fd->readUint32BE();
		request.tone.gmNumber = _fd->readUint32BE();

		if (_fd->readUint32BE() != 0xC0010017) // general event note request
			error("Invalid instrument end event");

		requests.push_back(request);
	}

	return requests;
}
