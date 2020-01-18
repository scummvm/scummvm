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

#ifndef ULTIMA_SHARED_EARLY_GAME_BASE_H
#define ULTIMA_SHARED_EARLY_GAME_BASE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "ultima/shared/engine/events.h"
#include "ultima/shared/core/game_state.h"
#include "ultima/shared/engine/input_handler.h"
#include "ultima/shared/engine/input_translator.h"

namespace Ultima {
namespace Shared {

class UltimaEngine;

namespace Gfx {
	class VisualItem;
}

class GameBase : public TreeItem, public EventTarget {
private:
	int _pendingLoadSlot;
	uint32 _priorLeftDownTime;
	uint32 _priorMiddleDownTime;
	uint32 _priorRightDownTime;
	Gfx::VisualItem *_currentView;
	Shared::GameState _gameState;
	InputHandler _inputHandler;
	InputTranslator _inputTranslator;
private:
	/**
	 * Checks for the presence of any savegames and, if present,
	 * lets the user pick one to resume
	 */
	int getSavegameSlot();

	void leftButtonDoubleClick(const Common::Point &mousePos);
	void middleButtonDoubleClick(const Common::Point &mousePos);
	void rightButtonDoubleClick(const Common::Point &mousePos);

	/**
	 * Returns true if the player can control the mouse
	 */
	bool isMouseControlEnabled() const { return true; }

	void changeView(const Common::String &name);
public:
	/**
	 * Constructor
	 */
	GameBase();
	
	/**
	 * Destructor
	 */
	virtual ~GameBase() {}

	/**
	 * Called to handle any regular updates the game requires
	 */
	virtual void onIdle();

	virtual void mouseMove(const Common::Point &mousePos);
	virtual void leftButtonDown(const Common::Point &mousePos);
	virtual void leftButtonUp(const Common::Point &mousePos);
	virtual void middleButtonDown(const Common::Point &mousePos);
	virtual void middleButtonUp(const Common::Point &mousePos);
	virtual void rightButtonDown(const Common::Point &mousePos);
	virtual void rightButtonUp(const Common::Point &mousePos);
	virtual void mouseWheel(const Common::Point &mousePos, bool wheelUp);
	virtual void keyDown(Common::KeyState keyState);

	/**
	 * Called when the game starts
	 */
	virtual void starting();

	/**
	 * Main draw method for the game
	 */
	void draw();

	/**
	 * Called once every frame to update the game
	 */
	void update();

	/**
	 * Called by the event handler when a mouse event has been generated
	 */
	void mouseChanged();

	/**
	 * Set the currently active view to display
	 */
	void setView(Gfx::VisualItem *view);

	/**
	 * Set the currently active view to display
	 */
	void setView(const Common::String &viewName);

	/**
	 * Returns the current view
	 */
	Gfx::VisualItem *getView() const { return _currentView; }
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
