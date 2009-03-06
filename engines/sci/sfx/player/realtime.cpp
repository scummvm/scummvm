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

/* OK... 'realtime' may be a little too euphemistic, as this one just
** prays for some reasonable amount of soft real-time, but it's close
** enough, I guess.  */

#include "sci/tools.h"
#include "sci/sfx/player.h"
#include "sci/sfx/sequencer.h"

#include "common/system.h"

namespace Sci {

static sfx_sequencer_t *seq;

extern sfx_player_t sfx_player_realtime;

/* Playing mechanism */

static inline int delta_time(const uint32 comp, const uint32 base) {
	return long(comp) - long(base);
}

static SongIterator *play_it = NULL;
static uint32 play_last_time;
static uint32 play_pause_started; /* Beginning of the last pause */
static uint32 play_pause_counter; /* Last point in time to mark a
				    ** play position augmentation  */
static int play_paused = 0;
static int play_it_done = 0;
static int play_writeahead = 0;
static int play_moredelay = 0;

static void play_song(SongIterator *it, uint32 *wakeup_time, int writeahead_time) {
	unsigned char buf[8];
	int result;

	if (play_paused) {
		uint32 ct = g_system->getMillis();

		*wakeup_time += delta_time(play_pause_counter, ct);

		play_pause_counter = ct;
	} else
		/* Not paused: */
		while (play_it && delta_time(*wakeup_time, g_system->getMillis())
		        < writeahead_time) {
			int delay;

			switch ((delay = songit_next(&(play_it),
			                             &(buf[0]), &result,
			                             IT_READER_MASK_ALL
			                             | IT_READER_MAY_FREE
			                             | IT_READER_MAY_CLEAN))) {

			case SI_FINISHED:
				play_it_done = 1;
				return;

			case SI_IGNORE:
			case SI_LOOP:
			case SI_RELATIVE_CUE:
			case SI_ABSOLUTE_CUE:
				break;

			case SI_PCM:
				sfx_play_iterator_pcm(play_it, 0);
				break;

			case 0:
				seq->event(buf[0], result - 1, buf + 1);

				break;

			default:
				play_moredelay = delay - 1;
				*wakeup_time += delay * SOUND_TICK;
				if (seq->delay)
					seq->delay(delay);
			}
		}
}

static void rt_tell_synth(int buf_nr, byte *buf) {
	seq->event(buf[0], buf_nr - 1, buf + 1);
}

static void rt_timer_callback(void) {
	if (play_it && !play_it_done) {
		if (!play_moredelay) {
			int delta = delta_time(play_last_time, g_system->getMillis());

			if (delta < 0) {
				play_writeahead -= (int)((double)delta * 1.2); /* Adjust upwards */
			} else if (delta > 15) {
				play_writeahead -= 3; /* Adjust downwards */
			}
		} else --play_moredelay;

		if (play_writeahead < seq->min_write_ahead_ms)
			play_writeahead = seq->min_write_ahead_ms;

		play_song(play_it, &play_last_time, play_writeahead);
	}
}

static Resource *find_patch(ResourceManager *resmgr, const char *seq_name, int patchfile) {
	Resource *res = NULL;

	if (patchfile != SFX_SEQ_PATCHFILE_NONE) {
		res = resmgr->findResource(kResourceTypePatch, patchfile, 0);
		if (!res) {
			fprintf(stderr, "[SFX] " __FILE__": patch.%03d requested by sequencer (%s), but not found\n",
			        patchfile, seq_name);
		}
	}

	return res;
}

/* API implementation */

static int rt_set_option(char *name, char *value) {
	return SFX_ERROR;
}

static int rt_init(ResourceManager *resmgr, int expected_latency) {
	Resource *res = NULL, *res2 = NULL;
	void *seq_dev = NULL;

	seq = sfx_find_sequencer(NULL);

	if (!seq) {
		fprintf(stderr, "[SFX] " __FILE__": Could not find sequencer\n");
		return SFX_ERROR;
	}

	sfx_player_realtime.polyphony = seq->polyphony;

	res = find_patch(resmgr, seq->name, seq->patchfile);
	res2 = find_patch(resmgr, seq->name, seq->patchfile2);

	if (seq->device)
		seq_dev = sfx_find_device(seq->device, NULL);

	if (seq->open(res ? res->size : 0,
	              res ? res->data : NULL,
	              res2 ? res2->size : 0,
	              res2 ? res2->data : NULL,
	              seq_dev)) {
		fprintf(stderr, "[SFX] " __FILE__": Sequencer failed to initialize\n");
		return SFX_ERROR;
	}

	play_writeahead = expected_latency;
	if (play_writeahead < seq->min_write_ahead_ms)
		play_writeahead = seq->min_write_ahead_ms;

	play_writeahead *= 1; /* milliseconds */

	if (seq->reset_timer)
		seq->reset_timer(0);

	return SFX_OK;
}

static int rt_add_iterator(SongIterator *it, uint32 start_time) {
	if (seq->reset_timer) /* Restart timer counting if possible */
		seq->reset_timer(start_time);

	SIMSG_SEND(it, SIMSG_SET_PLAYMASK(seq->playmask));
	SIMSG_SEND(it, SIMSG_SET_RHYTHM(seq->play_rhythm));

	play_last_time = start_time;
	play_it = sfx_iterator_combine(play_it, it);
	play_it_done = 0;
	play_moredelay = 0;

	return SFX_OK;
}

static int rt_fade_out(void) {
	fprintf(stderr, __FILE__": Attempt to fade out- not implemented yet\n");
	return SFX_ERROR;
}

static int rt_stop(void) {
	SongIterator *it = play_it;

	play_it = NULL;

	delete it;
	if (seq && seq->allstop)
		seq->allstop();

	return SFX_OK;
}

static int rt_send_iterator_message(SongIteratorMessage msg) {
	if (!play_it)
		return SFX_ERROR;

	songit_handle_message(&play_it, msg);
	return SFX_OK;
}

static int rt_pause(void) {
	play_pause_started = g_system->getMillis();
	/* Also, indicate that we haven't modified the time counter
	** yet  */
	play_pause_counter = play_pause_started;

	play_paused = 1;
	if (!seq->allstop) {
		sciprintf("[SFX] Cannot suspend sequencer, sound will continue for a bit\n");
		return SFX_OK;
	} else
		return seq->allstop();
}

static int rt_resume(void) {
	play_paused = 0;
	return SFX_OK;
}

static int rt_exit(void) {
	int retval = SFX_OK;

	if (seq->close()) {
		fprintf(stderr, "[SFX] Sequencer reported error on close\n");
		retval = SFX_ERROR;
	}

	return retval;
}

sfx_player_t sfx_player_realtime = {
	"realtime",
	"0.1",
	&rt_set_option,
	&rt_init,
	&rt_add_iterator,
	&rt_fade_out,
	&rt_stop,
	&rt_send_iterator_message,
	&rt_pause,
	&rt_resume,
	&rt_exit,
	&rt_timer_callback,
	&rt_tell_synth,
	0 /* polyphony */
};

} // End of namespace Sci
