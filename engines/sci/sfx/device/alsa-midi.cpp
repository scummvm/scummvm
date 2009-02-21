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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include <sfx_engine.h>
#include "../device.h"
#ifdef HAVE_ALSA

#include <alsa/asoundlib.h>

namespace Sci {

#define SCI_ALSA_MIDI_VERSION "0.1"

static snd_midi_event_t *parser = NULL;
static snd_seq_t *seq = NULL;
static int queue = -1;
static int delta = 0;
static int port_out = -1;
static int port_nr = 128;
static int subport_nr = 0;

static const char *seq_name = "default";

static void
_set_tempo(void) {
	int resolution = 60;
	int tempo = 1;
	snd_seq_queue_tempo_t *queue_tempo;

	snd_seq_queue_tempo_malloc(&queue_tempo);

	memset(queue_tempo, 0, snd_seq_queue_tempo_sizeof());
	snd_seq_queue_tempo_set_ppq(queue_tempo, resolution);
	snd_seq_queue_tempo_set_tempo(queue_tempo, 1000000 / tempo);

	snd_seq_set_queue_tempo(seq, queue, queue_tempo);

	snd_seq_queue_tempo_free(queue_tempo);

#if 0
	int tempo = 1000000 / 60;
	snd_seq_queue_tempo_t *queue_tempo;

	snd_seq_queue_tempo_malloc(&queue_tempo);
	snd_seq_queue_tempo_set_tempo(queue_tempo, tempo);
	snd_seq_queue_tempo_set_ppq(queue_tempo, 1);
	snd_seq_set_queue_tempo(seq, queue, queue_tempo);
	snd_seq_queue_tempo_free(queue_tempo);
#endif
}


static int
am_subscribe_to_ports(void) {
	if ((port_out = snd_seq_connect_to(seq, port_out, port_nr, subport_nr)) < 0) {
		fprintf(stderr, "[SFX] Could not connect to ALSA sequencer port: %s\n", snd_strerror(port_out));
		return SFX_ERROR;
	}
	return SFX_OK;
}


static int
aminit(midi_writer_t *self) {
	int err;

	snd_midi_event_new(4096, &parser);
	snd_midi_event_init(parser);

	sciprintf("[SFX] Initialising ALSA MIDI backend, v%s\n", SCI_ALSA_MIDI_VERSION);

	if (snd_seq_open(&seq, seq_name, SND_SEQ_OPEN_OUTPUT, SND_SEQ_NONBLOCK)) {
		fprintf(stderr, "[SFX] Failed to open ALSA MIDI sequencer '%s' for output\n",
		        seq_name);
		return SFX_ERROR;
	}

	if ((port_out = snd_seq_create_simple_port(seq, "FreeSCI",
	                SND_SEQ_PORT_CAP_WRITE |
	                SND_SEQ_PORT_CAP_SUBS_WRITE |
	                SND_SEQ_PORT_CAP_READ,
	                SND_SEQ_PORT_TYPE_MIDI_GENERIC)) < 0) {
		fprintf(stderr, "[SFX] Could not create ALSA sequencer port\n");
		return SFX_ERROR;
	}

	if (am_subscribe_to_ports())
		return SFX_ERROR;

	queue = snd_seq_alloc_queue(seq);
	_set_tempo();

	snd_seq_start_queue(seq, queue, NULL);

	if ((err = snd_seq_drain_output(seq))) {
		fflush(NULL);
		fprintf(stderr, "[SFX] Error while draining: %s\n",
		        snd_strerror(err));
		return SFX_ERROR;
	}

	return SFX_OK;
}

static int
amsetopt(midi_writer_t *self, char *name, char *value) {
	return SFX_ERROR;
}


static int
amwrite(midi_writer_t *self, unsigned char *buf, int len) {
	snd_seq_event_t evt;

#if 0
	{
		int i;
		fprintf(stderr, "[MID] ");
		for (i = 0; i < len; i++)
			fprintf(stderr, " %02x", buf[i]);
		fprintf(stderr, "\n");
	}
#endif

	snd_seq_ev_clear(&evt);
	snd_seq_ev_set_source(&evt, port_out);
	snd_seq_ev_set_subs(&evt); /* Broadcast to all subscribers */

	snd_midi_event_encode(parser, buf, len, &evt);
	snd_seq_ev_schedule_tick(&evt, queue, 0, delta);

	snd_seq_event_output_direct(seq, &evt);

#if 0
	{
		snd_seq_queue_status_t *status;
		snd_seq_queue_status_malloc(&status);

		snd_seq_get_queue_status(seq, queue, status);
		//snd_seq_tick_time_t snd_seq_queue_status_get_tick_time(const snd_seq_queue_status_t *info);
		fprintf(stderr, "Queue at %d/%d\n", delta, snd_seq_queue_status_get_tick_time(status));

		snd_seq_queue_status_free(status);
	}
#endif


	return SFX_OK;
}

static void
amdelay(midi_writer_t *self, int ticks) {
	delta += ticks;
}

static void
amreset_timer(midi_writer_t *self) {
	snd_seq_drain_output(seq);
	snd_seq_stop_queue(seq, queue, NULL);


	{
		snd_seq_event_t evt;
		snd_seq_ev_clear(&evt);
		snd_seq_ev_set_source(&evt, port_out);
		snd_seq_ev_set_subs(&evt); /* Broadcast to all subscribers */

		snd_seq_ev_set_queue_pos_tick(&evt, queue, 0);

		snd_seq_event_output_direct(seq, &evt);
	}
	delta = 0;



	snd_seq_start_queue(seq, queue, NULL);
}

static void
amclose(midi_writer_t *self) {
	snd_midi_event_free(parser);
	parser = NULL;
}


midi_writer_t sfx_device_midi_alsa = {
	"alsa",
	aminit,
	amsetopt,
	amwrite,
	amdelay,
	NULL,
	amreset_timer,
	amclose,
};

} // End of namespace Sci

#endif
