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

// Converse-specific stuff
#define CONVERSE_MAX_TEXTS      64
#define CONVERSE_MAX_TEXT_WIDTH (256 - 60)
#define CONVERSE_TEXT_HEIGHT	10
#define CONVERSE_TEXT_LINES     4

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
	BUTTON_SET = 0x04,
	BUTTON_VERB = BUTTON_LABEL | BUTTON_BITMAP | BUTTON_SET
};

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
	int set_button;//TODO: remove
	int nbuttons;
	InterfaceButton *buttons;
	SpriteList sprites;
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

struct Converse {
	char *text;
	int stringNum;
	int textNum;
	int replyId;
	int replyFlags;
	int replyBit;
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

public:
	void converseClear(void);
	bool converseAddText(const char *text, int replyId, byte replyFlags, int replyBit);
	void converseDisplayText(int pos);
	void converseSetTextLines(int row, int textcolor, bool btnDown);
	void converseChangePos(int chg);
	void converseSetPos(void);

private:
	void converseDisplayTextLine(int textcolor, bool btnUp, bool rebuild);


private:
	SagaEngine *_vm;

	bool _initialized;
	bool _active;
	RSCFILE_CONTEXT *_interfaceContext;
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

	Converse _converseText[CONVERSE_MAX_TEXTS];
	int _converseTextCount;
	int _converseStrCount;
	int _converseStartPos;
	int _converseEndPos;
	int _conversePos;

	byte _converseLastColors[2][CONVERSE_TEXT_LINES];
};

} // End of namespace Saga

#endif				/* INTERFACE_H__ */
/* end "r_interface.h" */
