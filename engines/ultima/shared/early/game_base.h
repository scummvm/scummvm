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
#include "common/serializer.h"
#include "ultima/shared/engine/events.h"
#include "ultima/shared/engine/input_handler.h"
#include "ultima/shared/engine/input_translator.h"

namespace Ultima {
namespace Shared {

class UltimaEngine;
class GameState;

namespace Gfx {
class Font;
class Popup;
class TextCursor;
class TextInput;
class VisualItem;
} // End of namespace Gfx

namespace Maps {
class Map;
} // End of namespace Maps

/**
 * Base class for the game implementations
 */
class GameBase : public TreeItem, public EventTarget {
private:
	/**
	 * Checks for the presence of any savegames and, if present,
	 * lets the user pick one to resume
	 */
	int getSavegameSlot();

	void leftButtonDoubleClick(const Point &mousePos) override;
	void middleButtonDoubleClick(const Point &mousePos) override;
	void rightButtonDoubleClick(const Point &mousePos);

	/**
	 * Returns true if the player can control the mouse
	 */
	bool isMouseControlEnabled() const { return true; }

	void changeView(const Common::String &name);
protected:
	uint32 _priorLeftDownTime;
	uint32 _priorMiddleDownTime;
	uint32 _priorRightDownTime;
	Gfx::VisualItem *_currentView;
	Gfx::Popup *_pendingPopup;
	InputHandler _inputHandler;
	InputTranslator _inputTranslator;
	Gfx::Font *_font;
public:
	Gfx::TextCursor *_textCursor;
	uint _videoMode;
public:
	/**
	 * Constructor
	 */
	GameBase();

	/**
	 * Destructor
	 */
	~GameBase() override;

	/**
	 * Called to handle any regular updates the game requires
	 */
	void onIdle() override;

	void mouseMove(const Point &mousePos) override;
	void leftButtonDown(const Point &mousePos) override;
	void leftButtonUp(const Point &mousePos) override;
	void middleButtonDown(const Point &mousePos) override;
	void middleButtonUp(const Point &mousePos) override;
	void rightButtonDown(const Point &mousePos) override;
	void rightButtonUp(const Point &mousePos) override;
	void mouseWheel(const Point &mousePos, bool wheelUp) override;
	void keyDown(Common::KeyState keyState) override;

	/**
	 * Called when the game starts
	 */
	virtual void starting(bool isLoading) {}

	/**
	 * Returns true if the current video mode is VGA
	 */
	virtual bool isVGA() const { return false; }

	/**
	 * Called once every frame to update the game and render the view
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
	 * Sets a popup to be shown
	 */
	void setPopup(Gfx::Popup *popup);

	/**
	 * Returns the current view
	 */
	Gfx::VisualItem *getView() const { return _currentView; }

	/**
	 * Set a font to use
	 */
	void setFont(Gfx::Font *font);

	/**
	 * Returns the current font
	 */
	Gfx::Font *getFont() const { return _font; }

	/**
	 * Returns the map
	 */
	virtual Maps::Map *getMap() const { return nullptr; }

	/**
	 * Gets a random number
	 */
	uint getRandomNumber(uint max);

	/**
	 * Gets a random number
	 */
	uint getRandomNumber(uint min, uint max);

	/**
	 * Wait for a given period of time
	 */
	void sleep(uint time);

	/**
	 * Return the current time
	 */
	uint32 getMillis() const;

	/**
	 * Returns true if a savegame can currently be loaded
	 */
	virtual bool canLoadGameStateCurrently() { return true; }

	/**
	 * Returns true if the game can currently be saved
	 */
	virtual bool canSaveGameStateCurrently() { return false; }

	/**
	 * Handles loading and saving games
	 */
	virtual void synchronize(Common::Serializer &s);
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
