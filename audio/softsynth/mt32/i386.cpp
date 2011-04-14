/* Copyright (c) 2003-2005 Various contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "mt32emu.h"

#ifdef MT32EMU_HAVE_X86

namespace MT32Emu {

#ifndef _MSC_VER

#define eflag(value) __asm__ __volatile__("pushfl \n popfl \n" : : "a"(value))
#define cpuid_flag  (1 << 21)

static inline bool atti386_DetectCPUID() {
	unsigned int result;

	// Is there a cpuid?
	result = cpuid_flag; // set test
	eflag(result);
	if (!(result & cpuid_flag))
		return false;

	result = 0; // clear test
	eflag(result);
	if (result & cpuid_flag)
		return false;

	return true;
}

static inline bool atti386_DetectSIMD() {
	unsigned int result;

	if (atti386_DetectCPUID() == false)
		return false;

	/* check cpuid */
	__asm__ __volatile__(
		"pushl  %%ebx            \n" \
		"movl   $1, %%eax        \n" \
		"cpuid                   \n" \
		"movl   %%edx, %0        \n" \
		"popl   %%ebx            \n" \
		: "=r"(result) : : "eax", "ecx", "edx");

	if (result & (1 << 25))
		return true;

	return false;
}

static inline bool atti386_Detect3DNow() {
	unsigned int result;

	if (atti386_DetectCPUID() == false)
		return false;

	// get cpuid
	__asm__ __volatile__(
		"pushl  %%ebx            \n" \
		"movl   $0x80000001, %%eax \n" \
		"cpuid                     \n" \
		"movl   %%edx, %0          \n" \
		"popl   %%ebx            \n" \
		: "=r"(result) : : "eax", "ecx", "edx");

	if (result & 0x80000000)
		return true;

	return false;
}


static inline float atti386_iir_filter_sse(float *output, float *hist1_ptr, float *coef_ptr) {
	__asm__  __volatile__ (
		"pushl  %1                      \n" \
		"pushl  %2                      \n" \
		"movss  0(%0), %%xmm1           \n" \
		"movups 0(%1), %%xmm2           \n" \
		"movlps 0(%2), %%xmm3           \n" \
		"                               \n" \
		"shufps $0x44, %%xmm3, %%xmm3   \n" \
		"                               \n" \
		"mulps  %%xmm3, %%xmm2          \n" \
		"                               \n" \
		"subss  %%xmm2, %%xmm1          \n" \
		"shufps $0x39,  %%xmm2, %%xmm2  \n" \
		"subss  %%xmm2, %%xmm1          \n" \
		"                               \n" \
		"movss  %%xmm1, 0(%2)           \n" \
		"                               \n" \
		"shufps $0x39,  %%xmm2, %%xmm2  \n" \
		"addss  %%xmm2, %%xmm1          \n" \
		"                               \n" \
		"shufps $0x39,  %%xmm2, %%xmm2  \n" \
		"addss  %%xmm2, %%xmm1          \n" \
		"                               \n" \
		"movss  %%xmm3, 4(%2)           \n" \
		"                               \n" \
		"addl   $16, %1                 \n" \
		"addl   $8, %2                  \n" \
		"                               \n" \
		"movups 0(%1), %%xmm2           \n" \
		"                               \n" \
		"movlps 0(%2), %%xmm3           \n" \
		"shufps $0x44, %%xmm3, %%xmm3   \n" \
		"                               \n" \
		"mulps %%xmm3, %%xmm2           \n" \
		"                               \n" \
		"subss  %%xmm2, %%xmm1          \n" \
		"shufps $0x39,  %%xmm2, %%xmm2  \n" \
		"subss  %%xmm2, %%xmm1          \n" \
		"                               \n" \
		"movss %%xmm1, 0(%2)            \n" \
		"                               \n" \
		"shufps $0x39, %%xmm2, %%xmm2   \n" \
		"addss %%xmm2, %%xmm1           \n" \
		"                               \n" \
		"shufps $0x39, %%xmm2, %%xmm2   \n" \
		"addss %%xmm2, %%xmm1           \n" \
		"                               \n" \
		"movss %%xmm3, 4(%2)            \n" \
		"movss %%xmm1, 0(%0)            \n" \
		"popl %2                        \n" \
		"popl %1                        \n" \
		: : "r"(output), "r"(coef_ptr), "r"(hist1_ptr)
		: "memory"
#ifdef __SSE__
		, "xmm1", "xmm2", "xmm3"
#endif
		);

	return *output;
}

