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

#include "iterator.h"
#include "iterator_internal.h"
#include <stdarg.h>
#include <stdio.h>

using namespace Sci;

#define ASSERT_S(x) if (!(x)) { error("Failed assertion in L%d: " #x, __LINE__); return; }
#define ASSERT(x) ASSERT_S(x)

/* Tests the song iterators */

int errors = 0;

void error(char *fmt, ...) {
	va_list ap;

	fprintf(stderr, "[ERROR] ");
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	++errors;
}


/* The simple iterator will finish after a fixed amount of time.  Before that,
** it emits (absolute) cues in ascending order.  */
struct simple_iterator : public SongIterator {
	int lifetime_remaining;
	char *cues;
	int cue_counter;
	int cue_progress;
	int cues_nr;
};

int simple_it_next(SongIterator *_self, unsigned char *buf, int *result) {
	simple_iterator *self = (simple_iterator *)_self;

	if (self->lifetime_remaining == -1) {
		error("Song iterator called post mortem");
		return SI_FINISHED;
	}

	if (self->lifetime_remaining) {

		if (self->cue_counter < self->cues_nr) {
			int time_to_cue = self->cues[self->cue_counter];

			if (self->cue_progress == time_to_cue) {
				++self->cue_counter;
				self->cue_progress = 0;
				*result = self->cue_counter;
				return SI_ABSOLUTE_CUE;
			} else {
				int retval = time_to_cue - self->cue_progress;
				self->cue_progress = time_to_cue;

				if (retval > self->lifetime_remaining) {
					retval = self->lifetime_remaining;
					self->lifetime_remaining = 0;
					self->cue_progress = retval;
					return retval;
				}

				self->lifetime_remaining -= retval;
				return retval;
			}
		} else {
			int retval = self->lifetime_remaining;
			self->lifetime_remaining = 0;
			return retval;
		}

	} else {
		self->lifetime_remaining = -1;
		return SI_FINISHED;
	}
}

Audio::AudioStream *simple_it_pcm_feed(SongIterator *_self) {
	error("No PCM feed");
	return NULL;
}

void simple_it_init(SongIterator *_self) {
}

SongIterator *simple_it_handle_message(SongIterator *_self, SongIterator::Message msg) {
	return NULL;
}

void simple_it_cleanup(SongIterator *_self) {
}

/* Initialises the simple iterator.
** Parameters: (int) delay: Number of ticks until the iterator finishes
**             (int *) cues: An array of cue delays (cue values are [1,2...])
**             (int) cues_nr:  Number of cues in ``cues''
** The first cue is emitted after cues[0] ticks, and it is 1.  After cues[1] additional ticks
** the next cue is emitted, and so on. */
SongIterator *setup_simple_iterator(int delay, char *cues, int cues_nr) {
	simple_iterator.lifetime_remaining = delay;
	simple_iterator.cues = cues;
	simple_iterator.cue_counter = 0;
	simple_iterator.cues_nr = cues_nr;
	simple_iterator.cue_progress = 0;

	simple_iterator.ID = 42;
	simple_iterator.channel_mask = 0x004f;
	simple_iterator.flags = 0;
	simple_iterator.priority = 1;

	simple_iterator.death_listeners_nr = 0;

	simple_iterator.cleanup = simple_it_cleanup;
	simple_iterator.init = simple_it_init;
	simple_iterator.handle_message = simple_it_handle_message;
	simple_iterator.get_pcm_feed = simple_it_pcm_feed;
	simple_iterator.next = simple_it_next;

	return (SongIterator *) &simple_iterator;
}

#define ASSERT_SIT ASSERT(it == simple_it)
#define ASSERT_FFIT ASSERT(it == ff_it)
#define ASSERT_NEXT(n) ASSERT(songit_next(&it, data, &result, IT_READER_MASK_ALL) == n)
#define ASSERT_RESULT(n) ASSERT(result == n)
#define ASSERT_CUE(n) ASSERT_NEXT(SI_ABSOLUTE_CUE); ASSERT_RESULT(n)

