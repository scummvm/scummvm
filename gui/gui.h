/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef GUI_H
#define GUI_H

// Forward declaration for GuiWidget
struct GuiWidget;

#define SAVEGAME_NAME_LEN 32

class Gui {
public:
	byte _color,_shadowcolor;
	byte _bgcolor;
	byte _textcolor;
	byte _textcolorhi;

	// Init
	void init(Scumm *s);

	// Dialogs
	void saveLoadDialog();
	void pause();
	void options();
	void launcher();

	void loop();

	bool isActive()	{ return _active; }

protected:
	Scumm *_s;
	const GuiWidget *_widgets[4];
	int _return_to;	
	VirtScreen *_vs;
	int _parentX, _parentY;
	bool _active;
	bool _inited;
	byte _clickTimer;
	byte _cur_page;
	byte _dialog;
	int _clickWidget;
	char *_queryMess;
	bool _old_soundsPaused;
	
	// mouse cursor state
	bool _old_cursor_mode;
	int _old_cursorHotspotX, _old_cursorHotspotY, _old_cursorWidth, _old_cursorHeight;
	byte _old_grabbedCursor[2048];

	// optiondialog specifics
	int _gui_variables[100];

	// checkboxes
	bool _cbox_checked[100];
	const char *_cbox_cfg_key[100];

	// savedialog specifics	
	int _slotIndex;
	int _editString;
	int _editLen;
	bool valid_games[9];
	char game_names[9][SAVEGAME_NAME_LEN];
	
	// Drawing
	void draw(int start, int end);
	void draw(int item) { draw(item,-1); }
	void drawWidget(const GuiWidget *w);	
	
    void line(int x, int y, int x2, int y2, byte color);
	void box(int x, int y, int width, int height);

	//void hline(int x, int y, int x2, byte color);
	//void vline(int x, int y, int y2, byte color);
	void drawChar(const char str, int xx, int yy);
	void drawString(const char *str, int x, int y, int w, byte color, bool center);
	void widgetClear(const GuiWidget *w);
	void widgetBorder(const GuiWidget *w);
	byte *getBasePtr(int x, int y);
	const GuiWidget *widgetFromPos(int x, int y);
	
	// Actions
	void leftMouseClick(int x, int y);
	void handleCommand(int cmd);
	void close();
	const char *queryString(int string, int id);
	void getSavegameNames(int start);
	void editString(int index);
	void showCaret(bool show);
	void addLetter(byte letter);	
	void queryMessage(const char *msg, const char *alts);
	byte getDefaultColor(int color);
	bool getCheckboxChecked(int id);
	void setCheckbox(bool state, int id);
	void clearCheckboxes();
	void checkHotKey(int keycode);

	char _gui_scroller[255];

	void handleSoundDialogCommand(int cmd);
	void handleOptionsDialogCommand(int cmd);
	void handleKeysDialogCommand(int cmd);
	void handleLauncherDialogCommand(int cmd);
	void handleMiscDialogCommand(int cmd);
};
#endif
