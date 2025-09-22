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

#include "audio/midiparser.h"
#include "audio/mididrv.h"
#include "common/textconsole.h"
#include "common/util.h"

//////////////////////////////////////////////////
//
// MidiParser implementation
//
//////////////////////////////////////////////////

MidiParser::MidiParser(int8 source) :
_source(source),
_hangingNotesCount(0),
_driver(nullptr),
_timerRate(0x4A0000),
_ppqn(96),
_tempo(500000),
_psecPerTick(5208), // 500000 / 96
_sysExDelay(0),
_autoLoop(false),
_smartJump(false),
_centerPitchWheelOnUnload(false),
_sendSustainOffOnNotesOff(false),
_disableAllNotesOffMidiEvents(false),
_disableAutoStartPlayback(false),
_loopStartPoint(0xFFFFFFFF),
_loopEndPoint(0xFFFFFFFF),
_loopStartPointMs(0xFFFFFFFF),
_loopEndPointMs(0xFFFFFFFF),
_numTracks(0),
_activeTrack(255),
_abortParse(false),
_jumpingToTick(false),
_doParse(true),
_pause(false) {
	memset(_activeNotes, 0, sizeof(_activeNotes));
	memset(_tracks, 0, sizeof(_tracks));
	memset(_numSubtracks, 1, sizeof(_numSubtracks));
	for (int i = 0; i < MAXIMUM_SUBTRACKS; i++) {
		_nextSubtrackEvents[i].clear();
		_nextSubtrackEvents[i].subtrack = i;
	}
	_nextEvent = &_nextSubtrackEvents[0];
}

void MidiParser::property(int prop, int value) {
	switch (prop) {
	case mpAutoLoop:
		_autoLoop = (value != 0);
		break;
	case mpSmartJump:
		_smartJump = (value != 0);
		break;
	case mpCenterPitchWheelOnUnload:
		_centerPitchWheelOnUnload = (value != 0);
		break;
	case mpSendSustainOffOnNotesOff:
		_sendSustainOffOnNotesOff = (value != 0);
		break;
	case mpDisableAllNotesOffMidiEvents:
		_disableAllNotesOffMidiEvents = (value != 0);
		break;
	case mpDisableAutoStartPlayback:
		_disableAutoStartPlayback = (value != 0);
		break;
	default:
		break;
	}
}

void MidiParser::sendToDriver(uint32 b) {
	if (_source < 0) {
		_driver->send(b);
	} else {
		_driver->send(_source, b);
	}
}

void MidiParser::sendMetaEventToDriver(byte type, const byte *data, uint16 length) {
	if (_source < 0) {
		_driver->metaEvent(type, data, length);
	} else {
		_driver->metaEvent(_source, type, data, length);
	}
}

void MidiParser::setTempo(uint32 tempo) {
	_tempo = tempo;
	if (_ppqn)
		_psecPerTick = (tempo + (_ppqn >> 2)) / _ppqn;
}

// This is the conventional (i.e. SMF) variable length quantity
uint32 MidiParser::readVLQ(const byte * &data) {
	byte str;
	uint32 value = 0;
	int i;

	for (i = 0; i < 4; ++i) {
		str = data[0];
		++data;
		value = (value << 7) | (str & 0x7F);
		if (!(str & 0x80))
			break;
	}
	return value;
}

void MidiParser::activeNote(byte channel, byte note, bool active) {
	if (note >= 128 || channel >= 16)
		return;

	if (active)
		_activeNotes[note] |= (1 << channel);
	else
		_activeNotes[note] &= ~(1 << channel);

	// See if there are hanging notes that we can cancel
	NoteTimer *ptr = _hangingNotes;
	int i;
	for (i = ARRAYSIZE(_hangingNotes); i; --i, ++ptr) {
		if (ptr->channel == channel && ptr->note == note && ptr->timeLeft) {
			ptr->timeLeft = 0;
			--_hangingNotesCount;
			break;
		}
	}
}

