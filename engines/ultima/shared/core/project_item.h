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

#ifndef ULTIMA_SHARED_CORE_PROJECT_ITEM_H
#define ULTIMA_SHARED_CORE_PROJECT_ITEM_H

#include "common/scummsys.h"
#include "common/str.h"
#include "engines/savestate.h"
#include "graphics/surface.h"
#include "ultima/shared/core/file.h"
#include "ultima/shared/core/named_item.h"

namespace Ultima {
namespace Shared {

class GameManager;

class ProjectItem : public NamedItem {
	DECLARE_MESSAGE_MAP;
private:
	GameManager *_gameManager;

	/**
	 * Called at the beginning of loading a game
	 */
	void preLoad();

	/**
	 * Does post-loading processing
	 */
	void postLoad();

	/**
	 * Called when a game is about to be saved
	 */
	void preSave();

	/**
	 * Called when a game has finished being saved
	 */
	void postSave();
private:
public:
	CLASSDEF;
	ProjectItem();
	virtual ~ProjectItem() {
		destroyChildren();
	}

	/**
	 * Sets the game manager for the project, if not already set
	 */
	void setGameManager(GameManager *gameManager);

	/**
	 * Resets the game manager field
	 */
	void resetGameManager();

	/**
	 * Load the entire project data for a given slot Id
	 */
	void loadGame(int slotId);

	/**
	 * Save the entire project data to a given savegame slot
	 */
	void saveGame(int slotId, const Common::String &desc);

	/**
	 * Clear any currently loaded project
	 */
	void clear();
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
