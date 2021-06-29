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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/audio.h"

#include "saga2/rect.h"
#include "saga2/queues.h"
#include "saga2/idtypes.h"
#include "saga2/audiosmp.h"
#include "saga2/audqueue.h"
#include "saga2/audiosys.h"

#include "saga2/audiobuf.h"

namespace Saga2 {

extern audioInterface *audio;

/*******************************************************************/
/*                                                                 */
/* Buffer Classes                                                  */
/*   Buffer : base class                                           */
/*   workBuffer : temporary work buffer                            */
/*   musicBuffer : AIL XMIDI song                                  */
/*                                                                 */
/* Member functions                                                */
/*   ctor, dtor : constructors & destructors                       */
/*   format : associate a particular audio format with a buffer    */
/*   laden  : determine whether the buffer can be written to       */
/*   sample_status : returns play status of buffer                 */
/*   washed : cleans the buffer if it's available                  */
/*   gave   : notify buffer that something was put into it         */
/*   took   : notify buffer that something was read from it        */
/*   fill   : tell buffer to consider itself full when it's not    */
/*   play   : called internally to play audio                      */
/*   activate : tell buffer it's being used                        */
/*   release : tell buffer its no longer in use                    */
/*   reset  : forcibly clears the buffer                           */
/*   setVolume : sets the volume ( if appropriate ) of a buffer      */
/*                                                                 */
/*******************************************************************/

Buffer::Buffer(size_t newSize) {
	assert(newSize > 0);

	internallyAllocated = true;
	size = newSize;
	data[0] = malloc(newSize);
	data[1] = NULL;

	wData = data[0];
	wSize = size;
	rData = data[0];
	rSize = 0;
}

Buffer::~Buffer(void) {
	if (internallyAllocated) {
		assert(data[0]);
		free(data[0]);
		data[0] = NULL;
	}
}

/***********************************/

workBuffer::workBuffer(size_t newSize, int16 newID)
	: Buffer(newSize) {
	bufID = newID;
	fillBuffer = 0;
	targetSated = false;
}

workBuffer::~workBuffer(void) {
}

/***********************************/


musicBuffer::musicBuffer(size_t newSize, audioInterface *sd, int16 newID)
	: Buffer(newSize) {
	warning("STUB: musicBuffer::musicBuffer()");
#if 0
	if (sd && sd->enabled(volMusic)) {
		assert(sd);
		assert(sd->mid);

		bufID = newID;
		fillBuffer = 0;
		targetSated = false;
		ailSampleHandle = AIL_allocate_sequence_handle(sd->mid);
		if (ailSampleHandle == 0)
			error("Unable to allocate music handle");
		data[1] = NULL;
		audioSet = 0;
	}
#endif
}

musicBuffer::~musicBuffer(void) {
	warning("STUB: musicBuffer::~musicBuffer()");
#if 0
	assert(ailSampleHandle);
	if (ailSampleHandle) {
		AIL_release_sequence_handle(ailSampleHandle);
	}
#endif
}


/*******************************************************************/
/*                                                                 */
/* This routine shifts the read buffer to the start of the buffer  */
/*   as a whole. This creates room for writing.                    */

void workBuffer::shiftdown(int16 bufNo) {
	long dif = size - (wSize + rSize);

	assert(bufNo == 0);
	assert(dif >= 0);
	assert(dif <= (size - (rSize + wSize)));
	assert(((char *)(data[bufNo]) + rSize) < ((char *) rData));
	assert(dif > rSize);
	assert(dif > wSize);
	assert(data[bufNo]);
	assert(rData);

	if (dif > 0 && rSize > 0) {
		char *tbuf = (char *)malloc(rSize);
		memcpy(tbuf, rData, rSize);
		memcpy(data[bufNo], tbuf, rSize);
		rData = data[bufNo];
		wSize += dif;
		wData = (void *)(((char *) data[bufNo]) + (size - wSize));
		free(tbuf);
	}

}

/*******************************************************************/
/*                                                                 */
/* Associate a particular audio format witha buffer                */


void Buffer::format(soundSample *) {
}


// work buffers could care less about format

void workBuffer::format(soundSample *) {
}

// sound buffers need to let AIL know about the formatting

void musicBuffer::format(soundSample *) {
}

/*******************************************************************/
/*                                                                 */
/* laden() determine whether a buffer can be written to.    */

bool Buffer::laden(void) {
	if (-1 == washed()) return true;
	activate(0);
	return false;
}

// fairly trivial for work buffers

bool workBuffer::laden(void) {
	if (-1 == washed())
		return true;
	activate(0);
	return false;
}

// sound buffers need to find out from AIL whether a buffer is free

bool musicBuffer::laden(void) {
	if (targetSated)
		activate(0);
	return (false);
}

/*******************************************************************/
/*                                                                 */
/* sample_status - find out whats going on with abuffer            */

uint32 Buffer::sample_status(void) {
	return (SMP_DONE);
}

uint32 workBuffer::sample_status(void) {
	return (SMP_DONE);
}


uint32 musicBuffer::sample_status(void) {
	warning("STUB: musicBuffer::sample_status()");
	return 0;
#if 0
	return (AIL_sequence_status(ailSampleHandle));
#endif
}

/*******************************************************************/
/*                                                                 */
/* washed() used internally to find a buffer with writeable     */
/*   space left.                                                   */


int16 Buffer::washed(void) {
	if ((rSize + wSize) == 0) {
		wSize = size;
		wData = data[0];
		rSize = 0;
		rData = data[0];
		return 0;
	}
	return 0;
}

int16 workBuffer::washed(void) {
	if ((rSize + wSize) == 0) {
		fillBuffer = 0;
		wSize = size;
		wData = data[fillBuffer];
		rSize = 0;
		rData = data[0];
		return 0;
	}
	return 0;
}

int16 musicBuffer::washed(void) {
	return 0;
}

/*******************************************************************/
/*                                                                 */
/* gave() notifies a buffer that it has been written to            */
/* took() notifies a buffer that it has been read from             */
/*                                                                 */

void Buffer::gave(size_t dSize) {
	assert(dSize <= wSize);
	wSize -= dSize;
	rSize += dSize;
	if (wSize)
		wData = (void *)(((char *) data[0]) + (size - wSize));

}

void workBuffer::gave(size_t dSize) {
	assert(dSize <= wSize);
	wSize -= dSize;
	rSize += dSize;
	if (wSize) {
		wData = (void *)(((char *) data[fillBuffer]) + (size - wSize));
	}

}

// when sound buffers get full they automatically trigger AIL

void musicBuffer::gave(size_t dSize) {
	assert(ailSampleHandle);
	assert(dSize <= wSize);
	wSize -= dSize;
	rSize += dSize;
	if (wSize) {
		wData = (void *)(((char *) data[fillBuffer]) + (size - wSize));
	} else {
		//fill( 0 )
		//play( 0 );
	}
}

// when work buffers get fully drained they reset themselves

void Buffer::took(size_t dSize) {
	assert(dSize <= rSize);
	rSize -= dSize;
	if (rSize > 0)
		rData = (void *)(((char *) rData) + dSize);
	if (rSize + wSize == 0)
		activate(0);
}

void workBuffer::took(size_t dSize) {
	assert(dSize <= rSize);
	rSize -= dSize;
	if (rSize > 0) {
		rData = (void *)(((char *) rData) + dSize);
	}
	if (rSize + wSize == 0) {
		activate(0);
	}
}

void musicBuffer::took(size_t dSize) {
	assert(dSize <= rSize);
	rSize -= dSize;
	if (rSize > 0)
		rData = (void *)(((char *) rData) + dSize);
}

/*******************************************************************/
/*                                                                 */
/* fill() - notify the buffer that it should deal with    */
/*   the current buffer data even though it is not a full buffer   */

void Buffer::fill(void) {
	if (rSize) {
		play(0);
		activate(0);
	}
}

// for work buffers flush any readable data and reset

void workBuffer::fill(void) {
	if (rSize) {
		play(0);
		activate(0);
	}
}

// sound buffers pass what they have to AIL

void musicBuffer::fill(void) {
	assert(ailSampleHandle);
	play(0);
}

/*******************************************************************/
/*                                                                 */
/* When a buffer is no longer needed this call resets it.          */

void Buffer::abortsound(void) {
}

void Buffer::release(void) {
}

void workBuffer::abortsound(void) {
	if (rSize)
		took(rSize);
	release();
}

void workBuffer::release(void) {
}

void musicBuffer::abortsound(void) {
	if (rSize)
		took(rSize);
	release();
}

void musicBuffer::release(void) {
	warning("STUB: musicBuffer::release()");
#if 0
	assert(ailSampleHandle);
// new
	AIL_end_sequence(ailSampleHandle);
	audioSet = 0;
#endif
}

/*******************************************************************/
/*                                                                 */
/* Used internally - for sound buffers this is the call that       */
/*   causes data to be passed along to AIL                         */

void Buffer::play(int16) {
	took(rSize);
}

void workBuffer::play(int16 bufNo) {
	assert(bufNo == 0);
	took(rSize);
}

void musicBuffer::play(int16 bufNo) {
	warning("STUB: musicBuffer::play()");
#if 0
	assert(bufNo == 0);
	assert(ailSampleHandle);
	if (AIL_init_sequence(ailSampleHandle, rData, 0) <= 0) {
		error("musicBuffer::play");
	}
	audioSet = 1;
	AIL_set_sequence_loop_count(ailSampleHandle, loopCount);
	AIL_start_sequence(ailSampleHandle);
	took(rSize);
#endif
}

/*******************************************************************/
/*                                                                 */
/* Used internally to keep track of and prepare buffers for writing*/

void Buffer::activate(int16) {
	if (washed() > -1) {
		wSize = size;
		wData = data[0];
		rSize = 0;
		rData = data[0];
	}
}

void workBuffer::activate(int16 bufNo) {
	assert(bufNo == 0);
	assert(rSize == 0);
	if (washed() > -1) {
		fillBuffer = 0;
		wSize = size;
		wData = data[fillBuffer];
		rSize = 0;
		rData = data[0];
	}
}

void musicBuffer::activate(int16 bufNo) {
	assert(ailSampleHandle);
	audioSet = 0;
	if (washed() > -1) {
		targetSated = false;
		fillBuffer = 0;
		wSize = size;
		wData = data[fillBuffer];
		rSize = 0;
		rData = data[fillBuffer];
	} else {
		fillBuffer = 0;
		wSize = 0;
		wData = data[0];
		rSize = 0;
		rData = data[0];
		targetSated = true;
	}
}

/*******************************************************************/
/* Initializes a buffer                                            */

void Buffer::reset(void) {
	wSize = size;
	wData = data[0];
	rSize = 0;
	rData = data[0];
}

void workBuffer::reset(void) {
	if (rSize) took(rSize);
	assert(rSize == 0);
	activate(0);
}

void musicBuffer::reset(void) {
	warning("STUB: musicBuffer::reset()");
#if 0
	AIL_end_sequence(ailSampleHandle);
	audioSet = 0;
	activate(0);
#endif
}

/*******************************************************************/
/* Initializes a buffer                                            */

void Buffer::setVolume(int8) {
}

void workBuffer::setVolume(int8) {
}

void musicBuffer::setVolume(int8 v) {
	warning("STUB: musicBuffer::setVolume()");
#if 0
	if (audioSet) {
		if (AIL_sequence_volume(ailSampleHandle) != v) {
			AIL_lock();
			AIL_set_sequence_volume(ailSampleHandle, v, 0);
			AIL_unlock();
		}
	}
#endif
}

void musicBuffer::fadeUp(int16 time, int8 volume) {
	warning("STUB: musicBuffer::fadeUp()");
#if 0
	if (audioSet) {
		AIL_lock();
		AIL_set_sequence_volume(ailSampleHandle, volume, time);
		AIL_unlock();
	}
#endif
}

void musicBuffer::fadeDown(int16 time) {
	warning("STUB: musicBuffer::fadeDown()");
#if 0
	if (audioSet) {
		AIL_lock();
		AIL_set_sequence_volume(ailSampleHandle, 0, time);
		AIL_unlock();
	}
#endif
}

} // end of namespace Saga2
