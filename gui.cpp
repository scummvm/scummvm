#include "stdafx.h"
#include "scumm.h"
#include "gui.h"

void Gui::draw(int start,int end) {
	int i;

	if (end==-1)
		end=start;

	for (i=0; i<sizeof(_widgets) / sizeof(_widgets[0]); i++) {
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

void Gui::drawString(const char *str, int x, int y, int w, byte color, bool center) {
	StringTab *st = &_s->string[5];
	st->charset = 1;
	st->center = center;
	st->color = color;
	st->xpos = x;
	st->ypos = y;
	st->right = x + w;
	_s->_messagePtr = (byte*)str;
	_s->drawString(5);
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
	case GUI_TEXT:
		s = queryString(w->_string_number,w->_id);
		if (s)
			drawString(s, x+_parentX, y+_parentY, w->_w,
				(_clickWidget && _clickWidget==w->_id) ? _textcolorhi : _textcolor,
				false);
		break;
	case GUI_IMAGE:
		;
	}
}

void Gui::widgetClear(const GuiWidget *wid) {
	int x,y,w,h;
	int i,j;

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
	int i;
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
}

const GuiWidget save_load_dialog[] = {
	{GUI_STAT,0xFF,GWF_DEFAULT|GWF_PARENT,30,20,260,120,0,NULL},
	{GUI_TEXT,0x01,0,40,5,128,16,0,1}, /* How may I serve you? */
	{GUI_TEXT,0x02,0,40,5,128,16,0,2}, /* Select a game to LOAD */
	{GUI_TEXT,0x04,0,40,5,128,16,0,3}, /* Name your SAVE game */

	{GUI_STAT,0xFF,GWF_DEFAULT,6,16,170,96,0,0},
	{GUI_TEXT,0x01,GWF_DEFAULT,180,20,16,40,0,0},
	{GUI_TEXT,0x01,GWF_DEFAULT,180,66,16,40,0,0},
	{GUI_TEXT,0xFE,GWF_DEFAULT,180,20,16,40,1,0},
	{GUI_TEXT,0xFE,GWF_DEFAULT,180,66,16,40,2,0},
	
	{GUI_TEXT,0x06,GWF_CLEARBG,10,20,160,10,20,0},
	{GUI_TEXT,0x06,GWF_CLEARBG,10,30,160,10,21,0},
	{GUI_TEXT,0x06,GWF_CLEARBG,10,40,160,10,22,0},
	{GUI_TEXT,0x06,GWF_CLEARBG,10,50,160,10,23,0},
	{GUI_TEXT,0x06,GWF_CLEARBG,10,60,160,10,24,0},
	{GUI_TEXT,0x06,GWF_CLEARBG,10,70,160,10,25,0},
	{GUI_TEXT,0x06,GWF_CLEARBG,10,80,160,10,26,0},
	{GUI_TEXT,0x06,GWF_CLEARBG,10,90,160,10,27,0},
	{GUI_TEXT,0x06,GWF_CLEARBG,10,100,160,10,28,0},

	{GUI_TEXT,0x01,GWF_BUTTON,200,25,54,16,3,4}, /* Save */
	{GUI_TEXT,0x01,GWF_BUTTON,200,45,54,16,4,5}, /* Load */
	{GUI_TEXT,0x01,GWF_BUTTON,200,65,54,16,5,6}, /* Play */
	{GUI_TEXT,0x01,GWF_BUTTON,200,85,54,16,6,8}, /* Quit */

	{GUI_TEXT,0x02,GWF_BUTTON,200,50,54,16,7,7}, /* Cancel */

	{GUI_TEXT,0x04,GWF_BUTTON,200,45,54,16,8,9}, /* Ok */
	{GUI_TEXT,0x04,GWF_BUTTON,200,65,54,16,7,7}, /* Cancel */

	{0}
};

void Gui::handleCommand(int cmd) {
	int lastEdit = _editString;
	showCaret(false);

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

const byte string_map_table_v6[] = {
	117, /* How may I serve you? */
	109, /* Select a game to LOAD */
	108, /* Name your SAVE game */
  96,  /* Save */
	97,  /* Load */
	98,  /* Play */
	99,  /* Cancel */
	100, /* Quit */
	101, /* Ok */
};

const byte string_map_table_v5[] = {
	0, /* How may I serve you? */
	20, /* Select a game to LOAD */
	21, /* Name your SAVE game */
  7,  /* Save */
	8,  /* Load */
	9,  /* Play */
	10,  /* Cancel */
	11, /* Quit */
	12, /* Ok */
};

const char *Gui::queryString(int string, int id) {
	static char namebuf[64];
	if (id>=20 && id<=28) {
		sprintf(namebuf, "%2d. %s", id-20+_slotIndex, game_names[id-20]);
		return namebuf;
	}

	if (_s->_majorScummVersion==6) {
		string = _s->_vars[string_map_table_v6[string-1]];
	} else {
		string = string_map_table_v5[string-1];
	}
	
	return (char*)_s->getStringAddress(string);
}

void Gui::showCaret(bool show) {
	int i;
	char *s;

	if (_editString==-1)
		return;

	i = _editLen;
	s = game_names[_editString];

	if (show) {
		if (i < 31) {
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

void Gui::addLetter(char letter) {
	if (_editString==-1)
		return;

	if (letter==13) {
		handleCommand(8);
		return;
	}

	if (letter>=32 && letter<128 && _editLen < 31) {
		game_names[_editString][_editLen++] = letter;	
	} else if (letter==8 && _editLen>0) {
		_editLen--;
	}
	showCaret(true);
}

void Gui::saveLoadDialog() {
	_widgets[0] = save_load_dialog;
	_editString = -1;
	_cur_page = 0;
	_active = 1;
}

byte Gui::getDefaultColor(int color) {
	if (_s->_majorScummVersion == 6) {
		if (color==8) color=1;
		return _s->readArray(110, 0, color);
	} else {
		return _s->getStringAddress(21)[color];
	}
}


void Gui::init(Scumm *s) {
	_s = s;
	_bgcolor = getDefaultColor(0);
	_color = getDefaultColor(1);
	_textcolor = getDefaultColor(2);
	_textcolorhi = getDefaultColor(6);
	_shadowcolor = getDefaultColor(8);
}

void Gui::loop() {
	if (_active==1) {
		_active++;
		draw(0,100);
		_s->_cursorAnimate++;
		_s->gdi._cursorActive = 1;
	}
	
	_s->getKeyInput(0);
	if (_s->_mouseButStat&0x8000) {
		leftMouseClick(_s->mouse.x, _s->mouse.y);
	} else if (_s->_lastKeyHit) {
		addLetter(_s->_lastKeyHit);
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
	_active = false;
}
