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
#include "mididrv.h"
#include "gameDetector.h"
#include "dc.h"
#include "icon.h"


Icon icon;


OSystem *OSystem_Dreamcast_create() {
	return OSystem_Dreamcast::create();
}

OSystem *OSystem_Dreamcast::create() {
	OSystem_Dreamcast *syst = new OSystem_Dreamcast();
	return syst;
}

/* CD Audio */
static bool find_track(int track, int &first_sec, int &last_sec)
{
  struct TOC *toc = cdfs_gettoc();
  if(!toc)
    return false;
  int i, first, last;
  first = TOC_TRACK(toc->first);
  last = TOC_TRACK(toc->last);
  if(first < 1 || last > 99 || first > last)
    return false;
  for(i=last; i>=first; --i)
    if(!(TOC_CTRL(toc->entry[i-1])&4))
      if(track==1) {
	first_sec = TOC_LBA(toc->entry[i-1]);
	last_sec = TOC_LBA(toc->entry[i]);
	return true;
      } else
	--track;
  return false;
}

void OSystem_Dreamcast::play_cdrom(int track, int num_loops,
				   int start_frame, int end_frame)
{
  int first_sec, last_sec;
#if 1
  if(num_loops)
    --num_loops;
#endif
  if(num_loops>14) num_loops=14;
  else if(num_loops<0) num_loops=15; // infinity
  if(!find_track(track, first_sec, last_sec))
    return;
  if(end_frame)
    last_sec = first_sec + start_frame + end_frame;
  first_sec += start_frame;
  play_cdda_sectors(first_sec, last_sec, num_loops);
}

void OSystem_Dreamcast::stop_cdrom()
{
  stop_cdda();
}

bool OSystem_Dreamcast::poll_cdrom()
{
  extern int getCdState();
  return getCdState() == 3;
}

void OSystem_Dreamcast::update_cdrom()
{
  // Dummy.  The CD drive takes care of itself.
}

uint32 OSystem_Dreamcast::property(int param, uint32 value)
{
  switch(param) {

  case PROP_GET_SAMPLE_RATE:
    return 22050;

  }
  
  return 0;
}

void OSystem_Dreamcast::quit() {
  exit(0);
}

void *OSystem_Dreamcast::create_thread(ThreadProc *proc, void *param) {
  warning("Creating a thread! (not supported.)\n");
}

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

int dc_setup(GameDetector &detector)
{
  static char *argv[] = { "scummvm", NULL, NULL, NULL };
  static int argc = 3;

#ifndef NOSERIAL
  serial_init(57600);
  usleep(2000000);
  printf("Serial OK\r\n");
#endif

  cdfs_init();
  maple_init();
  dc_setup_ta();
  init_arm();

  initSound();

  if(!selectGame(&detector, argv[2], argv[1], icon))
    exit(0);

  //  sound.initialize(&scumm, &snd_driv);

  detector.parseCommandLine(argc, argv);
}
