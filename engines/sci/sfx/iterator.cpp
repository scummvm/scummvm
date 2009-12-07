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

#include "sci/sci.h"
#include "sci/sfx/iterator_internal.h"
#include "sci/engine/state.h"	// for sfx_player_tell_synth :/
#include "sci/sfx/core.h"	// for sfx_player_tell_synth

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Sci {


static const int MIDI_cmdlen[16] = {0, 0, 0, 0, 0, 0, 0, 0,
                                    2, 2, 2, 2, 1, 1, 2, 0
                                   };

/*#define DEBUG_DECODING*/
/*#define DEBUG_VERBOSE*/

/** Find first set bit in bits and return its index. Returns 0 if bits is 0. */
static int sci_ffs(int bits) {
	if (!bits)
		return 0;

	int retval = 1;

	while (!(bits & 1)) {
		retval++;
		bits >>= 1;
	}

	return retval;
}

static void print_tabs_id(int nr, songit_id_t id) {
	while (nr-- > 0)
		fprintf(stderr, "\t");

	fprintf(stderr, "[%08lx] ", id);
}

BaseSongIterator::BaseSongIterator(byte *data, uint size, songit_id_t id)
	: _data(data, size) {
	ID = id;
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
	if (offset > _data.size()) { \
		warning("Reached end of song without terminator (%x/%x) at %d", offset, _data.size(), __LINE__); \
		return SI_FINISHED; \
	}

#define CHECK_FOR_END(offset_augment) \
	if ((channel->offset + (offset_augment)) > channel->end) { \
		channel->state = SI_STATE_FINISHED; \
		warning("Reached end of track %d without terminator (%x+%x/%x) at %d", channel->id, channel->offset, offset_augment, channel->end, __LINE__); \
		return SI_FINISHED; \
	}


static int _parse_ticks(byte *data, int *offset_p, int size) {
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


static int _sci0_get_pcm_data(Sci0SongIterator *self, int *rate, int *xoffset, uint *xsize);


#define PARSE_FLAG_LOOPS_UNLIMITED (1 << 0) /* Unlimited # of loops? */
#define PARSE_FLAG_PARAMETRIC_CUE (1 << 1) /* Assume that cues take an additional "cue value" argument */
/* This implements a difference between SCI0 and SCI1 cues. */

void SongIteratorChannel::init(int id_, int offset_, int end_) {
	playmask = PLAYMASK_NONE; /* Disable all channels */
	id = id_;
	state = SI_STATE_DELTA_TIME;
	loop_timepos = 0;
	total_timepos = 0;
	timepos_increment = 0;
	delay = 0; /* Only used for more than one channel */
	last_cmd = 0xfe;

	offset = loop_offset = initial_offset = offset_;
	end = end_;
}

void SongIteratorChannel::resetSynthChannels() {
	byte buf[5];

	// FIXME: Evil hack
	SfxState &sound = ((SciEngine*)g_engine)->getEngineState()->_sound;

	for (int i = 0; i < MIDI_CHANNELS; i++) {
		if (playmask & (1 << i)) {
			buf[0] = 0xe0 | i; /* Pitch bend */
			buf[1] = 0x80; /* Wheel center */
			buf[2] = 0x40;
			sound.sfx_player_tell_synth(3, buf);

			buf[0] = 0xb0 | i; // Set control
			buf[1] = 0x40; // Hold pedal
			buf[2] = 0x00; // Off
			sound.sfx_player_tell_synth(3, buf);
			/* TODO: Reset other controls? */
		}
	}
}

int BaseSongIterator::parseMidiCommand(byte *buf, int *result, SongIteratorChannel *channel, int flags) {
	byte cmd;
	int paramsleft;
	int midi_op;
	int midi_channel;

	channel->state = SI_STATE_DELTA_TIME;

	cmd = _data[channel->offset++];

	if (!(cmd & 0x80)) {
		/* 'Running status' mode */
		channel->offset--;
		cmd = channel->last_cmd;
	}

	if (cmd == 0xfe) {
		warning("song iterator subsystem: Corrupted sound resource detected.");
		return SI_FINISHED;
	}

	midi_op = cmd >> 4;
	midi_channel = cmd & 0xf;
	paramsleft = MIDI_cmdlen[midi_op];

#if 0
	if (1) {
		fprintf(stderr, "[IT]: off=%x, cmd=%02x, takes %d args ",
		        channel->offset - 1, cmd, paramsleft);
		fprintf(stderr, "[%02x %02x <%02x> %02x %02x %02x]\n",
		        _data[channel->offset-3],
		        _data[channel->offset-2],
		        _data[channel->offset-1],
		        _data[channel->offset],
		        _data[channel->offset+1],
		        _data[channel->offset+2]);
	}
#endif

	buf[0] = cmd;


	CHECK_FOR_END(paramsleft);
	memcpy(buf + 1, _data.begin() + channel->offset, paramsleft);
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
		    nextCommand(buf, result);


	if (cmd == SCI_MIDI_EOT) {
		/* End of track? */
		channel->resetSynthChannels();
		if (_loops > 1) {
			/* If allowed, decrement the number of loops */
			if (!(flags & PARSE_FLAG_LOOPS_UNLIMITED))
				*result = --_loops;

#ifdef DEBUG_DECODING
			fprintf(stderr, "%s L%d: (%p):%d Looping ", __FILE__, __LINE__, this, channel->id);
			if (flags & PARSE_FLAG_LOOPS_UNLIMITED)
				fprintf(stderr, "(indef.)");
			else
				fprintf(stderr, "(%d)", _loops);
			fprintf(stderr, " %x -> %x\n",
			        channel->offset, channel->loop_offset);
#endif
			channel->offset = channel->loop_offset;
			channel->state = SI_STATE_DELTA_TIME;
			channel->total_timepos = channel->loop_timepos;
			channel->last_cmd = 0xfe;
			debugC(2, kDebugLevelSound, "Looping song iterator %08lx.\n", ID);
			return SI_LOOP;
		} else {
			channel->state = SI_STATE_FINISHED;
			return SI_FINISHED;
		}

	} else if (cmd == SCI_MIDI_SET_SIGNAL) {
		if (buf[1] == SCI_MIDI_SET_SIGNAL_LOOP) {
			channel->loop_offset = channel->offset;
			channel->loop_timepos = channel->total_timepos;

			return /* Execute next command */
			    nextCommand(buf, result);
		} else {
			/* Used to be conditional <= 127 */
			*result = buf[1]; /* Absolute cue */
			return SI_ABSOLUTE_CUE;
		}
	} else if (SCI_MIDI_CONTROLLER(cmd)) {
		switch (buf[1]) {

		case SCI_MIDI_CUMULATIVE_CUE:
			if (flags & PARSE_FLAG_PARAMETRIC_CUE)
				_ccc += buf[2];
			else { /* No parameter to CC */
				_ccc++;
				/*				channel->offset--; */
			}
			*result = _ccc;
			return SI_RELATIVE_CUE;

		case SCI_MIDI_RESET_ON_SUSPEND:
			_resetflag = buf[2];
			break;

		case SCI_MIDI_SET_POLYPHONY:
			_polyphony[midi_channel] = buf[2];

#if 0
			{
				Sci1SongIterator *self1 = (Sci1SongIterator *)this;
				int i;
				int voices = 0;
				for (i = 0; i < self1->_numChannels; i++) {
					voices += _polyphony[i];
				}

				printf("SET_POLYPHONY(%d, %d) for a total of %d voices\n", midi_channel, buf[2], voices);
				printf("[iterator] DEBUG: Polyphony = [ ");
				for (i = 0; i < self1->_numChannels; i++)
					printf("%d ", _polyphony[i]);
				printf("]\n");
				printf("[iterator] DEBUG: Importance = [ ");
				printf("]\n");
			}
#endif
			break;

		case SCI_MIDI_SET_REVERB:
			break;

		case SCI_MIDI_CHANNEL_MUTE:
			warning("CHANNEL_MUTE(%d, %d)", midi_channel, buf[2]);
			break;

		case SCI_MIDI_HOLD: {
			// Safe cast: This controller is only used in SCI1
			Sci1SongIterator *self1 = (Sci1SongIterator *)this;

			if (buf[2] == self1->_hold) {
				channel->offset = channel->initial_offset;
				channel->state = SI_STATE_COMMAND;
				channel->total_timepos = 0;

				self1->_numLoopedChannels = self1->_numActiveChannels - 1;

				// FIXME:
				// This implementation of hold breaks getting out of the
				// limo when visiting the airport near the start of LSL5.
				// It seems like all channels should be reset here somehow,
				// but not sure how.
				// Forcing all channel offsets to 0 seems to fix the hang,
				// but somehow slows the exit sequence down to take 20 seconds
				// instead of about 3.

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
			    nextCommand(buf, result);

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
#if 0
		/* Perform remapping, if neccessary */
		if (cmd != SCI_MIDI_SET_SIGNAL
				&& cmd < 0xf0) { /* Not a generic command */
			int chan = cmd & 0xf;
			int op = cmd & 0xf0;

			chan = channel_remap[chan];
			buf[0] = chan | op;
		}
#endif

		/* Process as normal MIDI operation */
		return 0;
	}
}

int BaseSongIterator::processMidi(byte *buf, int *result,
	SongIteratorChannel *channel, int flags) {
	CHECK_FOR_END(0);

	switch (channel->state) {

	case SI_STATE_PCM: {
		if (_data[channel->offset] == 0
		        && _data[channel->offset + 1] == SCI_MIDI_EOT)
			/* Fake one extra tick to trick the interpreter into not killing the song iterator right away */
			channel->state = SI_STATE_PCM_MAGIC_DELTA;
		else
			channel->state = SI_STATE_DELTA_TIME;
		return SI_PCM;
	}

	case SI_STATE_PCM_MAGIC_DELTA: {
		int rate;
		int offset;
		uint size;
		int delay;
		if (_sci0_get_pcm_data((Sci0SongIterator *)this, &rate, &offset, &size))
			return SI_FINISHED; /* 'tis broken */
		channel->state = SI_STATE_FINISHED;
		delay = (size * 50 + rate - 1) / rate; /* number of ticks to completion*/

		debugC(2, kDebugLevelSound, "delaying %d ticks\n", delay);
		return delay;
	}

	case SI_STATE_UNINITIALISED:
		warning("Attempt to read command from uninitialized iterator");
		init();
		return nextCommand(buf, result);

	case SI_STATE_FINISHED:
		return SI_FINISHED;

	case SI_STATE_DELTA_TIME: {
		int offset;
		int ticks = _parse_ticks(_data.begin() + channel->offset,
		                         &offset,
		                         _data.size() - channel->offset);

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

		retval = parseMidiCommand(buf, result, channel, flags);

		if (retval == SI_FINISHED) {
			if (_numActiveChannels)
				--(_numActiveChannels);
#ifdef DEBUG_DECODING
			fprintf(stderr, "%s L%d: (%p):%d Finished channel, %d channels left\n",
			        __FILE__, __LINE__, this, channel->id,
			        _numActiveChannels);
#endif
			/* If we still have channels left... */
			if (_numActiveChannels) {
				return nextCommand(buf, result);
			}

			/* Otherwise, we have reached the end */
			_loops = 0;
		}

		return retval;
	}

	default:
		error("Invalid iterator state %d", channel->state);
		return SI_FINISHED;
	}
}

int Sci0SongIterator::nextCommand(byte *buf, int *result) {
	return processMidi(buf, result, &_channel, PARSE_FLAG_PARAMETRIC_CUE);
}

static int _sci0_header_magic_p(byte *data, int offset, int size) {
	if (offset + 0x10 > size)
		return 0;
	return (data[offset] == 0x1a)
	    && (data[offset + 1] == 0x00)
	    && (data[offset + 2] == 0x01)
	    && (data[offset + 3] == 0x00);
}


static int _sci0_get_pcm_data(Sci0SongIterator *self,
	int *rate, int *xoffset, uint *xsize) {
	int tries = 2;
	bool found_it = false;
	byte *pcm_data;
	int size;
	uint offset = SCI0_MIDI_OFFSET;

	if (self->_data[0] != 2)
		return 1;
	/* No such luck */

	while ((tries--) && (offset < self->_data.size()) && (!found_it)) {
		// Search through the garbage manually
		// FIXME: Replace offset by an iterator
		Common::Array<byte>::iterator iter = Common::find(self->_data.begin() + offset, self->_data.end(), SCI0_END_OF_SONG);

		if (iter == self->_data.end()) {
			warning("Playing unterminated song");
			return 1;
		}

		// add one to move it past the END_OF_SONG marker
		iter++;
		offset = iter - self->_data.begin();	// FIXME


		if (_sci0_header_magic_p(self->_data.begin(), offset, self->_data.size()))
			found_it = true;
	}

	if (!found_it) {
		warning("Song indicates presence of PCM, but"
		        " none found (finally at offset %04x)", offset);

		return 1;
	}

	pcm_data = self->_data.begin() + offset;

	size = READ_LE_UINT16(pcm_data + SCI0_PCM_SIZE_OFFSET);

	/* Two of the format parameters are fixed by design: */
	*rate = READ_LE_UINT16(pcm_data + SCI0_PCM_SAMPLE_RATE_OFFSET);

	if (offset + SCI0_PCM_DATA_OFFSET + size != self->_data.size()) {
		int d = offset + SCI0_PCM_DATA_OFFSET + size - self->_data.size();

		warning("PCM advertizes %d bytes of data, but %d"
		        " bytes are trailing in the resource",
		        size, self->_data.size() - (offset + SCI0_PCM_DATA_OFFSET));

		if (d > 0)
			size -= d; /* Fix this */
	}

	*xoffset = offset;
	*xsize = size;

	return 0;
}

static Audio::AudioStream *makeStream(byte *data, int size, int rate) {
	debugC(2, kDebugLevelSound, "Playing PCM data of size %d, rate %d\n", size, rate);

	// Duplicate the data
	byte *sound = (byte *)malloc(size);
	memcpy(sound, data, size);

	// Convert stream format flags
	int flags = Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_UNSIGNED;
	return Audio::makeLinearInputStream(sound, size, rate, flags, 0, 0);
}

Audio::AudioStream *Sci0SongIterator::getAudioStream() {
	int rate;
	int offset;
	uint size;
	if (_sci0_get_pcm_data(this, &rate, &offset, &size))
		return NULL;

	_channel.state = SI_STATE_FINISHED; /* Don't play both PCM and music */

	return makeStream(_data.begin() + offset + SCI0_PCM_DATA_OFFSET, size, rate);
}

SongIterator *Sci0SongIterator::handleMessage(Message msg) {
	if (msg._class == _SIMSG_BASE) {
		switch (msg._type) {

		case _SIMSG_BASEMSG_PRINT:
			print_tabs_id(msg._arg.i, ID);
			debugC(2, kDebugLevelSound, "SCI0: dev=%d, active-chan=%d, size=%d, loops=%d\n",
			        _deviceId, _numActiveChannels, _data.size(), _loops);
			break;

		case _SIMSG_BASEMSG_SET_LOOPS:
			_loops = msg._arg.i;
			break;

		case _SIMSG_BASEMSG_STOP: {
			songit_id_t sought_id = msg.ID;

			if (sought_id == ID)
				_channel.state = SI_STATE_FINISHED;
			break;
		}

		case _SIMSG_BASEMSG_SET_PLAYMASK: {
			int i;
			_deviceId = msg._arg.i;

			/* Set all but the rhytm channel mask bits */
			_channel.playmask &= ~(1 << MIDI_RHYTHM_CHANNEL);

			for (i = 0; i < MIDI_CHANNELS; i++)
				if (_data[2 + (i << 1)] & _deviceId
				        && i != MIDI_RHYTHM_CHANNEL)
					_channel.playmask |= (1 << i);
		}
		break;

		case _SIMSG_BASEMSG_SET_RHYTHM:
			_channel.playmask &= ~(1 << MIDI_RHYTHM_CHANNEL);
			if (msg._arg.i)
				_channel.playmask |= (1 << MIDI_RHYTHM_CHANNEL);
			break;

		case _SIMSG_BASEMSG_SET_FADE: {
			fade_params_t *fp = (fade_params_t *) msg._arg.p;
			fade.action = fp->action;
			fade.final_volume = fp->final_volume;
			fade.ticks_per_step = fp->ticks_per_step;
			fade.step_size = fp->step_size;
			break;
		}

		default:
			return NULL;
		}

		return this;
	}
	return NULL;
}

int Sci0SongIterator::getTimepos() {
	return _channel.total_timepos;
}

Sci0SongIterator::Sci0SongIterator(byte *data, uint size, songit_id_t id)
 : BaseSongIterator(data, size, id) {
	channel_mask = 0xffff;	// Allocate all channels by default
	_channel.state = SI_STATE_UNINITIALISED;

	for (int i = 0; i < MIDI_CHANNELS; i++)
		_polyphony[i] = data[1 + (i << 1)];

	init();
}

void Sci0SongIterator::init() {
	fade.action = FADE_ACTION_NONE;
	_resetflag = 0;
	_loops = 0;
	priority = 0;

	_ccc = 0; /* Reset cumulative cue counter */
	_numActiveChannels = 1;
	_channel.init(0, SCI0_MIDI_OFFSET, _data.size());
	_channel.resetSynthChannels();

	if (_data[0] == 2) /* Do we have an embedded PCM? */
		_channel.state = SI_STATE_PCM;
}

SongIterator *Sci0SongIterator::clone(int delta) {
	Sci0SongIterator *newit = new Sci0SongIterator(*this);
	return newit;
}


/***************************/
/*-- SCI1 song iterators --*/
/***************************/

#define SCI01_INVALID_DEVICE 0xff

/* Second index determines whether PCM output is supported */
static const int sci0_to_sci1_device_map[][2] = {
	{0x06, 0x0c}, /* MT-32 */
	{0xff, 0xff}, /* YM FB-01 */
	{0x00, 0x00}, /* CMS/Game Blaster-- we assume OPL/2 here... */
	{0xff, 0xff}, /* Casio MT540/CT460 */
	{0x13, 0x13}, /* Tandy 3-voice */
	{0x12, 0x12}, /* PC speaker */
	{0xff, 0xff},
	{0xff, 0xff},
}; /* Maps bit number to device ID */

int Sci1SongIterator::initSample(const int offset) {
	Sci1Sample sample;
	int rate;
	int length;
	int begin;
	int end;

	CHECK_FOR_END_ABSOLUTE((uint)offset + 10);
	if (_data[offset + 1] != 0)
		warning("[iterator-1] In sample at offset 0x04x: Byte #1 is %02x instead of zero",
		          _data[offset + 1]);

	rate = (int16)READ_LE_UINT16(_data.begin() + offset + 2);
	length = READ_LE_UINT16(_data.begin() + offset + 4);
	begin = (int16)READ_LE_UINT16(_data.begin() + offset + 6);
	end = (int16)READ_LE_UINT16(_data.begin() + offset + 8);

	CHECK_FOR_END_ABSOLUTE((uint)(offset + 10 + length));

	sample.delta = begin;
	sample.size = length;
	sample._data = _data.begin() + offset + 10;

#ifdef DEBUG_VERBOSE
	fprintf(stderr, "[SAMPLE] %x/%x/%x/%x l=%x\n",
	        offset + 10, begin, end, _data.size(), length);
#endif

	sample.rate = rate;

	sample.announced = false;

	/* Insert into the sample list at the right spot, keeping it sorted by delta */
	Common::List<Sci1Sample>::iterator seeker = _samples.begin();
	while (seeker != _samples.end() && seeker->delta < begin)
		++seeker;
	_samples.insert(seeker, sample);

	return 0; /* Everything's fine */
}

int Sci1SongIterator::initSong() {
	int last_time;
	uint offset = 0;
	_numChannels = 0;
	_samples.clear();
//	_deviceId = 0x0c;

	if (_data[offset] == 0xf0) {
		priority = _data[offset + 1];

		offset += 8;
	}

	while (_data[offset] != 0xff
	        && _data[offset] != _deviceId) {
		offset++;
		CHECK_FOR_END_ABSOLUTE(offset + 1);
		while (_data[offset] != 0xff) {
			CHECK_FOR_END_ABSOLUTE(offset + 7);
			offset += 6;
		}
		offset++;
	}

	if (_data[offset] == 0xff) {
		warning("[iterator] Song does not support hardware 0x%02x", _deviceId);
		return 1;
	}

	offset++;

	while (_data[offset] != 0xff) { /* End of list? */
		uint track_offset;
		int end;
		offset += 2;

		CHECK_FOR_END_ABSOLUTE(offset + 4);

		track_offset = READ_LE_UINT16(_data.begin() + offset);
		end = READ_LE_UINT16(_data.begin() + offset + 2);

		CHECK_FOR_END_ABSOLUTE(track_offset - 1);

		if (_data[track_offset] == 0xfe) {
			if (initSample(track_offset))
				return 1; /* Error */
		} else {
			/* Regular MIDI channel */
			if (_numChannels >= MIDI_CHANNELS) {
				warning("[iterator] Song has more than %d channels, cutting them off",
				          MIDI_CHANNELS);
				break; /* Scan for remaining samples */
			} else {
				int channel_nr = _data[track_offset] & 0xf;
				SongIteratorChannel &channel = _channels[_numChannels++];

				/*
				if (_data[track_offset] & 0xf0)
					printf("Channel %d has mapping bits %02x\n",
					       channel_nr, _data[track_offset] & 0xf0);
				*/

				// Add 2 to skip over header bytes */
				channel.init(channel_nr, track_offset + 2, track_offset + end);
				channel.resetSynthChannels();

				_polyphony[_numChannels - 1] = _data[channel.offset - 1] & 15;

				channel.playmask = ~0; /* Enable all */
				channel_mask |= (1 << channel_nr);

				CHECK_FOR_END_ABSOLUTE(offset + end);
			}
		}
		offset += 4;
		CHECK_FOR_END_ABSOLUTE(offset);
	}

	/* Now ensure that sample deltas are relative to the previous sample */
	last_time = 0;
	_numActiveChannels = _numChannels;
	_numLoopedChannels = 0;

	for (Common::List<Sci1Sample>::iterator seeker = _samples.begin();
			seeker != _samples.end(); ++seeker) {
		int prev_last_time = last_time;
		//printf("[iterator] Detected sample: %d Hz, %d bytes at time %d\n",
		//          seeker->format.rate, seeker->size, seeker->delta);
		last_time = seeker->delta;
		seeker->delta -= prev_last_time;
	}

	return 0; /* Success */
}

int Sci1SongIterator::getSmallestDelta() const {
	int d = -1;
	for (int i = 0; i < _numChannels; i++)
		if (_channels[i].state == SI_STATE_COMMAND
		        && (d == -1 || _channels[i].delay < d))
			d = _channels[i].delay;

	if (!_samples.empty() && _samples.begin()->delta < d)
		return _samples.begin()->delta;
	else
		return d;
}

void Sci1SongIterator::updateDelta(int delta) {
	if (!_samples.empty())
		_samples.begin()->delta -= delta;

	for (int i = 0; i < _numChannels; i++)
		if (_channels[i].state == SI_STATE_COMMAND)
			_channels[i].delay -= delta;
}

bool Sci1SongIterator::noDeltaTime() const {
	for (int i = 0; i < _numChannels; i++)
		if (_channels[i].state == SI_STATE_DELTA_TIME)
			return false;
	return true;
}

#define COMMAND_INDEX_NONE -1
#define COMMAND_INDEX_PCM -2

int Sci1SongIterator::getCommandIndex() const {
	/* Determine the channel # of the next active event, or -1 */
	int i;
	int base_delay = 0x7ffffff;
	int best_chan = COMMAND_INDEX_NONE;

	for (i = 0; i < _numChannels; i++)
		if ((_channels[i].state != SI_STATE_PENDING)
		        && (_channels[i].state != SI_STATE_FINISHED))  {

			if ((_channels[i].state == SI_STATE_DELTA_TIME)
			        && (_channels[i].delay == 0))
				return i;
			/* First, read all unknown delta times */

			if (_channels[i].delay < base_delay) {
				best_chan = i;
				base_delay = _channels[i].delay;
			}
		}

	if (!_samples.empty() && base_delay >= _samples.begin()->delta)
		return COMMAND_INDEX_PCM;

	return best_chan;
}


Audio::AudioStream *Sci1SongIterator::getAudioStream() {
	Common::List<Sci1Sample>::iterator sample = _samples.begin();
	if (sample != _samples.end() && sample->delta <= 0) {
		Audio::AudioStream *feed = makeStream(sample->_data, sample->size, sample->rate);
		_samples.erase(sample);

		return feed;
	} else
		return NULL;
}

int Sci1SongIterator::nextCommand(byte *buf, int *result) {

	if (!_initialised) {
		//printf("[iterator] DEBUG: Initialising for %d\n", _deviceId);
		_initialised = true;
		if (initSong())
			return SI_FINISHED;
	}


	if (_delayRemaining) {
		int delay = _delayRemaining;
		_delayRemaining = 0;
		return delay;
	}

	int retval = 0;
	do {	 /* All delays must be processed separately */
		int chan = getCommandIndex();

		if (chan == COMMAND_INDEX_NONE) {
			return SI_FINISHED;
		}

		if (chan == COMMAND_INDEX_PCM) {

			if (_samples.begin()->announced) {
				/* Already announced; let's discard it */
				Audio::AudioStream *feed = getAudioStream();
				delete feed;
			} else {
				int delay = _samples.begin()->delta;

				if (delay) {
					updateDelta(delay);
					return delay;
				}
				/* otherwise we're touching a PCM */
				_samples.begin()->announced = true;
				return SI_PCM;
			}
		} else { /* Not a PCM */

			retval = processMidi(buf, result,
			                     &(_channels[chan]),
			                     PARSE_FLAG_LOOPS_UNLIMITED);

			if (retval == SI_LOOP) {
				_numLoopedChannels++;
				_channels[chan].state = SI_STATE_PENDING;
				_channels[chan].delay = 0;

				if (_numLoopedChannels == _numActiveChannels) {
					int i;

					/* Everyone's ready: Let's loop */
					for (i = 0; i < _numChannels; i++)
						if (_channels[i].state == SI_STATE_PENDING)
							_channels[i].state = SI_STATE_DELTA_TIME;

					_numLoopedChannels = 0;
					return SI_LOOP;
				}
			} else if (retval == SI_FINISHED) {
#ifdef DEBUG
				fprintf(stderr, "FINISHED some channel\n");
#endif
			} else if (retval > 0) {
				int sd ;
				sd = getSmallestDelta();

				if (noDeltaTime() && sd) {
					/* No other channel is ready */
					updateDelta(sd);

					/* Only from here do we return delta times */
					return sd;
				}
			}

		} /* Not a PCM */

	} while (retval > 0);

	return retval;
}

SongIterator *Sci1SongIterator::handleMessage(Message msg) {
	if (msg._class == _SIMSG_BASE) { /* May extend this in the future */
		switch (msg._type) {

		case _SIMSG_BASEMSG_PRINT: {
			int playmask = 0;
			int i;

			for (i = 0; i < _numChannels; i++)
				playmask |= _channels[i].playmask;

			print_tabs_id(msg._arg.i, ID);
			debugC(2, kDebugLevelSound, "SCI1: chan-nr=%d, playmask=%04x\n",
			        _numChannels, playmask);
		}
		break;

		case _SIMSG_BASEMSG_STOP: {
			songit_id_t sought_id = msg.ID;
			int i;

			if (sought_id == ID) {
				ID = 0;

				for (i = 0; i < _numChannels; i++)
					_channels[i].state = SI_STATE_FINISHED;
			}
			break;
		}

		case _SIMSG_BASEMSG_SET_PLAYMASK:
			if (msg.ID == ID) {
				channel_mask = 0;

				_deviceId
				= sci0_to_sci1_device_map
				  [sci_ffs(msg._arg.i & 0xff) - 1]
				  [g_system->getMixer()->isReady()]
				  ;

				if (_deviceId == 0xff) {
					warning("[iterator] Device %d(%d) not supported",
					          msg._arg.i & 0xff, g_system->getMixer()->isReady());
				}
				if (_initialised) {
					int i;
					int toffset = -1;

					for (i = 0; i < _numChannels; i++)
						if (_channels[i].state != SI_STATE_FINISHED
						        && _channels[i].total_timepos > toffset) {
							toffset = _channels[i].total_timepos
							          + _channels[i].timepos_increment
							          - _channels[i].delay;
						}

					/* Find an active channel so that we can
					** get the correct time offset  */

					initSong();

					toffset -= _delayRemaining;
					_delayRemaining = 0;

					if (toffset > 0)
						return new_fast_forward_iterator(this, toffset);
				} else {
					initSong();
					_initialised = true;
				}

				break;

			}

		case _SIMSG_BASEMSG_SET_LOOPS:
			if (msg.ID == ID)
				_loops = (msg._arg.i > 32767) ? 99 : 0;
			/* 99 is arbitrary, but we can't use '1' because of
			** the way we're testing in the decoding section.  */
			break;

		case _SIMSG_BASEMSG_SET_HOLD:
			_hold = msg._arg.i;
			break;
		case _SIMSG_BASEMSG_SET_RHYTHM:
			/* Ignore */
			break;

		case _SIMSG_BASEMSG_SET_FADE: {
			fade_params_t *fp = (fade_params_t *) msg._arg.p;
			fade.action = fp->action;
			fade.final_volume = fp->final_volume;
			fade.ticks_per_step = fp->ticks_per_step;
			fade.step_size = fp->step_size;
			break;
		}

		default:
			warning("Unsupported command %d to SCI1 iterator", msg._type);
		}
		return this;
	}
	return NULL;
}

Sci1SongIterator::Sci1SongIterator(byte *data, uint size, songit_id_t id)
 : BaseSongIterator(data, size, id) {
	channel_mask = 0; // Defer channel allocation

	for (int i = 0; i < MIDI_CHANNELS; i++)
		_polyphony[i] = 0; // Unknown

	init();
}

void Sci1SongIterator::init() {
	fade.action = FADE_ACTION_NONE;
	_resetflag = 0;
	_loops = 0;
	priority = 0;

	_ccc = 0;
	_deviceId = 0x00; // Default to Sound Blaster/Adlib for purposes of cue computation
	_numChannels = 0;
	_initialised = false;
	_delayRemaining = 0;
	_loops = 0;
	_hold = 0;
	memset(_polyphony, 0, sizeof(_polyphony));
}

Sci1SongIterator::~Sci1SongIterator() {
}


SongIterator *Sci1SongIterator::clone(int delta) {
	Sci1SongIterator *newit = new Sci1SongIterator(*this);
	newit->_delayRemaining = delta;
	return newit;
}

int Sci1SongIterator::getTimepos() {
	int max = 0;
	int i;

	for (i = 0; i < _numChannels; i++)
		if (_channels[i].total_timepos > max)
			max = _channels[i].total_timepos;

	return max;
}

/**
 * A song iterator with the purpose of sending notes-off channel commands.
 */
class CleanupSongIterator : public SongIterator {
public:
	CleanupSongIterator(uint channels) {
		channel_mask = channels;
		ID = 17;
	}

	int nextCommand(byte *buf, int *result);
	Audio::AudioStream *getAudioStream() { return NULL; }
	SongIterator *handleMessage(Message msg);
	int getTimepos() { return 0; }
	SongIterator *clone(int delta) { return new CleanupSongIterator(*this); }
};

SongIterator *CleanupSongIterator::handleMessage(Message msg) {
	if (msg._class == _SIMSG_BASEMSG_PRINT && msg._type == _SIMSG_BASEMSG_PRINT) {
		print_tabs_id(msg._arg.i, ID);
		debugC(2, kDebugLevelSound, "CLEANUP\n");
	}

	return NULL;
}

int CleanupSongIterator::nextCommand(byte *buf, int *result) {
	/* Task: Return channel-notes-off for each channel */
	if (channel_mask) {
		int bs = sci_ffs(channel_mask) - 1;

		channel_mask &= ~(1 << bs);
		buf[0] = 0xb0 | bs; /* Controller */
		buf[1] = SCI_MIDI_CHANNEL_NOTES_OFF;
		buf[2] = 0; /* Hmm... */
		*result = 3;
		return 0;
	} else
		return SI_FINISHED;
}

/**********************/
/*-- Timer iterator --*/
/**********************/
int TimerSongIterator::nextCommand(byte *buf, int *result) {
	if (_delta) {
		int d = _delta;
		_delta = 0;
		return d;
	}
	return SI_FINISHED;
}

SongIterator *new_timer_iterator(int delta) {
	return new TimerSongIterator(delta);
}

/**********************************/
/*-- Fast-forward song iterator --*/
/**********************************/

int FastForwardSongIterator::nextCommand(byte *buf, int *result) {
	if (_delta <= 0)
		return SI_MORPH; /* Did our duty */

	while (1) {
		int rv = _delegate->nextCommand(buf, result);

		if (rv > 0) {
			/* Subtract from the delta we want to wait */
			_delta -= rv;

			/* Done */
			if (_delta < 0)
				return -_delta;
		}

		if (rv <= 0)
			return rv;
	}
}

Audio::AudioStream *FastForwardSongIterator::getAudioStream() {
	return _delegate->getAudioStream();
}

SongIterator *FastForwardSongIterator::handleMessage(Message msg) {
	if (msg._class == _SIMSG_PLASTICWRAP) {
		assert(msg._type == _SIMSG_PLASTICWRAP_ACK_MORPH);

		if (_delta <= 0) {
			SongIterator *it = _delegate;
			delete this;
			return it;
		}

		warning("[ff-iterator] Morphing without need");
		return this;
	}

	if (msg._class == _SIMSG_BASE && msg._type == _SIMSG_BASEMSG_PRINT) {
		print_tabs_id(msg._arg.i, ID);
		debugC(2, kDebugLevelSound, "FASTFORWARD:\n");
		msg._arg.i++;
	}

	// And continue with the delegate
	songit_handle_message(&_delegate, msg);

	return NULL;
}


int FastForwardSongIterator::getTimepos() {
	return _delegate->getTimepos();
}

FastForwardSongIterator::FastForwardSongIterator(SongIterator *capsit, int delta)
	: _delegate(capsit), _delta(delta) {

	channel_mask = capsit->channel_mask;
}

SongIterator *FastForwardSongIterator::clone(int delta) {
	FastForwardSongIterator *newit = new FastForwardSongIterator(*this);
	newit->_delegate = _delegate->clone(delta);
	return newit;
}

SongIterator *new_fast_forward_iterator(SongIterator *capsit, int delta) {
	if (capsit == NULL)
		return NULL;

	FastForwardSongIterator *it = new FastForwardSongIterator(capsit, delta);
	return it;
}


/********************/
/*-- Tee iterator --*/
/********************/


static void song_iterator_add_death_listener(SongIterator *it, TeeSongIterator *client) {
	for (int i = 0; i < SONGIT_MAX_LISTENERS; ++i) {
		if (it->_deathListeners[i] == 0) {
			it->_deathListeners[i] = client;
			return;
		}
	}
	error("FATAL: Too many death listeners for song iterator");
}

static void song_iterator_remove_death_listener(SongIterator *it, TeeSongIterator *client) {
	for (int i = 0; i < SONGIT_MAX_LISTENERS; ++i) {
		if (it->_deathListeners[i] == client) {
			it->_deathListeners[i] = 0;
			return;
		}
	}
}

static void song_iterator_transfer_death_listeners(SongIterator *it, SongIterator *it_from) {
	for (int i = 0; i < SONGIT_MAX_LISTENERS; ++i) {
		if (it_from->_deathListeners[i])
			song_iterator_add_death_listener(it, it_from->_deathListeners[i]);
		it_from->_deathListeners[i] = 0;
	}
}

static void songit_tee_death_notification(TeeSongIterator *self, SongIterator *corpse) {
	if (corpse == self->_children[TEE_LEFT].it) {
		self->_status &= ~TEE_LEFT_ACTIVE;
		self->_children[TEE_LEFT].it = NULL;
	} else if (corpse == self->_children[TEE_RIGHT].it) {
		self->_status &= ~TEE_RIGHT_ACTIVE;
		self->_children[TEE_RIGHT].it = NULL;
	} else {
		error("songit_tee_death_notification() failed: Breakpoint in %s, line %d", __FILE__, __LINE__);
	}
}

TeeSongIterator::TeeSongIterator(SongIterator *left, SongIterator *right) {
	int i;
	int firstfree = 1; /* First free channel */
	int incomplete_map = 0;

	_readyToMorph = false;
	_status = TEE_LEFT_ACTIVE | TEE_RIGHT_ACTIVE;

	_children[TEE_LEFT].it = left;
	_children[TEE_RIGHT].it = right;

	/* Default to lhs channels */
	channel_mask = left->channel_mask;
	for (i = 0; i < 16; i++)
		if (channel_mask & (1 << i) & right->channel_mask
		        && (i != MIDI_RHYTHM_CHANNEL) /* Share rhythm */) { /*conflict*/
			while ((firstfree == MIDI_RHYTHM_CHANNEL)
			        /* Either if it's the rhythm channel or if it's taken */
			        || (firstfree < MIDI_CHANNELS
			            && ((1 << firstfree) & channel_mask)))
				++firstfree;

			if (firstfree == MIDI_CHANNELS) {
				incomplete_map = 1;
				//warning("[songit-tee <%08lx,%08lx>] Could not remap right channel #%d: Out of channels",
				//        left->ID, right->ID, i);
			} else {
				_children[TEE_RIGHT].it->channel_remap[i] = firstfree;

				channel_mask |= (1 << firstfree);
			}
		}
#ifdef DEBUG_TEE_ITERATOR
	if (incomplete_map) {
		int c;
		fprintf(stderr, "[songit-tee <%08lx,%08lx>] Channels:"
		        " %04x <- %04x | %04x\n",
		        left->ID, right->ID,
		        channel_mask,
		        left->channel_mask, right->channel_mask);
		for (c = 0 ; c < 2; c++)
			for (i = 0 ; i < 16; i++)
				fprintf(stderr, "  map [%d][%d] -> %d\n",
				        c, i, _children[c].it->channel_remap[i]);
	}
#endif


	song_iterator_add_death_listener(left, this);
	song_iterator_add_death_listener(right, this);
}

TeeSongIterator::~TeeSongIterator() {
	// When we die, remove any listeners from our children
	if (_children[TEE_LEFT].it) {
		song_iterator_remove_death_listener(_children[TEE_LEFT].it, this);
	}

	if (_children[TEE_RIGHT].it) {
		song_iterator_remove_death_listener(_children[TEE_RIGHT].it, this);
	}
}


int TeeSongIterator::nextCommand(byte *buf, int *result) {
	static const int ready_masks[2] = {TEE_LEFT_READY, TEE_RIGHT_READY};
	static const int active_masks[2] = {TEE_LEFT_ACTIVE, TEE_RIGHT_ACTIVE};
	static const int pcm_masks[2] = {TEE_LEFT_PCM, TEE_RIGHT_PCM};
	int i;
	int retid;

#ifdef DEBUG_TEE_ITERATOR
	fprintf(stderr, "[Tee] %02x\n", _status);
#endif

	if (!(_status & (TEE_LEFT_ACTIVE | TEE_RIGHT_ACTIVE)))
		/* None is active? */
		return SI_FINISHED;

	if (_readyToMorph)
		return SI_MORPH;

	if ((_status & (TEE_LEFT_ACTIVE | TEE_RIGHT_ACTIVE))
	        != (TEE_LEFT_ACTIVE | TEE_RIGHT_ACTIVE)) {
		/* Not all are is active? */
		int which = 0;
#ifdef DEBUG_TEE_ITERATOR
		fprintf(stderr, "\tRequesting transformation...\n");
#endif
		if (_status & TEE_LEFT_ACTIVE)
			which = TEE_LEFT;
		else if (_status & TEE_RIGHT_ACTIVE)
			which = TEE_RIGHT;
		memcpy(buf, _children[which].buf, sizeof(buf));
		*result = _children[which].result;
		_readyToMorph = true;
		return _children[which].retval;
	}

	/* First, check for unreported PCMs */
	for (i = TEE_LEFT; i <= TEE_RIGHT; i++)
		if ((_status & (ready_masks[i] | pcm_masks[i]))
		        == (ready_masks[i] | pcm_masks[i])) {
			_status &= ~ready_masks[i];
			return SI_PCM;
		}

	for (i = TEE_LEFT; i <= TEE_RIGHT; i++)
		if (!(_status & ready_masks[i])) {

			/* Buffers aren't ready yet */
			_children[i].retval =
			    songit_next(&(_children[i].it),
			                _children[i].buf,
			                &(_children[i].result),
			                IT_READER_MASK_ALL
			                | IT_READER_MAY_FREE
			                | IT_READER_MAY_CLEAN);

			_status |= ready_masks[i];
#ifdef DEBUG_TEE_ITERATOR
			fprintf(stderr, "\t Must check %d: %d\n", i, _children[i].retval);
#endif

			if (_children[i].retval == SI_ABSOLUTE_CUE ||
			        _children[i].retval == SI_RELATIVE_CUE)
				return _children[i].retval;
			if (_children[i].retval == SI_FINISHED) {
				_status &= ~active_masks[i];
				/* Recurse to complete */
#ifdef DEBUG_TEE_ITERATOR
				fprintf(stderr, "\t Child %d signalled completion, recursing w/ status %02x\n", i, _status);
#endif
				return nextCommand(buf, result);
			} else if (_children[i].retval == SI_PCM) {
				_status |= pcm_masks[i];
				_status &= ~ready_masks[i];
				return SI_PCM;
			}
		}


	/* We've already handled PCM, MORPH and FINISHED, CUEs & LOOP remain */

	retid = TEE_LEFT;
	if ((_children[TEE_LEFT].retval > 0)
	        /* Asked to delay */
	        && (_children[TEE_RIGHT].retval <= _children[TEE_LEFT].retval))
		/* Is not delaying or not delaying as much */
		retid = TEE_RIGHT;

#ifdef DEBUG_TEE_ITERATOR
	fprintf(stderr, "\tl:%d / r:%d / chose %d\n",
	        _children[TEE_LEFT].retval, _children[TEE_RIGHT].retval, retid);
#endif

	/* Adjust delta times */
	if (_children[retid].retval > 0
	        && _children[1-retid].retval > 0) {
		if (_children[1-retid].retval
		        == _children[retid].retval)
			/* If both _children wait the same amount of time,
			** we have to re-fetch commands from both  */
			_status &= ~ready_masks[1-retid];
		else
			/* If they don't, we can/must re-use the other
			** child's delay time  */
			_children[1-retid].retval
			-= _children[retid].retval;
	}

	_status &= ~ready_masks[retid];
	memcpy(buf, _children[retid].buf, sizeof(buf));
	*result = _children[retid].result;

	return _children[retid].retval;
}

Audio::AudioStream *TeeSongIterator::getAudioStream() {
	static const int pcm_masks[2] = {TEE_LEFT_PCM, TEE_RIGHT_PCM};
	int i;

	for (i = TEE_LEFT; i <= TEE_RIGHT; i++)
		if (_status & pcm_masks[i]) {
			_status &= ~pcm_masks[i];
			return _children[i].it->getAudioStream();
		}

	return NULL; // No iterator
}

SongIterator *TeeSongIterator::handleMessage(Message msg) {
	if (msg._class == _SIMSG_PLASTICWRAP) {
		assert(msg._type == _SIMSG_PLASTICWRAP_ACK_MORPH);

		SongIterator *old_it;
		if (!(_status & (TEE_LEFT_ACTIVE | TEE_RIGHT_ACTIVE))) {
			delete this;
			return NULL;
		} else if (!(_status & TEE_LEFT_ACTIVE)) {
			delete _children[TEE_LEFT].it;
			_children[TEE_LEFT].it = 0;
			old_it = _children[TEE_RIGHT].it;
			song_iterator_remove_death_listener(old_it, this);
			song_iterator_transfer_death_listeners(old_it, this);
			delete this;
			return old_it;
		} else if (!(_status & TEE_RIGHT_ACTIVE)) {
			delete _children[TEE_RIGHT].it;
			_children[TEE_RIGHT].it = 0;
			old_it = _children[TEE_LEFT].it;
			song_iterator_remove_death_listener(old_it, this);
			song_iterator_transfer_death_listeners(old_it, this);
			delete this;
			return old_it;
		}

		warning("[tee-iterator] Morphing without need");
		return this;
	}

	if (msg._class == _SIMSG_BASE && msg._type == _SIMSG_BASEMSG_PRINT) {
		print_tabs_id(msg._arg.i, ID);
		debugC(2, kDebugLevelSound, "TEE:\n");
		msg._arg.i++;
	}

	// And continue with the children
	if (_children[TEE_LEFT].it)
		songit_handle_message(&(_children[TEE_LEFT].it), msg);
	if (_children[TEE_RIGHT].it)
		songit_handle_message(&(_children[TEE_RIGHT].it), msg);

	return NULL;
}

void TeeSongIterator::init() {
	_status = TEE_LEFT_ACTIVE | TEE_RIGHT_ACTIVE;
	_children[TEE_LEFT].it->init();
	_children[TEE_RIGHT].it->init();
}

SongIterator *TeeSongIterator::clone(int delta) {
	TeeSongIterator *newit = new TeeSongIterator(*this);

	if (_children[TEE_LEFT].it)
		newit->_children[TEE_LEFT].it = _children[TEE_LEFT].it->clone(delta);
	if (_children[TEE_RIGHT].it)
		newit->_children[TEE_RIGHT].it = _children[TEE_RIGHT].it->clone(delta);

	return newit;
}


/*************************************/
/*-- General purpose functionality --*/
/*************************************/

int songit_next(SongIterator **it, byte *buf, int *result, int mask) {
	int retval;

	if (!*it)
		return SI_FINISHED;

	do {
		retval = (*it)->nextCommand(buf, result);
		if (retval == SI_MORPH) {
			debugC(2, kDebugLevelSound, "  Morphing %p (stored at %p)\n", (void *)*it, (void *)it);
			if (!SIMSG_SEND((*it), SIMSG_ACK_MORPH)) {
				error("SI_MORPH failed. Breakpoint in %s, line %d", __FILE__, __LINE__);
			} else
				debugC(2, kDebugLevelSound, "SI_MORPH successful\n");
		}

		if (retval == SI_FINISHED)
			debugC(2, kDebugLevelSound, "[song-iterator] Song finished. mask = %04x, cm=%04x\n",
			        mask, (*it)->channel_mask);
		if (retval == SI_FINISHED
		        && (mask & IT_READER_MAY_CLEAN)
		        && (*it)->channel_mask) { /* This last test will fail
					       ** with a terminated
					       ** cleanup iterator */
			int channel_mask = (*it)->channel_mask;

			SongIterator *old_it = *it;
			*it = new CleanupSongIterator(channel_mask);
			for(uint i = 0; i < MIDI_CHANNELS; i++)
				(*it)->channel_remap[i] = old_it->channel_remap[i];
			song_iterator_transfer_death_listeners(*it, old_it);
			if (mask & IT_READER_MAY_FREE)
				delete old_it;
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

	if (retval == SI_FINISHED && (mask & IT_READER_MAY_FREE)) {
		delete *it;
		*it = NULL;
	}

	return retval;
}

SongIterator::SongIterator() {
	ID = 0;
	channel_mask = 0;
	fade.action = FADE_ACTION_NONE;
	priority = 0;
	memset(_deathListeners, 0, sizeof(_deathListeners));

	// By default, don't remap
	for (uint i = 0; i < 16; i++)
		channel_remap[i] = i;
}

SongIterator::SongIterator(const SongIterator &si) {
	ID = si.ID;
	channel_mask = si.channel_mask;
	fade = si.fade;
	priority = si.priority;
	memset(_deathListeners, 0, sizeof(_deathListeners));

	for (uint i = 0; i < 16; i++)
		channel_remap[i] = si.channel_remap[i];
}


SongIterator::~SongIterator() {
	for (int i = 0; i < SONGIT_MAX_LISTENERS; ++i)
		if (_deathListeners[i])
			songit_tee_death_notification(_deathListeners[i], this);
}

SongIterator *songit_new(byte *data, uint size, SongIteratorType type, songit_id_t id) {
	BaseSongIterator *it;

	if (!data || size < 22) {
		warning("Attempt to instantiate song iterator for null song data");
		return NULL;
	}


	switch (type) {
	case SCI_SONG_ITERATOR_TYPE_SCI0:
		it = new Sci0SongIterator(data, size, id);
		break;

	case SCI_SONG_ITERATOR_TYPE_SCI1:
		it = new Sci1SongIterator(data, size, id);
		break;

	default:
		/**-- Invalid/unsupported sound resources --**/
		warning("Attempt to instantiate invalid/unknown song iterator type %d", type);
		return NULL;
	}

	return it;
}

int songit_handle_message(SongIterator **it_reg_p, SongIterator::Message msg) {
	SongIterator *it = *it_reg_p;
	SongIterator *newit;

	newit = it->handleMessage(msg);

	if (!newit)
		return 0; /* Couldn't handle */

	*it_reg_p = newit; /* Might have self-morphed */
	return 1;
}

SongIterator *sfx_iterator_combine(SongIterator *it1, SongIterator *it2) {
	if (it1 == NULL)
		return it2;
	if (it2 == NULL)
		return it1;

	/* Both are non-NULL: */
	return new TeeSongIterator(it1, it2);
}

} // End of namespace Sci