void MidiParser::hangingNote(byte channel, byte note, uint32 timeLeft, bool recycle) {
	NoteTimer *best = nullptr;
	NoteTimer *ptr = _hangingNotes;
	int i;

	if (_hangingNotesCount >= ARRAYSIZE(_hangingNotes)) {
		warning("MidiParser::hangingNote(): Exceeded polyphony");
		return;
	}

	for (i = ARRAYSIZE(_hangingNotes); i; --i, ++ptr) {
		if (ptr->channel == channel && ptr->note == note) {
			if (ptr->timeLeft && ptr->timeLeft < timeLeft && recycle)
				return;
			best = ptr;
			if (ptr->timeLeft) {
				if (recycle)
					sendToDriver(0x80 | channel, note, 0);
				--_hangingNotesCount;
			}
			break;
		} else if (!best && ptr->timeLeft == 0) {
			best = ptr;
		}
	}

	// Occasionally we might get a zero or negative note
	// length, if the note should be turned on and off in
	// the same iteration. For now just set it to 1 and
	// we'll turn it off in the next cycle.
	if (!timeLeft || timeLeft & 0x80000000)
		timeLeft = 1;

	if (best) {
		best->channel = channel;
		best->note = note;
		best->timeLeft = timeLeft;
		++_hangingNotesCount;
	} else {
		// We checked this up top. We should never get here!
		warning("MidiParser::hangingNote(): Internal error");
	}
}

void MidiParser::onTimer() {
	uint32 endTime;
	uint32 eventTime;
	uint32 eventTick;

	// The SysEx delay can be decreased whenever time passes,
	// even if the parser does not parse events.
	_sysExDelay -= (_sysExDelay > _timerRate) ? _timerRate : _sysExDelay;

	if (!_position.isTracking() || !_driver || !_doParse || _pause || !_driver->isReady(_source))
		return;

	_abortParse = false;
	endTime = _position._playTime + _timerRate;

	// Scan our hanging notes for any
	// that should be turned off.
	if (_hangingNotesCount) {
		NoteTimer *ptr = &_hangingNotes[0];
		int i;
		for (i = ARRAYSIZE(_hangingNotes); i; --i, ++ptr) {
			if (ptr->timeLeft) {
				if (ptr->timeLeft <= _timerRate) {
					sendToDriver(0x80 | ptr->channel, ptr->note, 0);
					ptr->timeLeft = 0;
					--_hangingNotesCount;
				} else {
					ptr->timeLeft -= _timerRate;
				}
			}
		}
	}

	bool loopEvent = false;
	while (!_abortParse) {
		EventInfo &info = *_nextEvent;
		uint8 subtrack = info.subtrack;

		eventTick = _position._subtracks[subtrack]._lastEventTick + info.delta;
		eventTime = _position._lastEventTime + (eventTick - _position._lastEventTick) * _psecPerTick;

		if (_loopStartPoint == 0xFFFFFFFF && _loopStartPointMs != 0xFFFFFFFF && eventTime >= _loopStartPointMs) {
			_loopStartPoint = eventTick;
		}

		if (_loopStartPoint != 0xFFFFFFFF && _loopEndPoint != 0) {
			uint32 endTick = (endTime - _position._lastEventTime) / _psecPerTick + _position._lastEventTick;

			if ((_loopEndPoint != 0xFFFFFFFF && eventTick > _loopEndPoint && endTick > _loopEndPoint) ||
					(eventTime > _loopEndPointMs && endTime > _loopEndPointMs)) {
				// Loop
				jumpToTick(_loopStartPoint);
				_abortParse = true;
				break;
			}
		}

		if (eventTime > endTime)
			break;

		if (!info.noop) {
			// Process the next info.
			if (info.event < 0x80) {
				warning("Bad command or running status %02X", info.event);
				_position.stopTracking();
				return;
			}

			if (info.command() == 0x8) {
				activeNote(info.channel(), info.basic.param1, false);
			} else if (info.command() == 0x9) {
				if (info.length > 0)
					hangingNote(info.channel(), info.basic.param1, info.length * _psecPerTick - (endTime - eventTime));
				else
					activeNote(info.channel(), info.basic.param1, true);
			}

			// Player::metaEvent() in SCUMM will delete the parser object,
			// so return immediately if that might have happened.
			bool ret = processEvent(info);
			if (!ret)
				return;
		}

		loopEvent |= info.loop;

		if (!_abortParse) {
			_position._playTime = eventTime;
			_position._lastEventTime = eventTime;
			_position._subtracks[subtrack]._lastEventTime = eventTime;

			_position._playTick = eventTick;
			_position._lastEventTick = eventTick;
			_position._subtracks[subtrack]._lastEventTick = eventTick;

			if (_position.isTracking(subtrack)) {
				parseNextEvent(_nextSubtrackEvents[subtrack]);
			}
			determineNextEvent();
		}
	}

	if (!_abortParse) {
		_position._playTime = endTime;
		_position._playTick = (endTime - _position._lastEventTime) / _psecPerTick + _position._lastEventTick;
		if (loopEvent) {
			// One of the processed events has looped (part of) the MIDI data.
			// Infinite looping will cause the tracker playtime to overflow
			// eventually. Reset the tracker time and tick values to prevent
			// this from happening.
			rebaseTracking();
		}
	}
}

