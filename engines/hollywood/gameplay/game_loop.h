/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HOLLYWOOD_GAMEPLAY_GAME_LOOP_H
#define HOLLYWOOD_GAMEPLAY_GAME_LOOP_H

#include "common/array.h"
#include "common/types.h"

#include "hollywood/gameplay/hotspots.h"

namespace Hollywood {

class HollywoodEngine;

struct GameplayLoopCursorState {
	uint16 cursorX;
	uint16 cursorY;
	uint16 sceneX;
	uint16 sceneY;
	byte currentStrip;
	byte requestedStrip;
	byte resolvedItem;
};

class GameplayLoopDelegate {
public:
	virtual ~GameplayLoopDelegate();

	virtual const SceneHotspotTable &hotspots() const = 0;
	virtual const Common::Array<byte> &savedFramebuffer() const = 0;
	virtual uint16 viewportXOffset() const = 0;
	virtual uint16 viewportYOffset() const;

	virtual void prepareGameplayLoop();
	virtual void advanceGameplayLoop(uint32 deltaMillis) = 0;
	virtual void drawGameplayFrame() = 0;
	virtual void presentGameplayFrame(const SceneHoverCaption &hoverCaption) = 0;
	virtual bool shouldExitGameplayLoop() const;
	virtual void handleLeftClick(const GameplayLoopCursorState &state);
	virtual void handleRightClick(const GameplayLoopCursorState &state);
};

class GameplayLoop {
public:
	GameplayLoop(HollywoodEngine *vm, GameplayLoopDelegate *delegate);

	bool run();
	void setCurrentStrip(byte stripIndex);

	byte currentStrip() const { return _currentStrip; }
	const SceneHoverCaption &hoverCaption() const { return _hoverCaption; }

private:
	bool pollEvents();
	void handleLeftClick();
	void handleRightClick();
	void selectNextStrip();
	GameplayLoopCursorState makeCursorState() const;
	void refreshHoverCaption();

	HollywoodEngine *_vm;
	GameplayLoopDelegate *_delegate;
	SceneHoverCaption _hoverCaption;
	byte _currentStrip;
	bool _leftButtonDown;
	bool _rightButtonDown;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAMEPLAY_GAME_LOOP_H
