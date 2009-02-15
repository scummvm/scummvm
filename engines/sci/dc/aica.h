/*
 * Copyright 2000, 2001, 2002
 *         Dan Potter. All rights reserved.
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Cryptic Allusion nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __AICA_H
#define __AICA_H

/* volatile unsigned char *dc_snd_base = (unsigned char *)0x00800000; */
#define dc_snd_base ((volatile unsigned char *)0x00800000)

/* Some convienence macros */
#define SNDREG32A(x) ((volatile unsigned long *)(dc_snd_base + (x)))
#define SNDREG32(x) (*SNDREG32A(x))
#define SNDREG8A(x) (dc_snd_base + (x))
#define SNDREG8(x) (*SNDREG8A(x))
#define CHNREG32A(chn, x) SNDREG32A(0x80*(chn) + (x))
#define CHNREG32(chn, x) (*CHNREG32A(chn, x))
#define CHNREG8A(chn, x) SNDREG8A(0x80*(chn) + (x))
#define CHNREG8(chn, x) (*CHNREG8A(chn, x))

void aica_init();
void aica_play(int ch, int delay);
void aica_sync_play(uint32 chmap);
void aica_stop(int ch);
void aica_vol(int ch);
void aica_pan(int ch);
void aica_freq(int ch);
int aica_get_pos(int ch);

#endif	/* __AICA_H */
