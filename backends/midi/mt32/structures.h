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

#if !defined __MT32STRUCTURES_H__
#define __MT32STRUCTURES_H__

#include "stdafx.h"
#include "common/scummsys.h"

#if defined(_MSC_VER)
typedef unsigned __int64 uint64;
typedef   signed __int64 int64;
#else
typedef unsigned long long uint64;
typedef   signed long long int64;
#endif

#define INLINE


static inline void LOG_MSG(char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	fprintf(stdout, "\n");
	fflush(stdout);
}

#if defined(WIN32) && !(defined(__CYGWIN__) || defined(__MINGW32__))

#define ALIGN_PACKED

#else

//#define ALIGN_PACKED __attribute__ ((__packed__))
#define ALIGN_PACKED __attribute__ ((aligned (1)))

#ifdef HAVE_X86
#define eflag(value) __asm__ __volatile__("pushfl \n popfl \n" : : "a"(value))
#define cpuid_flag  (1 << 21)

static inline bool atti386_DetectCPUID()
{
	unsigned int result;
	
	/* is there a cpuid */
	result = cpuid_flag; /* set test */
	eflag(result);
	if (!(result & cpuid_flag))
		return false;
	
	result = 0; /* clear test */
	eflag(result);
	if (result & cpuid_flag)
		return false;
	
	return true;
}

static inline bool atti386_DetectSIMD()
{
	unsigned int result;
	
	if (atti386_DetectCPUID() == false)
		return false;
	
	/* check cpuid */
	__asm__ __volatile__(
			       "movl   $1, %%eax        \n" \
			       "cpuid		 	\n" \
                               "movl   %%edx, %0        \n" \
			     : "=r"(result) : : "eax", "ebx", "ecx", "edx");
		
	if (result & (1 << 25))
			return true;
	
	return false;
}

static inline bool atti386_Detect3DNow()
{
	unsigned int result;
	
	if (atti386_DetectCPUID() == false)
		return false;
		
	/* get cpuid */
	__asm__ __volatile__(
			       "movl   $0x80000001, %%eax \n" \
			       "cpuid		 	  \n" \
                               "movl   %%edx, %0          \n" \
			     : "=r"(result) : : "eax", "ebx", "ecx", "edx");
		
	if (result & 0x80000000)
			return true;
	
	return false;
}


static inline float atti386_iir_filter_sse(float *output, float *hist1_ptr, float *coef_ptr)
{
	
	__asm__  __volatile__ (
		"pushl %1                       \n" \
		"pushl %2                       \n" \
                "movss  0(%0), %%xmm1	        \n" \
                "movups 0(%1), %%xmm2	        \n" \
		"movlps 0(%2), %%xmm3	        \n" \
                "                               \n" \
		"shufps $0x44, %%xmm3, %%xmm3	\n" \
		"	       	       		\n" \
		"mulps %%xmm3, %%xmm2		\n" \
		"      	       			\n" \
		"subss  %%xmm2, %%xmm1		\n" \
		"shufps $0x39,  %%xmm2, %%xmm2	\n" \
		"subss  %%xmm2, %%xmm1		\n" \
		"				\n" \
		"movss  %%xmm1, 0(%2)	        \n" \
		"      	       			\n" \
		"shufps $0x39,  %%xmm2, %%xmm2	\n" \
		"addss  %%xmm2, %%xmm1		\n" \
		"				\n" \
		"shufps $0x39,  %%xmm2, %%xmm2	\n" \
		"addss  %%xmm2, %%xmm1		\n" \
		"				\n" \
		"movss  %%xmm3, 4(%2)	        \n" \
		"				\n" \
		"addl $16, %1			\n" \
		"addl $8, %2			\n" \
		"     	  			\n" \
		"movups 0(%1), %%xmm2	        \n" \
		"		  		\n" \
		"movlps 0(%2), %%xmm3	        \n" \
		"shufps $0x44, %%xmm3, %%xmm3	\n" \
		"	       	       		\n" \
		"mulps %%xmm3, %%xmm2		\n" \
		"      	       			\n" \
		"subss  %%xmm2, %%xmm1		\n" \
		"shufps $0x39,  %%xmm2, %%xmm2	\n" \
		"subss  %%xmm2, %%xmm1		\n" \
		"				\n" \
		"movss %%xmm1, 0(%2)		\n" \
		"      	       			\n" \
		"shufps $0x39, %%xmm2, %%xmm2	\n" \
		"addss %%xmm2, %%xmm1  		\n" \
		"      	       			\n" \
		"shufps $0x39, %%xmm2, %%xmm2	\n" \
		"addss %%xmm2, %%xmm1  		\n" \
		"      	       			\n" \
		"movss %%xmm3, 4(%2)		\n" \
		"movss %%xmm1, 0(%0)		\n" \
		"popl %2                        \n" \
		"popl %1                        \n" \
		: : "r"(output), "r"(coef_ptr), "r"(hist1_ptr)
		: "xmm1", "xmm2", "xmm3", "memory");
		
    return(*output);
}

