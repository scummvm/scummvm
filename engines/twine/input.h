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

#ifndef TWINE_KEYBOARD_H
#define TWINE_KEYBOARD_H

#include "common/events.h"
#include "common/keyboard.h"
#include "common/scummsys.h"
#include "common/util.h"

namespace TwinE {

class TwinEEngine;

extern const char *mainKeyMapId;
extern const char *uiKeyMapId;
extern const char *cutsceneKeyMapId;

enum TwinEActionType {
	Pause,
	NextRoom,
	PreviousRoom,
	ApplyCellingGrid,
	IncreaseCellingGridIndex,
	DecreaseCellingGridIndex,
	DebugGridCameraPressUp,
	DebugGridCameraPressDown,
	DebugGridCameraPressLeft,
	DebugGridCameraPressRight,
	QuickBehaviourNormal,
	QuickBehaviourAthletic,
	QuickBehaviourAggressive,
	QuickBehaviourDiscreet,
	ExecuteBehaviourAction,
	BehaviourMenu,
	OptionsMenu,
	RecenterScreenOnTwinsen,
	UseSelectedObject,
	ThrowMagicBall,
	MoveForward,
	MoveBackward,
	TurnRight,
	TurnLeft,
	UseProtoPack,
	OpenHolomap,
	InventoryMenu,
	SpecialAction,
	Escape,
	PageUp,

	UIEnter,
	UIAbort,
	UILeft,
	UIRight,
	UIUp,
	UIDown,
	UINextPage,

	CutsceneAbort,

	Max
};

struct MouseStatusStruct {
	int32 left = 0;
	int32 right = 0;
	int32 x = 0;
	int32 y = 0;
};

/**
 * @brief Activates the given key map id that is registered in the meta engine
 */
class ScopedKeyMap {
private:
	TwinEEngine* _engine;
	Common::String _prevKeyMap;
public:
	ScopedKeyMap(TwinEEngine* engine, const char *id);
	~ScopedKeyMap();
};

class Input {
private:
	TwinEEngine *_engine;
	Common::String _currentKeyMap;

	uint8 actionStates[TwinEActionType::Max]{false};
	int16 currentKey = 0;
public:
	Input(TwinEEngine *engine);

	int16 cursorKeys = 0;
	int16 pressedKey = 0;
	int16 leftMouse = 0;
	int16 rightMouse = 0;

	/**
	 * @brief Dependent on the context we are currently in the game, we might want to disable certain keymaps.
	 * Like disabling ui keymaps when we are in-game - or vice versa.
	 */
	void enableKeyMap(const char *id);

	const Common::String currentKeyMap() const;

	/**
	 * @param onlyFirstTime If this is set to @c true, repeating key press events are not taken into account here
	 * This means, that even if the key is held down, this will return @c false. @c false as value for this parameter
	 * will return @c true also for repeating key presses.
	 *
	 * @sa isPressed()
	 */
	bool isActionActive(TwinEActionType actionType, bool onlyFirstTime = true) const;

	/**
	 * @brief If the action is active, the internal state is reset and a following call of this method won't return
	 * @c true anymore
	 */
	bool toggleActionIfActive(TwinEActionType actionType);

	bool toggleAbortAction();

	bool isQuickBehaviourActionActive() const;
	bool isMoveOrTurnActionActive() const;
	bool isHeroActionActive() const;

	/**
	 * Gets mouse positions
	 * @param mouseData structure that contains mouse position info
	 */
	void getMousePositions(MouseStatusStruct *mouseData);

	void readKeys();
	uint8 processCustomEngineEventStart(const Common::Event& event);
	uint8 processCustomEngineEventEnd(const Common::Event& event);
};

inline const Common::String Input::currentKeyMap() const {
	return _currentKeyMap;
}

} // namespace TwinE

#endif
