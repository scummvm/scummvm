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

// Game interface module
#include "saga.h"

#include "gfx.h"
#include "game_mod.h"
#include "cvar_mod.h"
#include "actor_mod.h"
#include "console_mod.h"
#include "font_mod.h"
#include "objectmap.h"
#include "rscfile_mod.h"
#include "script_mod.h"
#include "sprite_mod.h"

#include "interface_mod.h"
#include "interface.h"
#include "sdata.h"

namespace Saga {

static R_INTERFACE_MODULE IfModule;

static R_VERB_DATA I_VerbData[] = {
	{I_VERB_WALKTO, "verb_walkto", "Walk to", S_VERB_WALKTO},
	{I_VERB_LOOKAT, "verb_lookat", "Look at", S_VERB_LOOKAT},
	{I_VERB_PICKUP, "verb_pickup", "Pick up", S_VERB_PICKUP},
	{I_VERB_TALKTO, "verb_talkto", "Talk to", S_VERB_TALKTO},
	{I_VERB_OPEN, "verb_open", "Open", S_VERB_OPEN},
	{I_VERB_CLOSE, "verb_close", "Close", S_VERB_CLOSE},
	{I_VERB_USE, "verb_use", "Use", S_VERB_USE},
	{I_VERB_GIVE, "verb_give", "Give", S_VERB_GIVE}
};

static R_INTERFACE_DESC ITE_interface = {
	ITE_STATUS_Y,
	ITE_STATUS_W,
	ITE_STATUS_H,
	ITE_STATUS_TEXT_Y,
	ITE_STATUS_TXTCOL,
	ITE_STATUS_BGCOL,

	ITE_CMD_TEXT_COL,
	ITE_CMD_TEXT_SHADOWCOL,
	ITE_CMD_TEXT_HILITECOL,

	COMMAND_DEFAULT_BUTTON,

	ITE_LPORTRAIT_X,
	ITE_LPORTRAIT_Y
};

static R_INTERFACE_BUTTON ITE_c_buttons[] = {
	{5, 4, 46, 47, "Portrait", 0, 0, BUTTON_NONE, 0},
	// "Walk To" and "Talk To" share button sprites
	{52, 4, 109, 14, "Walk To", 1, 2, BUTTON_VERB, I_VERB_WALKTO},
	{52, 15, 109, 25, "Look At", 3, 4, BUTTON_VERB, I_VERB_LOOKAT},
	{52, 26, 109, 36, "Pick Up", 5, 6, BUTTON_VERB, I_VERB_PICKUP},
	{52, 37, 109, 47, "Talk To", 1, 2, BUTTON_VERB, I_VERB_TALKTO},
	{110, 4, 166, 14, "Open", 7, 8, BUTTON_VERB, I_VERB_OPEN},
	{110, 15, 166, 25, "Close", 9, 10, BUTTON_VERB, I_VERB_CLOSE},
	{110, 26, 166, 36, "Use", 11, 12, BUTTON_VERB, I_VERB_USE},
	{110, 37, 166, 47, "Give", 13, 14, BUTTON_VERB, I_VERB_GIVE},

	{181, 6, 206, 24, "Inv1", 0, 0, BUTTON_NONE, 0},
	{213, 6, 240, 24, "Inv2", 0, 0, BUTTON_NONE, 0},
	{245, 6, 272, 24, "Inv3", 0, 0, BUTTON_NONE, 0},
	{277, 6, 304, 24, "Inv4", 0, 0, BUTTON_NONE, 0},
	{181, 27, 208, 45, "Inv5", 0, 0, BUTTON_NONE, 0},
	{213, 27, 240, 45, "Inv6", 0, 0, BUTTON_NONE, 0},
	{245, 27, 272, 45, "Inv7", 0, 0, BUTTON_NONE, 0},
	{277, 27, 304, 45, "Inv8", 0, 0, BUTTON_NONE, 0},
	{306, 6, 314, 11, "InvUp", 0, 0, BUTTON_NONE, 0},
	{306, 41, 314, 45, "InvDown", 0, 0, BUTTON_NONE, 0}
};

static R_INTERFACE_DESC IHNM_interface = {
	IHNM_STATUS_Y,
	IHNM_STATUS_W,
	IHNM_STATUS_H,
	IHNM_STATUS_TEXT_Y,
	IHNM_STATUS_TXTCOL,
	IHNM_STATUS_BGCOL,

	IHNM_CMD_TEXT_COL,
	IHNM_CMD_TEXT_SHADOWCOL,
	IHNM_CMD_TEXT_HILITECOL,

	COMMAND_DEFAULT_BUTTON,

	IHNM_LPORTRAIT_X,
	IHNM_LPORTRAIT_Y
};

static R_INTERFACE_BUTTON IHNM_c_buttons[] = {
	{5, 4, 46, 47, "Portrait", 0, 0, 0, 0}
};

int INTERFACE_RegisterLang(void) {
	size_t i;

	for (i = 0; i < ARRAYSIZE(I_VerbData); i++) {
		if (CVAR_Register_S(I_VerbData[i].verb_str,
			I_VerbData[i].verb_cvar,
			NULL, R_CVAR_CFG, R_VERB_STRLIMIT) != R_SUCCESS) {

			return R_FAILURE;
		}

		assert(CVAR_Find(I_VerbData[i].verb_cvar) != NULL);
	}

	return R_SUCCESS;
}

int INTERFACE_Init(void) {
	R_GAME_RESOURCEDESC g_resdesc;

	int game_type;
	int result;

	if (IfModule.init) {
		return R_FAILURE;
	}

	IfModule.i_thread = STHREAD_Create();
	if (IfModule.i_thread == NULL) {
		warning("Error creating script thread for game interface module");
		return R_FAILURE;
	}

	// Load interface module resource file context
	result = GAME_GetFileContext(&IfModule.i_file_ctxt, R_GAME_RESOURCEFILE, 0);
	if (result != R_SUCCESS) {
		return R_FAILURE;
	}

	// Initialize interface data by game type
	game_type = GAME_GetGameType();
	if (game_type == R_GAMETYPE_ITE) {
		// Load Inherit the Earth interface desc
		IfModule.c_panel.buttons = ITE_c_buttons;
		IfModule.c_panel.nbuttons = ARRAYSIZE(ITE_c_buttons);

		IfModule.i_desc = ITE_interface;
	} else if (game_type == R_GAMETYPE_IHNM) {
		// Load I Have No Mouth interface desc
		IfModule.c_panel.buttons = IHNM_c_buttons;
		IfModule.c_panel.nbuttons = ARRAYSIZE(IHNM_c_buttons);
		IfModule.i_desc = IHNM_interface;
	} else {
		return R_FAILURE;
	}

	// Load interface resources
	GAME_GetResourceInfo(&g_resdesc);

	// Load command panel resource
	result = RSC_LoadResource(IfModule.i_file_ctxt, g_resdesc.command_panel_rn,
							&IfModule.c_panel.res, &IfModule.c_panel.res_len);
	if (result != R_SUCCESS) {
		return R_FAILURE;
	}

	// Load dialogue panel resource
	result = RSC_LoadResource(IfModule.i_file_ctxt, g_resdesc.dialogue_panel_rn,
							&IfModule.d_panel.res, &IfModule.d_panel.res_len);
	if (result != R_SUCCESS) {
		return R_FAILURE;
	}

	SPRITE_LoadList(ITE_COMMAND_BUTTONSPRITES, &IfModule.c_panel.sprites);

	SPRITE_LoadList(ITE_DEFAULT_PORTRAITS, &IfModule.def_portraits);

	_vm->decodeBGImage(IfModule.c_panel.res, IfModule.c_panel.res_len, &IfModule.c_panel.img,
					&IfModule.c_panel.img_len, &IfModule.c_panel.img_w, &IfModule.c_panel.img_h);

	_vm->decodeBGImage(IfModule.d_panel.res, IfModule.d_panel.res_len,
					&IfModule.d_panel.img, &IfModule.d_panel.img_len,
					&IfModule.d_panel.img_w, &IfModule.d_panel.img_h);

	IfModule.c_panel.x = 0;
	IfModule.c_panel.y = 149;

	IfModule.d_panel.x = 0;
	IfModule.d_panel.y = 149;

	IfModule.c_panel.set_button = COMMAND_DEFAULT_BUTTON;
	IfModule.active_portrait = 0;

	IfModule.active_verb = I_VERB_WALKTO;

	IfModule.init = 1;

	return R_SUCCESS;
}

int INTERFACE_Activate() {
	IfModule.active = 1;
	INTERFACE_Draw();

	return R_SUCCESS;
}

int INTERFACE_Deactivate() {
	IfModule.active = 0;

	return R_SUCCESS;
}

int INTERFACE_SetStatusText(const char *new_txt) {
	assert(new_txt != NULL);

	strncpy(IfModule.status_txt, new_txt, R_STATUS_TEXT_LEN);

	return R_SUCCESS;
}

int INTERFACE_Draw() {
	R_GAME_DISPLAYINFO g_di;
	R_SURFACE *back_buf;

	int xbase;
	int ybase;
	int lportrait_x;
	int lportrait_y;

	R_RECT rect;
	R_POINT origin;

	back_buf = _vm->_gfx->getBackBuffer();

	if (!IfModule.active) {
		return R_SUCCESS;
	}

	// Get game display info
	GAME_GetDisplayInfo(&g_di);

	// Erase background of status bar
	rect.left = 0;
	rect.top = IfModule.i_desc.status_h - 1;

	rect.right = g_di.logical_w - 1;
	rect.bottom = IfModule.i_desc.status_y;

	_vm->_gfx->drawRect(back_buf, &rect, IfModule.i_desc.status_bgcol);

	// Draw command panel background
	if (IfModule.panel_mode == PANEL_COMMAND) {
		xbase = IfModule.c_panel.x;
		ybase = IfModule.c_panel.y;

		origin.x = 0;
		origin.y = g_di.logical_h - IfModule.c_panel.img_h;

		_vm->_gfx->bufToSurface(back_buf, IfModule.c_panel.img, IfModule.c_panel.img_w,
						IfModule.c_panel.img_h, NULL, &origin);
	} else {
		xbase = IfModule.d_panel.x;
		ybase = IfModule.d_panel.y;

		origin.x = 0;
		origin.y = g_di.logical_h - IfModule.c_panel.img_h;

		_vm->_gfx->bufToSurface(back_buf, IfModule.d_panel.img, IfModule.d_panel.img_w,
						IfModule.d_panel.img_h, NULL, &origin);
	}

	// Draw character portrait
	lportrait_x = xbase + IfModule.i_desc.lportrait_x;
	lportrait_y = ybase + IfModule.i_desc.lportrait_y;

	SPRITE_Draw(back_buf, IfModule.def_portraits, IfModule.active_portrait, lportrait_x, lportrait_y);

	return R_SUCCESS;
}

int INTERFACE_Update(R_POINT *imouse_pt, int update_flag) {
	R_GAME_DISPLAYINFO g_di;

	R_SURFACE *back_buf;

	int imouse_x, imouse_y;

	assert(imouse_pt != NULL);

	if (!IfModule.active) {
		return R_SUCCESS;
	}

	imouse_x = imouse_pt->x;
	imouse_y = imouse_pt->y;

	back_buf = _vm->_gfx->getBackBuffer();

	// Get game display info
	GAME_GetDisplayInfo(&g_di);

	// Update playfield space ( only if cursor is inside )
	if (imouse_y < g_di.scene_h) {
		// Mouse is in playfield space
		if (update_flag == UPDATE_MOUSEMOVE) {
			HandlePlayfieldUpdate(back_buf, imouse_pt);
		} else if (update_flag == UPDATE_MOUSECLICK) {
			HandlePlayfieldClick(back_buf, imouse_pt);
		}
	}

	// Update command space
	if (update_flag == UPDATE_MOUSEMOVE) {
		HandleCommandUpdate(back_buf, imouse_pt);
	} else if (update_flag == UPDATE_MOUSECLICK) {
		HandleCommandClick(back_buf, imouse_pt);
	}

	DrawStatusBar(back_buf);

	return R_SUCCESS;
}

int DrawStatusBar(R_SURFACE *ds) {
	R_GAME_DISPLAYINFO g_di;
	R_RECT rect;

	int string_w;

	// Get game display info
	GAME_GetDisplayInfo(&g_di);

	// Erase background of status bar
	rect.left = 0;
	rect.top = IfModule.i_desc.status_y;
	rect.right = g_di.logical_w - 1;
	rect.bottom = IfModule.i_desc.status_y + IfModule.i_desc.status_h - 1;

	_vm->_gfx->drawRect(ds, &rect, IfModule.i_desc.status_bgcol);

	string_w = FONT_GetStringWidth(SMALL_FONT_ID, IfModule.status_txt, 0, 0);

	FONT_Draw(SMALL_FONT_ID, ds, IfModule.status_txt, 0, (IfModule.i_desc.status_w / 2) - (string_w / 2),
			IfModule.i_desc.status_y + IfModule.i_desc.status_txt_y, IfModule.i_desc.status_txt_col, 0, 0);

	return R_SUCCESS;
}

int HandleCommandClick(R_SURFACE *ds, R_POINT *imouse_pt) {
	int hit_button;
	int ibutton_num;

	int x_base;
	int y_base;

	int button_x = 0;
	int button_y = 0;

	int old_set_button;
	int set_button;

	hit_button = INTERFACE_HitTest(imouse_pt, &ibutton_num);
	if (hit_button != R_SUCCESS) {
		// Clicking somewhere other than a button doesn't do anything
		return R_SUCCESS;
	}

	x_base = IfModule.c_panel.x;
	y_base = IfModule.c_panel.y;

	if (IfModule.c_panel.buttons[ibutton_num].flags & BUTTON_SET) {
		old_set_button = IfModule.c_panel.set_button;
		set_button = ibutton_num;
		IfModule.c_panel.set_button = set_button;

		if (IfModule.c_panel.buttons[set_button].flags & BUTTON_VERB) {
			IfModule.active_verb = IfModule.c_panel.buttons[ibutton_num].data;
		}

		if (IfModule.c_panel.buttons[set_button].flags & BUTTON_BITMAP) {
			button_x = x_base + IfModule.c_panel.buttons[set_button].x1;
			button_y = y_base + IfModule.c_panel.buttons[set_button].y1;

			SPRITE_Draw(ds, IfModule.c_panel.sprites, IfModule.c_panel.buttons[set_button].
						active_sprite - 1, button_x, button_y);
		}

		if (IfModule.c_panel.buttons[old_set_button].flags & BUTTON_BITMAP) {
			button_x = x_base + IfModule.c_panel.buttons[old_set_button].x1;
			button_y = y_base + IfModule.c_panel.buttons[old_set_button].y1;

			SPRITE_Draw(ds, IfModule.c_panel.sprites, IfModule.c_panel.buttons[old_set_button].
						inactive_sprite - 1, button_x, button_y);
		}
	}

	return R_SUCCESS;
}

int HandleCommandUpdate(R_SURFACE *ds, R_POINT *imouse_pt) {
	int hit_button;
	int ibutton_num;

	int button_x = 0;
	int button_y = 0;
	int button_w = 0;

	int verb_idx = 0;
	int string_w = 0;

	int color;
	int i;

	hit_button = INTERFACE_HitTest(imouse_pt, &ibutton_num);

	if (hit_button == R_SUCCESS) {
		// Hovering over a command panel button
		INTERFACE_SetStatusText(I_VerbData[IfModule.active_verb].verb_str);
	}

	for (i = 0; i < IfModule.c_panel.nbuttons; i++) {
		if (!(IfModule.c_panel.buttons[i].flags & BUTTON_LABEL)) {
			continue;
		}

		button_w = IfModule.c_panel.buttons[i].x2 - IfModule.c_panel.buttons[i].x1;

		verb_idx = IfModule.c_panel.buttons[i].data;

		string_w = FONT_GetStringWidth(SMALL_FONT_ID, I_VerbData[verb_idx].verb_str, 0, 0);

		if (i == hit_button) {
			color = IfModule.i_desc.cmd_txt_hilitecol;
		} else {
			color = IfModule.i_desc.cmd_txt_col;
		}

		button_x = IfModule.c_panel.x + IfModule.c_panel.buttons[i].x1;
		button_y = IfModule.c_panel.y + IfModule.c_panel.buttons[i].y1;

		FONT_Draw(SMALL_FONT_ID, ds, I_VerbData[verb_idx].verb_str, 0,
				button_x + ((button_w / 2) - (string_w / 2)), button_y + 1,
				color, IfModule.i_desc.cmd_txt_shadowcol, FONT_SHADOW);

		if ((i == IfModule.c_panel.set_button) && (IfModule.c_panel.buttons[i].flags & BUTTON_BITMAP)) {
			SPRITE_Draw(ds, IfModule.c_panel.sprites, IfModule.c_panel.buttons[i].active_sprite - 1,
						button_x, button_y);
		}
	}

	return R_SUCCESS;
}

int HandlePlayfieldClick(R_SURFACE *ds, R_POINT *imouse_pt) {
	int hit_object;
	int object_num;
	uint16 object_flags = 0;

	int script_num;
	R_POINT iactor_pt;

	hit_object = _vm->_objectMap->hitTest(imouse_pt, &object_num);

	if (hit_object != R_SUCCESS) {
		// Player clicked on empty spot - walk here regardless of verb
		ACTOR_StoA(&iactor_pt, imouse_pt);
		ACTOR_WalkTo(0, &iactor_pt, 0, NULL);
		return R_SUCCESS;
	}

	if (_vm->_objectMap->getFlags(object_num, &object_flags) != R_SUCCESS) {
		CON_Print("Invalid object number: %d\n", object_num);
		return R_FAILURE;
	}

	if (object_flags & R_OBJECT_NORMAL) {
		if (_vm->_objectMap->getEPNum(object_num, &script_num) == R_SUCCESS) {
			// Set active verb in script module
			_vm->_sdata->putWord(4, 4, I_VerbData[IfModule.active_verb].s_verb);

			// Execute object script if present
			if (script_num != 0) {
				STHREAD_Execute(IfModule.i_thread, script_num);
			}
		}
	} else {
		// Not a normal scene object - walk to it as if it weren't there
		ACTOR_StoA(&iactor_pt, imouse_pt);
		ACTOR_WalkTo(0, &iactor_pt, 0, NULL);
	}

	return R_SUCCESS;
}

int HandlePlayfieldUpdate(R_SURFACE *ds, R_POINT *imouse_pt) {
	const char *object_name;
	int object_num;
	uint16 object_flags = 0;

	char new_status[R_STATUS_TEXT_LEN];

	int hit_object;

	new_status[0] = 0;

	hit_object = _vm->_objectMap->hitTest(imouse_pt, &object_num);

	if (hit_object != R_SUCCESS) {
		// Cursor over nothing - just display current verb
		INTERFACE_SetStatusText(I_VerbData[IfModule.active_verb].verb_str);
		return R_SUCCESS;
	}

	if (_vm->_objectMap->getFlags(object_num, &object_flags) != R_SUCCESS) {
		CON_Print("Invalid object number: %d\n", object_num);
		return R_FAILURE;
	}

	_vm->_objectMap->getName(object_num, &object_name);

	if (object_flags & R_OBJECT_NORMAL) {
		// Normal scene object - display as subject of verb
		snprintf(new_status, R_STATUS_TEXT_LEN, "%s %s", I_VerbData[IfModule.active_verb].verb_str, object_name);
	} else {
		// Not normal scene object - override verb as we can only
		// walk to this object
		snprintf(new_status, R_STATUS_TEXT_LEN, "%s %s", I_VerbData[I_VERB_WALKTO].verb_str, object_name);
	}

	INTERFACE_SetStatusText(new_status);

	return R_SUCCESS;
}

int INTERFACE_HitTest(R_POINT *imouse_pt, int *ibutton) {
	R_INTERFACE_BUTTON *buttons;

	int nbuttons;
	int xbase;
	int ybase;

	int i;

	buttons = IfModule.c_panel.buttons;
	nbuttons = IfModule.c_panel.nbuttons;

	xbase = IfModule.c_panel.x;
	ybase = IfModule.c_panel.y;

	for (i = 0; i < nbuttons; i++) {
		if ((imouse_pt->x >= (xbase + buttons[i].x1)) && (imouse_pt->x < (xbase + buttons[i].x2)) &&
			(imouse_pt->y >= (ybase + buttons[i].y1)) && (imouse_pt->y < (ybase + buttons[i].y2))) {
			*ibutton = i;
			return R_SUCCESS;
		}
	}

	*ibutton = -1;
	return R_FAILURE;
}

int INTERFACE_Shutdown(void) {
	if (!IfModule.init) {
		return R_FAILURE;
	}

	IfModule.init = 0;

	return R_SUCCESS;
}

} // End of namespace Saga
