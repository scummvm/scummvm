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
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/game_mod.h"
#include "saga/cvar_mod.h"
#include "saga/actor.h"
#include "saga/console.h"
#include "saga/font.h"
#include "saga/objectmap.h"
#include "saga/rscfile_mod.h"
#include "saga/scene.h"
#include "saga/script.h"
#include "saga/sprite.h"

#include "saga/interface.h"
#include "saga/sdata.h"

namespace Saga {

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
	ITE_LPORTRAIT_Y,
	ITE_RPORTRAIT_X,
	ITE_RPORTRAIT_Y
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
	IHNM_LPORTRAIT_Y,
	IHNM_RPORTRAIT_X,
	IHNM_RPORTRAIT_Y
};

static R_INTERFACE_BUTTON IHNM_c_buttons[] = {
	{5, 4, 46, 47, "Portrait", 0, 0, 0, 0}
};

int Interface::registerLang(void) {
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

Interface::Interface(SagaEngine *vm) : _vm(vm), _initialized(false) {
	R_GAME_RESOURCEDESC g_resdesc;

	int game_type;
	int result;

	if (_initialized) {
		return;
	}

	_iThread = _vm->_script->SThreadCreate();
	if (_iThread == NULL) {
		warning("Interface::Interface(): Error creating script thread for game interface module");
		return;
	}

	// Load interface module resource file context
	result = GAME_GetFileContext(&_interfaceContext, R_GAME_RESOURCEFILE, 0);
	if (result != R_SUCCESS) {
		return;
	}

	// Initialize interface data by game type
	game_type = GAME_GetGameType();
	if (game_type == GID_ITE) {
		// Load Inherit the Earth interface desc
		_cPanel.buttons = ITE_c_buttons;
		_cPanel.nbuttons = ARRAYSIZE(ITE_c_buttons);

		_iDesc = ITE_interface;
	} else if (game_type == GID_IHNM) {
		// Load I Have No Mouth interface desc
		_cPanel.buttons = IHNM_c_buttons;
		_cPanel.nbuttons = ARRAYSIZE(IHNM_c_buttons);
		_iDesc = IHNM_interface;
	} else {
		return;
	}

	// Load interface resources
	GAME_GetResourceInfo(&g_resdesc);

	// Load command panel resource
	result = RSC_LoadResource(_interfaceContext, g_resdesc.command_panel_rn,
							&_cPanel.res, &_cPanel.res_len);
	if (result != R_SUCCESS) {
		return;
	}

	// Load dialogue panel resource
	result = RSC_LoadResource(_interfaceContext, g_resdesc.dialogue_panel_rn,
							&_dPanel.res, &_dPanel.res_len);
	if (result != R_SUCCESS) {
		return;
	}

	_vm->_sprite->loadList(ITE_COMMAND_BUTTONSPRITES, &_cPanel.sprites);

	_vm->_sprite->loadList(ITE_DEFAULT_PORTRAITS, &_defPortraits);

	_vm->decodeBGImage(_cPanel.res, _cPanel.res_len, &_cPanel.img,
					&_cPanel.img_len, &_cPanel.img_w, &_cPanel.img_h);

	_vm->decodeBGImage(_dPanel.res, _dPanel.res_len,
					&_dPanel.img, &_dPanel.img_len,
					&_dPanel.img_w, &_dPanel.img_h);

	_cPanel.x = 0;
	_cPanel.y = 149;

	_dPanel.x = 0;
	_dPanel.y = 149;

	_cPanel.set_button = COMMAND_DEFAULT_BUTTON;
	_scenePortraits = 0;
	_leftPortrait = 0;
	_rightPortrait = 0;

	_activeVerb = I_VERB_WALKTO;

	_active = 0;
	_panelMode = PANEL_COMMAND;
	*_statusText = 0;

	_initialized = true;
}

Interface::~Interface(void) {
	_initialized = false;
}

int Interface::activate() {
	_active = 1;
	draw();

	return R_SUCCESS;
}

int Interface::deactivate() {
	_active = 0;

	return R_SUCCESS;
}

int Interface::setMode(R_PANEL_MODES mode) {
	// TODO: Is this where we should hide/show the mouse cursor?

	_panelMode = mode;
	draw();

	return R_SUCCESS;
}

int Interface::setStatusText(const char *new_txt) {
	assert(new_txt != NULL);

	strncpy(_statusText, new_txt, R_STATUS_TEXT_LEN);

	return R_SUCCESS;
}

int Interface::loadScenePortraits(int res) {
	if (_scenePortraits)
		_vm->_sprite->freeSprite(_scenePortraits);

	return _vm->_sprite->loadList(res, &_scenePortraits);
}

int Interface::setLeftPortrait(int portrait) {
	_leftPortrait = portrait;
	draw();

	return R_SUCCESS;
}

int Interface::setRightPortrait(int portrait) {
	_rightPortrait = portrait;
	draw();

	return R_SUCCESS;
}

int Interface::draw() {
	R_GAME_DISPLAYINFO g_di;
	R_SURFACE *back_buf;

	int xbase;
	int ybase;
	int lportrait_x;
	int lportrait_y;
	int rportrait_x;
	int rportrait_y;

	Rect rect;
	Point origin;

	back_buf = _vm->_gfx->getBackBuffer();

	if (!_active) {
		return R_SUCCESS;
	}

	// Get game display info
	GAME_GetDisplayInfo(&g_di);

	// Erase background of status bar
	rect.left = 0;
	rect.top = _iDesc.status_y;
	rect.right = g_di.logical_w;
	rect.bottom = _iDesc.status_y + _iDesc.status_h;

	_vm->_gfx->drawRect(back_buf, &rect, _iDesc.status_bgcol);

	// Draw command panel background
	if (_panelMode == PANEL_COMMAND) {
		xbase = _cPanel.x;
		ybase = _cPanel.y;

		origin.x = 0;
		origin.y = g_di.logical_h - _cPanel.img_h;

		_vm->_gfx->bufToSurface(back_buf, _cPanel.img, _cPanel.img_w,
						_cPanel.img_h, NULL, &origin);
	} else {
		xbase = _dPanel.x;
		ybase = _dPanel.y;

		origin.x = 0;
		origin.y = g_di.logical_h - _cPanel.img_h;

		_vm->_gfx->bufToSurface(back_buf, _dPanel.img, _dPanel.img_w,
						_dPanel.img_h, NULL, &origin);
	}

	// Draw character portrait
	lportrait_x = xbase + _iDesc.lportrait_x;
	lportrait_y = ybase + _iDesc.lportrait_y;

	_vm->_sprite->draw(back_buf, _defPortraits, _leftPortrait, lportrait_x, lportrait_y);

	if (_panelMode == PANEL_DIALOGUE && _iDesc.rportrait_x >= 0) {
		rportrait_x = xbase + _iDesc.rportrait_x;
		rportrait_y = ybase + _iDesc.rportrait_y;

		_vm->_sprite->draw(back_buf, _scenePortraits, _rightPortrait, rportrait_x, rportrait_y);
	}

	return R_SUCCESS;
}

int Interface::update(const Point& imousePt, int update_flag) {
	R_GAME_DISPLAYINFO g_di;

	R_SURFACE *back_buf;

	int imouse_x, imouse_y;

	if (!_active) {
		return R_SUCCESS;
	}

	imouse_x = imousePt.x;
	imouse_y = imousePt.y;

	back_buf = _vm->_gfx->getBackBuffer();

	// Get game display info
	GAME_GetDisplayInfo(&g_di);

	if (_panelMode == PANEL_COMMAND) {
		// Update playfield space ( only if cursor is inside )
		if (imouse_y < g_di.scene_h) {
			// Mouse is in playfield space
			if (update_flag == UPDATE_MOUSEMOVE) {
				handlePlayfieldUpdate(back_buf, imousePt);
			} else if (update_flag == UPDATE_MOUSECLICK) {
				handlePlayfieldClick(back_buf, imousePt);
			}
		}

		// Update command space
		if (update_flag == UPDATE_MOUSEMOVE) {
			handleCommandUpdate(back_buf, imousePt);
		} else if (update_flag == UPDATE_MOUSECLICK) {
			handleCommandClick(back_buf, imousePt);
		}
	}

	drawStatusBar(back_buf);

	return R_SUCCESS;
}

int Interface::drawStatusBar(R_SURFACE *ds) {
	R_GAME_DISPLAYINFO g_di;
	Rect rect;

	int string_w;

	// Get game display info
	GAME_GetDisplayInfo(&g_di);

	// Erase background of status bar
	rect.left = 0;
	rect.top = _iDesc.status_y;
	rect.right = g_di.logical_w;
	rect.bottom = _iDesc.status_y + _iDesc.status_h;

	_vm->_gfx->drawRect(ds, &rect, _iDesc.status_bgcol);

	string_w = _vm->_font->getStringWidth(SMALL_FONT_ID, _statusText, 0, 0);

	_vm->_font->draw(SMALL_FONT_ID, ds, _statusText, 0, (_iDesc.status_w / 2) - (string_w / 2),
			_iDesc.status_y + _iDesc.status_txt_y, _iDesc.status_txt_col, 0, 0);

	return R_SUCCESS;
}

int Interface::handleCommandClick(R_SURFACE *ds, const Point& imousePt) {
	int hit_button;
	int ibutton_num;

	int x_base;
	int y_base;

	int button_x = 0;
	int button_y = 0;

	int old_set_button;
	int set_button;

	hit_button = hitTest(imousePt, &ibutton_num);
	if (hit_button != R_SUCCESS) {
		// Clicking somewhere other than a button doesn't do anything
		return R_SUCCESS;
	}

	x_base = _cPanel.x;
	y_base = _cPanel.y;

	if (_cPanel.buttons[ibutton_num].flags & BUTTON_SET) {
		old_set_button = _cPanel.set_button;
		set_button = ibutton_num;
		_cPanel.set_button = set_button;

		if (_cPanel.buttons[set_button].flags & BUTTON_VERB) {
			_activeVerb = _cPanel.buttons[ibutton_num].data;
		}

		if (_cPanel.buttons[set_button].flags & BUTTON_BITMAP) {
			button_x = x_base + _cPanel.buttons[set_button].x1;
			button_y = y_base + _cPanel.buttons[set_button].y1;

			_vm->_sprite->draw(ds, _cPanel.sprites, _cPanel.buttons[set_button].
						active_sprite - 1, button_x, button_y);
		}

		if (_cPanel.buttons[old_set_button].flags & BUTTON_BITMAP) {
			button_x = x_base + _cPanel.buttons[old_set_button].x1;
			button_y = y_base + _cPanel.buttons[old_set_button].y1;

			_vm->_sprite->draw(ds, _cPanel.sprites, _cPanel.buttons[old_set_button].
						inactive_sprite - 1, button_x, button_y);
		}
	}

	return R_SUCCESS;
}

int Interface::handleCommandUpdate(R_SURFACE *ds, const Point& imousePt) {
	int hit_button;
	int ibutton_num;

	int button_x = 0;
	int button_y = 0;
	int button_w = 0;

	int verb_idx = 0;
	int string_w = 0;

	int color;
	int i;

	hit_button = hitTest(imousePt, &ibutton_num);

	if (hit_button == R_SUCCESS) {
		// Hovering over a command panel button
		setStatusText(I_VerbData[_activeVerb].verb_str);
	}

	for (i = 0; i < _cPanel.nbuttons; i++) {
		if (!(_cPanel.buttons[i].flags & BUTTON_LABEL)) {
			continue;
		}

		button_w = _cPanel.buttons[i].x2 - _cPanel.buttons[i].x1;

		verb_idx = _cPanel.buttons[i].data;

		string_w = _vm->_font->getStringWidth(SMALL_FONT_ID, I_VerbData[verb_idx].verb_str, 0, 0);

		if (i == hit_button) {
			color = _iDesc.cmd_txt_hilitecol;
		} else {
			color = _iDesc.cmd_txt_col;
		}

		button_x = _cPanel.x + _cPanel.buttons[i].x1;
		button_y = _cPanel.y + _cPanel.buttons[i].y1;

		_vm->_font->draw(SMALL_FONT_ID, ds, I_VerbData[verb_idx].verb_str, 0,
				button_x + ((button_w / 2) - (string_w / 2)), button_y + 1,
				color, _iDesc.cmd_txt_shadowcol, FONT_SHADOW);

		if ((i == _cPanel.set_button) && (_cPanel.buttons[i].flags & BUTTON_BITMAP)) {
			_vm->_sprite->draw(ds, _cPanel.sprites, _cPanel.buttons[i].active_sprite - 1,
						button_x, button_y);
		}
	}

	return R_SUCCESS;
}

int Interface::handlePlayfieldClick(R_SURFACE *ds, const Point& imousePt) {
	int objectNum;
	uint16 object_flags = 0;

	int script_num;
	Point iactor_pt;

	objectNum = _vm->_scene->_objectMap->hitTest(imousePt);

	if (objectNum == -1) {
		// Player clicked on empty spot - walk here regardless of verb
		_vm->_actor->StoA(&iactor_pt, imousePt);
		_vm->_actor->walkTo(0, &iactor_pt, 0, NULL);
		return R_SUCCESS;
	}

	object_flags = _vm->_scene->_objectMap->getFlags(objectNum);

	if (object_flags & OBJECT_EXIT) { // FIXME. This is wrong
		if ((script_num = _vm->_scene->_objectMap->getEPNum(objectNum)) != -1) {
			// Set active verb in script module
			_vm->_sdata->putWord(4, 4, I_VerbData[_activeVerb].s_verb);

			// Execute object script if present
			if (script_num != 0) {
				_vm->_script->SThreadExecute(_iThread, script_num);
			}
		}
	} else {
		// Not a normal scene object - walk to it as if it weren't there
		_vm->_actor->StoA(&iactor_pt, imousePt);
		_vm->_actor->walkTo(0, &iactor_pt, 0, NULL);
	}

	return R_SUCCESS;
}

int Interface::handlePlayfieldUpdate(R_SURFACE *ds, const Point& imousePt) {
	const char *object_name;
	int objectNum;
	uint16 object_flags = 0;

	char new_status[R_STATUS_TEXT_LEN];

	new_status[0] = 0;

	objectNum = _vm->_scene->_objectMap->hitTest(imousePt);

	if (objectNum == -1) {
		// Cursor over nothing - just display current verb
		setStatusText(I_VerbData[_activeVerb].verb_str);
		return R_SUCCESS;
	}

	object_flags = _vm->_scene->_objectMap->getFlags(objectNum);

	object_name = _vm->_scene->_objectMap->getName(objectNum);

	if (object_flags & OBJECT_EXIT) { // FIXME. This is wrong
		// Normal scene object - display as subject of verb
		snprintf(new_status, R_STATUS_TEXT_LEN, "%s %s", I_VerbData[_activeVerb].verb_str, object_name);
	} else {
		// Not normal scene object - override verb as we can only
		// walk to this object
		snprintf(new_status, R_STATUS_TEXT_LEN, "%s %s", I_VerbData[I_VERB_WALKTO].verb_str, object_name);
	}

	setStatusText(new_status);

	return R_SUCCESS;
}

int Interface::hitTest(const Point& imousePt, int *ibutton) {
	R_INTERFACE_BUTTON *buttons;

	int nbuttons;
	int xbase;
	int ybase;

	int i;

	buttons = _cPanel.buttons;
	nbuttons = _cPanel.nbuttons;

	xbase = _cPanel.x;
	ybase = _cPanel.y;

	for (i = 0; i < nbuttons; i++) {
		if ((imousePt.x >= (xbase + buttons[i].x1)) && (imousePt.x < (xbase + buttons[i].x2)) &&
			(imousePt.y >= (ybase + buttons[i].y1)) && (imousePt.y < (ybase + buttons[i].y2))) {
			*ibutton = i;
			return R_SUCCESS;
		}
	}

	*ibutton = -1;
	return R_FAILURE;
}

} // End of namespace Saga
