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
 * Alsa support by Nicolas Noble <nicolas@nobis-crew.org> copied from
 *    both the QuickTime support and (vkeybd http://www.alsa-project.org/~iwai/alsa.html)
 */

#include "stdafx.h"
#include "scumm.h"
#include "mididrv.h"
#include "fmopl.h"
#include "mixer.h"

#if defined(WIN32) && !defined(_WIN32_WCE)

/* Windows MIDI driver */
class MidiDriver_WIN:public MidiDriver {
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
		MMRESULT res = midiOutOpen((HMIDIOUT *) & _mo, MIDI_MAPPER, NULL, NULL, 0);
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

			/* end of stream? */
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
					/* change tempo event */
					ev->dwEvent = (ME_TEMPO << 24) | (my_ev->event & 0xFFFFFF);
					break;
				default:
					error("Invalid event type passed");
				}

				/* increase stream pointer by 12 bytes 
				 * (need to be 12 bytes, and sizeof(MIDIEVENT) is 16) 
				 */
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

	//printMidi(u.bData[0], u.bData[1], u.bData[2], u.bData[3]);
	check_error(midiOutShortMsg(_mo, u.dwData));
}

void MidiDriver_WIN::pause(bool pause)
{
	if (_mode == MO_STREAMING) {
		if (pause)
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
		error("MM System Error '%s'", buf);
	}
}

uint32 MidiDriver_WIN::property(int prop, uint32 param)
{
	switch (prop) {

		/* 16-bit time division according to standard midi specification */
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

#endif // WIN32

#ifdef __MORPHOS__
#include <exec/types.h>
#include <devices/amidi.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/amidi.h>

#include "morphos_sound.h"

/* MorphOS MIDI driver */
class MidiDriver_AMIDI:public MidiDriver {
public:
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool pause);
	void set_stream_callback(void *param, StreamCallback *sc);

private:
	StreamCallback *_stream_proc;
	void *_stream_param;
	int _mode;
};

void MidiDriver_AMIDI::set_stream_callback(void *param, StreamCallback *sc)
{
	_stream_param = param;
	_stream_proc = sc;
}

int MidiDriver_AMIDI::open(int mode)
{
	_mode = mode;
	init_morphos_music(0);
	return 0;
}

void MidiDriver_AMIDI::close()
{
	exit_morphos_music();
	_mode = 0;
}

void MidiDriver_AMIDI::send(uint32 b)
{
	if (_mode != MO_SIMPLE)
		error("MidiDriver_AMIDI:send called but driver is not in simple mode");

	if (ScummMidiRequest) {
		ULONG midi_data = READ_LE_UINT32(&b);
		SendShortMidiMsg(ScummMidiRequest, midi_data);
	}
}

void MidiDriver_AMIDI::pause(bool pause)
{
	if (_mode == MO_STREAMING) {
	}
}

MidiDriver *MidiDriver_AMIDI_create()
{
	return new MidiDriver_AMIDI();
}

#endif // __MORPHOS__

#if defined(UNIX) && !defined(__BEOS__)
#define SEQ_MIDIPUTC    5
#define SPECIAL_CHANNEL 9

class MidiDriver_SEQ:public MidiDriver {
public:
	MidiDriver_SEQ();
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool pause);
	void set_stream_callback(void *param, StreamCallback *sc);

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
	_mode = mode;
	if (mode != MO_SIMPLE)
		return MERR_STREAMING_NOT_AVAILABLE;

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

void MidiDriver_SEQ::pause(bool pause)
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
class MidiDriver_QT:public MidiDriver {
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
	void *_stream_param;
	int _mode;
};

void MidiDriver_QT::set_stream_callback(void *param, StreamCallback *sc)
{
	_stream_param = param;
	_stream_proc = sc;
}

int MidiDriver_QT::open(int mode)
{
	ComponentResult qtErr = noErr;
	int i;

	qtNoteAllocator = NULL;

	if (mode == MO_STREAMING)
		return MERR_STREAMING_NOT_AVAILABLE;

	_mode = mode;

	for (i = 0; i < 15; i++)
		qtNoteChannel[i] = NULL;

	qtNoteAllocator = OpenDefaultComponent(kNoteAllocatorComponentType, 0);
	if (qtNoteAllocator == NULL)
		goto bail;

	simpleNoteRequest.info.flags = 0;
	*(short *)(&simpleNoteRequest.info.polyphony) = EndianS16_NtoB(15);	// simultaneous tones
	*(Fixed *) (&simpleNoteRequest.info.typicalPolyphony) = EndianU32_NtoB(0x00010000);

	qtErr = NAStuffToneDescription(qtNoteAllocator, 1, &simpleNoteRequest.tone);
	if (qtErr != noErr)
		goto bail;

	for (i = 0; i < 15; i++) {
		qtErr = NANewNoteChannel(qtNoteAllocator, &simpleNoteRequest, &(qtNoteChannel[i]));
		if ((qtErr != noErr) || (qtNoteChannel == NULL))
			goto bail;
	}
	return 0;

bail:
	error("Init QT failed %x %x %d\n", (int)qtNoteAllocator, (int)qtNoteChannel, (int)qtErr);
	for (i = 0; i < 15; i++) {
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

void MidiDriver_QT::close()
{
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

void MidiDriver_QT::send(uint32 b)
{
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
			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerVolume, midiCmd[2] * 300);
			break;

		case 0x0A:									// Pan
			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerPan,
											(midiCmd[2] << 1) + 0xFF);
			break;

		case 0x40:									// Sustain on/off
			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerSustain, midiCmd[2]);
			break;

		case 0x5b:									// ext effect depth
			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerReverb, midiCmd[2] << 8);
			break;

		case 0x5d:									// chorus depth
			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerChorus, midiCmd[2] << 8);
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
			long theBend = ((((long)midiCmd[1] + (long)(midiCmd[2] << 8))) - 0x4000) / 4;
			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerPitchBend, theBend);
		}
		break;

	default:
		error("Unknown Command: %08x\n", (int)b);
		NASendMIDI(qtNoteAllocator, qtNoteChannel[chanID], &midPacket);
		break;
	}
}

void MidiDriver_QT::pause(bool)
{
}

MidiDriver *MidiDriver_QT_create()
{
	return new MidiDriver_QT();
}

#endif // __APPLE__ || macintosh


#ifdef __APPLE__

#include <AudioUnit/AudioUnit.h>

/* CoreAudio MIDI driver */
/* Based on code by Benjamin W. Zale */
class MidiDriver_CORE:public MidiDriver {
public:
	MidiDriver_CORE():au_MusicDevice(NULL), au_output(NULL) {
	} int open(int mode);
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


void MidiDriver_CORE::set_stream_callback(void *param, StreamCallback *sc)
{
	_stream_param = param;
	_stream_proc = sc;
}

int MidiDriver_CORE::open(int mode)
{

	if (au_output != NULL)
		return MERR_ALREADY_OPEN;

	_mode = mode;

	int err;
	struct AudioUnitConnection auconnect;
	ComponentDescription compdesc;
	Component compid;

	// Open the Music Device
	compdesc.componentType = kAudioUnitComponentType;
	compdesc.componentSubType = kAudioUnitSubType_MusicDevice;
	compdesc.componentManufacturer = kAudioUnitID_DLSSynth;
	compdesc.componentFlags = 0;
	compdesc.componentFlagsMask = 0;
	compid = FindNextComponent(NULL, &compdesc);
	au_MusicDevice = (AudioUnit) OpenComponent(compid);

	// open the output unit
	au_output = (AudioUnit) OpenDefaultComponent(kAudioUnitComponentType, kAudioUnitSubType_Output);

	// connect the units
	auconnect.sourceAudioUnit = au_MusicDevice;
	auconnect.sourceOutputNumber = 0;
	auconnect.destInputNumber = 0;
	err =
		AudioUnitSetProperty(au_output, kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input, 0,
												 (void *)&auconnect, sizeof(struct AudioUnitConnection));

	// initialize the units
	AudioUnitInitialize(au_MusicDevice);
	AudioUnitInitialize(au_output);

	// start the output
	AudioOutputUnitStart(au_output);

	return 0;
}

void MidiDriver_CORE::close()
{

	// Stop the output
	AudioOutputUnitStop(au_output);

	// Cleanup
	CloseComponent(au_output);
	CloseComponent(au_MusicDevice);

	_mode = 0;
}

void MidiDriver_CORE::send(uint32 b)
{
	if (_mode != MO_SIMPLE)
		error("MidiDriver_CORE:send called but driver is not in simple mode");

	unsigned char first_byte, seccond_byte, status_byte;
	status_byte = (b & 0x000000FF);
	first_byte = (b & 0x0000FF00) >> 8;
	seccond_byte = (b & 0x00FF0000) >> 16;
	MusicDeviceMIDIEvent(au_MusicDevice, status_byte, first_byte, seccond_byte, 0);
}

void MidiDriver_CORE::pause(bool)
{
}

MidiDriver *MidiDriver_CORE_create()
{
	return new MidiDriver_CORE();
}

#endif // __APPLE__

/* NULL driver */
class MidiDriver_NULL:public MidiDriver {
public:
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool pause);
	void set_stream_callback(void *param, StreamCallback *sc);
private:
};