void MidiParser::rebaseTracking() {
	uint32 earliestLastEventTick = 0xFFFFFFFF;
	int earliestLastEventTickSubtrack = -1;
	for (int i = 0; i < _numSubtracks[_activeTrack]; i++) {
		if (_position.isTracking(i) && _position._subtracks[i]._lastEventTick < earliestLastEventTick) {
			earliestLastEventTick = _position._subtracks[i]._lastEventTick;
			earliestLastEventTickSubtrack = i;
		}
	}
	if (earliestLastEventTickSubtrack == -1)
		// Shouldn't happen
		return;
	uint32 earliestLastEventTime = _position._subtracks[earliestLastEventTickSubtrack]._lastEventTime;

	// Subtract the same value from all time and tick values to keep
	// a common timebase.
	for (int i = 0; i < _numSubtracks[_activeTrack]; i++) {
		if (_position.isTracking(i)) {
			if (_position._subtracks[i]._lastEventTime >= earliestLastEventTime) {
				_position._subtracks[i]._lastEventTime -= earliestLastEventTime;
			} else {
				// This shouldn't happen; maybe due to rounding?
				// Just to be sure there is no underflow...
				_position._subtracks[i]._lastEventTime = 0;
			}
			_position._subtracks[i]._lastEventTick -= earliestLastEventTick;
		}
	}
	if (_position._playTime >= earliestLastEventTime) {
		_position._playTime -= earliestLastEventTime;
	} else {
		_position._playTime = 0;
	}
	if (_position._lastEventTime >= earliestLastEventTime) {
		_position._lastEventTime -= earliestLastEventTime;
	} else {
		_position._lastEventTime = 0;
	}
	_position._playTick -= earliestLastEventTick;
	_position._lastEventTick -= earliestLastEventTick;
}

void MidiParser::determineNextEvent() {
	uint32 lowestNextEventTick = 0xFFFFFFFF;
	int lowestNextEventTickSubtrack = -1;
	for (int i = 0; i < _numSubtracks[_activeTrack]; i++) {
		if (_position.isTracking(i)) {
			uint32 subtrackNextEventTick = _position._subtracks[i]._lastEventTick + _nextSubtrackEvents[i].delta;
			if (subtrackNextEventTick < lowestNextEventTick) {
				lowestNextEventTick = subtrackNextEventTick;
				lowestNextEventTickSubtrack = i;
			}
		}
	}
	_nextEvent = &_nextSubtrackEvents[lowestNextEventTickSubtrack >= 0 ? lowestNextEventTickSubtrack : 0];
}

