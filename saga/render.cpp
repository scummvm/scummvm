/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Main rendering loop

#include "reinherit.h"

#include "systimer.h"
#include "yslib.h"

#include <SDL.h>

#include "actor_mod.h"
#include "console_mod.h"
#include "cvar_mod.h"
#include "font_mod.h"
#include "game_mod.h"
#include "gfx_mod.h"
#include "interface_mod.h"
#include "scene_mod.h"
#include "sprite_mod.h"
#include "text_mod.h"

#include "actionmap_mod.h"
#include "objectmap_mod.h"

#include "render_mod.h"
#include "render.h"

namespace Saga {

static R_RENDER_MODULE RenderModule;

const char *test_txt = "The quick brown fox jumped over the lazy dog. She sells sea shells down by the sea shore.";

int RENDER_Register() {
	// Register "r_fullscreen" cfg cvar
	RenderModule.r_fullscreen = R_FULLSCREEN_DEFAULT;

	if (CVAR_Register_I(&RenderModule.r_fullscreen,
		"r_fullscreen", NULL, R_CVAR_CFG, 0, 1) != R_SUCCESS) {
		return R_FAILURE;
	}

	// Register "r_doubleres" cfg cvar
	RenderModule.r_doubleres = R_DOUBLERES_DEFAULT;

	if (CVAR_Register_I(&RenderModule.r_doubleres,
		"r_doubleres", NULL, R_CVAR_CFG, 0, 1) != R_SUCCESS) {
		return R_FAILURE;
	}

	// Register "r_hicolor" cfg cvar
	RenderModule.r_hicolor = R_HICOLOR_DEFAULT;

	if (CVAR_Register_I(&RenderModule.r_hicolor,
		"r_hicolor", NULL, R_CVAR_CFG, 0, 1) != R_SUCCESS) {
		return R_FAILURE;
	}

	// Register "r_softcursor" cfg cvar
	RenderModule.r_softcursor = R_SOFTCURSOR_DEFAULT;

	if (CVAR_Register_I(&RenderModule.r_softcursor,
		"r_softcursor", NULL, R_CVAR_CFG, 0, 1) != R_SUCCESS) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int RENDER_Init() {
	R_GAME_DISPLAYINFO disp_info;
	R_SYSGFX_INIT gfx_init;
	int result;
	int tmp_w, tmp_h, tmp_bytepp;

	// Initialize system graphics
	GAME_GetDisplayInfo(&disp_info);

	gfx_init.backbuf_bpp = 8;	// all games are 8 bpp so far
	gfx_init.backbuf_w = disp_info.logical_w;
	gfx_init.backbuf_h = disp_info.logical_h;

	if (RenderModule.r_hicolor) {
		gfx_init.screen_bpp = 16;
	} else {
		gfx_init.screen_bpp = 8;
	}

	gfx_init.screen_w = disp_info.logical_w;
	gfx_init.screen_h = disp_info.logical_h;

	// Don't try to double a game exceeding the resolution limit
	// (640x480 would get doubled to 1280 x 960!) */
	if (disp_info.logical_w > R_DOUBLE_RESLIMIT) {
		RenderModule.r_doubleres = 0;
	}

	if (RenderModule.r_doubleres) {
		gfx_init.screen_w *= 2;
		gfx_init.screen_h *= 2;
	}

	gfx_init.fullscreen = RenderModule.r_fullscreen;

	if (SYSGFX_Init(&gfx_init) != R_SUCCESS) {
		return R_FAILURE;
	}

	// Initialize FPS timer callback
	result = SYSTIMER_CreateTimer(&RenderModule.r_fps_timer, 1000, NULL, RENDER_FpsTimer);
	if (result != R_SUCCESS) {
		return R_FAILURE;
	}

	// Create background buffer 
	RenderModule.r_bg_buf_w = disp_info.logical_w;
	RenderModule.r_bg_buf_h = disp_info.logical_h;
	RenderModule.r_bg_buf = (byte *)calloc(disp_info.logical_w, disp_info.logical_h);

	if (RenderModule.r_bg_buf == NULL) {
		return R_MEM;
	}

	// Allocate temp buffer for animation decoding, 
	// graphics scalers (2xSaI), etc.
	tmp_w = disp_info.logical_w;
	tmp_h = disp_info.logical_h + 4; // BG unbanking requres extra rows
	tmp_bytepp = 1;

	if (RenderModule.r_doubleres) {
		tmp_w *= 2;
		tmp_h *= 2;
	}

	if (RenderModule.r_hicolor) {
		tmp_bytepp = 2;
	}

	RenderModule.r_tmp_buf = (byte *)calloc(1, tmp_w * tmp_h * tmp_bytepp);
	if (RenderModule.r_tmp_buf == NULL) {

		free(RenderModule.r_bg_buf);
		return R_MEM;
	}

	RenderModule.r_tmp_buf_w = tmp_w;
	RenderModule.r_tmp_buf_h = tmp_h;

	RenderModule.r_screen_surface = SYSGFX_GetScreenSurface();
	RenderModule.r_backbuf_surface = SYSGFX_GetBackBuffer();

	// Initialize cursor state
	if (RenderModule.r_softcursor) {
		SYSINPUT_HideMouse();
	}

	RenderModule.initialized = 1;

	return R_SUCCESS;
}

int RENDER_DrawScene() {
	R_SURFACE *screen_surface;
	R_SURFACE *backbuf_surface;
	R_SURFACE *display_surface;
	R_GAME_DISPLAYINFO disp_info;
	R_SCENE_INFO scene_info;
	SCENE_BGINFO bg_info;
	R_POINT bg_pt;
	char txt_buf[20];
	int fps_width;
	R_POINT mouse_pt;
	int mouse_x, mouse_y;
	int surface_converted = 0;

	if (!RenderModule.initialized) {
		return R_FAILURE;
	}

	RenderModule.r_framecount++;

	screen_surface = RenderModule.r_screen_surface;
	backbuf_surface = RenderModule.r_backbuf_surface;

	// Get mouse coordinates
	SYSINPUT_GetMousePos(&mouse_x, &mouse_y);

	mouse_pt.x = mouse_x;
	mouse_pt.y = mouse_y;

	if (RenderModule.r_doubleres) {
		mouse_pt.x /= 2;
		mouse_pt.y /= 2;
	}

	SCENE_GetBGInfo(&bg_info);
	GAME_GetDisplayInfo(&disp_info);
	bg_pt.x = 0;
	bg_pt.y = 0;

	// Display scene background
	SCENE_Draw(backbuf_surface);

	// Display scene maps, if applicable
	if (RENDER_GetFlags() & RF_OBJECTMAP_TEST) {
		OBJECTMAP_Draw(backbuf_surface, &mouse_pt, SYSGFX_GetWhite(), SYSGFX_GetBlack());
		ACTIONMAP_Draw(backbuf_surface, SYSGFX_MatchColor(R_RGB_RED));
	}

	// Draw queued actors
	ACTOR_DrawList();

	// Draw queued text strings
	SCENE_GetInfo(&scene_info);

	TEXT_DrawList(scene_info.text_list, backbuf_surface);

	// Handle user input
	SYSINPUT_ProcessInput();

	// Display rendering information
	if (RenderModule.r_flags & RF_SHOW_FPS) {
		sprintf(txt_buf, "%d", RenderModule.r_fps);
		fps_width = FONT_GetStringWidth(SMALL_FONT_ID, txt_buf, 0, FONT_NORMAL);
		FONT_Draw(SMALL_FONT_ID, backbuf_surface, txt_buf, 0, backbuf_surface->buf_w - fps_width, 2,
					SYSGFX_GetWhite(), SYSGFX_GetBlack(), FONT_OUTLINE);
		switch (RenderModule.r_mode) {
		case RM_SCANLINES:
			FONT_Draw(SMALL_FONT_ID, backbuf_surface, "Scanlines", 0, 2, 2,
						SYSGFX_GetWhite(), SYSGFX_GetBlack(), FONT_OUTLINE);
			break;
		case RM_2XSAI:
			FONT_Draw(SMALL_FONT_ID, backbuf_surface, "2xSaI", 0, 2, 2,
						SYSGFX_GetWhite(), SYSGFX_GetBlack(), FONT_OUTLINE);
			break;
		case RM_SUPER2XSAI:
			FONT_Draw(SMALL_FONT_ID, backbuf_surface, "Super2xSaI", 0, 2, 2,
						SYSGFX_GetWhite(), SYSGFX_GetBlack(), FONT_OUTLINE);
			break;
		case RM_SUPEREAGLE:
			FONT_Draw(SMALL_FONT_ID, backbuf_surface, "SuperEagle", 0, 2, 2,
						SYSGFX_GetWhite(), SYSGFX_GetBlack(), FONT_OUTLINE);
			break;
		}

	}

	// Display "paused game" message, if applicable
	if (RenderModule.r_flags & RF_RENDERPAUSE) {
		int msg_len = strlen(R_PAUSEGAME_MSG);
		int msg_w = FONT_GetStringWidth(BIG_FONT_ID, R_PAUSEGAME_MSG, msg_len, FONT_OUTLINE);
		FONT_Draw(BIG_FONT_ID, backbuf_surface, R_PAUSEGAME_MSG, msg_len,
				(backbuf_surface->buf_w - msg_w) / 2, 90, SYSGFX_GetWhite(), SYSGFX_GetBlack(), FONT_OUTLINE);
	}

	// Update user interface

	INTERFACE_Update(&mouse_pt, UPDATE_MOUSEMOVE);

	if (RenderModule.r_softcursor) {
		GFX_DrawCursor(backbuf_surface, &mouse_pt);
	}

	// Display text formatting test, if applicable
	if (RenderModule.r_flags & RF_TEXT_TEST) {
		TEXT_Draw(MEDIUM_FONT_ID, backbuf_surface, test_txt, mouse_pt.x, mouse_pt.y,
				SYSGFX_GetWhite(), SYSGFX_GetBlack(), FONT_OUTLINE | FONT_CENTERED);
	}

	// Display palette test, if applicable
	if (RenderModule.r_flags & RF_PALETTE_TEST) {
		GFX_DrawPalette(backbuf_surface);
	}

	// Draw console
	CON_Draw(backbuf_surface);

	// Display the current frame
	if (RenderModule.r_hicolor) {
		display_surface = SYSGFX_FormatToDisplay(backbuf_surface);
		if (display_surface == NULL) {
			R_printf(R_STDERR, "Error: Back buffer conversion failed!\n");
			return R_FAILURE;
		}
		surface_converted = 1;
	} else {
		display_surface = backbuf_surface;
	}

	SYSGFX_LockSurface(screen_surface);
	SYSGFX_LockSurface(display_surface);

	switch (RenderModule.r_mode) {
	case RM_SCANLINES:
		break;
	default:
		if (RenderModule.r_doubleres) {
			GFX_Scale2x(screen_surface, display_surface);
		} else {
			GFX_SimpleBlit(screen_surface, display_surface);
		}
		break;
	}

	SYSGFX_UnlockSurface(display_surface);
	SYSGFX_UnlockSurface(screen_surface);

	if (surface_converted) {
		SYSGFX_DestroySurface(display_surface);
	}

	// FIXME
	SDL_UpdateRect((SDL_Surface *)screen_surface->impl_src, 0, 0, 0, 0);

	return R_SUCCESS;
}

unsigned int RENDER_GetFrameCount() {
	return RenderModule.r_framecount;
}

unsigned int RENDER_ResetFrameCount() {
	unsigned int framecount = RenderModule.r_framecount;

	RenderModule.r_framecount = 0;

	return framecount;
}

void RENDER_FpsTimer(unsigned long interval, void *param) {
	YS_IGNORE_PARAM(interval);
	YS_IGNORE_PARAM(param);

	RenderModule.r_fps = RenderModule.r_framecount;
	RenderModule.r_framecount = 0;

	return;
}

void RENDER_ConvertMousePt(R_POINT *mouse_pt) {
	assert(mouse_pt != NULL);

	if (RenderModule.r_doubleres) {

		mouse_pt->x /= 2;
		mouse_pt->y /= 2;
	}
}

unsigned int RENDER_GetFlags() {
	return RenderModule.r_flags;
}

void RENDER_SetFlag(unsigned int flag) {
	RenderModule.r_flags |= flag;
}

void RENDER_ToggleFlag(unsigned int flag) {
	RenderModule.r_flags ^= flag;
}

int RENDER_SetMode(int mode) {
	switch (mode) {
	case RM_SCANLINES:
		if (!RenderModule.r_doubleres) {
			return R_FAILURE;
		}
		break;
	case RM_2XSAI:
		if (!RenderModule.r_doubleres || !RenderModule.r_hicolor) {
			return R_FAILURE;
		}
		break;
	case RM_SUPER2XSAI:
		if (!RenderModule.r_doubleres || !RenderModule.r_hicolor) {
			return R_FAILURE;
		}
		break;
	case RM_SUPEREAGLE:
		if (!RenderModule.r_doubleres || !RenderModule.r_hicolor) {
			return R_FAILURE;
		}
		break;
	default:
		break;
	}

	RenderModule.r_mode = mode;

	return R_SUCCESS;
}

int RENDER_GetBufferInfo(R_BUFFER_INFO *r_bufinfo) {
	assert(r_bufinfo != NULL);

	r_bufinfo->r_bg_buf = RenderModule.r_bg_buf;
	r_bufinfo->r_bg_buf_w = RenderModule.r_bg_buf_w;
	r_bufinfo->r_bg_buf_h = RenderModule.r_bg_buf_h;

	r_bufinfo->r_tmp_buf = RenderModule.r_tmp_buf;
	r_bufinfo->r_tmp_buf_w = RenderModule.r_tmp_buf_w;
	r_bufinfo->r_tmp_buf_h = RenderModule.r_tmp_buf_h;

	return R_SUCCESS;
}

} // End of namespace Saga
