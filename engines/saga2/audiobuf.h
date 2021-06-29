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

#ifndef SAGA2_AUDIOBUF_H
#define SAGA2_AUDIOBUF_H

namespace Saga2 {

class Buffer;
class workBuffer;
class doubleBuffer;
class singleBuffer;
class musicBuffer;
class cacheBuffer;

class soundDecoder;
class decoderSet;

class soundSample;

class soundQueue;

class audioInterface;

/* ===================================================================== *
   Simple base buffer class
 * ===================================================================== */

enum soundBufferStatus {
	bufferNull = 0,
	bufferFree,        // buffer available for use
	bufferDone,        // buffer is done playing
	bufferLoad,        // buffer is being filled
	bufferFull,        // buffer is full & ready to go
	bufferPlay,        // buffer is playing
	bufferWork,        // indicates a permanent work buffer
	bufferUsed
};


class Buffer {
private:
	bool            internallyAllocated;         // buffer allocated in ctor

protected:
	uint32          size;                        // true size of buffer( s )
	void            *data[2];                    // pointers to buffer( s )

public:
	// These two fields comprise the virtual write buffer
	uint32          wSize;                       // virtual remaining write size
	void            *wData;                      // virtual write data buffer

	// These two fields comprise the virtual read buffer
	uint32          rSize;                       // virtual remaining read size
	void            *rData;                      // virtual read data buffer

	Buffer(size_t newSize);
	virtual ~Buffer(void);

	virtual int16 ID(void) {
		return -1;    // for buffer Manager
	}

	virtual uint32 sample_status(void);           // gives the status of the buffer as a whole
	virtual void reset(void);                    // initialize buffer
	virtual void format(soundSample *ss);        // set various sound attribs.
	virtual bool laden(void);                     // check for full targets

	virtual void gave(size_t dSize);              // notify amount written
	virtual void took(size_t dSize);             // notify amount read
	virtual void fill(void);                      // convince buffer it is full
	virtual void abortsound(void);                     // kill sound & return buffer to its heap
	virtual void release(void);               // return buffer to its heap
	virtual void setVolume(int8 val);         // internal : set buffer to fill & play

protected:
	virtual int16 washed(void);                  // internal : check for a clean buffer
	virtual void play(int16 bufNo = 0);          // internal : use the data
	virtual void activate(int16 bufNo);           // internal : set buffer to fill & play
};


class workBuffer : public Buffer {
private:
	int16           bufID;             // for buffer manager
	int16           fillBuffer,        // buffer being filled
	                targetSated;       // target full

public:
	int16 ID(void) {
		return bufID;    // for buffer Manager
	}
	void shiftdown(int16 bufNo = 0);

	workBuffer(size_t newSize, int16 newID);
	~workBuffer(void);

	uint32 sample_status(void);           // gives the status of the buffer as a whole
	void reset(void);                    // initialize buffer
	void format(soundSample *ss);        // set various sound attribs.
	bool laden(void);                     // check for full targets

	void gave(size_t dSize);              // notify amount written
	void took(size_t dSize);               // notify amount read
	void fill(void);                      // convince buffer it is full
	void abortsound(void);                    // kill sound & return buffer to its heap
	void release(void);               // return buffer to its heap
	void setVolume(int8 val);         // internal : set buffer to fill & play

protected:
	int16 washed(void);                  // check for a clean buffer
	void play(int16 bufNo = 0);          // internal: use the data
	void activate(int16 bufNo);           // set buffer to fill & play
};
class musicBuffer : public Buffer {
private:
	int16           bufID;             // for buffer manager
	//int16             AILLOCated;        // last buffer ( 0, 1 ) reported free
	int16           fillBuffer,        // buffer being filled
	                targetSated;       // target full
	int16           loopCount;


public:
	HSEQUENCE       ailSampleHandle;   // sample handle
	int16           audioSet;

public:
	int16 ID(void) {
		return bufID;    // for buffer Manager
	}

	musicBuffer(size_t newSize, audioInterface *sd, int16 newID);
	~musicBuffer(void);

	uint32 sample_status(void);           // gives the status of the buffer as a whole
	void reset(void);                    // initialize buffer
	void format(soundSample *ss);        // set various sound attribs.
	bool laden(void);                     // check for full targets

	void gave(size_t dSize);              // notify amount written
	void took(size_t dSize);               // notify amount read
	void fill(void);                      // convince buffer it is full
	void abortsound(void);                    // kill sound & return buffer to its heap
	void release(void);               // return buffer to its heap
	void setVolume(int8 val);         // internal : set buffer to fill & play

	void setLoopCount(int16 loops) {
		loopCount = loops;
	}
	int16 getLoopCount(void) {
		return loopCount;
	}

protected:
	int16 washed(void);                  // check for a clean buffer
	void play(int16 bufNo = 0);          // internal: use the data
	void activate(int16 bufNo);           // set buffer to fill & play

public:
	void fadeUp(int16 time, int8 volume);
	void fadeDown(int16 time);
};

} // end of namespace Saga2

#endif
