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

#include "graphics/colormasks.h"
#include "gui/message.h"

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
	return Graphics::RGBToColor<Graphics::ColorMasks<565> >(r, g, b);
}

void OSystem::colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b) {
	Graphics::colorToRGB<Graphics::ColorMasks<565> >(color, r, g, b);
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
	if (screen && screen->pixels)
		memset(screen->pixels, 0, screen->h * screen->pitch);
	unlockScreen();
}


/*
 FIXME: Maybe we should push the default config file loading/saving code below
 out to all the backends?
*/


#if defined(UNIX)
#define DEFAULT_CONFIG_FILE ".scummvmrc"
#else
#define DEFAULT_CONFIG_FILE "scummvm.ini"
#endif

Common::SeekableReadStream *OSystem::openConfigFileForReading() {
	Common::FSNode file(DEFAULT_CONFIG_FILE);
	return file.openForReading();
}

Common::WriteStream *OSystem::openConfigFileForWriting() {
#ifdef __DC__
	return 0;
#else
	Common::FSNode file(DEFAULT_CONFIG_FILE);
	return file.openForWriting();
#endif
}