bool MidiParser::processEvent(const EventInfo &info, bool fireEvents) {
	if (info.event == 0xF0) {
		// SysEx event
		// Check for trailing 0xF7 -- if present, remove it.
		if (fireEvents) {
			if (_sysExDelay > 0)
				// Don't process this event if the delay from
				// the previous SysEx hasn't passed yet.
				return false;

			uint16 delay;
			if (info.ext.data[info.length-1] == 0xF7)
				delay = _driver->sysExNoDelay(info.ext.data, (uint16)info.length-1);
			else
				delay = _driver->sysExNoDelay(info.ext.data, (uint16)info.length);

			// Set the delay in microseconds so the next
			// SysEx event will be delayed if necessary.
			_sysExDelay = delay * 1000;
		}
	} else if (info.event == 0xFF) {
		// META event
		bool sendEventToDriver = true;
		if (info.ext.type == 0x2F) {
			// End of Track must be processed by us,
			// as well as sending it to the output device.
			_position.stopTracking(info.subtrack);
			if (!_position.isTracking()) {
				// All subtracks have finished playing
				if (_loopStartPoint != 0xFFFFFFFF && _loopEndPoint == 0) {
					jumpToTick(_loopStartPoint);
				}
				else if (_autoLoop) {
					jumpToTick(0);
				}
				else {
					stopPlaying();
					if (fireEvents)
						sendMetaEventToDriver(info.ext.type, info.ext.data, (uint16)info.length);
				}
				return false;
			}
			// Do not send End of Track to driver when there are
			// still subtracks playing
			sendEventToDriver = false;
		} else if (info.ext.type == 0x51) {
			if (info.length >= 3) {
				setTempo(info.ext.data[0] << 16 | info.ext.data[1] << 8 | info.ext.data[2]);
			}
		}
		if (fireEvents && sendEventToDriver)
			sendMetaEventToDriver(info.ext.type, info.ext.data, (uint16)info.length);
	} else {
		if (fireEvents)
			sendToDriver(info.event, info.basic.param1, info.basic.param2);
	}

	return true;
}


void MidiParser::allNotesOff() {
	if (!_driver)
		return;

	int i, j;

	// Turn off all active notes
	for (i = 0; i < 128; ++i) {
		for (j = 0; j < 16; ++j) {
			if (_activeNotes[i] & (1 << j)) {
				sendToDriver(0x80 | j, i, 0);
			}
		}
	}

	// Turn off all hanging notes
	for (i = 0; i < ARRAYSIZE(_hangingNotes); i++) {
		if (_hangingNotes[i].timeLeft) {
			sendToDriver(0x80 | _hangingNotes[i].channel, _hangingNotes[i].note, 0);
			_hangingNotes[i].timeLeft = 0;
		}
	}
	_hangingNotesCount = 0;

	if (!_disableAllNotesOffMidiEvents) {
		// To be sure, send an "All Note Off" event (but not all MIDI devices
		// support this...).
		_driver->stopAllNotes(_sendSustainOffOnNotesOff);
	}

	memset(_activeNotes, 0, sizeof(_activeNotes));
}

void MidiParser::resetTracking() {
	_position.clear();
}

void MidiParser::setLoopSection(uint32 startPoint, uint32 endPoint) {
	_loopStartPoint = startPoint;
	_loopEndPoint = endPoint;
}

void MidiParser::setLoopSectionMicroseconds(uint32 startPoint, uint32 endPoint) {
	_loopStartPointMs = startPoint;
	_loopEndPointMs = endPoint;
	_loopStartPoint = (startPoint == 0 ? 0 : 0xFFFFFFFF);
	_loopEndPoint = (endPoint == 0 ? 0 : 0xFFFFFFFF);
}

bool MidiParser::setTrack(int track) {
	if (track < 0 || track >= _numTracks)
		return false;
	// We allow restarting the track via setTrack when
	// it isn't playing anymore. This allows us to reuse
	// a MidiParser when a track has finished and will
	// be restarted via setTrack by the client again.
	// This isn't exactly how setTrack behaved before though,
	// the old MidiParser code did not allow setTrack to be
	// used to restart a track, which was already finished.
	//
	// TODO: Check if any engine has problem with this
	// handling, if so we need to find a better way to handle
	// track restarts. (KYRA relies on this working)
	else if (track == _activeTrack && isPlaying())
		return true;

	if (_smartJump)
		hangAllActiveNotes();
	else if (isPlaying())
		allNotesOff();

	resetTracking();
	_pause = false;
	memset(_activeNotes, 0, sizeof(_activeNotes));
	if (_disableAutoStartPlayback)
		_doParse = false;
	for (int i = 0; i < MAXIMUM_SUBTRACKS; i++) {
		_nextSubtrackEvents[i].clear();
	}
	_nextEvent = &_nextSubtrackEvents[0];

	onTrackStart(track);

	_activeTrack = track;
	for (int i = 0; i < _numSubtracks[_activeTrack]; i++) {
		_position._subtracks[i]._playPos = _tracks[_activeTrack][i];
		parseNextEvent(_nextSubtrackEvents[i]);
	}
	determineNextEvent();

	return true;
}

