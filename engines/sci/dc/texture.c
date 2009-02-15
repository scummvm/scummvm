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

#include "gp.h"

/* Creates the utility texture used for the font and mouse cursor. The
   resulting texture will be 256x256. */
#include "mouse1.h"
pvr_ptr_t util_texture;
pvr_poly_hdr_t util_txr_hdr;
void setup_util_texture() {
	uint16	*vram;
	int	x, y;
	pvr_poly_cxt_t	cxt;

	util_texture = pvr_mem_malloc(256*256*2);
	printf("util_texture at %08x\n", util_texture);
	vram = (uint16 *)util_texture;
	
	/* First dump in the mouse cursor */
	for (y=0; y<16; y++) {
		for (x=0; x<10; x++) {
			if (mouse1_xpm[y*10+x] == '.')
				*vram = 0xffff;
			else if (mouse1_xpm[y*10+x] == '+')
				*vram = 0xf000;
			else
				*vram = 0x0000;
			vram++;
		}
		vram += 256 - 10;
	}
	
	/* Now add the rest as ASCII characters */
	vram = (uint16 *)util_texture;
	for (y=0; y<8; y++) {
		for (x=0; x<16; x++) {
			/* Skip the first (it's a mouse pointer) */
			if (x != 0 || y != 0) 
				bfont_draw(vram, 256, 0, y*16+x);
			vram += 16;
		}
		vram += 23*256;
	}

	/* Setup a polygon header for the util texture */
	pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444 | PVR_TXRFMT_NONTWIDDLED,
		256, 256, util_texture, PVR_FILTER_NONE);
	pvr_poly_compile(&util_txr_hdr, &cxt);
}