static inline float atti386_iir_filter_3DNow(float output, float *hist1_ptr, float *coef_ptr) {
	float tmp;

	__asm__ __volatile__ (
		"movq %0, %%mm1       \n" \
		"                     \n" \
		"movl  %1, %%edi      \n" \
		"movq 0(%%edi), %%mm2 \n" \
		"                     \n" \
		"movl %2, %%eax;      \n" \
		"movq 0(%%eax), %%mm3 \n" \
		"                     \n" \
		"pfmul %%mm3, %%mm2   \n" \
		"pfsub %%mm2, %%mm1   \n" \
		"                     \n" \
		"psrlq $32, %%mm2     \n" \
		"pfsub %%mm2, %%mm1   \n" \
		"                     \n" \
		"movd %%mm1, %3       \n" \
		"                     \n" \
		"addl  $8, %%edi      \n" \
		"movq 0(%%edi), %%mm2 \n" \
		"movq 0(%%eax), %%mm3 \n" \
		"                     \n" \
		"pfmul %%mm3, %%mm2   \n" \
		"pfadd %%mm2, %%mm1   \n" \
		"                     \n" \
		"psrlq $32, %%mm2     \n" \
		"pfadd %%mm2, %%mm1   \n" \
		"                     \n" \
		"pushl %3             \n" \
		"popl 0(%%eax)        \n" \
		"                     \n" \
		"movd %%mm3, 4(%%eax) \n" \
		"                     \n" \
		"addl $8, %%edi       \n" \
		"addl $8, %%eax       \n" \
		"                     \n" \
		"movq 0(%%edi), %%mm2 \n" \
		"movq 0(%%eax), %%mm3 \n" \
		"                     \n" \
		"pfmul %%mm3, %%mm2   \n" \
		"pfsub %%mm2, %%mm1   \n" \
		"                     \n" \
		"psrlq $32, %%mm2     \n" \
		"pfsub %%mm2, %%mm1   \n" \
		"                     \n" \
		"movd %%mm1, %3       \n" \
		"                     \n" \
		"addl $8, %%edi       \n" \
		"movq 0(%%edi), %%mm2 \n" \
		"movq 0(%%eax), %%mm3 \n" \
		"                     \n" \
		"pfmul %%mm3, %%mm2   \n" \
		"pfadd %%mm2, %%mm1   \n" \
		"                     \n" \
		"psrlq $32, %%mm2     \n" \
		"pfadd %%mm2, %%mm1   \n" \
		"                     \n" \
		"pushl %3             \n" \
		"popl 0(%%eax)        \n" \
		"movd %%mm3, 4(%%eax) \n" \
		"                     \n" \
		"movd %%mm1, %0       \n" \
		"femms                \n" \
		: "=m"(output) : "g"(coef_ptr), "g"(hist1_ptr), "m"(tmp)
		: "eax", "edi", "memory"
#ifdef __MMX__
		, "mm1", "mm2", "mm3"
#endif
		);

	return output;
}

