/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <common/scummsys.h>
#include <engines/engine.h>
#include <base/main.h>
#include <base/plugins.h>
#include "dc.h"
#include "dcutils.h"
#include "icon.h"
#include "DCLauncherDialog.h"
#include "backends/mutex/null/null-mutex.h"
#include <common/config-manager.h>
#include <common/memstream.h>
#include <common/endian.h>

#include "audio/mixer_intern.h"


Icon icon;
char gGameName[32];


OSystem_Dreamcast::OSystem_Dreamcast()
  : _devpoll(0), screen(NULL), mouse(NULL), overlay(NULL), _softkbd(this),
	_ms_buf(NULL), _mixer(NULL),
	_current_shake_x_pos(0), _current_shake_y_pos(0), _aspect_stretch(false), _softkbd_on(false),
	_softkbd_motion(0), _enable_cursor_palette(false), _overlay_in_gui(false), _screenFormat(0)
{
  memset(screen_tx, 0, sizeof(screen_tx));
  memset(mouse_tx, 0, sizeof(mouse_tx));
  memset(ovl_tx, 0, sizeof(ovl_tx));
  _fsFactory = this;
}

void OSystem_Dreamcast::initBackend()
{
  ConfMan.setInt("autosave_period", 0);
  _savefileManager = createSavefileManager();
  _timerManager = new DefaultTimerManager();

  uint sampleRate = initSound();
  _mixer = new Audio::MixerImpl(sampleRate);
  _mixer->setReady(true);

  _audiocdManager = new DCCDManager();

  EventsBaseBackend::initBackend();
}


/* CD Audio */
static bool find_track(int track, int &first_sec, int &last_sec)
{
  struct TOC *toc = cdfs_gettoc();
  if (!toc)
	return false;
  int i, first, last;
  first = TOC_TRACK(toc->first);
  last = TOC_TRACK(toc->last);
  if (first < 1 || last > 99 || first > last)
	return false;
  for (i=first; i<=last; i++)
	if (!(TOC_CTRL(toc->entry[i-1])&4)) {
	  if (track==1) {
	first_sec = TOC_LBA(toc->entry[i-1]);
	last_sec = TOC_LBA(toc->entry[i]);
	return true;
	  } else
	--track;
	}
  return false;
}

bool DCCDManager::play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate,
		Audio::Mixer::SoundType soundType) {
	// Prefer emulation
	if (DefaultAudioCDManager::play(track, numLoops, startFrame, duration, onlyEmulate, soundType))
		return true;

	// If we're playing now return here
	if (isPlaying()) {
		return true;
	}

	// If we should only play emulated tracks stop here.
	if (onlyEmulate) {
		return false;
	}

	int firstSec, lastSec;
#if 1
	if (numLoops)
		--numLoops;
#endif

	if (numLoops > 14)
		numLoops = 14;
	else if (numLoops < 0)
		numLoops = 15; // infinity

	if (!find_track(track, firstSec, lastSec))
		return false;

	if (duration)
		lastSec = firstSec + startFrame + duration;

	firstSec += startFrame;
	play_cdda_sectors(firstSec, lastSec, numLoops);

	return true;
}

void DCCDManager::stop() {
	DefaultAudioCDManager::stop();
	stop_cdda();
}

bool DCCDManager::isPlaying() const {
	if (DefaultAudioCDManager::isPlaying())
		return true;

	return getCdState() == 3;
}

void OSystem_Dreamcast::setWindowCaption(const Common::U32String &caption)
{
  Common::strlcpy(gGameName, caption.encode(Common::kISO8859_1).c_str(), 32);
}

void OSystem_Dreamcast::quit() {
  (*(void(**)(int))0x8c0000e0)(0);
}

/* Mutex handling */
Common::MutexInternal *OSystem_Dreamcast::createMutex()
{
  return new NullMutexInternal();
}

/* Features */
bool OSystem_Dreamcast::hasFeature(Feature f)
{
  switch(f) {
  case kFeatureAspectRatioCorrection:
  case kFeatureVirtualKeyboard:
  case kFeatureOverlaySupportsAlpha:
  case kFeatureCursorPalette:
  case kFeatureCursorAlpha:
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
	if (screen)
	  setScaling();
	break;
  case kFeatureVirtualKeyboard:
	_softkbd_on = enable;
	break;
  case kFeatureCursorPalette:
	_enable_cursor_palette = enable;
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
  case kFeatureCursorPalette:
	return _enable_cursor_palette;
  default:
	return false;
  }
}

void OSystem_Dreamcast::getTimeAndDate(TimeDate &td, bool skipRecord) const {
  time_t curTime;
  time(&curTime);
  struct tm t = *localtime(&curTime);
  td.tm_sec = t.tm_sec;
  td.tm_min = t.tm_min;
  td.tm_hour = t.tm_hour;
  td.tm_mday = t.tm_mday;
  td.tm_mon = t.tm_mon;
  td.tm_year = t.tm_year;
  td.tm_wday = t.tm_wday;
}

Common::SeekableReadStream *OSystem_Dreamcast::createConfigReadStream() {
  Common::FSNode file("/scummvm.ini");
  Common::SeekableReadStream *s = file.createReadStream();
  return s? s : new Common::MemoryReadStream((const byte *)"", 0);
}

Common::WriteStream *OSystem_Dreamcast::createConfigWriteStream() {
  return 0;
}

void OSystem_Dreamcast::logMessage(LogMessageType::Type type, const char *message) {
#ifndef NOSERIAL
  report(message);
#endif
}

Common::String OSystem_Dreamcast::getSystemLanguage() const {
  static const char *languages[] = {
	"ja_JP",
	"en_US",
	"de_DE",
	"fr_FR",
	"es_ES",
	"it_IT"
  };
  int l = DC_Flash::get_locale_setting();
  if (l<0 || ((unsigned)l)>=sizeof(languages)/sizeof(languages[0]))
	l = 1;
  return Common::String(languages[l]);
}


void DCHardware::dc_init_hardware()
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

static OSystem_Dreamcast osys_dc;

int main()
{
  static const char *argv[] = { "scummvm", NULL, };
  static int argc = 1;

  g_system = &osys_dc;

#ifdef DYNAMIC_MODULES
  PluginManager::instance().addPluginProvider(&osys_dc);
#endif

  scummvm_main(argc, argv);

  g_system->quit();
}

int DCLauncherDialog::runModal()
{
  char *engineId = NULL, *gameId = NULL, *dir = NULL;
  Common::Language language = Common::UNK_LANG;
  Common::Platform platform = Common::kPlatformUnknown;

  if (!selectGame(engineId, gameId, dir, language, platform, icon))
	g_system->quit();

  // Set the game path.
  ConfMan.addGameDomain(gameId);
  ConfMan.set("engineid", engineId, gameId);
  ConfMan.set("gameid", gameId, gameId);

  if (dir != NULL)
	ConfMan.set("path", dir, gameId);

  // Set the game language.
  if (language != Common::UNK_LANG)
	ConfMan.set("language", Common::getLanguageCode(language), gameId);

  // Set the game platform.
  if (platform != Common::kPlatformUnknown)
	ConfMan.set("platform", Common::getPlatformCode(platform), gameId);

  // Set the target.
  ConfMan.setActiveDomain(gameId);

  return 0;
}