int MidiDriver_NULL::open(int mode)
{
	warning("Music not enabled - MIDI support selected with no MIDI driver available. Try Adlib");
	return 0;
}
void MidiDriver_NULL::close()
{
}
void MidiDriver_NULL::send(uint32 b)
{
}
void MidiDriver_NULL::pause(bool pause)
{
}
void MidiDriver_NULL::set_stream_callback(void *param, StreamCallback *sc)
{
}

MidiDriver *MidiDriver_NULL_create()
{
	return new MidiDriver_NULL();
}



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

/* This is the internal emulated MIDI driver using the included OPL2 sound chip 
 * FM instrument definitions below borrowed from the Allegro library by
 * Phil Hassey, <philhassey@hotmail.com> (www.imitationpickles.org)
 */

static unsigned char midi_fm_instruments_table[128][11] = {
	/* This set of GM instrument patches was provided by Jorrit Rouwe...
	 */
	{0x21, 0x21, 0x8f, 0x0c, 0xf2, 0xf2, 0x45, 0x76, 0x00, 0x00, 0x08},	/* Acoustic Grand */
	{0x31, 0x21, 0x4b, 0x09, 0xf2, 0xf2, 0x54, 0x56, 0x00, 0x00, 0x08},	/* Bright Acoustic */
	{0x31, 0x21, 0x49, 0x09, 0xf2, 0xf2, 0x55, 0x76, 0x00, 0x00, 0x08},	/* Electric Grand */
	{0xb1, 0x61, 0x0e, 0x09, 0xf2, 0xf3, 0x3b, 0x0b, 0x00, 0x00, 0x06},	/* Honky-Tonk */
	{0x01, 0x21, 0x57, 0x09, 0xf1, 0xf1, 0x38, 0x28, 0x00, 0x00, 0x00},	/* Electric Piano 1 */
	{0x01, 0x21, 0x93, 0x09, 0xf1, 0xf1, 0x38, 0x28, 0x00, 0x00, 0x00},	/* Electric Piano 2 */
	{0x21, 0x36, 0x80, 0x17, 0xa2, 0xf1, 0x01, 0xd5, 0x00, 0x00, 0x08},	/* Harpsichord */
	{0x01, 0x01, 0x92, 0x09, 0xc2, 0xc2, 0xa8, 0x58, 0x00, 0x00, 0x0a},	/* Clav */
	{0x0c, 0x81, 0x5c, 0x09, 0xf6, 0xf3, 0x54, 0xb5, 0x00, 0x00, 0x00},	/* Celesta */
	{0x07, 0x11, 0x97, 0x89, 0xf6, 0xf5, 0x32, 0x11, 0x00, 0x00, 0x02},	/* Glockenspiel */
	{0x17, 0x01, 0x21, 0x09, 0x56, 0xf6, 0x04, 0x04, 0x00, 0x00, 0x02},	/* Music Box */
	{0x18, 0x81, 0x62, 0x09, 0xf3, 0xf2, 0xe6, 0xf6, 0x00, 0x00, 0x00},	/* Vibraphone */
	{0x18, 0x21, 0x23, 0x09, 0xf7, 0xe5, 0x55, 0xd8, 0x00, 0x00, 0x00},	/* Marimba */
	{0x15, 0x01, 0x91, 0x09, 0xf6, 0xf6, 0xa6, 0xe6, 0x00, 0x00, 0x04},	/* Xylophone */
	{0x45, 0x81, 0x59, 0x89, 0xd3, 0xa3, 0x82, 0xe3, 0x00, 0x00, 0x0c},	/* Tubular Bells */
	{0x03, 0x81, 0x49, 0x89, 0x74, 0xb3, 0x55, 0x05, 0x01, 0x00, 0x04},	/* Dulcimer */
	{0x71, 0x31, 0x92, 0x09, 0xf6, 0xf1, 0x14, 0x07, 0x00, 0x00, 0x02},	/* Drawbar Organ */
	{0x72, 0x30, 0x14, 0x09, 0xc7, 0xc7, 0x58, 0x08, 0x00, 0x00, 0x02},	/* Percussive Organ */
	{0x70, 0xb1, 0x44, 0x09, 0xaa, 0x8a, 0x18, 0x08, 0x00, 0x00, 0x04},	/* Rock Organ */
	{0x23, 0xb1, 0x93, 0x09, 0x97, 0x55, 0x23, 0x14, 0x01, 0x00, 0x04},	/* Church Organ */
	{0x61, 0xb1, 0x13, 0x89, 0x97, 0x55, 0x04, 0x04, 0x01, 0x00, 0x00},	/* Reed Organ */
	{0x24, 0xb1, 0x48, 0x09, 0x98, 0x46, 0x2a, 0x1a, 0x01, 0x00, 0x0c},	/* Accoridan */
	{0x61, 0x21, 0x13, 0x09, 0x91, 0x61, 0x06, 0x07, 0x01, 0x00, 0x0a},	/* Harmonica */
	{0x21, 0xa1, 0x13, 0x92, 0x71, 0x61, 0x06, 0x07, 0x00, 0x00, 0x06},	/* Tango Accordian */
	{0x02, 0x41, 0x9c, 0x89, 0xf3, 0xf3, 0x94, 0xc8, 0x01, 0x00, 0x0c},	/* Acoustic Guitar(nylon) */
	{0x03, 0x11, 0x54, 0x09, 0xf3, 0xf1, 0x9a, 0xe7, 0x01, 0x00, 0x0c},	/* Acoustic Guitar(steel) */
	{0x23, 0x21, 0x5f, 0x09, 0xf1, 0xf2, 0x3a, 0xf8, 0x00, 0x00, 0x00},	/* Electric Guitar(jazz) */
	{0x03, 0x21, 0x87, 0x89, 0xf6, 0xf3, 0x22, 0xf8, 0x01, 0x00, 0x06},	/* Electric Guitar(clean) */
	{0x03, 0x21, 0x47, 0x09, 0xf9, 0xf6, 0x54, 0x3a, 0x00, 0x00, 0x00},	/* Electric Guitar(muted) */
	{0x23, 0x21, 0x4a, 0x0e, 0x91, 0x84, 0x41, 0x19, 0x01, 0x00, 0x08},	/* Overdriven Guitar */
	{0x23, 0x21, 0x4a, 0x09, 0x95, 0x94, 0x19, 0x19, 0x01, 0x00, 0x08},	/* Distortion Guitar */
	{0x09, 0x84, 0xa1, 0x89, 0x20, 0xd1, 0x4f, 0xf8, 0x00, 0x00, 0x08},	/* Guitar Harmonics */
	{0x21, 0xa2, 0x1e, 0x09, 0x94, 0xc3, 0x06, 0xa6, 0x00, 0x00, 0x02},	/* Acoustic Bass */
	{0x31, 0x31, 0x12, 0x09, 0xf1, 0xf1, 0x28, 0x18, 0x00, 0x00, 0x0a},	/* Electric Bass(finger) */
	{0x31, 0x31, 0x8d, 0x09, 0xf1, 0xf1, 0xe8, 0x78, 0x00, 0x00, 0x0a},	/* Electric Bass(pick) */
	{0x31, 0x32, 0x5b, 0x09, 0x51, 0x71, 0x28, 0x48, 0x00, 0x00, 0x0c},	/* Fretless Bass */
	{0x01, 0x21, 0x8b, 0x49, 0xa1, 0xf2, 0x9a, 0xdf, 0x00, 0x00, 0x08},	/* Slap Bass 1 */
	{0x21, 0x21, 0x8b, 0x11, 0xa2, 0xa1, 0x16, 0xdf, 0x00, 0x00, 0x08},	/* Slap Bass 2 */
	{0x31, 0x31, 0x8b, 0x09, 0xf4, 0xf1, 0xe8, 0x78, 0x00, 0x00, 0x0a},	/* Synth Bass 1 */
	{0x31, 0x31, 0x12, 0x09, 0xf1, 0xf1, 0x28, 0x18, 0x00, 0x00, 0x0a},	/* Synth Bass 2 */
	{0x31, 0x21, 0x15, 0x09, 0xdd, 0x56, 0x13, 0x26, 0x01, 0x00, 0x08},	/* Violin */
	{0x31, 0x21, 0x16, 0x09, 0xdd, 0x66, 0x13, 0x06, 0x01, 0x00, 0x08},	/* Viola */
	{0x71, 0x31, 0x49, 0x09, 0xd1, 0x61, 0x1c, 0x0c, 0x01, 0x00, 0x08},	/* Cello */
	{0x21, 0x23, 0x4d, 0x89, 0x71, 0x72, 0x12, 0x06, 0x01, 0x00, 0x02},	/* Contrabass */
	{0xf1, 0xe1, 0x40, 0x09, 0xf1, 0x6f, 0x21, 0x16, 0x01, 0x00, 0x02},	/* Tremolo Strings */
	{0x02, 0x01, 0x1a, 0x89, 0xf5, 0x85, 0x75, 0x35, 0x01, 0x00, 0x00},	/* Pizzicato Strings */
	{0x02, 0x01, 0x1d, 0x89, 0xf5, 0xf3, 0x75, 0xf4, 0x01, 0x00, 0x00},	/* Orchestral Strings */
	{0x10, 0x11, 0x41, 0x09, 0xf5, 0xf2, 0x05, 0xc3, 0x01, 0x00, 0x02},	/* Timpani */
	{0x21, 0xa2, 0x9b, 0x0a, 0xb1, 0x72, 0x25, 0x08, 0x01, 0x00, 0x0e},	/* String Ensemble 1 */
	{0xa1, 0x21, 0x98, 0x09, 0x7f, 0x3f, 0x03, 0x07, 0x01, 0x01, 0x00},	/* String Ensemble 2 */
	{0xa1, 0x61, 0x93, 0x09, 0xc1, 0x4f, 0x12, 0x05, 0x00, 0x00, 0x0a},	/* SynthStrings 1 */
	{0x21, 0x61, 0x18, 0x09, 0xc1, 0x4f, 0x22, 0x05, 0x00, 0x00, 0x0c},	/* SynthStrings 2 */
	{0x31, 0x72, 0x5b, 0x8c, 0xf4, 0x8a, 0x15, 0x05, 0x00, 0x00, 0x00},	/* Choir Aahs */
	{0xa1, 0x61, 0x90, 0x09, 0x74, 0x71, 0x39, 0x67, 0x00, 0x00, 0x00},	/* Voice Oohs */
	{0x71, 0x72, 0x57, 0x09, 0x54, 0x7a, 0x05, 0x05, 0x00, 0x00, 0x0c},	/* Synth Voice */
	{0x90, 0x41, 0x00, 0x09, 0x54, 0xa5, 0x63, 0x45, 0x00, 0x00, 0x08},	/* Orchestra Hit */
	{0x21, 0x21, 0x92, 0x0a, 0x85, 0x8f, 0x17, 0x09, 0x00, 0x00, 0x0c},	/* Trumpet */
	{0x21, 0x21, 0x94, 0x0e, 0x75, 0x8f, 0x17, 0x09, 0x00, 0x00, 0x0c},	/* Trombone */
	{0x21, 0x61, 0x94, 0x09, 0x76, 0x82, 0x15, 0x37, 0x00, 0x00, 0x0c},	/* Tuba */
	{0x31, 0x21, 0x43, 0x09, 0x9e, 0x62, 0x17, 0x2c, 0x01, 0x01, 0x02},	/* Muted Trumpet */
	{0x21, 0x21, 0x9b, 0x09, 0x61, 0x7f, 0x6a, 0x0a, 0x00, 0x00, 0x02},	/* French Horn */
	{0x61, 0x22, 0x8a, 0x0f, 0x75, 0x74, 0x1f, 0x0f, 0x00, 0x00, 0x08},	/* Brass Section */
	{0xa1, 0x21, 0x86, 0x8c, 0x72, 0x71, 0x55, 0x18, 0x01, 0x00, 0x00},	/* SynthBrass 1 */
	{0x21, 0x21, 0x4d, 0x09, 0x54, 0xa6, 0x3c, 0x1c, 0x00, 0x00, 0x08},	/* SynthBrass 2 */
	{0x31, 0x61, 0x8f, 0x09, 0x93, 0x72, 0x02, 0x0b, 0x01, 0x00, 0x08},	/* Soprano Sax */
	{0x31, 0x61, 0x8e, 0x09, 0x93, 0x72, 0x03, 0x09, 0x01, 0x00, 0x08},	/* Alto Sax */
	{0x31, 0x61, 0x91, 0x09, 0x93, 0x82, 0x03, 0x09, 0x01, 0x00, 0x0a},	/* Tenor Sax */
	{0x31, 0x61, 0x8e, 0x09, 0x93, 0x72, 0x0f, 0x0f, 0x01, 0x00, 0x0a},	/* Baritone Sax */
	{0x21, 0x21, 0x4b, 0x09, 0xaa, 0x8f, 0x16, 0x0a, 0x01, 0x00, 0x08},	/* Oboe */
	{0x31, 0x21, 0x90, 0x09, 0x7e, 0x8b, 0x17, 0x0c, 0x01, 0x01, 0x06},	/* English Horn */
	{0x31, 0x32, 0x81, 0x09, 0x75, 0x61, 0x19, 0x19, 0x01, 0x00, 0x00},	/* Bassoon */
	{0x32, 0x21, 0x90, 0x09, 0x9b, 0x72, 0x21, 0x17, 0x00, 0x00, 0x04},	/* Clarinet */
	{0xe1, 0xe1, 0x1f, 0x09, 0x85, 0x65, 0x5f, 0x1a, 0x00, 0x00, 0x00},	/* Piccolo */
	{0xe1, 0xe1, 0x46, 0x09, 0x88, 0x65, 0x5f, 0x1a, 0x00, 0x00, 0x00},	/* Flute */
	{0xa1, 0x21, 0x9c, 0x09, 0x75, 0x75, 0x1f, 0x0a, 0x00, 0x00, 0x02},	/* Recorder */
	{0x31, 0x21, 0x8b, 0x09, 0x84, 0x65, 0x58, 0x1a, 0x00, 0x00, 0x00},	/* Pan Flute */
	{0xe1, 0xa1, 0x4c, 0x09, 0x66, 0x65, 0x56, 0x26, 0x00, 0x00, 0x00},	/* Blown Bottle */
	{0x62, 0xa1, 0xcb, 0x09, 0x76, 0x55, 0x46, 0x36, 0x00, 0x00, 0x00},	/* Skakuhachi */
	{0x62, 0xa1, 0xa2, 0x09, 0x57, 0x56, 0x07, 0x07, 0x00, 0x00, 0x0b},	/* Whistle */
	{0x62, 0xa1, 0x9c, 0x09, 0x77, 0x76, 0x07, 0x07, 0x00, 0x00, 0x0b},	/* Ocarina */
	{0x22, 0x21, 0x59, 0x09, 0xff, 0xff, 0x03, 0x0f, 0x02, 0x00, 0x00},	/* Lead 1 (square) */
	{0x21, 0x21, 0x0e, 0x09, 0xff, 0xff, 0x0f, 0x0f, 0x01, 0x01, 0x00},	/* Lead 2 (sawtooth) */
	{0x22, 0x21, 0x46, 0x89, 0x86, 0x64, 0x55, 0x18, 0x00, 0x00, 0x00},	/* Lead 3 (calliope) */
	{0x21, 0xa1, 0x45, 0x09, 0x66, 0x96, 0x12, 0x0a, 0x00, 0x00, 0x00},	/* Lead 4 (chiff) */
	{0x21, 0x22, 0x8b, 0x09, 0x92, 0x91, 0x2a, 0x2a, 0x01, 0x00, 0x00},	/* Lead 5 (charang) */
	{0xa2, 0x61, 0x9e, 0x49, 0xdf, 0x6f, 0x05, 0x07, 0x00, 0x00, 0x02},	/* Lead 6 (voice) */
	{0x20, 0x60, 0x1a, 0x09, 0xef, 0x8f, 0x01, 0x06, 0x00, 0x02, 0x00},	/* Lead 7 (fifths) */
	{0x21, 0x21, 0x8f, 0x86, 0xf1, 0xf4, 0x29, 0x09, 0x00, 0x00, 0x0a},	/* Lead 8 (bass+lead) */
	{0x77, 0xa1, 0xa5, 0x09, 0x53, 0xa0, 0x94, 0x05, 0x00, 0x00, 0x02},	/* Pad 1 (new age) */
	{0x61, 0xb1, 0x1f, 0x89, 0xa8, 0x25, 0x11, 0x03, 0x00, 0x00, 0x0a},	/* Pad 2 (warm) */
	{0x61, 0x61, 0x17, 0x09, 0x91, 0x55, 0x34, 0x16, 0x00, 0x00, 0x0c},	/* Pad 3 (polysynth) */
	{0x71, 0x72, 0x5d, 0x09, 0x54, 0x6a, 0x01, 0x03, 0x00, 0x00, 0x00},	/* Pad 4 (choir) */
	{0x21, 0xa2, 0x97, 0x09, 0x21, 0x42, 0x43, 0x35, 0x00, 0x00, 0x08},	/* Pad 5 (bowed) */
	{0xa1, 0x21, 0x1c, 0x09, 0xa1, 0x31, 0x77, 0x47, 0x01, 0x01, 0x00},	/* Pad 6 (metallic) */
	{0x21, 0x61, 0x89, 0x0c, 0x11, 0x42, 0x33, 0x25, 0x00, 0x00, 0x0a},	/* Pad 7 (halo) */
	{0xa1, 0x21, 0x15, 0x09, 0x11, 0xcf, 0x47, 0x07, 0x01, 0x00, 0x00},	/* Pad 8 (sweep) */
	{0x3a, 0x51, 0xce, 0x09, 0xf8, 0x86, 0xf6, 0x02, 0x00, 0x00, 0x02},	/* FX 1 (rain) */
	{0x21, 0x21, 0x15, 0x09, 0x21, 0x41, 0x23, 0x13, 0x01, 0x00, 0x00},	/* FX 2 (soundtrack) */
	{0x06, 0x01, 0x5b, 0x09, 0x74, 0xa5, 0x95, 0x72, 0x00, 0x00, 0x00},	/* FX 3 (crystal) */
	{0x22, 0x61, 0x92, 0x8c, 0xb1, 0xf2, 0x81, 0x26, 0x00, 0x00, 0x0c},	/* FX 4 (atmosphere) */
	{0x41, 0x42, 0x4d, 0x09, 0xf1, 0xf2, 0x51, 0xf5, 0x01, 0x00, 0x00},	/* FX 5 (brightness) */
	{0x61, 0xa3, 0x94, 0x89, 0x11, 0x11, 0x51, 0x13, 0x01, 0x00, 0x06},	/* FX 6 (goblins) */
	{0x61, 0xa1, 0x8c, 0x89, 0x11, 0x1d, 0x31, 0x03, 0x00, 0x00, 0x06},	/* FX 7 (echoes) */
	{0xa4, 0x61, 0x4c, 0x09, 0xf3, 0x81, 0x73, 0x23, 0x01, 0x00, 0x04},	/* FX 8 (sci-fi) */
	{0x02, 0x07, 0x85, 0x0c, 0xd2, 0xf2, 0x53, 0xf6, 0x00, 0x01, 0x00},	/* Sitar */
	{0x11, 0x13, 0x0c, 0x89, 0xa3, 0xa2, 0x11, 0xe5, 0x01, 0x00, 0x00},	/* Banjo */
	{0x11, 0x11, 0x06, 0x09, 0xf6, 0xf2, 0x41, 0xe6, 0x01, 0x02, 0x04},	/* Shamisen */
	{0x93, 0x91, 0x91, 0x09, 0xd4, 0xeb, 0x32, 0x11, 0x00, 0x01, 0x08},	/* Koto */
	{0x04, 0x01, 0x4f, 0x09, 0xfa, 0xc2, 0x56, 0x05, 0x00, 0x00, 0x0c},	/* Kalimba */
	{0x21, 0x22, 0x49, 0x09, 0x7c, 0x6f, 0x20, 0x0c, 0x00, 0x01, 0x06},	/* Bagpipe */
	{0x31, 0x21, 0x85, 0x09, 0xdd, 0x56, 0x33, 0x16, 0x01, 0x00, 0x0a},	/* Fiddle */
	{0x20, 0x21, 0x04, 0x8a, 0xda, 0x8f, 0x05, 0x0b, 0x02, 0x00, 0x06},	/* Shanai */
	{0x05, 0x03, 0x6a, 0x89, 0xf1, 0xc3, 0xe5, 0xe5, 0x00, 0x00, 0x06},	/* Tinkle Bell */
	{0x07, 0x02, 0x15, 0x09, 0xec, 0xf8, 0x26, 0x16, 0x00, 0x00, 0x0a},	/* Agogo */
	{0x05, 0x01, 0x9d, 0x09, 0x67, 0xdf, 0x35, 0x05, 0x00, 0x00, 0x08},	/* Steel Drums */
	{0x18, 0x12, 0x96, 0x09, 0xfa, 0xf8, 0x28, 0xe5, 0x00, 0x00, 0x0a},	/* Woodblock */
	{0x10, 0x00, 0x86, 0x0c, 0xa8, 0xfa, 0x07, 0x03, 0x00, 0x00, 0x06},	/* Taiko Drum */
	{0x11, 0x10, 0x41, 0x0c, 0xf8, 0xf3, 0x47, 0x03, 0x02, 0x00, 0x04},	/* Melodic Tom */
	{0x01, 0x10, 0x8e, 0x09, 0xf1, 0xf3, 0x06, 0x02, 0x02, 0x00, 0x0e},	/* Synth Drum */
	{0x0e, 0xc0, 0x00, 0x09, 0x1f, 0x1f, 0x00, 0xff, 0x00, 0x03, 0x0e},	/* Reverse Cymbal */
	{0x06, 0x03, 0x80, 0x91, 0xf8, 0x56, 0x24, 0x84, 0x00, 0x02, 0x0e},	/* Guitar Fret Noise */
	{0x0e, 0xd0, 0x00, 0x0e, 0xf8, 0x34, 0x00, 0x04, 0x00, 0x03, 0x0e},	/* Breath Noise */
	{0x0e, 0xc0, 0x00, 0x09, 0xf6, 0x1f, 0x00, 0x02, 0x00, 0x03, 0x0e},	/* Seashore */
	{0xd5, 0xda, 0x95, 0x49, 0x37, 0x56, 0xa3, 0x37, 0x00, 0x00, 0x00},	/* Bird Tweet */
	{0x35, 0x14, 0x5c, 0x11, 0xb2, 0xf4, 0x61, 0x15, 0x02, 0x00, 0x0a},	/* Telephone ring */
	{0x0e, 0xd0, 0x00, 0x09, 0xf6, 0x4f, 0x00, 0xf5, 0x00, 0x03, 0x0e},	/* Helicopter */
	{0x26, 0xe4, 0x00, 0x09, 0xff, 0x12, 0x01, 0x16, 0x00, 0x01, 0x0e},	/* Applause */
	{0x00, 0x00, 0x00, 0x09, 0xf3, 0xf6, 0xf0, 0xc9, 0x00, 0x02, 0x0e}	/* Gunshot */

};

