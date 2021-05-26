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

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/audio.h"

namespace Saga2 {

extern audioInterface *audio;

/*******************************************************************/
/*                                                                 */
/* Buffer Classes                                                  */
/*   Buffer : base class                                           */
/*   workBuffer : temporary work buffer                            */
/*   soundBuffer : AIL double buffered sound                       */
/*   singleBuffer : AIL single buffered sound                      */
/*   musicBuffer : AIL XMIDI song                                  */
/*   cacheBuffer : in memory storage                               */
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
	VERIFY(newSize > 0);

	internallyAllocated = TRUE;
	size = newSize;
	data[0] = audioAlloc(newSize, "Audio data buffer"); //( void * ) new char[newSize];
	audio_lock(data[0], size);   //musicRes->size( s ));
	data[1] = NULL;

	wData = data[0];
	wSize = size;
	rData = data[0];
	rSize = 0;
}

Buffer::~Buffer(void) {
	if (internallyAllocated) {
		VERIFY(data[0]);
		audio_unlock(data[0], size);
		audioFree(data[0]);  //delete [] data[0];
		data[0] = NULL;
	}
}

/***********************************/

workBuffer::workBuffer(size_t newSize, int16 newID)
	: Buffer(newSize) {
	bufID = newID;
	fillBuffer = 0;
	targetSated = FALSE;
}

workBuffer::~workBuffer(void) {
}

/***********************************/

cacheBuffer::cacheBuffer(size_t newSize, int16 newID)
	: Buffer(newSize) {
	bufID = newID;
	hasData = 0;
}

cacheBuffer::~cacheBuffer(void) {
}

/***********************************/


doubleBuffer::doubleBuffer(size_t newSize, audioInterface *sd, int16 newID)
	: Buffer(newSize) {
	if (sd && sd->enabled(volVoice)) {
		VERIFY(sd);
		//VERIFY( sd->dig );

		bufID = newID;
		fillBuffer = 0;
		targetPos = 0;
		targetSated = FALSE;
		ailSampleHandle = AIL_allocate_sample_handle(sd->dig);
		if (ailSampleHandle == NULL)
			audioFatal("Unable to allocate audio handle");
		AIL_init_sample(ailSampleHandle);
		AILLOCated = -1;
		audioSet = 0;

		data[1] = audioAlloc(newSize, "audio double buffer"); // ( void * ) new char[newSize];
		audio_lock(data[1], newSize);   //musicRes->size( s ));
		//drain( 1 );
	}
}

doubleBuffer::~doubleBuffer(void) {
	VERIFY(ailSampleHandle);
	if (data[1]) {
		audio_unlock(data[1], size);
		audioFree(data[1]);  //delete [] data[1];
		data[1] = NULL;
	}
	if (ailSampleHandle) {
		AIL_release_sample_handle(ailSampleHandle);
	}
}


/***********************************/


singleBuffer::singleBuffer(size_t newSize, audioInterface *sd, int16 newID)
	: Buffer(newSize) {
	if (sd && sd->enabled(volSound)) {
		VERIFY(sd);
		VERIFY(sd->dig);

		bufID = newID;
		fillBuffer = 0;
		targetSated = FALSE;
		ailSampleHandle = AIL_allocate_sample_handle(sd->dig);
		if (ailSampleHandle == NULL)
			audioFatal("Unable to allocate audio handle");
		AIL_init_sample(ailSampleHandle);
		AILLOCated = -1;
		audioSet = 0;
		data[1] = NULL;
		lastRSize = 0;
	}
}

singleBuffer::~singleBuffer(void) {
	VERIFY(ailSampleHandle);
	if (ailSampleHandle) {
		AIL_release_sample_handle(ailSampleHandle);
	}
}


/***********************************/


musicBuffer::musicBuffer(size_t newSize, audioInterface *sd, int16 newID)
	: Buffer(newSize) {
	if (sd && sd->enabled(volMusic)) {
		VERIFY(sd);
		VERIFY(sd->mid);

		bufID = newID;
		fillBuffer = 0;
		targetSated = FALSE;
		ailSampleHandle = AIL_allocate_sequence_handle(sd->mid);
		if (ailSampleHandle == NULL)
			audioFatal("Unable to allocate music handle");
		data[1] = NULL;
		audioSet = 0;
	}
}

