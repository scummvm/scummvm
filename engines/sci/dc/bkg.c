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


/* This module will manage the spiffy background effects */

static int ang = 0;
static pvr_ptr_t chktexture;
static float horizon = 340.0f;

/* Make a nice (now familiar =) XOR pattern texture */
void bkg_setup() {
	int x, y;
	uint16 *texture;
	
	chktexture = pvr_mem_malloc(64*64*2);
	texture = (uint16*)chktexture;
	for (y=0; y<64; y++)
		for (x=0; x<64; x++) {
			int v = ((x*4)^(y*4)) & 255;
			*texture++ = ((v >> 3) << 11)
				| ((v >> 2) << 5)
				| ((v >> 3) << 0); 
		}
}

/* Draws the floor polygon */
static void draw_floor_poly() {
	pvr_vertex_t vert;
	float u2 = 0.374f, j = 0.0f;

	vert.flags = PVR_CMD_VERTEX;
	vert.x = 0.0f; vert.y = 480.0f; vert.z = 64.0f;
	vert.u = u2 - 0.5f; vert.v = 1.0f - 0.5f;
	rotate(ang, 0, 0, &vert.u, &vert.v, &j);
	vert.u += 0.5f; vert.v += 0.5f;
	vert.argb = PVR_PACK_COLOR(1.0f, 0.2f, 1.0f, 1.0f);
	vert.oargb = 0;
	pvr_prim(&vert, sizeof(vert));

	vert.x = 0.0f; vert.y = horizon; vert.z = 16.0f;
	vert.u = 0.0f - 0.5f; vert.v = 0.0f - 0.5f;
	rotate(ang, 0, 0, &vert.u, &vert.v, &j);
	vert.u += 0.5f; vert.v += 0.5f;
	vert.argb = PVR_PACK_COLOR(1.0f, 0.5f, 0.3f, 1.0f);
	pvr_prim(&vert, sizeof(vert));

	vert.x = 640.0f; vert.y = 480.0f; vert.z = 64.0f;
	vert.u = (1.0f - u2) - 0.5f; vert.v = 1.0f - 0.5f;
	rotate(ang, 0, 0, &vert.u, &vert.v, &j);
	vert.u += 0.5f; vert.v += 0.5f;
	vert.argb = PVR_PACK_COLOR(1.0f, 0.2f, 1.0f, 1.0f);
	pvr_prim(&vert, sizeof(vert));

	vert.flags = PVR_CMD_VERTEX_EOL;
	vert.x = 640.0f; vert.y = horizon; vert.z = 16.0f;
	vert.u = 1.0f - 0.5f; vert.v = 0.0f - 0.5f;
	rotate(ang, 0, 0, &vert.u, &vert.v, &j);
	vert.u += 0.5f; vert.v += 0.5f;
	vert.argb = PVR_PACK_COLOR(1.0f, 0.5f, 0.3f, 1.0f);
	pvr_prim(&vert, sizeof(vert));
}

/* Draws the "wall" polygon */
static void draw_wall_poly() {
	pvr_vertex_t	vertc;

	vertc.flags = PVR_CMD_VERTEX;
	vertc.x = 0.0f; vertc.y = horizon; vertc.z = 64.0f;
	vertc.argb = PVR_PACK_COLOR(1.0f, 0.5f, 0.3f, 1.0f);
	vertc.oargb = 0;
	pvr_prim(&vertc, sizeof(vertc));
	
	vertc.x = 0.0f; vertc.y = 0.0f; vertc.z = 64.0f;
	vertc.argb = PVR_PACK_COLOR(1.0f, 0.0f, 0.0f, 0.0f);
	pvr_prim(&vertc, sizeof(vertc));

	vertc.x = 640.0f; vertc.y = horizon; vertc.z = 64.0f;
	vertc.argb = PVR_PACK_COLOR(1.0f, 0.5f, 0.3f, 1.0f);
	pvr_prim(&vertc, sizeof(vertc));
	
	vertc.flags = PVR_CMD_VERTEX_EOL;
	vertc.x = 640.0f; vertc.y = 0.0f; vertc.z = 64.0f;
	vertc.argb = PVR_PACK_COLOR(1.0f, 0.0f, 0.0f, 0.0f);
	pvr_prim(&vertc, sizeof(vertc));
}

void bkg_render() {
	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t poly;
	
	/* "floor" polygon */
	pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED,
		64, 64, chktexture, PVR_FILTER_BILINEAR);
	pvr_poly_compile(&poly, &cxt);
	pvr_prim(&poly, sizeof(poly));

	draw_floor_poly();
	
	/* "wall" polygon */
	pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
	pvr_poly_compile(&poly, &cxt);
	pvr_prim(&poly, sizeof(poly));
	
	draw_wall_poly();
	
	ang = (ang-1) & 255;
}