static inline float atti386_iir_filter_3DNow(float output, float *hist1_ptr, float *coef_ptr)
{
	float tmp;
	
	__asm__  __volatile__ (
		"movq %0, %%mm1		 \n" \
		"	       		 \n" \
		"movl  %1, %%ebx	 \n" \
		"movq 0(%%ebx), %%mm2	 \n" \
		"     			 \n" \
		"movl %2, %%eax;	 \n" \
		"movq 0(%%eax), %%mm3	 \n" \
		"			 \n" \
		"pfmul %%mm3, %%mm2	 \n" \
		"pfsub %%mm2, %%mm1	 \n" \
		"	       		 \n" \
		"psrlq $32, %%mm2	 \n" \
		"pfsub %%mm2, %%mm1	 \n" \
		"      	      		 \n" \
		"movd %%mm1, %3		 \n" \
		"     	     		 \n" \
		"addl  $8, %%ebx	 \n" \
		"movq 0(%%ebx), %%mm2	 \n" \
		"movq 0(%%eax), %%mm3	 \n" \
		"     			 \n" \
		"pfmul %%mm3, %%mm2	 \n" \
		"pfadd %%mm2, %%mm1	 \n" \
		"      	      		 \n" \
		"psrlq $32, %%mm2	 \n" \
		"pfadd %%mm2, %%mm1	 \n" \
		"      	      		 \n" \
		"pushl %3		 \n" \
		"popl 0(%%eax)		 \n" \
		"     			 \n" \
		"movd %%mm3, 4(%%eax)	 \n" \
		"     	     		 \n" \
		"addl $8, %%ebx		 \n" \
		"addl $8, %%eax		 \n" \
		"     	  		 \n" \
		"movq 0(%%ebx), %%mm2	 \n" \
		"movq 0(%%eax), %%mm3	 \n" \
		"     			 \n" \
		"pfmul %%mm3, %%mm2	 \n" \
		"pfsub %%mm2, %%mm1	 \n" \
		"      	      		 \n" \
		"psrlq $32, %%mm2	 \n" \
		"pfsub %%mm2, %%mm1	 \n" \
		"      	      		 \n" \
		"movd %%mm1, %3		 \n" \
		"     	     		 \n" \
		"addl $8, %%ebx		 \n" \
		"movq 0(%%ebx), %%mm2	 \n" \
		"movq 0(%%eax), %%mm3	 \n" \
		"     			 \n" \
		"pfmul %%mm3, %%mm2	 \n" \
		"pfadd %%mm2, %%mm1	 \n" \
		"      	      		 \n" \
		"psrlq $32, %%mm2	 \n" \
		"pfadd %%mm2, %%mm1	 \n" \
		"      	      		 \n" \
		"pushl %3		 \n" \
		"popl 0(%%eax)		 \n" \
		"movd %%mm3, 4(%%eax)	 \n" \
		"     	     		 \n" \
		"movd %%mm1, %0		 \n" \
		"femms	     		 \n" \
                               : "=m"(output) : "g"(coef_ptr), "g"(hist1_ptr), "m"(tmp)
			       : "eax", "ebx", "mm1", "mm2", "mm3", "memory");
	
	return(output);
}

static inline float atti386_iir_filter_3dnow(float input,float *hist1_ptr, float *coef_ptr, int revLevel)
{
	return 0;
}

