/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001 The ScummVM project
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

#include "stdafx.h"
#include "scumm.h"
#include "gui.h"
#include "sound.h"

#ifdef _WIN32_WCE
// Additional variables for Win32 specific GUI
	#include "gapi_keys.h"
	extern bool toolbar_drawn;
	extern bool draw_keyboard;
	extern bool get_key_mapping;
	extern struct keyops keyMapping;
	extern void registry_save(void);
	uint16 _key_mapping_required;
#else
	#define registry_save() ;
	bool get_key_mapping;
	uint16 _key_mapping_required;
#endif

enum {
	SAVELOAD_DIALOG,
	PAUSE_DIALOG,
	SOUND_DIALOG,
	KEYS_DIALOG,
	OPTIONS_DIALOG,
	ABOUT_DIALOG,
	LAUNCHER_DIALOG
};


void Gui::draw(int start,int end) {
	int i;

	if (end==-1)
		end=start;

	for (i=0; i<(int)(sizeof(_widgets) / sizeof(_widgets[0])); i++) {
		const GuiWidget *w = _widgets[i];
		if (w) {
			_parentX = 0;
			_parentY = 0;
			while (w->_type != GUI_NONE) {
				if (w->_id>=start && w->_id<=end && (w->_page&(1<<_cur_page)) ) {
					drawWidget(w);
				}
				if (w->_flags & GWF_PARENT) {
					_parentX += w->_x;
					_parentY += w->_y;
				}
				w++;
			}
		}
	}
}

const GuiWidget *Gui::widgetFromPos(int x, int y) {
	int i;

	for (i=sizeof(_widgets) / sizeof(_widgets[0])-1; i>=0; i--) {
		const GuiWidget *w = _widgets[i];
		if (w) {
			while (w->_type != GUI_NONE) {
				if ((w->_page&(1<<_cur_page)) && w->_id && 
						(uint16)(x-w->_x) < w->_w && (uint16)(y-w->_y) < w->_h)
					return w;
				if (w->_flags & GWF_PARENT) {
					x -= w->_x;
					y -= w->_y;
				}
				w++;
			}
		}
	}
	return NULL;
}

void Gui::drawChar(const char str, int xx, int yy) { 
  unsigned int buffer, mask = 0, x, y;    
  byte *tmp;
  int tempc = _color;
  _color = _textcolor;

  tmp = &guifont[0];
  tmp += 224 + (str + 1)*8;

  for(y=0;y<8;y++){
    for(x=0;x<8;x++){
		unsigned char color;
    	if ((mask >>= 1) == 0) {buffer = *tmp++;  mask = 0x80;}
        color = ((buffer & mask) != 0);
        if (color)
				vline(xx + x, yy + y, yy + y);
      }
  }
  _color = tempc;

}
void Gui::drawString(const char *str, int x, int y, int w, byte color, bool center) {
	StringTab *st = &_s->string[5];
	st->charset = 1;
	st->center = center;
	st->color = color;
	st->xpos = x;
	st->ypos = y;
	st->right = x + w;

	if (_s->_gameId) {		/* If a game is active.. */
		_s->_messagePtr = (byte*)str;
		_s->drawString(5);
	} else {		
		for (uint letter = 0; letter < strlen(str); letter++)
			drawChar(str[letter], st->xpos + (letter * 8), st->ypos);		
	}
}

void Gui::drawWidget(const GuiWidget *w) {
	const char *s;
	int x,y;
	
	x = w->_x;
	y = w->_y;

	if (w->_flags & GWF_CLEARBG)
		widgetClear(w);
		
	if (w->_flags & GWF_BORDER) {
		widgetBorder(w);
		x += 4;
		y += 4;
	}

	switch(w->_type) {
	case GUI_CUSTOMTEXT:
	case GUI_VARTEXT:
	case GUI_KEYTEXT:
	case GUI_ACTIONTEXT:
	case GUI_RESTEXT: {
		char text[500];
		text[0] = '\0';

		switch(w->_type) {
			case GUI_CUSTOMTEXT:
				strcpy(text, string_map_table_custom[w->_string_number]);
				break;
			case GUI_RESTEXT:
				s = queryString(w->_string_number,w->_id);
				if (s)
					strcpy(text, s);
				break;
			case GUI_VARTEXT:
				sprintf(text, "%s %d", string_map_table_custom[w->_string_number],  _gui_variables[w->_string_number]);
				break;
#ifdef _WIN32_WCE
			case GUI_KEYTEXT:
				strcpy(text, getGAPIKeyName(getAction(w->_string_number - 1)->action_key));
				break;
			case GUI_ACTIONTEXT:
				strcpy(text, getActionName(getAction(w->_string_number - 1)->action_type));
				break;
#endif
		}
		
		if (*text) {
			printf("drawString(%s)\n", text);
			drawString(text, x+_parentX, y+_parentY, w->_w,
				(_clickWidget && _clickWidget==w->_id) ? _textcolorhi : _textcolor,
				false);
		}
		break;
	}
	case GUI_IMAGE:
		;
	}
}

