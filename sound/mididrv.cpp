/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

/*
 * Raw output support by Michael Pearce
 * MorphOS support by Ruediger Hanke 
 * Alsa support by Nicolas Noble <nicolas@nobis-crew.org> copied from
 *    both the QuickTime support and (vkeybd http://www.alsa-project.org/~iwai/alsa.html)
 */

#include "stdafx.h"
#include "mididrv.h"
#include "mpu401.h"
#include "common/engine.h"	// for warning/error/debug
#include "common/util.h"	// for ARRAYSIZE



/* Default (empty) property method */
uint32 MidiDriver::property(int prop, uint32 param)
{
	return 0;
}


/* retrieve a string representation of an error code */
const char *MidiDriver::get_error_name(int error_code)
{
	static const char *const midi_errors[] = {
		"No error",
		"Cannot connect",
		"Streaming not available",
		"Device not available",
		"Driver already open"
	};

	if ((uint) error_code >= ARRAYSIZE(midi_errors))
		return "Unknown Error";
	return midi_errors[error_code];
}

#ifdef __MORPHOS__
#include <exec/memory.h>
#include <exec/types.h>
#include <devices/etude.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/etude.h>

#include "morphos_sound.h"

/* MorphOS MIDI driver */
class MidiDriver_ETUDE:public MidiDriver_MPU401 {
public:
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool p);
	void set_stream_callback(void *param, StreamCallback *sc);
	// void setPitchBendRange (byte channel, uint range);

private:
	enum {
		NUM_BUFFERS = 2,
		MIDI_EVENT_SIZE = 64,
		BUFFER_SIZE = MIDI_EVENT_SIZE * 12,
	};

	static void midi_callback(ULONG msg, struct IOMidiRequest *req, APTR user_data);
	void fill_all();
	uint32 property(int prop, uint32 param);

	StreamCallback *_stream_proc;
	void *_stream_param;
	IOMidiRequest *_stream_req[NUM_BUFFERS];
	void *_stream_buf[NUM_BUFFERS];
	bool _req_sent[NUM_BUFFERS];
	int _mode;
	uint16 _time_div;
};

void MidiDriver_ETUDE::set_stream_callback(void *param, StreamCallback *sc)
{
	_stream_param = param;
	_stream_proc = sc;
}

int MidiDriver_ETUDE::open(int mode)
{
	if (_mode != 0)
		return MERR_ALREADY_OPEN;
	_mode = mode;
	if (!init_morphos_music(0, _mode == MO_STREAMING ? ETUDEF_STREAMING : ETUDEF_DIRECT))
		return MERR_DEVICE_NOT_AVAILABLE;

	if (_mode == MO_STREAMING && ScummMidiRequest) {
		_stream_req[0] = ScummMidiRequest;
		_stream_req[1] = (IOMidiRequest *) AllocVec(sizeof (IOMidiRequest), MEMF_PUBLIC);
		_stream_buf[0] = AllocVec(BUFFER_SIZE, MEMF_PUBLIC);
		_stream_buf[1] = AllocVec(BUFFER_SIZE, MEMF_PUBLIC);
		_req_sent[0] = _req_sent[1] = false;

		if (_stream_req[1] == NULL || _stream_buf[0] == NULL || _stream_buf[1] == NULL) {
			close();
			return MERR_DEVICE_NOT_AVAILABLE;
		}

		if (ScummMidiRequest)
		{
			memcpy(_stream_req[1], _stream_req[0], sizeof (IOMidiRequest));
			struct TagItem MidiStreamTags[] = { { ESA_Callback, (ULONG) &midi_callback },
															{ ESA_UserData, (ULONG) this },
															{ ESA_TimeDiv, _time_div },
															{ TAG_DONE, 0 }
														 };
			SetMidiStreamAttrsA(ScummMidiRequest, MidiStreamTags);
			fill_all();
		}
	}

	return 0;
}

void MidiDriver_ETUDE::close()
{
	if (_mode == MO_STREAMING) {
		if (_req_sent[0]) {
			AbortIO((IORequest *) _stream_req[0]);
			WaitIO((IORequest *) _stream_req[0]);
			_req_sent[0] = false;
		}
		if (_req_sent[1]) {
			AbortIO((IORequest *) _stream_req[1]);
			WaitIO((IORequest *) _stream_req[1]);
			_req_sent[1] = false;
		}

		if (_stream_req[1]) FreeVec(_stream_req[1]);
		if (_stream_buf[0]) FreeVec(_stream_buf[0]);
		if (_stream_buf[1]) FreeVec(_stream_buf[1]);
	}

	exit_morphos_music();
	_mode = 0;
}

void MidiDriver_ETUDE::send(uint32 b)
{
	if (_mode != MO_SIMPLE)
		error("MidiDriver_ETUDE::send called but driver is not in simple mode");

	if (ScummMidiRequest) {
		ULONG midi_data = READ_LE_UINT32(&b);
		SendShortMidiMsg(ScummMidiRequest, midi_data);
	}
}

void MidiDriver_ETUDE::midi_callback(ULONG msg, struct IOMidiRequest *req, APTR user_data)
{
	switch (msg) {
		case ETUDE_STREAM_MSG_BLOCKEND: {
			MidiDriver_ETUDE *md = ((MidiDriver_ETUDE *) user_data);
			if (md && md->_mode)
				md->fill_all();
			break;
		}
	}
}

