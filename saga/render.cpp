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
#include "saga.h"
#include "reinherit.h"

#include "systimer.h"
#include "yslib.h"

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
static OSystem *_system;

const char *test_txt = "The quick brown fox jumped over the lazy dog. She sells sea shells down by the sea shore.";

int RENDER_Register() {
	// Register "r_softcursor" cfg cvar
	RenderModule.r_softcursor = R_SOFTCURSOR_DEFAULT;

	if (CVAR_Register_I(&RenderModule.r_softcursor,
		"r_softcursor", NULL, R_CVAR_CFG, 0, 1) != R_SUCCESS) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int RENDER_Init(OSystem *system) {
	R_GAME_DISPLAYINFO disp_info;
	int result;
	int tmp_w, tmp_h, tmp_bytepp;

	// Initialize system graphics
	GAME_GetDisplayInfo(&disp_info);

	if (SYSGFX_Init(system, disp_info.logical_w, disp_info.logical_h) != R_SUCCESS) {
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

	RenderModule.r_tmp_buf = (byte *)calloc(1, tmp_w * tmp_h * tmp_bytepp);
	if (RenderModule.r_tmp_buf == NULL) {

		free(RenderModule.r_bg_buf);
		return R_MEM;
	}

	RenderModule.r_tmp_buf_w = tmp_w;
	RenderModule.r_tmp_buf_h = tmp_h;

	RenderModule.r_backbuf_surface = SYSGFX_GetBackBuffer();

	// Initialize cursor state
	if (RenderModule.r_softcursor) {
		SYSINPUT_HideMouse();
	}

	_system = system;
	RenderModule.initialized = 1;

	return R_SUCCESS;
}

int RENDER_DrawScene() {
	R_SURFACE *backbuf_surface;
	R_GAME_DISPLAYINFO disp_info;
	R_SCENE_INFO scene_info;
	SCENE_BGINFO bg_info;
	R_POINT bg_pt;
	char txt_buf[20];
	int fps_width;
	R_POINT mouse_pt;
	int mouse_x, mouse_y;

	if (!RenderModule.initialized) {
		return R_FAILURE;
	}

	RenderModule.r_framecount++;

	backbuf_surface = RenderModule.r_backbuf_surface;

	// Get mouse coordinates
	SYSINPUT_GetMousePos(&mouse_x, &mouse_y);

	mouse_pt.x = mouse_x;
	mouse_pt.y = mouse_y;

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

	_system->copyRectToScreen(backbuf_surface->buf, backbuf_surface->buf_w, 0, 0, 
							  backbuf_surface->buf_w, backbuf_surface->buf_h);

	_system->updateScreen();
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

unsigned int RENDER_GetFlags() {
	return RenderModule.r_flags;
}

void RENDER_SetFlag(unsigned int flag) {
	RenderModule.r_flags |= flag;
}

void RENDER_ToggleFlag(unsigned int flag) {
	RenderModule.r_flags ^= flag;
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
