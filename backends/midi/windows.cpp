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

#if defined(WIN32) && !defined(_WIN32_WCE)

#include "stdafx.h"
#include "sound/mpu401.h"
#include "common/engine.h" // For warning/error/debug

////////////////////////////////////////
//
// Windows MIDI driver
//
////////////////////////////////////////

class MidiDriver_WIN : public MidiDriver_MPU401 {
public:
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool p);
	void set_stream_callback(void *param, StreamCallback *sc);

private:
	struct MyMidiHdr {
		MIDIHDR hdr;
	};

	enum {
		NUM_PREPARED_HEADERS = 2,
		MIDI_EVENT_SIZE = 64,
		BUFFER_SIZE = MIDI_EVENT_SIZE * 12,
	};

	StreamCallback *_stream_proc;
	void *_stream_param;
	int _mode;

	HMIDIOUT _mo;
	HMIDISTRM _ms;

	MyMidiHdr *_prepared_headers;

	uint16 _time_div;

	void unprepare();
	void prepare();
	void check_error(MMRESULT result);
	void fill_all();
	uint32 property(int prop, uint32 param);

	static void CALLBACK midi_callback(HMIDIOUT hmo, UINT wMsg,
	                                   DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
};

void MidiDriver_WIN::set_stream_callback(void *param, StreamCallback *sc)
{
	_stream_param = param;
	_stream_proc = sc;
}

void CALLBACK MidiDriver_WIN::midi_callback(HMIDIOUT hmo, UINT wMsg,
                                            DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{

	switch (wMsg) {
	case MM_MOM_DONE:{
			MidiDriver_WIN *md = ((MidiDriver_WIN *) dwInstance);
			if (md->_mode)
				md->fill_all();
			break;
		}
	}
}

int MidiDriver_WIN::open(int mode)
{
	if (_mode != 0)
		return MERR_ALREADY_OPEN;

	_mode = mode;

	if (mode == MO_SIMPLE) {
		MMRESULT res = midiOutOpen((HMIDIOUT *) & _mo, MIDI_MAPPER, 0, 0, 0);
		if (res != MMSYSERR_NOERROR)
			check_error(res);
	} else {
		// Streaming mode
		MIDIPROPTIMEDIV mptd;
		UINT _midi_device_id = 0;

		check_error(midiStreamOpen(&_ms, &_midi_device_id, 1,
                                   (uint32)midi_callback, (uint32)this, CALLBACK_FUNCTION));

		prepare();

		mptd.cbStruct = sizeof(mptd);
		mptd.dwTimeDiv = _time_div;

		check_error(midiStreamProperty(_ms, (byte *)&mptd, MIDIPROP_SET | MIDIPROP_TIMEDIV));

		fill_all();
	}

	return 0;
}

void MidiDriver_WIN::fill_all()
{
	if (_stream_proc == NULL) {
		error("MidiDriver_WIN::fill_all() called, but _stream_proc==NULL");
	}

	uint i;
	MyMidiHdr *mmh = _prepared_headers;
	MidiEvent my_evs[64];

	for (i = 0; i != NUM_PREPARED_HEADERS; i++, mmh++) {
		if (!(mmh->hdr.dwFlags & MHDR_INQUEUE)) {
			int num = _stream_proc(_stream_param, my_evs, 64);
			int i;

			// End of stream?
			if (num == 0)
				break;

			MIDIEVENT *ev = (MIDIEVENT *)mmh->hdr.lpData;
			MidiEvent *my_ev = my_evs;

			for (i = 0; i != num; i++, my_ev++) {
				ev->dwStreamID = 0;
				ev->dwDeltaTime = my_ev->delta;

				switch (my_ev->event >> 24) {
				case 0:
					ev->dwEvent = my_ev->event;
					break;
				case ME_TEMPO:
					// Change tempo event
					ev->dwEvent = (ME_TEMPO << 24) | (my_ev->event & 0xFFFFFF);
					break;
				default:
					error("Invalid event type passed");
				}

				// Increase stream pointer by 12 bytes.
				// (Need to be 12 bytes, and sizeof(MIDIEVENT) is 16)
				ev = (MIDIEVENT *)((byte *)ev + 12);
			}

			mmh->hdr.dwBytesRecorded = num * 12;
			check_error(midiStreamOut(_ms, &mmh->hdr, sizeof(mmh->hdr)));
		}
	}
}

void MidiDriver_WIN::prepare()
{
	int i;
	MyMidiHdr *mmh;

	_prepared_headers = (MyMidiHdr *) calloc(sizeof(MyMidiHdr), 2);

	for (i = 0, mmh = _prepared_headers; i != NUM_PREPARED_HEADERS; i++, mmh++) {
		mmh->hdr.dwBufferLength = BUFFER_SIZE;
		mmh->hdr.lpData = (LPSTR) calloc(BUFFER_SIZE, 1);

		check_error(midiOutPrepareHeader((HMIDIOUT) _ms, &mmh->hdr, sizeof(mmh->hdr)));
	}
}

void MidiDriver_WIN::unprepare()
{
	uint i;
	MyMidiHdr *mmh = _prepared_headers;

	for (i = 0; i != NUM_PREPARED_HEADERS; i++, mmh++) {
		check_error(midiOutUnprepareHeader((HMIDIOUT) _ms, &mmh->hdr, sizeof(mmh->hdr)));
		free(mmh->hdr.lpData);
		mmh->hdr.lpData = NULL;
	}

	free(_prepared_headers);
}

void MidiDriver_WIN::close()
{
	int mode_was = _mode;
	_mode = 0;

	switch (mode_was) {
	case MO_SIMPLE:
		check_error(midiOutClose(_mo));
		break;
	case MO_STREAMING:;
		check_error(midiStreamStop(_ms));
		check_error(midiOutReset((HMIDIOUT) _ms));
		unprepare();
		check_error(midiStreamClose(_ms));
		break;
	}
}

void MidiDriver_WIN::send(uint32 b)
{
	union {
		DWORD dwData;
		BYTE bData[4];
	} u;

	if (_mode != MO_SIMPLE)
		error("MidiDriver_WIN:send called but driver is not in simple mode");

	u.bData[3] = (byte)((b & 0xFF000000) >> 24);
	u.bData[2] = (byte)((b & 0x00FF0000) >> 16);
	u.bData[1] = (byte)((b & 0x0000FF00) >> 8);
	u.bData[0] = (byte)(b & 0x000000FF);

	// printMidi(u.bData[0], u.bData[1], u.bData[2], u.bData[3]);
	check_error(midiOutShortMsg(_mo, u.dwData));
}

void MidiDriver_WIN::pause(bool p)
{
	if (_mode == MO_STREAMING) {
		if (p)
			check_error(midiStreamPause(_ms));
		else
			check_error(midiStreamRestart(_ms));
	}
}

void MidiDriver_WIN::check_error(MMRESULT result)
{
	char buf[200];
	if (result != MMSYSERR_NOERROR) {
		midiOutGetErrorText(result, buf, 200);
		warning("MM System Error '%s'", buf);
	}
}

uint32 MidiDriver_WIN::property(int prop, uint32 param)
{
	switch (prop) {

	// 16-bit time division per the standard MIDI specification
	case PROP_TIMEDIV:
		_time_div = (uint16)param;
		return 1;
	}

	return 0;
}

MidiDriver *MidiDriver_WIN_create()
{
	return new MidiDriver_WIN();
}

#endif
