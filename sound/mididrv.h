/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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

#ifndef SOUND_MIDIDRV_H
#define SOUND_MIDIDRV_H

#include "scummsys.h"

struct MidiEvent {
	uint32 delta;
	uint32 event;
};

/* Lowlevel Abstract Midi Driver Class */
class MidiDriver {

public:
	/* called whenever the midi driver is in streaming mode,
	 * and more midi commands need to be generated
	 * return 0 to tell the mididriver that the end of stream was reached
	 */
	typedef int StreamCallback (void *param, MidiEvent * ev, int num);


	/* open modes, pass one of those to open() */
	enum {
		MO_SIMPLE = 1,
		MO_STREAMING = 2,
	};

	/* Special events that can be inserted in a MidiEvent.
	 * event = (ME_xxx<<24) | <24-bit data associated with event>
	 */
	enum {
		ME_NONE = 0,
		ME_TEMPO = 1,
	};

	/* error codes returned by open.
	 * can be converted to a string with get_error_name()
	 */
	enum {
		MERR_CANNOT_CONNECT = 1,
		MERR_STREAMING_NOT_AVAILABLE = 2,
		MERR_DEVICE_NOT_AVAILABLE = 3,
		MERR_ALREADY_OPEN = 4,
	};

	enum {
		PROP_TIMEDIV = 1,
	};


	/* open the midi driver.
	 * returns 0 if successful.
	 * otherwise an error code. */
	virtual int open(int mode) = 0;

	/* close the midi driver */
	virtual void close() = 0;

	/* output a packed midi command to the midi stream
	 * valid only if mode is MO_SIMPLE
	 */
	virtual void send(uint32 b) = 0;

	/* set callback when more streams need to be generated.
	 * valid only when mode==MO_STREAMING
	 */
	virtual void set_stream_callback(void *param, StreamCallback *sc) = 0;

	/* Pause or resume streaming MIDI */
	virtual void pause(bool pause) = 0;


	/* Get or set a property */
	virtual uint32 property(int prop, uint32 param);

	/* retrieve a string representation of an error code */
	static const char *get_error_name(int error_code);

	// HIGH-LEVEL SEMANTIC METHODS
	virtual void setPitchBendRange (byte channel, uint range)
	{
		send((  0   << 16) | (101 << 8) | (0xB0 | channel));
		send((  0   << 16) | (100 << 8) | (0xB0 | channel));
		send((range << 16) | (  6 << 8) | (0xB0 | channel));
		send((  0   << 16) | ( 38 << 8) | (0xB0 | channel));
	}

	virtual void sysEx_customInstrument (byte channel, uint32 type, byte *instr) { }

	// Timing functions - MidiDriver now operates timers
	virtual void setTimerCallback (void *timer_param, void (*timer_proc) (void *)) = 0;
	virtual uint32 getBaseTempo (void) = 0;
};



/* driver types */
enum {
	MD_AUTO = 0,
	MD_NULL = 1,
	MD_WINDOWS = 2,
	MD_TIMIDITY = 3,
	MD_SEQ = 4,
	MD_QTMUSIC = 5,
	MD_ETUDE = 6,
	MD_COREAUDIO = 7,
	MD_MIDIEMU = 8,
	MD_ALSA = 9,
	MD_ADLIB = 10
};


/* Factory functions => no need to include the specific classes
 * in this header => faster compile */
extern MidiDriver *MidiDriver_NULL_create();
extern MidiDriver *MidiDriver_ADLIB_create();
extern MidiDriver *MidiDriver_WIN_create();
extern MidiDriver *MidiDriver_TIMIDITY_create();
extern MidiDriver *MidiDriver_SEQ_create();
extern MidiDriver *MidiDriver_QT_create();
extern MidiDriver *MidiDriver_CORE_create();
extern MidiDriver *MidiDriver_ETUDE_create();
extern MidiDriver *MidiDriver_MIDIEMU_create();
extern MidiDriver *MidiDriver_ALSA_create();

#endif