/* logarithmic relationship between midi and FM volumes */
static int my_midi_fm_vol_table[128] = {
	0, 11, 16, 19, 22, 25, 27, 29, 32, 33, 35, 37, 39, 40, 42, 43,
	45, 46, 48, 49, 50, 51, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
	64, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 75, 76, 77,
	78, 79, 80, 80, 81, 82, 83, 83, 84, 85, 86, 86, 87, 88, 89, 89,
	90, 91, 91, 92, 93, 93, 94, 95, 96, 96, 97, 97, 98, 99, 99, 100,
	101, 101, 102, 103, 103, 104, 104, 105, 106, 106, 107, 107, 108,
	109, 109, 110, 110, 111, 112, 112, 113, 113, 114, 114, 115, 115,
	116, 117, 117, 118, 118, 119, 119, 120, 120, 121, 121, 122, 122,
	123, 123, 124, 124, 125, 125, 126, 126, 127
};

class MidiDriver_MIDIEMU:public MidiDriver {
public:
	MidiDriver_MIDIEMU();
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool pause);
	void set_stream_callback(void *param, StreamCallback *sc);
	static int midiemu_callback_thread(void *param);

private:
	struct midi_channel {
		int inum;
		unsigned char ins[11];
		int vol;
		int nshift;
		int on;
		int pitchbend;
	};

	enum {
		ADLIB_MELODIC = 0,
		ADLIB_RYTHM = 1
	};

	void generate_samples(int16 *buf, uint len);
	static void premix_proc(void *param, int16 *buf, unsigned int len);

	void midi_write_adlib(unsigned int reg, unsigned char val);
	void midi_fm_instrument(int voice, unsigned char *inst);
	void midi_fm_volume(int voice, int volume);
	void midi_fm_playnote(int voice, int note, int volume, int pitchbend);
	void midi_fm_endnote(int voice);
	unsigned char adlib_data[256];
	int adlib_style;
	int adlib_mode;

	StreamCallback *_stream_proc;
	void *_stream_param;
	int _mode;
	int chp[9][3];
	unsigned char myinsbank[128][11];
	FM_OPL *_opl;
	midi_channel ch[16];
};

