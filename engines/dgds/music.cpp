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

#include "common/debug.h"
#include "common/config-manager.h"
#include "audio/midiparser.h"

#include "dgds/sound.h"
#include "dgds/music.h"

namespace Dgds {

/**
 * The Standard MIDI File version of MidiParser.
 */
class MidiParser_DGDS : public MidiParser {
protected:
	byte *_init;
	const byte *_last;

	byte _numParts;
	const byte *_trackPtr[0xFF];
	uint16 _trackSz[0xFF];

protected:
	void parseNextEvent(EventInfo &info) override;

public:
	MidiParser_DGDS();
	~MidiParser_DGDS();

	void sendInitCommands() {};

	bool loadMusic(byte *data, uint32 size) override;

	bool validateNextRead(uint i, const uint16 *trackPos) const;
	byte midiGetNextChannel(const uint16 *trackPos, const uint32 *trackTimer, long ticker) const;
	void mixChannels();
};

MidiParser_DGDS::MidiParser_DGDS() : _init(nullptr), _last(nullptr), _numParts(0) {
	memset(_trackSz, 0, sizeof(_trackSz));
	memset(_trackPtr, 0, sizeof(_trackPtr));
}

MidiParser_DGDS::~MidiParser_DGDS() {
	free(_init);
}

void MidiParser_DGDS::parseNextEvent(EventInfo &info) {
	if (_position._playPos >= _last) {
		// fake an end-of-track meta event
		info.delta = 0;
		info.event = 0xFF;
		info.ext.type = 0x2F;
		info.length = 0;
		return;
	}

	info.start = _position._playPos;
	info.delta = 0;
	while (*_position._playPos == 0xF8) {
		info.delta += 240;
		_position._playPos++;
	}
	info.delta += *_position._playPos++;

	// Process the next info.
	if ((_position._playPos[0] & 0xF0) >= 0x80)
		info.event = *_position._playPos++;
	else
		info.event = _position._runningStatus;
	if (info.event < 0x80)
		return;

	_position._runningStatus = info.event;
	switch (info.command()) {
	case 0x9:
		info.basic.param1 = *_position._playPos++;
		info.basic.param2 = *_position._playPos++;
		if (info.basic.param2 == 0) {
			// NoteOn with param2==0 is a NoteOff
			info.event = info.channel() | 0x80;
		}
		info.length = 0;
		break;

	case 0xC:
	case 0xD:
		info.basic.param1 = *_position._playPos++;
		info.basic.param2 = 0;
		break;

	case 0x8:
	case 0xA:
	case 0xB:
	case 0xE:
		info.basic.param1 = *_position._playPos++;
		info.basic.param2 = *_position._playPos++;
		info.length = 0;
		break;

	case 0xF: // System Common, Meta or SysEx event
		switch (info.event & 0x0F) {
		case 0x2: // Song Position Pointer
			info.basic.param1 = *_position._playPos++;
			info.basic.param2 = *_position._playPos++;
			break;

		case 0x3: // Song Select
			info.basic.param1 = *_position._playPos++;
			info.basic.param2 = 0;
			break;

		case 0x6:
		case 0x8:
		case 0xA:
		case 0xB:
		case 0xC:
		case 0xE:
			info.basic.param1 = info.basic.param2 = 0;
			break;

		case 0x0: // SysEx
			info.length = readVLQ(_position._playPos);
			info.ext.data = _position._playPos;
			_position._playPos += info.length;
			break;

		case 0xF: // META event
			info.ext.type = *_position._playPos++;
			info.length = readVLQ(_position._playPos);
			info.ext.data = _position._playPos;
			_position._playPos += info.length;
			break;

		default:
			warning("Unexpected midi event 0x%02X in midi data", info.event);
		}
	}
}

byte MidiParser_DGDS::midiGetNextChannel(const uint16 *trackPos, const uint32 *trackTimer, long ticker) const {
	byte curr = 0xFF;
	uint32 closest = ticker + 1000000, next = 0;

	for (byte i = 0; i < _numParts; i++) {
		if (trackTimer[i] == 0xFFFFFFFF) // channel ended
			continue;
		if (trackPos[i] >= _trackSz[i])
			continue;
		next = _trackPtr[i][trackPos[i]]; // when the next event should occur
		if (next == 0xF8) // 0xF8 means 240 ticks delay
			next = 240;
		next += trackTimer[i];
		if (next < closest) {
			curr = i;
			closest = next;
		}
	}

	return curr;
}

bool MidiParser_DGDS::validateNextRead(uint i, const uint16 *trackPos) const {
	if (_trackSz[i] <= trackPos[i]) {
		warning("Unexpected end. Music may sound wrong due to game resource corruption");
		return false;
	} else {
		return true;
	}
}

static const byte CMD_LENGTHS[] = { 2, 2, 2, 2, 1, 1, 2, 0 };

void MidiParser_DGDS::mixChannels() {
	int totalSize = 0;

	uint16 trackPos[0xFF];
	uint32 trackTimer[0xFF];
	byte _prev[0xFF];
	for (byte i = 0; i < _numParts; i++) {
		trackTimer[i] = 0;
		_prev[i] = 0;
		trackPos[i] = 0;
		totalSize += _trackSz[i];
	}

	byte *output = (byte *)malloc(totalSize * 2);
	_tracks[0] = output;

	uint32 ticker = 0;
	byte channel, curDelta;
	byte midiCommand = 0, midiParam, globalPrev = 0;
	uint32 newDelta;

	while ((channel = midiGetNextChannel(trackPos, trackTimer, ticker)) != 0xFF) { // there is still an active channel
		if (!validateNextRead(channel, trackPos))
			goto end;
		curDelta = _trackPtr[channel][trackPos[channel]++];
		trackTimer[channel] += (curDelta == 0xF8 ? 240 : curDelta); // when the command is supposed to occur
		if (curDelta == 0xF8)
			continue;
		newDelta = trackTimer[channel] - ticker;
		ticker += newDelta;

		if (!validateNextRead(channel, trackPos))
			goto end;
		midiCommand = _trackPtr[channel][trackPos[channel]++];
		if (midiCommand != 0xFC) {
			// Write delta
			while (newDelta > 240) {
				*output++ = 0xF8;
				newDelta -= 240;
			}
			*output++ = (byte)newDelta;
		}
		// Write command
		switch (midiCommand) {
		case 0xF0: // sysEx
			*output++ = midiCommand;
			do {
				if (!validateNextRead(channel, trackPos))
					goto end;
				midiParam = _trackPtr[channel][trackPos[channel]++];
				*output++ = midiParam;
			} while (midiParam != 0xF7);
			break;
		case 0xFC: // end of channel
			trackTimer[channel] = 0xFFFFFFFF;
			break;
		default: // MIDI command
			if (midiCommand & 0x80) {
				if (!validateNextRead(channel, trackPos))
					goto end;
				midiParam = _trackPtr[channel][trackPos[channel]++];
			} else {// running status
				midiParam = midiCommand;
				midiCommand = _prev[channel];
			}

			// remember which channel got used for channel remapping
			//byte midiChannel = midiCommand & 0xF;
			//_channelUsed[midiChannel] = true;

			if (midiCommand != globalPrev)
				*output++ = midiCommand;
			*output++ = midiParam;
			if (CMD_LENGTHS[(midiCommand >> 4) - 8] == 2) {
				if (!validateNextRead(channel, trackPos))
					goto end;
				*output++ = _trackPtr[channel][trackPos[channel]++];
			}
			_prev[channel] = midiCommand;
			globalPrev = midiCommand;
			break;
		}
	}

end:
	_last = output;
}

bool MidiParser_DGDS::loadMusic(byte *data, uint32 size) {
	unloadMusic();

	if (!data)
		return false;

	_numParts = loadSndTrack(TRACK_MT32, _trackPtr, _trackSz, data, size);
	if (_numParts == 0)
		return false;

	for (byte part = 0; part < _numParts; part++) {
		const byte *ptr = _trackPtr[part];

		byte number, voices;
		number = (*ptr++);
		voices = (*ptr++) & 0x0F;
		debug(" - #%u: voices: %u", number, voices);

		_trackPtr[part] += 2;
		_trackSz[part] -= 2;
	}

	mixChannels();

	_init = _tracks[0];
	_numTracks = 1;

	_ppqn = 1;
	setTempo(16667);

	// Note that we assume the original data passed in
	// will persist beyond this call, i.e. we do NOT
	// copy the data to our own buffer. Take warning....
	resetTracking();
	setTrack(0);

	return true;
}


DgdsMidiPlayer::DgdsMidiPlayer(bool isSfx) : _isSfx(isSfx), Audio::MidiPlayer() {
	MidiPlayer::createDriver();

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();
		_driver->setTimerCallback(this, &timerCallback);
	}
}

void DgdsMidiPlayer::syncVolumeForChannel() {
	int volume = ConfMan.getInt(_isSfx ? "sfx_volume": "music_volume");
	if (ConfMan.getBool("mute")) {
		volume = -1;
	}
	setVolume(volume);
}

void DgdsMidiPlayer::play(byte *data, uint32 size) {
	Common::StackLock lock(_mutex);

	stop();
	if (!data) return;

	MidiParser_DGDS *parser = new MidiParser_DGDS();
	if (parser->loadMusic(data, size)) {
		parser->setMidiDriver(this);
		parser->sendInitCommands();
		parser->setTimerRate(_driver->getBaseTempo());
		/*
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
		parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);
		*/
		_parser = parser;
		syncVolumeForChannel();

		_isLooping = false;
		_isPlaying = true;
		debug("Playing music track sz %d", size);
	} else {
		debug("Cannot play music track");
		delete parser;
	}
}

void DgdsMidiPlayer::stop() {
	_driver->stopAllNotes();
	_isPlaying = false;
	_isLooping = false;
	Audio::MidiPlayer::stop();
	debug("Stopping track");
}

} // End of namespace Dgds

