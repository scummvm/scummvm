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

#ifndef MT32EMU_I386_H
#define MT32EMU_I386_H

namespace MT32Emu {
#ifdef MT32EMU_HAVE_X86

// Function that detects the availablity of SSE SIMD instructions
bool DetectSIMD();
// Function that detects the availablity of 3DNow instructions
bool Detect3DNow();

float iir_filter_sse(float input,float *hist1_ptr, float *coef_ptr);
float iir_filter_3dnow(float input,float *hist1_ptr, float *coef_ptr);
float iir_filter_normal(float input,float *hist1_ptr, float *coef_ptr);

#if MT32EMU_USE_MMX > 0
int i386_partialProductOutput(int len, Bit16s leftvol, Bit16s rightvol, Bit16s *partialBuf, Bit16s *mixedBuf);
int i386_mixBuffers(Bit16s * buf1, Bit16s *buf2, int len);
int i386_mixBuffersRingMix(Bit16s * buf1, Bit16s *buf2, int len);
int i386_mixBuffersRing(Bit16s * buf1, Bit16s *buf2, int len);
int i386_produceOutput1(Bit16s *useBuf, Bit16s *stream, Bit32u len, Bit16s volume);
#endif

#endif

}

#endif
