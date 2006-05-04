/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "backends/intern.h"

#include "gui/message.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/util.h"

OSystem *g_system = 0;

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

void OSystem::displayMessageOnOSD(const char *msg) {
	// Display the message for 1.5 seconds
	GUI::TimedMessageDialog dialog(msg, 1500);
	dialog.runModal();
}

Common::SaveFileManager *OSystem::getSavefileManager() {
	return new Common::DefaultSaveFileManager();
}