static inline void atti386_produceOutput1(int tmplen, int16 myvolume, int16 *useBuf, int16 *snd)
{
	__asm__ __volatile__(
				"movl %0,  %%ecx	\n" \
				"movw %1,  %%ax		\n" \
				"shll $16, %%eax	\n" \
				"movw %1,  %%ax		\n" \
				"movd %%eax, %%mm3	\n" \
				"movd %%eax, %%mm2	\n" \
				"psllq $32, %%mm3	\n" \
				"por %%mm2, %%mm3	\n" \
				"movl %2, %%esi		\n" \
				"movl %3, %%edi		\n" \
			        "1:   	  		\n" \
				"movq 0(%%esi), %%mm1	\n" \
				"movq 0(%%edi), %%mm2	\n" \
				"pmulhw %%mm3, %%mm1	\n" \
				"paddw %%mm2, %%mm1	\n" \
				"movq %%mm1, 0(%%edi)	\n" \
				" 			\n" \
				"addl $8, %%esi		\n" \
				"addl $8, %%edi		\n" \
				"     	  		\n" \
				"decl %%ecx		\n" \
				"cmpl $0, %%ecx		\n" \
				"jg   1b  		\n" \
				"emms		 	\n" \
			     : : "g"(tmplen), "g"(myvolume), "g"(useBuf), "g"(snd)
			     : "eax", "ecx", "edi", "esi", "mm1", "mm2", "mm3", "memory");
}

// FIXME: This is buggy
static inline void atti386_produceOutput2(uint32 len, int16 *snd, float *sndbufl, float *sndbufr, float *multFactor)
{
	__asm__ __volatile__(
			        "movl  %4, %%ecx		\n" \
				"shrl  $1, %%ecx		\n" \
				"addl  $4, %%ecx		\n" \
				"pushl %%ecx			\n" \
				"     				\n" \
				"movl %0, %%esi			\n" \
				"movups 0(%%esi), %%xmm1	\n" \
				" 		  		\n" \
				"movl %1, %%esi			\n" \
				"movl %2, %%edi			\n" \
				"1:   	  			\n" \
				"xorl %%eax, %%eax		\n" \
				"movw 0(%1), %%ax		\n" \
				"cwde 	    			\n" \
				"incl %1			\n" \
				"incl %1			\n" \
				"movd  %%eax, %%mm1		\n" \
				"psrlq $32, %%mm1		\n" \
				"movw 0(%1), %%ax		\n" \
				"incl %1    			\n" \
				"incl %1			\n" \
				"movd %%eax, %%mm2		\n" \
				"por %%mm2, %%mm1		\n" \
				"    	    			\n" \
				"decl %%ecx			\n" \
				"jnz 1b				\n" \
				"    				\n" \
				"popl %%ecx			\n" \
				"movl %1, %%esi			\n" \
				"movl %3, %%edi			\n" \
				"incl %%esi			\n" \
				"2:   				\n" \
				"decl %%ecx			\n" \
				"jnz 2b				\n" \
			     : : "g"(multFactor), "r"(snd), "g"(sndbufl), "g"(sndbufr), "g"(len)
			     : "eax", "ecx", "edi", "esi", "mm1", "mm2", "xmm1", "memory");
}

static inline void atti386_mixBuffers(int16 * buf1, int16 *buf2, int len)
{
	__asm__ __volatile__(
			     "movl %0, %%ecx       \n" \
			     "movl %1, %%esi       \n" \
			     "movl %2, %%edi       \n" \
			     "1:                   \n" \
			     "movq 0(%%edi), %%mm1 \n" \
			     "movq 0(%%esi), %%mm2 \n" \
			     "paddw %%mm2, %%mm1   \n" \
			     "movq %%mm1, 0(%%esi) \n" \
			     "addl $8, %%edi       \n" \
			     "addl $8, %%esi       \n" \
			     "decl %%ecx           \n" \
			     "cmpl $0, %%ecx       \n" \
			     "jg   1b              \n" \
			     "emms                 \n" \
			     : : "g"(len), "g"(buf1), "g"(buf2)
			     : "ecx", "edi", "esi", "mm1", "mm2", "memory");
}

static inline void atti386_mixBuffersRingMix(int16 * buf1, int16 *buf2, int len)
{		
	__asm__ __volatile__(
			     "movl %0, %%ecx       \n" \
			     "movl %1, %%esi       \n" \
			     "movl %2, %%edi       \n" \
			     "1:                   \n" \
			     "movq 0(%%esi), %%mm1 \n" \
			     "movq 0(%%edi), %%mm2 \n" \
			     "movq %%mm1, %%mm3    \n" \
			     "pmulhw %%mm2, %%mm1  \n" \
			     "paddw %%mm3, %%mm1   \n" \
			     "movq %%mm1, 0(%%esi) \n" \
			     "addl $8, %%edi       \n" \
			     "addl $8, %%esi       \n" \
			     "decl %%ecx           \n" \
			     "cmpl $0, %%ecx       \n" \
			     "jg   1b              \n" \
			     "emms                 \n" \
			     : : "g"(len), "g"(buf1), "g"(buf2)
			     : "ecx", "edi", "esi", "mm1", "mm2", "mm3", "memory");	
}

