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
 * EmuMIDI support by Lionel Ulmer <lionel.ulmer@free.fr> copied from
 *         AdPlug (http://adplug.sourceforge.net)
 * Timidity support by Lionel Ulmer <lionel.ulmer@free.fr>
 * QuickTime support by Florent Boudet <flobo@ifrance.com>
 * Raw output support by Michael Pearce
 * MorphOS support by Ruediger Hanke 
 */

#include "stdafx.h"
#include "scumm.h"
#include "mididrv.h"
#include "fmopl.h"
#include "mixer.h"

#if defined(WIN32) && !defined(_WIN32_WCE)

/* Windows MIDI driver */
class MidiDriver_WIN : public MidiDriver {
public:
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

	//printMidi(u.bData[0], u.bData[1], u.bData[2], u.bData[3]);
	check_error(midiOutShortMsg(_mo, u.dwData));
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

#ifdef __MORPHOS__
#include <exec/types.h>
#include <devices/amidi.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>

extern struct IOMidiRequest *ScummMidiRequest;

/* MorphOS MIDI driver */
class MidiDriver_AMIDI : public MidiDriver {
public:
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool pause);
	void set_stream_callback(void *param, StreamCallback *sc);

private:
	StreamCallback *_stream_proc;
	void  *_stream_param;
	int 	 _mode;
};

void MidiDriver_AMIDI::set_stream_callback(void *param, StreamCallback *sc) {
	_stream_param = param;
	_stream_proc = sc;
}

int MidiDriver_AMIDI::open(int mode) {
	_mode = mode;
	return 0;
}

void MidiDriver_AMIDI::close() {
	_mode = 0;
}

void MidiDriver_AMIDI::send(uint32 b) {
	if (_mode != MO_SIMPLE)
		error("MidiDriver_AMIDI:send called but driver is not in simple mode");

	if (ScummMidiRequest) {
		ULONG midi_data = b;				// you never know about an int's size ;-)
		ScummMidiRequest->amr_Std.io_Command = CMD_WRITE;
		ScummMidiRequest->amr_Std.io_Data = &midi_data;
		ScummMidiRequest->amr_Std.io_Length = 4;
		DoIO((struct IORequest *)ScummMidiRequest);
	}
}

void MidiDriver_AMIDI::pause(bool pause) {
	if (_mode == MO_STREAMING) {
	}
}

MidiDriver *MidiDriver_AMIDI_create() {
	return new MidiDriver_AMIDI();
}

#endif // __MORPHOS__

#if defined(UNIX) && !defined(__BEOS__)
#define SEQ_MIDIPUTC    5
#define SPECIAL_CHANNEL 9
#define DEVICE_NUM 1

class MidiDriver_SEQ : public MidiDriver {
public:
        MidiDriver_SEQ();
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool pause);
	void set_stream_callback(void *param, StreamCallback *sc);

private:
	StreamCallback *_stream_proc;
	void  *_stream_param;
	int 	 _mode;
	int device;
};

MidiDriver_SEQ::MidiDriver_SEQ(){
  _mode=0;
  device=0;
}

int MidiDriver_SEQ::open(int mode) {
        if (_mode != 0)
		return MERR_ALREADY_OPEN;
	device = 0;
	_mode=mode;
	if (mode!=MO_SIMPLE) return MERR_STREAMING_NOT_AVAILABLE;

	char *device_name = getenv("SCUMMVM_MIDI");
	if (device_name != NULL) {
		device = (::open((device_name), O_RDWR, 0));
	} else {
		warning("You need to set-up the SCUMMVM_MIDI environment variable properly (see readme.txt) ");
	}
	if ((device_name == NULL) || (device < 0)) {
		if (device_name == NULL)
			warning("Opening /dev/null (no music will be heard) ");
		else
			warning("Cannot open rawmidi device %s - using /dev/null (no music will be heard) ", device_name);
		device = (::open(("/dev/null"), O_RDWR, 0));
		if (device < 0)
			error("Cannot open /dev/null to dump midi output");
	}

	return 0;
}

void MidiDriver_SEQ::close() {
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
		buf[position++] = DEVICE_NUM;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)((b >> 8) & 0x7F);
		buf[position++] = DEVICE_NUM;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)((b >> 16) & 0x7F);
		buf[position++] = DEVICE_NUM;
		buf[position++] = 0;
		break;
	case 0xC0:
	case 0xD0:
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)b;
		buf[position++] = DEVICE_NUM;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)((b >> 8) & 0x7F);
		buf[position++] = DEVICE_NUM;
		buf[position++] = 0;
		break;
	default:
		fprintf(stderr, "Unknown : %08x\n", (int) b);
		break;
	}
	write(device, buf, position);
}

void MidiDriver_SEQ::pause(bool pause) {
	if (_mode == MO_STREAMING) {
	}
}

void MidiDriver_SEQ::set_stream_callback(void *param, StreamCallback *sc) {
	_stream_param = param;
	_stream_proc = sc;
}

MidiDriver *MidiDriver_SEQ_create() {
	return new MidiDriver_SEQ();
}

#endif

#if defined(__APPLE__) || defined(macintosh)
// FIXME - this is for Mac OS X and Mac OS 9. It's not really possible
// to check for these *cleanly* without a configure script, though..


#if defined(MACOSX)
 // On MacOSX, these are in a frameworks!
 #include <QuickTime/QuickTimeComponents.h>
 #include <QuickTime/QuickTimeMusic.h>
#else
 #include <QuickTimeComponents.h>
 #include <QuickTimeMusic.h>
#endif


/* QuickTime MIDI driver */
class MidiDriver_QT : public MidiDriver {
public:
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool pause);
	void set_stream_callback(void *param, StreamCallback *sc);

private:
	NoteAllocator qtNoteAllocator;
	NoteChannel qtNoteChannel[16];
	NoteRequest simpleNoteRequest;

	StreamCallback *_stream_proc;
	void  *_stream_param;
	int 	 _mode;
};

void MidiDriver_QT::set_stream_callback(void *param, StreamCallback *sc) {
	_stream_param = param;
	_stream_proc = sc;
}