static unsigned char adlib_opadd[] = { 0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12 };
MidiDriver_MIDIEMU::MidiDriver_MIDIEMU()
{
	int i, j;

	_mode = 0;
	for (i = 0; i < 128; i++)
		for (j = 0; j < 11; j++)
			myinsbank[i][j] = midi_fm_instruments_table[i][j];
	for (i = 0; i < 16; i++) {
		ch[i].inum = 0;
		for (j = 0; j < 11; j++)
			ch[i].ins[j] = myinsbank[ch[i].inum][j];
		ch[i].vol = 127;
		ch[i].nshift = -25;
		ch[i].on = 1;
		ch[i].pitchbend = 0x2000;
	}

	/* General init */
	for (i = 0; i < 9; i++) {
		chp[i][0] = -1;
		chp[i][2] = 0;
	}

	adlib_mode = ADLIB_MELODIC;
}

int MidiDriver_MIDIEMU::open(int mode)
{
	_opl = OPLCreate(OPL_TYPE_YM3812, 3579545, g_system->property(OSystem::PROP_GET_SAMPLE_RATE, 0));
	g_mixer->setup_premix((void *)this, premix_proc);
	if (_stream_proc)
		g_system->create_thread(midiemu_callback_thread, this);
	return 0;
}

void MidiDriver_MIDIEMU::close()
{
	_mode = 0;
}

