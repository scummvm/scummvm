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

#ifndef AUDIO_MIDIPARSER_QT_H
#define AUDIO_MIDIPARSER_QT_H

#include "audio/midiparser.h"
#include "common/array.h"
#include "common/quicktime.h"

class MidiParser_QT : public MidiParser, public Common::QuickTimeParser {
public:
	MidiParser_QT() {}
	~MidiParser_QT() {}

	// MidiParser
	bool loadMusic(byte *data, uint32 size);
	void unloadMusic();

	// Custom
	bool loadFromTune(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	bool loadFromContainerStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	bool loadFromContainerFile(const Common::String &fileName);

protected:
	// MidiParser
	void parseNextEvent(EventInfo &info);

	// QuickTimeParser
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

	struct MIDITrackInfo {
		NoteRequestList noteRequests;
		byte *data;
		uint32 timeScale;
	};

	class MIDISampleDesc : public SampleDesc {
	public:
		MIDISampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag);
		~MIDISampleDesc() {}

		NoteRequestList _noteRequests;
	};

	Common::String readString31(Common::SeekableReadStream *stream);
	Common::Rational readFixed(Common::SeekableReadStream *stream);
	NoteRequestList readNoteRequestList(Common::SeekableReadStream *stream);

	byte *readWholeTrack(Common::QuickTimeParser::Track *track);

	Common::Array<MIDITrackInfo> _trackInfo;

	void initFromContainerTracks();
	void initCommon();
};

#endif