static inline void atti386_mixBuffersRing(int16 * buf1, int16 *buf2, int len)
{
	__asm__ __volatile__(
			     "movl %0, %%ecx       \n" \
			     "movl %1, %%esi       \n" \
			     "movl %2, %%edi       \n" \
			     "1:                   \n" \
			     "movq 0(%%esi), %%mm1 \n" \
			     "movq 0(%%edi), %%mm2 \n" \
			     "pmulhw %%mm2, %%mm1  \n" \
			     "movq %%mm1, 0(%%esi) \n" \
			     "addl $8, %%edi       \n" \
			     "addl $8, %%esi       \n" \
			     "decl %%ecx           \n" \
			     "cmpl $0, %%ecx       \n" \
			     "jg   1b              \n" \
			     "emms                 \n" \
			     : : "g"(len), "g"(buf1), "g"(buf2)
			     : "ecx", "edi", "esi", "mm1", "mm2", "memory");
}

static inline void atti386_PartProductOutput(int quadlen, int16 leftvol, int16 rightvol,
					     int16 *partialBuf, int16 *p1buf)
{
	__asm__ __volatile__(
			     "movl %0, %%ecx       \n"  \
			     "movw %1, %%ax        \n"  \
			     "shll $16, %%eax      \n"  \
			     "movw %2, %%ax        \n"  \
			     "movd %%eax, %%mm1    \n"  \
			     "movd %%eax, %%mm2    \n"  \
			     "psllq $32, %%mm1     \n"  \
			     "por  %%mm2, %%mm1    \n"  \
			     "movl %3, %%edi       \n"  \
			     "movl %4, %%esi       \n"  \
			     "1:                   \n"  \
			     "movw 0(%%esi), %%bx  \n"  \
			     "addl $2, %%esi       \n"  \
			     "movw 0(%%esi), %%dx  \n"  \
			     "addl $2, %%esi       \n"  \
			     ""                         \
			     "movw %%dx, %%ax      \n"  \
			     "shll $16, %%eax      \n"  \
			     "movw %%dx, %%ax      \n"  \
			     "movd %%eax, %%mm2    \n"  \
			     "psllq $32, %%mm2     \n"  \
			     "movw %%bx, %%ax      \n"  \
			     "shll $16, %%eax      \n"  \
			     "movw %%bx, %%ax      \n"  \
			     "movd %%eax, %%mm3    \n"  \
			     "por  %%mm3, %%mm2    \n"  \
			     ""    	     	      	\
			     "pmulhw %%mm1, %%mm2  \n"  \
			     "movq %%mm2, 0(%%edi) \n"  \
			     "addl $8, %%edi	   \n"  \
			     ""    	  	      	\
			     "decl %%ecx	   \n"  \
			     "cmpl $0, %%ecx	   \n"  \
			     "jg 1b	  	   \n"  \
			     "emms                 \n"  \
			     :  :"g"(quadlen), "g"(leftvol), "g"(rightvol), "g"(partialBuf), "g"(p1buf)
			     : "eax", "ebx", "ecx", "edx", "edi", "esi", "mm1", "mm2", "mm3", "memory");	
}
#endif

#endif

extern bool enabled3DNow;
extern bool enabledSSE;

#pragma pack(1)
struct timbreParam {
	struct commonParam {
		char name[10];
		char pstruct12;  // 1&2  0-12 (1-13)
		char pstruct34;  // #3&4  0-12 (1-13)
		char pmute;  // 0-15 (0000-1111)
		char nosustain; // 0-1(Normal, No sustain)
	} ALIGN_PACKED common;

	struct partialParam {
		struct wgParam {
			char coarse;  // 0-96 (C1,C#1-C9)
			char fine;  // 0-100 (-50 - +50)
			char keyfollow;  // 0-16 (-1,-1/2,0,1,1/8,1/4,3/8,1/2,5/8,3/4,7/8,1,5/4,3/2,2.s1,s2)
			char bender;  // 0,1 (ON/OFF)
			char waveform; //  0-1 (SQU/SAW)
			char pcmwave; // 0-127 (1-128)
			char pulsewid; // 0-100
			char pwvelo; // 0-14 (-7 - +7)
		} ALIGN_PACKED wg;

