/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 * Based on Tristan's conversion of Canadacow's code
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef __CPARTIALMT32_H__
#define __CPARTIALMT32_H__

#include "backends/midi/mt32/structures.h"

// Class definition of MT-32 partials.  32 in all.
class CPartialMT32 {
private:
	int useMix;
	int partNum;

	int pN;




	Bit16s myBuffer[2048];
	// For temporary output of paired buffer
	Bit16s pairBuffer[2048];

	void mixBuffers(Bit16s * buf1, Bit16s * buf2, int len);
	void mixBuffersRingMix(Bit16s * buf1, Bit16s * buf2, int len);
	void mixBuffersRing(Bit16s * buf1, Bit16s * buf2, int len);
	void mixBuffersStereo(Bit16s * buf1, Bit16s * buf2, Bit16s * outBuf, int len);


public:
	patchCache *tcache;
	patchCache cachebackup[4];

	//FILE *fp;
	//FILE *fp2;

	dpoly::partialStatus *partCache;

	CPartialMT32 *tibrePair;
	bool isActive;
	bool alreadyOutputed;
	int ownerChan;
	Bit64s age;
	int timbreNum;
	dpoly *tmppoly;

	CPartialMT32(int partialNum) {

		isActive = false;		
		pN = partialNum;

		/*
		sprintf(buffer, "partial%d.raw",pN);
		fp = fopen(buffer,"wb");

		sprintf(buffer, "partial%dx.raw",pN);
		fp2 = fopen(buffer,"wb");
		*/

		
	};
	
	void startPartial(dpoly *usePoly, patchCache *useCache, dpoly::partialStatus * usePart, CPartialMT32 * pairPart, int mixType, int num, int ownChan, int timNum) {

		//LOG_MSG("Starting partial %d for %d", num, ownChan);
		tmppoly = usePoly;
		tcache = useCache;
		partCache = usePart;
		tibrePair = pairPart;
		isActive = true;
		useMix = mixType;
		partNum = num;
		age = 0;
		ownerChan = ownChan;
		alreadyOutputed = false;
		timbreNum = timNum;
		memset(usePart->history,0,sizeof(usePart->history));

	}

	void stopPartial(void) { isActive = false; }

	
	// Returns true only if data written to buffer
	// This function (unline the one below it) returns processed stereo samples
	// made from combining this single partial with its pair, if it has one.
	bool produceOutput(Bit16s * partialBuf, long length);

	// This function produces mono sample output of the specific partial
	void generateSamples(Bit16s * partialBuf, long length);

};


#endif