void MidiParser::stopPlaying() {
	if (isPlaying())
		allNotesOff();
	resetTracking();
	_pause = false;
}

bool MidiParser::startPlaying() {
	if (_activeTrack >= _numTracks || _pause)
		return false;
	if (!_position.isTracking()) {
		for (int i = 0; i < MAXIMUM_SUBTRACKS; i++) {
			_nextSubtrackEvents[i].clear();
		}
		_nextEvent = &_nextSubtrackEvents[0];
		for (int i = 0; i < _numSubtracks[_activeTrack]; i++) {
			_position._subtracks[i]._playPos = _tracks[_activeTrack][i];
			parseNextEvent(_nextSubtrackEvents[i]);
		}
		determineNextEvent();
	}

	_doParse = true;
	return true;
}

void MidiParser::pausePlaying() {
	if (isPlaying() && !_pause) {
		_pause = true;
		allNotesOff();
	}
}

void MidiParser::resumePlaying() {
	_pause = false;
}

void MidiParser::hangAllActiveNotes() {
	// Search for note off events until we have
	// accounted for every active note.
	uint16 tempActive[128];
	memcpy(tempActive, _activeNotes, sizeof (tempActive));
	Tracker currentPos(_position);

	while (true) {
		int i;
		for (i = 0; i < 128; ++i)
			if (tempActive[i] != 0)
				break;
		if (i == 128)
			break;

		if (_position.isTracking(_nextEvent->subtrack))
			parseNextEvent(_nextSubtrackEvents[_nextEvent->subtrack]);
		determineNextEvent();

		uint8 subtrack = _nextEvent->subtrack;
		uint32 eventTick = _position._subtracks[subtrack]._lastEventTick + _nextEvent->delta;
		if (_nextEvent->command() == 0x8) {
			if (tempActive[_nextEvent->basic.param1] & (1 << _nextEvent->channel())) {
				hangingNote(_nextEvent->channel(), _nextEvent->basic.param1, (eventTick - currentPos._lastEventTick) * _psecPerTick, false);
				tempActive[_nextEvent->basic.param1] &= ~(1 << _nextEvent->channel());
			}
		} else if (!_position.isTracking() || (_nextEvent->event == 0xFF && _nextEvent->ext.type == 0x2F)) {
			_position.stopTracking(subtrack);
			if (!_position.isTracking()) {
				// warning("MidiParser::hangAllActiveNotes(): Hit End of Track with active notes left");
				for (i = 0; i < 128; ++i) {
					for (int j = 0; j < 16; ++j) {
						if (tempActive[i] & (1 << j)) {
							activeNote(j, i, false);
							sendToDriver(0x80 | j, i, 0);
						}
					}
				}
				break;
			}
		}

		_position._lastEventTick = eventTick;
		_position._subtracks[subtrack]._lastEventTick = eventTick;
	}

	_position = currentPos;
}

