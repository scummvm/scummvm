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

/* Modified by Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl> */

#include <stdarg.h>
#include <stdio.h>
#include "gp.h"

#include "sintab.h"

/* Some misc 3D utils */

/* Rotate a 3-pair of coordinates by the given angle 0..255 */
void rotate(int zang, int xang, int yang, float *x, float *y, float *z) {
	float tx, ty, tz;
	
	tx = (mcos(zang)* *x - msin(zang)* *y);
	ty = (mcos(zang)* *y + msin(zang)* *x);
	*x = tx; *y = ty;
		
	tz = (mcos(xang)* *z - msin(xang)* *y);
	ty = (mcos(xang)* *y + msin(xang)* *z);
	*y = ty; *z = tz;

	tx = (mcos(yang)* *x - msin(yang)* *z);
	tz = (mcos(yang)* *z + msin(yang)* *x);
	*x = tx; *z = tz;
}

/* Draw the mouse cursor at the given location */
void draw_poly_mouse(int ptrx, int ptry, float alpha) {
	pvr_vertex_t	vert;

	/* Start a textured polygon set (with the font texture) */
	pvr_prim(&util_txr_hdr, sizeof(util_txr_hdr));

	vert.flags = PVR_CMD_VERTEX;
	vert.x = ptrx;
	vert.y = ptry + 16.0f;
	vert.z = 512.0f + alpha;
	vert.u = 0.0f;
	vert.v = 16.0f / 256.0f;
	vert.argb = PVR_PACK_COLOR(0.80f * alpha, 1.0f, 1.0f, 1.0f);
	vert.oargb = 0;
	pvr_prim(&vert, sizeof(vert));
	
	vert.x = ptrx;
	vert.y = ptry;
	vert.u = 0.0f;
	vert.v = 0.0f;
	pvr_prim(&vert, sizeof(vert));
	
	vert.x = ptrx + 10.0f;
	vert.y = ptry + 16.0f;
	vert.u = 10.0f / 256.0f;
	vert.v = 16.0f / 256.0f;
	pvr_prim(&vert, sizeof(vert));

	vert.flags = PVR_CMD_VERTEX_EOL;
	vert.x = ptrx + 10.0f;
	vert.y = ptry;
	vert.u = 10.0f / 256.0f;
	vert.v = 0.0f;
	pvr_prim(&vert, sizeof(vert));
}

/* Draw one font character (12x24); assumes polygon header already sent */
void draw_poly_char(float x1, float y1, float z1, float a, float r, float g, float b, int c) {
	pvr_vertex_t	vert;
	int ix = (c % 16) * 16;
	int iy = (c / 16) * 24;
	float u1 = ix * 1.0f / 256.0f;
	float v1 = iy * 1.0f / 256.0f;
	float u2 = (ix+12) * 1.0f / 256.0f;
	float v2 = (iy+24) * 1.0f / 256.0f;

	vert.flags = PVR_CMD_VERTEX;
	vert.x = x1;
	vert.y = y1 + 24.0f;
	vert.z = z1;
	vert.u = u1;
	vert.v = v2;
	vert.argb = PVR_PACK_COLOR(a,r,g,b);
	vert.oargb = 0;
	pvr_prim(&vert, sizeof(vert));
	
	vert.x = x1;
	vert.y = y1;
	vert.u = u1;
	vert.v = v1;
	pvr_prim(&vert, sizeof(vert));
	
	vert.x = x1 + 12.0f;
	vert.y = y1 + 24.0f;
	vert.u = u2;
	vert.v = v2;
	pvr_prim(&vert, sizeof(vert));

	vert.flags = PVR_CMD_VERTEX_EOL;
	vert.x = x1 + 12.0f;
	vert.y = y1;
	vert.u = u2;
	vert.v = v1;
	pvr_prim(&vert, sizeof(vert));
}

/* Draw a set of textured polygons at the given depth and color that
   represent a string of text. */
static char strbuf[1024];
void draw_poly_strf(float x1, float y1, float z1, float a, float r,
		float g, float b, char *fmt, ...) {
	va_list args;
	char *s;
	
	va_start(args, fmt);
	vsprintf(strbuf, fmt, args);
	va_end(args);

	pvr_prim(&util_txr_hdr, sizeof(util_txr_hdr));
	s = strbuf;
	while (*s) {
		if (*s == ' ') {
			x1 += 12.0f; s++;
		} else {
			draw_poly_char(x1+=12.0f, y1, z1, a, r, g, b, *s++);
		}
	}
}

/* Draw a horizontally centered set of textured polygons at the given depth
   and color that represent a string of text. Only for video mode 640x480. */
void draw_poly_strf_ctr(float y1, float z1, float a, float r, float g, float b,
		char *fmt, ...) {
	float x1;
	va_list args;
	char *s;
	
	va_start(args, fmt);
	vsnprintf(strbuf, 1024, fmt, args);
	va_end(args);

	x1 = 320.0f - strlen(strbuf) * 6.0f;

	pvr_prim(&util_txr_hdr, sizeof(util_txr_hdr));
	s = strbuf;
	while (*s) {
		if (*s == ' ') {
			x1 += 12.0f; s++;
		} else {
			draw_poly_char(x1+=12.0f, y1, z1, a, r, g, b, *s++);
		}
	}
}

/* Draw a polygon for a shaded box; wow, a nasty looking func =) */
void draw_poly_box(float x1, float y1, float x2, float y2, float z,
		float a1, float r1, float g1, float b1,
		float a2, float r2, float g2, float b2) {
	pvr_poly_cxt_t	cxt;
	pvr_poly_hdr_t	poly;
	pvr_vertex_t	vert;

	pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
	pvr_poly_compile(&poly, &cxt);
	pvr_prim(&poly, sizeof(poly));
	
	vert.flags = PVR_CMD_VERTEX;
	vert.x = x1; vert.y = y2; vert.z = z;
	vert.argb = PVR_PACK_COLOR(
		(a1+a2)/2,
		(r1+r2)/2,
		(g1+g2)/2,
		(b1+b2)/2);
	vert.oargb = 0;
	pvr_prim(&vert, sizeof(vert));
	
	vert.y = y1;
	vert.argb = PVR_PACK_COLOR(a1, r1, g1, b1);
	pvr_prim(&vert, sizeof(vert));
	
	vert.x = x2; vert.y = y2;
	vert.argb = PVR_PACK_COLOR(a2, r2, g2, b2);
	pvr_prim(&vert, sizeof(vert));
	
	vert.flags = PVR_CMD_VERTEX_EOL;
	vert.y = y1;
	vert.argb = PVR_PACK_COLOR(
		(a1+a2)/2,
		(r1+r2)/2,
		(g1+g2)/2,
		(b1+b2)/2);
	pvr_prim(&vert, sizeof(vert));
}
