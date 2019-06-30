/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include <3ds.h>
#include "osystem.h"

#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "backends/events/default/default-events.h"
#include "audio/mixer_intern.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/str.h"
#include "config.h"

#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/fs/posix/posix-fs.h"
#include <unistd.h>
#include <time.h>

namespace _3DS {

OSystem_3DS::OSystem_3DS():
	_focusDirty(true),
	_focusRect(Common::Rect(1, 1)),
	_focusPosX(0),
	_focusPosY(0),
	_focusTargetPosX(0),
	_focusTargetPosY(0),
	_focusStepPosX(0),
	_focusStepPosY(0),
	_focusScaleX(1.f),
	_focusScaleY(1.f),
	_focusTargetScaleX(1.f),
	_focusTargetScaleY(1.f),
	_focusStepScaleX(0.f),
	_focusStepScaleY(0.f),
	_focusClearTime(0),
	_cursorPaletteEnabled(false),
	_cursorVisible(false),
	_cursorScalable(false),
	_cursorX(0),
	_cursorY(0),
	_cursorHotspotX(0),
	_cursorHotspotY(0),
	_gameTopX(0),
	_gameTopY(0),
	_gameBottomX(0),
	_gameBottomY(0),
	_gameWidth(320),
	_gameHeight(240),
	_overlayVisible(false),
	exiting(false),
	sleeping(false)
{
	chdir("sdmc:/");
	_fsFactory = new POSIXFilesystemFactory();
	Posix::assureDirectoryExists("/3ds/scummvm/saves/");
}

OSystem_3DS::~OSystem_3DS() {
	exiting = true;
	destroyEvents();
	destroyAudio();
	destroyGraphics();

	delete _timerManager;
	_timerManager = 0;
}

void OSystem_3DS::quit() {
	printf("OSystem_3DS::quit()\n");
}

void OSystem_3DS::initBackend() {
	loadConfig();
	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", true);
	if (!ConfMan.hasKey("vkeybd_pack_name"))
		ConfMan.set("vkeybd_pack_name", "vkeybd_small");
	if (!ConfMan.hasKey("vkeybdpath"))
		ConfMan.set("vkeybdpath", "/3ds/scummvm/kb");
	if (!ConfMan.hasKey("themepath"))
		ConfMan.set("themepath", "/3ds/scummvm");
	if (!ConfMan.hasKey("gui_theme"))
		ConfMan.set("gui_theme", "builtin");

	_timerManager = new DefaultTimerManager();
	_savefileManager = new DefaultSaveFileManager("/3ds/scummvm/saves/");

	initGraphics();
	initAudio();
	initEvents();
	EventsBaseBackend::initBackend();
}

void OSystem_3DS::updateConfig() {
	if (_gameScreen.getPixels()) {
		updateSize();
		warpMouse(_cursorX, _cursorY);
	}
}

Common::String OSystem_3DS::getDefaultConfigFileName() {
	return "/3ds/scummvm/scummvm.ini";
}

uint32 OSystem_3DS::getMillis(bool skipRecord) {
	return svcGetSystemTick() / TICKS_PER_MSEC;
}

void OSystem_3DS::delayMillis(uint msecs) {
	svcSleepThread(msecs * 1000000);
}

void OSystem_3DS::getTimeAndDate(TimeDate& td) const {
	time_t curTime = time(0);
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
	td.tm_wday = t.tm_wday;
}

OSystem::MutexRef OSystem_3DS::createMutex() {
	RecursiveLock *mutex = new RecursiveLock();
	RecursiveLock_Init(mutex);
	return (OSystem::MutexRef) mutex;
}
void OSystem_3DS::lockMutex(MutexRef mutex) {
	RecursiveLock_Lock((RecursiveLock*)mutex);
}
void OSystem_3DS::unlockMutex(MutexRef mutex) {
	RecursiveLock_Unlock((RecursiveLock*)mutex);
}
void OSystem_3DS::deleteMutex(MutexRef mutex) {
	delete (RecursiveLock*)mutex;
}

Common::String OSystem_3DS::getSystemLanguage() const {
	u8 langcode;
	CFGU_GetSystemLanguage(&langcode);
	switch (langcode) {
		case CFG_LANGUAGE_JP: return "ja_JP";
		case CFG_LANGUAGE_EN: return "en_US";
		case CFG_LANGUAGE_FR: return "fr_FR";
		case CFG_LANGUAGE_DE: return "de_DE";
		case CFG_LANGUAGE_IT: return "it_IT";
		case CFG_LANGUAGE_ES: return "es_ES";
		case CFG_LANGUAGE_ZH: return "zh_CN";
		case CFG_LANGUAGE_KO: return "ko_KR";
		case CFG_LANGUAGE_NL: return "nl_NL";
		case CFG_LANGUAGE_PT: return "pt_BR";
		case CFG_LANGUAGE_RU: return "ru_RU";
		case CFG_LANGUAGE_TW: return "zh_HK";
		default:              return "en_US";
	}
}

void OSystem_3DS::fatalError() {
	printf("FatalError!\n");
}

void OSystem_3DS::logMessage(LogMessageType::Type type, const char *message) {
	printf("3DS log: %s\n", message);
}

} // namespace _3DS