musicBuffer::~musicBuffer(void) {
	VERIFY(ailSampleHandle);
	if (ailSampleHandle) {
		AIL_release_sequence_handle(ailSampleHandle);
	}
}


/*******************************************************************/
/*                                                                 */
/* This routine shifts the read buffer to the start of the buffer  */
/*   as a whole. This creates room for writing.                    */

void workBuffer::shiftdown(int16 bufNo) {
	long dif = size - (wSize + rSize);

	VERIFY(bufNo == 0);
	VERIFY(dif >= 0);
	VERIFY(dif <= (size - (rSize + wSize)));
	VERIFY(((char *)(data[bufNo]) + rSize) < ((char *) rData));
	VERIFY(dif > rSize);
	VERIFY(dif > wSize);
	VERIFY(data[bufNo]);
	VERIFY(rData);

	if (dif > 0 && rSize > 0) {
		char *tbuf = (char *) audioAlloc(rSize, "audio work buffer"); //new char[rSize];
		memcpy(tbuf, rData, rSize);
		memcpy(data[bufNo], tbuf, rSize);
		rData = data[bufNo];
		wSize += dif;
		wData = (void *)(((char *) data[bufNo]) + (size - wSize));
		audioFree(tbuf);
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

// work buffers could care less about format

void cacheBuffer::format(soundSample *) {

}

// sound buffers need to let AIL know about the formatting

void doubleBuffer::format(soundSample *ss) {
	if (audioSet == 0) {
		VERIFY(ailSampleHandle);
		AIL_init_sample(ailSampleHandle);
		AIL_set_sample_type(ailSampleHandle, ss->format(), ss->flags());
		AIL_set_sample_playback_rate(ailSampleHandle, ss->speed);
		AIL_set_sample_volume(ailSampleHandle, ss->getVolume());
		activate(washed());
		targetPos = 0;
		audioSet = 1;
	}
}

void singleBuffer::format(soundSample *ss) {
	VERIFY(ailSampleHandle);
	AIL_init_sample(ailSampleHandle);
	AIL_set_sample_type(ailSampleHandle, ss->format(), ss->flags());
	AIL_set_sample_playback_rate(ailSampleHandle, ss->speed);
	AIL_set_sample_volume(ailSampleHandle, ss->getVolume());
	activate(washed());
	audioSet = 1;
}

void musicBuffer::format(soundSample *) {
}

/*******************************************************************/
/*                                                                 */
/* laden() determine whether a buffer can be written to.    */

bool Buffer::laden(void) {
	if (-1 == washed()) return TRUE;
	activate(0);
	return FALSE;
}

// fairly trivial for work buffers

bool workBuffer::laden(void) {
	if (-1 == washed())
		return TRUE;
	activate(0);
	return FALSE;
}

// sound buffers need to find out from AIL whether a buffer is free

bool doubleBuffer::laden(void) {
	VERIFY(ailSampleHandle);
	if (-1 == washed())
		return TRUE;
	else if (targetSated)
		activate(fillBuffer);
	return (FALSE);
}

bool singleBuffer::laden(void) {
	VERIFY(ailSampleHandle);
	if (targetSated)
		activate(0);
	return (FALSE);
}

bool musicBuffer::laden(void) {
	if (targetSated)
		activate(0);
	return (FALSE);
}

bool cacheBuffer::laden(void) {
	if (-1 == washed())
		return TRUE;
	activate(0);
	return FALSE;
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


uint32 doubleBuffer::sample_status(void) {
	VERIFY(ailSampleHandle);
#if 0
	int32 newPos = AIL_sample_position(ailSampleHandle);
	if (targetPos == 0 || newPos >= targetPos) { //( newPos==lastPos && lastPos==distPos)
		return (SMP_DONE);
	}
	distPos = lastPos;
	lastPos = newPos;
#endif
	return (AIL_sample_status(ailSampleHandle));
}

uint32 singleBuffer::sample_status(void) {
	VERIFY(ailSampleHandle);
	return (AIL_sample_status(ailSampleHandle));
}

uint32 musicBuffer::sample_status(void) {
	return (AIL_sequence_status(ailSampleHandle));
}

uint32 cacheBuffer::sample_status(void) {
	return (SMP_DONE);
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

int16 doubleBuffer::washed(void) {
	VERIFY(ailSampleHandle);
	if (AILLOCated > -1) return AILLOCated;
	AILLOCated = AIL_sample_buffer_ready(ailSampleHandle);
	return AILLOCated;
}

int16 singleBuffer::washed(void) {
	return 0;
}

int16 musicBuffer::washed(void) {
	return 0;
}

int16 cacheBuffer::washed(void) {
	return 0;
}


/*******************************************************************/
/*                                                                 */
/* gave() notifies a buffer that it has been written to            */
/* took() notifies a buffer that it has been read from             */
/*                                                                 */

void Buffer::gave(size_t dSize) {
	VERIFY(dSize <= wSize);
	wSize -= dSize;
	rSize += dSize;
	if (wSize)
		wData = (void *)(((char *) data[0]) + (size - wSize));

}

void workBuffer::gave(size_t dSize) {
	VERIFY(dSize <= wSize);
	wSize -= dSize;
	rSize += dSize;
	if (wSize) {
		wData = (void *)(((char *) data[fillBuffer]) + (size - wSize));
	}

}

// when sound buffers get full they automatically trigger AIL

void doubleBuffer::gave(size_t dSize) {
	VERIFY(ailSampleHandle);
	VERIFY(dSize <= wSize);
	wSize -= dSize;
	rSize += dSize;
	if (wSize) {
		wData = (void *)(((char *) data[fillBuffer]) + (size - wSize));
	} else {
		VERIFY(AILLOCated >= 0);
		play(AILLOCated);
		activate(1 - fillBuffer);
	}
}

void singleBuffer::gave(size_t dSize) {
	VERIFY(ailSampleHandle);
	VERIFY(dSize <= wSize);
	lastRSize = rSize;
	wSize -= dSize;
	rSize += dSize;
	if (wSize) {
		wData = (void *)(((char *) data[fillBuffer]) + (size - wSize));
	} else {
		play(0);
	}
}

void musicBuffer::gave(size_t dSize) {
	VERIFY(ailSampleHandle);
	VERIFY(dSize <= wSize);
	wSize -= dSize;
	rSize += dSize;
	if (wSize) {
		wData = (void *)(((char *) data[fillBuffer]) + (size - wSize));
	} else {
		//fill( 0 )
		//play( 0 );
	}
}

void cacheBuffer::gave(size_t dSize) {
	VERIFY(dSize <= wSize);
	wSize -= dSize;
	rSize += dSize;
	if (wSize) {
		wData = (void *)(((char *) data[0]) + (size - wSize));
	}

}

// when work buffers get fully drained they reset themselves

void Buffer::took(size_t dSize) {
	VERIFY(dSize <= rSize);
	rSize -= dSize;
	if (rSize > 0)
		rData = (void *)(((char *) rData) + dSize);
	if (rSize + wSize == 0)
		activate(0);
}

void workBuffer::took(size_t dSize) {
	VERIFY(dSize <= rSize);
	rSize -= dSize;
	if (rSize > 0) {
		rData = (void *)(((char *) rData) + dSize);
	}
	if (rSize + wSize == 0) {
		activate(0);
	}
}

void doubleBuffer::took(size_t dSize) {
	VERIFY(dSize <= rSize);
	rSize -= dSize;
	if (rSize > 0)
		rData = (void *)(((char *) rData) + dSize);
}

void singleBuffer::took(size_t dSize) {
	VERIFY(dSize <= rSize);
	lastRSize = rSize;
	rSize -= dSize;
	if (rSize > 0)
		rData = (void *)(((char *) rData) + dSize);
}

void musicBuffer::took(size_t dSize) {
	VERIFY(dSize <= rSize);
	rSize -= dSize;
	if (rSize > 0)
		rData = (void *)(((char *) rData) + dSize);
}

void cacheBuffer::took(size_t dSize) {
	VERIFY(dSize <= rSize);
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

void doubleBuffer::fill(void) {
	VERIFY(ailSampleHandle);
	VERIFY(AILLOCated >= 0);
	if (rSize) {
		play(AILLOCated);
		activate(1 - fillBuffer);
	} else {
		play(AILLOCated);
	}
}

void singleBuffer::fill(void) {
	VERIFY(ailSampleHandle);
	if (rSize) {
		play(0);
	}
}

void musicBuffer::fill(void) {
	VERIFY(ailSampleHandle);
	play(0);
}

void cacheBuffer::fill(void) {
	if (rSize) {
		hasData = 1;
	}
}

/*******************************************************************/
/*                                                                 */
/* When a buffer is no longer needed this call resets it.          */

void Buffer::abort(void) {
}

void Buffer::release(void) {
}

void workBuffer::abort(void) {
	if (rSize)
		took(rSize);
	release();
}

void workBuffer::release(void) {
}

void doubleBuffer::abort(void) {
	if (rSize)
		took(rSize);
	release();
	targetPos = 0;
}

void doubleBuffer::release(void) {
	VERIFY(ailSampleHandle);
	VERIFY(rSize == 0);

	if (washed() <= -1) {
		if (sample_status() != SMP_STOPPED)
			AIL_end_sample(ailSampleHandle);
	} else if (AILLOCated >= 0) {
		AIL_load_sample_buffer(ailSampleHandle, AILLOCated, rData, rSize);
		AILLOCated = -1;
	}

	AIL_end_sample(ailSampleHandle);
	audioSet = 0;
	audio->resetState((audioInterface::BufferRequest) ID());
	VERIFY(AILLOCated == -1);
}

void singleBuffer::abort(void) {
	AIL_end_sample(ailSampleHandle);
	if (rSize)
		took(rSize);
	release();
}

void singleBuffer::release(void) {
	VERIFY(ailSampleHandle);
	VERIFY(rSize == 0);
//	AIL_end_sample( ailSampleHandle );
	audioSet = 0;
	VERIFY(AILLOCated == -1);
}

void musicBuffer::abort(void) {
	if (rSize)
		took(rSize);
	release();
}

void musicBuffer::release(void) {
	VERIFY(ailSampleHandle);
// new
	AIL_end_sequence(ailSampleHandle);
	audioSet = 0;
}

void cacheBuffer::abort(void) {
}

void cacheBuffer::release(void) {
}

/*******************************************************************/
/*                                                                 */
/* Used internally - for sound buffers this is the call that       */
/*   causes data to be passed along to AIL                         */

void Buffer::play(int16) {
	took(rSize);
}

void workBuffer::play(int16 bufNo) {
	VERIFY(bufNo == 0);
	took(rSize);
}

void doubleBuffer::play(int16 bufNo) {
	VERIFY(bufNo >= 0 && bufNo <= 1);
	VERIFY(ailSampleHandle);
	targetPos = (int32)((char *) rData - (char *)data[AILLOCated]) + rSize;
	AIL_load_sample_buffer(ailSampleHandle, AILLOCated, rData, rSize);
	took(rSize);
	AILLOCated = -1;
}

void singleBuffer::play(int16 bufNo) {
	VERIFY(bufNo == 0);
	VERIFY(ailSampleHandle);
	AIL_set_sample_address(ailSampleHandle, rData, rSize);
	AIL_set_sample_loop_count(ailSampleHandle, loopCount);
	AIL_start_sample(ailSampleHandle);
	took(rSize);
}

void singleBuffer::replay(void) {
	VERIFY(ailSampleHandle);
	rSize = lastRSize; //((uint8 *)rData)-((uint8 *)data[0]) ;
	rData = data[0];
	VERIFY(rSize);
	AIL_set_sample_address(ailSampleHandle, rData, rSize);
	AIL_set_sample_loop_count(ailSampleHandle, loopCount);
	AIL_start_sample(ailSampleHandle);
}

void musicBuffer::play(int16 bufNo) {
	VERIFY(bufNo == 0);
	VERIFY(ailSampleHandle);
	if (AIL_init_sequence(ailSampleHandle, rData, 0) <= 0) {
		error("musicBuffer::play");
	}
	audioSet = 1;
	AIL_set_sequence_loop_count(ailSampleHandle, loopCount);
	AIL_start_sequence(ailSampleHandle);
	took(rSize);
}

void cacheBuffer::play(int16) {
	hasData = 1;
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
	VERIFY(bufNo == 0);
	VERIFY(rSize == 0);
	if (washed() > -1) {
		fillBuffer = 0;
		wSize = size;
		wData = data[fillBuffer];
		rSize = 0;
		rData = data[0];
	}
}

void doubleBuffer::activate(int16 bufNo) {
	int32 n;
	VERIFY(ailSampleHandle);
	n = bufNo;
	if (washed() > -1) {
		targetSated = FALSE;
		fillBuffer = AILLOCated;
		wSize = size;
		wData = data[fillBuffer];
		rSize = 0;
		rData = data[fillBuffer];
	} else {
		fillBuffer = AILLOCated;
		wSize = 0;
		wData = data[0];
		rSize = 0;
		rData = data[0];
		targetSated = TRUE;
	}
}

void singleBuffer::activate(int16 bufNo) {
	int32 n;
	VERIFY(ailSampleHandle);
	n = bufNo;
	targetSated = FALSE;
	fillBuffer = 0;
	wSize = size;
	wData = data[fillBuffer];
	rSize = 0;
	rData = data[fillBuffer];
}

void musicBuffer::activate(int16 bufNo) {
	int32 n;
	VERIFY(ailSampleHandle);
	n = bufNo;
	audioSet = 0;
	if (washed() > -1) {
		targetSated = FALSE;
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
		targetSated = TRUE;
	}
}

void cacheBuffer::activate(int16 bufNo) {
	VERIFY(bufNo == 0);
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
	VERIFY(rSize == 0);
	activate(0);
}

void doubleBuffer::reset(void) {
	VERIFY(AILLOCated == -1);
	AIL_init_sample(ailSampleHandle);
	audioSet = 0;
	targetPos = 0;
}

void singleBuffer::reset(void) {
//	VERIFY( AILLOCated==-1 );
	AIL_init_sample(ailSampleHandle);
	audioSet = 0;
}

void musicBuffer::reset(void) {
	AIL_end_sequence(ailSampleHandle);
	audioSet = 0;
	activate(0);
}

void cacheBuffer::reset(void) {
	VERIFY(rSize == 0);
	activate(0);
}

/*******************************************************************/
/* Initializes a buffer                                            */

void Buffer::setVolume(int8) {
}

void workBuffer::setVolume(int8) {
}

void doubleBuffer::setVolume(int8 v) {
	if (AIL_sample_volume(ailSampleHandle) != v) {
		AIL_lock();
		AIL_set_sample_volume(ailSampleHandle, v);
		AIL_unlock();
	}
}

void singleBuffer::setVolume(int8 v) {
	if (AIL_sample_volume(ailSampleHandle) != v) {
		AIL_lock();
		AIL_set_sample_volume(ailSampleHandle, v);
		AIL_unlock();
	}
}

void musicBuffer::setVolume(int8 v) {
	if (audioSet) {
		if (AIL_sequence_volume(ailSampleHandle) != v) {
			AIL_lock();
			AIL_set_sequence_volume(ailSampleHandle, v, 0);
			AIL_unlock();
		}
	}
}

void cacheBuffer::setVolume(int8) {
}

void musicBuffer::fadeUp(int16 time, int8 volume) {
	if (audioSet) {
		AIL_lock();
		AIL_set_sequence_volume(ailSampleHandle, volume, time);
		AIL_unlock();
	}
}

void musicBuffer::fadeDown(int16 time) {
	if (audioSet) {
		AIL_lock();
		AIL_set_sequence_volume(ailSampleHandle, 0, time);
		AIL_unlock();
	}
}

} // end of namespace Saga2