void Gui::widgetClear(const GuiWidget *wid) {
	int x,y,w,h,i;

	x = wid->_x;
	y = wid->_y;
	w = wid->_w;
	h = wid->_h;
	
	byte *ptr = getBasePtr(x,y);
	if(ptr==NULL)
		return;

	_s->setVirtscreenDirty(_vs, x+_parentX, y+_parentY, x+_parentX+w, y+_parentY+h);

	if (wid->_flags & GWF_BORDER) {
		ptr += 320 + 1;
		w-=2;
		h-=2;
	}
	
	while (--h>=0) {
		for(i=0; i<w; i++)
			ptr[i] = _bgcolor;
		ptr += 320;
	}
}

void Gui::widgetBorder(const GuiWidget *w) {
	int x = w->_x, y = w->_y;
	int x2 = x + w->_w-1, y2 = y + w->_h-1;
	byte tmp;
	
	hline(x+1,y,x2-1);
	hline(x,y+1,x2);
	vline(x,y+1,y2-1);
	vline(x+1,y,y2);

	tmp = _color;
	_color = _shadowcolor;

	hline(x+1,y2-1,x2);
	hline(x+1,y2,x2-1);
	vline(x2,y+1,y2-1);
	vline(x2-1,y+1,y2);

	_color = tmp;
}

void Gui::hline(int x, int y, int x2) {
	moveto(x,y);
	lineto(x2,y);
}

void Gui::vline(int x, int y, int y2) {
	moveto(x,y);
	lineto(x,y2);
}

void Gui::moveto(int x, int y) {
	_curX = x;
	_curY = y;
}

byte *Gui::getBasePtr(int x, int y) {
	x += _parentX;
	y += _parentY;
	_vs = _s->findVirtScreen(y);
	
	if (_vs==NULL)
		return NULL;
	
	return _s->getResourceAddress(rtBuffer, _vs->number+1) + x + (y-_vs->topline)*320 + _s->_screenStartStrip*8;
}

void Gui::lineto(int x, int y) {
	byte *ptr;
	int x2 = _curX;
	int y2 = _curY;

	_curX = x;
	_curY = y;

	if (x2 < x)
		x2^=x^=x2^=x;

	if (y2 < y)
		y2^=y^=y2^=y;

	ptr = getBasePtr(x, y);
	
	if (ptr==NULL)
		return;

	if (x==x2) {
		/* vertical line */
		while (y++ <= y2) {
			*ptr = _color;
			ptr += 320;
		}
	} else if (y==y2) {
		/* horizontal line */
		while (x++ <= x2) {
			*ptr++ = _color;
		}
	}
}

