/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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

#include "backends/intern.h"

#include "base/gameDetector.h"

#include "common/config-manager.h"
#include "common/system.h"

static OSystem *s_system = 0;

static OSystem *createSystem() {
	// Attention: Do not call parseGraphicsMode() here, nor any other function
	// which needs to access the OSystem instance, else you get stuck in an
	// endless loop.

#if defined(USE_NULL_DRIVER)
	return OSystem_NULL_create();
#elif defined(__DC__)
	return OSystem_Dreamcast_create();
#elif defined(X11_BACKEND)
	return OSystem_X11_create();
#elif defined(__MORPHOS__)
	return OSystem_MorphOS_create(gfx_mode, ConfMan.getBool("fullscreen"));
#elif defined(_WIN32_WCE)
	return OSystem_WINCE3_create();
#elif defined(__GP32__)	// ph0x
	return OSystem_GP32_create(GFX_NORMAL, true);
#elif defined(__PALM_OS__) //chrilith
	return OSystem_PALMOS_create();
#else
	/* SDL is the default driver for now */
	return OSystem_SDL_create();
#endif
}

OSystem *OSystem::instance() {
	if (!s_system)
		s_system = createSystem();
	return s_system;
}


bool OSystem::setGraphicsMode(const char *name) {
	if (!name)
		return false;

	const GraphicsMode *gm = getSupportedGraphicsModes();

	// Sepcial case for the 'default' filter
	if (!scumm_stricmp(name, "normal") || !scumm_stricmp(name, "default")) {
#ifdef _WIN32_WCE
		name = "1x";
#else
		name = "2x";
#endif
	}

	while (gm->name) {
		if (!scumm_stricmp(gm->name, name)) {
			return setGraphicsMode(gm->id);
		}
		gm++;
	}

	return false;
}