void MidiDriver_MIDIEMU::generate_samples(int16 *data, uint len)
{
	if (!_opl) {
		memset(data, 0, len * sizeof(*data));
	} else {
		YM3812UpdateOne(_opl, data, len);
	}
}

void MidiDriver_MIDIEMU::premix_proc(void *param, int16 *buf, unsigned int len)
{
	((MidiDriver_MIDIEMU *) param)->generate_samples(buf, len);
}

void MidiDriver_MIDIEMU::send(uint32 b)
{
	unsigned char channel = (char)(b & 0x0F);

	switch (b & 0xF0) {
	case 0x80:{									/*note off */
			unsigned char note = (unsigned char)((b >> 8) & 0x7F);
			for (int i = 0; i < 9; i++)
				if ((chp[i][0] == channel) && (chp[i][1] == note)) {
					midi_fm_endnote(i);
					chp[i][0] = -1;
				}
		}
		break;
	case 0x90:{									/*note on */
			unsigned char note = (unsigned char)((b >> 8) & 0x7F);
			unsigned char vel = (unsigned char)((b >> 16) & 0x7F);
			int i, j;
			int onl, on, nv = 0;
			unsigned char ins[11];
			on = -1;
			if (ch[channel].on != 0) {
				for (i = 0; i < 9; i++)
					chp[i][2]++;

				j = 0;
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
						ins[0] = ins[1] = ch[channel].ins[0];
						ins[2] = ins[3] = ch[channel].ins[2];
						ins[4] = ins[5] = ch[channel].ins[4];
						ins[6] = ins[7] = ch[channel].ins[6];
						ins[8] = ins[9] = ch[channel].ins[8];
						ins[10] = (ch[channel].ins[10] | 1);
						midi_fm_instrument(on, ins);
					}

					nv = (ch[channel].vol * vel) / 128;
#ifdef LUCAS_MODE
					nv *= 2;
#endif
					if (nv > 127)
						nv = 127;
					nv = my_midi_fm_vol_table[nv];
#ifdef LUCAS_MODE
					nv = (int)((double)sqrt((double)nv) * 11);	/* TO CHANGE !!! */
#endif
				}

				midi_fm_playnote(on, note + ch[channel].nshift, nv * 2, ch[channel].pitchbend);
				chp[on][0] = channel;
				chp[on][1] = note;
				chp[on][2] = 0;
			} else {
				if (vel == 0) {					//same code as end note
					for (i = 0; i < 9; i++)
						if ((chp[i][0] == channel) && (chp[i][1] == note)) {
							// midi_fm_volume(i,0);  // really end the note
							midi_fm_endnote(i);
							chp[i][0] = -1;
						}
				} else {								// i forget what this is for.
					chp[on][0] = -1;
					chp[on][2] = 0;
				}
			}
		}
		break;

	case 0xa0:{									/*key after touch */
			unsigned char note = (unsigned char)((b >> 8) & 0x7F);
			unsigned char vel = (unsigned char)((b >> 16) & 0x7F);

			for (int i = 0; i < 9; i++)
				if ((chp[i][0] == channel) & (chp[i][1] == note)) {
					int nv = (ch[channel].vol * vel) / 128;
					if (nv > 127)
						nv = 127;
					nv = my_midi_fm_vol_table[nv];
					midi_fm_volume(i, nv * 2);
				}
		}
		break;

	case 0xb0:{									/* control change */
			int i;
			unsigned char ctrl = (unsigned char)((b >> 8) & 0x7F);
			unsigned char vel = (unsigned char)((b >> 16) & 0x7F);

			/* FIXME: Except for Volume, the Modulation and Sustain
			   code is just a random guess. */
			switch (ctrl) {
			case 0x01:								/* Modulation */
				for (i = 0; i < 9; i++)
					if (chp[i][0] == channel)
						midi_write_adlib(0x20 + adlib_opadd[i], vel);
				break;
			case 0x07:								/* Volume */
				ch[channel].vol = vel;
				break;
			case 0x0A:								/* Pan */
				/* This is not a warning as we do not support OPL3 => no stereo anyway 
				   debug(1, "MIDI sub-command 0xB0 (Control Change) case %02X (Pan) not handled in MIDIEMU driver.", ctrl); */
				break;
			case 0x40:								/* Sustain on/off */
				for (i = 0; i < 9; i++)
					if (chp[i][0] == channel)
						midi_write_adlib(0x80 + adlib_opadd[i], vel);
				break;
			case 0x5B:								/* Extended depth effect */
				debug(1,
							"MIDI sub-command 0xB0 (Control Change) case %02X (Extended Depth) not handled in MIDIEMU driver.",
							ctrl);
				break;
			case 0x5D:								/* Chorus depth */
				debug(1,
							"MIDI sub-command 0xB0 (Control Change) case %02X (Chorus Depth) not handled in MIDIEMU driver.",
							ctrl);
				break;
			case 0x7B:								/* All notes off */
				for (i = 0; i < 9; i++) {
					if (chp[i][0] == channel) {
						midi_fm_endnote(i);
						chp[i][0] = -1;
					}
				}
				break;
			default:
				debug(1, "MIDI sub-command 0xB0 (Control Change) case %02X not handled in MIDIEMU driver.",
							ctrl);
				break;
			}
		}
		break;

	case 0xc0:{									/* patch change */
			unsigned char instrument = (unsigned char)((b >> 8) & 0x7F);
			ch[channel].inum = instrument;
			for (int j = 0; j < 11; j++)
				ch[channel].ins[j] = myinsbank[ch[channel].inum][j];
		} break;

	case 0xd0:										/* channel touch */
		debug(1, "MIDI command 0xD0 (Channel Touch) not handled in MIDIEMU driver.");
		break;

	case 0xe0:{									/* pitch wheel */
			int pitchbend = ((b >> 8) & 0x7F) | (((b >> 16) & 0x7F) << 7);
			ch[channel].pitchbend = pitchbend;
			for (int i = 0; i < 9; i++) {
				if (chp[i][0] == channel) {
					int nv = ch[channel].vol;
					if (nv > 127)
						nv = 127;
					nv = my_midi_fm_vol_table[nv];
					midi_fm_playnote(i, chp[i][1] + ch[channel].nshift, nv * 2, pitchbend);
				}
			}
		}
		break;

	default:
		fprintf(stderr, "Unknown : %08x\n", (int)b);
		break;
	}
}

