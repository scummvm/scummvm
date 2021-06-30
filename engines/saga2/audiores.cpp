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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/audio.h"
#include "saga2/hresmgr.h"
#include "saga2/audiodec.h"
#include "saga2/rect.h"

#include "saga2/queues.h"
#include "saga2/idtypes.h"
#include "saga2/audiosmp.h"
#include "saga2/audiobuf.h"

namespace Saga2 {

#define ASYNCH_AUDIO 0
#define ASYNCH_READ_SIZE 65536L

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern audioInterface *audio;
extern bool debugStatuses;
extern bool debugResource;
extern uint32 maxClicks;
extern int32 clickSizes[];
extern uint8 *clickData[];


bool bufCheckResID(hResContext *hrc, uint32 s) {
	return s != 0;
}

bool hResCheckResID(hResContext *hrc, uint32 s) {
	if (hrc != NULL)
		return hrc->seek(s);
	return false;
}

bool hResCheckResID(hResContext *hrc, uint32 s[]) {
	if (s != NULL) {
		if (s[0] == 0)
			return false;

		for (int i = 0; s[i]; i++) {
			if (!hResCheckResID(hrc, s[i]))
				return false;
		}
	}
	return true;
}

/* ===================================================================== *
   HRes loader code
 * ===================================================================== */




//-----------------------------------------------------------------------
//	open / seek

int16 hResSeek(Buffer &sb, soundSample &ss, hResContext *hrc, bool Cheksize) {
	if (hrc->seek(ss.curSeg) == false) {
		warning("Audio: %d is an invalid res ID", ss.curSeg);
		return 1;
	}
	if (Cheksize && sb.wSize < hrc->bytesleft()) {
		int bufferSize = sb.wSize;
		int soundSize = hrc->bytesleft();

		warning("Buffer too small %d sample: %d", bufferSize, soundSize);
		return 1;
	}
	ss.channels = soundSample::channelMono;
	ss.speed = soundRate22K;
	ss.dataSize = soundSample::granularity16Bit;
	return 0;
}

//-----------------------------------------------------------------------
//	read

int16 hResRead(Buffer &sb, soundSample &ss, hResContext *hrc) {
	size_t bread;
	size_t count = MIN<uint>(sb.wSize, hrc->bytesleft());
	int16 rVal = 0;

#if ASYNCH_AUDIO
	bool partial = false;
	count = min(count, ASYNCH_READ_SIZE);
	if (count > ASYNCH_READ_SIZE) {
		count = ASYNCH_READ_SIZE;
		partial = true;
	}
#endif

	bread = hrc->readbytes(sb.wData, count);
	sb.gave(bread);

	assert(bread <= count);

#if ASYNCH_AUDIO
	if (partial)
		rVal |= msgSamplePause;
#endif
	if (bread == count)
		rVal |= msgBufferFull;
	if (hrc->eor())
		rVal |= msgSegmentEnd;
	return rVal;
}

//-----------------------------------------------------------------------
//	flush / close

int16 hResFlush(Buffer &sb, soundSample &ss, hResContext *hrc) {
	return 0;
}

/* ===================================================================== *
   In memory loader code
 * ===================================================================== */

//-----------------------------------------------------------------------
//	open / seek

int16 bufSeek(Buffer &sb, soundSample &ss) {
	if (ss.curSeg >= maxClicks) {
		warning("bufSeek: open failed");
		return 1;
	}
	ss.channels = soundSample::channelMono;
	ss.speed = soundRate22K;
	ss.dataSize = soundSample::granularity16Bit;
	return 0;
}

//-----------------------------------------------------------------------
//	read

int16 bufRead(Buffer &sb, soundSample &ss) {
	size_t bread;
	size_t count = MIN<uint>(sb.wSize, clickSizes[ss.curSeg]);
	int16 rVal = 0;

	bread = count;
	memcpy(sb.wData, clickData[ss.curSeg], count);
	sb.gave(bread);

	assert(bread <= count);

	if (bread == count)
		rVal |= msgBufferFull;
	rVal |= msgSegmentEnd;
	return rVal;
}

//-----------------------------------------------------------------------
//	flush / close

int16 bufFlush(Buffer &sb, soundSample &ss) {
	return 0;
}

} // end of namespace Saga2
