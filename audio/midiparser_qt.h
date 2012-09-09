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

/**
 * The QuickTime MIDI version of MidiParser.
 */
class MidiParser_QT : public MidiParser, public Common::QuickTimeParser {
public:
	MidiParser_QT() {}
	~MidiParser_QT() {}

	// MidiParser
	bool loadMusic(byte *data, uint32 size);
	void unloadMusic();

	/**
	 * Load the MIDI from a 'Tune' resource
	 */
	bool loadFromTune(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

	/**
	 * Load the MIDI from a QuickTime stream
	 */
	bool loadFromContainerStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

	/**
	 * Load the MIDI from a QuickTime file
	 */
	bool loadFromContainerFile(const Common::String &fileName);

protected:
	// MidiParser
	void parseNextEvent(EventInfo &info);

	// QuickTimeParser
	SampleDesc *readSampleDesc(Track *track, uint32 format, uint32 descSize);

private:
	struct MIDITrackInfo {
		byte *data;
		uint32 size;
		uint32 timeScale;
	};

	class MIDISampleDesc : public SampleDesc {
	public:
		MIDISampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag);
		~MIDISampleDesc() {}

		byte *_requestData;
		uint32 _requestSize;
	};

	uint32 readNextEvent(EventInfo &info);
	void handleGeneralEvent(EventInfo &info, uint32 control);

	byte *readWholeTrack(Common::QuickTimeParser::Track *track, uint32 &trackSize);

	Common::Array<MIDITrackInfo> _trackInfo;

	void initFromContainerTracks();
	void initCommon();
	uint32 readUint32();
};

#endif