int MidiDriver_QT::open(int mode) {
	ComponentResult qtErr = noErr;
	qtNoteAllocator = NULL;

	if (mode == MO_STREAMING)
		return MERR_STREAMING_NOT_AVAILABLE;

	_mode = mode;

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
	return 0;

bail:
	error("Init QT failed %x %x %d\n", (int)qtNoteAllocator, (int)qtNoteChannel,
					(int)qtErr);
	for (int i = 0; i < 15; i++) {
		if (qtNoteChannel[i] != NULL)
			NADisposeNoteChannel(qtNoteAllocator, qtNoteChannel[i]);
		qtNoteChannel[i] = NULL;
	}

	if (qtNoteAllocator != NULL) {
		CloseComponent(qtNoteAllocator);
		qtNoteAllocator = NULL;
	}

	return MERR_DEVICE_NOT_AVAILABLE;
}

void MidiDriver_QT::close() {
	_mode = 0;

	for (int i = 0; i < 15; i++) {
		if (qtNoteChannel[i] != NULL)
			NADisposeNoteChannel(qtNoteAllocator, qtNoteChannel[i]);
		qtNoteChannel[i] = NULL;
	}

	if (qtNoteAllocator != NULL) {
		CloseComponent(qtNoteAllocator);
		qtNoteAllocator = NULL;
	}
}

void MidiDriver_QT::send(uint32 b) {
	if (_mode != MO_SIMPLE)
		error("MidiDriver_QT:send called but driver is not in simple mode");

	MusicMIDIPacket midPacket;
	unsigned char *midiCmd = midPacket.data;
	midPacket.length = 3;
	midiCmd[3] = (b & 0xFF000000) >> 24;
	midiCmd[2] = (b & 0x00FF0000) >> 16;
	midiCmd[1] = (b & 0x0000FF00) >> 8;
	midiCmd[0] = (b & 0x000000FF);

	unsigned char chanID = midiCmd[0] & 0x0F;
	switch (midiCmd[0] & 0xF0) {
	case 0x80:										// Note off
		NAPlayNote(qtNoteAllocator, qtNoteChannel[chanID], midiCmd[1], 0);
		break;

	case 0x90:										// Note on
		NAPlayNote(qtNoteAllocator, qtNoteChannel[chanID], midiCmd[1], midiCmd[2]);
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
			error("Unknown MIDI effect: %08x\n", (int)b);
			break;
		}
		break;

	case 0xC0:										// Program change
		NASetInstrumentNumber(qtNoteAllocator, qtNoteChannel[chanID], midiCmd[1] + 1);
		break;

	case 0xE0:{									// Pitch bend
			long theBend =
				((((long)midiCmd[1] + (long)(midiCmd[2] << 8))) - 0x4000) / 4;
			NASetController(qtNoteAllocator, qtNoteChannel[chanID],
											kControllerPitchBend, theBend);
		}
		break;

	default:
		error("Unknown Command: %08x\n", (int)b);
		NASendMIDI(qtNoteAllocator, qtNoteChannel[chanID], &midPacket);
		break;
	}
}

void MidiDriver_QT::pause(bool pause) {
}

MidiDriver *MidiDriver_QT_create() {
	return new MidiDriver_QT();
}

#endif // __APPLE__ || macintosh


#ifdef __APPLE__

#include <AudioUnit/AudioUnit.h>

/* CoreAudio MIDI driver */
/* Based on code by Benjamin W. Zale */
class MidiDriver_CORE : public MidiDriver {
public:
	MidiDriver_CORE() : au_MusicDevice(NULL), au_output(NULL) {}

	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool pause);
	void set_stream_callback(void *param, StreamCallback *sc);

private:
	AudioUnit au_MusicDevice;
	AudioUnit au_output;

	StreamCallback *_stream_proc;
	void *_stream_param;
	int _mode;
};


void MidiDriver_CORE::set_stream_callback(void *param, StreamCallback *sc) {
	_stream_param = param;
	_stream_proc = sc;
}

int MidiDriver_CORE::open(int mode) {

	if (au_output != NULL)
		return MERR_ALREADY_OPEN;

	_mode = mode;
	
	int err;
	struct AudioUnitConnection	auconnect;
	ComponentDescription compdesc;
	Component compid;
	
	// Open the Music Device
	compdesc.componentType = kAudioUnitComponentType;
	compdesc.componentSubType = kAudioUnitSubType_MusicDevice;
	compdesc.componentManufacturer = kAudioUnitID_DLSSynth;
	compdesc.componentFlags = 0;
	compdesc.componentFlagsMask = 0;
	compid = FindNextComponent(NULL,&compdesc);
	au_MusicDevice = (AudioUnit)OpenComponent(compid);
	
	// open the output unit
	au_output = (AudioUnit)OpenDefaultComponent(kAudioUnitComponentType,kAudioUnitSubType_Output);

	// connect the units
	auconnect.sourceAudioUnit = au_MusicDevice;
	auconnect.sourceOutputNumber = 0;
	auconnect.destInputNumber = 0;
	err = AudioUnitSetProperty(au_output,kAudioUnitProperty_MakeConnection,kAudioUnitScope_Input,0,(void*)&auconnect,sizeof(struct AudioUnitConnection));

	// initialize the units
	AudioUnitInitialize(au_MusicDevice);
	AudioUnitInitialize(au_output);
	
	// start the output
	AudioOutputUnitStart(au_output);
	
	return 0;
}

void MidiDriver_CORE::close() {

	// Stop the output
	AudioOutputUnitStop(au_output);
	
	// Cleanup
	CloseComponent(au_output);
	CloseComponent(au_MusicDevice);

	_mode = 0;
}

void MidiDriver_CORE::send(uint32 b) {
	if (_mode != MO_SIMPLE)
		error("MidiDriver_CORE:send called but driver is not in simple mode");

	unsigned char first_byte, seccond_byte, status_byte;
	status_byte   = (b & 0x000000FF);
	first_byte    = (b & 0x0000FF00) >> 8;
	seccond_byte  = (b & 0x00FF0000) >> 16;
	MusicDeviceMIDIEvent(au_MusicDevice, status_byte, first_byte, seccond_byte, 0);
}

void MidiDriver_CORE::pause(bool pause) {
}

MidiDriver *MidiDriver_CORE_create() {
	return new MidiDriver_CORE();
}

#endif // __APPLE__

/* NULL driver */
class MidiDriver_NULL : public MidiDriver {
public:
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool pause);
	void set_stream_callback(void *param, StreamCallback *sc);
private:
};

int MidiDriver_NULL::open(int mode) {
	warning("Music not enabled - MIDI support selected with no MIDI driver available. Try Adlib");
	return 0;
}
void MidiDriver_NULL::close() {}
void MidiDriver_NULL::send(uint32 b) {}
void MidiDriver_NULL::pause(bool pause) {}
void MidiDriver_NULL::set_stream_callback(void *param, StreamCallback *sc) {}