void MidiDriver_MIDIEMU::pause(bool pause)
{
	if (_mode == MO_STREAMING) {
	}
}

#define NUMBER_MIDI_EVENTS 64

int MidiDriver_MIDIEMU::midiemu_callback_thread(void *param)
{
	MidiDriver_MIDIEMU *driver = (MidiDriver_MIDIEMU *) param;
	MidiEvent my_evs[NUMBER_MIDI_EVENTS];
	bool need_midi_data = true;

	for (;;) {
		int number = 0;
		int i;

		if (need_midi_data) {
			number = driver->_stream_proc(driver->_stream_param, my_evs, NUMBER_MIDI_EVENTS);
			if (!number) {
				// No MIDI data available for the moment
				g_system->delay_msecs(10);
				continue;
			}
			need_midi_data = false;
		}

		for (i = 0; i < number; i++) {
			uint32 event;

			event = my_evs[i].event;
			if ((event >> 24) == ME_TEMPO) {
				event = (ME_TEMPO << 24) | (event & 0xFFFFFF);
			}
			driver->send(event);
			if (my_evs[i].delta) {
				g_system->delay_msecs(my_evs[i].delta);
			}
		}

		need_midi_data = true;
	}

	return 0;
}

void MidiDriver_MIDIEMU::set_stream_callback(void *param, StreamCallback *sc)
{
	_stream_param = param;
	_stream_proc = sc;
}

MidiDriver *MidiDriver_MIDIEMU_create()
{
	return new MidiDriver_MIDIEMU();
}

void MidiDriver_MIDIEMU::midi_write_adlib(unsigned int reg, unsigned char val)
{
	OPLWrite(_opl, 0, reg);
	OPLWrite(_opl, 1, val);
	adlib_data[reg] = val;
}