static inline void atti386_produceOutput1(int tmplen, Bit16s myvolume, Bit16s *useBuf, Bit16s *snd) {
	__asm__ __volatile__(
		"movl %0,  %%ecx      \n" \
		"movw %1,  %%ax       \n" \
		"shll $16, %%eax      \n" \
		"movw %1,  %%ax       \n" \
		"movd %%eax, %%mm3    \n" \
		"movd %%eax, %%mm2    \n" \
		"psllq $32, %%mm3     \n" \
		"por %%mm2, %%mm3     \n" \
		"movl %2, %%esi       \n" \
		"movl %3, %%edi       \n" \
		"1:                   \n" \
		"movq 0(%%esi), %%mm1 \n" \
		"movq 0(%%edi), %%mm2 \n" \
		"pmulhw %%mm3, %%mm1  \n" \
		"paddw %%mm2, %%mm1   \n" \
		"movq %%mm1, 0(%%edi) \n" \
		"                     \n" \
		"addl $8, %%esi       \n" \
		"addl $8, %%edi       \n" \
		"                     \n" \
		"decl %%ecx           \n" \
		"cmpl $0, %%ecx       \n" \
		"jg   1b              \n" \
		"emms                 \n" \
		: : "g"(tmplen), "g"(myvolume), "g"(useBuf), "g"(snd)
		: "eax", "ecx", "edi", "esi", "memory"
#ifdef __MMX__
		, "mm1", "mm2", "mm3"
#endif
		);
}

static inline void atti386_produceOutput2(Bit32u len, Bit16s *snd, float *sndbufl, float *sndbufr, float *multFactor) {
	__asm__ __volatile__(
		"movl  %4, %%ecx         \n" \
		"shrl  $1, %%ecx         \n" \
		"addl  $4, %%ecx         \n" \
		"pushl %%ecx             \n" \
		"                        \n" \
		"movl %0, %%esi          \n" \
		"movups 0(%%esi), %%xmm1 \n" \
		"                        \n" \
		"movl %1, %%esi          \n" \
		"movl %2, %%edi          \n" \
		"1:                      \n" \
		"xorl %%eax, %%eax       \n" \
		"movw 0(%1), %%ax        \n" \
		"cwde                    \n" \
		"incl %1                 \n" \
		"incl %1                 \n" \
		"movd  %%eax, %%mm1      \n" \
		"psrlq $32, %%mm1        \n" \
		"movw 0(%1), %%ax        \n" \
		"incl %1                 \n" \
		"incl %1                 \n" \
		"movd %%eax, %%mm2       \n" \
		"por %%mm2, %%mm1        \n" \
		"                        \n" \
		"decl %%ecx              \n" \
		"jnz 1b                  \n" \
		"                        \n" \
		"popl %%ecx              \n" \
		"movl %1, %%esi          \n" \
		"movl %3, %%edi          \n" \
		"incl %%esi              \n" \
		"2:                      \n" \
		"decl %%ecx              \n" \
		"jnz 2b                  \n" \
		: : "g"(multFactor), "r"(snd), "g"(sndbufl), "g"(sndbufr), "g"(len)
		: "eax", "ecx", "edi", "esi", "mm1", "mm2", "xmm1", "memory");
}

static inline void atti386_mixBuffers(Bit16s * buf1, Bit16s *buf2, int len) {
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
		: "ecx", "edi", "esi", "memory"
#ifdef __MMX__
		, "mm1", "mm2"
#endif
		);
}

static inline void atti386_mixBuffersRingMix(Bit16s * buf1, Bit16s *buf2, int len) {
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
		: "ecx", "edi", "esi", "memory"
#ifdef __MMX__
		, "mm1", "mm2", "mm3"
#endif
		);
}

static inline void atti386_mixBuffersRing(Bit16s * buf1, Bit16s *buf2, int len) {
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
		: "ecx", "edi", "esi", "memory"
#ifdef __MMX__
		, "mm1", "mm2"
#endif
		);
}

