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

#include <common/stdafx.h>
#include <common/scummsys.h>
#include "dc.h"


uint32 OSystem_Dreamcast::get_msecs()
{
  static uint32 msecs=0;
  static unsigned int t0=0;

  unsigned int t = Timer();
  unsigned int dm, dt = t - t0;

  t0 = t;
  dm = (dt << 6)/3125U;
  dt -= (dm * 3125U)>>6;
  t0 -= dt;

  return msecs += dm;
}

void OSystem_Dreamcast::delay_msecs(uint msecs)
{
  get_msecs();
  unsigned int t, start = Timer();
  int time = (((unsigned int)msecs)*100000U)>>11;
  while(((int)((t = Timer())-start))<time)
    checkSound();
  get_msecs();
}

void OSystem_Dreamcast::set_timer(TimerProc callback, int timer)
{
  if (callback != NULL) {
    _timer_duration = timer;
    _timer_next_expiry = Timer() + USEC_TO_TIMER(1000*timer);
    _timer_callback = callback;
    _timer_active = true;
  } else {
    _timer_active = false;
  }
}


/*
void waitForTimer(Scumm *s, int time)
{
  if(time<0)
    return;
  unsigned int start = Timer();
  unsigned int devpoll = start+USEC_TO_TIMER(25000);
  unsigned int t;
  int oldmousex = s->mouse.x, oldmousey = s->mouse.y;
  time = (((unsigned int)time)*100000U)>>11;
  int mask = getimask();
  while(((int)((t = Timer())-start))<time)
    if(((int)(t-devpoll))>0) {
      setimask(15);
      checkSound();
      handleInput(locked_get_pads(), s->mouse.x, s->mouse.y,
		  s->_leftBtnPressed, s->_rightBtnPressed, s->_keyPressed);
      setimask(mask);
      devpoll += USEC_TO_TIMER(17000);
      if(s->mouse.x != oldmousex || s->mouse.y != oldmousey) {
	extern void updateScreen(Scumm *s);
	updateScreen(s);
	oldmousex = s->mouse.x;
	oldmousey = s->mouse.y;
      }
    }
}
*/
