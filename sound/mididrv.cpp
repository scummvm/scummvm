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
 * Alsa support by Nicolas Noble <nicolas@nobis-crew.org> copied from
 *    both the QuickTime support and (vkeybd http://www.alsa-project.org/~iwai/alsa.html)
 */

#include "stdafx.h"
#include "mididrv.h"
#include "fmopl.h"
#include "mixer.h"
#include "common/engine.h"	// for warning/error/debug
#include "common/util.h"

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

		// Send initial pitch bend sensitivity values for +/- 12 semitones.
		// For information on control change registered parameters,
		// which includes the Pitch Bend sensitivity settings,
		// visit http://www.midi.org/about-midi/table3.htm,
		// Table 3a.
		int chan;
		for (chan = 0; chan < 16; ++chan) {
			send(( 0 << 16) | (101 << 8) | (0xB0 | chan));
			send(( 0 << 16) | (100 << 8) | (0xB0 | chan));
			send((12 << 16) | (  6 << 8) | (0xB0 | chan));
			send(( 0 << 16) | ( 38 << 8) | (0xB0 | chan));
		} // next for
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
		warning("MM System Error '%s'", buf);
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
			long theBend = (long)midiCmd[1] | (long)(midiCmd[2] << 8);
			// QuickTime specifies pitchbend in semitones, using 8.8 fixed point values;
			// but iMuse sends us the pitch bend data relative to +/- 12 semitones. Thus
			// we have to convert it to the QT format.
			theBend = (theBend - 0x4000) * 6 / 128;
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
