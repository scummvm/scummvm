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
 * Clouds of Xeen, Darkside of Xeen, and Worlds of Xeen specific
 * game code
 */
class WorldOfXeenEngine: public XeenEngine, public WorldOfXeenCutscenes {
protected:
	/**
	 * Show the starting sequence/intro
	 */
	void showStartup() override;

	/**
	 * Show the startup menu
	 */
	void showMainMenu() override;

	/**
	 * Death cutscene
	 */
	void death() override;
public:
	WorldOfXeenEngine(OSystem *syst, const XeenGameDescription *gameDesc);
	~WorldOfXeenEngine() override {}

	/**
	 * Show a cutscene
	 */
	void showCutscene(const Common::String &name, int status, uint score) override;

	/**
	 * Dream sequence
	 */
	void dream() override;
};

#define WOX_VM (*(::Xeen::WorldOfXeen::WorldOfXeenEngine *)g_vm)

} // End of namespace WorldOfXeen
} // End of namespace Xeen

#endif /* XEEN_WORLDOFXEEN_WORLDOFXEEN_H */
