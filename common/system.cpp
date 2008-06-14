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

/*
 * Include header files needed for the getFilesystemFactory() method.
 *
 * TODO: Remove these gradually and move the getFilesystemFactory() implementations
 * to the respective backends. Then turn it into a pure virtual method of OSystem.
 */
#if defined(PALMOS_MODE)
	#include "backends/fs/palmos/palmos-fs-factory.h"
#elif defined(__PLAYSTATION2__)
	#include "backends/fs/ps2/ps2-fs-factory.h"
#endif

FilesystemFactory *OSystem::getFilesystemFactory() {
	#if defined(__amigaos4__) || defined(__DC__) || defined(__SYMBIAN32__) || defined(UNIX) || defined(WIN32) || defined(__WII__) || defined(__PSP__) || defined(__DS__)
		// These ports already implement this function, so it should never be called.
		return 0;
	#elif defined(PALMOS_MODE)
		return &PalmOSFilesystemFactory::instance();
	#elif defined(__PLAYSTATION2__)
		return &Ps2FilesystemFactory::instance();
	#else
		#error Unknown and unsupported backend in OSystem::getFilesystemFactory
	#endif
}
