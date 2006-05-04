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

#ifndef BASE_GAMEDETECTOR_H
#define BASE_GAMEDETECTOR_H

#include "common/str.h"
#include "common/config-manager.h"
#include "base/game.h"

class Plugin;


class GameDetector {
	typedef Common::String String;

public:
	GameDetector();

	static Common::String parseCommandLine(Common::StringMap &settings, int argc, char **argv);
	static void processSettings(Common::String &target, Common::StringMap &settings);
	static const Plugin *detectMain();

public:
	static GameDescriptor findGame(const String &gameName, const Plugin **plugin = NULL);

//protected:
	static void setTarget(const String &name);	// TODO: This should be protected
};

#endif