void Gui::leftMouseClick(int x, int y) {
	const GuiWidget *w = widgetFromPos(x,y);
	int old;
	
	_clickTimer = 0;

	old = _clickWidget;
	_clickWidget = w?w->_id : 0;

	if (old)
		draw(old);
	if (_clickWidget) {
		draw(_clickWidget);
		if(w->_flags&GWF_DELAY)
			_clickTimer = 5;
		else
			handleCommand(_clickWidget);
	}
	
	if (_dialog == PAUSE_DIALOG)
		close();
}
const GuiWidget launcher_dialog[] = {
	{GUI_STAT, 0xFF, GWF_DEFAULT, 0, 0, 320, 200, 0, 0 },	
	{GUI_CUSTOMTEXT,0x01,GWF_CLEARBG, 5,   180, 45, 15, 20, 12},
	{GUI_CUSTOMTEXT,0x01,GWF_CLEARBG, 130, 180, 65, 15, 21,17},
	{GUI_CUSTOMTEXT,0x01,GWF_CLEARBG, 265, 180, 50, 15, 22, 7},
	{0,0,0,0,0,0,0,0,0}
};
const GuiWidget keys_dialog[] = {
	{GUI_STAT, 0xFF, GWF_DEFAULT, 30, 10, 260, 130, 0, 0 },

	// First action
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 10 + 10, 15, 15, 10, 3}, // CUSTOMTEXT_PLUS
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 10 + 10, 15, 15, 11, 4}, // CUSTOMTEXT_MINUS
	{GUI_ACTIONTEXT, 0x01, GWF_BUTTON, 30 + 11 + 33 + 10, 10 + 10, 100, 15, 100, 1},
	{GUI_KEYTEXT, 0x01, 0, 30 + 11 + 33 + 120, 10 + 10 + 3, 100, 15, 1, 1},

	//Second action
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 10 + 10 + 15 + 5, 15, 15, 20, 3}, // CUSTOMTEXT_PLUS
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 10 + 10 + 15 + 5, 15, 15, 21, 4}, // CUSTOMTEXT_MINUS
	{GUI_ACTIONTEXT, 0x01, GWF_BUTTON, 30 + 10 + 33 + 10, 10 + 10 + 15 + 5, 100, 15, 101, 2},
	{GUI_KEYTEXT, 0x01, 0, 30 + 11 + 33 + 120, 10 + 10 + 15 + 5 + 3, 100, 15, 2,  2},
	
	//Third action
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 10 + 10 + 15 + 5 + 15 + 5, 15, 15, 30, 3},// CUSTOMTEXT_PLUS
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 10 + 10 + 15 + 5 + 15 + 5, 15, 15, 31, 4}, // CUSTOMTEXT_MINUS
	{GUI_ACTIONTEXT, 0x01, GWF_BUTTON, 30 + 10 + 33 + 10, 10 + 10 + 15 + 5 + 15 + 5, 100, 15, 102, 3},
	{GUI_KEYTEXT, 0x01, 0, 30 + 11 + 33 + 120, 10 + 10 + 15 + 5 + 15 + 5 + 3, 100, 15, 3, 3},

	//Fourth action
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5, 15, 15, 40, 3},
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5, 15, 15, 41, 4},
	{GUI_ACTIONTEXT, 0x01, GWF_BUTTON, 30 + 10 + 33 + 10, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5, 100, 15, 103, 4},
	{GUI_KEYTEXT, 0x01, 0, 30 + 11 + 33 + 120, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5 + 3, 100, 15, 4, 4},

	//Fifth action
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5 + 15 + 5, 15, 15, 50, 3},
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5 + 15 + 5, 15, 15, 51, 4},
	{GUI_ACTIONTEXT, 0x01, GWF_BUTTON, 30 + 10 + 33 + 10, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5 + 15 + 5, 100, 15, 104, 5},
	{GUI_KEYTEXT, 0x01, 0, 30 + 11 + 33 + 120, 10 + 10 + 15 + 5 + 15 + 5 + 15 + 5 + 15 + 5 + 3, 100, 15, 5, 5},

	//OK
	{GUI_RESTEXT, 0x01, GWF_BUTTON, 30 + 113, 10 + 106, 54, 16, 60, 9 },
	{0,0,0,0,0,0,0,0,0}
};


const GuiWidget about_dialog[] = {
	{GUI_STAT, 0xFF, GWF_DEFAULT, 30, 20, 260, 120, 0, 0 },
	// {GUI_CUSTOMTEXT, 0x01, 0, 30 + 95, 20 + 10, 100, 15, 0, }, .. pocketscummvm
	{GUI_CUSTOMTEXT, 0x01, 0, 30 + 68, 20 + 10 + 15 + 5, 160, 15, 0, 9}, // Build
	{GUI_CUSTOMTEXT, 0x01, 0, 30 + 10, 20 + 10 + 15 + 5 + 15, 230, 15, 0, 10},	 // ScummVM Url
	{GUI_CUSTOMTEXT, 0x01, 0, 30 + 75, 20 + 10 + 15 + 5 + 15 + 15 + 15, 150, 15, 0, 11}, // Lucasarts
	{GUI_RESTEXT, 0x01, GWF_BUTTON, 30 + 113, 20 + 96, 54, 16, 40, 9 },
	{0,0,0,0,0,0,0,0,0}
};

