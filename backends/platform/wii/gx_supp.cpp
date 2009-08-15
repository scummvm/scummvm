/****************************************************************************
*	Generic GX Support for Emulators
*	softdev 2007
*	dhewg 2008
*
*	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation; either version 2 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License along
*	with this program; if not, write to the Free Software Foundation, Inc.,
*	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
* NGC GX Video Functions
*
* These are pretty standard functions to setup and use GX scaling.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "gx_supp.h"

#define DEFAULT_FIFO_SIZE (256 * 1024)

#define HASPECT 320
#define VASPECT 240

#ifdef __cplusplus
extern "C" {
#endif

/*** 2D ***/
static u32 whichfb;
static u32 *xfb[2];
GXRModeObj *vmode = NULL;

/*** 3D GX ***/
static u8 *gp_fifo;

/*** Texture memory ***/
static u8 *texturemem = NULL;
static u32 texturesize;

static GXTexObj texobj;
static Mtx view;
static u16 vwidth, vheight, oldvwidth, oldvheight;
static float tex_xT = 0.0f, tex_yT = 0.0f;

/* New texture based scaler */
typedef struct tagcamera {
	guVector pos;
	guVector up;
	guVector view;
} camera;

static s16 square[] ATTRIBUTE_ALIGN(32) = {
	-HASPECT, VASPECT, 0,
	HASPECT, VASPECT, 0,
	HASPECT, -VASPECT, 0,
	-HASPECT, -VASPECT, 0,
};

static camera cam = {
	{ 0.0f, 0.0f, 370.0f },
	{ 0.0f, 0.5f, 0.0f },
	{ 0.0f, 0.0f, -0.5f }
};

void GX_InitVideo() {
	vmode = VIDEO_GetPreferredMode(NULL);

	vmode->viWidth = 688;
	vmode->viXOrigin = (VI_MAX_WIDTH_PAL - 688) / 2;

	VIDEO_Configure(vmode);

	xfb[0] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer(vmode));
	xfb[1] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer(vmode));
	gp_fifo = (u8 *) memalign(32, DEFAULT_FIFO_SIZE);

	VIDEO_ClearFrameBuffer(vmode, xfb[0], COLOR_BLACK);
	VIDEO_ClearFrameBuffer(vmode, xfb[1], COLOR_BLACK);

	whichfb = 0;
	VIDEO_SetNextFramebuffer(xfb[whichfb]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
}

void GX_SetTexTrans(float xT, float yT) {
	tex_xT = xT;
	tex_yT = yT;
}

void GX_SetCamPosZ(float f) {
	cam.pos.z = f;
}

/****************************************************************************
 * Scaler Support Functions
 ****************************************************************************/
static void draw_init(void) {
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_INDEX8);
	GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX8);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	GX_SetArray(GX_VA_POS, square, 3 * sizeof(s16));

	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	GX_InvalidateTexAll();

	GX_InitTexObj(&texobj, texturemem, vwidth, vheight, GX_TF_RGB565,
					GX_CLAMP, GX_CLAMP, GX_FALSE);
}

static void draw_vert(u8 pos, u8 c, f32 s, f32 t) {
	GX_Position1x8(pos);
	GX_Color1x8(c);
	GX_TexCoord2f32(s, t);
}

static void draw_square(Mtx v) {
	Mtx m;
	Mtx mv;

	guMtxIdentity(m);
	guMtxTransApply(m, m, tex_xT, tex_yT, -100);
	guMtxConcat(v, m, mv);

	GX_LoadPosMtxImm(mv, GX_PNMTX0);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	draw_vert(0, 0, 0.0, 0.0);
	draw_vert(1, 0, 1.0, 0.0);
	draw_vert(2, 0, 1.0, 1.0);
	draw_vert(3, 0, 0.0, 1.0);
	GX_End();
}

/****************************************************************************
 * StartGX
 ****************************************************************************/
