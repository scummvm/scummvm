/***************************************************************************
 realtime.c Copyright (C) 2002..04 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/
/* OK... 'realtime' may be a little too euphemistic, as this one just
** prays for some reasonable amount of soft real-time, but it's close
** enough, I guess.  */

#include "sci/include/sfx_player.h"
#include "sci/sfx/sequencer.h"

static sfx_sequencer_t *seq;

extern sfx_player_t sfx_player_realtime;

/* Playing mechanism */

static inline GTimeVal
current_time(void)
{
	GTimeVal tv;
	sci_get_current_time(&tv);
	return tv;
}

static inline GTimeVal
add_time_delta(GTimeVal tv, long delta)
{
	int sec_d;

	tv.tv_usec += delta;
	sec_d = tv.tv_usec / 1000000;
	tv.tv_usec -= sec_d * 1000000;

	tv.tv_sec += sec_d;

	return tv;
}

static inline long
delta_time(GTimeVal comp_tv, GTimeVal base)
{
	GTimeVal tv;
	sci_get_current_time(&tv);
	return (comp_tv.tv_sec - tv.tv_sec) * 1000000
		+ (comp_tv.tv_usec - tv.tv_usec);
}

static song_iterator_t *play_it = NULL;
static GTimeVal play_last_time;
static GTimeVal play_pause_started; /* Beginning of the last pause */
static GTimeVal play_pause_counter; /* Last point in time to mark a
				    ** play position augmentation  */
static int play_paused = 0;
static int play_it_done = 0;
static int play_writeahead = 0;
static int play_moredelay = 0;

static void
play_song(song_iterator_t *it, GTimeVal *wakeup_time, int writeahead_time)
{
	unsigned char buf[8];
	int result;

	if (play_paused) {
		GTimeVal ct;
		sci_get_current_time(&ct);

		*wakeup_time =
			add_time_delta(*wakeup_time, delta_time(play_pause_counter, ct));
		play_pause_counter = ct;
	} else
	/* Not paused: */
	while (play_it && delta_time(*wakeup_time, current_time())
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
			seq->event(buf[0], result-1, buf+1);

			break;

		default:
			play_moredelay = delay - 1;
			*wakeup_time = song_next_wakeup_time(wakeup_time, delay);
			if (seq->delay)
				seq->delay(delay);
		}
	}
}

static void
rt_tell_synth(int buf_nr, byte *buf)
{
  seq->event(buf[0], buf_nr-1, buf+1);
}

static void
rt_timer_callback(void)
{
	if (play_it && !play_it_done) {
		if (!play_moredelay) {
			long delta = delta_time(play_last_time, current_time());

			if (delta < 0) {
				play_writeahead -= (int)((double)delta * 1.2); /* Adjust upwards */
			} else if (delta > 15000) {
				play_writeahead -= 2500; /* Adjust downwards */
			}
		} else --play_moredelay;

		if (play_writeahead < seq->min_write_ahead_ms)
			play_writeahead = seq->min_write_ahead_ms;

		play_song(play_it, &play_last_time, play_writeahead);
	}
}

static resource_t *
find_patch(resource_mgr_t *resmgr, const char *seq, int patchfile)
{
	resource_t *res = NULL;

	if (patchfile != SFX_SEQ_PATCHFILE_NONE) {
		res = scir_find_resource(resmgr, sci_patch, patchfile, 0);
		if (!res) {
			fprintf(stderr, "[SFX] " __FILE__": patch.%03d requested by sequencer (%s), but not found\n",
				patchfile, seq);
		}
	}

	return res;
}

/* API implementation */

static int
rt_set_option(char *name, char *value)
{
	return SFX_ERROR;
}

static int
rt_init(resource_mgr_t *resmgr, int expected_latency)
{
	resource_t *res = NULL, *res2 = NULL;
	void *seq_dev = NULL;
	GTimeVal foo = {0,0};

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

	if (seq->open(res? res->size : 0,
		      res? res->data : NULL,
		      res2? res2->size : 0,
		      res2? res2->data : NULL,
		      seq_dev)) {
		fprintf(stderr, "[SFX] " __FILE__": Sequencer failed to initialize\n");
		return SFX_ERROR;
	}

	play_writeahead = expected_latency;
	if (play_writeahead < seq->min_write_ahead_ms)
		play_writeahead = seq->min_write_ahead_ms;

	play_writeahead *= 1000; /* microseconds */

	if (seq->reset_timer)
		seq->reset_timer(foo);

	return SFX_OK;
}

static int
rt_add_iterator(song_iterator_t *it, GTimeVal start_time)
{
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

static int
rt_fade_out(void)
{
	fprintf(stderr, __FILE__": Attempt to fade out- not implemented yet\n");
	return SFX_ERROR;
}

static int
rt_stop(void)
{
	song_iterator_t *it = play_it;

	play_it = NULL;

	if (it)
		songit_free(it);
	if (seq && seq->allstop)
		seq->allstop();

	return SFX_OK;
}

static int
rt_send_iterator_message(song_iterator_message_t msg)
{
	if (!play_it)
		return SFX_ERROR;

	songit_handle_message(&play_it, msg);
	return SFX_OK;
}

static int
rt_pause(void)
{
	sci_get_current_time(&play_pause_started);
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

static int
rt_resume(void)
{
	play_paused = 0;
	return SFX_OK;
}

static int
rt_exit(void)
{
	int retval = SFX_OK;

	if(seq->close()) {
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