const GuiWidget options_dialog[] = {
	{GUI_STAT, 0xFF, GWF_DEFAULT, 50, 80, 210, 35, 0, 0 },
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 50 + 10 , 80 + 10, 40, 15, 1, 5}, // Sound
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 50 + 10 + 40 + 30 , 80 + 10, 40, 15 , 2, 6}, // Keys
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 50 + 10 + 40 + 30 + 40 + 30, 80 + 10, 40, 15, 3, 7},	 // About
	{0,0,0,0,0,0,0,0,0}
};

const GuiWidget sound_dialog[] = {
	{GUI_STAT, 0xFF, GWF_DEFAULT, 30, 20, 260, 120, 0, 0 },
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 20 + 11, 15, 15, 1, 3}, // Plus
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 20 + 11, 15, 15, 2, 4}, // Minus
	{GUI_VARTEXT, 0x01, GWF_DEFAULT, 30 + 73, 20 + 11, 128, 15, 3, 0}, // Master
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 20 + 25 + 11, 15, 15, 11, 3}, // Plus
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 20 + 25 + 11, 15, 15, 12, 4}, // Minus
	{GUI_VARTEXT, 0x01, GWF_BUTTON, 30 + 73, 20 + 25 + 11, 128, 15, 13, 1},	 // Music
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 11, 20 + 25 + 25 + 11, 15, 15, 21, 3}, // Plus
	{GUI_CUSTOMTEXT, 0x01, GWF_BUTTON, 30 + 33, 20 + 25 + 25 + 11, 15, 15, 22, 4}, // Minus
	{GUI_VARTEXT, 0x01, GWF_BUTTON, 30 + 73, 20 + 25 + 25 + 11, 128, 15, 23, 2}, // SFX
	{GUI_RESTEXT,0x01,GWF_BUTTON,30 + (260 / 2) - 80, 20 + 25 + 25 + 11 + 25 ,54,16,40,9}, /* OK */
	{GUI_RESTEXT,0x01,GWF_BUTTON,30 + (260 / 2), 20 + 25 + 25 + 11 + 25 ,54,16,50,7}, /* Cancel */
	{0,0,0,0,0,0,0,0,0}
};

const GuiWidget save_load_dialog[] = {
	{GUI_STAT,0xFF,GWF_DEFAULT|GWF_PARENT,30,20,260,125,0,0},
	{GUI_RESTEXT,0x01,0,40,5,128,16,0,1}, /* How may I serve you? */
	{GUI_RESTEXT,0x02,0,40,5,128,16,0,2}, /* Select a game to LOAD */
	{GUI_RESTEXT,0x04,0,40,5,128,16,0,3}, /* Name your SAVE game */

	{GUI_STAT,0xFF,GWF_DEFAULT,6,16,170,96,0,0},
	{GUI_RESTEXT,0x01,GWF_DEFAULT,180,20,16,40,0,0},
	{GUI_RESTEXT,0x01,GWF_DEFAULT,180,66,16,40,0,0},
	{GUI_RESTEXT,0xFE,GWF_DEFAULT,180,20,16,40,1,0},
	{GUI_RESTEXT,0xFE,GWF_DEFAULT,180,66,16,40,2,0},
	
	{GUI_RESTEXT,0x06,GWF_CLEARBG,10,20,160,10,20,0},
	{GUI_RESTEXT,0x06,GWF_CLEARBG,10,30,160,10,21,0},
	{GUI_RESTEXT,0x06,GWF_CLEARBG,10,40,160,10,22,0},
	{GUI_RESTEXT,0x06,GWF_CLEARBG,10,50,160,10,23,0},
	{GUI_RESTEXT,0x06,GWF_CLEARBG,10,60,160,10,24,0},
	{GUI_RESTEXT,0x06,GWF_CLEARBG,10,70,160,10,25,0},
	{GUI_RESTEXT,0x06,GWF_CLEARBG,10,80,160,10,26,0},
	{GUI_RESTEXT,0x06,GWF_CLEARBG,10,90,160,10,27,0},
	{GUI_RESTEXT,0x06,GWF_CLEARBG,10,100,160,10,28,0},

	{GUI_RESTEXT,0x01,GWF_BUTTON,200,25,54,16,3,4}, /* Save */
	{GUI_RESTEXT,0x01,GWF_BUTTON,200,45,54,16,4,5}, /* Load */
	{GUI_RESTEXT,0x01,GWF_BUTTON,200,65,54,16,5,6}, /* Play */
	{GUI_CUSTOMTEXT,0x01,GWF_BUTTON,200,85,54,16,9,17}, /* Options */
	{GUI_RESTEXT,0x01,GWF_BUTTON,200,105,54,16,6,8}, /* Quit */
	
	{GUI_RESTEXT,0x02,GWF_BUTTON,200,50,54,16,7,7}, /* Cancel */

	{GUI_RESTEXT,0x04,GWF_BUTTON,200,45,54,16,8,9}, /* Ok */
	{GUI_RESTEXT,0x04,GWF_BUTTON,200,65,54,16,7,7}, /* Cancel */
	{0,0,0,0,0,0,0,0,0}
};

