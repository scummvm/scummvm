/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(UNIX) && defined(USE_ALSA)

#include "sound/mpu401.h"

#include "common/stdafx.h"
#include "common/util.h"
#include "common/config-manager.h"

#include <alsa/asoundlib.h>

/*
 *     ALSA sequencer driver
 * Mostly cut'n'pasted from Virtual Tiny Keyboard (vkeybd) by Takashi Iwai
 *                                      (you really rox, you know?)
 */

#if SND_LIB_MAJOR >= 1 || SND_LIB_MINOR >= 6
#define snd_seq_flush_output(x) snd_seq_drain_output(x)
#define snd_seq_set_client_group(x,name)	/*nop */
#define my_snd_seq_open(seqp) snd_seq_open(seqp, "hw", SND_SEQ_OPEN_DUPLEX, 0)
#else
/* SND_SEQ_OPEN_OUT causes oops on early version of ALSA */
#define my_snd_seq_open(seqp) snd_seq_open(seqp, SND_SEQ_OPEN)
#endif

/*
 * parse address string
 */

#define ADDR_DELIM      ".:"

class MidiDriver_ALSA:public MidiDriver_MPU401 {
public:
	MidiDriver_ALSA();
	int open();
	void close();
	void send(uint32 b);
	void sysEx(byte *msg, uint16 length);

private:
	void send_event(int do_flush);
	bool _isOpen;
	snd_seq_event_t ev;
	snd_seq_t *seq_handle;
	int seq_client, seq_port;
	int my_client, my_port;
	static int parse_addr(const char *arg, int *client, int *port);
};

MidiDriver_ALSA::MidiDriver_ALSA()
 : _isOpen(false), seq_handle(0), seq_client(0), seq_port(0), my_client(0), my_port(0)
{
}

int MidiDriver_ALSA::open() {
	char *var;

	if (_isOpen)
		return MERR_ALREADY_OPEN;
	_isOpen = true;

	if (!(var = getenv("SCUMMVM_PORT"))) {
		// use config option if no var specified
		if (parse_addr(ConfMan.get("alsa_port").c_str(), &seq_client, &seq_port) < 0) {
			error("Invalid port %s", var);
			return -1;
		}
	} else {	
		if (parse_addr(var, &seq_client, &seq_port) < 0) {
			error("Invalid port %s", var);
			return -1;
		}
	}

	if (my_snd_seq_open(&seq_handle) < 0) {
		error("Can't open sequencer");
		return -1;
	}

	my_client = snd_seq_client_id(seq_handle);
	if (snd_seq_set_client_name(seq_handle, "SCUMMVM") < 0) {
		error("Can't set sequencer client name");
	}
	snd_seq_set_client_group(seq_handle, "input");

	my_port = snd_seq_create_simple_port(seq_handle, "SCUMMVM port 0",
		SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE |
		SND_SEQ_PORT_CAP_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC);

	if (my_port < 0) {
		snd_seq_close(seq_handle);
		error("Can't create port");
		return -1;
	}

	if (seq_client != SND_SEQ_ADDRESS_SUBSCRIBERS) {
		/* subscribe to MIDI port */
		if (snd_seq_connect_to(seq_handle, my_port, seq_client, seq_port) < 0) {
			error("Can't subscribe to MIDI port (%d:%d) see README for help", seq_client, seq_port);
		}
		else printf("Connected to Alsa sequencer client [%d:%d]\n", seq_client, seq_port);
	}

	printf("ALSA client initialised [%d:0]\n", my_client);

	return 0;
}

void MidiDriver_ALSA::close() {
	_isOpen = false;
	MidiDriver_MPU401::close();
	if (seq_handle)
		snd_seq_close(seq_handle);
}

void MidiDriver_ALSA::send(uint32 b) {
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
	case 0xD0:
		snd_seq_ev_set_chanpress(&ev, chanID, midiCmd[1]);
		send_event(0);
		break;
	case 0xE0:{
			// long theBend = ((((long)midiCmd[1] + (long)(midiCmd[2] << 7))) - 0x2000) / 4;
			// snd_seq_ev_set_pitchbend(&ev, chanID, theBend);
			long theBend = ((long)midiCmd[1] + (long)(midiCmd[2] << 7)) - 0x2000;
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

void MidiDriver_ALSA::sysEx(byte *msg, uint16 length) {
	unsigned char buf[1024];

	if (length > 254) {
		warning("Cannot send SysEx block - data too large");
		return;
	}
	buf[0] = 0xF0;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;
	snd_seq_ev_set_sysex(&ev, length + 2, &buf);
	send_event(1);
}

int MidiDriver_ALSA::parse_addr(const char *arg, int *client, int *port) {
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

void MidiDriver_ALSA::send_event(int do_flush) {
	snd_seq_ev_set_direct(&ev);
	snd_seq_ev_set_source(&ev, my_port);
	snd_seq_ev_set_dest(&ev, seq_client, seq_port);

	snd_seq_event_output(seq_handle, &ev);
	if (do_flush)
		snd_seq_flush_output(seq_handle);
}

MidiDriver *MidiDriver_ALSA_create() {
	return new MidiDriver_ALSA();
}

#endif