void GX_Start(u16 width, u16 height, s16 haspect, s16 vaspect) {
	static bool inited = false;
	Mtx p;
	GXColor gxbackground = { 0, 0, 0, 0xff };

	/*** Set new aspect ***/
	square[0] = square[9] = -haspect;
	square[3] = square[6] = haspect;
	square[1] = square[4] = vaspect;
	square[7] = square[10] = -vaspect;

	/*** Allocate 32byte aligned texture memory ***/
	texturesize = (width * height) * 2;

	if (texturemem)
		free(texturemem);

	texturemem = (u8 *) memalign(32, texturesize);
	memset(texturemem, 0, texturesize);

	/*** Setup for first call to scaler ***/
	oldvwidth = oldvheight = 0;

	if (inited)
		return;

	inited = true;

	/*** Clear out FIFO area ***/
	memset(gp_fifo, 0, DEFAULT_FIFO_SIZE);

	/*** Initialise GX ***/
	GX_Init(gp_fifo, DEFAULT_FIFO_SIZE);
	GX_SetCopyClear(gxbackground, 0x00ffffff);

	GX_SetViewport(0, 0, vmode->fbWidth, vmode->efbHeight, 0, 1);
	GX_SetDispCopyYScale((f32) vmode->xfbHeight / (f32) vmode->efbHeight);
	GX_SetScissor(0, 0, vmode->fbWidth, vmode->efbHeight);
	GX_SetDispCopySrc(0, 0, vmode->fbWidth, vmode->efbHeight);
	GX_SetDispCopyDst(vmode->fbWidth, vmode->xfbHeight);
	GX_SetCopyFilter(vmode->aa, vmode->sample_pattern, GX_TRUE,
						vmode->vfilter);
	GX_SetFieldMode(vmode->field_rendering,
					((vmode->viHeight == 2 * vmode->xfbHeight) ?
					GX_ENABLE : GX_DISABLE));
	GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(xfb[whichfb ^ 1], GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);

	guPerspective(p, 60, 1.33f, 10.0f, 1000.0f);
	GX_LoadProjectionMtx(p, GX_PERSPECTIVE);

	GX_Flush();
}

/****************************************************************************
* GX_Render
*
* Pass in a buffer, width and height to update as a tiled RGB565 texture
****************************************************************************/
void GX_Render(u16 width, u16 height, u8 *buffer, u16 pitch) {
	u16 h, w;
	u64 *dst = (u64 *) texturemem;
	u64 *src1 = (u64 *) buffer;
	u64 *src2 = (u64 *) (buffer + pitch);
	u64 *src3 = (u64 *) (buffer + (pitch * 2));
	u64 *src4 = (u64 *) (buffer + (pitch * 3));
	u16 rowpitch = (pitch >> 3) * 3 + pitch % 8;

	vwidth = width;
	vheight = height;

	whichfb ^= 1;

	if ((oldvheight != vheight) || (oldvwidth != vwidth)) {
		/** Update scaling **/
		oldvwidth = vwidth;
		oldvheight = vheight;
		draw_init();
		memset(&view, 0, sizeof(Mtx));
		guLookAt(view, &cam.pos, &cam.up, &cam.view);
		GX_SetViewport(0, 0, vmode->fbWidth, vmode->efbHeight, 0, 1);
	}

	GX_InvVtxCache();
	GX_InvalidateTexAll();
	GX_SetTevOp(GX_TEVSTAGE0, GX_DECAL);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

	for (h = 0; h < vheight; h += 4) {
		for (w = 0; w < (vwidth >> 2); w++) {
			*dst++ = *src1++;
			*dst++ = *src2++;
			*dst++ = *src3++;
			*dst++ = *src4++;
		}

		src1 += rowpitch;
		src2 += rowpitch;
		src3 += rowpitch;
		src4 += rowpitch;
	}

	DCFlushRange(texturemem, texturesize);

	GX_SetNumChans(1);
	GX_LoadTexObj(&texobj, GX_TEXMAP0);

	draw_square(view);

	GX_DrawDone();

	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_CopyDisp(xfb[whichfb], GX_TRUE);
	GX_Flush();

	VIDEO_SetNextFramebuffer(xfb[whichfb]);
	VIDEO_Flush();
}

#ifdef __cplusplus
}
#endif