bool MidiParser::jumpToTick(uint32 tick, bool fireEvents, bool stopNotes, bool dontSendNoteOn) {
	if (_activeTrack >= _numTracks || _pause)
		return false;

	assert(!_jumpingToTick); // This function is not re-entrant
	_jumpingToTick = true;

	Tracker currentPos(_position);
	EventInfo *currentEvent = _nextEvent;
	EventInfo currentSubtrackEvents[MAXIMUM_SUBTRACKS];
	for (int i = 0; i < _numSubtracks[_activeTrack]; i++) {
		currentSubtrackEvents[i] = _nextSubtrackEvents[i];
	}

	resetTracking();
	for (int i = 0; i < _numSubtracks[_activeTrack]; i++) {
		_position._subtracks[i]._playPos = _tracks[_activeTrack][i];
		parseNextEvent(_nextSubtrackEvents[i]);
	}
	determineNextEvent();
	if (tick > 0) {
		while (true) {
			EventInfo &info = *_nextEvent;
			uint8 subtrack = info.subtrack;
			uint32 eventTick = _position._subtracks[subtrack]._lastEventTick + info.delta;
			if (eventTick >= tick) {
				_position._playTime += (tick - _position._lastEventTick) * _psecPerTick;
				_position._playTick = tick;
				break;
			}

			// Some special processing for the fast-forward case
			if (info.command() == 0x9 && dontSendNoteOn) {
				// Don't send note on; doing so creates a "warble" with
				// some instruments on the MT-32. Refer to bug #9262
			} else if (info.event == 0xFF && info.ext.type == 0x2F) {
				// End of track
				_position.stopTracking(info.subtrack);
				if (!_position.isTracking()) {
					// This means that we failed to find the right tick.
					_position = currentPos;
					_nextEvent = currentEvent;
					for (int i = 0; i < _numSubtracks[_activeTrack]; i++) {
						_nextSubtrackEvents[i]  = currentSubtrackEvents[i];
					}
					_jumpingToTick = false;
					return false;
				}
			} else {
				processEvent(info, fireEvents);
			}

			uint32 eventTime = _position._lastEventTime + (eventTick - _position._lastEventTick) * _psecPerTick;
			_position._playTime = eventTime;
			_position._lastEventTime = eventTime;
			_position._subtracks[subtrack]._lastEventTime = eventTime;

			_position._playTick = eventTick;
			_position._lastEventTick = eventTick;
			_position._subtracks[subtrack]._lastEventTick = eventTick;

			if (_position.isTracking(subtrack)) {
				parseNextEvent(_nextSubtrackEvents[subtrack]);
			}
			determineNextEvent();
		}
	}

	if (stopNotes) {
		if (!_smartJump || !currentPos.isTracking()) {
			allNotesOff();
		} else {
			Tracker targetPosition(_position);
			EventInfo *targetEvent = _nextEvent;
			EventInfo targetSubtrackEvents[MAXIMUM_SUBTRACKS];
			for (int i = 0; i < _numSubtracks[_activeTrack]; i++) {
				targetSubtrackEvents[i]  = _nextSubtrackEvents[i];
			}

			_position = currentPos;
			_nextEvent = currentEvent;
			for (int i = 0; i < _numSubtracks[_activeTrack]; i++) {
				_nextSubtrackEvents[i] = currentSubtrackEvents[i];
			}
			hangAllActiveNotes();

			_position = targetPosition;
			_nextEvent = targetEvent;
			for (int i = 0; i < _numSubtracks[_activeTrack]; i++) {
				_nextSubtrackEvents[i] = targetSubtrackEvents[i];
			}
		}
	}

	_abortParse = true;
	_jumpingToTick = false;
	return true;
}

void MidiParser::unloadMusic() {
	if (_numTracks == 0)
		// No music data loaded
		return;

	stopPlaying();
	_numTracks = 0;
	_activeTrack = 255;
	_abortParse = true;
	memset(_tracks, 0, sizeof(_tracks));
	memset(_numSubtracks, 1, sizeof(_numSubtracks));
	for (int i = 0; i < MAXIMUM_SUBTRACKS; i++) {
		_nextSubtrackEvents[i].clear();
		_nextSubtrackEvents[i].subtrack = i;
	}
	_nextEvent = &_nextSubtrackEvents[0];

	if (_centerPitchWheelOnUnload) {
		// Center the pitch wheels in preparation for the next piece of
		// music. It's not safe to do this from within allNotesOff(),
		// and might not even be safe here, so we only do it if the
		// client has explicitly asked for it.

		if (_driver) {
			for (int i = 0; i < 16; ++i) {
				sendToDriver(0xE0 | i, 0, 0x40);
			}
		}
	}
}
