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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "backends/events/default/default-events.h"
#include "backends/fs/fs-factory.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/util.h"

#include "graphics/colormasks.h"
#include "gui/message.h"
#include "sound/mixer.h"

#include <time.h>

OSystem *g_system = 0;

OSystem::OSystem() {
}

OSystem::~OSystem() {
}

bool OSystem::setGraphicsMode(const char *name) {
	if (!name)
		return false;

	// Special case for the 'default' filter
	if (!scumm_stricmp(name, "normal") || !scumm_stricmp(name, "default")) {
		return setGraphicsMode(getDefaultGraphicsMode());
	}

	const GraphicsMode *gm = getSupportedGraphicsModes();

	while (gm->name) {
		if (!scumm_stricmp(gm->name, name)) {
			return setGraphicsMode(gm->id);
		}
		gm++;
	}

	return false;
}

OverlayColor OSystem::RGBToColor(uint8 r, uint8 g, uint8 b) {
	return ::RGBToColor<ColorMasks<565> >(r, g, b);
}

void OSystem::colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b) {
	::colorToRGB<ColorMasks<565> >(color, r, g, b);
}

OverlayColor OSystem::ARGBToColor(uint8 a, uint8 r, uint8 g, uint8 b) {
	return RGBToColor(r, g, b);
}

void OSystem::colorToARGB(OverlayColor color, uint8 &a, uint8 &r, uint8 &g, uint8 &b) {
	colorToRGB(color, r, g, b);
	a = 255;
}

void OSystem::displayMessageOnOSD(const char *msg) {
	// Display the message for 1.5 seconds
	GUI::TimedMessageDialog dialog(msg, 1500);
	dialog.runModal();
}


bool OSystem::openCD(int drive) {
	return false;
}

bool OSystem::pollCD() {
	return false;
}

void OSystem::playCD(int track, int num_loops, int start_frame, int duration) {
}

void OSystem::stopCD() {
}

void OSystem::updateCD() {
}

static Common::EventManager *s_eventManager = 0;

Common::EventManager *OSystem::getEventManager() {
	// FIXME/TODO: Eventually this method should be turned into an abstract one,
	// to force backends to implement this conciously (even if they
	// end up returning the default event manager anyway).
	if (!s_eventManager)
		s_eventManager = new DefaultEventManager(this);
	return s_eventManager;
}

void OSystem::clearScreen() {
	Graphics::Surface *screen = lockScreen();
	memset(screen->pixels, 0, screen->h * screen->pitch);
	unlockScreen();
}

void OSystem::getTimeAndDate(struct tm &t) const {
	time_t curTime = time(0);
	t = *localtime(&curTime);
}

/*
 * All the following includes choose, at compile time, which specific backend will be used
 * during the execution of the ScummVM.
 *
 * TODO: Remove these gradually and instead move the getFilesystemFactory() implementatios
 * to the respective backends.
 */
#if defined(__amigaos4__)
	#include "backends/fs/amigaos4/amigaos4-fs-factory.cpp"
#elif defined(__DC__)
	#include "backends/fs/dc/ronincd-fs-factory.cpp"
#elif defined(__DS__)
	#include "backends/fs/ds/ds-fs-factory.cpp"
#elif defined(__GP32__)
	#include "backends/fs/gp32/gp32-fs-factory.cpp"
#elif defined(__MORPHOS__)
	#include "backends/fs/morphos/abox-fs-factory.cpp"
#elif defined(PALMOS_MODE)
	#include "backends/fs/palmos/palmos-fs-factory.cpp"
#elif defined(__PLAYSTATION2__)
	#include "backends/fs/ps2/ps2-fs-factory.cpp"
#elif defined(__PSP__)
	#include "backends/fs/psp/psp-fs-factory.cpp"
#elif defined(__SYMBIAN32__)
	#include "backends/fs/symbian/symbian-fs-factory.cpp"
#elif defined(UNIX)
	#include "backends/fs/posix/posix-fs-factory.cpp"
#elif defined(WIN32)
	#include "backends/fs/windows/windows-fs-factory.cpp"
#endif

FilesystemFactory *OSystem::getFilesystemFactory() {
	#if defined(__amigaos4__)
		return &AmigaOSFilesystemFactory::instance();
	#elif defined(__DC__)
		return &RoninCDFilesystemFactory::instance();
	#elif defined(__DS__)
		return &DSFilesystemFactory::instance();
	#elif defined(__GP32__)
		return &GP32FilesystemFactory::instance();
	#elif defined(__MORPHOS__)
		return &ABoxFilesystemFactory::instance();
	#elif defined(PALMOS_MODE)
		return &PalmOSFilesystemFactory::instance();
	#elif defined(__PLAYSTATION2__)
		return &Ps2FilesystemFactory::instance();
	#elif defined(__PSP__)
		return &PSPFilesystemFactory::instance();
	#elif defined(__SYMBIAN32__)
		return &SymbianFilesystemFactory::instance();
	#elif defined(UNIX)
		return &POSIXFilesystemFactory::instance();
	#elif defined(WIN32)
		return &WindowsFilesystemFactory::instance();
	#else
		#error Unknown and unsupported backend in OSystem::getFilesystemFactory
	#endif
}