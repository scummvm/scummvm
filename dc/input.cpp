/* ScummVM - Scumm Interpreter
 * Dreamcast port
 * Copyright (C) 2002  Marcus Comstedt
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm.h"
#include "dc.h"

void handleInput(struct mapledev *pad, int16 &mouse_x, int16 &mouse_y,
		 byte &leftBtnPressed, byte &rightBtnPressed, int &keyPressed)
{
  int lmb=0, rmb=0, newkey=0;
  static int lastkey = 0;
  static byte lastlmb = 0, lastrmb = 0;
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
	case 0x4c:
	  if((shift & 0x11) && (shift & 0x44))
	    exit(0);
	  break;
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

  if(lmb && !lastlmb) {
    leftBtnPressed |= msClicked|msDown;
    lastlmb = 1;
  } else if(lastlmb && !lmb) {
    leftBtnPressed &= ~msDown;
    lastlmb = 0;
  }
  if(rmb && !lastrmb) {
    rightBtnPressed |= msClicked|msDown;
    lastrmb = 1;
  } else if(lastrmb && !rmb) {
    rightBtnPressed &= ~msDown;    
    lastrmb = 0;
  }

  if(!newkey)
    lastkey = 0;
  else if(newkey != lastkey)
    keyPressed = lastkey = newkey;

  if (mouse_x<0) mouse_x=0;
  if (mouse_x>319) mouse_x=319;
  if (mouse_y<0) mouse_y=0;
  if (mouse_y>199) mouse_y=199;
}

