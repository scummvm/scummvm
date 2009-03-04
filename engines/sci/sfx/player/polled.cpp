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

#include "common/util.h"
#include "common/file.h"
#include "sci/sfx/player.h"
#include "sci/sfx/softseq.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Sci {

static song_iterator_t *play_it;
static int play_paused = 0;
static sfx_softseq_t *seq;
static int volume = 100;
static Audio::Timestamp new_timestamp;
static int new_song = 0;

/* The time counter is used to determine how close to the end of a tick we are.
** For each frame played, it is decreased by 60.  */
#define TIME_INC 60
static int time_counter = 0;

static void pp_tell_synth(int buf_nr, byte *buf) {
	seq->handle_command(seq, buf[0], buf_nr - 1, buf + 1);
}


class PolledPlayerAudioStream : public Audio::AudioStream {
protected:
	enum FeedMode {
		FEED_MODE_ALIVE,
		FEED_MODE_IDLE,
		FEED_MODE_RESTART
	};

	/* Whether feed is alive or dead. */
	FeedMode _mode;

	/* Blank gap in frames. */
	int _gap;

	/* Audio format. */
	sfx_pcm_config_t _conf;

	/* Timestamp of next frame requested by stream driver. */
	Audio::Timestamp _time;

public:
	PolledPlayerAudioStream(sfx_pcm_config_t conf)
		: _conf(conf),
		  _time(g_system->getMillis(), conf.rate) {

		_mode = FEED_MODE_ALIVE;
		_gap = 0;
	}

	~PolledPlayerAudioStream() {
	}

	virtual int readBuffer(int16 *buffer, const int numSamples);

	virtual bool isStereo() const { return _conf.stereo; }
	virtual int getRate() const { return _conf.rate; }

	virtual bool endOfData() const { return false; }

protected:
	void queryTimestamp();
};


void PolledPlayerAudioStream::queryTimestamp() {
	Audio::Timestamp stamp;
	
	if (!new_song) {
		_mode = FEED_MODE_IDLE;
	} else {
		// Otherwise, we have a timestamp:
		stamp = new_timestamp;
		new_song = 0;

		_gap = stamp.frameDiff(_time);
		if (_gap >= 0)
			_mode = FEED_MODE_ALIVE;
		else {
			// FIXME: I don't quite understand what FEED_MODE_RESTART is for.
			// The original DC mixer seemed to just stop and restart the stream.
			// But why? To catch up with lagging sound?
			//
			// Walter says the following:
			// "The FEED_MODE_RESTART might be there to re-sync after a debugger
			//  session where time passes for the mixer but not for the engine.
			//  I may have added this as a workaround for not being able to come
			//  up with a convenient way to implement mixer->pause() and mixer->resume()
			//  on DC."
			// That makes some sense. However, maybe it is sufficient to just
			// go to FEED_MODE_ALIVE ? With the current code, the player will
			// permanently get stuck in FEED_MODE_RESTART if we ever get here...
			_mode = FEED_MODE_RESTART;
			_time = Audio::Timestamp(g_system->getMillis(), _conf.rate);
			_gap = stamp.frameDiff(_time);

			if (_gap < 0)
				_gap = 0;
		}
	}
}

static void U8_to_S16(byte *buf, int samples) {
	for (int i = samples - 1; i >= 0; i--) {
		buf[i * 2 + 1] = buf[i] - 0x80;
		buf[i * 2] = 0;
	}
}

static int ppf_poll(int frame_size, byte *dest, int size);

int PolledPlayerAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	// FIXME: If ScummVM's mixer supported timestamps, then it would pass them
	// as a parameter to this function. But currently, it doesn't. Therefore, we
	// create a fake timestamp based on the current time. For comparison, a real
	// timestamp could be adjusted for pauses in sound processing. And it would
	// be synced for all audio streams.
	Audio::Timestamp timestamp(g_system->getMillis(), _conf.rate);

	int channels, frames_req;
	int frames_recv = 0;

	_time = timestamp;
	channels = _conf.stereo == SFX_PCM_MONO ? 1 : 2;
	frames_req = numSamples / channels;

	while (frames_req != frames_recv) {
		int frames_left = frames_req - frames_recv;
		byte *buf_pos = ((byte *)buffer) + frames_recv * channels * 2;

		if (_mode == FEED_MODE_IDLE)
			queryTimestamp();

		if (_mode == FEED_MODE_IDLE) {
			memset(buf_pos, 0, frames_left * channels * 2);

			_time = _time.addFrames(frames_left);
			break;
		}

		if (_gap) {
			int frames = _gap;

			if (frames > frames_left)
				frames = frames_left;

			memset(buf_pos, 0, frames * channels * 2);

			_gap -= frames;
			frames_recv += frames;
			_time = _time.addFrames(frames);
		} else {
			int frames = ppf_poll(channels * ((_conf.format & SFX_PCM_FORMAT_16) ? 2 : 1), buf_pos, frames_left);

			if (_conf.format == SFX_PCM_FORMAT_U8)
				U8_to_S16(buf_pos, frames * channels);

			frames_recv += frames;
			_time = _time.addFrames(frames);

			if (frames < frames_left)
				queryTimestamp();
		}
	}

	return numSamples;
}

/*----------------------*/
/* Mixer implementation */
/*----------------------*/
static int ppf_poll(int frame_size, byte *dest, int size) {
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

		seq->poll(seq, dest + written * frame_size, do_play);
		written += do_play;
	}

	return size; /* Apparently, we wrote all that was requested */
}

extern sfx_player_t sfx_player_polled;

/*=======================*/
/* Player implementation */
/*=======================*/


/*--------------------*/
/* API implementation */
/*--------------------*/

static void pp_timer_callback() {
	/* Hey, we're polled anyway ;-) */
}

static int pp_set_option(char *name, char *value) {
	return SFX_ERROR;
}

static int pp_init(ResourceManager *resmgr, int expected_latency) {
	Resource *res = NULL, *res2 = NULL;

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
		res = resmgr->findResource(kResourceTypePatch, seq->patch_nr, 0);
	}

	if (seq->patch2_nr != SFX_SEQ_PATCHFILE_NONE) {
		res2 = resmgr->findResource(kResourceTypePatch, seq->patch2_nr, 0);
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

	seq->set_volume(seq, volume);

	PolledPlayerAudioStream *newStream = new PolledPlayerAudioStream(seq->pcm_conf);
	// FIXME: Is this sound type appropriate?
	g_system->getMixer()->playInputStream(Audio::Mixer::kSFXSoundType, 0, newStream);

	sfx_player_polled.polyphony = seq->polyphony;
	return SFX_OK;
}

static int pp_add_iterator(song_iterator_t *it, uint32 start_time) {
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
		new_timestamp = Audio::Timestamp(start_time, seq->pcm_conf.rate);
		/* ASAP otherwise */
		time_counter = 0;
		new_song = 1;
	}

	return SFX_OK;
}

static int pp_fade_out() {
	warning(__FILE__": Attempt to fade out- not implemented yet");
	return SFX_ERROR;
}

static int pp_stop() {
	song_iterator_t *it = play_it;

	play_it = NULL;
	warning("[play] Now stopping it %p", (void *)it);
	if (it)
		songit_free(it);

	seq->allstop(seq);

	return SFX_OK;
}

static int pp_send_iterator_message(song_iterator_message_t msg) {
	if (!play_it)
		return SFX_ERROR;

	songit_handle_message(&play_it, msg);
	return SFX_OK;
}

static int pp_pause() {
	play_paused = 1;
	seq->set_volume(seq, 0);

	return SFX_OK;
}

static int pp_resume() {
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

static int pp_exit() {
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

} // End of namespace Sci
