/* 
 * Gamecube/Wii VIDEO/GX subsystem wrapper
 *
 * Copyright (C) 2008, 2009		Andre Heider "dhewg" <dhewg@wiibrew.org>
 *
 * This code is licensed to you under the terms of the GNU GPL, version 2;
 * see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 */

#include <malloc.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "gfx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FIFO_SIZE (256 * 1024)
#define ORIGIN_Z (-500)

static GXRModeObj *_vm = NULL;
static bool _dualstrike = false;

static u32 *_fb[2] = { NULL, NULL };
static u8 _fb_active = 0;

static u8 *_fifo = NULL;

static u16 _underscan_x = 0;
static u16 _underscan_y = 0;
static f32 _ar = 4.0 / 3.0;
static bool _pillarboxing = false;
static Mtx _view;

static struct {
	guVector pos;
	guVector up;
	guVector view;
} _camera = {
	{ 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.5f, 0.0f },
	{ 0.0f, 0.0f, -0.5f }
};

// Standard, StandardAa, Ds, DsAa
static GXRModeObj *mode_table[5][4] = {
	{ &TVNtsc480Prog, &TVNtsc480ProgAa, &TVNtsc240Ds, &TVNtsc240DsAa },
	{ &TVNtsc480IntDf, &TVNtsc480IntAa, &TVNtsc240Ds, &TVNtsc240DsAa },
	{ &TVPal528IntDf, &TVPal524IntAa, &TVPal264Ds, &TVPal264DsAa },
	{ &TVEurgb60Hz480IntDf, &TVEurgb60Hz480IntAa, &TVEurgb60Hz240Ds, &TVEurgb60Hz240DsAa },
	{ &TVMpal480IntDf, &TVMpal480IntAa, &TVMpal240Ds, &TVMpal240DsAa }
};

