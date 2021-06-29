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

#ifndef SAGA2_AUDIODEC_H
#define SAGA2_AUDIODEC_H

namespace Saga2 {

class Buffer;
class audioInterface;
class soundDecoder;
class soundSample;
class workBuffer;
class soundQueue;

/* ===================================================================== *

   Sound Sample Decoders
     These are audio buffer decoders & fillers used in playback.
     They are intended to be chained together.
     They come in three flavors:

     decodeInPlace - which operate on existing buffers
       mute() - zeroes out buffer

     decodeBuffer  - which require work buffers
       unShorten() - undoes Shorten 1.22 compression
       thunk8to16() - converts 8 bit samples to 16 bit

     decodeSource - aliases to soundSource calls

   NOTE : If you are trying to follow the flow of the code, and you get
     lost around here, you can probably pick things up again in the
     audiofnc modules or in some locally defined decoders

 * ===================================================================== */

enum soundDecoderType {
	decodeInPlace,
	decodeBuffer,
	decodeSource
};

typedef int16 INPLACEDECODER(Buffer &, soundDecoder *, soundSample &);
typedef int16 BUFFERDECODER(Buffer &, soundDecoder *, soundSample &, workBuffer *);
typedef int16 BUFFERLOADER(Buffer &, soundSample &);

union soundServer {
	INPLACEDECODER  *pDec;
	BUFFERDECODER   *bDec;
	BUFFERLOADER    *lDec;

	soundServer() {
		pDec = NULL;
	}
	soundServer(soundDecoderType sdt, void *fn) {
		switch (sdt)   {
		case decodeInPlace:
			pDec = (INPLACEDECODER *) fn;
			break;
		case decodeBuffer :
			bDec = (BUFFERDECODER *)  fn;
			break;
		case decodeSource :
			lDec = (BUFFERLOADER *)   fn;
			break;
		}
	}
	soundServer(const soundServer &src)         {
		pDec = src.pDec;
	}
	soundServer &operator=(const soundServer &src)     {
		this->pDec = src.pDec;
		return *this;
	}
	inline bool operator==(const soundServer &src2) const {
		return pDec == src2.pDec;
	}
	inline operator bool() const {
		return pDec != NULL;
	}
};

/*******************************************************************/
/* Decoder status flags                                            */

enum decoderStatuses {
	msgBufferFull   = 1 << 0,   // targetbuffer filled
	msgSegmentEnd   = 1 << 1,   // source segment exhausted
	msgBufferFree   = 1 << 2,   // waiting for a free buffer
	msgSamplePause  = 1 << 3,   // partial load - continue next time
	msgSampleDone   = 1 << 4,   // sample complete
	msgWaitForBuffer = 1 << 5,  // waiting for free buffer
	msgIOError      = 1 << 6    // an I/O error occurred reading the file
};

class soundDecoder {
	bool                inUse;      // to avoid buffer tangles
	soundDecoderType    type;       // in-place, buffered, loader
	soundServer         openf;      // open/seek function
	soundServer         readf;      // read/load function
	soundServer         closef;     // close function

public:
	soundDecoder        *next;      // linked list
	workBuffer          *wkBuffer;  // pointer to work buffer ( if any )

private:
	soundDecoder();
public:
	soundDecoder(INPLACEDECODER, INPLACEDECODER, INPLACEDECODER);
	soundDecoder(BUFFERDECODER, BUFFERDECODER, BUFFERDECODER, int16, audioInterface *, int16);
	soundDecoder(BUFFERLOADER, BUFFERLOADER, BUFFERLOADER);
	~soundDecoder(void);
	soundDecoder &operator=(const soundDecoder &src);
	soundDecoder(const soundDecoder &src);
	bool operator==(const soundDecoder &src2) const;
	inline operator bool() const {
		warning("STUB: soundDecoder::bool()");
		return true;
	}

	void setNext(soundDecoder *sd) {
		next = sd;
	}

	int16 use(Buffer &, soundDecoder *, soundSample &);
	int16 seek(Buffer &, soundDecoder *, soundSample &);
	int16 flush(Buffer &, soundDecoder *, soundSample &);

	void reset(void);

	//void notBusy( void );              // idle time function

};


/*******************************************************************/
/*                                                                 */
/* DecoderSet class : class used to process sound                  */
/*                                                                 */
/*******************************************************************/

class decoderSet { //: private DList
public:
	soundDecoder            *decode;

	decoderSet(void) {
		decode = NULL;
	}
	~decoderSet(void) {
		if (decode) delete decode;
		decode = NULL;
	}

	void addDecoder(soundDecoder *sodec);

#if DEBUG_AUDIO
	void check(void);
#else
	inline void check(void) {}
#endif

	void reset(void);
	void format(soundSample *ss);   // set various sound attribs.

	int16 openCall(Buffer *sobu, soundQueue *queue);
	int16 openCall(Buffer *sobu, soundSample *queue);

	int16 loadCall(Buffer *sobu, soundQueue *queue);
	int16 loadCall(Buffer *sobu, soundSample *queue);

	int16 closeCall(Buffer *sobu, soundQueue *queue);
	int16 closeCall(Buffer *sobu, soundSample *queue);

private:
	//void notBusy( void );              // idle time function
};

} // end of namespace Saga2

#endif