		struct envParam {
			char depth; // 0-10
			char sensitivity; // 1-100
			char timekeyfollow; // 0-4
			char time[4]; // 1-100
			char level[5]; // 1-100 (-50 - +50)
		} ALIGN_PACKED env;
		
		struct lfoParam {
			char rate; // 0-100
			char depth; // 0-100
			char modsense; // 0-100
		} ALIGN_PACKED lfo;

		struct tvfParam {
			char cutoff; // 0-100
			char resonance; // 0-30
			char keyfollow; // 0-16 (-1,-1/2,1/4,0,1,1/8,1/4,3/8,1/2,5/8,3/2,7/8,1,5/4,3/2,2,s1,s2)
			char biaspoint; // 0-127 (<1A-<7C >1A-7C)
			char biaslevel; // 0-14 (-7 - +7)
			char envdepth; // 0-100
			char envsense; // 0-100
			char envdkf; // DEPTH KEY FOLL0W 0-4
			char envtkf; // TIME KEY FOLLOW 0-4
			char envtime[5]; // 1-100
			char envlevel[4]; // 1-100
		} ALIGN_PACKED tvf;
		
		struct tvaParam {
			char level; // 0-100
			char velosens; // 0-100
			char biaspoint1; // 0-127 (<1A-<7C >1A-7C)
			char biaslevel1; // 0-12 (-12 - 0)
			char biaspoint2; // 0-127 (<1A-<7C >1A-7C)
			char biaslevel2; // 0-12 (-12 - 0)
			char envtkf; // TIME KEY FOLLOW 0-4
			char envvkf; // VELOS KEY FOLL0W 0-4
			char envtime[5]; // 1-100
			char envlevel[4]; // 1-100
		} ALIGN_PACKED tva;
			
	} ALIGN_PACKED partial[4];
	//char dummy[20];
} ALIGN_PACKED;

struct memParams {
	struct patchTemp {
		char timbreGroup; // TIMBRE GROUP  0-3 (group A, group B, Memory, Rhythm)
		char timbreNum; // TIMBRE NUMBER 0-63
		char keyShift; // KEY SHIFT 0-48 (-24 - +24)
		char fineTune; // FINE TUNE 0-100 (-50 - +50)
		char benderRange; // BENDER RANGE 0-24
		char assignMode;  // ASSIGN MODE 0-3 (POLY1, POLY2, POLY3, POLY4)
		char reverbSwitch;  // REVERB SWITCH 0-1 (OFF,ON)
		char dummy; // (DUMMY)
		char outlevel; // OUTPUT LEVEL 0-100
		char panpot; // PANPOT 0-14 (R-L)
		char dummyv[6];
	} ALIGN_PACKED tmpSettings[8];
	struct ryhTemp {
		char timbre; // TIMBRE  0-94 (M1-M64,R1-30,OFF)
		char outlevel; // OUTPUT LEVEL 0-100
		char panpot; // PANPOT 0-14 (R-L)
		char reverbSwitch;  // REVERB SWITCH 0-1 (OFF,ON)
	} ALIGN_PACKED rhySettings[64];

	timbreParam timTemp[8];

	struct patchArea {
		char timbreGroup; // TIMBRE GROUP  0-3 (group A, group B, Memory, Rhythm)
		char timbreNum; // TIMBRE NUMBER 0-63
		char keyShift; // KEY SHIFT 0-48 (-24 - +24)
		char fineTune; // FINE TUNE 0-100 (-50 - +50)
		char benderRange; // BENDER RANGE 0-24
		char assignMode;  // ASSIGN MODE 0-3 (POLY1, POLY2, POLY3, POLY4)
		char reverbSwitch;  // REVERB SWITCH 0-1 (OFF,ON)
		char dummy; // (DUMMY)
	} ALIGN_PACKED pSettings[128];
	timbreParam patch[192];
	struct systemArea {
		char masterTune; // MASTER TUNE 0-127 432.1-457.6Hz
		char reverbMode; // REVERB MODE 0-3 (room, hall, plate, tap delay)
		char reverbTime; // REVERB TIME 0-7 (1-8)
		char reverbLevel; // REVERB LEVEL 0-7 (1-8)
		char reserveSettings[9]; // PARTIAL RESERVE (PART 1) 0-32
		char chanAssign[9]; // MIDI CHANNEL (PART1) 0-16 (1-16,OFF)
		char masterVol; // MASTER VOLUME 0-100
	} ALIGN_PACKED system;

} ALIGN_PACKED;

