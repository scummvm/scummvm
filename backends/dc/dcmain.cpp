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


#define REAL_MAIN

#include <common/scummsys.h>
#include <common/stdafx.h>
#include <common/engine.h>
#include "dc.h"
#include "icon.h"


Icon icon;
const char *gGameName;

OSystem *OSystem_Dreamcast_create() {
	return OSystem_Dreamcast::create();
}

OSystem *OSystem_Dreamcast::create() {
	OSystem_Dreamcast *syst = new OSystem_Dreamcast();
	return syst;
}

OSystem_Dreamcast::OSystem_Dreamcast()
  : screen(NULL), mouse(NULL), overlay(NULL), _ms_buf(NULL),
    _sound_proc(NULL), _timer_active(false)
{
  memset(screen_tx, 0, sizeof(screen_tx));
  memset(mouse_tx, 0, sizeof(screen_tx));
  memset(ovl_tx, 0, sizeof(screen_tx));
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

uint32 OSystem_Dreamcast::property(int param, Property *value)
{
  switch(param) {

  case PROP_GET_SAMPLE_RATE:
    return 22050;

  case PROP_SET_WINDOW_CAPTION:
    gGameName = value->caption;
    break;

  }
  
  return 0;
}

void OSystem_Dreamcast::quit() {
  exit(0);
}

void OSystem_Dreamcast::create_thread(ThreadProc *proc, void *param) {
  warning("Creating a thread! (not supported.)\n");
}


/* Mutex handling */
void *OSystem_Dreamcast::create_mutex()
{
  return NULL;
}

void OSystem_Dreamcast::lock_mutex(void *mutex)
{
}
 
void OSystem_Dreamcast::unlock_mutex(void *mutex)
{
}

void OSystem_Dreamcast::delete_mutex(void *mutex)
{
}


void dc_init_hardware()
{
#ifndef NOSERIAL
  serial_init(57600);
  usleep(2000000);
  printf("Serial OK\r\n");
#endif

  cdfs_init();
  maple_init();
  dc_setup_ta();
  init_arm();
}

int main()
{
  extern int scumm_main(int argc, char *argv[]);

  static char *argv[] = { "scummvm", NULL, NULL, NULL };
  static int argc = 3;

  dc_init_hardware();
  initSound();

  if(!selectGame(argv[2], argv[1], icon))
    exit(0);

  scumm_main(argc, argv);

  exit(0);
}
