/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

// Game interface module private header file

#ifndef SAGA_INTERFACE_H__
#define SAGA_INTERFACE_H__

#include "saga/sprite.h"
#include "saga/script.h"

namespace Saga {

enum INTERFACE_UPDATE_FLAGS {
	UPDATE_MOUSEMOVE = 1,
	UPDATE_MOUSECLICK
};

#define ITE_INVENTORY_SIZE 24

#define VERB_STRLIMIT 32

#define STATUS_TEXT_LEN 128

#define COMMAND_DEFAULT_BUTTON 1

// Inherit the Earth interface values
#define ITE_STATUS_Y      137
#define ITE_STATUS_W      320
#define ITE_STATUS_H      12
#define ITE_STATUS_TEXT_Y 2
#define ITE_STATUS_TXTCOL 186
#define ITE_STATUS_BGCOL  15

#define ITE_CMD_TEXT_COL       147
#define ITE_CMD_TEXT_SHADOWCOL 15
#define ITE_CMD_TEXT_HILITECOL 96

#define ITE_LPORTRAIT_X 5
#define ITE_LPORTRAIT_Y 4
#define ITE_RPORTRAIT_X 274
#define ITE_RPORTRAIT_Y 4

#define ITE_INVENTORY_XSTART 181
#define ITE_INVENTORY_YSTART 155
#define ITE_INVENTORY_ROWS 2
#define ITE_INVENTORY_COLUMNS 4
#define ITE_INVENTORY_ICON_WIDTH 29
#define ITE_INVENTORY_ICON_HEIGHT 20
#define ITE_INVENTORY_ICON_XOFFSET 1
#define ITE_INVENTORY_ICON_YOFFSET 0
#define ITE_INVENTORY_XSPACING 3
#define ITE_INVENTORY_YSPACING 1

// IHNMAIMS interface values
#define IHNM_STATUS_Y      304
#define IHNM_STATUS_W      640
#define IHNM_STATUS_H      24
#define IHNM_STATUS_TEXT_Y 8
#define IHNM_STATUS_TXTCOL 186
#define IHNM_STATUS_BGCOL  11

#define IHNM_CMD_TEXT_COL       147
#define IHNM_CMD_TEXT_SHADOWCOL 15
#define IHNM_CMD_TEXT_HILITECOL 96

#define IHNM_LPORTRAIT_X 5
#define IHNM_LPORTRAIT_Y 4
#define IHNM_RPORTRAIT_X -1
#define IHNM_RPORTRAIT_Y -1

// TODO
#define IHNM_INVENTORY_XSTART 0
#define IHNM_INVENTORY_YSTART 0
#define IHNM_INVENTORY_ROWS 0
#define IHNM_INVENTORY_COLUMNS 0
#define IHNM_INVENTORY_ICON_WIDTH 0
#define IHNM_INVENTORY_ICON_HEIGHT 0
#define IHNM_INVENTORY_ICON_XOFFSET 0
#define IHNM_INVENTORY_ICON_YOFFSET 0
#define IHNM_INVENTORY_XSPACING 0
#define IHNM_INVENTORY_YSPACING 0

enum PANEL_MODES {
	kPanelNull,
	kPanelMain,
	kPanelOption,
	kPanelTextBox,
	kPanelQuit,
	kPanelError,
	kPanelLoad,
	kPanelConverse,
	kPanelProtect,
	kPanelPlacard,
	kPanelMap,
	kPanelInventory,
	kPanelFade
};

enum BUTTON_FLAGS {
	BUTTON_NONE = 0x0,
	BUTTON_LABEL = 0x01,
	BUTTON_BITMAP = 0x02,
	BUTTON_SET = 0x04
};

#define BUTTON_VERB ( BUTTON_LABEL | BUTTON_BITMAP | BUTTON_SET )

struct InterfaceButton {
	int x1;
	int y1;
	int x2;
	int y2;
	const char *label;
	int inactive_sprite;
	int active_sprite;
	int flags;
	int data;
};

struct InterfacePanel {
	byte *res;
	size_t res_len;
	int x;
	int y;
	byte *img;
	size_t img_len;
	int img_w;
	int img_h;
	int set_button;
	int nbuttons;
	InterfaceButton *buttons;
	SpriteList sprites;
};

struct INTERFACE_DESC {
	int status_y;
	int status_w;
	int status_h;
	int status_txt_y;
	int status_txt_col;
	int status_bgcol;
	int cmd_txt_col;
	int cmd_txt_shadowcol;
	int cmd_txt_hilitecol;
	int cmd_defaultbutton;
	int lportrait_x;
	int lportrait_y;
	int rportrait_x;
	int rportrait_y;
	int inv_xstart;
	int inv_ystart;
	int inv_rows;
	int inv_columns;
	int inv_icon_width;
	int inv_icon_height;
	int inv_icon_xoffset;
	int inv_icon_yoffset;
	int inv_xspacing;
	int inv_yspacing;
};

struct INTERFACE_MODULE {
};

enum INTERFACE_VERBS {
	I_VERB_WALKTO,
	I_VERB_LOOKAT,
	I_VERB_PICKUP,
	I_VERB_TALKTO,
	I_VERB_OPEN,
	I_VERB_CLOSE,
	I_VERB_USE,
	I_VERB_GIVE
};

struct VERB_DATA {
	int i_verb;
	const char *verb_cvar;
	char verb_str[VERB_STRLIMIT];
	int s_verb;
};

class Interface {
public:
	Interface(SagaEngine *vm);
	~Interface(void);

	int registerLang();
	int activate();
	int deactivate();
	int setMode(int mode, bool force = false);
	int getMode(void) { return _panelMode; }
	void rememberMode();
	void restoreMode();
	void lockMode() { _lockedMode = _panelMode; }
	void unlockMode() { _panelMode = _lockedMode; }
	bool isInMainMode() { return _inMainMode; }
	int setStatusText(const char *new_txt);
	int loadScenePortraits(int resourceId);
	int setLeftPortrait(int portrait);
	int setRightPortrait(int portrait);
	int draw();
	int drawStatusBar(SURFACE *ds);
	int update(const Point& imousePt, int update_flag);

	void addToInventory(int sprite);
	void removeFromInventory(int sprite);
	void drawInventory();
	int inventoryTest(const Point& imousePt, int *ibutton);

private:
	int hitTest(const Point& imousePt, int *ibutton);
	int handleCommandUpdate(SURFACE *ds, const Point& imousePt);
	int handleCommandClick(SURFACE *ds, const Point& imousePt);
	int handlePlayfieldUpdate(SURFACE *ds, const Point& imousePt);
	int handlePlayfieldClick(SURFACE *ds, const Point& imousePt);
	void drawVerb(int verb, int state);

private:
	SagaEngine *_vm;

	bool _initialized;
	bool _active;
	RSCFILE_CONTEXT *_interfaceContext;
	INTERFACE_DESC _iDesc;
	int _panelMode;
	int _savedMode;
	int _lockedMode;
	bool _inMainMode;
	InterfacePanel _mainPanel;
	InterfacePanel _conversePanel;
	char _statusText[STATUS_TEXT_LEN];
	int _leftPortrait;
	int _rightPortrait;
	SpriteList _defPortraits;
	SpriteList _scenePortraits;
	int _activeVerb;
	SCRIPT_THREAD *_iThread;

	uint16 *_inventory;
	int _inventorySize;
	byte _inventoryCount;
};

} // End of namespace Saga

#endif				/* INTERFACE_H__ */
/* end "r_interface.h" */