const GuiWidget pause_dialog[] = {
	{GUI_RESTEXT,0x01,GWF_DEFAULT,50,80,220,16,0,10},
	{0,0,0,0,0,0,0,0,0}
};

void Gui::handleSoundDialogCommand(int cmd) {
	if (cmd == 40 || cmd == 50) {
		if (cmd == 40) {
			SoundEngine *se = (SoundEngine*)_s->_soundEngine;
			_s->_sound_volume_master =	_gui_variables[0]; // Master
			_s->_sound_volume_music =	_gui_variables[1]; // Music
			_s->_sound_volume_sfx =		_gui_variables[2]; // SFX
			se->set_music_volume(_s->_sound_volume_music);
			se->set_master_volume(_s->_sound_volume_master);
			registry_save();
		}

	close();
	return;
	}
	if ((cmd % 10) == 1) {
		if (_gui_variables[cmd / 10] < 100)
			_gui_variables[cmd / 10]+=5;
	}
	else {
		if (_gui_variables[cmd / 10] > 0)
			_gui_variables[cmd / 10]-=5;
	}
	
	draw((cmd / 10) * 10 + 3, (cmd / 10) * 10 + 3);
}

void Gui::handleOptionsDialogCommand(int cmd) {
	switch(cmd) {
		case 1:
			_widgets[0] = sound_dialog;
			_gui_variables[0] = _s->_sound_volume_master;
			_gui_variables[1] = _s->_sound_volume_music;
			_gui_variables[2] = _s->_sound_volume_sfx;
			_active = true;
			_cur_page = 0;
			_dialog = SOUND_DIALOG;
			draw(0, 100);
			return;
		case 2:
			_key_mapping_required = 0;
			get_key_mapping = true;
			_widgets[0] = keys_dialog;
			_active = true;
			_cur_page = 0;
			_dialog = KEYS_DIALOG;
			draw(0, 200);
			return;
		case 3:
			_widgets[0] = about_dialog;
			_active = true;
			_cur_page = 0;
			_return_to = 0;
			_dialog = ABOUT_DIALOG;
			draw(0, 100);
			return;
	}
}

void Gui::handleKeysDialogCommand(int cmd) {
#ifdef _WIN32_WCE
	if (cmd < 100 && cmd != 60) {

		if ((cmd % 10) == 1) 
			setNextType((cmd / 10) - 1);
		else 
			setPreviousType((cmd / 10) - 1);

		draw(0, 200);

		return;
	}
			
	_key_mapping_required = cmd;

	if (cmd == 60) {
		get_key_mapping = false;
		registry_save();
		close();
	}
#else
	close();
#endif
}

void Gui::handleLauncherDialogCommand(int cmd) {
	printf("handle launcher command\n");
 switch(cmd) {
	case 20:
		close();
	break;
	case 21:
		// Nothing yet
	break;
	case 22:
			_widgets[0] = about_dialog;
			_active = true;
			_cur_page = 0;
			_return_to = LAUNCHER_DIALOG;
			_dialog = ABOUT_DIALOG;
			draw(0, 100);
			printf("about dialog\n");
	break;
	default:
		printf("default\n");
		close();
 }
}

