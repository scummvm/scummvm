#include <stdlib.h>

#include "native.h"
#include "globals.h"

#ifdef COMPILE_PA1SND
#	include <endianutils.h>
#endif


static Int32 diffLookup[16] = {
   1,3,5,7,9,11,13,15,
   -1,-3,-5,-7,-9,-11,-13,-15,
};

static Int32 indexScale[16] = {
   0x0e6, 0x0e6, 0x0e6, 0x0e6, 0x133, 0x199, 0x200, 0x266,
   0x0e6, 0x0e6, 0x0e6, 0x0e6, 0x133, 0x199, 0x200, 0x266 /* same value for speedup */
};

static int limit(int val,int min,int max) {
   if (val<min) return min;
   else if (val>max) return max;
   else return val;
}

void pcm2adpcm(Int16 *src, UInt8 *dst, UInt32 length) {
#ifndef COMPILE_PA1SND
/*
	if (OPTIONS_TST(kOptDeviceARM)) {
		PnoDescriptor pno;
		ARMPa1SndType userData = {src, dst, length};

		MemPtr armP =	_PnoInit(ARM_PA1SND, &pno);
						_PnoCall(&pno, &userData);
						_PnoFree(&pno, armP);

		return;
	}
*/
	int data,val,diff;
	int signal,step;
#else
	long chan1, chan2;
	long data,val,diff;
	long signal,step;
#endif

	signal = 0;
	step = 0x7F;
	length >>= 3;	// 16bit stereo -> 4bit mono

	do {

		// high nibble
#ifdef COMPILE_PA1SND
		chan1 = ByteSwap16(*src);
		src++;
		chan2 = ByteSwap16(*src);
		src++;

		diff = ((chan1 + chan2) >> 1) - signal;
#else
		diff = ((*src++ + *src++) >> 1) - signal;
#endif
		diff <<= 3;
		diff /= step;

		val = abs(diff) >> 1;

		if (val  > 7)	val = 7;
		if (diff < 0)	val+= 8;

		signal+= (step * diffLookup[val]) >> 3;
		signal = limit(signal, -32768, 32767);

		step = (step * indexScale[val]) >> 8;
		step = limit(step, 0x7F, 0x6000);

		data = val;

		// low nibble
#ifdef COMPILE_PA1SND
		chan1 = ByteSwap16(*src);
		src++;
		chan2 = ByteSwap16(*src);
		src++;

		diff = ((chan1 + chan2) >> 1) - signal;
#else
		diff = ((*src++ + *src++) >> 1) - signal;
#endif
		diff <<= 3;
		diff /= step;

		val = abs(diff) >> 1;

		if (val  > 7)	val = 7;
		if (diff < 0)	val+= 8;

		signal+= (step * diffLookup[val]) >> 3;
		signal = limit(signal, -32768, 32767);

		step = (step * indexScale[val]) >> 8;
		step = limit(step, 0x7F, 0x6000);

		data |= val << 4;

		*dst++ = (UInt8)data;

	} while (--length);

}
