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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AUDIO_MIDIPARSER_QT_H
#define AUDIO_MIDIPARSER_QT_H

#include "audio/midiparser.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/queue.h"
#include "common/quicktime.h"

/**
 * @defgroup audio_midiparser_qt QT MIDI parser
 * @ingroup audio
 *
 * @brief The QuickTime Music version of MidiParser class.
 * @{
 */

/**
 * The QuickTime Music version of MidiParser.
 *
 * QuickTime Music is actually a superset of MIDI. It has its own custom
 * instruments and supports more than 15 non-percussion channels. It also
 * has custom control changes and a more advanced pitch bend (which we
 * convert to GM pitch bend as best as possible). We then use the fallback
 * GM instrument that each QuickTime instrument definition has to provide.
 *
 * Furthermore, Apple's documentation on this is terrible. You know
 * documentation is bad when it contradicts itself three times on the same
 * subject (like about setting the GM instrument field to percussion).
 *
 * This is as close to a proper QuickTime Music parser as we can currently
 * implement using our MidiParser interface.
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
	void resetTracking();

	// QuickTimeParser
	SampleDesc *readSampleDesc(Track *track, uint32 format, uint32 descSize);

private:
	struct MIDITrackInfo {
		byte *data;
		uint32 size;
		uint32 timeScale;
	};

	struct PartStatus {
		uint32 instrument;
		byte volume;
		byte pan;
		uint16 pitchBend;
	};

	class MIDISampleDesc : public SampleDesc {
	public:
		MIDISampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag);
		~MIDISampleDesc() {}

		byte *_requestData;
		uint32 _requestSize;
	};

	uint32 readNextEvent();
	void handleGeneralEvent(uint32 control);
	void handleControllerEvent(uint32 control, uint32 part, byte intPart, byte fracPart);
	void handleNoteEvent(uint32 part, byte pitch, byte velocity, uint32 length);

	void definePart(uint32 part, uint32 instrument);
	void setupPart(uint32 part);

	byte getChannel(uint32 part);
	bool isChannelAllocated(byte channel) const;
	byte findFreeChannel(uint32 part);
	void deallocateFreeChannel();
	void deallocateChannel(byte channel);
	bool allChannelsAllocated() const;

	byte *readWholeTrack(Common::QuickTimeParser::Track *track, uint32 &trackSize);

	Common::Array<MIDITrackInfo> _trackInfo;
	Common::Queue<EventInfo> _queuedEvents;

	typedef Common::HashMap<uint, PartStatus> PartMap;
	PartMap _partMap;

	typedef Common::HashMap<uint, byte> ChannelMap;
	ChannelMap _channelMap;

	void initFromContainerTracks();
	void initCommon();
	uint32 readUint32();
};
/** @} */
#endif
