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
 * Timidity support by Lionel Ulmer <lionel.ulmer@free.fr>
 * QuickTime support by Florent Boudet <flobo@ifrance.com>
 * Raw output support by Michael Pearce
 * MorphOS support by Ruediger Hanke 
 */

#ifdef __MORPHOS__
#include <devices/timer.h>
#endif


#include "stdafx.h"
#include "scumm.h"
#include "gmidi.h"


#ifdef WIN32

/* Windows MIDI driver */
class MidiDriver_WIN : public MidiDriver {
public:
	void destroy();
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool pause);
	void set_stream_callback(void *param, StreamCallback *sc);

private:
	struct MyMidiHdr {
		MIDIHDR hdr;
	};

	enum {
		NUM_PREPARED_HEADERS=2,
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

void MidiDriver_WIN::set_stream_callback(void *param, StreamCallback *sc) {
	_stream_param = param;
	_stream_proc = sc;
}

void MidiDriver_WIN::destroy() {
	close();
	delete this;
}

void CALLBACK MidiDriver_WIN::midi_callback(HMIDIOUT hmo, UINT wMsg,
	DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {

	switch(wMsg) {
	case MM_MOM_DONE:{
		MidiDriver_WIN *md = ((MidiDriver_WIN*)dwInstance);
		if (md->_mode)
			md->fill_all();
		break;
		}
	}
}

int MidiDriver_WIN::open(int mode) {
	if (_mode != 0)
		return MERR_ALREADY_OPEN;
	
	_mode = mode;

	if (mode == MO_SIMPLE) {
		MMRESULT res = midiOutOpen((HMIDIOUT *) &_mo, MIDI_MAPPER, NULL, NULL, 0);
		if (res != MMSYSERR_NOERROR)
			check_error(res);
	} else {
		/* streaming mode */
		MIDIPROPTIMEDIV mptd;
		UINT _midi_device_id = 0;

		check_error(midiStreamOpen(&_ms, &_midi_device_id, 1, 
			(uint32)midi_callback, (uint32)this, CALLBACK_FUNCTION));
		
		prepare();

		mptd.cbStruct = sizeof(mptd);
		mptd.dwTimeDiv = _time_div;

		check_error(midiStreamProperty(_ms, (byte*)&mptd, 
			MIDIPROP_SET | MIDIPROP_TIMEDIV));

		fill_all();
	}

	return 0;
}

void MidiDriver_WIN::fill_all() {
	if (_stream_proc == NULL) {
		error("MidiDriver_WIN::fill_all() called, but _stream_proc==NULL");
	}

	uint i;
	MyMidiHdr *mmh = _prepared_headers;
	MidiEvent my_evs[64];

	for(i=0; i!=NUM_PREPARED_HEADERS; i++,mmh++) {
		if (!(mmh->hdr.dwFlags & MHDR_INQUEUE)) {
			int num = _stream_proc(_stream_param, my_evs, 64);
			int i;

			/* end of stream? */
			if (num == 0)
				break;

			MIDIEVENT *ev = (MIDIEVENT*)mmh->hdr.lpData;
			MidiEvent *my_ev = my_evs;

			for(i=0; i!=num; i++,my_ev++) {
				ev->dwStreamID = 0;
				ev->dwDeltaTime = my_ev->delta;

				switch(my_ev->event>>24) {
				case 0:
					ev->dwEvent = my_ev->event;
					break;
				case ME_TEMPO:
					/* change tempo event */
					ev->dwEvent = (MEVT_TEMPO<<24) | (my_ev->event&0xFFFFFF);
					break;
				default:
					error("Invalid event type passed");
				}
				
				/* increase stream pointer by 12 bytes 
				 * (need to be 12 bytes, and sizeof(MIDIEVENT) is 16) 
				 */
				ev = (MIDIEVENT*)((byte*)ev + 12);
			}

			mmh->hdr.dwBytesRecorded = num * 12;
			check_error(midiStreamOut(_ms, &mmh->hdr, sizeof(mmh->hdr)));
		}
	}
}

void MidiDriver_WIN::prepare() {
	int i;
	MyMidiHdr *mmh;
		
	_prepared_headers = (MyMidiHdr*)calloc(sizeof(MyMidiHdr), 2);

	for(i=0,mmh=_prepared_headers; i!=NUM_PREPARED_HEADERS; i++,mmh++) {
		mmh->hdr.dwBufferLength = BUFFER_SIZE;
		mmh->hdr.lpData = (LPSTR)calloc(BUFFER_SIZE,1);

		check_error(midiOutPrepareHeader(
			(HMIDIOUT)_ms, &mmh->hdr, sizeof(mmh->hdr)));
	}
}

void MidiDriver_WIN::unprepare() {
	uint i;
	MyMidiHdr *mmh = _prepared_headers;

	for(i=0; i!=NUM_PREPARED_HEADERS; i++,mmh++) {
		check_error(midiOutUnprepareHeader(
			(HMIDIOUT)_ms, &mmh->hdr, sizeof(mmh->hdr)));
		free(mmh->hdr.lpData);
		mmh->hdr.lpData = NULL;
	}

	free(_prepared_headers);
}

void MidiDriver_WIN::close() {
	int mode_was = _mode;
	_mode = 0;

	switch(mode_was) {
	case MO_SIMPLE:
		check_error(midiOutClose(_mo));
		break;
	case MO_STREAMING:;
		check_error(midiStreamStop(_ms));
		check_error(midiOutReset((HMIDIOUT)_ms));
		unprepare();
		check_error(midiStreamClose(_ms));
		break;
	}
}

void MidiDriver_WIN::send(uint32 b) {
	if (_mode != MO_SIMPLE)
		error("MidiDriver_WIN:send called but driver is not in simple mode");
	check_error(midiOutShortMsg(_mo, b));
}

void MidiDriver_WIN::pause(bool pause) {
	if (_mode == MO_STREAMING) {
		if (pause)
			check_error(midiStreamPause(_ms));
		else
			check_error(midiStreamRestart(_ms));
	}
}

void MidiDriver_WIN::check_error(MMRESULT result) {
	char buf[200];
	if (result != MMSYSERR_NOERROR) {
		midiOutGetErrorText(result, buf, 200);
		error("MM System Error '%s'", buf);
	}
}

uint32 MidiDriver_WIN::property(int prop, uint32 param) {
	switch(prop) {
	
	/* 16-bit time division according to standard midi specification */
	case PROP_TIMEDIV: 
		_time_div = (uint16)param;
		return 1;
	}

	return 0;
}

MidiDriver *MidiDriver_WIN_create() {
	return new MidiDriver_WIN();
}

#endif // WIN32


/* NULL driver */
class MidiDriver_NULL : public MidiDriver {
public:
	void destroy();
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool pause);
	void set_stream_callback(void *param, StreamCallback *sc);
	uint32 property(int prop, uint32 param);
private:
};

int MidiDriver_NULL::open(int mode) {
	warning("Music not enabled - MIDI support selected with no MIDI driver available. Try Adlib");
	return 0;
}
void MidiDriver_NULL::close() {}
void MidiDriver_NULL::destroy() {}
void MidiDriver_NULL::send(uint32 b) {}
void MidiDriver_NULL::pause(bool pause) {}
void MidiDriver_NULL::set_stream_callback(void *param, StreamCallback *sc) {}
uint32 MidiDriver_NULL::property(int prop, uint32 param) { return 0; }

MidiDriver *MidiDriver_NULL_create() {
	return new MidiDriver_NULL();
}


/* retrieve a string representation of an error code */
const char *MidiDriver::get_error_name(int error_code) {
	static const char * const midi_errors[] = {
		"No error",
		"Cannot connect",
		"Streaming not available",
		"Device not available",
		"Driver already open"
	};
	
	if ((uint)error_code >= ARRAYSIZE(midi_errors))
		return "Unknown Error";
	return midi_errors[error_code];
}








#if 0





void MidiDriver::midiInit()
{
	if (MidiInitialized != true) {
		switch (DeviceType) {
		case MIDI_NULL:
			midiInitNull();
			break;
		case MIDI_WINDOWS:
			midiInitWindows();
			break;
		case MIDI_TIMIDITY:
			midiInitTimidity();
			break;
		case MIDI_SEQ:
			midiInitSeq();
			break;
		case MIDI_QTMUSIC:
			midiInitQuicktime();
			break;
		case MIDI_AMIDI:
			break;
		default:
			DeviceType = 0;
			midiInitNull();
			break;
		}
		MidiInitialized = true;
	} else {
		error("Midi driver already initialized");
	}
}

void MidiDriver::MidiOut(int b)
{
	if (MidiInitialized != true)
		midiInit();

	if (MidiInitialized == true) {
		switch (DeviceType) {
		case MIDI_NULL:
			break;
		case MIDI_WINDOWS:
			MidiOutWindows(_mo, b);
			break;
		case MIDI_TIMIDITY:
		case MIDI_SEQ:
			MidiOutSeq(_mo, b);
			break;
		case MIDI_QTMUSIC:
			MidiOutQuicktime(_mo, b);
			break;
		case MIDI_AMIDI:
			MidiOutMorphOS(_mo, b);
			break;
		default:
			error("Invalid midi device type ");
			break;
		}
	} else {
		warning("Trying to write midi data without the driver being initialized");
	}
}

/*********** Windows			*/
void MidiDriver::midiInitWindows()
{
#ifdef WIN32
	if (midiOutOpen((HMIDIOUT *) & _mo, MIDI_MAPPER, NULL, NULL, 0) !=
			MMSYSERR_NOERROR)
		error("midiOutOpen failed");
#endif
}

void MidiDriver::MidiOutWindows(void *a, int b)
{
#ifdef WIN32
	midiOutShortMsg((HMIDIOUT) a, b);
#endif
}

/*********** Raw midi support	*/
void MidiDriver::midiInitSeq()
{
	int device = open_sequencer_device();
	_mo = (void *)device;
}

int MidiDriver::open_sequencer_device()
{
	int device = 0;
#if !defined(__APPLE__CW)				// No getenv support on Apple Carbon
	char *device_name = getenv("SCUMMVM_MIDI");
	if (device_name != NULL) {
		device = (open((device_name), O_RDWR, 0));
	} else {
		warning
			("You need to set-up the SCUMMVM_MIDI environment variable properly (see readme.txt) ");
	}
	if ((device_name == NULL) || (device < 0)) {
		if (device_name == NULL)
			warning("Opening /dev/null (no music will be heard) ");
		else
			warning
				("Cannot open rawmidi device %s - using /dev/null (no music will be heard) ",
				 device_name);
		device = (open(("/dev/null"), O_RDWR, 0));
		if (device < 0)
			error("Cannot open /dev/null to dump midi output");
	}
#endif
	return device;
}

/*********** Timidity		*/
int MidiDriver::connect_to_timidity(int port)
{
	int s = 0;
#if !defined(__APPLE__CW) && !defined(__MORPHOS__)	// No socket support on Apple Carbon or Morphos
	struct hostent *serverhost;
	struct sockaddr_in sadd;

	serverhost = gethostbyname("localhost");
	if (serverhost == NULL)
		error("Could not resolve Timidity host ('localhost')");

	sadd.sin_family = serverhost->h_addrtype;
	sadd.sin_port = htons(port);
	memcpy(&(sadd.sin_addr), serverhost->h_addr_list[0], serverhost->h_length);

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		error("Could not open Timidity socket");

	if (connect(s, (struct sockaddr *)&sadd, sizeof(struct sockaddr_in)) < 0)
		error("Could not connect to Timidity server");
#endif
	return s;
}

void MidiDriver::midiInitTimidity()
{
	int s, s2;
	int len;
	int dummy, newport;
	char buf[256];

	s = connect_to_timidity(7777);
	len = read(s, buf, 256);			// buf[len] = '\0'; printf("%s", buf);
	sprintf(buf, "SETBUF %f %f\n", 0.1, 0.15);
	write(s, buf, strlen(buf));
	len = read(s, buf, 256);			// buf[len] = '\0'; printf("%s", buf); 

	sprintf(buf, "OPEN lsb\n");
	write(s, buf, strlen(buf));
	len = read(s, buf, 256);			// buf[len] = '\0'; printf("%s", buf); 

	sscanf(buf, "%d %d", &dummy, &newport);
	printf("	 => port = %d\n", newport);

	s2 = connect_to_timidity(newport);
	_mo = (void *)s2;
}

void MidiDriver::MidiOutSeq(void *a, int b)
{
	int s = (int)a;
	unsigned char buf[256];
	int position = 0;

	switch (b & 0xF0) {
	case 0x80:
	case 0x90:
	case 0xA0:
	case 0xB0:
	case 0xE0:
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = b;
		buf[position++] = DEVICE_NUM;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (b >> 8) & 0x7F;
		buf[position++] = DEVICE_NUM;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (b >> 16) & 0x7F;
		buf[position++] = DEVICE_NUM;
		buf[position++] = 0;
		break;
	case 0xC0:
	case 0xD0:
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = b;
		buf[position++] = DEVICE_NUM;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (b >> 8) & 0x7F;
		buf[position++] = DEVICE_NUM;
		buf[position++] = 0;
		break;
	default:
		fprintf(stderr, "Unknown : %08x\n", b);
		break;
	}
	write(s, buf, position);
}

/* Quicktime music support */
void MidiDriver::midiInitQuicktime()
{
#ifdef __APPLE__CW
	ComponentResult qtErr = noErr;
	qtNoteAllocator = NULL;

	for (int i = 0; i < 15; i++)
		qtNoteChannel[i] = NULL;

	qtNoteAllocator = OpenDefaultComponent(kNoteAllocatorComponentType, 0);
	if (qtNoteAllocator == NULL)
		goto bail;

	simpleNoteRequest.info.flags = 0;
	*(short *)(&simpleNoteRequest.info.polyphony) = EndianS16_NtoB(15);	// simultaneous tones
	*(Fixed *) (&simpleNoteRequest.info.typicalPolyphony) =
		EndianU32_NtoB(0x00010000);

	qtErr = NAStuffToneDescription(qtNoteAllocator, 1, &simpleNoteRequest.tone);
	if (qtErr != noErr)
		goto bail;

	for (int i = 0; i < 15; i++) {
		qtErr =
			NANewNoteChannel(qtNoteAllocator, &simpleNoteRequest,
											 &(qtNoteChannel[i]));
		if ((qtErr != noErr) || (qtNoteChannel == NULL))
			goto bail;
	}
	return;

bail:
	fprintf(stderr, "Init QT failed %x %x %d\n", qtNoteAllocator, qtNoteChannel,
					qtErr);
	for (int i = 0; i < 15; i++) {
		if (qtNoteChannel[i] != NULL)
			NADisposeNoteChannel(qtNoteAllocator, qtNoteChannel[i]);
	}

	if (qtNoteAllocator != NULL)
		CloseComponent(qtNoteAllocator);
#endif
}

void MidiDriver::MidiOutQuicktime(void *a, int b)
{
#ifdef __APPLE__CW
	MusicMIDIPacket midPacket;
	unsigned char *midiCmd = midPacket.data;
	midPacket.length = 3;
	midiCmd[3] = (b & 0xFF000000) >> 24;
	midiCmd[2] = (b & 0x00FF0000) >> 16;
	midiCmd[1] = (b & 0x0000FF00) >> 8;
	midiCmd[0] = b;

	unsigned char chanID = midiCmd[0] & 0x0F;
	switch (midiCmd[0] & 0xF0) {
	case 0x80:										// Note off
		NAPlayNote(qtNoteAllocator, qtNoteChannel[chanID], midiCmd[1], 0);
		break;

	case 0x90:										// Note on
		NAPlayNote(qtNoteAllocator, qtNoteChannel[chanID], midiCmd[1],
							 midiCmd[2]);
		break;

	case 0xB0:										// Effect
		switch (midiCmd[1]) {
		case 0x01:									// Modulation
			NASetController(qtNoteAllocator, qtNoteChannel[chanID],
											kControllerModulationWheel, midiCmd[2] << 8);
			break;

		case 0x07:									// Volume
			NASetController(qtNoteAllocator, qtNoteChannel[chanID],
											kControllerVolume, midiCmd[2] * 300);
			break;

		case 0x0A:									// Pan
			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerPan,
											(midiCmd[2] << 1) + 0xFF);
			break;

		case 0x40:									// Sustain on/off
			NASetController(qtNoteAllocator, qtNoteChannel[chanID],
											kControllerSustain, midiCmd[2]);
			break;

		case 0x5b:									// ext effect depth
			NASetController(qtNoteAllocator, qtNoteChannel[chanID],
											kControllerReverb, midiCmd[2] << 8);
			break;

		case 0x5d:									// chorus depth
			NASetController(qtNoteAllocator, qtNoteChannel[chanID],
											kControllerChorus, midiCmd[2] << 8);
			break;

		case 0x7b:									// mode message all notes off
			for (int i = 0; i < 128; i++)
				NAPlayNote(qtNoteAllocator, qtNoteChannel[chanID], i, 0);
			break;

		default:
			fprintf(stderr, "Unknown MIDI effect: %08x\n", b);
			break;
		}
		break;

	case 0xC0:										// Program change
		NASetInstrumentNumber(qtNoteAllocator, qtNoteChannel[chanID], midiCmd[1]);
		break;

	case 0xE0:{									// Pitch bend
			long theBend =
				((((long)midiCmd[1] + (long)(midiCmd[2] << 8))) - 0x4000) / 4;
			NASetController(qtNoteAllocator, qtNoteChannel[chanID],
											kControllerPitchBend, theBend);
		}
		break;

	default:
		fprintf(stderr, "Unknown Command: %08x\n", b);
		NASendMIDI(qtNoteAllocator, qtNoteChannel[chanID], &midPacket);
		break;
	}
#endif
}

/*********** MorphOS            */
void MidiDriver::MidiOutMorphOS(void *a, int b)
{
#ifdef __MORPHOS__
	if (ScummMidiRequest) {
		ULONG midi_data = b;				// you never know about an int's size ;-)
		ScummMidiRequest->amr_Std.io_Command = CMD_WRITE;
		ScummMidiRequest->amr_Std.io_Data = &midi_data;
		ScummMidiRequest->amr_Std.io_Length = 4;
		DoIO((struct IORequest *)ScummMidiRequest);
	}
#endif
}



void MidiDriver::midiInitNull()
{
	warning
		("Music not enabled - MIDI support selected with no MIDI driver available. Try Adlib");
}
#endif