/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

//! \brief Declarations related to the MidiParser class

#ifndef INCLUDED_MIDIPARSER
#define INCLUDED_MIDIPARSER

class MidiParser;

#include "common/scummsys.h"

class MidiDriver;



//////////////////////////////////////////////////
//
// Support entities
//
//////////////////////////////////////////////////

//! Maintains time and position state within a MIDI stream.
/*! A single Tracker struct is used by MidiParser to keep track
 *  of its current position in the MIDI stream. The Tracker
 *  struct, however, allows alternative locations to be cached.
 *  See MidiParser::jumpToTick() for an example of tracking
 *  multiple locations within a MIDI stream. NOTE: It is
 *  important to also maintain pre-parsed EventInfo data for
 *  each Tracker location.
 */

struct Tracker {
	byte * _play_pos;        //!< A pointer to the next event to be parsed
	uint32 _play_time;       //!< Current time in microseconds; may be in between event times
	uint32 _play_tick;       //!< Current MIDI tick; may be in between event ticks
	uint32 _last_event_time; //!< The time, in microseconds, of the last event that was parsed
	uint32 _last_event_tick; //!< The tick at which the last parsed event occurs
	byte   _running_status;  //!< Cached MIDI command, for MIDI streams that rely on implied event codes

	Tracker() { clear(); }

	//! Copy constructor for each duplication of Tracker information.
	Tracker (const Tracker &copy) :
	_play_pos (copy._play_pos),
	_play_time (copy._play_time),
	_play_tick (copy._play_tick),
	_last_event_time (copy._last_event_time),
	_last_event_tick (copy._last_event_tick),
	_running_status (copy._running_status)
	{ }

	//! Clears all data; used by the constructor for initialization.
	void clear() {
		_play_pos = 0;
		_play_time = 0;
		_play_tick = 0;
		_last_event_time = 0;
		_last_event_tick = 0;
		_running_status = 0;
	}
};

//! Provides comprehensive information on the next event in the MIDI stream.
/*! An EventInfo struct is instantiated by format-specific implementations
 *  of MidiParser::parseNextEvent() each time another event is needed.
 */

struct EventInfo {
	byte * start; //!< Position in the MIDI stream where the event starts.
	              //!< For delta-based MIDI streams (e.g. SMF and XMIDI), this points to the delta.
	uint32 delta; //!< The number of ticks after the previous event that this event should occur.
	byte   event; //!< Upper 4 bits are the command code, lower 4 bits are the MIDI channel.
	              //!< For META, event == 0xFF. For SysEx, event == 0xF0.
	union {
		struct {
			byte param1; //!< The first parameter in a simple MIDI message.
			byte param2; //!< The second parameter in a simple MIDI message.
		} basic;
		struct {
			byte   type; //!< For META events, this indicates the META type.
			byte * data; //!< For META and SysEx events, this points to the start of the data.
		} ext;
	};
	uint32 length; //!< For META and SysEx blocks, this indicates the length of the data.
	               //!< For Note On events, a non-zero value indicates that no Note Off event
	               //!< will occur, and the MidiParser will have to generate one itself.
	               //!< For all other events, this value should always be zero.

	byte channel() { return event & 0x0F; } //!< Separates the MIDI channel from the event.
	byte command() { return event >> 4; }   //!< Separates the command code from the event.
};

//! Provides expiration tracking for hanging notes.
/*! Hanging notes are used when a MIDI format does not include explicit Note Off
 *  events, or when "Smart Jump" is enabled so that active notes are intelligently
 *  expired when a jump occurs. The NoteTimer struct keeps track of how much
 *  longer a note should remain active before being turned off.
 */

struct NoteTimer {
	byte channel;     //!< The MIDI channel on which the note was played
	byte note;        //!< The note number for the active note
	uint32 time_left; //!< The time, in microseconds, remaining before the note should be turned off
	NoteTimer() : channel(0), note(0), time_left(0) {}
};




//////////////////////////////////////////////////
//
// MidiParser declaration
//
//////////////////////////////////////////////////

//! A framework and common functionality for parsing MIDI streams.
/*! The MidiParser provides a framework in which to load
 *  parse and traverse MIDI streams. Loading and parsing
 *  events for specific formats requires a class to be
 *  derived from MidiParser.
 *
 *  The MidiParser must be provided with a MidiDriver interface,
 *  which it uses to transmit events. However, it does NOT
 *  automatically hook into the MidiDriver's timer callback
 *  or set up its timer rate from the MidiDriver. The client
 *  using the MidiParser must set the timer rate and provide
 *  a means of firing the MidiParser's onTimer() method at
 *  appropriate intervals. The onTimer() event may be called
 *  by the client or by manually hooking and unhooking the
 *  MidiParser to the MidiDriver's timer callback.
 */

