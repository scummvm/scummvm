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

/* Song iterators */

#include "common/util.h"
#include "sci/sfx/iterator_internal.h"
#include "sci/sfx/player.h"
#include "sci/tools.h"
#include "sci/sci_memory.h"

namespace Sci {

static const int MIDI_cmdlen[16] = {0, 0, 0, 0, 0, 0, 0, 0,
                                    2, 2, 2, 2, 1, 1, 2, 0
                                   };

/*#define DEBUG_DECODING*/
/*#define DEBUG_VERBOSE*/

void print_tabs_id(int nr, songit_id_t id) {
	while (nr-- > 0)
		fprintf(stderr, "\t");

	fprintf(stderr, "[%08lx] ", id);
}

static unsigned char *sci_memchr(void *_data, int c, int n) {
	unsigned char *data = (unsigned char *) _data;

	while (n && !(*data == c)) {
		++data;
		--n;
	}

	if (n)
		return data;
	else
		return NULL;
}

static void _common_init(base_song_iterator_t *self) {
	self->fade.action = FADE_ACTION_NONE;
	self->resetflag = 0;
	self->loops = 0;
	self->priority = 0;
}


/************************************/
/*-- SCI0 iterator implementation --*/
/************************************/

#define SCI0_MIDI_OFFSET 33
#define SCI0_END_OF_SONG 0xfc /* proprietary MIDI command */

#define SCI0_PCM_SAMPLE_RATE_OFFSET 0x0e
#define SCI0_PCM_SIZE_OFFSET 0x20
#define SCI0_PCM_DATA_OFFSET 0x2c

#define CHECK_FOR_END_ABSOLUTE(offset) \
	if (offset > self->size) { \
		warning(SIPFX "Reached end of song without terminator (%x/%x) at %d!", offset, self->size, __LINE__); \
		return SI_FINISHED; \
	}

#define CHECK_FOR_END(offset_augment) \
	if ((channel->offset + (offset_augment)) > channel->end) { \
		channel->state = SI_STATE_FINISHED; \
		warning(SIPFX "Reached end of track %d without terminator (%x+%x/%x) at %d!", channel->id, channel->offset, offset_augment, channel->end, __LINE__); \
		return SI_FINISHED; \
	}


static inline int _parse_ticks(byte *data, int *offset_p, int size) {
	int ticks = 0;
	int tempticks;
	int offset = 0;

	do {
		tempticks = data[offset++];
		ticks += (tempticks == SCI_MIDI_TIME_EXPANSION_PREFIX) ?
		         SCI_MIDI_TIME_EXPANSION_LENGTH : tempticks;
	} while (tempticks == SCI_MIDI_TIME_EXPANSION_PREFIX
	         && offset < size);

	if (offset_p)
		*offset_p = offset;

	return ticks;
}


static int _sci0_read_next_command(sci0_song_iterator_t *self,
	unsigned char *buf, int *result);


static int _sci0_get_pcm_data(sci0_song_iterator_t *self,
	sfx_pcm_config_t *format, int *xoffset, unsigned int *xsize);

#define PARSE_FLAG_LOOPS_UNLIMITED (1 << 0) /* Unlimited # of loops? */
#define PARSE_FLAG_PARAMETRIC_CUE (1 << 1) /* Assume that cues take an additional "cue value" argument */
/* This implements a difference between SCI0 and SCI1 cues. */

void _reset_synth_channels(base_song_iterator_t *self, song_iterator_channel_t *channel) {
	int i;
	byte buf[5];
	tell_synth_func *tell = sfx_get_player_tell_func();

	for (i = 0; i < MIDI_CHANNELS; i++) {
		if (channel->saw_notes & (1 << i)) {
			buf[0] = 0xe0 | i; /* Pitch bend */
			buf[1] = 0x80; /* Wheel center */
			buf[2] = 0x40;
			if (tell)
				tell(3, buf);
			/* TODO: Reset other controls? */
		}
	}
}

static int _parse_sci_midi_command(base_song_iterator_t *self, unsigned char *buf,
	int *result, song_iterator_channel_t *channel, int flags) {
	unsigned char cmd;
	int paramsleft;
	int midi_op;
	int midi_channel;

	channel->state = SI_STATE_DELTA_TIME;

	cmd = self->data[channel->offset++];

	if (!(cmd & 0x80)) {
		/* 'Running status' mode */
		channel->offset--;
		cmd = channel->last_cmd;
	}

	if (cmd == 0xfe) {
		fprintf(stderr, "song iterator subsystem: Corrupted sound resource detected.\n");
		return SI_FINISHED;
	}

	midi_op = cmd >> 4;
	midi_channel = cmd & 0xf;
	paramsleft = MIDI_cmdlen[midi_op];
	channel->saw_notes |= 1 << midi_channel;

#if 0
	if (1) {
		fprintf(stderr, "[IT]: off=%x, cmd=%02x, takes %d args ",
		        channel->offset - 1, cmd, paramsleft);
		fprintf(stderr, "[%02x %02x <%02x> %02x %02x %02x]\n",
		        self->data[channel->offset-3],
		        self->data[channel->offset-2],
		        self->data[channel->offset-1],
		        self->data[channel->offset],
		        self->data[channel->offset+1],
		        self->data[channel->offset+2]);
	}
#endif

	buf[0] = cmd;


	CHECK_FOR_END(paramsleft);
	memcpy(buf + 1, self->data + channel->offset, paramsleft);
	*result = 1 + paramsleft;

	channel->offset += paramsleft;

	channel->last_cmd = cmd;

	/* Are we supposed to play this channel? */
	if (
	    /* First, exclude "global" properties-- such as cues-- from consideration */
	    (midi_op < 0xf
	     && !(cmd == SCI_MIDI_SET_SIGNAL)
	     && !(SCI_MIDI_CONTROLLER(cmd)
	          && buf[1] == SCI_MIDI_CUMULATIVE_CUE))

	    /* Next, check if the channel is allowed */
	    && (!((1 << midi_channel) & channel->playmask)))
		return  /* Execute next command */
		    self->next((song_iterator_t *) self, buf, result);


	if (cmd == SCI_MIDI_EOT) {
		/* End of track? */
		_reset_synth_channels(self, channel);
		/*		fprintf(stderr, "eot; loops = %d, notesplayed=%d\n", self->loops, channel->notes_played);*/
		if (self->loops > 1 /* && channel->notes_played*/) {
			/* If allowed, decrement the number of loops */
			if (!(flags & PARSE_FLAG_LOOPS_UNLIMITED))
				*result = --self->loops;

#ifdef DEBUG_DECODING
			fprintf(stderr, "%s L%d: (%p):%d Looping ", __FILE__, __LINE__, self, channel->id);
			if (flags & PARSE_FLAG_LOOPS_UNLIMITED)
				fprintf(stderr, "(indef.)");
			else
				fprintf(stderr, "(%d)", self->loops);
			fprintf(stderr, " %x -> %x\n",
			        channel->offset, channel->loop_offset);
#endif
			channel->offset = channel->loop_offset;
			channel->notes_played = 0;
			channel->state = SI_STATE_DELTA_TIME;
			channel->total_timepos = channel->loop_timepos;
			channel->last_cmd = 0xfe;
			fprintf(stderr, "Looping song iterator %08lx.\n", self->ID);
			return SI_LOOP;
		} else {
			channel->state = SI_STATE_FINISHED;
#ifdef DEBUG_DECODING
			fprintf(stderr, "%s L%d: (%p):%d EOT because"
			        " %d notes, %d loops\n",
			        __FILE__, __LINE__, self, channel->id,
			        channel->notes_played, self->loops);
#endif
			return SI_FINISHED;
		}

	} else if (cmd == SCI_MIDI_SET_SIGNAL) {
		if (buf[1] == SCI_MIDI_SET_SIGNAL_LOOP) {
			channel->loop_offset = channel->offset;
			channel->loop_timepos = channel->total_timepos;

			return /* Execute next command */
			    self->next((song_iterator_t *) self, buf, result);
		} else {
			/* Used to be conditional <= 127 */
			*result = buf[1]; /* Absolute cue */
			return SI_ABSOLUTE_CUE;
		}
	} else if (SCI_MIDI_CONTROLLER(cmd)) {
		switch (buf[1]) {

		case SCI_MIDI_CUMULATIVE_CUE:
			if (flags & PARSE_FLAG_PARAMETRIC_CUE)
				self->ccc += buf[2];
			else { /* No parameter to CC */
				self->ccc++;
				/*				channel->offset--; */
			}
			*result = self->ccc;
			return SI_RELATIVE_CUE;

		case SCI_MIDI_RESET_ON_SUSPEND:
			self->resetflag = buf[2];
			break;

		case SCI_MIDI_SET_POLYPHONY:
			self->polyphony[midi_channel] = buf[2];

#if 0
			{
				int i;
				int voices = 0;
				for (i = 0; i < ((sci1_song_iterator_t *) self)->channels_nr; i++) {
					voices += self->polyphony[i];
				}

				sciprintf("SET_POLYPHONY(%d, %d) for a total of %d voices\n", midi_channel, buf[2], voices);
				sciprintf("[iterator-1] DEBUG: Polyphony = [ ");
				for (i = 0; i < ((sci1_song_iterator_t *) self)->channels_nr; i++)
					sciprintf("%d ", self->polyphony[i]);
				sciprintf("]\n");
				sciprintf("[iterator-1] DEBUG: Importance = [ ");
				for (i = 0; i < ((sci1_song_iterator_t *) self)->channels_nr; i++)
					sciprintf("%d ", self->importance[i]);
				sciprintf("]\n");
			}
#endif
			break;

		case SCI_MIDI_SET_REVERB:
			break;

		case SCI_MIDI_CHANNEL_MUTE:
			sciprintf("CHANNEL_MUTE(%d, %d)\n", midi_channel, buf[2]);
			break;

		case SCI_MIDI_HOLD: {
			// Safe cast: This controller is only used in SCI1
			sci1_song_iterator_t *self1 = (sci1_song_iterator_t *) self;

			if (buf[2] == self1->hold) {
				channel->offset = channel->initial_offset;
				channel->notes_played = 0;
				channel->state = SI_STATE_COMMAND;
				channel->total_timepos = 0;

				self1->channels_looped = self1->active_channels - 1;

				return SI_LOOP;
			}

			break;
		}
		case 0x04: /* UNKNOWN NYI (happens in LSL2 gameshow) */
		case 0x46: /* UNKNOWN NYI (happens in LSL3 binoculars) */
		case 0x61: /* UNKNOWN NYI (special for adlib? Iceman) */
		case 0x73: /* UNKNOWN NYI (happens in Hoyle) */
		case 0xd1: /* UNKNOWN NYI (happens in KQ4 when riding the unicorn) */
			return /* Execute next command */
			    self->next((song_iterator_t *) self, buf, result);

		case 0x01: /* modulation */
		case 0x07: /* volume */
		case 0x0a: /* panpot */
		case 0x0b: /* expression */
		case 0x40: /* hold */
		case 0x79: /* reset all */
			/* No special treatment neccessary */
			break;

		}
		return 0;

	} else {
		if ((cmd & 0xf0) == 0x90) /* note on? */
			channel->notes_played++;

		/* Process as normal MIDI operation */
		return 0;
	}
}

static int _sci_midi_process_state(base_song_iterator_t *self, unsigned char *buf, int *result,
	song_iterator_channel_t *channel, int flags) {
	CHECK_FOR_END(0);

	switch (channel->state) {

	case SI_STATE_PCM: {
		if (*(self->data + channel->offset) == 0
		        && *(self->data + channel->offset + 1) == SCI_MIDI_EOT)
			/* Fake one extra tick to trick the interpreter into not killing the song iterator right away */
			channel->state = SI_STATE_PCM_MAGIC_DELTA;
		else
			channel->state = SI_STATE_DELTA_TIME;
		return SI_PCM;
	}

	case SI_STATE_PCM_MAGIC_DELTA: {
		sfx_pcm_config_t format;
		int offset;
		unsigned int size;
		int delay;
		if (_sci0_get_pcm_data((sci0_song_iterator_t *) self, &format, &offset, &size))
			return SI_FINISHED; /* 'tis broken */
		channel->state = SI_STATE_FINISHED;
		delay = (size * 50 + format.rate - 1) / format.rate; /* number of ticks to completion*/

		fprintf(stderr, "delaying %d ticks\n", delay);
		return delay;
	}

	case SI_STATE_UNINITIALISED:
		warning(SIPFX "Attempt to read command from uninitialized iterator!");
		self->init((song_iterator_t *) self);
		return self->next((song_iterator_t *) self, buf, result);

	case SI_STATE_FINISHED:
		return SI_FINISHED;

	case SI_STATE_DELTA_TIME: {
		int offset;
		int ticks = _parse_ticks(self->data + channel->offset,
		                         &offset,
		                         self->size - channel->offset);

		channel->offset += offset;
		channel->delay += ticks;
		channel->timepos_increment = ticks;

		CHECK_FOR_END(0);

		channel->state = SI_STATE_COMMAND;

		if (ticks)
			return ticks;
	}

	/* continute otherwise... */

	case SI_STATE_COMMAND: {
		int retval;
		channel->total_timepos += channel->timepos_increment;
		channel->timepos_increment = 0;

		retval = _parse_sci_midi_command(self, buf, result,
		                                 channel, flags);

		if (retval == SI_FINISHED) {
			if (self->active_channels)
				--(self->active_channels);
#ifdef DEBUG_DECODING
			fprintf(stderr, "%s L%d: (%p):%d Finished channel, %d channels left\n",
			        __FILE__, __LINE__, self, channel->id,
			        self->active_channels);
#endif
			/* If we still have channels left... */
			if (self->active_channels) {
				return self->next((song_iterator_t *) self, buf, result);
			}

			/* Otherwise, we have reached the end */
			self->loops = 0;
		}

		return retval;
	}

	default:
		warning(SIPFX "Invalid iterator state %d!", channel->state);
		BREAKPOINT();
		return SI_FINISHED;
	}
}

static inline int _sci_midi_process(base_song_iterator_t *self, unsigned char *buf, int *result,
	song_iterator_channel_t *channel, int flags) {
	return _sci_midi_process_state(self, buf, result,
	                               channel,
	                               flags);
}

static int _sci0_read_next_command(sci0_song_iterator_t *self, unsigned char *buf, int *result) {
	return _sci_midi_process((base_song_iterator_t *) self, buf, result,
	                         &(self->channel),
	                         PARSE_FLAG_PARAMETRIC_CUE);
}

static inline int _sci0_header_magic_p(unsigned char *data, int offset, int size) {
	if (offset + 0x10 > size)
		return 0;
	return
	    (data[offset] == 0x1a)
	    && (data[offset + 1] == 0x00)
	    && (data[offset + 2] == 0x01)
	    && (data[offset + 3] == 0x00);
}


static int _sci0_get_pcm_data(sci0_song_iterator_t *self,
	sfx_pcm_config_t *format, int *xoffset, unsigned int *xsize) {
	int tries = 2;
	int found_it = 0;
	unsigned char *pcm_data;
	int size;
	unsigned int offset = SCI0_MIDI_OFFSET;

	if (self->data[0] != 2)
		return 1;
	/* No such luck */

	while ((tries--) && (offset < self->size) && (!found_it)) {
		/* Search through the garbage manually */
		unsigned char *fc = sci_memchr(self->data + offset,
		                    SCI0_END_OF_SONG,
		                    self->size - offset);

		if (!fc) {
			warning(SIPFX "Playing unterminated song!");
			return 1;
		}

		/* add one to move it past the END_OF_SONG marker */
		offset = fc - self->data + 1;


		if (_sci0_header_magic_p(self->data, offset, self->size))
			found_it = 1;
	}

	if (!found_it) {
		warning(SIPFX "Song indicates presence of PCM, but"
		        " none found (finally at offset %04x)", offset);

		return 1;
	}

	pcm_data = self->data + offset;

	size = getUInt16(pcm_data + SCI0_PCM_SIZE_OFFSET);

	/* Two of the format parameters are fixed by design: */
	format->format = SFX_PCM_FORMAT_U8;
	format->stereo = SFX_PCM_MONO;
	format->rate = getUInt16(pcm_data + SCI0_PCM_SAMPLE_RATE_OFFSET);

	if (offset + SCI0_PCM_DATA_OFFSET + size != self->size) {
		int d = offset + SCI0_PCM_DATA_OFFSET + size - self->size;

		warning(SIPFX "PCM advertizes %d bytes of data, but %d"
		        " bytes are trailing in the resource!",
		        size, self->size - (offset + SCI0_PCM_DATA_OFFSET));

		if (d > 0)
			size -= d; /* Fix this */
	}

	*xoffset = offset;
	*xsize = size;

	return 0;
}

static sfx_pcm_feed_t *_sci0_check_pcm(sci0_song_iterator_t *self) {
	sfx_pcm_config_t format;
	int offset;
	unsigned int size;
	if (_sci0_get_pcm_data(self, &format, &offset, &size))
		return NULL;

	self->channel.state
	= SI_STATE_FINISHED; /* Don't play both PCM and music */

	return sfx_iterator_make_feed(self->data,
	                              offset + SCI0_PCM_DATA_OFFSET,
	                              size,
	                              format);
}

static song_iterator_t *_sci0_handle_message(sci0_song_iterator_t *self, song_iterator_message_t msg) {
	if (msg.recipient == _SIMSG_BASE) {
		switch (msg.type) {

		case _SIMSG_BASEMSG_PRINT:
			print_tabs_id(msg.args[0].i, self->ID);
			fprintf(stderr, "SCI0: dev=%d, active-chan=%d, size=%d, loops=%d\n",
			        self->device_id, self->active_channels, self->size,
			        self->loops);
			break;

		case _SIMSG_BASEMSG_SET_LOOPS:
			self->loops = msg.args[0].i;
			break;

		case _SIMSG_BASEMSG_CLONE: {
			int tsize = sizeof(sci0_song_iterator_t);
			base_song_iterator_t *mem = (base_song_iterator_t*)sci_malloc(tsize);
			memcpy(mem, self, tsize);
			sci_refcount_incref(mem->data);
#ifdef DEBUG_VERBOSE
			fprintf(stderr, "** CLONE INCREF for new %p from %p at %p\n", mem, self, mem->data);
#endif
			return (song_iterator_t *) mem; /* Assume caller has another copy of this */
		}

		case _SIMSG_BASEMSG_STOP: {
			songit_id_t sought_id = msg.ID;

			if (sought_id == self->ID)
				self->channel.state = SI_STATE_FINISHED;
			break;
		}

		case _SIMSG_BASEMSG_SET_PLAYMASK: {
			int i;
			self->device_id = msg.args[0].i;

			/* Set all but the rhytm channel mask bits */
			self->channel.playmask &= ~(1 << MIDI_RHYTHM_CHANNEL);

			for (i = 0; i < MIDI_CHANNELS; i++)
				if (self->data[2 + (i << 1)] & self->device_id
				        && i != MIDI_RHYTHM_CHANNEL)
					self->channel.playmask |= (1 << i);
		}
		break;

		case _SIMSG_BASEMSG_SET_RHYTHM:
			self->channel.playmask &= ~(1 << MIDI_RHYTHM_CHANNEL);
			if (msg.args[0].i)
				self->channel.playmask |= (1 << MIDI_RHYTHM_CHANNEL);
			break;

		case _SIMSG_BASEMSG_SET_FADE: {
			fade_params_t *fp = (fade_params_t *) msg.args[0].p;
			self->fade.action = fp->action;
			self->fade.final_volume = fp->final_volume;
			self->fade.ticks_per_step = fp->ticks_per_step;
			self->fade.step_size = fp->step_size;
			break;
		}

		default:
			return NULL;
		}

		return (song_iterator_t *)self;
	}
	return NULL;
}

static int _sci0_get_timepos(sci0_song_iterator_t *self) {
	return self->channel.total_timepos;
}

static void _base_init_channel(song_iterator_channel_t *channel, int id, int offset,
	int end) {
	channel->playmask = PLAYMASK_NONE; /* Disable all channels */
	channel->id = id;
	channel->notes_played = 0;
	channel->state = SI_STATE_DELTA_TIME;
	channel->loop_timepos = 0;
	channel->total_timepos = 0;
	channel->timepos_increment = 0;
	channel->delay = 0; /* Only used for more than one channel */
	channel->last_cmd = 0xfe;

	channel->offset
	= channel->loop_offset
	  = channel->initial_offset
	    = offset;
	channel->end = end;
	channel->saw_notes = 0;
}

static void _sci0_init(sci0_song_iterator_t *self) {
	_common_init((base_song_iterator_t *) self);

	self->ccc = 0; /* Reset cumulative cue counter */
	self->active_channels = 1;
	_base_init_channel(&(self->channel), 0, SCI0_MIDI_OFFSET, self->size);
	_reset_synth_channels((base_song_iterator_t *) self,
	                      &(self->channel));
	self->delay_remaining = 0;

	if (self->data[0] == 2) /* Do we have an embedded PCM? */
		self->channel.state = SI_STATE_PCM;
}


static void _sci0_cleanup(sci0_song_iterator_t *self) {
#ifdef DEBUG_VERBOSE
	fprintf(stderr, "** FREEING it %p: data at %p\n", self, self->data);
#endif
	if (self->data)
		sci_refcount_decref(self->data);
	self->data = NULL;
}

/***************************/
/*-- SCI1 song iterators --*/
/***************************/

#define SCI01_INVALID_DEVICE 0xff

/* Second index determines whether PCM output is supported */
static int sci0_to_sci1_device_map[][2] = {
	{0x06, 0x0c}, /* MT-32 */
	{0xff, 0xff}, /* YM FB-01 */
	{0x00, 0x00}, /* CMS/Game Blaster-- we assume OPL/2 here... */
	{0xff, 0xff}, /* Casio MT540/CT460 */
	{0x13, 0x13}, /* Tandy 3-voice */
	{0x12, 0x12}, /* PC speaker */
	{0xff, 0xff},
	{0xff, 0xff},
}; /* Maps bit number to device ID */

#define SONGDATA(x) self->data[offset + (x)]
#define SCI1_CHANDATA(off) self->data[channel->offset + (off)]

static int _sci1_sample_init(sci1_song_iterator_t *self, int offset) {
	sci1_sample_t *sample, **seekerp;
	int rate;
	int length;
	int begin;
	int end;

	CHECK_FOR_END_ABSOLUTE((unsigned int)offset + 10);
	if (self->data[offset + 1] != 0)
		sciprintf("[iterator-1] In sample at offset 0x04x: Byte #1 is %02x instead of zero\n",
		          self->data[offset + 1]);

	rate = getInt16(self->data + offset + 2);
	length = getUInt16(self->data + offset + 4);
	begin = getInt16(self->data + offset + 6);
	end = getInt16(self->data + offset + 8);

	CHECK_FOR_END_ABSOLUTE((unsigned int)(offset + 10 + length));

	sample = (sci1_sample_t*)sci_malloc(sizeof(sci1_sample_t));
	sample->delta = begin;
	sample->size = length;
	sample->data = self->data + offset + 10;

#ifdef DEBUG_VERBOSE
	fprintf(stderr, "[SAMPLE] %x/%x/%x/%x l=%x\n",
	        offset + 10, begin, end, self->size, length);
#endif

	sample->format.format = SFX_PCM_FORMAT_U8;
	sample->format.stereo = SFX_PCM_MONO;
	sample->format.rate = rate;

	sample->announced = 0;

	/* Perform insertion sort */
	seekerp = &(self->next_sample);

	while (*seekerp && (*seekerp)->delta < begin)
		seekerp = &((*seekerp)->next);

	sample->next = *seekerp;
	*seekerp = sample;

	return 0; /* Everything's fine */
}

static int _sci1_song_init(sci1_song_iterator_t *self) {
	sci1_sample_t *seeker;
	int last_time;
	unsigned int offset = 0;
	self->channels_nr = 0;
	self->next_sample = 0;
//	self->device_id = 0x0c;

	if (SONGDATA(0) == 0xf0) {
		self->priority = SONGDATA(1);

		offset += 8;
	}

	while (SONGDATA(0) != 0xff
	        && SONGDATA(0) != self->device_id) {
		offset++;
		CHECK_FOR_END_ABSOLUTE(offset + 1);
		while (SONGDATA(0) != 0xff) {
			CHECK_FOR_END_ABSOLUTE(offset + 7);
			offset += 6;
		}
		offset++;
	}

	if (SONGDATA(0) == 0xff) {
		sciprintf("[iterator-1] Song does not support"
		          " hardware 0x%02x\n",
		          self->device_id);
		return 1;
	}

	offset++;

	while (SONGDATA(0) != 0xff) { /* End of list? */
		unsigned int track_offset;
		int end;
		offset += 2;

		CHECK_FOR_END_ABSOLUTE(offset + 4);

		track_offset = getUInt16(self->data + offset);
		end = getUInt16(self->data + offset + 2);

		CHECK_FOR_END_ABSOLUTE(track_offset - 1);

		if (self->data[track_offset] == 0xfe) {
			if (_sci1_sample_init(self, track_offset))
				return 1; /* Error */
		} else {
			/* Regular MIDI channel */
			if (self->channels_nr >= MIDI_CHANNELS) {
				sciprintf("[iterator-1] Warning: Song has more than %d channels, cutting them off\n",
				          MIDI_CHANNELS);
				break; /* Scan for remaining samples */
			} else {
				int channel_nr
				= self->data[track_offset] & 0xf;
				song_iterator_channel_t *channel =
				    &(self->channels[self->channels_nr++]);

				if (self->data[track_offset] & 0xf0)
					printf("Channel %d has mapping bits %02x\n",
					       channel_nr, self->data[track_offset] & 0xf0);

				_base_init_channel(channel,
				                   channel_nr,
				                   /* Skip over header bytes: */
				                   track_offset + 2,
				                   track_offset + end);
				_reset_synth_channels((base_song_iterator_t *) self,
				                      channel);

				self->polyphony[self->channels_nr - 1]
				= SCI1_CHANDATA(-1);
				self->importance[self->channels_nr - 1]
				= self->polyphony[self->channels_nr - 1] >> 4;
				self->polyphony[self->channels_nr - 1] &= 15;

				channel->playmask = ~0; /* Enable all */
				self->channel_mask |= (1 << channel_nr);

				CHECK_FOR_END_ABSOLUTE(offset + end);
			}
		}
		offset += 4;
		CHECK_FOR_END_ABSOLUTE(offset);
	}

	/* Now ensure that sapmle deltas are relative to the previous sample */
	seeker = self->next_sample;
	last_time = 0;
	self->active_channels = self->channels_nr;
	self->channels_looped = 0;

	while (seeker) {
		int prev_last_time = last_time;
		sciprintf("[iterator-1] Detected sample: %d Hz, %d bytes at time %d\n",
		          seeker->format.rate, seeker->size, seeker->delta);
		last_time = seeker->delta;
		seeker->delta -= prev_last_time;
		seeker = seeker->next;
	}

	return 0; /* Success */
}

#undef SONGDATA

static inline int _sci1_get_smallest_delta(sci1_song_iterator_t *self) {
	int i, d = -1;
	for (i = 0; i < self->channels_nr; i++)
		if (self->channels[i].state == SI_STATE_COMMAND
		        && (d == -1 || self->channels[i].delay < d))
			d = self->channels[i].delay;

	if (self->next_sample && self->next_sample->delta < d)
		return self->next_sample->delta;
	else
		return d;
}

static inline void _sci1_update_delta(sci1_song_iterator_t *self, int delta) {
	int i;

	if (self->next_sample)
		self->next_sample->delta -= delta;

	for (i = 0; i < self->channels_nr; i++)
		if (self->channels[i].state == SI_STATE_COMMAND)
			self->channels[i].delay -= delta;
}

static inline int _sci1_no_delta_time(sci1_song_iterator_t *self) { /* Checks that none of the channels is waiting for its delta to be read */
	int i;

	for (i = 0; i < self->channels_nr; i++)
		if (self->channels[i].state == SI_STATE_DELTA_TIME)
			return 0;

	return 1;
}

#if 0
// Unreferenced - removed
static void _sci1_dump_state(sci1_song_iterator_t *self) {
	int i;

	sciprintf("-- [%p] ------------------------\n", self);
	for (i = 0; i < self->channels_nr; i++) {
		int j;
		sciprintf("%d(s%02d): d-%d:\t(%x/%x)  ",
		          self->channels[i].id,
		          self->channels[i].state,
		          self->channels[i].delay,
		          self->channels[i].offset,
		          self->channels[i].end);
		for (j = -3; j < 9; j++) {
			if (j == 0)
				sciprintf(">");
			else
				sciprintf(" ");

			sciprintf("%02x", self->data[self->channels[i].offset + j]);

			if (j == 0)
				sciprintf("<");
			else
				sciprintf(" ");
		}
		sciprintf("\n");
	}
	if (self->next_sample) {
		sciprintf("\t[sample %d]\n",
		          self->next_sample->delta);
	}
	sciprintf("------------------------------------------\n");
}
#endif

#define COMMAND_INDEX_NONE -1
#define COMMAND_INDEX_PCM -2

static inline int _sci1_command_index(sci1_song_iterator_t *self) {
	/* Determine the channel # of the next active event, or -1 */
	int i;
	int base_delay = 0x7ffffff;
	int best_chan = COMMAND_INDEX_NONE;

	for (i = 0; i < self->channels_nr; i++)
		if ((self->channels[i].state != SI_STATE_PENDING)
		        && (self->channels[i].state != SI_STATE_FINISHED))  {

			if ((self->channels[i].state == SI_STATE_DELTA_TIME)
			        && (self->channels[i].delay == 0))
				return i;
			/* First, read all unknown delta times */

			if (self->channels[i].delay < base_delay) {
				best_chan = i;
				base_delay = self->channels[i].delay;
			}
		}

	if (self->next_sample && base_delay >= self->next_sample->delta)
		return COMMAND_INDEX_PCM;

	return best_chan;
}


static sfx_pcm_feed_t *_sci1_get_pcm(sci1_song_iterator_t *self) {
	if (self->next_sample
	        && self->next_sample->delta <= 0) {
		sci1_sample_t *sample = self->next_sample;
		sfx_pcm_feed_t *feed
		= sfx_iterator_make_feed(self->data,
		                         sample->data - self->data,
		                         sample->size,
		                         sample->format);

		self->next_sample = self->next_sample->next;

		free(sample);

		return feed;
	} else
		return NULL;
}

static int _sci1_process_next_command(sci1_song_iterator_t *self,
	unsigned char *buf, int *result) {
	int retval = -42; /* Shouldn't happen, but gcc doesn't agree */
	int chan;

	if (!self->initialised) {
		sciprintf("[iterator-1] DEBUG: Initialising for %d\n",
		          self->device_id);
		self->initialised = 1;
		if (_sci1_song_init(self))
			return SI_FINISHED;
	}


	if (self->delay_remaining) {
		int delay = self->delay_remaining;
		self->delay_remaining = 0;
		return delay;
	}

	do {
		chan = _sci1_command_index(self);

		if (chan == COMMAND_INDEX_NONE) {
			return SI_FINISHED;
		}

		if (chan == COMMAND_INDEX_PCM) {

			if (self->next_sample->announced) {
				/* Already announced; let's discard it */
				sfx_pcm_feed_t *feed
				= _sci1_get_pcm(self);
				feed->destroy(feed);
			} else {
				int delay = self->next_sample->delta;

				if (delay) {
					_sci1_update_delta(self, delay);
					return delay;
				}
				/* otherwise we're touching a PCM */
				self->next_sample->announced = 1;
				return SI_PCM;
			}
		} else { /* Not a PCM */

			retval = _sci_midi_process((base_song_iterator_t *) self,
			                           buf, result,
			                           &(self->channels[chan]),
			                           PARSE_FLAG_LOOPS_UNLIMITED);

			if (retval == SI_LOOP) {
				self->channels_looped++;
				self->channels[chan].state = SI_STATE_PENDING;
				self->channels[chan].delay = 0;

				if (self->channels_looped == self->active_channels) {
					int i;

					/* Everyone's ready: Let's loop */
					for (i = 0; i < self->channels_nr; i++)
						if (self->channels[i].state
						        == SI_STATE_PENDING)
							self->channels[i].state
							= SI_STATE_DELTA_TIME;

					self->channels_looped = 0;
					return SI_LOOP;
				}
			} else if (retval == SI_FINISHED) {
#ifdef DEBUG
				fprintf(stderr, "FINISHED some channel\n");
#endif
			} else if (retval > 0) {
				int sd ;
				sd = _sci1_get_smallest_delta(self);

				if (_sci1_no_delta_time(self) && sd) {
					/* No other channel is ready */
					_sci1_update_delta(self, sd);

					/* Only from here do we return delta times */
					return sd;
				}
			}

		} /* Not a PCM */

	} while (retval > 0); /* All delays must be processed separately */

	return retval;
}

static song_iterator_t *_sci1_handle_message(sci1_song_iterator_t *self,
			                     song_iterator_message_t msg) {
	if (msg.recipient == _SIMSG_BASE) { /* May extend this in the future */
		switch (msg.type) {

		case _SIMSG_BASEMSG_PRINT: {
			int playmask = 0;
			int i;

			for (i = 0; i < self->channels_nr; i++)
				playmask |= self->channels[i].playmask;

			print_tabs_id(msg.args[0].i, self->ID);
			fprintf(stderr, "SCI1: chan-nr=%d, playmask=%04x\n",
			        self->channels_nr, playmask);
		}
		break;

		case _SIMSG_BASEMSG_CLONE: {
			int tsize = sizeof(sci1_song_iterator_t);
			sci1_song_iterator_t *mem = (sci1_song_iterator_t*)sci_malloc(tsize);
			sci1_sample_t **samplep;
			int delta = msg.args[0].i; /* Delay until next step */

			memcpy(mem, self, tsize);
			samplep = &(mem->next_sample);

			sci_refcount_incref(mem->data);

			mem->delay_remaining += delta;

			/* Clone chain of samples */
			while (*samplep) {
				sci1_sample_t *newsample
				= (sci1_sample_t*)sci_malloc(sizeof(sci1_sample_t));
				memcpy(newsample, *samplep,
				       sizeof(sci1_sample_t));
				*samplep = newsample;
				samplep = &(newsample->next);
			}

			return (song_iterator_t *) mem; /* Assume caller has another copy of this */
		}

		case _SIMSG_BASEMSG_STOP: {
			songit_id_t sought_id = msg.ID;
			int i;

			if (sought_id == self->ID) {
				self->ID = 0;

				for (i = 0; i < self->channels_nr; i++)
					self->channels[i].state = SI_STATE_FINISHED;
			}
			break;
		}

		case _SIMSG_BASEMSG_SET_PLAYMASK:
			if (msg.ID == self->ID) {
				self->channel_mask = 0;

				self->device_id
				= sci0_to_sci1_device_map
				  [sci_ffs(msg.args[0].i & 0xff) - 1]
				  [sfx_pcm_available()]
				  ;

				if (self->device_id == 0xff) {
					sciprintf("[iterator-1] Warning: Device %d(%d) not supported",
					          msg.args[0].i & 0xff, sfx_pcm_available());
				}
				if (self->initialised) {
					int i;
					int toffset = -1;

					for (i = 0; i < self->channels_nr; i++)
						if (self->channels[i].state != SI_STATE_FINISHED
						        && self->channels[i].total_timepos > toffset) {
							toffset = self->channels[i].total_timepos
							          + self->channels[i].timepos_increment
							          - self->channels[i].delay;
						}

					/* Find an active channel so that we can
					** get the correct time offset  */

					_sci1_song_init(self);

					toffset -= self->delay_remaining;
					self->delay_remaining = 0;

					if (toffset > 0)
						return new_fast_forward_iterator((song_iterator_t *) self,
						                                 toffset);
				} else {
					_sci1_song_init(self);
					self->initialised = 1;
				}

				break;

			}

		case _SIMSG_BASEMSG_SET_LOOPS:
			if (msg.ID == self->ID)
				self->loops = (msg.args[0].i > 32767) ? 99 : 0;
			/* 99 is arbitrary, but we can't use '1' because of
			** the way we're testing in the decoding section.  */
			break;

		case _SIMSG_BASEMSG_SET_HOLD:
			self->hold = msg.args[0].i;
			break;
		case _SIMSG_BASEMSG_SET_RHYTHM:
			/* Ignore */
			break;

		case _SIMSG_BASEMSG_SET_FADE: {
			fade_params_t *fp = (fade_params_t *) msg.args[0].p;
			self->fade.action = fp->action;
			self->fade.final_volume = fp->final_volume;
			self->fade.ticks_per_step = fp->ticks_per_step;
			self->fade.step_size = fp->step_size;
			break;
		}

		default:
			warning(SIPFX "Unsupported command %d to SCI1 iterator", msg.type);
		}
		return (song_iterator_t *) self;
	}
	return NULL;
}

static int _sci1_read_next_command(sci1_song_iterator_t *self, unsigned char *buf, int *result) {
	return _sci1_process_next_command(self, buf, result);
}

static void _sci1_init(sci1_song_iterator_t *self) {
	_common_init((base_song_iterator_t *) self);
	self->ccc = 127;
	self->device_id = 0x00; /* Default to Sound Blaster/Adlib for purposes
				** of cue computation  */
	self->next_sample = NULL;
	self->channels_nr = 0;
	self->initialised = 0;
	self->delay_remaining = 0;
	self->loops = 0;
	self->hold = 0;
	memset(self->polyphony, 0, sizeof(self->polyphony));
	memset(self->importance, 0, sizeof(self->importance));
}

static void _sci1_cleanup(sci1_song_iterator_t *it) {
	sci1_sample_t *sample_seeker = it->next_sample;
	while (sample_seeker) {
		sci1_sample_t *old_sample = sample_seeker;
		sample_seeker = sample_seeker->next;
		free(old_sample);
	}

	_sci0_cleanup((sci0_song_iterator_t *)it);
}

static int _sci1_get_timepos(sci1_song_iterator_t *self) {
	int max = 0;
	int i;

	for (i = 0; i < self->channels_nr; i++)
		if (self->channels[i].total_timepos > max)
			max = self->channels[i].total_timepos;

	return max;
}

/*****************************/
/*-- Cleanup song iterator --*/
/*****************************/


static void _cleanup_iterator_init(song_iterator_t *it) {
}

static song_iterator_t *_cleanup_iterator_handle_message(song_iterator_t *i, song_iterator_message_t msg) {
	if (msg.recipient == _SIMSG_BASEMSG_PRINT
	        && msg.type == _SIMSG_BASEMSG_PRINT) {
		print_tabs_id(msg.args[0].i, i->ID);
		fprintf(stderr, "CLEANUP\n");
	}

	return NULL;
}

static int _cleanup_iterator_next(song_iterator_t *self, unsigned char *buf, int *result) {
	/* Task: Return channel-notes-off for each channel */
	if (self->channel_mask) {
		int bs = sci_ffs(self->channel_mask) - 1;

		self->channel_mask &= ~(1 << bs);
		buf[0] = 0xb0 | bs; /* Controller */
		buf[1] = SCI_MIDI_CHANNEL_NOTES_OFF;
		buf[2] = 0; /* Hmm... */
		*result = 3;
		return 0;
	} else
		return SI_FINISHED;
}

song_iterator_t *new_cleanup_iterator(unsigned int channels) {
	song_iterator_t *it = (song_iterator_t*)sci_malloc(sizeof(song_iterator_t));
	it->channel_mask = channels;
	it->ID = 17;
	it->flags = 0;
	it->death_listeners_nr = 0;

	it->cleanup = NULL;
	it->get_pcm_feed = NULL;
	it->init = _cleanup_iterator_init;
	it->handle_message = _cleanup_iterator_handle_message;
	it->get_timepos = NULL;
	it->next = _cleanup_iterator_next;
	return it;
}

/**********************************/
/*-- Fast-forward song iterator --*/
/**********************************/

static int _ff_read_next_command(fast_forward_song_iterator_t *self,
	byte *buf, int *result) {
	int rv;

	if (self->delta <= 0)
		return SI_MORPH; /* Did our duty */

	while (1) {
		rv = self->delegate->next(self->delegate, buf, result);

		if (rv > 0) {
			/* Subtract from the delta we want to wait */
			self->delta -= rv;

			/* Done */
			if (self->delta < 0)
				return -self->delta;
		}

		if (rv <= 0)
			return rv;
	}
}

static sfx_pcm_feed_t *_ff_check_pcm(fast_forward_song_iterator_t *self) {
	return self->delegate->get_pcm_feed(self->delegate);
}

static song_iterator_t *_ff_handle_message(fast_forward_song_iterator_t *self,
	song_iterator_message_t msg) {
	if (msg.recipient == _SIMSG_PLASTICWRAP)
		switch (msg.type) {

		case _SIMSG_PLASTICWRAP_ACK_MORPH:
			if (self->delta <= 0) {
				song_iterator_t *it = self->delegate;
				free(self);
				return it;
			}
			break;

		default:
			BREAKPOINT();
		}
	else if (msg.recipient == _SIMSG_BASE) {
		switch (msg.type) {

		case _SIMSG_BASEMSG_CLONE: {
			int tsize = sizeof(fast_forward_song_iterator_t);
			fast_forward_song_iterator_t *clone = (fast_forward_song_iterator_t *)sci_malloc(tsize);
			memcpy(clone, self, tsize);
			songit_handle_message(&clone->delegate, msg);
			return (song_iterator_t *) clone;
		}

		case _SIMSG_BASEMSG_PRINT:
			print_tabs_id(msg.args[0].i, self->ID);
			fprintf(stderr, "PLASTICWRAP:\n");
			msg.args[0].i++;
			songit_handle_message(&(self->delegate), msg);
			break;

		default:
			songit_handle_message(&(self->delegate), msg);
		}
	} else
		songit_handle_message(&(self->delegate), msg);

	return NULL;
}


static void _ff_init(fast_forward_song_iterator_t *self) {
	return;
}

static int _ff_get_timepos(fast_forward_song_iterator_t *self) {
	return self->delegate->get_timepos(self->delegate);
}

song_iterator_t *new_fast_forward_iterator(song_iterator_t *capsit, int delta) {
	fast_forward_song_iterator_t *it =
	    (fast_forward_song_iterator_t*)sci_malloc(sizeof(fast_forward_song_iterator_t));

	if (capsit == NULL) {
		free(it);
		return NULL;
	}
	it->ID = 0;

	it->delegate = capsit;
	it->delta = delta;
	it->death_listeners_nr = 0;

	it->next = (int(*)(song_iterator_t *, unsigned char *, int *))
	           _ff_read_next_command;
	it->get_pcm_feed = (sfx_pcm_feed_t * (*)(song_iterator_t *))
	                   _ff_check_pcm;
	it->handle_message = (song_iterator_t * (*)(song_iterator_t *,
	                      song_iterator_message_t))
	                     _ff_handle_message;
	it->get_timepos = (int(*)(song_iterator_t *))_ff_get_timepos;
	it->init = (void(*)(song_iterator_t *))
	           _ff_init;
	it->cleanup = NULL;
	it->channel_mask = capsit->channel_mask;


	return (song_iterator_t *) it;
}


/********************/
/*-- Tee iterator --*/
/********************/


static int _tee_read_next_command(tee_song_iterator_t *it, unsigned char *buf,
	int *result) {
	static int ready_masks[2] = {TEE_LEFT_READY, TEE_RIGHT_READY};
	static int active_masks[2] = {TEE_LEFT_ACTIVE, TEE_RIGHT_ACTIVE};
	static int pcm_masks[2] = {TEE_LEFT_PCM, TEE_RIGHT_PCM};
	int i;
	int retid;

#ifdef DEBUG_TEE_ITERATOR
	fprintf(stderr, "[Tee] %02x\n", it->status);
#endif

	if (!(it->status & (TEE_LEFT_ACTIVE | TEE_RIGHT_ACTIVE)))
		/* None is active? */
		return SI_FINISHED;

	if (it->morph_deferred == TEE_MORPH_READY)
		return SI_MORPH;

	if ((it->status & (TEE_LEFT_ACTIVE | TEE_RIGHT_ACTIVE))
	        != (TEE_LEFT_ACTIVE | TEE_RIGHT_ACTIVE)) {
		/* Not all are is active? */
		int which = 0;
#ifdef DEBUG_TEE_ITERATOR
		fprintf(stderr, "\tRequesting transformation...\n");
#endif
		if (it->status & TEE_LEFT_ACTIVE)
			which = TEE_LEFT;
		else if (it->status & TEE_RIGHT_ACTIVE)
			which = TEE_RIGHT;
		memcpy(buf, it->children[which].buf, MAX_BUF_SIZE);
		*result = it->children[which].result;
		it->morph_deferred = TEE_MORPH_READY;
		return it->children[which].retval;
	}

	/* First, check for unreported PCMs */
	for (i = TEE_LEFT; i <= TEE_RIGHT; i++)
		if ((it->status & (ready_masks[i] | pcm_masks[i]))
		        == (ready_masks[i] | pcm_masks[i])) {
			it->status &= ~ready_masks[i];
			return SI_PCM;
		}

	for (i = TEE_LEFT; i <= TEE_RIGHT; i++)
		if (!(it->status & ready_masks[i])) {

			/* Buffers aren't ready yet */
			it->children[i].retval =
			    songit_next(&(it->children[i].it),
			                it->children[i].buf,
			                &(it->children[i].result),
			                IT_READER_MASK_ALL
			                | IT_READER_MAY_FREE
			                | IT_READER_MAY_CLEAN);

			it->status |= ready_masks[i];
#ifdef DEBUG_TEE_ITERATOR
			fprintf(stderr, "\t Must check %d: %d\n", i,
			        it->children[i].retval);
#endif

			if (it->children[i].retval == SI_ABSOLUTE_CUE ||
			        it->children[i].retval == SI_RELATIVE_CUE)
				return it->children[i].retval;
			if (it->children[i].retval == SI_FINISHED) {
				it->status &= ~active_masks[i];
				/* Recurse to complete */
#ifdef DEBUG_TEE_ITERATOR
				fprintf(stderr, "\t Child %d signalled completion, recursing w/ status %02x\n", i, it->status);
#endif
				return _tee_read_next_command(it, buf, result);
			} else if (it->children[i].retval == SI_PCM) {
				it->status |= pcm_masks[i];
				it->status &= ~ready_masks[i];
				return SI_PCM;
			}
		}


	/* We've already handled PCM, MORPH and FINISHED, CUEs & LOOP remain */

	retid = TEE_LEFT;
	if ((it->children[TEE_LEFT].retval > 0)
	        /* Asked to delay */
	        && (it->children[TEE_RIGHT].retval <= it->children[TEE_LEFT].retval))
		/* Is not delaying or not delaying as much */
		retid = TEE_RIGHT;

#ifdef DEBUG_TEE_ITERATOR
	fprintf(stderr, "\tl:%d / r:%d / chose %d\n",
	        it->children[TEE_LEFT].retval, it->children[TEE_RIGHT].retval, retid);
#endif
#if 0
	if (it->children[retid].retval == 0) {
		/* Perform remapping, if neccessary */
		byte *buf = it->children[retid].buf;
		if (*buf != SCI_MIDI_SET_SIGNAL
		        && *buf < 0xf0) { /* Not a generic command */
			int chan = *buf & 0xf;
			int op = *buf & 0xf0;

			chan = it->children[retid].channel_remap[chan];

			*buf = chan | op;
		}
	}
#endif

	/* Adjust delta times */
	if (it->children[retid].retval > 0
	        && it->children[1-retid].retval > 0) {
		if (it->children[1-retid].retval
		        == it->children[retid].retval)
			/* If both children wait the same amount of time,
			** we have to re-fetch commands from both  */
			it->status &= ~ready_masks[1-retid];
		else
			/* If they don't, we can/must re-use the other
			** child's delay time  */
			it->children[1-retid].retval
			-= it->children[retid].retval;
	}

	it->status &= ~ready_masks[retid];
	memcpy(buf, it->children[retid].buf, MAX_BUF_SIZE);
	*result = it->children[retid].result;

	return it->children[retid].retval;
}

static sfx_pcm_feed_t *_tee_check_pcm(tee_song_iterator_t *it) {
	static int pcm_masks[2] = {TEE_LEFT_PCM, TEE_RIGHT_PCM};
	int i;

	for (i = TEE_LEFT; i <= TEE_RIGHT; i++)
		if (it->status & pcm_masks[i]) {

			it->status &= ~pcm_masks[i];
			return it->children[i].it->
			       get_pcm_feed(it->children[i].it);
		}

	return NULL; /* No iterator */
}

static song_iterator_t *_tee_handle_message(tee_song_iterator_t *self, song_iterator_message_t msg) {
	if (msg.recipient == _SIMSG_BASE) {
		switch (msg.type) {

		case _SIMSG_BASEMSG_PRINT:
			print_tabs_id(msg.args[0].i, self->ID);
			fprintf(stderr, "TEE:\n");
			msg.args[0].i++;
			break; /* And continue with our children */

		case _SIMSG_BASEMSG_CLONE: {
			tee_song_iterator_t *newit
			= (tee_song_iterator_t*)sci_malloc(sizeof(tee_song_iterator_t));
			memcpy(newit, self, sizeof(tee_song_iterator_t));

			if (newit->children[TEE_LEFT].it)
				newit->children[TEE_LEFT].it =
				    songit_clone(newit->children[TEE_LEFT].it, msg.args[0].i);
			if (newit->children[TEE_RIGHT].it)
				newit->children[TEE_RIGHT].it =
				    songit_clone(newit->children[TEE_RIGHT].it, msg.args[0].i);

			return (song_iterator_t *) newit;
		}

		default:
			break;
		}
	}

	if (msg.recipient == _SIMSG_PLASTICWRAP) {
		song_iterator_t *old_it;
		switch (msg.type) {

		case _SIMSG_PLASTICWRAP_ACK_MORPH:
			if (!(self->status & (TEE_LEFT_ACTIVE | TEE_RIGHT_ACTIVE))) {
				songit_free((song_iterator_t *) self);
				return NULL;
			} else if (!(self->status & TEE_LEFT_ACTIVE)) {
				if (self->may_destroy)
					songit_free(self->children[TEE_LEFT].it);
				old_it = self->children[TEE_RIGHT].it;
				free(self);
				return old_it;
			} else if (!(self->status & TEE_RIGHT_ACTIVE)) {
				if (self->may_destroy)
					songit_free(self->children[TEE_RIGHT].it);
				old_it = self->children[TEE_LEFT].it;
				free(self);
				return old_it;
			} else {
				sciprintf("[tee-iterator] WARNING:"
				          " Morphing without need\n");
				return (song_iterator_t *) self;
			}

		default:
			BREAKPOINT();
		}
	}

	if (self->children[TEE_LEFT].it)
		songit_handle_message(&(self->children[TEE_LEFT].it), msg);
	if (self->children[TEE_RIGHT].it)
		songit_handle_message(&(self->children[TEE_RIGHT].it), msg);

	return NULL;
}

static void _tee_init(tee_song_iterator_t *it) {
	it->status = TEE_LEFT_ACTIVE | TEE_RIGHT_ACTIVE;
	it->children[TEE_LEFT].it->init(it->children[TEE_LEFT].it);
	it->children[TEE_RIGHT].it->init(it->children[TEE_RIGHT].it);
}

#if 0
// Unreferenced - removed
static void _tee_free(tee_song_iterator_t *it) {
	int i;
	for (i = TEE_LEFT; i <= TEE_RIGHT; i++)
		if (it->children[i].it && it->may_destroy)
			songit_free(it->children[i].it);
}
#endif

static void songit_tee_death_notification(tee_song_iterator_t *self,
	song_iterator_t *corpse) {
	if (corpse == self->children[TEE_LEFT].it) {
		self->status &= ~TEE_LEFT_ACTIVE;
		self->children[TEE_LEFT].it = NULL;
	} else if (corpse == self->children[TEE_RIGHT].it) {
		self->status &= ~TEE_RIGHT_ACTIVE;
		self->children[TEE_RIGHT].it = NULL;
	} else {
		BREAKPOINT();
	}
}


song_iterator_t *songit_new_tee(song_iterator_t *left, song_iterator_t *right, int may_destroy) {
	int i;
	int firstfree = 1; /* First free channel */
	int incomplete_map = 0;
	tee_song_iterator_t *it = (tee_song_iterator_t*)sci_malloc(sizeof(tee_song_iterator_t));

	it->ID = 0;

	it->morph_deferred = TEE_MORPH_NONE;
	it->status = TEE_LEFT_ACTIVE | TEE_RIGHT_ACTIVE;
	it->may_destroy = may_destroy;

	it->children[TEE_LEFT].it = left;
	it->children[TEE_RIGHT].it = right;
	it->death_listeners_nr = 0;

	/* By default, don't remap */
	for (i = 0; i < 16; i++)
		it->children[TEE_LEFT].channel_remap[i]
		= it->children[TEE_RIGHT].channel_remap[i] = i;

	/* Default to lhs channels */
	it->channel_mask = left->channel_mask;
	for (i = 0; i < 16; i++)
		if (it->channel_mask & (1 << i) & right->channel_mask
		        && (i != MIDI_RHYTHM_CHANNEL) /* Share rhythm */) { /*conflict*/
			while ((firstfree == MIDI_RHYTHM_CHANNEL)
			        /* Either if it's the rhythm channel or if it's taken */
			        || (firstfree < MIDI_CHANNELS
			            && ((1 << firstfree) & it->channel_mask)))
				++firstfree;

			if (firstfree == MIDI_CHANNELS) {
				incomplete_map = 1;
				fprintf(stderr, "[songit-tee <%08lx,%08lx>] "
				        "Could not remap right channel #%d:"
				        " Out of channels\n",
				        left->ID, right->ID, i);
			} else {
				it->children[TEE_RIGHT].channel_remap[i]
				= firstfree;

				it->channel_mask |= (1 << firstfree);
			}
		}
#ifdef DEBUG_TEE_ITERATOR
	if (incomplete_map) {
		int c;
		fprintf(stderr, "[songit-tee <%08lx,%08lx>] Channels:"
		        " %04x <- %04x | %04x\n",
		        left->ID, right->ID,
		        it->channel_mask,
		        left->channel_mask, right->channel_mask);
		for (c = 0 ; c < 2; c++)
			for (i = 0 ; i < 16; i++)
				fprintf(stderr, "  map [%d][%d] -> %d\n",
				        c, i, it->children[c].channel_remap[i]);
	}
#endif


	it->next = (int(*)(song_iterator_t *, unsigned char *, int *))
	           _tee_read_next_command;

	it->get_pcm_feed = (sfx_pcm_feed_t * (*)(song_iterator_t *))
	                   _tee_check_pcm;

	it->handle_message = (song_iterator_t * (*)(song_iterator_t *,
	                      song_iterator_message_t))
	                     _tee_handle_message;

	it->init = (void(*)(song_iterator_t *))
	           _tee_init;

	it->get_timepos = NULL;

	song_iterator_add_death_listener((song_iterator_t *)it,
	                                 left, (void (*)(void *, void*))
	                                 songit_tee_death_notification);
	song_iterator_add_death_listener((song_iterator_t *)it,
	                                 right, (void (*)(void *, void*))
	                                 songit_tee_death_notification);

	it->cleanup = NULL;

	return (song_iterator_t *) it;
}


/*************************************/
/*-- General purpose functionality --*/
/*************************************/

int songit_next(song_iterator_t **it, unsigned char *buf, int *result, int mask) {
	int retval;

	if (!*it)
		return SI_FINISHED;

	do {
		retval = (*it)->next(*it, buf, result);
		if (retval == SI_MORPH) {
			fprintf(stderr, "  Morphing %p (stored at %p)\n", (void *)*it, (void *)it);
			if (!SIMSG_SEND((*it), SIMSG_ACK_MORPH)) {
				BREAKPOINT();
			} else fprintf(stderr, "SI_MORPH successful\n");
		}

		if (retval == SI_FINISHED)
			fprintf(stderr, "[song-iterator] Song finished. mask = %04x, cm=%04x\n",
			        mask, (*it)->channel_mask);
		if (retval == SI_FINISHED
		        && (mask & IT_READER_MAY_CLEAN)
		        && (*it)->channel_mask) { /* This last test will fail
					       ** with a terminated
					       ** cleanup iterator */
			int channel_mask = (*it)->channel_mask;

			if (mask & IT_READER_MAY_FREE)
				songit_free(*it);
			*it = new_cleanup_iterator(channel_mask);
			retval = -9999; /* Continue */
		}
	} while (!(  /* Until one of the following holds */
	             (retval > 0 && (mask & IT_READER_MASK_DELAY))
	             || (retval == 0 && (mask & IT_READER_MASK_MIDI))
	             || (retval == SI_LOOP && (mask & IT_READER_MASK_LOOP))
	             || (retval == SI_ABSOLUTE_CUE &&
	                 (mask & IT_READER_MASK_CUE))
	             || (retval == SI_RELATIVE_CUE &&
	                 (mask & IT_READER_MASK_CUE))
	             || (retval == SI_PCM && (mask & IT_READER_MASK_PCM))
	             || (retval == SI_FINISHED)
	         ));

	if (retval == SI_FINISHED
	        && (mask & IT_READER_MAY_FREE)) {
		songit_free(*it);
		*it = NULL;
	}

	return retval;
}

song_iterator_t *songit_new(unsigned char *data, unsigned int size, int type, songit_id_t id) {
	base_song_iterator_t *it;
	int i;

	if (!data || size < 22) {
		warning(SIPFX "Attempt to instantiate song iterator for null song data");
		return NULL;
	}


	switch (type) {

	case SCI_SONG_ITERATOR_TYPE_SCI0:
		/**-- Playing SCI0 sound resources --**/
		it = (base_song_iterator_t*)sci_malloc(sizeof(sci0_song_iterator_t));
		it->channel_mask = 0xffff; /* Allocate all channels by default */

		for (i = 0; i < MIDI_CHANNELS; i++)
			it->polyphony[i] = data[1 + (i << 1)];

		it->next = (int(*)(song_iterator_t *, unsigned char *, int *))
		           _sci0_read_next_command;
		it->get_pcm_feed = (sfx_pcm_feed_t * (*)(song_iterator_t *))
		                   _sci0_check_pcm;
		it->handle_message = (song_iterator_t * (*)(song_iterator_t *, song_iterator_message_t))
		                     _sci0_handle_message;
		it->init = (void(*)(song_iterator_t *))_sci0_init;
		it->cleanup = (void(*)(song_iterator_t *))_sci0_cleanup;
		((sci0_song_iterator_t *)it)->channel.state
		= SI_STATE_UNINITIALISED;
		it->get_timepos = (int(*)(song_iterator_t *))_sci0_get_timepos;
		break;

	case SCI_SONG_ITERATOR_TYPE_SCI1:
		/**-- SCI01 or later sound resource --**/
		it = (base_song_iterator_t*)sci_malloc(sizeof(sci1_song_iterator_t));
		it->channel_mask = 0; /* Defer channel allocation */

		for (i = 0; i < MIDI_CHANNELS; i++)
			it->polyphony[i] = 0; /* Unknown */

		it->next = (int(*)(song_iterator_t *, unsigned char *, int *))
		           _sci1_read_next_command;
		it->get_pcm_feed = (sfx_pcm_feed_t * (*)(song_iterator_t *))
		                   _sci1_get_pcm;
		it->handle_message = (song_iterator_t * (*)(song_iterator_t *, song_iterator_message_t))
		                     _sci1_handle_message;
		it->init = (void(*)(song_iterator_t *))_sci1_init;
		it->cleanup = (void(*)(song_iterator_t *))_sci1_cleanup;
		it->get_timepos = (int(*)(song_iterator_t *))_sci1_get_timepos;
		break;

	default:
		/**-- Invalid/unsupported sound resources --**/
		warning(SIPFX "Attempt to instantiate invalid/unknown"
		        " song iterator type %d", type);
		return NULL;
	}
	it->ID = id;

	it->death_listeners_nr = 0;

	it->data = (unsigned char*)sci_refcount_memdup(data, size);
	it->size = size;

	it->init((song_iterator_t *) it);

	return (song_iterator_t *) it;
}

void songit_free(song_iterator_t *it) {
	if (it) {
		int i;

		if (it->cleanup)
			it->cleanup(it);

		for (i = 0; i < it->death_listeners_nr; i++)
			it->death_listeners[i].notify(it->death_listeners[i].self, it);

		free(it);
	}
}

song_iterator_message_t songit_make_message(songit_id_t id, int recipient, int type, int a1, int a2) {
	song_iterator_message_t rv;
	rv.ID = id;
	rv.recipient = recipient;
	rv.type = type;
	rv.args[0].i = a1;
	rv.args[1].i = a2;

	return rv;
}

song_iterator_message_t songit_make_ptr_message(songit_id_t id, int recipient, int type, void * a1, int a2) {
	song_iterator_message_t rv;
	rv.ID = id;
	rv.recipient = recipient;
	rv.type = type;
	rv.args[0].p = a1;
	rv.args[1].i = a2;

	return rv;
}

int songit_handle_message(song_iterator_t **it_reg_p, song_iterator_message_t msg) {
	song_iterator_t *it = *it_reg_p;
	song_iterator_t *newit;

	newit = it->handle_message(it, msg);

	if (!newit)
		return 0; /* Couldn't handle */

	*it_reg_p = newit; /* Might have self-morphed */
	return 1;
}

song_iterator_t *songit_clone(song_iterator_t *it, int delta) {
	SIMSG_SEND(it, SIMSG_CLONE(delta));
	it->death_listeners_nr = 0;
	it->flags |= SONGIT_FLAG_CLONE;
	return it;
}

void song_iterator_add_death_listener(song_iterator_t *it,
	void *client, void (*notify)(void *self, void *notifier)) {
	if (it->death_listeners_nr >= SONGIT_MAX_LISTENERS) {
		error("FATAL: Too many death listeners for song iterator");
	}

	it->death_listeners[it->death_listeners_nr].notify = notify;
	it->death_listeners[it->death_listeners_nr].self = client;

	it->death_listeners_nr++;
}

void song_iterator_remove_death_listener(song_iterator_t *it, void *client) {
	int i;
	for (i = 0; i < it->death_listeners_nr; i++) {
		if (it->death_listeners[i].self == client) {
			--it->death_listeners_nr;

			/* Overwrite, if this wasn't the last one */
			if (i + 1  < it->death_listeners_nr)
				it->death_listeners[i]
				= it->death_listeners[it->death_listeners_nr];

			return;
		}
	}

	error("FATAL: Could not remove death listener from song iterator\n");
}


song_iterator_t *sfx_iterator_combine(song_iterator_t *it1, song_iterator_t *it2) {
	if (it1 == NULL)
		return it2;
	if (it2 == NULL)
		return it1;

	/* Both are non-NULL: */
	return songit_new_tee(it1, it2, 1); /* 'may destroy' */
}

} // End of namespace Sci
