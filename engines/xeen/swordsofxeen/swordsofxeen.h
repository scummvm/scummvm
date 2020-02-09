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

#ifndef XEEN_SWORDSOFXEEN_SWORDSOFXEEN_H
#define XEEN_SWORDSOFXEEN_SWORDSOFXEEN_H

#include "xeen/xeen.h"
#include "xeen/worldofxeen/worldofxeen_cutscenes.h"

namespace Xeen {
namespace SwordsOfXeen {

/**
 * Implements a descendant of the base Xeen engine to handle
 * Swords of Xeen specific game code
 */
class SwordsOfXeenEngine: public XeenEngine {
private:
	/**
	 * Show the ending "You won" screen
	 */
	bool showEnding();
protected:
	/**
	 * Show the starting sequence/intro
	 */
	void showStartup() override { 
		// Swords of Xeen doesn't have a starting title or intro
		_gameMode = GMODE_MENU;
	}

	/**
	 * Show the startup menu
	 */
	void showMainMenu() override;

	/**
	 * Death cutscene
	 */
	void death() override;
public:
	SwordsOfXeenEngine(OSystem *syst, const XeenGameDescription *gameDesc);
	~SwordsOfXeenEngine() override {}

	/**
	 * Show a cutscene
	 */
	void showCutscene(const Common::String &name, int status, uint score) override;

	/**
	 * Dream sequence
	 */
	void dream() override;
};

#define SWORDS_VM (*(::Xeen::SwordsOfXeen::SwordsOfXeenEngine *)g_vm)

} // End of namespace SwordsOfXeen
} // End of namespace Xeen

#endif /* XEEN_SWORDSOFXEEN_SWORDSOFXEEN_H */
