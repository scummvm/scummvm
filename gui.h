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

#if !defined(gui_h)
#define gui_h

struct ResString {
	int num;
	char string[80];
};

enum {
	GUI_NONE = 0,
	GUI_RESTEXT = 1,
	GUI_IMAGE = 2,
	GUI_STAT = 3,
	GUI_CUSTOMTEXT = 4,
	GUI_VARTEXT = 5,
	GUI_ACTIONTEXT = 6,
	GUI_KEYTEXT = 7
};

enum {
	GWF_BORDER = 1,
	GWF_CLEARBG = 2,
	GWF_PARENT = 4,
	GWF_DELAY = 8,
	GWF_DEFAULT = GWF_BORDER|GWF_CLEARBG,
	GWF_BUTTON = GWF_BORDER|GWF_CLEARBG|GWF_DELAY

};

struct GuiWidget {
	byte _type;
	byte _page;
	byte _flags;
	int16 _x,_y;
	uint16 _w,_h;
	uint16 _id;
	byte _string_number;
};

#define SAVEGAME_NAME_LEN 32

class Gui {
public:
	Scumm *_s;
	const GuiWidget *_widgets[4];
	int _return_to;
	int _curX, _curY;
	VirtScreen *_vs;
	byte _color,_shadowcolor;
	byte _bgcolor;
	byte _textcolor;
	byte _textcolorhi;
	bool _old_cursor_mode;
	int _parentX, _parentY;
	byte _active;
	byte _clickTimer;
	byte _cur_page;
	byte _dialog;
	int _clickWidget;
	char *_queryMess;

	/* optiondialog specifics */
	int _gui_variables[100];

	/* savedialog specifics */	
	int _slotIndex;
	int _editString;
	int _editLen;
	bool valid_games[9];
	char game_names[9][SAVEGAME_NAME_LEN];
	void loop();
	void init(Scumm *s);
	void draw(int start, int end);
	void draw(int item) { draw(item,-1); }
	void drawWidget(const GuiWidget *w);
	void moveto(int x, int y);
	void lineto(int x, int y);
	void hline(int x, int y, int x2);
	void vline(int x, int y, int y2);
	void drawChar(const char str, int xx, int yy);
	void drawString(const char *str, int x, int y, int w, byte color, bool center);
	void widgetClear(const GuiWidget *w);
	void widgetBorder(const GuiWidget *w);
	byte *getBasePtr(int x, int y);
	const GuiWidget *widgetFromPos(int x, int y);
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

	// Dialogs
	void saveLoadDialog();
	void pause();
	void options();
	void launcher();

	void handleSoundDialogCommand(int cmd);
	void handleOptionsDialogCommand(int cmd);
	void handleKeysDialogCommand(int cmd);
	void handleLauncherDialogCommand(int cmd);
};
#endif
