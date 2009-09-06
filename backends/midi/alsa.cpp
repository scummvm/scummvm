/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "common/scummsys.h"

#if defined(UNIX) && defined(USE_ALSA)

#include "common/config-manager.h"
#include "common/util.h"
#include "sound/musicplugin.h"
#include "sound/mpu401.h"

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
	void sysEx(const byte *msg, uint16 length);

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
	memset(&ev, 0, sizeof(ev));
}

int MidiDriver_ALSA::open() {
	const char *var = NULL;

	if (_isOpen)
		return MERR_ALREADY_OPEN;
	_isOpen = true;

	var = getenv("SCUMMVM_PORT");
	if (!var && ConfMan.hasKey("alsa_port")) {
		var = ConfMan.get("alsa_port").c_str();
	}

	if (var) {
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

	if (var) {
		if (seq_client != SND_SEQ_ADDRESS_SUBSCRIBERS) {
			// subscribe to MIDI port
			if (snd_seq_connect_to(seq_handle, my_port, seq_client, seq_port) < 0) {
				error("Can't subscribe to MIDI port (%d:%d) see README for help", seq_client, seq_port);
			}
		}
	} else {
		int defaultPorts[] = {
			65, 0,
			17, 0
		};
		int i;

		for (i = 0; i < ARRAYSIZE(defaultPorts); i += 2) {
			seq_client = defaultPorts[i];
			seq_port = defaultPorts[i + 1];
			if (snd_seq_connect_to(seq_handle, my_port, seq_client, seq_port) >= 0)
				break;
		}

		if (i >= ARRAYSIZE(defaultPorts))
			error("Can't subscribe to MIDI port (65:0) or (17:0)");
	}

	printf("Connected to Alsa sequencer client [%d:%d]\n", seq_client, seq_port);
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
	case 0xA0:
		snd_seq_ev_set_keypress(&ev, chanID, midiCmd[1], midiCmd[2]);
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
		send_event(1);
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
		warning("Unknown MIDI Command: %08x", (int)b);
		/* I don't know if this works but, well... */
		send_event(1);
		break;
	}
}

void MidiDriver_ALSA::sysEx(const byte *msg, uint16 length) {
	unsigned char buf[266];

	assert(length + 2 <= ARRAYSIZE(buf));

	// Add SysEx frame
	buf[0] = 0xF0;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;

	// Send it
	snd_seq_ev_set_sysex(&ev, length + 2, &buf);
	send_event(1);
}

int MidiDriver_ALSA::parse_addr(const char *arg, int *client, int *port) {
	const char *p;

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


// Plugin interface

class AlsaMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "ALSA";
	}

	const char *getId() const {
		return "alsa";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(Audio::Mixer *mixer, MidiDriver **mididriver) const;
};

#define perm_ok(pinfo,bits) ((snd_seq_port_info_get_capability(pinfo) & (bits)) == (bits))

static int check_permission(snd_seq_port_info_t *pinfo)
{
	if (perm_ok(pinfo, SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE)) {
		if (!(snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_NO_EXPORT))
			return 1;
	}
	return 0;
}

MusicDevices AlsaMusicPlugin::getDevices() const {
	MusicDevices devices;

	snd_seq_t *seq;
	if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0)
		return devices; // can't open sequencer

	snd_seq_client_info_t *cinfo;
	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_t *pinfo;
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_client_info_set_client(cinfo, -1);
	while (snd_seq_query_next_client(seq, cinfo) >= 0) {
		bool found_valid_port = false;

		/* reset query info */
		snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
		snd_seq_port_info_set_port(pinfo, -1);
		while (!found_valid_port && snd_seq_query_next_port(seq, pinfo) >= 0) {
			if (check_permission(pinfo)) {
				found_valid_port = true;
				// TODO: Return a different music type depending on the configuration
				devices.push_back(MusicDevice(this, snd_seq_client_info_get_name(cinfo), MT_GM));
				//snd_seq_client_info_get_client(cinfo) : snd_seq_port_info_get_port(pinfo)
			}
		}
	}
	snd_seq_close(seq);

	return devices;
}

Common::Error AlsaMusicPlugin::createInstance(Audio::Mixer *mixer, MidiDriver **mididriver) const {
	*mididriver = new MidiDriver_ALSA();

	return Common::kNoError;
}

MidiDriver *MidiDriver_ALSA_create(Audio::Mixer *mixer) {
	MidiDriver *mididriver;

	AlsaMusicPlugin p;
	p.createInstance(mixer, &mididriver);

	return mididriver;
}

//#if PLUGIN_ENABLED_DYNAMIC(ALSA)
	//REGISTER_PLUGIN_DYNAMIC(ALSA, PLUGIN_TYPE_MUSIC, AlsaMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(ALSA, PLUGIN_TYPE_MUSIC, AlsaMusicPlugin);
//#endif

#endif