void Gui::handleCommand(int cmd) {
	int lastEdit = _editString;
	showCaret(false);

	if (_dialog == LAUNCHER_DIALOG) {
		handleLauncherDialogCommand(cmd);
		return;
	}
	if (_dialog == SOUND_DIALOG) {
		handleSoundDialogCommand(cmd);
		return;
	}	
	
	if (_dialog == OPTIONS_DIALOG) {		
		handleOptionsDialogCommand(cmd);
		return;
	}

	if (_dialog == KEYS_DIALOG) {
		handleKeysDialogCommand(cmd);
		return;
	}

	if (_dialog == ABOUT_DIALOG) {
		if (_return_to == LAUNCHER_DIALOG) {
			_widgets[0] = launcher_dialog;
			_active = true;
			_cur_page = 0;
			_dialog = LAUNCHER_DIALOG;
			draw(0, 100);
		} else 
			close();
		return;
	}

	switch(cmd) {
	case 1: /* up button */
		if (_slotIndex==0)
			return;
		getSavegameNames(_slotIndex-9);
		draw(20,28);
		return;
	case 2:
		if (_slotIndex > 80)
			return;
		getSavegameNames(_slotIndex+9);
		draw(20,28);
		return;
	case 3: /* save button */
		_cur_page = 2;
		getSavegameNames(0);
		draw(0,100);
		return;
	case 4: /* load button */
		_cur_page = 1;
		getSavegameNames(0);
		draw(0,100);
		return;
	case 5: /* play button */
		close();
		return;
	case 6: /* quit button */
		exit(1);
		return;
	case 7: /* cancel button */
		_cur_page = 0;
		draw(0,100);
		return;
	case 8:
		if (lastEdit==-1 || game_names[lastEdit][0]==0)
			return;
		_s->_saveLoadSlot = lastEdit + _slotIndex;
		_s->_saveLoadCompatible = false;
		_s->_saveLoadFlag = 1;
		memcpy(_s->_saveLoadName, game_names[lastEdit], sizeof(_s->_saveLoadName));
		close();
		return;
	case 9: /* options button */
		options();
		draw(0, 100);
		return;
	default:
		if (cmd>=20 && cmd<=28) {
			if(_cur_page==1) {
				if (valid_games[cmd-20]) {
					_s->_saveLoadSlot = cmd-20+_slotIndex;
					_s->_saveLoadCompatible = false;
					_s->_saveLoadFlag = 2;
					close();
				}
				return;
			} else if (_cur_page==2) {
				_clickWidget = cmd;
				editString(cmd - 20);
			}
		}
	}

}

void Gui::getSavegameNames(int start) {
	int i;
	_slotIndex = start;
	for(i=0; i<9; i++,start++) {
		valid_games[i] = _s->getSavegameName(start, game_names[i]);
	}
}

const char *Gui::queryString(int stringno, int id) {
	static char namebuf[64];
	char *result;
	int  string;
	if (id>=20 && id<=28) {
		sprintf(namebuf, "%2d. %s", id-20+_slotIndex, game_names[id-20]);
		return namebuf;
	}

	if (stringno == 0)
		return NULL;

	if (_s->_features&GF_AFTER_V6)
		string = _s->_vars[string_map_table_v6[stringno-1].num];
	else
		string = string_map_table_v5[stringno-1].num;
	
	result = (char*)_s->getStringAddress(string);

	if (!result)	// Gracelessly degrade to english :)
		if (_s->_features&GF_AFTER_V6)
			return string_map_table_v6[stringno-1].string;
		else
			return string_map_table_v5[stringno-1].string;

	return result;
}

void Gui::showCaret(bool show) {
	int i;
	char *s;

	if (_editString==-1)
		return;

	i = _editLen;
	s = game_names[_editString];

	if (show) {
		if (i < SAVEGAME_NAME_LEN-1) {
			s[i] = '_';
			s[i+1] = 0;
		}
	} else {
		s[i] = 0;
	}
	
	draw(_editString + 20);

	if (!show)
		_editString = -1;
}