void test_simple_it() {
	SongIterator *it;
	SongIterator *simple_it = (SongIterator *) & simple_iterator;
	unsigned char data[4];
	int result;
	puts("[TEST] simple iterator (test artifact)");

	it = setup_simple_iterator(42, NULL, 0);

	ASSERT_SIT;
	ASSERT_NEXT(42);
	ASSERT_SIT;
	ASSERT_NEXT(SI_FINISHED);
	ASSERT_SIT;

	it = setup_simple_iterator(42, "\003\004", 2);
	ASSERT_SIT;
	ASSERT_NEXT(3);
	ASSERT_CUE(1);
	ASSERT_SIT;
	ASSERT_NEXT(4);
	ASSERT_CUE(2);
	ASSERT_SIT;
//	warning("XXX => %d", songit_next(&it, data, &result, IT_READER_MASK_ALL));
	ASSERT_NEXT(35);
	ASSERT_NEXT(SI_FINISHED);
	ASSERT_SIT;

	puts("[TEST] Test OK.");
}

void test_fastforward() {
	SongIterator *it;
	SongIterator *simple_it = (SongIterator *) & simple_iterator;
	SongIterator *ff_it;
	unsigned char data[4];
	int result;
	puts("[TEST] fast-forward iterator");

	it = setup_simple_iterator(42, NULL, 0);
	ff_it = it = new_fast_forward_iterator(it, 0);
	ASSERT_FFIT;
	ASSERT_NEXT(42);
	ASSERT_SIT; /* Must have morphed back */
	ASSERT_NEXT(SI_FINISHED);
	ASSERT_SIT;

	it = setup_simple_iterator(42, NULL, 0);
	ff_it = it = new_fast_forward_iterator(it, 1);
	ASSERT_FFIT;
	ASSERT_NEXT(41);
	/* May or may not have morphed back here */
	ASSERT_NEXT(SI_FINISHED);
	ASSERT_SIT;

	it = setup_simple_iterator(42, NULL, 0);
	ff_it = it = new_fast_forward_iterator(it, 41);
	ASSERT_FFIT;
	ASSERT_NEXT(1);
	/* May or may not have morphed back here */
	ASSERT_NEXT(SI_FINISHED);
	ASSERT_SIT;

	it = setup_simple_iterator(42, NULL, 0);
	ff_it = it = new_fast_forward_iterator(it, 42);
	ASSERT_NEXT(SI_FINISHED);
	/* May or may not have morphed back here */

	it = setup_simple_iterator(42, NULL, 0);
	ff_it = it = new_fast_forward_iterator(it, 10000);
	ASSERT_NEXT(SI_FINISHED);
	/* May or may not have morphed back here */

	it = setup_simple_iterator(42, "\003\004", 2);
	ff_it = it = new_fast_forward_iterator(it, 2);
	ASSERT_FFIT;
	ASSERT_NEXT(1);
	ASSERT_CUE(1);
	ASSERT_SIT;
	ASSERT_NEXT(4);
	ASSERT_CUE(2);
	ASSERT_SIT;
	ASSERT_NEXT(35);
	ASSERT_NEXT(SI_FINISHED);
	ASSERT_SIT;

	it = setup_simple_iterator(42, "\003\004", 2);
	ff_it = it = new_fast_forward_iterator(it, 5);
	ASSERT_FFIT;
	ASSERT_CUE(1);
	ASSERT_FFIT;
	ASSERT_NEXT(2);
	ASSERT_CUE(2);
	ASSERT_SIT;
	ASSERT_NEXT(35);
	ASSERT_NEXT(SI_FINISHED);
	ASSERT_SIT;

	it = setup_simple_iterator(42, "\003\004", 2);
	ff_it = it = new_fast_forward_iterator(it, 41);
	ASSERT_FFIT;
	ASSERT_CUE(1);
	ASSERT_FFIT;
	ASSERT_CUE(2);
	ASSERT_FFIT;
	ASSERT_NEXT(1);
	ASSERT_NEXT(SI_FINISHED);
	ASSERT_SIT;

	puts("[TEST] Test OK.");
}

#define SIMPLE_SONG_SIZE 50

static unsigned char simple_song[SIMPLE_SONG_SIZE] = {
	0x00, /* Regular song */
	/* Only use channel 0 for all devices */
	0x02, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* Song begins here */
	42, 0x90, 60, 0x7f,		/* Play C after 42 ticks */
	02, 64, 0x42,			/* Play E after 2 more ticks, using running status mode */
	0xf8, 10, 0x80, 60, 0x02,	/* Stop C after 250 ticks */
	0, 64, 0x00,			/* Stop E immediately */
	00, 0xfc	/* Stop song */
};

#define ASSERT_MIDI3(cmd, arg0, arg1) \
	ASSERT(data[0] == cmd);		\
	ASSERT(data[1] == arg0);	\
	ASSERT(data[2] == arg1);

