/* ScummVM - Scumm Interpreter
 * Dreamcast port
 * Copyright (C) 2002-2004  Marcus Comstedt
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */


#define REAL_MAIN

#include <common/stdafx.h>
#include <common/scummsys.h>
#include <base/engine.h>
#include <base/gameDetector.h>
#include <base/plugins.h>
#include "dc.h"
#include "icon.h"
#include "DCLauncherDialog.h"
#include <common/config-manager.h>


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
  : screen(NULL), mouse(NULL), overlay(NULL), _softkbd(this), _ms_buf(NULL),
    _sound_proc(NULL), _timer_active(false), _current_shake_pos(0),
    _aspect_stretch(false), _softkbd_on(false), _softkbd_motion(0),
    _enable_cursor_palette(false)
{
  memset(screen_tx, 0, sizeof(screen_tx));
  memset(mouse_tx, 0, sizeof(mouse_tx));
  memset(ovl_tx, 0, sizeof(ovl_tx));
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
  for(i=first; i<=last; i++)
    if(!(TOC_CTRL(toc->entry[i-1])&4))
      if(track==1) {
	first_sec = TOC_LBA(toc->entry[i-1]);
	last_sec = TOC_LBA(toc->entry[i]);
	return true;
      } else
	--track;
  return false;
}

void OSystem_Dreamcast::playCD(int track, int num_loops, int start_frame, int duration)
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
  if(duration)
    last_sec = first_sec + start_frame + duration;
  first_sec += start_frame;
  play_cdda_sectors(first_sec, last_sec, num_loops);
}

void OSystem_Dreamcast::stopCD()
{
  stop_cdda();
}

bool OSystem_Dreamcast::pollCD()
{
  extern int getCdState();
  return getCdState() == 3;
}

void OSystem_Dreamcast::updateCD()
{
  // Dummy.  The CD drive takes care of itself.
}

bool OSystem_Dreamcast::openCD(int drive)
{
  // Dummy.
  return true;
}

void OSystem_Dreamcast::setWindowCaption(const char *caption)
{
  gGameName = caption;
}

void OSystem_Dreamcast::quit() {
  exit(0);
}

/* Mutex handling */
OSystem::MutexRef OSystem_Dreamcast::createMutex()
{
  return NULL;
}

void OSystem_Dreamcast::lockMutex(MutexRef mutex)
{
}

void OSystem_Dreamcast::unlockMutex(MutexRef mutex)
{
}

void OSystem_Dreamcast::deleteMutex(MutexRef mutex)
{
}


/* Features */
bool OSystem_Dreamcast::hasFeature(Feature f)
{
  switch(f) {
  case kFeatureAspectRatioCorrection:
  case kFeatureVirtualKeyboard:
  case kFeatureOverlaySupportsAlpha:
  case kFeatureCursorHasPalette:
    return true;
  default:
    return false;
  }
}

void OSystem_Dreamcast::setFeatureState(Feature f, bool enable)
{
  switch(f) {
  case kFeatureAspectRatioCorrection:
    _aspect_stretch = enable;
    if(screen)
      setScaling();
    break;
  case kFeatureVirtualKeyboard:
    _softkbd_on = enable;
    break;
  default:
    break;
  }
}

bool OSystem_Dreamcast::getFeatureState(Feature f)
{
  switch(f) {
  case kFeatureAspectRatioCorrection:
    return _aspect_stretch;
  case kFeatureVirtualKeyboard:
    return _softkbd_on;
  default:
    return false;
  }
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

extern "C" int scummvm_main(int argc, char *argv[]);
int main()
{
  static char *argv[] = { "scummvm", NULL, };
  static int argc = 1;

  dc_init_hardware();
  initSound();

  scummvm_main(argc, argv);

  exit(0);
}

int DCLauncherDialog::runModal()
{
  char *base = NULL, *dir = NULL;

  if(!selectGame(base, dir, icon))
    exit(0);

  // Set the game path.
  if(dir != NULL)
    ConfMan.set("path", dir, Common::ConfigManager::kTransientDomain);

  // Set the target.
  _detector.setTarget(base);

  return 0;
}