static inline void atti386_partialProductOutput(int quadlen, Bit16s leftvol, Bit16s rightvol, Bit16s *partialBuf, Bit16s *p1buf) {
	__asm__ __volatile__(
		"movl %0, %%ecx       \n" \
		"movw %1, %%ax        \n" \
		"shll $16, %%eax      \n" \
		"movw %2, %%ax        \n" \
		"movd %%eax, %%mm1    \n" \
		"movd %%eax, %%mm2    \n" \
		"psllq $32, %%mm1     \n" \
		"por  %%mm2, %%mm1    \n" \
		"movl %3, %%edi       \n" \
		"movl %4, %%esi       \n" \
	    "pushl %%ebx          \n" \
		"1:                   \n" \
		"movw 0(%%esi), %%bx  \n" \
		"addl $2, %%esi       \n" \
		"movw 0(%%esi), %%dx  \n" \
		"addl $2, %%esi       \n" \
		""                        \
		"movw %%dx, %%ax      \n" \
		"shll $16, %%eax      \n" \
		"movw %%dx, %%ax      \n" \
		"movd %%eax, %%mm2    \n" \
		"psllq $32, %%mm2     \n" \
		"movw %%bx, %%ax      \n" \
		"shll $16, %%eax      \n" \
		"movw %%bx, %%ax      \n" \
		"movd %%eax, %%mm3    \n" \
		"por  %%mm3, %%mm2    \n" \
		""                        \
		"pmulhw %%mm1, %%mm2  \n" \
		"movq %%mm2, 0(%%edi) \n" \
		"addl $8, %%edi       \n" \
		""                        \
		"decl %%ecx           \n" \
		"cmpl $0, %%ecx       \n" \
		"jg 1b                \n" \
		"emms                 \n"  \
	    "popl %%ebx           \n" \
		: : "g"(quadlen), "g"(leftvol), "g"(rightvol), "g"(partialBuf), "g"(p1buf)
		: "eax", "ecx", "edx", "edi", "esi", "memory"
#ifdef __MMX__
		, "mm1", "mm2", "mm3"
#endif
		);
}

#endif

bool DetectSIMD() {
#ifdef _MSC_VER
	bool found_simd;
	__asm {
		pushfd
		pop eax // get EFLAGS into eax
		mov ebx,eax // keep a copy
		xor eax,0x200000
		// toggle CPUID bit

		push eax
		popfd // set new EFLAGS
		pushfd
		pop eax // EFLAGS back into eax

		xor eax,ebx
		// have we changed the ID bit?

		je NO_SIMD
		// No, no CPUID instruction

		// we could toggle the
		// ID bit so CPUID is present
		mov eax,1

		cpuid // get processor features
		test edx,1<<25 // check the SIMD bit
		jz NO_SIMD
		mov found_simd,1
		jmp DONE
		NO_SIMD:
		mov found_simd,0
		DONE:
	}
	return found_simd;
#else
	return atti386_DetectSIMD();
#endif
}

bool Detect3DNow() {
#ifdef _MSC_VER
	bool found3D = false;
	__asm {
		pushfd
		pop eax
		mov edx, eax
		xor eax, 00200000h
		push eax
		popfd
		pushfd
		pop eax
		xor eax, edx
		jz NO_3DNOW

		mov eax, 80000000h
		cpuid

		cmp eax, 80000000h
		jbe NO_3DNOW

		mov eax, 80000001h
		cpuid
		test edx, 80000000h
		jz NO_3DNOW
		mov found3D, 1
NO_3DNOW:

	}
	return found3D;
#else
	return atti386_Detect3DNow();
#endif
}