void Gui::editString(int i) {
	char *s = game_names[i];
	if(!valid_games[i]) {
		valid_games[i] = true;
		*s = 0;
	}
	_editString = i;
	_editLen = strlen(s);
	showCaret(true);
}

void Gui::addLetter(byte letter) {
	switch(_dialog) {
	case SAVELOAD_DIALOG:
		if (_editString==-1)
			return;

		if (letter==13) {
			handleCommand(8);
			return;
		}

		if (letter>=32 && letter<128 && _editLen < SAVEGAME_NAME_LEN-1) {
			game_names[_editString][_editLen++] = letter;	
		} else if (letter==8 && _editLen>0) {
			_editLen--;
		}
		showCaret(true);
		break;
	case PAUSE_DIALOG:
		if (letter==32)
			close();
		break;

#ifdef _WIN32_WCE
	case KEYS_DIALOG:
		clearActionKey(letter);
		if (_key_mapping_required) 
			getAction(_key_mapping_required - 100)->action_key = letter;
		_key_mapping_required = 0;
		draw(0, 200);
		break;
#endif
	}
}

byte Gui::getDefaultColor(int color) {
	if((_s->_features & GF_AFTER_V7) || (_s->_features & GF_SMALL_HEADER))
		return 0;
	if (_s->_features&GF_AFTER_V6) {
		if (color==8) color=1;
		return _s->readArray(110, 0, color);
	} else {
		return _s->getStringAddress(21)[color];
	}
}

void Gui::init(Scumm *s) {
	_s = s;
	if (_s->_gameId && !(_s->_features & GF_SMALL_HEADER)) {
		_bgcolor =  getDefaultColor(0);
		_color = getDefaultColor(1);
		_textcolor = getDefaultColor(2);
		_textcolorhi = getDefaultColor(6);
		_shadowcolor = getDefaultColor(8);
	} else {
		_bgcolor = 0;
		_color	= 0;
		_textcolor = 8;	// 15 is nice
		_textcolorhi = 15;
		_shadowcolor = 0;
	}
}

void Gui::loop() {
	if (_active==1) {
		_active++;
		draw(0,200); // was 100
		_s->_cursorAnimate++;
		_s->gdi._cursorActive = 1;
		_s->pauseSounds(true);
	}
	
	_s->getKeyInput(0);
	if (_s->_mouseButStat&MBS_LEFT_CLICK) {
		leftMouseClick(_s->mouse.x, _s->mouse.y);
	} else if (_s->_lastKeyHit) {
		if (_dialog != KEYS_DIALOG)
			addLetter((unsigned char)_s->_lastKeyHit);
#ifdef _WIN32_WCE
		else if (_s->_lastKeyHit > 1000) // GAPI
			addLetter(_s->_lastKeyHit - 1000);
#endif

	}
	
	if (_clickTimer && !--_clickTimer) {
		int old = _clickWidget;
		_clickWidget = 0;
		draw(old);
		handleCommand(old);
	}
	
	_s->drawDirtyScreenParts();
	_s->_mouseButStat = 0;
}

void Gui::close() {
	_s->_fullRedraw = true;
	_s->_completeScreenRedraw = true;
	_s->_cursorAnimate--;
	_s->pauseSounds(false);
	_active = false;

#ifdef _WIN32_WCE

	// Option dialog can be accessed from the file dialog now, always check
	if (draw_keyboard) {
		draw_keyboard = false;
		toolbar_drawn = false;
	}
#endif
}

void Gui::saveLoadDialog() {
	_widgets[0] = save_load_dialog;
	_editString = -1;
	_cur_page = 0;
	_active = true;
	_dialog = SAVELOAD_DIALOG;
}

void Gui::pause() {
	_widgets[0] = pause_dialog;
	_cur_page = 0;
	_active = true;
	_dialog = PAUSE_DIALOG;
}

void Gui::options() {
	_widgets[0] = options_dialog;
	_active = true;
	_cur_page = 0;
	_dialog = OPTIONS_DIALOG;
}

void Gui::launcher() {
	_widgets[0] = launcher_dialog;
	_active = true;
	_cur_page = 0;
	_dialog = LAUNCHER_DIALOG;
}
