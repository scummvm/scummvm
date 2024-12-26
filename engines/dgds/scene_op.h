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

#ifndef DGDS_SCENE_OP_H
#define DGDS_SCENE_OP_H

#include "common/types.h"
#include "common/array.h"

#include "dgds/scene_condition.h"

namespace Dgds {

enum SceneOpCode {
	kSceneOpNone = 0,
	kSceneOpChangeScene = 1,  	// args: scene num
	kSceneOpNoop = 2,		 	// args: none. Maybe should close dialogue?
	kSceneOpGlobal = 3,			// args: array of uints
	kSceneOpSegmentStateOps = 4,	// args: array of uint pairs [op seg, op seg], term with 0,0 that modify segment states
	kSceneOpSetItemAttr = 5,	// args: [item num, item param 0x28, item param 0x2c]. set item attrs?
	kSceneOpSetDragItem = 6,		// args: item num. give item?
	kSceneOpOpenInventory = 7,	// args: none.
	kSceneOpShowDlg = 8,		// args: dialogue number.
	kSceneOpShowInvButton = 9,		// args: none.
	kSceneOpHideInvButton = 10,	// args: none.
	kSceneOpEnableTrigger = 11,	// args: trigger num
	kSceneOpChangeSceneToStored = 12,	// args: none. Change scene to stored number
	kSceneOpAddFlagToDragItem = 13,			// args: none.
	kSceneOpOpenInventoryZoom = 14,	// args: none.
	kSceneOpMoveItemsBetweenScenes = 15,	// args: none.
	kSceneOpShowClock = 16,		// args: none.  set clock script-visible.
	kSceneOpHideClock = 17,		// args: none.  set clock script-hidden.
	kSceneOpShowMouse = 18,		// args: none.
	kSceneOpHideMouse = 19,		// args: none.
	// Op 20 onward are common, but not in dragon

	kSceneOpLoadTalkDataAndSetFlags = 20, // args: tdsnum to load, headnum
	kSceneOpDrawVisibleTalkHeads = 21, // args: none
	kSceneOpLoadTalkData = 22, 	// args: tds num to load
	kSceneOpLoadDDSData = 24, 	// args: dds num to load
	kSceneOpFreeDDSData = 25,	// args: dds num to free
	kSceneOpFreeTalkData = 26, 	// args: tds num to free

	// Dragon-specific opcodes
	kSceneOpPasscode = 100,				// args: none.
	kSceneOpMeanwhile = 101,			// args: none. Clears screen and displays "meanwhile".
	kSceneOpOpenGameOverMenu = 102,		// args: none.
	kSceneOpTiredDialog = 103,			// args: none. Something about "boy am I tired"?
	kSceneOpArcadeTick = 104,			// args: none. Called in arcade post-tick.
	kSceneOpDrawDragonCountdown1 = 105,	// args: none. Draw special countdown number at 141, 56
	kSceneOpDrawDragonCountdown2 = 106,	// args: none. Draw some number at 250, 42
	kSceneOpOpenPlaySkipIntroMenu = 107, // args: none.  DRAGON: Show menu 50, the "Play Introduction" / "Skip Introduction" menu.
	kSceneOpOpenBetterSaveGameMenu = 108,			// args: none. DRAGON: Show menu 46, the "Before arcade maybe you better save your game" menu.

	// China-specific opcodes
	kSceneOpChinaTankInit = 100,
	kSceneOpChinaTankEnd = 101,
	kSceneOpChinaTankTick = 102,
	kSceneOpChinaSetLanding = 103,
	kSceneOpChinaScrollIntro = 104,
	kSceneOpChinaScrollLeft = 105,
	kSceneOpChinaScrollRight = 107,
	kSceneOpShellGameInit = 108,
	kSceneOpShellGameEnd = 109,
	kSceneOpShellGameTick = 110,
	kSceneOpChinaTrainInit = 111,
	kSceneOpChinaTrainEnd = 112,
	kSceneOpChinaTrainTick = 113,
	kSceneOpChinaOpenGameOverMenu = 114,	// args: none.
	kSceneOpChinaOpenSkipCreditsMenu = 115,	// args: none.
	kSceneOpChinaOnIntroTick = 116,	// args: none.
	kSceneOpChinaOnIntroInit = 117,	// args: none.
	kSceneOpChinaOnIntroEnd = 118,	// args: none.

	// Beamish-specific opcodes
	kSceneOpOpenBeamishGameOverMenu = 100,
	kSceneOpOpenBeamishOpenSkipCreditsMenu = 101,

	kSceneOpMaxCode = 255, // for checking file load

	kSceneOpHasConditionalOpsFlag = 0x8000,
};

class SceneOp {
public:
	Common::Array<SceneConditions> _conditionList;
	Common::Array<uint16> _args;
	SceneOpCode _opCode;

	Common::String dump(const Common::String &indent) const;
	bool runOp() const;

private:
	bool runCommonOp() const;
	bool runDragonOp() const;
	bool runChinaOp() const;
	bool runBeamishOp() const;
};

class ConditionalSceneOp {
public:
	SceneOpCode _opCode;
	Common::Array<SceneConditions> _conditionList;
	Common::Array<SceneOp> _opList;

	Common::String dump(const Common::String &indent) const;
};


} // end namespace Dgds

#endif // DGDS_SCENE_OP_H
