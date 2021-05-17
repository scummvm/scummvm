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

#ifndef SAGA2_AUDQUEUE_H
#define SAGA2_AUDQUEUE_H

namespace Saga2 {

class soundQueue {
public:
	positionedSample            *tip;
	soundSegment            curSeg;
	soundSegment            headSeg;
private:
	PublicPtrQueue<positionedSample>    sampleList;

public:
	soundQueue(void) {
		tip = NULL;
	}
	~soundQueue(void);
	void *operator new (size_t s) {
		return audioAlloc(s, "aud smpl queue");
	}
	void operator delete (void *m) {
		audioFree(m);
	}

	positionedSample *firstSample(void);
	positionedSample *nextSample(void);

	soundSegment firstSegment(void);
	soundSegment nextSegment(void);

	void pushSample(positionedSample *sam, decoderSet *);
	bool findSample(soundSegment s);

	int16 getSize(void);
	int16 getSampleSize(void);
};

} // end of namespace Saga2

#endif