void MidiDriver_MIDIEMU::midi_fm_instrument(int voice, unsigned char *inst)
{
#if 0
	/* Just gotta make sure this happens because who knows when it'll be reset otherwise.... */
	midi_write_adlib(0xbd, 0);
#endif

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
		midi_write_adlib(0x43 + adlib_opadd[voice], inst[3]);
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

void MidiDriver_MIDIEMU::midi_fm_volume(int voice, int volume)
{
	volume >>= 2;

	if ((adlib_data[0xc0 + voice] & 1) == 1)
		midi_write_adlib(0x40 + adlib_opadd[voice],
										 (unsigned char)((63 - volume) |
																		 (adlib_data[0x40 + adlib_opadd[voice]] & 0xc0)));
	midi_write_adlib(0x43 + adlib_opadd[voice],
									 (unsigned char)((63 - volume) | (adlib_data[0x43 + adlib_opadd[voice]] & 0xc0)));
}

static int fnums[] =
	{ 0x16b, 0x181, 0x198, 0x1b0, 0x1ca, 0x1e5, 0x202, 0x220, 0x241, 0x263, 0x287, 0x2ae };

/* These tables 'borrowed' from Timidity tables.c

   Copyright (C) 1999-2001 Masanao Izumo <mo@goice.co.jp>
   Copyright (C) 1995 Tuukka Toivonen <tt@cgs.fi>
*/
double bend_fine[256] = {
	1.0, 1.0002256593050698, 1.0004513695322617, 1.0006771306930664,
	1.0009029427989777, 1.0011288058614922, 1.0013547198921082, 1.0015806849023274,
	1.0018067009036538, 1.002032767907594, 1.0022588859256572, 1.0024850549693551,
	1.0027112750502025, 1.0029375461797159, 1.0031638683694153, 1.0033902416308227,
	1.0036166659754628, 1.0038431414148634, 1.0040696679605541, 1.0042962456240678,
	1.0045228744169397, 1.0047495543507072, 1.0049762854369111, 1.0052030676870944,
	1.0054299011128027, 1.0056567857255843, 1.00588372153699, 1.006110708558573,
	1.0063377468018897, 1.0065648362784985, 1.0067919769999607, 1.0070191689778405,
	1.0072464122237039, 1.0074737067491204, 1.0077010525656616, 1.0079284496849015,
	1.0081558981184175, 1.008383397877789, 1.008610948974598, 1.0088385514204294,
	1.0090662052268706, 1.0092939104055114, 1.0095216669679448, 1.0097494749257656,
	1.009977334290572, 1.0102052450739643, 1.0104332072875455, 1.0106612209429215,
	1.0108892860517005, 1.0111174026254934, 1.0113455706759138, 1.0115737902145781,
	1.0118020612531047, 1.0120303838031153, 1.0122587578762337, 1.012487183484087,
	1.0127156606383041, 1.0129441893505169, 1.0131727696323602, 1.0134014014954713,
	1.0136300849514894, 1.0138588200120575, 1.0140876066888203, 1.0143164449934257,
	1.0145453349375237, 1.0147742765327674, 1.0150032697908125, 1.0152323147233171,
	1.015461411341942, 1.0156905596583505, 1.0159197596842091, 1.0161490114311862,
	1.0163783149109531, 1.0166076701351838, 1.0168370771155553, 1.0170665358637463,
	1.0172960463914391, 1.0175256087103179, 1.0177552228320703, 1.0179848887683858,
	1.0182146065309567, 1.0184443761314785, 1.0186741975816487, 1.0189040708931674,
	1.0191339960777379, 1.0193639731470658, 1.0195940021128593, 1.0198240829868295,
	1.0200542157806898, 1.0202844005061564, 1.0205146371749483, 1.0207449257987866,
	1.0209752663893958, 1.0212056589585028, 1.0214361035178368, 1.0216666000791297,
	1.0218971486541166, 1.0221277492545349, 1.0223584018921241, 1.0225891065786274,
	1.0228198633257899, 1.0230506721453596, 1.023281533049087, 1.0235124460487257,
	1.0237434111560313, 1.0239744283827625, 1.0242054977406807, 1.0244366192415495,
	1.0246677928971357, 1.0248990187192082, 1.025130296719539, 1.0253616269099028,
	1.0255930093020766, 1.0258244439078401, 1.0260559307389761, 1.0262874698072693,
	1.0265190611245079, 1.0267507047024822, 1.0269824005529853, 1.027214148687813,
	1.0274459491187637, 1.0276778018576387, 1.0279097069162415, 1.0281416643063788,
	1.0283736740398595, 1.0286057361284953, 1.0288378505841009, 1.0290700174184932,
	1.0293022366434921, 1.0295345082709197, 1.0297668323126017, 1.0299992087803651,
	1.030231637686041, 1.0304641190414621, 1.0306966528584645, 1.0309292391488862,
	1.0311618779245688, 1.0313945691973556, 1.0316273129790936, 1.0318601092816313,
	1.0320929581168212, 1.0323258594965172, 1.0325588134325767, 1.0327918199368598,
	1.0330248790212284, 1.0332579906975481, 1.0334911549776868, 1.033724371873515,
	1.0339576413969056, 1.0341909635597348, 1.0344243383738811, 1.0346577658512259,
	1.034891246003653, 1.0351247788430489, 1.0353583643813031, 1.0355920026303078,
	1.0358256936019572, 1.0360594373081489, 1.0362932337607829, 1.0365270829717617,
	1.0367609849529913, 1.0369949397163791, 1.0372289472738365, 1.0374630076372766,
	1.0376971208186156, 1.0379312868297725, 1.0381655056826686, 1.0383997773892284,
	1.0386341019613787, 1.0388684794110492, 1.0391029097501721, 1.0393373929906822,
	1.0395719291445176, 1.0398065182236185, 1.0400411602399278, 1.0402758552053915,
	1.0405106031319582, 1.0407454040315787, 1.0409802579162071, 1.0412151647977996,
	1.0414501246883161, 1.0416851375997183, 1.0419202035439705, 1.0421553225330404,
	1.042390494578898, 1.042625719693516, 1.0428609978888699, 1.043096329176938,
	1.0433317135697009, 1.0435671510791424, 1.0438026417172486, 1.0440381854960086,
	1.0442737824274138, 1.044509432523459, 1.044745135796141, 1.0449808922574599,
	1.0452167019194181, 1.0454525647940205, 1.0456884808932754, 1.0459244502291931,
	1.0461604728137874, 1.0463965486590741, 1.046632677777072, 1.0468688601798024,
	1.0471050958792898, 1.047341384887561, 1.0475777272166455, 1.047814122878576,
	1.048050571885387, 1.0482870742491166, 1.0485236299818055, 1.0487602390954964,
	1.0489969016022356, 1.0492336175140715, 1.0494703868430555, 1.0497072096012419,
	1.0499440858006872, 1.0501810154534512, 1.050417998571596, 1.0506550351671864,
	1.0508921252522903, 1.0511292688389782, 1.0513664659393229, 1.0516037165654004,
	1.0518410207292894, 1.0520783784430709, 1.0523157897188296, 1.0525532545686513,
	1.0527907730046264, 1.0530283450388465, 1.0532659706834067, 1.0535036499504049,
	1.0537413828519411, 1.0539791694001188, 1.0542170096070436, 1.0544549034848243,
	1.0546928510455722, 1.0549308523014012, 1.0551689072644284, 1.0554070159467728,
	1.0556451783605572, 1.0558833945179062, 1.0561216644309479, 1.0563599881118126,
	1.0565983655726334, 1.0568367968255465, 1.0570752818826903, 1.0573138207562065,
	1.057552413458239, 1.0577910600009348, 1.0580297603964437, 1.058268514656918,
	1.0585073227945128, 1.0587461848213857, 1.058985100749698, 1.0592240705916123
};

double bend_coarse[128] = {
	1.0, 1.0594630943592953, 1.122462048309373, 1.189207115002721,
	1.2599210498948732, 1.3348398541700344, 1.4142135623730951, 1.4983070768766815,
	1.5874010519681994, 1.681792830507429, 1.7817974362806785, 1.8877486253633868,
	2.0, 2.1189261887185906, 2.244924096618746, 2.3784142300054421,
	2.5198420997897464, 2.6696797083400687, 2.8284271247461903, 2.996614153753363,
	3.1748021039363992, 3.363585661014858, 3.5635948725613571, 3.7754972507267741,
	4.0, 4.2378523774371812, 4.4898481932374912, 4.7568284600108841,
	5.0396841995794928, 5.3393594166801366, 5.6568542494923806, 5.993228307506727,
	6.3496042078727974, 6.727171322029716, 7.1271897451227151, 7.5509945014535473,
	8.0, 8.4757047548743625, 8.9796963864749824, 9.5136569200217682,
	10.079368399158986, 10.678718833360273, 11.313708498984761, 11.986456615013454,
	12.699208415745595, 13.454342644059432, 14.25437949024543, 15.101989002907095,
	16.0, 16.951409509748721, 17.959392772949972, 19.027313840043536,
	20.158736798317967, 21.357437666720553, 22.627416997969522, 23.972913230026901,
	25.398416831491197, 26.908685288118864, 28.508758980490853, 30.203978005814196,
	32.0, 33.902819019497443, 35.918785545899944, 38.054627680087073,
	40.317473596635935, 42.714875333441107, 45.254833995939045, 47.945826460053802,
	50.796833662982394, 53.817370576237728, 57.017517960981706, 60.407956011628393,
	64.0, 67.805638038994886, 71.837571091799887, 76.109255360174146,
	80.63494719327187, 85.429750666882214, 90.509667991878089, 95.891652920107603,
	101.59366732596479, 107.63474115247546, 114.03503592196341, 120.81591202325679,
	128.0, 135.61127607798977, 143.67514218359977, 152.21851072034829,
	161.26989438654374, 170.85950133376443, 181.01933598375618, 191.78330584021521,
	203.18733465192958, 215.26948230495091, 228.07007184392683, 241.63182404651357,
	256.0, 271.22255215597971, 287.35028436719938, 304.43702144069658,
	322.53978877308765, 341.71900266752868, 362.03867196751236, 383.56661168043064,
	406.37466930385892, 430.53896460990183, 456.14014368785394, 483.26364809302686,
	512.0, 542.44510431195943, 574.70056873439876, 608.87404288139317,
	645.0795775461753, 683.43800533505737, 724.07734393502471, 767.13322336086128,
	812.74933860771785, 861.07792921980365, 912.28028737570787, 966.52729618605372,
	1024.0, 1084.8902086239189, 1149.4011374687975, 1217.7480857627863,
	1290.1591550923506, 1366.8760106701147, 1448.1546878700494, 1534.2664467217226
};

void MidiDriver_MIDIEMU::midi_fm_playnote(int voice, int note, int volume, int pitchbend)
{
	int freq = fnums[note % 12];
	int oct = note / 12;
	int c;
	float pf;

	pitchbend -= 0x2000;
	if (pitchbend != 0) {
		pitchbend *= 2;
		if (pitchbend >= 0)
			pf = (float)(bend_fine[(pitchbend >> 5) & 0xFF] * bend_coarse[(pitchbend >> 13) & 0x7F]);
		else {
			pitchbend = -pitchbend;
			pf =
				(float)(1.0 / (bend_fine[(pitchbend >> 5) & 0xFF] * bend_coarse[(pitchbend >> 13) & 0x7F]));
		}
		freq = (int)((float)freq * pf);

		while (freq >= (fnums[0] * 2)) {
			freq /= 2;
			oct += 1;
		}
		while (freq < fnums[0]) {
			freq *= 2;
			oct -= 1;
		}
	}

	midi_fm_volume(voice, volume);
	midi_write_adlib(0xa0 + voice, (unsigned char)(freq & 0xff));

	c = ((freq & 0x300) >> 8) + (oct << 2) + (1 << 5);
	midi_write_adlib(0xb0 + voice, (unsigned char)c);
}

void MidiDriver_MIDIEMU::midi_fm_endnote(int voice)
{
	midi_write_adlib(0xb0 + voice, (unsigned char)(adlib_data[0xb0 + voice] & (255 - 32)));
}


#if defined(UNIX) && defined(USE_ALSA)

#include <alsa/asoundlib.h>

/*
 *     ALSA sequencer driver
 * Mostly cut'n'pasted from Virtual Tiny Keyboard (vkeybd) by Takashi Iwai
 *                                      (you really rox, you know?)
 */

#if SND_LIB_MINOR >= 6
#define snd_seq_flush_output(x) snd_seq_drain_output(x)
#define snd_seq_set_client_group(x,name)	/*nop */
#define my_snd_seq_open(seqp) snd_seq_open(seqp, "hw", SND_SEQ_OPEN_OUTPUT, 0)
#else
/* SND_SEQ_OPEN_OUT causes oops on early version of ALSA */
#define my_snd_seq_open(seqp) snd_seq_open(seqp, SND_SEQ_OPEN)
#endif

/*
 * parse address string
 */

#define ADDR_DELIM      ".:"

class MidiDriver_ALSA:public MidiDriver {
public:
	MidiDriver_ALSA();
	int open(int mode);
	void close();
	void send(uint32 b);
	void pause(bool pause);
	void set_stream_callback(void *param, StreamCallback *sc);

private:
	void send_event(int do_flush);
	snd_seq_event_t ev;
	StreamCallback *_stream_proc;
	void *_stream_param;
	int _mode;
	snd_seq_t *seq_handle;
	int seq_client, seq_port;
	int my_client, my_port;
	static int parse_addr(char *arg, int *client, int *port);
};

MidiDriver_ALSA::MidiDriver_ALSA():_mode(0), seq_handle(0), seq_client(0), seq_port(0), my_client(0),
my_port(0)
{
}

int MidiDriver_ALSA::parse_addr(char *arg, int *client, int *port)
{
	char *p;

	if (isdigit(*arg)) {
		if ((p = strpbrk(arg, ADDR_DELIM)) == NULL)
			return -1;
		*client = atoi(arg);
		*port = atoi(p + 1);
	} else {
		if (*arg == 's' || *arg == 'S') {
			*client = SND_SEQ_ADDRESS_SUBSCRIBERS;
			*port = 0;
		} else
			return -1;
	}
	return 0;
}

void MidiDriver_ALSA::send_event(int do_flush)
{
	snd_seq_ev_set_direct(&ev);
	snd_seq_ev_set_source(&ev, my_port);
	snd_seq_ev_set_dest(&ev, seq_client, seq_port);

	snd_seq_event_output(seq_handle, &ev);
	if (do_flush)
		snd_seq_flush_output(seq_handle);
}

int MidiDriver_ALSA::open(int mode)
{
	char *var;
	unsigned int caps;

	if (_mode != 0)
		return MERR_ALREADY_OPEN;
	_mode = mode;

	if (mode != MO_SIMPLE)
		return MERR_STREAMING_NOT_AVAILABLE;

	if (!(var = getenv("SCUMMVM_PORT"))) {
		error("You have to define the environnement variable SCUMMVM_PORT");
		return -1;
	}

	if (parse_addr(var, &seq_client, &seq_port) < 0) {
		error("Invalid port %s", var);
		return -1;
	}

	if (my_snd_seq_open(&seq_handle)) {
		error("Can't open sequencer");
		return -1;
	}

	my_client = snd_seq_client_id(seq_handle);
	snd_seq_set_client_name(seq_handle, "SCUMMVM");
	snd_seq_set_client_group(seq_handle, "input");

	caps = SND_SEQ_PORT_CAP_READ;
	if (seq_client == SND_SEQ_ADDRESS_SUBSCRIBERS)
		caps = ~SND_SEQ_PORT_CAP_SUBS_READ;
	my_port =
		snd_seq_create_simple_port(seq_handle, "SCUMMVM", caps,
															 SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
	if (my_port < 0) {
		snd_seq_close(seq_handle);
		error("Can't create port");
		return -1;
	}

	if (seq_client != SND_SEQ_ADDRESS_SUBSCRIBERS) {
		/* subscribe to MIDI port */
		if (snd_seq_connect_to(seq_handle, my_port, seq_client, seq_port) < 0) {
			snd_seq_close(seq_handle);
			error("Can't subscribe to MIDI port (%d:%d)", seq_client, seq_port);
			return -1;
		}
	}

	printf("ALSA client initialised [%d:%d]\n", my_client, my_port);

	return 0;
}

void MidiDriver_ALSA::close()
{
	_mode = 0;
	if (seq_handle)
		snd_seq_close(seq_handle);
}


void MidiDriver_ALSA::send(uint32 b)
{
	unsigned int midiCmd[4];
	ev.type = SND_SEQ_EVENT_OSS;

	midiCmd[3] = (b & 0xFF000000) >> 24;
	midiCmd[2] = (b & 0x00FF0000) >> 16;
	midiCmd[1] = (b & 0x0000FF00) >> 8;
	midiCmd[0] = (b & 0x000000FF);
	ev.data.raw32.d[0] = midiCmd[0];
	ev.data.raw32.d[1] = midiCmd[1];
	ev.data.raw32.d[2] = midiCmd[2];

	unsigned char chanID = midiCmd[0] & 0x0F;
	switch (midiCmd[0] & 0xF0) {
	case 0x80:
		snd_seq_ev_set_noteoff(&ev, chanID, midiCmd[1], midiCmd[2]);
		send_event(1);
		break;
	case 0x90:
		snd_seq_ev_set_noteon(&ev, chanID, midiCmd[1], midiCmd[2]);
		send_event(1);
		break;
	case 0xB0:
		/* is it this simple ? Wow... */
		snd_seq_ev_set_controller(&ev, chanID, midiCmd[1], midiCmd[2]);
		send_event(1);
		break;
	case 0xC0:
		snd_seq_ev_set_pgmchange(&ev, chanID, midiCmd[1]);
		send_event(0);
		break;

	case 0xE0:{
			long theBend = ((((long)midiCmd[1] + (long)(midiCmd[2] << 8))) - 0x4000) / 4;
			snd_seq_ev_set_pitchbend(&ev, chanID, theBend);
			send_event(1);
		}
		break;

	default:
		error("Unknown Command: %08x\n", (int)b);
		/* I don't know if this works but, well... */
		send_event(1);
		break;
	}
}

void MidiDriver_ALSA::pause(bool pause)
{
	if (_mode == MO_STREAMING) {
		/* Err... and what? */
	}
}

void MidiDriver_ALSA::set_stream_callback(void *param, StreamCallback *sc)
{
	_stream_param = param;
	_stream_proc = sc;
}

MidiDriver *MidiDriver_ALSA_create()
{
	return new MidiDriver_ALSA();
}

#endif
