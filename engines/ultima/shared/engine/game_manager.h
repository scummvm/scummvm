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

#ifndef ULTIMA_SHARED_ENGINE_GAME_MANAGER_H
#define ULTIMA_SHARED_ENGINE_GAME_MANAGER_H

#include "common/scummsys.h"
#include "audio/mixer.h"
#include "ultima/shared/engine/game_state.h"
#include "ultima/shared/engine/input_handler.h"
#include "ultima/shared/engine/input_translator.h"
#include "ultima/shared/gfx/visual_item.h"

namespace Ultima {
namespace Shared {

class ProjectItem;

class GameManager {
private:
	int _transitionCtr;
public:
	ProjectItem *_project;
	GameState _gameState;
	InputHandler _inputHandler;
	InputTranslator _inputTranslator;
	Gfx::VisualItem *_view;
public:
	GameManager(ProjectItem *project, Audio::Mixer *mixer);
	~GameManager();

	/**
	 * Lock the input handler
	 */
	void lockInputHandler() { _inputHandler.incLockCount(); }

	/**
	 * Unlock the input handler
	 */
	void unlockInputHandler() { _inputHandler.decLockCount(); }

	/**
	 * Main frame update method for the game
	 */
	void update();

	/**
	 * Called when the visual item being viewed changes
	 */
	void viewChange();

	/**
	 * Returns true if no transition is currently in progress
	 */
	bool isntTransitioning() const { return !_transitionCtr; }

	/**
	 * Increments the number of active transitions
	 */
	void incTransitions() { ++_transitionCtr; }

	/**
	 * Decrements the number of active transitions
	 */
	void decTransitions() { --_transitionCtr; }

	/**
	 * Sets the currently active visual item
	 */
	void setView(Gfx::VisualItem *view) { _view = view; }

	/**
	 * Returns the currently active visual item
	 */
	Gfx::VisualItem *getView() const { return _view; }

	/**
	 * Changes to a different view
	 */
	void changeView(const Common::String &name);
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
