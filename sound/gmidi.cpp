 /* ScummVM - Scumm Interpreter
 * Copyright (C) 2001 The ScummVM project
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
#include "sound.h"
#include "gmidi.h"

void MidiSoundDriver::midiSetDriver(int devicetype) {	
	_midi_driver.DeviceType = devicetype;
	_midi_driver.midiInit();
}

void MidiDriver::midiInit() {
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

void MidiDriver::MidiOut(int b) {
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
void MidiDriver::midiInitWindows() {
	#ifdef WIN32
		if (midiOutOpen((HMIDIOUT*)&_mo, MIDI_MAPPER, NULL, NULL, 0) != MMSYSERR_NOERROR)
			error("midiOutOpen failed");
	#endif
}

void MidiDriver::MidiOutWindows(void *a, int b) {
	#ifdef WIN32
	midiOutShortMsg((HMIDIOUT) a, b);
	#endif	
}

/*********** Raw midi support	*/
void MidiDriver::midiInitSeq() {
	int device = open_sequencer_device();
	_mo = (void *) device;
}

int MidiDriver::open_sequencer_device() {
	int device = 0;
#if !defined(__APPLE__CW)		// No getenv support on Apple Carbon
	char *device_name = getenv("SCUMMVM_MIDI");
	if (device_name != NULL) {
		device = (open((device_name), O_RDWR, 0));
	} else {
		warning("You need to set-up the SCUMMVM_MIDI environment variable properly (see readme.txt) ");
	}
	if ((device_name == NULL) || (device < 0)) {
		if (device_name == NULL)
			warning("Opening /dev/null (no music will be heard) ");
		else
			warning("Cannot open rawmidi device %s - using /dev/null (no music will be heard) ", device_name);
		device = (open(("/dev/null"), O_RDWR, 0));
		if (device < 0)
			error("Cannot open /dev/null to dump midi output");
	}
#endif
	return device;
}

/*********** Timidity		*/
int MidiDriver::connect_to_timidity(int port) {
	int s = 0;
#if !defined(__APPLE__CW) && !defined(__MORPHOS__)  // No socket support on Apple Carbon or Morphos
	struct hostent *serverhost;
	struct sockaddr_in sadd;	

	serverhost = gethostbyname("localhost");
	if (serverhost == NULL)
		error("Could not resolve Timidity host ('localhost')");
	
	sadd.sin_family = serverhost->h_addrtype;
	sadd.sin_port = htons(port);
	memcpy(&(sadd.sin_addr), serverhost->h_addr_list[0], serverhost->h_length);

	s = socket(AF_INET,SOCK_STREAM,0);
	if (s < 0)
		error("Could not open Timidity socket");

	if (connect(s, (struct sockaddr *) &sadd, sizeof(struct sockaddr_in)) < 0)
		error("Could not connect to Timidity server");	
#endif
	return s;
}

void MidiDriver::midiInitTimidity() {
	int s, s2;
	int len;
	int dummy, newport;
	char buf[256];

	s = connect_to_timidity(7777);
	len = read(s, buf, 256); // buf[len] = '\0'; printf("%s", buf);
	sprintf(buf, "SETBUF %f %f\n", 0.1, 0.15);
	write(s, buf, strlen(buf));
	len = read(s, buf, 256); // buf[len] = '\0'; printf("%s", buf);	

	sprintf(buf, "OPEN lsb\n");
	write(s, buf, strlen(buf));
	len = read(s, buf, 256); // buf[len] = '\0'; printf("%s", buf);	

	sscanf(buf, "%d %d", &dummy, &newport);
	printf("	 => port = %d\n", newport);

	s2 = connect_to_timidity(newport);
	_mo = (void *) s2;
}

