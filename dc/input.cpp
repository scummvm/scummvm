#include "stdafx.h"
#include "scumm.h"
#include "dc.h"

void handleInput(struct mapledev *pad, int16 &mouse_x, int16 &mouse_y,
		 byte &leftBtnPressed, byte &rightBtnPressed, int &keyPressed)
{
  int lmb=0, rmb=0, newkey=0;
  static int lastkey = 0;
  for(int i=0; i<4; i++, pad++)
    if(pad->func & MAPLE_FUNC_CONTROLLER) {
      int buttons = pad->cond.controller.buttons;

      if(!(buttons & 0x060e)) exit(0);

      if(!(buttons & 4)) lmb++;
      if(!(buttons & 2)) rmb++;

      if(!(buttons & 8)) newkey = 319;
      else if(!(buttons & 512)) newkey = ' ';
      else if(!(buttons & 1024)) newkey = '0';

      if(!(buttons & 128)) mouse_x++;
      if(!(buttons & 64)) mouse_x--;
      if(!(buttons & 32)) mouse_y++;
      if(!(buttons & 16)) mouse_y--;

      mouse_x += ((int)pad->cond.controller.joyx-128)>>4;
      mouse_y += ((int)pad->cond.controller.joyy-128)>>4;
    } else if(pad->func & MAPLE_FUNC_MOUSE) {
      int buttons = pad->cond.mouse.buttons;

      if(!(buttons & 4)) lmb++;
      if(!(buttons & 2)) rmb++;

      if(!(buttons & 8)) newkey = 319;
      
      mouse_x += pad->cond.mouse.axis1;
      mouse_y += pad->cond.mouse.axis2;
      pad->cond.mouse.axis1 = 0;
      pad->cond.mouse.axis2 = 0;
    } else if(pad->func & MAPLE_FUNC_KEYBOARD) {
      for(int p=0; p<6; p++) {
	int shift = pad->cond.kbd.shift;
	int key = pad->cond.kbd.key[p];
	if(shift & 0x08) lmb++;
	if(shift & 0x80) rmb++;
	if(key >= 4 && key <= 0x1d)
	  newkey = key+((shift & 0x22)? ('A'-4) : ('a'-4));
	else if(key >= 0x1e && key <= 0x26)
	  newkey = key+((shift & 0x22)? ('!'-0x1e) : ('1'-0x1e));
	else if(key >= 0x59 && key <= 0x61)
	  newkey = key+('1'-0x59);
	else if(key >= 0x3a && key <= 0x43)
	  newkey = key+(315-0x3a);
	else switch(key) {
	case 0x27: case 0x62:
	  newkey = ((shift & 0x22)? '~' : '0'); break;
	case 0x28: case 0x58:
	  newkey = 13; break;
	case 0x29:
	  newkey = 27; break;
	case 0x2a:
	  newkey = 8; break;
	case 0x2b:
	  newkey = 9; break;
	case 0x2c:
	  newkey = ' '; break;
	case 0x4f:
	  mouse_x++; break;
	case 0x50:
	  mouse_x--; break;
	case 0x51:
	  mouse_y++; break;
	case 0x52:
	  mouse_y--; break;
	}
      }
    }

  if(lmb)
    leftBtnPressed |= msClicked|msDown;
  else
    leftBtnPressed &= ~msDown;
  if(rmb)
    rightBtnPressed |= msClicked|msDown;
  else
    rightBtnPressed &= ~msDown;    

  if(!newkey)
    lastkey = 0;
  else if(newkey != lastkey)
    keyPressed = lastkey = newkey;
}