class MidiParser {
private:
	uint16    _active_notes[128];   //!< Each uint16 is a bit mask for channels that have that note on.
	NoteTimer _hanging_notes[32];   //!< Maintains expiration info for up to 32 notes.
	                                //!< Used for "Smart Jump" and MIDI formats that do not include explicit Note Off events.
	byte      _hanging_notes_count; //!< Count of hanging notes, used to optimize expiration.

protected:
	MidiDriver *_driver;    //!< The device to which all events will be transmitted.
	uint32 _timer_rate;     //!< The time in microseconds between onTimer() calls. Obtained from the MidiDriver.
	uint32 _ppqn;           //!< Pulses Per Quarter Note. (We refer to "pulses" as "ticks".)
	uint32 _tempo;          //!< Microseconds per quarter note.
	uint32 _psec_per_tick;  //!< Microseconds per tick (_tempo / _ppqn).
	bool   _autoLoop;       //!< For lightweight clients that don't provide their own flow control.
	bool   _smartJump;      //!< Support smart expiration of hanging notes when jumping

	byte * _tracks[32];     //!< Multi-track MIDI formats are supported, up to 32 tracks.
	byte   _num_tracks;     //!< Count of total tracks for multi-track MIDI formats. 1 for single-track formats.
	byte   _active_track;   //!< Keeps track of the currently active track, in multi-track formats.

	Tracker _position;      //!< The current time/position in the active track.
	EventInfo _next_event;  //!< The next event to transmit. Events are preparsed
	                        //!< so each event is parsed only once; this permits
	                        //!< simulated events in certain formats.
	bool   _abort_parse;    //!< If a jump or other operation interrupts parsing, flag to abort.

protected:
	static uint32 readVLQ (byte * &data);
	virtual void resetTracking();
	virtual void allNotesOff();
	virtual void parseNextEvent (EventInfo &info) = 0;

	void activeNote (byte channel, byte note, bool active);
	void hangingNote (byte channel, byte note, uint32 ticks_left);
	void hangAllActiveNotes();

	//! Platform independent BE uint32 read-and-advance.
	/*! This helper function reads Big Endian 32-bit numbers
	 *  from a memory pointer, at the same time advancing
	 *  the pointer.
	 */
	uint32 read4high (byte * &data) {
		uint32 val = 0;
		int i;
		for (i = 0; i < 4; ++i) val = (val << 8) | *data++;
		return val;
	}

	//! Platform independent LE uint16 read-and-advance.
	/*! This helper function reads Little Endian 16-bit numbers
	 *  from a memory pointer, at the same time advancing
	 *  the pointer.
	 */
	uint16 read2low  (byte * &data) {
		uint16 val = 0;
		int i;
		for (i = 0; i < 2; ++i) val |= (*data++) << (i * 8);
		return val;
	}

public:
	//! Configuration options for MidiParser
	/*! The following options can be set to modify MidiParser's
	 *  behavior.
	 *
	 *  \b mpMalformedPitchBends - Events containing a pitch bend
	 *  command should be treated as single-byte padding before the
	 *  real event. This allows the MidiParser to work with some
	 *  malformed SMF files from Simon 1/2.
	 *
	 *  \b mpAutoLoop - Sets auto-looping, which can be used by
	 *  lightweight clients that don't provide their own flow control.
	 *
	 *  \b mpSmartJump - Sets smart jumping, which intelligently
	 *  expires notes that are active when a jump is made, rather
	 *  than just cutting them off.
	 */
	enum {
		mpMalformedPitchBends = 1,
		mpAutoLoop = 2,
		mpSmartJump = 3
	};

public:
	MidiParser();
	virtual ~MidiParser() { allNotesOff(); }

	virtual bool loadMusic (byte *data, uint32 size) = 0;
	virtual void unloadMusic();
	virtual void property (int prop, int value);

	void setMidiDriver (MidiDriver *driver) { _driver = driver; }
	void setTimerRate (uint32 rate) { _timer_rate = rate; }
	void setTempo (uint32 tempo);
	void onTimer();

	bool setTrack (int track);
	bool jumpToTick (uint32 tick, bool fireEvents = false);
	uint32 getTick() { return _position._play_tick; }

	static MidiParser *createParser_SMF();
	static MidiParser *createParser_XMIDI();
	static void timerCallback (void *data) { ((MidiParser *) data)->onTimer(); }
};

#endif