MidiDriver *MidiDriver_NULL_create() {
	return new MidiDriver_NULL();
}



/* Default (empty) property method */
uint32 MidiDriver::property(int prop, uint32 param)
{
	return 0;
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

/* This is the internal emulated MIDI driver using the included OPL2 sound chip 
 * FM instrument definitions below borrowed from the Allegro library by
 * Phil Hassey, <philhassey@hotmail.com> (www.imitationpickles.org)
 */
 
static unsigned char midi_fm_instruments_table[128][14] =
{

   /* This set of GM instrument patches was provided by Jorrit Rouwe...
    */

   { 0x21, 0x21, 0x8f, 0x0c, 0xf2, 0xf2, 0x45, 0x76, 0x00, 0x00, 0x08, 0, 0, 0 }, /* Acoustic Grand */
   { 0x31, 0x21, 0x4b, 0x09, 0xf2, 0xf2, 0x54, 0x56, 0x00, 0x00, 0x08, 0, 0, 0 }, /* Bright Acoustic */
   { 0x31, 0x21, 0x49, 0x09, 0xf2, 0xf2, 0x55, 0x76, 0x00, 0x00, 0x08, 0, 0, 0 }, /* Electric Grand */
   { 0xb1, 0x61, 0x0e, 0x09, 0xf2, 0xf3, 0x3b, 0x0b, 0x00, 0x00, 0x06, 0, 0, 0 }, /* Honky-Tonk */
   { 0x01, 0x21, 0x57, 0x09, 0xf1, 0xf1, 0x38, 0x28, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Electric Piano 1 */
   { 0x01, 0x21, 0x93, 0x09, 0xf1, 0xf1, 0x38, 0x28, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Electric Piano 2 */
   { 0x21, 0x36, 0x80, 0x17, 0xa2, 0xf1, 0x01, 0xd5, 0x00, 0x00, 0x08, 0, 0, 0 }, /* Harpsichord */
   { 0x01, 0x01, 0x92, 0x09, 0xc2, 0xc2, 0xa8, 0x58, 0x00, 0x00, 0x0a, 0, 0, 0 }, /* Clav */
   { 0x0c, 0x81, 0x5c, 0x09, 0xf6, 0xf3, 0x54, 0xb5, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Celesta */
   { 0x07, 0x11, 0x97, 0x89, 0xf6, 0xf5, 0x32, 0x11, 0x00, 0x00, 0x02, 0, 0, 0 }, /* Glockenspiel */
   { 0x17, 0x01, 0x21, 0x09, 0x56, 0xf6, 0x04, 0x04, 0x00, 0x00, 0x02, 0, 0, 0 }, /* Music Box */
   { 0x18, 0x81, 0x62, 0x09, 0xf3, 0xf2, 0xe6, 0xf6, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Vibraphone */
   { 0x18, 0x21, 0x23, 0x09, 0xf7, 0xe5, 0x55, 0xd8, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Marimba */
   { 0x15, 0x01, 0x91, 0x09, 0xf6, 0xf6, 0xa6, 0xe6, 0x00, 0x00, 0x04, 0, 0, 0 }, /* Xylophone */
   { 0x45, 0x81, 0x59, 0x89, 0xd3, 0xa3, 0x82, 0xe3, 0x00, 0x00, 0x0c, 0, 0, 0 }, /* Tubular Bells */
   { 0x03, 0x81, 0x49, 0x89, 0x74, 0xb3, 0x55, 0x05, 0x01, 0x00, 0x04, 0, 0, 0 }, /* Dulcimer */
   { 0x71, 0x31, 0x92, 0x09, 0xf6, 0xf1, 0x14, 0x07, 0x00, 0x00, 0x02, 0, 0, 0 }, /* Drawbar Organ */
   { 0x72, 0x30, 0x14, 0x09, 0xc7, 0xc7, 0x58, 0x08, 0x00, 0x00, 0x02, 0, 0, 0 }, /* Percussive Organ */
   { 0x70, 0xb1, 0x44, 0x09, 0xaa, 0x8a, 0x18, 0x08, 0x00, 0x00, 0x04, 0, 0, 0 }, /* Rock Organ */
   { 0x23, 0xb1, 0x93, 0x09, 0x97, 0x55, 0x23, 0x14, 0x01, 0x00, 0x04, 0, 0, 0 }, /* Church Organ */
   { 0x61, 0xb1, 0x13, 0x89, 0x97, 0x55, 0x04, 0x04, 0x01, 0x00, 0x00, 0, 0, 0 }, /* Reed Organ */
   { 0x24, 0xb1, 0x48, 0x09, 0x98, 0x46, 0x2a, 0x1a, 0x01, 0x00, 0x0c, 0, 0, 0 }, /* Accoridan */
   { 0x61, 0x21, 0x13, 0x09, 0x91, 0x61, 0x06, 0x07, 0x01, 0x00, 0x0a, 0, 0, 0 }, /* Harmonica */
   { 0x21, 0xa1, 0x13, 0x92, 0x71, 0x61, 0x06, 0x07, 0x00, 0x00, 0x06, 0, 0, 0 }, /* Tango Accordian */
   { 0x02, 0x41, 0x9c, 0x89, 0xf3, 0xf3, 0x94, 0xc8, 0x01, 0x00, 0x0c, 0, 0, 0 }, /* Acoustic Guitar(nylon) */
   { 0x03, 0x11, 0x54, 0x09, 0xf3, 0xf1, 0x9a, 0xe7, 0x01, 0x00, 0x0c, 0, 0, 0 }, /* Acoustic Guitar(steel) */
   { 0x23, 0x21, 0x5f, 0x09, 0xf1, 0xf2, 0x3a, 0xf8, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Electric Guitar(jazz) */
   { 0x03, 0x21, 0x87, 0x89, 0xf6, 0xf3, 0x22, 0xf8, 0x01, 0x00, 0x06, 0, 0, 0 }, /* Electric Guitar(clean) */
   { 0x03, 0x21, 0x47, 0x09, 0xf9, 0xf6, 0x54, 0x3a, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Electric Guitar(muted) */
   { 0x23, 0x21, 0x4a, 0x0e, 0x91, 0x84, 0x41, 0x19, 0x01, 0x00, 0x08, 0, 0, 0 }, /* Overdriven Guitar */
   { 0x23, 0x21, 0x4a, 0x09, 0x95, 0x94, 0x19, 0x19, 0x01, 0x00, 0x08, 0, 0, 0 }, /* Distortion Guitar */
   { 0x09, 0x84, 0xa1, 0x89, 0x20, 0xd1, 0x4f, 0xf8, 0x00, 0x00, 0x08, 0, 0, 0 }, /* Guitar Harmonics */
   { 0x21, 0xa2, 0x1e, 0x09, 0x94, 0xc3, 0x06, 0xa6, 0x00, 0x00, 0x02, 0, 0, 0 }, /* Acoustic Bass */
   { 0x31, 0x31, 0x12, 0x09, 0xf1, 0xf1, 0x28, 0x18, 0x00, 0x00, 0x0a, 0, 0, 0 }, /* Electric Bass(finger) */
   { 0x31, 0x31, 0x8d, 0x09, 0xf1, 0xf1, 0xe8, 0x78, 0x00, 0x00, 0x0a, 0, 0, 0 }, /* Electric Bass(pick) */
   { 0x31, 0x32, 0x5b, 0x09, 0x51, 0x71, 0x28, 0x48, 0x00, 0x00, 0x0c, 0, 0, 0 }, /* Fretless Bass */
   { 0x01, 0x21, 0x8b, 0x49, 0xa1, 0xf2, 0x9a, 0xdf, 0x00, 0x00, 0x08, 0, 0, 0 }, /* Slap Bass 1 */
   { 0x21, 0x21, 0x8b, 0x11, 0xa2, 0xa1, 0x16, 0xdf, 0x00, 0x00, 0x08, 0, 0, 0 }, /* Slap Bass 2 */
   { 0x31, 0x31, 0x8b, 0x09, 0xf4, 0xf1, 0xe8, 0x78, 0x00, 0x00, 0x0a, 0, 0, 0 }, /* Synth Bass 1 */
   { 0x31, 0x31, 0x12, 0x09, 0xf1, 0xf1, 0x28, 0x18, 0x00, 0x00, 0x0a, 0, 0, 0 }, /* Synth Bass 2 */
   { 0x31, 0x21, 0x15, 0x09, 0xdd, 0x56, 0x13, 0x26, 0x01, 0x00, 0x08, 0, 0, 0 }, /* Violin */
   { 0x31, 0x21, 0x16, 0x09, 0xdd, 0x66, 0x13, 0x06, 0x01, 0x00, 0x08, 0, 0, 0 }, /* Viola */
   { 0x71, 0x31, 0x49, 0x09, 0xd1, 0x61, 0x1c, 0x0c, 0x01, 0x00, 0x08, 0, 0, 0 }, /* Cello */
   { 0x21, 0x23, 0x4d, 0x89, 0x71, 0x72, 0x12, 0x06, 0x01, 0x00, 0x02, 0, 0, 0 }, /* Contrabass */
   { 0xf1, 0xe1, 0x40, 0x09, 0xf1, 0x6f, 0x21, 0x16, 0x01, 0x00, 0x02, 0, 0, 0 }, /* Tremolo Strings */
   { 0x02, 0x01, 0x1a, 0x89, 0xf5, 0x85, 0x75, 0x35, 0x01, 0x00, 0x00, 0, 0, 0 }, /* Pizzicato Strings */
   { 0x02, 0x01, 0x1d, 0x89, 0xf5, 0xf3, 0x75, 0xf4, 0x01, 0x00, 0x00, 0, 0, 0 }, /* Orchestral Strings */
   { 0x10, 0x11, 0x41, 0x09, 0xf5, 0xf2, 0x05, 0xc3, 0x01, 0x00, 0x02, 0, 0, 0 }, /* Timpani */
   { 0x21, 0xa2, 0x9b, 0x0a, 0xb1, 0x72, 0x25, 0x08, 0x01, 0x00, 0x0e, 0, 0, 0 }, /* String Ensemble 1 */
   { 0xa1, 0x21, 0x98, 0x09, 0x7f, 0x3f, 0x03, 0x07, 0x01, 0x01, 0x00, 0, 0, 0 }, /* String Ensemble 2 */
   { 0xa1, 0x61, 0x93, 0x09, 0xc1, 0x4f, 0x12, 0x05, 0x00, 0x00, 0x0a, 0, 0, 0 }, /* SynthStrings 1 */
   { 0x21, 0x61, 0x18, 0x09, 0xc1, 0x4f, 0x22, 0x05, 0x00, 0x00, 0x0c, 0, 0, 0 }, /* SynthStrings 2 */
   { 0x31, 0x72, 0x5b, 0x8c, 0xf4, 0x8a, 0x15, 0x05, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Choir Aahs */
   { 0xa1, 0x61, 0x90, 0x09, 0x74, 0x71, 0x39, 0x67, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Voice Oohs */
   { 0x71, 0x72, 0x57, 0x09, 0x54, 0x7a, 0x05, 0x05, 0x00, 0x00, 0x0c, 0, 0, 0 }, /* Synth Voice */
   { 0x90, 0x41, 0x00, 0x09, 0x54, 0xa5, 0x63, 0x45, 0x00, 0x00, 0x08, 0, 0, 0 }, /* Orchestra Hit */
   { 0x21, 0x21, 0x92, 0x0a, 0x85, 0x8f, 0x17, 0x09, 0x00, 0x00, 0x0c, 0, 0, 0 }, /* Trumpet */
   { 0x21, 0x21, 0x94, 0x0e, 0x75, 0x8f, 0x17, 0x09, 0x00, 0x00, 0x0c, 0, 0, 0 }, /* Trombone */
   { 0x21, 0x61, 0x94, 0x09, 0x76, 0x82, 0x15, 0x37, 0x00, 0x00, 0x0c, 0, 0, 0 }, /* Tuba */
   { 0x31, 0x21, 0x43, 0x09, 0x9e, 0x62, 0x17, 0x2c, 0x01, 0x01, 0x02, 0, 0, 0 }, /* Muted Trumpet */
   { 0x21, 0x21, 0x9b, 0x09, 0x61, 0x7f, 0x6a, 0x0a, 0x00, 0x00, 0x02, 0, 0, 0 }, /* French Horn */
   { 0x61, 0x22, 0x8a, 0x0f, 0x75, 0x74, 0x1f, 0x0f, 0x00, 0x00, 0x08, 0, 0, 0 }, /* Brass Section */
   { 0xa1, 0x21, 0x86, 0x8c, 0x72, 0x71, 0x55, 0x18, 0x01, 0x00, 0x00, 0, 0, 0 }, /* SynthBrass 1 */
   { 0x21, 0x21, 0x4d, 0x09, 0x54, 0xa6, 0x3c, 0x1c, 0x00, 0x00, 0x08, 0, 0, 0 }, /* SynthBrass 2 */
   { 0x31, 0x61, 0x8f, 0x09, 0x93, 0x72, 0x02, 0x0b, 0x01, 0x00, 0x08, 0, 0, 0 }, /* Soprano Sax */
   { 0x31, 0x61, 0x8e, 0x09, 0x93, 0x72, 0x03, 0x09, 0x01, 0x00, 0x08, 0, 0, 0 }, /* Alto Sax */
   { 0x31, 0x61, 0x91, 0x09, 0x93, 0x82, 0x03, 0x09, 0x01, 0x00, 0x0a, 0, 0, 0 }, /* Tenor Sax */
   { 0x31, 0x61, 0x8e, 0x09, 0x93, 0x72, 0x0f, 0x0f, 0x01, 0x00, 0x0a, 0, 0, 0 }, /* Baritone Sax */
   { 0x21, 0x21, 0x4b, 0x09, 0xaa, 0x8f, 0x16, 0x0a, 0x01, 0x00, 0x08, 0, 0, 0 }, /* Oboe */
   { 0x31, 0x21, 0x90, 0x09, 0x7e, 0x8b, 0x17, 0x0c, 0x01, 0x01, 0x06, 0, 0, 0 }, /* English Horn */
   { 0x31, 0x32, 0x81, 0x09, 0x75, 0x61, 0x19, 0x19, 0x01, 0x00, 0x00, 0, 0, 0 }, /* Bassoon */
   { 0x32, 0x21, 0x90, 0x09, 0x9b, 0x72, 0x21, 0x17, 0x00, 0x00, 0x04, 0, 0, 0 }, /* Clarinet */
   { 0xe1, 0xe1, 0x1f, 0x09, 0x85, 0x65, 0x5f, 0x1a, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Piccolo */
   { 0xe1, 0xe1, 0x46, 0x09, 0x88, 0x65, 0x5f, 0x1a, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Flute */
   { 0xa1, 0x21, 0x9c, 0x09, 0x75, 0x75, 0x1f, 0x0a, 0x00, 0x00, 0x02, 0, 0, 0 }, /* Recorder */
   { 0x31, 0x21, 0x8b, 0x09, 0x84, 0x65, 0x58, 0x1a, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Pan Flute */
   { 0xe1, 0xa1, 0x4c, 0x09, 0x66, 0x65, 0x56, 0x26, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Blown Bottle */
   { 0x62, 0xa1, 0xcb, 0x09, 0x76, 0x55, 0x46, 0x36, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Skakuhachi */
   { 0x62, 0xa1, 0xa2, 0x09, 0x57, 0x56, 0x07, 0x07, 0x00, 0x00, 0x0b, 0, 0, 0 }, /* Whistle */
   { 0x62, 0xa1, 0x9c, 0x09, 0x77, 0x76, 0x07, 0x07, 0x00, 0x00, 0x0b, 0, 0, 0 }, /* Ocarina */
   { 0x22, 0x21, 0x59, 0x09, 0xff, 0xff, 0x03, 0x0f, 0x02, 0x00, 0x00, 0, 0, 0 }, /* Lead 1 (square) */
   { 0x21, 0x21, 0x0e, 0x09, 0xff, 0xff, 0x0f, 0x0f, 0x01, 0x01, 0x00, 0, 0, 0 }, /* Lead 2 (sawtooth) */
   { 0x22, 0x21, 0x46, 0x89, 0x86, 0x64, 0x55, 0x18, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Lead 3 (calliope) */
   { 0x21, 0xa1, 0x45, 0x09, 0x66, 0x96, 0x12, 0x0a, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Lead 4 (chiff) */
   { 0x21, 0x22, 0x8b, 0x09, 0x92, 0x91, 0x2a, 0x2a, 0x01, 0x00, 0x00, 0, 0, 0 }, /* Lead 5 (charang) */
   { 0xa2, 0x61, 0x9e, 0x49, 0xdf, 0x6f, 0x05, 0x07, 0x00, 0x00, 0x02, 0, 0, 0 }, /* Lead 6 (voice) */
   { 0x20, 0x60, 0x1a, 0x09, 0xef, 0x8f, 0x01, 0x06, 0x00, 0x02, 0x00, 0, 0, 0 }, /* Lead 7 (fifths) */
   { 0x21, 0x21, 0x8f, 0x86, 0xf1, 0xf4, 0x29, 0x09, 0x00, 0x00, 0x0a, 0, 0, 0 }, /* Lead 8 (bass+lead) */
   { 0x77, 0xa1, 0xa5, 0x09, 0x53, 0xa0, 0x94, 0x05, 0x00, 0x00, 0x02, 0, 0, 0 }, /* Pad 1 (new age) */
   { 0x61, 0xb1, 0x1f, 0x89, 0xa8, 0x25, 0x11, 0x03, 0x00, 0x00, 0x0a, 0, 0, 0 }, /* Pad 2 (warm) */
   { 0x61, 0x61, 0x17, 0x09, 0x91, 0x55, 0x34, 0x16, 0x00, 0x00, 0x0c, 0, 0, 0 }, /* Pad 3 (polysynth) */
   { 0x71, 0x72, 0x5d, 0x09, 0x54, 0x6a, 0x01, 0x03, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Pad 4 (choir) */
   { 0x21, 0xa2, 0x97, 0x09, 0x21, 0x42, 0x43, 0x35, 0x00, 0x00, 0x08, 0, 0, 0 }, /* Pad 5 (bowed) */
   { 0xa1, 0x21, 0x1c, 0x09, 0xa1, 0x31, 0x77, 0x47, 0x01, 0x01, 0x00, 0, 0, 0 }, /* Pad 6 (metallic) */
   { 0x21, 0x61, 0x89, 0x0c, 0x11, 0x42, 0x33, 0x25, 0x00, 0x00, 0x0a, 0, 0, 0 }, /* Pad 7 (halo) */
   { 0xa1, 0x21, 0x15, 0x09, 0x11, 0xcf, 0x47, 0x07, 0x01, 0x00, 0x00, 0, 0, 0 }, /* Pad 8 (sweep) */
   { 0x3a, 0x51, 0xce, 0x09, 0xf8, 0x86, 0xf6, 0x02, 0x00, 0x00, 0x02, 0, 0, 0 }, /* FX 1 (rain) */
   { 0x21, 0x21, 0x15, 0x09, 0x21, 0x41, 0x23, 0x13, 0x01, 0x00, 0x00, 0, 0, 0 }, /* FX 2 (soundtrack) */
   { 0x06, 0x01, 0x5b, 0x09, 0x74, 0xa5, 0x95, 0x72, 0x00, 0x00, 0x00, 0, 0, 0 }, /* FX 3 (crystal) */
   { 0x22, 0x61, 0x92, 0x8c, 0xb1, 0xf2, 0x81, 0x26, 0x00, 0x00, 0x0c, 0, 0, 0 }, /* FX 4 (atmosphere) */
   { 0x41, 0x42, 0x4d, 0x09, 0xf1, 0xf2, 0x51, 0xf5, 0x01, 0x00, 0x00, 0, 0, 0 }, /* FX 5 (brightness) */
   { 0x61, 0xa3, 0x94, 0x89, 0x11, 0x11, 0x51, 0x13, 0x01, 0x00, 0x06, 0, 0, 0 }, /* FX 6 (goblins) */
   { 0x61, 0xa1, 0x8c, 0x89, 0x11, 0x1d, 0x31, 0x03, 0x00, 0x00, 0x06, 0, 0, 0 }, /* FX 7 (echoes) */
   { 0xa4, 0x61, 0x4c, 0x09, 0xf3, 0x81, 0x73, 0x23, 0x01, 0x00, 0x04, 0, 0, 0 }, /* FX 8 (sci-fi) */
   { 0x02, 0x07, 0x85, 0x0c, 0xd2, 0xf2, 0x53, 0xf6, 0x00, 0x01, 0x00, 0, 0, 0 }, /* Sitar */
   { 0x11, 0x13, 0x0c, 0x89, 0xa3, 0xa2, 0x11, 0xe5, 0x01, 0x00, 0x00, 0, 0, 0 }, /* Banjo */
   { 0x11, 0x11, 0x06, 0x09, 0xf6, 0xf2, 0x41, 0xe6, 0x01, 0x02, 0x04, 0, 0, 0 }, /* Shamisen */
   { 0x93, 0x91, 0x91, 0x09, 0xd4, 0xeb, 0x32, 0x11, 0x00, 0x01, 0x08, 0, 0, 0 }, /* Koto */
   { 0x04, 0x01, 0x4f, 0x09, 0xfa, 0xc2, 0x56, 0x05, 0x00, 0x00, 0x0c, 0, 0, 0 }, /* Kalimba */
   { 0x21, 0x22, 0x49, 0x09, 0x7c, 0x6f, 0x20, 0x0c, 0x00, 0x01, 0x06, 0, 0, 0 }, /* Bagpipe */
   { 0x31, 0x21, 0x85, 0x09, 0xdd, 0x56, 0x33, 0x16, 0x01, 0x00, 0x0a, 0, 0, 0 }, /* Fiddle */
   { 0x20, 0x21, 0x04, 0x8a, 0xda, 0x8f, 0x05, 0x0b, 0x02, 0x00, 0x06, 0, 0, 0 }, /* Shanai */
   { 0x05, 0x03, 0x6a, 0x89, 0xf1, 0xc3, 0xe5, 0xe5, 0x00, 0x00, 0x06, 0, 0, 0 }, /* Tinkle Bell */
   { 0x07, 0x02, 0x15, 0x09, 0xec, 0xf8, 0x26, 0x16, 0x00, 0x00, 0x0a, 0, 0, 0 }, /* Agogo */
   { 0x05, 0x01, 0x9d, 0x09, 0x67, 0xdf, 0x35, 0x05, 0x00, 0x00, 0x08, 0, 0, 0 }, /* Steel Drums */
   { 0x18, 0x12, 0x96, 0x09, 0xfa, 0xf8, 0x28, 0xe5, 0x00, 0x00, 0x0a, 0, 0, 0 }, /* Woodblock */
   { 0x10, 0x00, 0x86, 0x0c, 0xa8, 0xfa, 0x07, 0x03, 0x00, 0x00, 0x06, 0, 0, 0 }, /* Taiko Drum */
   { 0x11, 0x10, 0x41, 0x0c, 0xf8, 0xf3, 0x47, 0x03, 0x02, 0x00, 0x04, 0, 0, 0 }, /* Melodic Tom */
   { 0x01, 0x10, 0x8e, 0x09, 0xf1, 0xf3, 0x06, 0x02, 0x02, 0x00, 0x0e, 0, 0, 0 }, /* Synth Drum */
   { 0x0e, 0xc0, 0x00, 0x09, 0x1f, 0x1f, 0x00, 0xff, 0x00, 0x03, 0x0e, 0, 0, 0 }, /* Reverse Cymbal */
   { 0x06, 0x03, 0x80, 0x91, 0xf8, 0x56, 0x24, 0x84, 0x00, 0x02, 0x0e, 0, 0, 0 }, /* Guitar Fret Noise */
   { 0x0e, 0xd0, 0x00, 0x0e, 0xf8, 0x34, 0x00, 0x04, 0x00, 0x03, 0x0e, 0, 0, 0 }, /* Breath Noise */
   { 0x0e, 0xc0, 0x00, 0x09, 0xf6, 0x1f, 0x00, 0x02, 0x00, 0x03, 0x0e, 0, 0, 0 }, /* Seashore */
   { 0xd5, 0xda, 0x95, 0x49, 0x37, 0x56, 0xa3, 0x37, 0x00, 0x00, 0x00, 0, 0, 0 }, /* Bird Tweet */
   { 0x35, 0x14, 0x5c, 0x11, 0xb2, 0xf4, 0x61, 0x15, 0x02, 0x00, 0x0a, 0, 0, 0 }, /* Telephone ring */
   { 0x0e, 0xd0, 0x00, 0x09, 0xf6, 0x4f, 0x00, 0xf5, 0x00, 0x03, 0x0e, 0, 0, 0 }, /* Helicopter */
   { 0x26, 0xe4, 0x00, 0x09, 0xff, 0x12, 0x01, 0x16, 0x00, 0x01, 0x0e, 0, 0, 0 }, /* Applause */
   { 0x00, 0x00, 0x00, 0x09, 0xf3, 0xf6, 0xf0, 0xc9, 0x00, 0x02, 0x0e, 0, 0, 0 }  /* Gunshot */

};

/* logarithmic relationship between midi and FM volumes */
static int my_midi_fm_vol_table[128] = {
   0,  11, 16, 19, 22, 25, 27, 29, 32, 33, 35, 37, 39, 40, 42, 43,
   45, 46, 48, 49, 50, 51, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
   64, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 75, 76, 77,
   78, 79, 80, 80, 81, 82, 83, 83, 84, 85, 86, 86, 87, 88, 89, 89,
   90, 91, 91, 92, 93, 93, 94, 95, 96, 96, 97, 97, 98, 99, 99, 100,
   101, 101, 102, 103, 103, 104, 104, 105, 106, 106, 107, 107, 108,
   109, 109, 110, 110, 111, 112, 112, 113, 113, 114, 114, 115, 115,
   116, 117, 117, 118, 118, 119, 119, 120, 120, 121, 121, 122, 122,
   123, 123, 124, 124, 125, 125, 126, 126, 127
};

class MidiDriver_MIDIEMU : public MidiDriver {
public:
        MidiDriver_MIDIEMU();
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool pause);
	void set_stream_callback(void *param, StreamCallback *sc);

private:
	struct midi_channel {
		int inum;
		unsigned char ins[11];
		int vol;
		int nshift;
		int on;
	};

	enum {
		ADLIB_MELODIC = 0,
		ADLIB_RYTHM = 1
	};

	void generate_samples(int16 *buf, uint len);
	static void premix_proc(void *param, int16 *buf, unsigned int len);

	void midi_write_adlib(unsigned int r, unsigned char v);
	void midi_fm_instrument(int voice, unsigned char *inst);
	void midi_fm_volume(int voice, int volume);
	void midi_fm_playnote(int voice, int note, int volume);
	void midi_fm_endnote(int voice);
	unsigned char adlib_data[256];
	int adlib_style;
	int adlib_mode;

	StreamCallback *_stream_proc;
	void *_stream_param;
	int _mode;
	FM_OPL *_opl;
	int chp[9][3];
	unsigned char myinsbank[128][16];
	midi_channel ch[16];
};

MidiDriver_MIDIEMU::MidiDriver_MIDIEMU(){
	int i, j;
	
	_mode = 0;
	for (i = 0; i < 128; i++)
        	for (j = 0; j < 16; j++)
			myinsbank[i][j] = midi_fm_instruments_table[i][j];
	for (i = 0; i < 16; i++) {
		ch[i].inum = 0;
        	for (j = 0; j < 11; j++)
			ch[i].ins[j] = myinsbank[ch[i].inum][j];
        	ch[i].vol = 127;
		ch[i].nshift = -25;
		ch[i].on = 1;
        }

	/* General init */
	for (i=0; i < 9; i++) {
		chp[i][0] = -1;
		chp[i][2] = 0;
        }
	
	adlib_mode = ADLIB_MELODIC;
}

int MidiDriver_MIDIEMU::open(int mode) {
	_opl = OPLCreate(OPL_TYPE_YM3812, 3579545, g_scumm->_system->property(OSystem::PROP_GET_SAMPLE_RATE,0));
	g_scumm->_mixer->setup_premix((void*) this, premix_proc);
	return 0;
}

void MidiDriver_MIDIEMU::close() {
	_mode = 0;
}

void MidiDriver_MIDIEMU::generate_samples(int16 *data, uint len) {
	if (!_opl) {
		memset(data, 0, len * sizeof(*data));
	} else {
		YM3812UpdateOne(_opl, data, len);
	}
}

void MidiDriver_MIDIEMU::premix_proc(void *param, int16 *buf, unsigned int len) {
	((MidiDriver_MIDIEMU*)param)->generate_samples(buf, len);
}

void MidiDriver_MIDIEMU::send(uint32 b)
{
	unsigned char channel = b & 0x0F;

	switch (b & 0xF0) {
	case 0x80: {/*note off*/
		unsigned char note = (unsigned char)((b >> 8) & 0x7F);
                for (int i = 0; i < 9; i++)
                	if ((chp[i][0] == channel) && (chp[i][1] == note)) {
                        	midi_fm_endnote(i);
                        	chp[i][0] = -1;
                        }
	}
	break;
	case 0x90: { /*note on*/
		unsigned char note = (unsigned char)((b >> 8) & 0x7F);
		unsigned char vel = (unsigned char)((b >> 16) & 0x7F);
		int i, j;
		int onl, on, nv;
		unsigned char ins[11];

                if (ch[channel].on != 0) {
			for (i = 0; i < 9; i++)
				chp[i][2]++;

			j = 0;
			on = -1;
			onl = 0;
			for (i = 0; i < 9; i++)
				if ((chp[i][0] == -1) && (chp[i][2] > onl)) {
					onl = chp[i][2];
					on = i;
					j = 1;
				}

			if (on == -1) {
				onl = 0;
				for (i = 0; i < 9; i++)
					if (chp[i][2] > onl) {
						onl = chp[i][2];
						on = i;
					}
			}

			if (j == 0)
				midi_fm_endnote(on);

			if ((vel != 0) && (ch[channel].inum >= 0) && (ch[channel].inum < 128)) {
				if ((adlib_mode == ADLIB_MELODIC) || (channel < 12))
					midi_fm_instrument(on, ch[channel].ins);
				else {
					//the following fails to be effective
					//at doing rythm sounds .. (cmf blah)
					ins[0]  = ins[1] = ch[channel].ins[0];
					ins[2]  = ins[3] = ch[channel].ins[2];
					ins[4]  = ins[5] = ch[channel].ins[4];
					ins[6]  = ins[7] = ch[channel].ins[6];
					ins[8]  = ins[9] = ch[channel].ins[8];
					ins[10] = (ch[channel].ins[10] | 1);
					midi_fm_instrument(on,ins);
				}

				nv = (ch[channel].vol * vel) / 128;
#ifdef LUCAS_MODE
				nv *= 2;
#endif
				if (nv > 127) nv = 127;
				nv = my_midi_fm_vol_table[nv];
#ifdef LUCAS_MODE
				nv = (int)((double) sqrt((double) nv) * 11); /* TO CHANGE !!! */
#endif
                        }

			midi_fm_playnote(on, note + ch[channel].nshift, nv * 2);
			chp[on][0] = channel;
			chp[on][1] = note;
			chp[on][2] = 0;
		} else {
                	if (vel == 0) { //same code as end note
				for (i = 0; i < 9; i++)
					if ((chp[i][0] == channel) && (chp[i][1] == note)) {
						// midi_fm_volume(i,0);  // really end the note
						midi_fm_endnote(i);
						chp[i][0] = -1;
					}
                        } else {        // i forget what this is for.
				chp[on][0] = -1;
				chp[on][2] = 0;
                        }
		}
	}
	break;

	case 0xa0: /*key after touch */
		/* this might all be good
		   note=getnext(1); vel=getnext(1);
		   for (i=0; i<9; i++)
		   if (chp[i][0]==c & chp[i][1]==note)
		   midi_fm_playnote(i,note+cnote[c],my_midi_fm_vol_table[(cvols[c]*vel)/128]*2);
                */
		debug(1, "MIDI command 0xA0 (Key After Touch) not handled in MIDIEMU driver.");
	break;

	case 0xb0: { /* control change .. pitch bend? */
		unsigned char ctrl = (unsigned char)((b >> 8) & 0x7F);
		unsigned char vel = (unsigned char)((b >> 16) & 0x7F);

                switch(ctrl) {
		case 0x07:
			ch[channel].vol = vel;
			break;
		default:
			debug(1, "MIDI sub-command 0xB0 (Key After Touch) case %02X not handled in MIDIEMU driver.", ctrl);
                	break;
		}
	} break;
	
	case 0xc0: { /* patch change */
		unsigned char instrument = (unsigned char)((b >> 8) & 0x7F);
		ch[channel].inum = instrument;
		for (int j = 0; j < 11; j++)
			ch[channel].ins[j] = myinsbank[ch[channel].inum][j];
	} break;

	case 0xd0: /* channel touch */
		debug(1, "MIDI command 0xD0 (Channel Touch) not handled in MIDIEMU driver.");
	break;
	case 0xe0: /* pitch wheel */
		debug(1, "MIDI command 0xE0 (Pitch Wheel) not handled in MIDIEMU driver.");
	break;

	default:
		fprintf(stderr, "Unknown : %08x\n", (int) b);
		break;
	}
}

void MidiDriver_MIDIEMU::pause(bool pause) {
	if (_mode == MO_STREAMING) {
	}
}

void MidiDriver_MIDIEMU::set_stream_callback(void *param, StreamCallback *sc) {
	_stream_param = param;
	_stream_proc = sc;
}

MidiDriver *MidiDriver_MIDIEMU_create() {
	return new MidiDriver_MIDIEMU();
}

void MidiDriver_MIDIEMU::midi_write_adlib(unsigned int r, unsigned char v) {
	OPLWriteReg(_opl, r, v);
	adlib_data[r] = v;
}

static unsigned char adlib_opadd[] = {0x00  ,0x01 ,0x02  ,0x08  ,0x09  ,0x0A  ,0x10 ,0x11  ,0x12};

void MidiDriver_MIDIEMU::midi_fm_instrument(int voice, unsigned char *inst) {
	/* Just gotta make sure this happens because who knows when it'll be reset otherwise.... */
	midi_write_adlib(0xbd, 0);

	midi_write_adlib(0x20 + adlib_opadd[voice], inst[0]);
	midi_write_adlib(0x23 + adlib_opadd[voice], inst[1]);
	
#ifdef LUCAS_MODE
	midi_write_adlib(0x43 + adlib_opadd[voice], 0x3f);
	if ((inst[10] & 1) == 0)
		midi_write_adlib(0x40 + adlib_opadd[voice], inst[2]);
	else
		midi_write_adlib(0x40 + adlib_opadd[voice], 0x3f);
#else
	midi_write_adlib(0x40 + adlib_opadd[voice], inst[2]);
	if ((inst[10] & 1) == 0)
		midi_write_adlib(0x43 + adlib_opadd[voice],inst[3]);
	else
		midi_write_adlib(0x43 + adlib_opadd[voice], 0);
#endif	

	midi_write_adlib(0x60 + adlib_opadd[voice], inst[4]);
	midi_write_adlib(0x63 + adlib_opadd[voice], inst[5]);
	midi_write_adlib(0x80 + adlib_opadd[voice], inst[6]);
	midi_write_adlib(0x83 + adlib_opadd[voice], inst[7]);
	midi_write_adlib(0xe0 + adlib_opadd[voice], inst[8]);
	midi_write_adlib(0xe3 + adlib_opadd[voice], inst[9]);
	
	midi_write_adlib(0xc0 + voice, inst[10]);
}

void MidiDriver_MIDIEMU::midi_fm_volume(int voice, int volume) {
	volume >>= 2;

	if ((adlib_data[0xc0 + voice] & 1) == 1)
		midi_write_adlib(0x40 + adlib_opadd[voice],
		                 (unsigned char)((63 - volume) | (adlib_data[0x40 + adlib_opadd[voice]] & 0xc0)));
	midi_write_adlib(0x43 + adlib_opadd[voice],
	                 (unsigned char)((63 - volume) | (adlib_data[0x43 + adlib_opadd[voice]] & 0xc0)));
}

static int fnums[] = { 0x16b,0x181,0x198,0x1b0,0x1ca,0x1e5,0x202,0x220,0x241,0x263,0x287,0x2ae };

void MidiDriver_MIDIEMU::midi_fm_playnote(int voice, int note, int volume) {
	int freq = fnums[note % 12];
	int oct = note / 12;
	int c;

	midi_fm_volume(voice, volume);
	midi_write_adlib(0xa0 + voice, (unsigned char) (freq & 0xff));

	c = ((freq & 0x300) >> 8) + (oct << 2) + (1 << 5);
	midi_write_adlib(0xb0 + voice, (unsigned char) c);
}

void MidiDriver_MIDIEMU::midi_fm_endnote(int voice) {
	midi_write_adlib(0xb0 + voice, (unsigned char) (adlib_data[0xb0 + voice] & (255 - 32)));
}


#if 0

/* Old code for timidity support, maybe somebody can rewrite this for the 
   new midi driver system?
 */


/*********** Timidity		*/
int MidiDriver::connect_to_timidity(int port)
{
	int s = 0;
#if !defined(macintosh) && !defined(__MORPHOS__)	// No socket support on Apple Carbon or Morphos
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


#endif /* 0 */
