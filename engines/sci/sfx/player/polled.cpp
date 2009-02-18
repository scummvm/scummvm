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

/* Polled player, mostly for PCM-based thingies (which _can_ poll, after all) */

#include "sci/include/sfx_player.h"
#include "sci/sfx/softseq.h"
#include "sci/sfx/mixer.h"

#include "common/file.h"

static song_iterator_t *play_it;
static int play_paused = 0;
static sfx_softseq_t *seq;
static int volume = 100;
static sfx_timestamp_t new_timestamp;
static int new_song = 0;

/* The time counter is used to determine how close to the end of a tick we are.
** For each frame played, it is decreased by 60.  */
#define TIME_INC 60
static int time_counter = 0;

static void
pp_tell_synth(int buf_nr, byte *buf) {
	seq->handle_command(seq, buf[0], buf_nr - 1, buf + 1);
}


/*----------------------*/
/* Mixer implementation */
/*----------------------*/
int
ppf_poll(sfx_pcm_feed_t *self, byte *dest, int size) {
	int written = 0;
	byte buf[4];
	int buf_nr;

	if (!play_it)
		return 0;

	if (play_paused)
		return 0;

	while (written < size) {
		int can_play;
		int do_play;

		while (time_counter <= TIME_INC) {
			int next_stat = songit_next(&play_it,
			                            &(buf[0]), &buf_nr,
			                            IT_READER_MASK_ALL
			                            | IT_READER_MAY_FREE
			                            | IT_READER_MAY_CLEAN);

			switch (next_stat) {
			case SI_PCM:
				sfx_play_iterator_pcm(play_it, 0);
				break;

			case SI_FINISHED:
				songit_free(play_it);
				play_it = NULL;
				return written; /* We're done... */

			case SI_IGNORE:
			case SI_LOOP:
			case SI_RELATIVE_CUE:
			case SI_ABSOLUTE_CUE:
				break; /* Boooring... .*/

			case 0: /* MIDI command */

				seq->handle_command(seq, buf[0], buf_nr - 1, buf + 1);
				break;

			default:
				time_counter += next_stat * seq->pcm_conf.rate;
			}
		}

		can_play = time_counter / TIME_INC;
		do_play = (can_play > (size - written)) ? (size - written) : can_play;

		time_counter -= do_play * TIME_INC;

		seq->poll(seq, dest + written * self->frame_size, do_play);
		written += do_play;
	}

	return size; /* Apparently, we wrote all that was requested */
}

void
ppf_destroy(sfx_pcm_feed_t *self) {
	/* no-op */
}

int
ppf_get_timestamp(sfx_pcm_feed_t *self, sfx_timestamp_t *timestamp) {
	if (!new_song)
		return PCM_FEED_IDLE;

	/* Otherwise, we have a timestamp: */

	*timestamp = new_timestamp;
	new_song = 0;
	return PCM_FEED_TIMESTAMP;
}

extern sfx_player_t sfx_player_polled;
static
sfx_pcm_feed_t pcmfeed = {
	ppf_poll,
	ppf_destroy,
	ppf_get_timestamp,
	NULL,
	{0, 0, 0},
	"polled-player-feed",
	0,
	0 /* filled in by the mixer */
};

/*=======================*/
/* Player implementation */
/*=======================*/


/*--------------------*/
/* API implementation */
/*--------------------*/

static void
pp_timer_callback(void) {
	/* Hey, we're polled anyway ;-) */
}

static int
pp_set_option(char *name, char *value) {
	return SFX_ERROR;
}

static int
pp_init(resource_mgr_t *resmgr, int expected_latency) {
	resource_t *res = NULL, *res2 = NULL;

	if (!mixer)
		return SFX_ERROR;

	/* FIXME Temporary hack to detect Amiga games. */
	if (!Common::File::exists("bank.001"))
		seq = sfx_find_softseq(NULL);
	else
		seq = sfx_find_softseq("amiga");

	if (!seq) {
		sciprintf("[sfx:seq:polled] Initialisation failed: Could not find software sequencer\n");
		return SFX_ERROR;
	}

	if (seq->patch_nr != SFX_SEQ_PATCHFILE_NONE) {
		res = scir_find_resource(resmgr, sci_patch, seq->patch_nr, 0);
	}

	if (seq->patch2_nr != SFX_SEQ_PATCHFILE_NONE) {
		res2 = scir_find_resource(resmgr, sci_patch, seq->patch2_nr, 0);
	}

	if (seq->init(seq,
	              (res) ? res->data : NULL,
	              (res) ? res->size : 0,
	              (res2) ? res2->data : NULL,
	              (res2) ? res2->size : 0)) {
		sciprintf("[sfx:seq:polled] Initialisation failed: Sequencer '%s', v%s failed to initialise\n",
		          seq->name, seq->version);
		return SFX_ERROR;
	}

	pcmfeed.conf = seq->pcm_conf;

	seq->set_volume(seq, volume);
	mixer->subscribe(mixer, &pcmfeed);

	sfx_player_polled.polyphony = seq->polyphony;
	return SFX_OK;
}

static int
pp_add_iterator(song_iterator_t *it, GTimeVal start_time) {
	song_iterator_t *old = play_it;

	SIMSG_SEND(it, SIMSG_SET_PLAYMASK(seq->playmask));
	SIMSG_SEND(it, SIMSG_SET_RHYTHM(seq->play_rhythm));

	if (play_it == NULL)
		seq->allstop(seq);

	play_it = sfx_iterator_combine(play_it, it);

	seq->set_volume(seq, volume);

	/* The check must happen HERE, and not at the beginning of the
	   function, to avoid a race condition with the mixer. */
	if (old == NULL) {
		new_timestamp = sfx_new_timestamp(start_time.tv_sec,
		                                  start_time.tv_usec,
		                                  seq->pcm_conf.rate);
		/* ASAP otherwise */
		time_counter = 0;
		new_song = 1;
	}

	return SFX_OK;
}

static int
pp_fade_out(void) {
	fprintf(stderr, __FILE__": Attempt to fade out- not implemented yet\n");
	return SFX_ERROR;
}

static int
pp_stop(void) {
	song_iterator_t *it = play_it;

	play_it = NULL;
	fprintf(stderr, "[play] Now stopping it %p\n", (void *)it);
	if (it)
		songit_free(it);

	seq->allstop(seq);

	return SFX_OK;
}

static int
pp_send_iterator_message(song_iterator_message_t msg) {
	if (!play_it)
		return SFX_ERROR;

	songit_handle_message(&play_it, msg);
	return SFX_OK;
}

static int
pp_pause(void) {
	play_paused = 1;
	seq->set_volume(seq, 0);

	return SFX_OK;
}

static int
pp_resume(void) {
	if (!play_it) {
		play_paused = 0;
		return SFX_OK; /* Nothing to resume */
	}

	if (play_paused)
		new_song = 1; /* Fake starting a new song, re-using the old
			      ** time stamp (now long in the past) to indicate
			      ** resuming ASAP  */

	play_paused = 0;
	seq->set_volume(seq, volume);
	return SFX_OK;
}

static int
pp_exit(void) {
	seq->exit(seq);
	songit_free(play_it);
	play_it = NULL;

	return SFX_OK;
}

sfx_player_t sfx_player_polled = {
	"polled",
	"0.1",
	&pp_set_option,
	&pp_init,
	&pp_add_iterator,
	&pp_fade_out,
	&pp_stop,
	&pp_send_iterator_message,
	&pp_pause,
	&pp_resume,
	&pp_exit,
	&pp_timer_callback,
	&pp_tell_synth,
	0 /* polyphony */
};