struct memBanks {
	char pTemp[8][sizeof(memParams::patchTemp)];
	char rTemp[64][sizeof(memParams::ryhTemp)];
	char tTemp[8][sizeof(timbreParam)];
	char patchmemory[128][sizeof(memParams::patchArea)];
	char patchbanks[128][sizeof(timbreParam)];
	char timbrebanks[64][sizeof(timbreParam)];
	char systemBank[sizeof(memParams::systemArea)];
} ALIGN_PACKED;

struct memAbsolute {
	char mt32memory[sizeof(memBanks)];
} ALIGN_PACKED;

#pragma pack()

struct partialFormat {
	uint32 addr;
	uint16 len;
	bool loop;
	float tune;
	int32 ampval;
};

struct partialTable {
	uint32 addr;
	uint32 len;
	uint32 pcmnum;
	int32 ampval;
	bool loop;
	int32 aggSound; // This variable is for the last 9 PCM samples, which are actually loop combinations
};



union soundaddr {
	uint32 pcmabs;
	struct offsets {
#if defined(SCUMM_LITTLE_ENDIAN)
		uint16 pcmoffset;
		uint16 pcmplace;
#else
		uint16 pcmplace;
		uint16 pcmoffset;
#endif
	} pcmoffs;
};


struct volset {
	int16 leftvol;
	int16 rightvol;
	int16 leftvol2;
	int16 rightvol2;
};

struct patchCache {
	int rawPCM;
	partialTable convPCM;

	bool playPartial;
	bool usePartial;
	bool PCMPartial;
	char waveform;
	int pulsewidth;
	int pwsens;
	int pitchshift;
	int fineshift;
	bool sustain;

	int lfodepth;
	int lforate;
	uint32 lfoperiod;
	int modsense;

	int keydir;
	int pitchkeyfollow;
	int pitchkeydir;

	int filtkeyfollow;

	int tvfbias;
	int tvfblevel;
	int tvfdir;

	int ampbias[2];
	int ampblevel[2];
	int ampdir[2];

	int ampdepth;
	int ampenvdir;
	int amplevel;
	int tvfdepth;

	int prevsample;

	bool useBender;

	timbreParam::partialParam::envParam pitchEnv;
	timbreParam::partialParam::tvaParam ampEnv;
	timbreParam::partialParam::tvfParam filtEnv;

	int32 ampsustain;
	int32 pitchsustain;
	int32 filtsustain;

	uint32 partCount;

	uint8 padding[64]; //Used to pad the patch cache to 4096 bytes.  This replaces an imul with a shl 12

};

struct dpoly {
	bool isPlaying;
	bool isDecay;
	bool isActive;

	bool partActive[4];

	bool isRy;
	uint32 *bendptr;
	uint32 drumbend;
	int32 *volumeptr;
	volset *pansetptr;

	int pcmnum;
	int freq;
	int freqnum;
	int vel;

	uint32 partCount;

	soundaddr pcmoff;
	uint32 pcmdelta;


	struct partialStatus {
		// Note played on keyboard
		int noteval;
		// Keyfollowed note values
		int keyedval;

		// Keyfollowed filter values
		int realval;
		int filtval;
		// Keyfollowed filter w/o table
		int filtnoval;
		int pulsewidth;

		struct envstatus {
			int32 envpos;
			int32 envstat;
			int32 envbase;
			int32 envdist;
			int32 envsize;

			bool sustaining;
			bool decaying;
			bool notdecayed;
			uint32 decay;
			int32 prevlevel;

			int32 counter;
			int32 count;

		} envs[4];

		uint32 lfopos;
		soundaddr partialOff;
		soundaddr wgOff;

		uint32 ampEnvCache;
		uint32 pitchEnvCache;

		bool isDecayed;
		bool PCMDone;

		float history[32];

		float pastfilt;
		bool pitchsustain;
		bool playPartial;
		bool usePartial;

		int looppos;
		int partNum;

		patchCache *tcache;

		void * myPart;



	} pStatus[4];


	int chan;

	int origpat;
	int drumnum;
	
	int age;

	bool pedalhold;
	bool firstsamp;

	uint32 P1Mix;
	uint32 P2Mix;
	bool sustain;
};

#endif