void test_iterator_sci0() {
	SongIterator *it = songit_new(simple_song, SIMPLE_SONG_SIZE, SCI_SONG_ITERATOR_TYPE_SCI0, 0l);
	unsigned char data[4];
	int result;
	SIMSG_SEND(it, SIMSG_SET_PLAYMASK(0x0001)); /* Initialise song, enabling channel 0 */

	puts("[TEST] SCI0-style song");
	ASSERT_NEXT(42);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x90, 60, 0x7f);
	ASSERT_NEXT(2);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x90, 64, 0x42);
	ASSERT_NEXT(250);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x80, 60, 0x02);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x80, 64, 0x00);
	ASSERT_NEXT(SI_FINISHED);
	puts("[TEST] Test OK.");
}



void test_iterator_sci0_loop() {
	SongIterator *it = songit_new(simple_song, SIMPLE_SONG_SIZE, SCI_SONG_ITERATOR_TYPE_SCI0, 0l);
	unsigned char data[4];
	int result;
	SIMSG_SEND(it, SIMSG_SET_PLAYMASK(0x0001)); /* Initialise song, enabling channel 0 */
	SIMSG_SEND(it, SIMSG_SET_LOOPS(2)); /* Loop one additional time */

	puts("[TEST] SCI0-style song with looping");
	ASSERT_NEXT(42);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x90, 60, 0x7f);
	ASSERT_NEXT(2);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x90, 64, 0x42);
	ASSERT_NEXT(250);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x80, 60, 0x02);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x80, 64, 0x00);
	ASSERT_NEXT(SI_LOOP);
	ASSERT_NEXT(42);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x90, 60, 0x7f);
	ASSERT_NEXT(2);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x90, 64, 0x42);
	ASSERT_NEXT(250);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x80, 60, 0x02);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x80, 64, 0x00);
	ASSERT_NEXT(SI_FINISHED);
	puts("[TEST] Test OK.");
}



#define LOOP_SONG_SIZE 54

unsigned char loop_song[LOOP_SONG_SIZE] = {
	0x00, /* Regular song song */
	/* Only use channel 0 for all devices */
	0x02, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* Song begins here */
	42, 0x90, 60, 0x7f,	/* Play C after 42 ticks */
	13, 0x80, 60, 0x00,	/* Stop C after 13 ticks */
	00, 0xCF, 0x7f,	/* Set loop point */
	02, 0x90, 64, 0x42,	/* Play E after 2 more ticks, using running status mode */
	03, 0x80, 64, 0x00,	/* Stop E after 3 ticks */
	00, 0xfc	/* Stop song/loop */
};


void test_iterator_sci0_mark_loop() {
	SongIterator *it = songit_new(loop_song, LOOP_SONG_SIZE, SCI_SONG_ITERATOR_TYPE_SCI0, 0l);
	unsigned char data[4];
	int result;
	SIMSG_SEND(it, SIMSG_SET_PLAYMASK(0x0001)); /* Initialise song, enabling channel 0 */
	SIMSG_SEND(it, SIMSG_SET_LOOPS(3)); /* Loop once more */

	puts("[TEST] SCI0-style song with loop mark, looping");
	ASSERT_NEXT(42);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x90, 60, 0x7f);
	ASSERT_NEXT(13);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x80, 60, 0x00);
	/* Loop point here: we don't observe that in the iterator interface yet, though */
	ASSERT_NEXT(2);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x90, 64, 0x42);
	ASSERT_NEXT(3);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x80, 64, 0x00);
	/* Now we loop back to the loop pont */
	ASSERT_NEXT(SI_LOOP);
	ASSERT_NEXT(2);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x90, 64, 0x42);
	ASSERT_NEXT(3);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x80, 64, 0x00);
	/* ...and one final time */
	ASSERT_NEXT(SI_LOOP);
	ASSERT_NEXT(2);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x90, 64, 0x42);
	ASSERT_NEXT(3);
	ASSERT_NEXT(0);
	ASSERT_MIDI3(0x80, 64, 0x00);

	ASSERT_NEXT(SI_FINISHED);
	puts("[TEST] Test OK.");
}



int main(int argc, char **argv) {
	test_simple_it();
	test_fastforward();
	test_iterator_sci0();
	test_iterator_sci0_loop();
	test_iterator_sci0_mark_loop();
	if (errors != 0)
		warning("[ERROR] %d errors total", errors);
	return (errors != 0);
}