float iir_filter_sse(float input,float *hist1_ptr, float *coef_ptr) {
	float output;

	// 1st number of coefficients array is overall input scale factor, or filter gain
	output = input * (*coef_ptr++);

#ifdef _MSC_VER
	__asm {

		movss xmm1,	output

		mov eax, coef_ptr
		movups xmm2, [eax]

		mov eax, hist1_ptr
		movlps xmm3, [eax]
		shufps xmm3, xmm3, 44h
		// hist1_ptr+1, hist1_ptr, hist1_ptr+1, hist1_ptr

		mulps xmm2, xmm3

		subss xmm1, xmm2
		// Rotate elements right
		shufps xmm2, xmm2, 39h
		subss xmm1, xmm2

		// Store new_hist
		movss DWORD PTR [eax], xmm1

		// Rotate elements right
		shufps xmm2, xmm2, 39h
		addss xmm1, xmm2

		// Rotate elements right
		shufps xmm2, xmm2, 39h
		addss xmm1, xmm2

		// Store previous hist
		movss DWORD PTR [eax+4], xmm3

		add coef_ptr, 16
		add hist1_ptr, 8

		mov eax, coef_ptr
		movups xmm2, [eax]

		mov eax, hist1_ptr
		movlps xmm3, [eax]
		shufps xmm3, xmm3, 44h
		// hist1_ptr+1, hist1_ptr, hist1_ptr+1, hist1_ptr

		mulps xmm2, xmm3

		subss xmm1, xmm2
		// Rotate elements right
		shufps xmm2, xmm2, 39h
		subss xmm1, xmm2

		// Store new_hist
		movss DWORD PTR [eax], xmm1

		// Rotate elements right
		shufps xmm2, xmm2, 39h
		addss xmm1, xmm2

		// Rotate elements right
		shufps xmm2, xmm2, 39h
		addss xmm1, xmm2

		// Store previous hist
		movss DWORD PTR [eax+4], xmm3

		movss output, xmm1
	}
#else
	output = atti386_iir_filter_sse(&output, hist1_ptr, coef_ptr);
#endif
	return output;
}

float iir_filter_3dnow(float input,float *hist1_ptr, float *coef_ptr) {
	float output;

	// 1st number of coefficients array is overall input scale factor, or filter gain
	output = input * (*coef_ptr++);

	// I find it very sad that 3DNow requires twice as many instructions as Intel's SSE
	// Intel does have the upper hand here.
#ifdef _MSC_VER
	float tmp;
	__asm {
		movq mm1, output
		mov ebx, coef_ptr
		movq mm2, [ebx]

		mov eax, hist1_ptr;
		movq mm3, [eax]

		pfmul mm2, mm3
		pfsub mm1, mm2

		psrlq mm2, 32
		pfsub mm1, mm2

		// Store new hist
		movd tmp, mm1

		add ebx, 8
		movq mm2, [ebx]
		movq mm3, [eax]

		pfmul mm2, mm3
		pfadd mm1, mm2

		psrlq mm2, 32
		pfadd mm1, mm2

		push tmp
		pop DWORD PTR [eax]

		movd DWORD PTR [eax+4], mm3

		add ebx, 8
		add eax, 8

		movq mm2, [ebx]
		movq mm3, [eax]

		pfmul mm2, mm3
		pfsub mm1, mm2

		psrlq mm2, 32
		pfsub mm1, mm2

		// Store new hist
		movd tmp, mm1

		add ebx, 8
		movq mm2, [ebx]
		movq mm3, [eax]

		pfmul mm2, mm3
		pfadd mm1, mm2

		psrlq mm2, 32
		pfadd mm1, mm2

		push tmp
		pop DWORD PTR [eax]
		movd DWORD PTR [eax+4], mm3

		movd output, mm1

		femms
	}
#else
	output = atti386_iir_filter_3DNow(output, hist1_ptr, coef_ptr);
#endif
	return output;
}

#if MT32EMU_USE_MMX > 0