void MidiDriver_ETUDE::fill_all()
{
	if (_stream_proc == NULL) {
		error("MidiDriver_ETUDE::fill_all() called, but _stream_proc == NULL");
	}

	uint buf;
	MidiEvent my_evs[64];

	for (buf = 0; buf < NUM_BUFFERS; buf++) {
		if (!_req_sent[buf] || CheckIO((IORequest *) _stream_req[buf])) {
			int num = _stream_proc(_stream_param, my_evs, 64);

			if (_req_sent[buf]) {
				WaitIO((IORequest *) _stream_req[buf]);
				_req_sent[buf] = false;
			}

			/* end of stream? */
			if (num == 0)
				break;

			MIDIEVENT *ev = (MIDIEVENT *) _stream_buf[buf];
			MidiEvent *my_ev = my_evs;

			for (int i = 0; i < num; i++, my_ev++) {
				ev->me_StreamID = 0;
				ev->me_DeltaTime = my_ev->delta;

				switch (my_ev->event >> 24) {
				case 0:
					ev->me_Event = my_ev->event;
					break;
				case ME_TEMPO:
					/* change tempo event */
					ev->me_Event = (MEVT_TEMPO << 24) | (my_ev->event & 0xFFFFFF);
					break;
				default:
					error("Invalid event type passed");
				}

				/* increase stream pointer by 12 bytes 
				 * (need to be 12 bytes, and sizeof(MIDIEVENT) is 16) 
				 */
				ev = (MIDIEVENT *)((byte *)ev + 12);
			}

			ConvertWindowsMidiStream(_stream_buf[buf], num * 12);

			_stream_req[buf]->emr_Std.io_Command = CMD_WRITE;
			_stream_req[buf]->emr_Std.io_Data = _stream_buf[buf];
			_stream_req[buf]->emr_Std.io_Length = num * 12;
		   SendIO((IORequest *) _stream_req[buf]);
			_req_sent[buf] = true;
		}
	}
}

void MidiDriver_ETUDE::pause(bool p)
{
	if (_mode == MO_STREAMING && ScummMidiRequest) {
		if (p)
			PauseMidiStream(ScummMidiRequest);
		else
			RestartMidiStream(ScummMidiRequest);
	}
}

uint32 MidiDriver_ETUDE::property(int prop, uint32 param)
{
	switch (prop) {
		/* 16-bit time division according to standard midi specification */
		case PROP_TIMEDIV:
			_time_div = (uint16)param;
			return 1;
	}

	return 0;
}

extern MidiDriver* EtudeMidiDriver;

MidiDriver *MidiDriver_ETUDE_create()
{
	EtudeMidiDriver = new MidiDriver_ETUDE();
	return EtudeMidiDriver;
}

#endif // __MORPHOS__

#if defined(UNIX) && !defined(__BEOS__)
#define SEQ_MIDIPUTC    5
#define SPECIAL_CHANNEL 9

class MidiDriver_SEQ:public MidiDriver_MPU401 {
public:
	MidiDriver_SEQ();
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool p);
	void set_stream_callback(void *param, StreamCallback *sc);
	// void setPitchBendRange (byte channel, uint range);

private:
	StreamCallback *_stream_proc;
	void *_stream_param;
	int _mode;
	int device, _device_num;
};

MidiDriver_SEQ::MidiDriver_SEQ()
{
	_mode = 0;
	device = 0;
	_device_num = 0;
}

int MidiDriver_SEQ::open(int mode)
{
	if (_mode != 0)
		return MERR_ALREADY_OPEN;
	device = 0;
	if (mode != MO_SIMPLE)
		return MERR_STREAMING_NOT_AVAILABLE;

	_mode = mode;

	char *device_name = getenv("SCUMMVM_MIDI");
	if (device_name != NULL) {
		device = (::open((device_name), O_RDWR, 0));
	} else {
		warning("You need to set-up the SCUMMVM_MIDI environment variable properly (see README) ");
	}
	if ((device_name == NULL) || (device < 0)) {
		if (device_name == NULL)
			warning("Opening /dev/null (no music will be heard) ");
		else
			warning("Cannot open rawmidi device %s - using /dev/null (no music will be heard) ",
							device_name);
		device = (::open(("/dev/null"), O_RDWR, 0));
		if (device < 0)
			error("Cannot open /dev/null to dump midi output");
	}

	if (getenv("SCUMMVM_MIDIPORT"))
		_device_num = atoi(getenv("SCUMMVM_MIDIPORT"));
	return 0;
}

void MidiDriver_SEQ::close()
{
	::close(device);
	_mode = 0;
}


void MidiDriver_SEQ::send(uint32 b)
{
	unsigned char buf[256];
	int position = 0;

	switch (b & 0xF0) {
	case 0x80:
	case 0x90:
	case 0xA0:
	case 0xB0:
	case 0xE0:
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)b;
		buf[position++] = _device_num;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)((b >> 8) & 0x7F);
		buf[position++] = _device_num;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)((b >> 16) & 0x7F);
		buf[position++] = _device_num;
		buf[position++] = 0;
		break;
	case 0xC0:
	case 0xD0:
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)b;
		buf[position++] = _device_num;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)((b >> 8) & 0x7F);
		buf[position++] = _device_num;
		buf[position++] = 0;
		break;
	default:
		fprintf(stderr, "Unknown : %08x\n", (int)b);
		break;
	}
	write(device, buf, position);
}

void MidiDriver_SEQ::pause(bool p)
{
	if (_mode == MO_STREAMING) {
	}
}

void MidiDriver_SEQ::set_stream_callback(void *param, StreamCallback *sc)
{
	_stream_param = param;
	_stream_proc = sc;
}

MidiDriver *MidiDriver_SEQ_create()
{
	return new MidiDriver_SEQ();
}

#endif

