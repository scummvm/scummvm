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

#ifndef XEEN_WORLDOFXEEN_WORLDOFXEEN_H
#define XEEN_WORLDOFXEEN_WORLDOFXEEN_H

#include "xeen/xeen.h"
#include "xeen/worldofxeen/worldofxeen_cutscenes.h"

namespace Xeen {
namespace WorldOfXeen {

/**
 * Implements a descendant of the base Xeen engine to handle
 * Clouds of Xeen, Dark Side of Xeen, and Worlds of Xeen specific
 * game code
 */
class WorldOfXeenEngine: public XeenEngine, public WorldOfXeenCutscenes {
protected:
	/**
	 * Show the starting sequence/intro
	 */
	virtual void showStartup();

	/**
	 * Show the startup menu
	 */
	virtual void showMainMenu();

	/**
	 * Death cutscene
	 */
	virtual void death();
public:
	WorldOfXeenEngine(OSystem *syst, const XeenGameDescription *gameDesc);
	virtual ~WorldOfXeenEngine() {}

	/**
	 * Show a cutscene
	 */
	virtual void showCutscene(const Common::String &name, int status, uint score);

	/**
	 * Dream sequence
	 */
	virtual void dream();
};

#define WOX_VM (*(::Xeen::WorldOfXeen::WorldOfXeenEngine *)g_vm)

} // End of namespace WorldOfXeen
} // End of namespace Xeen

#endif /* XEEN_WORLDOFXEEN_WORLDOFXEEN_H */