int i386_partialProductOutput(int len, Bit16s leftvol, Bit16s rightvol, Bit16s *partialBuf, Bit16s *mixedBuf) {
	int tmplen = len >> 1;
	if (tmplen == 0) {
		return 0;
	}
#ifdef _MSC_VER
	__asm {
		mov ecx,tmplen
		mov ax, leftvol
		shl eax,16
		mov ax, rightvol
		movd mm1, eax
		movd mm2, eax
		psllq mm1, 32
		por mm1, mm2
		mov edi, partialBuf
		mov esi, mixedBuf
mmxloop1:
		mov bx, [esi]
		add esi,2
		mov dx, [esi]
		add esi,2

		mov ax, dx
		shl eax, 16
		mov ax, dx
		movd mm2,eax
		psllq mm2, 32
		mov ax, bx
		shl eax, 16
		mov ax, bx
		movd mm3,eax
		por mm2,mm3

		pmulhw mm2, mm1
		movq [edi], mm2
		add edi, 8

		dec ecx
		cmp ecx,0
		jg mmxloop1
		emms
	}
#else
	atti386_partialProductOutput(tmplen, leftvol, rightvol, partialBuf, mixedBuf);
#endif
	return tmplen << 1;
}

int i386_mixBuffers(Bit16s * buf1, Bit16s *buf2, int len) {
	int tmplen = len >> 2;
	if (tmplen == 0) {
		return 0;
	}
#ifdef _MSC_VER
	__asm {
		mov ecx, tmplen
		mov esi, buf1
		mov edi, buf2

mixloop1:
		movq mm1, [edi]
		movq mm2, [esi]
		paddw mm1,mm2
		movq [esi],mm1
		add edi,8
		add esi,8

		dec ecx
		cmp ecx,0
		jg mixloop1
		emms
	}
#else
	atti386_mixBuffers(buf1, buf2, tmplen);
#endif
	return tmplen << 2;
}


int i386_mixBuffersRingMix(Bit16s * buf1, Bit16s *buf2, int len) {
	int tmplen = len >> 2;
	if (tmplen == 0) {
		return 0;
	}
#ifdef _MSC_VER
	__asm {
		mov ecx, tmplen
		mov esi, buf1
		mov edi, buf2

mixloop2:
		movq mm1, [esi]
		movq mm2, [edi]
		movq mm3, mm1
		pmulhw mm1, mm2
		paddw mm1,mm3
		movq [esi],mm1
		add edi,8
		add esi,8

		dec ecx
		cmp ecx,0
		jg mixloop2
		emms
	}
#else
	atti386_mixBuffersRingMix(buf1, buf2, tmplen);
#endif
	return tmplen << 2;
}

int i386_mixBuffersRing(Bit16s * buf1, Bit16s *buf2, int len) {
	int tmplen = len >> 2;
	if (tmplen == 0) {
		return 0;
	}
#ifdef _MSC_VER
	__asm {
		mov ecx, tmplen
		mov esi, buf1
		mov edi, buf2

mixloop3:
		movq mm1, [esi]
		movq mm2, [edi]
		pmulhw mm1, mm2
		movq [esi],mm1
		add edi,8
		add esi,8

		dec ecx
		cmp ecx,0
		jg mixloop3
		emms
	}
#else
	atti386_mixBuffersRing(buf1, buf2, tmplen);
#endif
	return tmplen << 2;
}

int i386_produceOutput1(Bit16s *useBuf, Bit16s *stream, Bit32u len, Bit16s volume) {
	int tmplen = (len >> 1);
	if (tmplen == 0) {
		return 0;
	}
#ifdef _MSC_VER
	__asm {
		mov ecx, tmplen
		mov ax,volume
		shl eax,16
		mov ax,volume
		movd mm3,eax
		movd mm2,eax
		psllq mm3, 32
		por mm3,mm2
		mov esi, useBuf
		mov edi, stream
mixloop4:
		movq mm1, [esi]
		movq mm2, [edi]
		pmulhw mm1, mm3
		paddw mm1,mm2
		movq [edi], mm1

		add esi,8
		add edi,8

		dec ecx
		cmp ecx,0
		jg mixloop4
		emms
	}
#else
	atti386_produceOutput1(tmplen, volume, useBuf, stream);
#endif
	return tmplen << 1;
}

#endif

}

#endif
