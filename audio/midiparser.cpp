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

#include "audio/midiparser.h"
#include "audio/mididrv.h"
#include "common/textconsole.h"
#include "common/util.h"

//////////////////////////////////////////////////
//
// MidiParser implementation
//
//////////////////////////////////////////////////

MidiParser::MidiParser() :
_hangingNotesCount(0),
_driver(0),
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
_numTracks(0),
_activeTrack(255),
_abortParse(false),
_jumpingToTick(false),
_doParse(true),
_pause(false) {
	memset(_activeNotes, 0, sizeof(_activeNotes));
	memset(_tracks, 0, sizeof(_tracks));
	_nextEvent.start = NULL;
	_nextEvent.delta = 0;
	_nextEvent.event = 0;
	_nextEvent.length = 0;
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
	_driver->send(b);
}

void MidiParser::sendMetaEventToDriver(byte type, byte *data, uint16 length) {
	_driver->metaEvent(type, data, length);
}

void MidiParser::setTempo(uint32 tempo) {
	_tempo = tempo;
	if (_ppqn)
		_psecPerTick = (tempo + (_ppqn >> 2)) / _ppqn;
}

// This is the conventional (i.e. SMF) variable length quantity
uint32 MidiParser::readVLQ(byte * &data) {
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
	NoteTimer *best = 0;
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

	// Occassionally we might get a zero or negative note
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

	// The SysEx delay can be decreased whenever time passes,
	// even if the parser does not parse events.
	_sysExDelay -= (_sysExDelay > _timerRate) ? _timerRate : _sysExDelay;

	if (!_position._playPos || !_driver || !_doParse || _pause || !_driver->isReady())
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
		EventInfo &info = _nextEvent;

		eventTime = _position._lastEventTime + info.delta * _psecPerTick;
		if (eventTime > endTime)
			break;

		// Process the next info.
		if (info.event < 0x80) {
			warning("Bad command or running status %02X", info.event);
			_position._playPos = 0;
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

		loopEvent |= info.loop;

		if (!_abortParse) {
			_position._lastEventTime = eventTime;
			_position._lastEventTick += info.delta;
			parseNextEvent(_nextEvent);
		}
	}

	if (!_abortParse) {
		_position._playTime = endTime;
		_position._playTick = (_position._playTime - _position._lastEventTime) / _psecPerTick + _position._lastEventTick;
		if (loopEvent) {
			// One of the processed events has looped (part of) the MIDI data.
			// Infinite looping will cause the tracker to overflow eventually.
			// Reset the tracker positions to prevent this from happening.
			_position._playTime -= _position._lastEventTime;
			_position._lastEventTime = 0;
			_position._playTick -= _position._lastEventTick;
			_position._lastEventTick = 0;
		}
	}
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
		if (info.ext.type == 0x2F) {
			// End of Track must be processed by us,
			// as well as sending it to the output device.
			if (_autoLoop) {
				jumpToTick(0);
				parseNextEvent(_nextEvent);
			} else {
				stopPlaying();
				if (fireEvents)
					sendMetaEventToDriver(info.ext.type, info.ext.data, (uint16)info.length);
			}
			return false;
		} else if (info.ext.type == 0x51) {
			if (info.length >= 3) {
				setTempo(info.ext.data[0] << 16 | info.ext.data[1] << 8 | info.ext.data[2]);
			}
		}
		if (fireEvents)
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

	onTrackStart(track);

	_activeTrack = track;
	_position._playPos = _tracks[track];
	parseNextEvent(_nextEvent);
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
	if (!_position._playPos) {
		_position._playPos = _tracks[_activeTrack];
		parseNextEvent(_nextEvent);
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

	uint32 advanceTick = _position._lastEventTick;
	while (true) {
		int i;
		for (i = 0; i < 128; ++i)
			if (tempActive[i] != 0)
				break;
		if (i == 128)
			break;
		parseNextEvent(_nextEvent);
		advanceTick += _nextEvent.delta;
		if (_nextEvent.command() == 0x8) {
			if (tempActive[_nextEvent.basic.param1] & (1 << _nextEvent.channel())) {
				hangingNote(_nextEvent.channel(), _nextEvent.basic.param1, (advanceTick - _position._lastEventTick) * _psecPerTick, false);
				tempActive[_nextEvent.basic.param1] &= ~(1 << _nextEvent.channel());
			}
		} else if (_nextEvent.event == 0xFF && _nextEvent.ext.type == 0x2F) {
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
}

bool MidiParser::jumpToTick(uint32 tick, bool fireEvents, bool stopNotes, bool dontSendNoteOn) {
	if (_activeTrack >= _numTracks || _pause)
		return false;

	assert(!_jumpingToTick); // This function is not re-entrant
	_jumpingToTick = true;

	Tracker currentPos(_position);
	EventInfo currentEvent(_nextEvent);

	resetTracking();
	_position._playPos = _tracks[_activeTrack];
	parseNextEvent(_nextEvent);
	if (tick > 0) {
		while (true) {
			EventInfo &info = _nextEvent;
			if (_position._lastEventTick + info.delta >= tick) {
				_position._playTime += (tick - _position._lastEventTick) * _psecPerTick;
				_position._playTick = tick;
				break;
			}

			_position._lastEventTick += info.delta;
			_position._lastEventTime += info.delta * _psecPerTick;
			_position._playTick = _position._lastEventTick;
			_position._playTime = _position._lastEventTime;

			// Some special processing for the fast-forward case
			if (info.command() == 0x9 && dontSendNoteOn) {
				// Don't send note on; doing so creates a "warble" with
				// some instruments on the MT-32. Refer to patch #3117577
			} else if (info.event == 0xFF && info.ext.type == 0x2F) {
				// End of track
				// This means that we failed to find the right tick.
				_position = currentPos;
				_nextEvent = currentEvent;
				_jumpingToTick = false;
				return false;
			} else {
				processEvent(info, fireEvents);
			}

			parseNextEvent(_nextEvent);
		}
	}

	if (stopNotes) {
		if (!_smartJump || !currentPos._playPos) {
			allNotesOff();
		} else {
			EventInfo targetEvent(_nextEvent);
			Tracker targetPosition(_position);

			_position = currentPos;
			_nextEvent = currentEvent;
			hangAllActiveNotes();

			_nextEvent = targetEvent;
			_position = targetPosition;
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
