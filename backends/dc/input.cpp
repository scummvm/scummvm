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

#define RONIN_TIMER_ACCESS

#include <common/scummsys.h>
#include <common/stdafx.h>
#include "dc.h"

int handleInput(struct mapledev *pad, int &mouse_x, int &mouse_y,
		byte &shiftFlags)
{
  int lmb=0, rmb=0, newkey=0;
  static int lastkey = 0;
  static byte lastlmb = 0, lastrmb = 0;
  shiftFlags = 0;
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
	if(shift & 0x11) shiftFlags |= OSystem::KBD_CTRL;
	if(shift & 0x44) shiftFlags |= OSystem::KBD_ALT;
	if(shift & 0x22) shiftFlags |= OSystem::KBD_SHIFT;
	if(key >= 4 && key <= 0x1d)
	  newkey = key+('a'-4);
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
    lastlmb = 1;
    return -OSystem::EVENT_LBUTTONDOWN;
  } else if(lastlmb && !lmb) {
    lastlmb = 0;
    return -OSystem::EVENT_LBUTTONUP;
  }
  if(rmb && !lastrmb) {
    lastrmb = 1;
    return -OSystem::EVENT_RBUTTONDOWN;
  } else if(lastrmb && !rmb) {
    lastrmb = 0;
    return -OSystem::EVENT_RBUTTONUP;
  }

  if(!newkey || (lastkey && newkey != lastkey)) {
    int upkey = lastkey;
    lastkey = 0;
    if(upkey)
      return upkey | (1<<30);
  } else if(!lastkey)
    return lastkey = newkey;

  return 0;
}

bool OSystem_Dreamcast::poll_event(Event *event)
{
  unsigned int t = Timer();

  if(_timer_active && ((int)(t-_timer_next_expiry))>=0) {
    _timer_duration = _timer_callback(_timer_duration);
    _timer_next_expiry = t+USEC_TO_TIMER(1000*_timer_duration);
  }

  if(((int)(t-_devpoll))<0)
    return false;
  _devpoll += USEC_TO_TIMER(17000);
  if(((int)(t-_devpoll))>=0)
    _devpoll = t + USEC_TO_TIMER(17000);
  int mask = getimask();
  setimask(15);
  checkSound();
  int e = handleInput(locked_get_pads(), _ms_cur_x, _ms_cur_y,
		      event->kbd.flags);
  setimask(mask);
  if (_ms_cur_x<0) _ms_cur_x=0;
  if (_ms_cur_x>319) _ms_cur_x=319;
  if (_ms_cur_y<0) _ms_cur_y=0;
  if (_ms_cur_y>=(_hires? (_screen_h>>1):_screen_h))
    _ms_cur_y=(_hires? (_screen_h>>1):_screen_h)-1;
  event->mouse.x = (_hires? (_ms_cur_x<<1):_ms_cur_x);
  event->mouse.y = (_hires? (_ms_cur_y<<1):_ms_cur_y);
  if (_overlay_visible) {
    event->mouse.x -= _overlay_x;
    event->mouse.y -= _overlay_y;
  }
  event->kbd.ascii = event->kbd.keycode = 0;
  if(e<0) {
    event->event_code = (EventCode)-e;
    return true;
  } else if(e>0) {
    event->event_code = ((e&(1<<30))? EVENT_KEYUP : EVENT_KEYDOWN);
    e &= ~(1<<30);
    event->kbd.keycode = e;
    event->kbd.ascii = (e>='a' && e<='z' && (event->kbd.flags & KBD_SHIFT)?
			e &~ 0x20 : e);
    return true;
  } else if(_ms_cur_x != _ms_old_x || _ms_cur_y != _ms_old_y) {
    event->event_code = EVENT_MOUSEMOVE;
    _ms_old_x = _ms_cur_x;
    _ms_old_y = _ms_cur_y;
    return true;
  } else {
    event->event_code = (EventCode)0;
    return false;
  }
}