void MidiDriver::MidiOutSeq(void *a, int b) {
	int s = (int) a;
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
void MidiDriver::midiInitQuicktime() {
#ifdef __APPLE__CW
	ComponentResult qtErr = noErr;
 	qtNoteAllocator = NULL;

	for (int i = 0 ; i < 15 ; i++)
 		qtNoteChannel[i] = NULL;

 	qtNoteAllocator = OpenDefaultComponent(kNoteAllocatorComponentType, 0);
 	if (qtNoteAllocator == NULL)
 		goto bail;
 		
 	simpleNoteRequest.info.flags = 0;
 	*(short *)(&simpleNoteRequest.info.polyphony) = EndianS16_NtoB(15);				// simultaneous tones
 	*(Fixed *)(&simpleNoteRequest.info.typicalPolyphony) = EndianU32_NtoB(0x00010000);
 	
	qtErr = NAStuffToneDescription(qtNoteAllocator, 1, &simpleNoteRequest.tone);
 	if (qtErr != noErr)
 		goto bail;
 	
 	for (int i = 0 ; i < 15 ; i++) {
 		qtErr = NANewNoteChannel(qtNoteAllocator, &simpleNoteRequest, &(qtNoteChannel[i]));
		if ((qtErr != noErr) || (qtNoteChannel == NULL))
 			goto bail;
 	}
 	return;
 	
	bail:
 		fprintf(stderr, "Init QT failed %x %x %d\n", qtNoteAllocator, qtNoteChannel, qtErr);
 		for (int i = 0 ; i < 15 ; i++) {
 		if (qtNoteChannel[i] != NULL)
 			NADisposeNoteChannel(qtNoteAllocator, qtNoteChannel[i]);
 		}
 	
 		if (qtNoteAllocator != NULL)
 			CloseComponent(qtNoteAllocator);
#endif
}

void MidiDriver::MidiOutQuicktime(void *a, int b) {
#ifdef __APPLE__CW
	MusicMIDIPacket midPacket;
	unsigned char *midiCmd = midPacket.data;
	midPacket.length = 3;
	midiCmd[3] = (b & 0xFF000000)>>24;
	midiCmd[2] = (b & 0x00FF0000)>>16;
	midiCmd[1] = (b & 0x0000FF00)>>8;
	midiCmd[0] = b;

	unsigned char chanID =  midiCmd[0] & 0x0F;
	switch (midiCmd[0] & 0xF0) {
		case 0x80: // Note off
			NAPlayNote(qtNoteAllocator, qtNoteChannel[chanID], midiCmd[1], 0);
		break;
		
		case 0x90: // Note on
			NAPlayNote(qtNoteAllocator, qtNoteChannel[chanID], midiCmd[1], midiCmd[2]);
		break;
		
		case 0xB0: // Effect
			switch (midiCmd[1]) {
				case 0x01: // Modulation
					NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerModulationWheel, midiCmd[2]<<8);
				break;

				case 0x07: // Volume
					NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerVolume, midiCmd[2]*300);
	 			break;

		 		case 0x0A: // Pan
		 			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerPan, (midiCmd[2]<<1)+0xFF);
	 			break;

		 		case 0x40: // Sustain on/off
		 			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerSustain, midiCmd[2]);
 				break;

 				case 0x5b: // ext effect depth
 					NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerReverb, midiCmd[2]<<8);
 				break;

 				case 0x5d: // chorus depth
	 				NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerChorus, midiCmd[2]<<8);
 				break;

 				case 0x7b: // mode message all notes off
 					for (int i = 0 ; i < 128 ; i++)
 						NAPlayNote(qtNoteAllocator, qtNoteChannel[chanID], i, 0);
 				break;

 				default:
 					fprintf(stderr, "Unknown MIDI effect: %08x\n", b);
 				break;
 		}
	 	break;
 			
		case 0xC0: // Program change
 			NASetInstrumentNumber(qtNoteAllocator, qtNoteChannel[chanID], midiCmd[1]); 
  		break;
 			
		case 0xE0: { // Pitch bend
 			long theBend = ((((long)midiCmd[1] + (long)(midiCmd[2] << 8)))-0x4000)/4;
 			NASetController(qtNoteAllocator, qtNoteChannel[chanID], kControllerPitchBend, theBend);
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
void MidiDriver::MidiOutMorphOS(void *a, int b) {
#ifdef __MORPHOS__
      if( ScummMidiRequest ) {
              ULONG midi_data = b;    // you never know about an int's size ;-)
              ScummMidiRequest->amr_Std.io_Command = CMD_WRITE;
              ScummMidiRequest->amr_Std.io_Data = &midi_data;
              ScummMidiRequest->amr_Std.io_Length = 4;
              DoIO( (struct IORequest *)ScummMidiRequest );
      }
#endif
}







void MidiDriver::midiInitNull() {warning("Music not enabled - MIDI support selected with no MIDI driver available. Try Adlib");}



/************************* Common midi code **********************/
void MidiSoundDriver::midiPitchBend(byte chan, int16 pitchbend) {
	uint16 tmp;

	if (_midi_pitchbend_last[chan] != pitchbend) {
		_midi_pitchbend_last[chan] = pitchbend;
		tmp = (pitchbend<<2) + 0x2000;
		_midi_driver.MidiOut(((tmp>>7)&0x7F)<<16 | (tmp&0x7F)<<8 | 0xE0 | chan);
	}
}

void MidiSoundDriver::midiVolume(byte chan, byte volume) {
	if (_midi_volume_last[chan] != volume) {
		_midi_volume_last[chan] = volume;
		_midi_driver.MidiOut(volume<<16 | 7<<8 | 0xB0 | chan);
	}
}
void MidiSoundDriver::midiPedal(byte chan, bool pedal) {
	if (_midi_pedal_last[chan] != pedal) {
		_midi_pedal_last[chan] = pedal;
		_midi_driver.MidiOut(pedal<<16 | 64<<8 | 0xB0 | chan);
	}
}

void MidiSoundDriver::midiModWheel(byte chan, byte modwheel) {
	if (_midi_modwheel_last[chan] != modwheel) {
		_midi_modwheel_last[chan] = modwheel;
		_midi_driver.MidiOut(modwheel<<16 | 1<<8 | 0xB0 | chan);
	}
}

void MidiSoundDriver::midiEffectLevel(byte chan, byte level) {
	if (_midi_effectlevel_last[chan] != level) {
		_midi_effectlevel_last[chan] = level;
		_midi_driver.MidiOut(level<<16 | 91<<8 | 0xB0 | chan);
	}
}

void MidiSoundDriver::midiChorus(byte chan, byte chorus) {
	if (_midi_chorus_last[chan] != chorus) {
		_midi_chorus_last[chan] = chorus;
		_midi_driver.MidiOut(chorus<<16 | 93<<8 | 0xB0 | chan);
	}
}

void MidiSoundDriver::midiControl0(byte chan, byte value) {
	_midi_driver.MidiOut(value<<16 | 0<<8 | 0xB0 | chan);
}

void MidiSoundDriver::midiProgram(byte chan, byte program) {
	if ((chan + 1) != 10) {	/* Ignore percussion prededed by patch change */
		if (_se->_mt32emulate)			
			program=mt32_to_gmidi[program];
		
		_midi_driver.MidiOut(program<<8 | 0xC0 | chan);
	}
}

void MidiSoundDriver::midiPan(byte chan, int8 pan) {
	if (_midi_pan_last[chan] != pan) {
		_midi_pan_last[chan] = pan;
		_midi_driver.MidiOut(((pan-64)&0x7F)<<16 | 10<<8 | 0xB0 | chan);
	}
}

void MidiSoundDriver::midiNoteOn(byte chan, byte note, byte velocity) {
	_midi_driver.MidiOut(velocity<<16 | note<<8 | 0x90 | chan);	
}

void MidiSoundDriver::midiNoteOff(byte chan, byte note) {
	_midi_driver.MidiOut(note<<8 | 0x80 | chan);	
}

void MidiSoundDriver::midiSilence(byte chan) {
	_midi_driver.MidiOut((64<<8)|0xB0|chan);
	_midi_driver.MidiOut((123<<8)|0xB0|chan);
}


void MidiSoundDriver::part_key_on(Part *part, byte note, byte velocity) {
	MidiChannelGM *mc = part->_mc->gm();

	if (mc) {
		mc->_actives[note>>4] |= (1<<(note&0xF));
		midiNoteOn(mc->_chan, note, velocity);
	} else if (part->_percussion) {
		midiVolume(SPECIAL_CHANNEL, part->_vol_eff);
		midiProgram(SPECIAL_CHANNEL, part->_bank);
		midiNoteOn(SPECIAL_CHANNEL, note, velocity);
	}
}

void MidiSoundDriver::part_key_off(Part *part, byte note) {
	MidiChannelGM *mc = part->_mc->gm();

	if (mc) {
		mc->_actives[note>>4] &= ~(1<<(note&0xF));
		midiNoteOff(mc->_chan, note);
	} else if (part->_percussion) {
		midiNoteOff(SPECIAL_CHANNEL, note);
	}
}

void MidiSoundDriver::init(SoundEngine *eng) {
	int i;
	MidiChannelGM *mc;

	_se = eng;

	for(i=0,mc=_midi_channels; i!=ARRAYSIZE(_midi_channels);i++,mc++)
		mc->_chan = i;
}

void MidiSoundDriver::update_pris() {
	Part *part,*hipart;
	int i;
	byte hipri,lopri;
	MidiChannelGM *mc,*lomc;

	while(true) {
		hipri = 0;
		hipart = NULL;
		for(i=32,part=_se->parts_ptr(); i; i--,part++) {
			if (part->_player && !part->_percussion && part->_on && !part->_mc && part->_pri_eff>=hipri) {
				hipri = part->_pri_eff;
				hipart = part;
			}
		}

		if (!hipart)
			return;

		lopri = 255;
		lomc = NULL;
		for(i=ARRAYSIZE(_midi_channels),mc=_midi_channels;;mc++) {
			if (!mc->_part) {
				lomc = mc;
				break;
			}
			if (mc->_part->_pri_eff<=lopri) {
				lopri = mc->_part->_pri_eff;
				lomc = mc;
			}

			if (!--i) {
				if (lopri >= hipri)
					return;
				lomc->_part->off();
				break;
			}
		}

		hipart->_mc = lomc;
		lomc->_part = hipart;
		hipart->changed(pcAll);
	}
}

int MidiSoundDriver::part_update_active(Part *part, uint16 *active) {
	int i,j;
	uint16 *act,mask,bits;
	int count = 0;

	bits = 1<<part->_chan;

	act = part->_mc->gm()->_actives;

	for(i=8; i; i--) {
		mask = *act++;
		if (mask) {
			for(j=16; j; j--,mask>>=1,active++) {
				if (mask&1 && !(*active&bits)) {
					*active|=bits;
					count++;
				}
			}
		} else {
			active += 16;
		}
	}
	return count;
}

void MidiSoundDriver::part_changed(Part *part, byte what) {
	MidiChannelGM *mc;

	/* Mark for re-schedule if program changed when in pre-state */
	if (what&pcProgram && part->_percussion) {
		part->_percussion = false;
		update_pris();
	}
	
	if (!(mc = part->_mc->gm()))
		return;

	if (what & pcMod)
		midiPitchBend(mc->_chan, clamp(part->_pitchbend + part->_detune_eff + (part->_transpose_eff<<7), -2048, 2047));	

	if (what & pcVolume)
		midiVolume(mc->_chan, part->_vol_eff);

	if (what & pcPedal)
		midiPedal(mc->_chan, part->_pedal);

	if (what & pcModwheel)
		midiModWheel(mc->_chan, part->_modwheel);

	if (what & pcPan)
		midiPan(mc->_chan, part->_pan_eff);

	if (what & pcEffectLevel)
		midiEffectLevel(mc->_chan, part->_effect_level);

	if (what & pcProgram) {
		if (part->_bank) {
			midiControl0(mc->_chan, part->_bank);
			midiProgram(mc->_chan, part->_program);
			midiControl0(mc->_chan, 0);
		} else {
			midiProgram(mc->_chan, part->_program);
		}
	}

	if (what & pcChorus)
		midiChorus(mc->_chan, part->_effect_level);
}


void MidiSoundDriver::part_off(Part *part) {
	MidiChannelGM *mc = part->_mc->gm();
	if (mc) {
		part->_mc = NULL;
		mc->_part = NULL;
		memset(mc->_actives, 0, sizeof(mc->_actives));
		midiSilence(mc->_chan);
	}
}
