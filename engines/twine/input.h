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

	CutsceneAbort,

	Max
};

// TODO: get rid of this table
static constexpr const struct ActionMapping {
	TwinEActionType action;
	uint8 localKey;
} twineactions[] = {
    {Pause, 0x19},
    {NextRoom, 0x13},
    {PreviousRoom, 0x21},
    {ApplyCellingGrid, 0x14},
    {IncreaseCellingGridIndex, 0x22},
    {DecreaseCellingGridIndex, 0x30},
    {DebugGridCameraPressUp, 0x2E},
    {DebugGridCameraPressDown, 0x2C},
    {DebugGridCameraPressLeft, 0x1F},
    {DebugGridCameraPressRight, 0x2D},
    {QuickBehaviourNormal, 0x3B},
    {QuickBehaviourAthletic, 0x3C},
    {QuickBehaviourAggressive, 0x3D},
    {QuickBehaviourDiscreet, 0x3E},
    {ExecuteBehaviourAction, 0x39},
    {BehaviourMenu, 0x1D},
    {OptionsMenu, 0x40},
    {RecenterScreenOnTwinsen, 0x1C},
    {UseSelectedObject, 0x1C},
    {ThrowMagicBall, 0x38},
    {MoveForward, 0x48},
    {MoveBackward, 0x50},
    {TurnRight, 0x4D},
    {TurnLeft, 0x4B},
    {UseProtoPack, 0x24},
    {OpenHolomap, 0x23},
    {InventoryMenu, 0x36},
    {SpecialAction, 0x11},
    {Escape, 0x01},
    {PageUp, 0x49}, // TODO: used for what?
    {UIEnter, 0x00},
    {UIAbort, 0x00},
    {UILeft, 0x00},
    {UIRight, 0x00},
    {UIUp, 0x00},
    {UIDown, 0x00},
    {CutsceneAbort, 0x00}};

static_assert(ARRAYSIZE(twineactions) == TwinEActionType::Max, "Unexpected action mapping array size");

struct MouseStatusStruct {
	int32 left = 0;
	int32 right = 0;
	int32 x = 0;
	int32 y = 0;
};

/**
 * @brief Whenever text input is needed (like the playername)
 * you have to disable the keymaps
 */
struct ScopedKeyMapperDisable {
	ScopedKeyMapperDisable();
	~ScopedKeyMapperDisable();
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
	uint8 _pressed[Common::KEYCODE_LAST]{0};
	Common::String _currentKeyMap;

	uint8 actionStates[TwinEActionType::Max]{false};
	int16 internalKeyCode = 0;
	int16 currentKey = 0;
public:
	Input(TwinEEngine *engine);

	int16 key = 0;
	int16 skippedKey = 0;
	int16 pressedKey = 0;
	int16 leftMouse = 0;
	int16 rightMouse = 0;

	/**
	 * @brief Dependent on the context we are currently in the game, we might want to disable certain keymaps.
	 * Like disabling ui keymaps when we are in-game - or vice versa.
	 */
	void enabledKeyMap(const char *id);

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

	/**
	 * @param onlyFirstTime If this is set to @c true, repeating key press events are not taken into account here
	 * This means, that even if the key is held down, this will return @c false. @c false as value for this parameter
	 * will return @c true also for repeating key presses.
	 *
	 * @note You won't receive any pressed events if you have that key bound to a @c TwinEActionType value.
	 * @sa isActionActive()
	 */
	bool isPressed(Common::KeyCode keycode, bool onlyFirstTime = true) const;

	inline bool isPressedEnter(bool onlyFirstTime = true) const {
		return isPressed(Common::KEYCODE_RETURN, onlyFirstTime) || isPressed(Common::KEYCODE_KP_ENTER, onlyFirstTime);
	}

	bool isQuickBehaviourActionActive() const;

	/**
	 * Gets mouse positions
	 * @param mouseData structure that contains mouse position info
	 */
	void getMousePositions(MouseStatusStruct *mouseData);

	void readKeys();
};

inline const Common::String Input::currentKeyMap() const {
	return _currentKeyMap;
}

} // namespace TwinE

#endif