static gfx_video_mode_t _gfx_video_get_mode(void) {
	gfx_video_mode_t mode;

#ifdef HW_RVL
	if ((CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable()) {
		mode = GFX_MODE_PROGRESSIVE;
	} else {
		switch (CONF_GetVideo()) {
		case CONF_VIDEO_PAL:
			if (CONF_GetEuRGB60() > 0)
				mode = GFX_MODE_EURGB60;
			else
				mode = GFX_MODE_PAL;
			break;

		case CONF_VIDEO_MPAL:
			mode = GFX_MODE_MPAL;
			break;

		default:
			mode = GFX_MODE_NTSC;
			break;
		}
	}
#else
	switch (VIDEO_GetCurrentTvMode()) {
	case VI_PAL:
		mode = GFX_MODE_PAL;
		break;
	case VI_MPAL:
		mode = GFX_MODE_MPAL;
		break;
	default:
		mode = GFX_MODE_NTSC;
		break;
	}
#endif

	return mode;
}

void gfx_video_init(gfx_video_mode_t mode, gfx_video_setup_t setup) {
	u8 i;

	if (mode == GFX_MODE_AUTO)
		mode = _gfx_video_get_mode();

	_vm = mode_table[mode][setup];

	_vm->viWidth = 672;
	_vm->viXOrigin = (VI_MAX_WIDTH_NTSC - _vm->viWidth) / 2;

	if (_vm)
		VIDEO_WaitVSync();

	VIDEO_Configure(_vm);

	if (_fb[0])
		free(MEM_K1_TO_K0(_fb[0]));
	if (_fb[1])
		free(MEM_K1_TO_K0(_fb[1]));

	_fb[0] = (u32 *) MEM_K0_TO_K1(SYS_AllocateFramebuffer(_vm));
	_fb[1] = (u32 *) MEM_K0_TO_K1(SYS_AllocateFramebuffer(_vm));

	VIDEO_ClearFrameBuffer(_vm, _fb[0], COLOR_BLACK);
	VIDEO_ClearFrameBuffer(_vm, _fb[1], COLOR_BLACK);

	VIDEO_SetNextFramebuffer(_fb[_fb_active]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();

	for (i = 0; i < 4; ++i)
		VIDEO_WaitVSync();
}

void gfx_video_deinit(void) {
	u8 i;

	VIDEO_WaitVSync();
	VIDEO_SetBlack(TRUE);
	VIDEO_SetNextFramebuffer(NULL);
	VIDEO_Flush();

	for (i = 0; i < 4; ++i)
		VIDEO_WaitVSync();

	if (_fb[0]) {
		free(MEM_K1_TO_K0(_fb[0]));
		_fb[0] = NULL;
	}

	if (_fb[1]) {
		free(MEM_K1_TO_K0(_fb[1]));
		_fb[1] = NULL;
	}
}

u16 gfx_video_get_width(void) {
	return _vm->fbWidth;
}

u16 gfx_video_get_height(void) {
	return _vm->efbHeight;
}

static void _update_viewport(void) {
	f32 ar;
	u16 correction;

	u16 usy = _underscan_y;

	if (!_dualstrike)
		usy *= 2;

	u16 x1 = _underscan_x * 2;
	u16 y1 = usy;
	u16 x2 = _vm->fbWidth - _underscan_x * 4;
	u16 y2 = _vm->efbHeight - usy * 2;

	if (_pillarboxing)
		ar = 16.0 / 9.0;
	else
		ar = 4.0 / 3.0;

	if (fabs(ar - _ar) > 0.01) {
		if (ar > _ar) {
			correction = _vm->viWidth - 
						(u16) round((f32) _vm->viWidth * _ar / ar);

			x1 += correction / 2;
			x2 -= correction;
		} else {
			correction = _vm->efbHeight - 
						(u16) round((f32) _vm->efbHeight * ar / _ar);

			if (_dualstrike)
				correction /= 2;

			y1 += correction / 2;
			y2 -= correction;
		}
	}

	GX_SetViewport(x1, y1, x2, y2, 0, 1);
	GX_SetScissor(x1, y1, x2, y2);
}

void gfx_init(void) {
	Mtx44 p;
	GXColor bg = { 0, 0, 0, 0xff };
	f32 yscale;
	u32 xfbHeight;

	GX_AbortFrame();

	if (!_fifo)
		_fifo = (u8 *) memalign(32, FIFO_SIZE);

	memset(_fifo, 0, FIFO_SIZE);
	GX_Init(_fifo, FIFO_SIZE);

	GX_SetCopyClear(bg, 0x00ffffff);

	_update_viewport();

	_dualstrike = _vm->viHeight == 2 * _vm->xfbHeight;
	yscale = GX_GetYScaleFactor(_vm->efbHeight, _vm->xfbHeight);
	xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetDispCopySrc(0, 0, _vm->fbWidth, _vm->efbHeight);
	GX_SetDispCopyDst(_vm->fbWidth, xfbHeight);
	GX_SetCopyFilter(_vm->aa, _vm->sample_pattern, GX_TRUE, _vm->vfilter);
	GX_SetFieldMode(_vm->field_rendering, _dualstrike ? GX_ENABLE : GX_DISABLE);

	if (_vm->aa)
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	GX_SetCullMode(GX_CULL_NONE);
	GX_SetDispCopyGamma(GX_GM_1_0);

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	GX_InvVtxCache();
	GX_InvalidateTexAll();

	memset(&_view, 0, sizeof(Mtx));
	guLookAt(_view, &_camera.pos, &_camera.up, &_camera.view);

	guOrtho(p, 0, _vm->efbHeight, 0, _vm->fbWidth, 100, 1000);
	GX_LoadProjectionMtx (p, GX_ORTHOGRAPHIC);

	GX_Flush();
}

void gfx_deinit(void) {
	GX_AbortFrame();

	free(_fifo);
	_fifo = NULL;
}

void gfx_set_underscan(u16 underscan_x, u16 underscan_y) {
	_underscan_x = underscan_x;
	_underscan_y = underscan_y;

	if (_underscan_x > 32)
		_underscan_x = 32;

	if (_underscan_y > 32)
		_underscan_y = 32;

	_update_viewport();
	GX_Flush();
}

void gfx_set_ar(f32 ar) {
	_ar = ar;

	if (ar < 16.0 / 480.0)
		ar = 16.0 / 480.0;

	if (ar > 640.0 / 16.0)
		ar = 640.0 / 16.0;

	_update_viewport();
	GX_Flush();
}

void gfx_set_pillarboxing(bool enable) {
	_pillarboxing = enable;

	_update_viewport();
	GX_Flush();
}

bool gfx_tex_init(gfx_tex_t *tex, gfx_tex_format_t format, u32 tlut_name,
					u16 width, u16 height) {
	u8 bpp;
	u8 fmt_tex;
	bool tlut;
	u8 fmt_tlut;
	u32 ax, ay;
	u32 memsize;

	if (!tex)
		return false;

	switch(format) {
	case GFX_TF_RGB565:
		bpp = 2;
		fmt_tex = GX_TF_RGB565;
		tlut = false;
		fmt_tlut = 0;
		ax = 3;
		ay = 3;
		break;

	case GFX_TF_RGB5A3:
		bpp = 2;
		fmt_tex = GX_TF_RGB5A3;
		tlut = false;
		fmt_tlut = 0;
		ax = 3;
		ay = 3;
		break;

	case GFX_TF_PALETTE_RGB565:
		bpp = 1;
		fmt_tex = GX_TF_CI8;
		tlut = true;
		fmt_tlut = GX_TL_RGB565;
		ax = 7;
		ay = 3;
		break;

	case GFX_TF_PALETTE_RGB5A3:
		bpp = 1;
		fmt_tex = GX_TF_CI8;
		tlut = true;
		fmt_tlut = GX_TL_RGB5A3;
		ax = 7;
		ay = 3;
		break;

	default:
		gfx_tex_deinit(tex);
		return false;
	}

	if ((width & ax) || (height & ay)) {
		gfx_tex_deinit(tex);
		return false;
	}

	if (tlut) {
		if (!tex->palette) {
			tex->palette = (u16 *) memalign(32, 256 * 2);
			if (!tex->palette) {
				gfx_tex_deinit(tex);
				return false;
			}

			memset(tex->palette, 0, 256 * 2);
			DCFlushRange(tex->palette, 256 * 2);
		}

		tex->tlut_name = tlut_name;
		GX_InitTlutObj(&tex->tlut, tex->palette, fmt_tlut, 256);
	} else {
		tex->tlut_name = 0;
		free(tex->palette);
		tex->palette = NULL;
	}

	if (!tex->pixels || (width != tex->width) || (height != tex->height ||
			bpp != tex->bpp)) {
		free(tex->pixels);

		memsize = width * height * bpp;
		tex->pixels = memalign(32, memsize);

		if (!tex->pixels) {
			gfx_tex_deinit(tex);
			return false;
		}
	
		memset(tex->pixels, 0, memsize);
		DCFlushRange(tex->pixels, memsize);
	}

	tex->format = format;
	tex->width = width;
	tex->height = height;
	tex->bpp = bpp;

	if (tlut) {
		GX_LoadTlut(&tex->tlut, tlut_name);
		GX_InitTexObjCI(&tex->obj, tex->pixels, width, height, fmt_tex,
						GX_CLAMP, GX_CLAMP, GX_FALSE, tlut_name);
	} else {
		GX_InitTexObj(&tex->obj, tex->pixels, width, height, fmt_tex,
						GX_CLAMP, GX_CLAMP, GX_FALSE);
	}

	return true;
}

void gfx_tex_deinit(gfx_tex_t *tex) {
	if (!tex)
		return;

	free(tex->pixels);
	free(tex->palette);
	memset(tex, 0, sizeof(gfx_tex_t));
}

void gfx_coords(gfx_coords_t *coords, gfx_tex_t *tex, gfx_coord_t type) {
	if (!coords || !tex)
		return;

	switch(type) {
	case GFX_COORD_FULLSCREEN:
		coords->x = 0.0;
		coords->y = 0.0;
		coords->w = _vm->fbWidth;
		coords->h = _vm->efbHeight;
		break;

	case GFX_COORD_CENTER:
		coords->x = (_vm->fbWidth - tex->width) / 2;
		coords->y = (_vm->efbHeight - tex->height) / 2;
		coords->w = tex->width;
		coords->h = tex->height;
		break;

	default:
		break;
	}
}

bool gfx_tex_flush_texture(gfx_tex_t *tex) {
	if (!tex)
		return false;

	DCFlushRange(tex->pixels, tex->width * tex->height * tex->bpp);
	return true;
}

bool gfx_tex_flush_palette(gfx_tex_t *tex) {
	if (!tex || !tex->palette)
		return false;

	DCFlushRange(tex->palette, 256 * 2);
	GX_LoadTlut(&tex->tlut, tex->tlut_name);

	return true;
}

bool gfx_tex_clear_palette(gfx_tex_t *tex) {
	if (!tex || !tex->palette)
		return false;

	memset(tex->palette, 0, 256 * 2);
	DCFlushRange(tex->palette, 256 * 2);
	GX_LoadTlut(&tex->tlut, tex->tlut_name);

	return true;
}

bool gfx_tex_convert(gfx_tex_t *tex, const void *src) {
	bool ret;
	u16 w, h, x, y;
	u64 *dst, *src1, *src2, *src3, *src4;
	u16 rowpitch;
	u16 pitch;
	const u8 *s = (const u8 *) src;

	if (!tex)
		return false;

	ret = false;
	w = tex->width;
	h = tex->height;

	switch(tex->format) {
	case GFX_TF_RGB565:
	case GFX_TF_RGB5A3:
		pitch = w * 2;
		dst = (u64 *) tex->pixels;
		src1 = (u64 *) s;
		src2 = (u64 *) (s + pitch);
		src3 = (u64 *) (s + (pitch * 2));
		src4 = (u64 *) (s + (pitch * 3));
		rowpitch = (pitch >> 3) * 3 + pitch % 8;

		for (y = 0; y < h; y += 4) {
			for (x = 0; x < (w >> 2); x++) {
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

		ret = true;
		break;

	case GFX_TF_PALETTE_RGB565:
	case GFX_TF_PALETTE_RGB5A3:
		pitch = w;
		dst = (u64 *) tex->pixels;
		src1 = (u64 *) s;
		src2 = (u64 *) (s + pitch);
		src3 = (u64 *) (s + (pitch * 2));
		src4 = (u64 *) (s + (pitch * 3));
		rowpitch = (pitch >> 3) * 3 + pitch % 8;

		for (y = 0; y < h; y += 4) {
			for (x = 0; x < (w >> 3); x++) {
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

		ret = true;
		break;

	default:
		break;
	}

	if (ret)
		DCFlushRange(tex->pixels, w * h * tex->bpp);

	return ret;
}

void gfx_frame_start(void) {
	GX_InvVtxCache();
	GX_InvalidateTexAll();

	GX_SetNumChans(1);
	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
}

void gfx_frame_end(void) {
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);

	GX_CopyDisp(_fb[_fb_active], GX_TRUE);
	GX_Flush();
	GX_DrawDone();

	VIDEO_SetNextFramebuffer(_fb[_fb_active]);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	_fb_active ^= 1;
}

static void _tex_vert(f32 x, f32 y, f32 z, f32 s, f32 t, u32 c) {
	GX_Position3f32(x, y, z);
	GX_Color1u32(c);
	GX_TexCoord2f32(s, t);
}

void gfx_draw_tex(gfx_tex_t *tex, gfx_coords_t *coords) {
	Mtx m;
	Mtx mv;

	if (!tex || !coords)
		return;

	GX_LoadTexObj(&tex->obj, GX_TEXMAP0);

	guMtxIdentity(m);
	guMtxTrans(m, coords->x, coords->y, ORIGIN_Z);
	guMtxConcat(_view, m, mv);
	GX_LoadPosMtxImm(mv, GX_PNMTX0);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	_tex_vert(0.0, 0.0, 0.0, 0.0, 0.0, 0xffffffff);
	_tex_vert(coords->w, 0.0, 0.0, 1.0, 0.0, 0xffffffff);
	_tex_vert(coords->w, coords->h, 0.0, 1.0, 1.0, 0xffffffff);
	_tex_vert(0.0, coords->h, 0.0, 0.0, 1.0, 0xffffffff);
	GX_End();
}

#ifdef __cplusplus
}
#endif

