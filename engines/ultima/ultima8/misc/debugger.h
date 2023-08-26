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

#ifndef ULTIMA_ULTIMA8_ENGINE_DEBUGGER_H
#define ULTIMA_ULTIMA8_ENGINE_DEBUGGER_H

#include "ultima/ultima8/misc/common_types.h"
#include "ultima/shared/std/containers.h"
#include "common/debug.h"
#include "common/stream.h"
#include "gui/debugger.h"

namespace Ultima {
namespace Ultima8 {

/**
 * Debugger base class
 */
class Debugger : public GUI::Debugger {
private:
	const char *strBool(bool flag) {
		return flag ? "true" : "false";
	}

	// Engine
	bool cmdSaveGame(int argc, const char **argv);
	bool cmdLoadGame(int argc, const char **argv);
	bool cmdNewGame(int argc, const char **argv);
	bool cmdQuit(int argc, const char **argv);
	bool cmdSetVideoMode(int argc, const char **argv);
	bool cmdEngineStats(int argc, const char **argv);
	bool cmdToggleAvatarInStasis(int argc, const char **argv);
	bool cmdTogglePaintEditorItems(int argc, const char **argv);
	bool cmdToggleShowTouchingItems(int argc, const char **argv);
	bool cmdCloseItemGumps(int argc, const char **argv);

	// Avatar mover
	bool cmdStartJump(int argc, const char **argv);
	bool cmdStopJump(int argc, const char **argv);
	bool cmdStartTurnLeft(int argc, const char **argv);
	bool cmdStartTurnRight(int argc, const char **argv);
	bool cmdStartMoveForward(int argc, const char **argv);
	bool cmdStartMoveBack(int argc, const char **argv);
	bool cmdStopTurnLeft(int argc, const char **argv);
	bool cmdStopTurnRight(int argc, const char **argv);
	bool cmdStopMoveForward(int argc, const char **argv);
	bool cmdStopMoveBack(int argc, const char **argv);
	bool cmdStartMoveUp(int argc, const char **argv);
	bool cmdStartMoveDown(int argc, const char **argv);
	bool cmdStartMoveLeft(int argc, const char **argv);
	bool cmdStartMoveRight(int argc, const char **argv);
	bool cmdStopMoveUp(int argc, const char **argv);
	bool cmdStopMoveDown(int argc, const char **argv);
	bool cmdStopMoveLeft(int argc, const char **argv);
	bool cmdStopMoveRight(int argc, const char **argv);

	bool cmdStartMoveRun(int argc, const char **argv);
	bool cmdStopMoveRun(int argc, const char **argv);
	bool cmdStartMoveStep(int argc, const char **argv);
	bool cmdStopMoveStep(int argc, const char **argv);
	bool cmdStartAttack(int argc, const char **argv);
	bool cmdStopAttack(int argc, const char **argv);

	// One-shot Avatar mover commands
	bool cmdShortJump(int argc, const char **argv);
	bool cmdStepLeft(int argc, const char **argv);
	bool cmdStepRight(int argc, const char **argv);
	bool cmdStepForward(int argc, const char **argv);
	bool cmdStepBack(int argc, const char **argv);
	bool cmdRollLeft(int argc, const char **argv);
	bool cmdRollRight(int argc, const char **argv);
	bool cmdToggleCrouch(int argc, const char **argv);

	bool cmdCameraOnAvatar(int argc, const char **argv);

	// Audio Process
	bool cmdListSFX(int argc, const char **argv);
	bool cmdStopSFX(int argc, const char **argv);
	bool cmdPlaySFX(int argc, const char **argv);

	// Cheats
	bool cmdToggleCheatMode(int argc, const char **argv);
	bool cmdCheatItems(int argc, const char **argv);
	bool cmdCheatEquip(int argc, const char **argv);
	bool cmdMaxStats(int argc, const char **argv);
	bool cmdHeal(int argc, const char **argv);
	bool cmdToggleInvincibility(int argc, const char **argv);

	// Game Map Gump
	bool cmdStartHighlightItems(int argc, const char **argv);
	bool cmdStopHighlightItems(int argc, const char **argv);
	bool cmdToggleHighlightItems(int argc, const char **argv);
	bool cmdDumpMap(int argc, const char **argvv);
	bool cmdDumpAllMaps(int argc, const char **argv);
	bool cmdIncrementSortOrder(int argc, const char **argv);
	bool cmdDecrementSortOrder(int argc, const char **argv);

	// Kernel
	bool cmdProcessTypes(int argc, const char **argv);
	bool cmdListProcesses(int argc, const char **argv);
	bool cmdProcessInfo(int argc, const char **argv);
	bool cmdToggleFrameByFrame(int argc, const char **argv);
	bool cmdAdvanceFrame(int argc, const char **argv);

	// Main Actor
	bool cmdTeleport(int argc, const char **argv);
	bool cmdMark(int argc, const char **argv);
	bool cmdRecall(int argc, const char **argv);
	bool cmdListMarks(int argc, const char **argv);
	bool cmdName(int argc, const char **argv);
	bool cmdUseBackpack(int argc, const char **argv);
	bool cmdUseInventory(int argc, const char **argv);
	bool cmdUseRecall(int argc, const char **argv);
	bool cmdUseBedroll(int argc, const char **argv);
	bool cmdUseKeyring(int argc, const char **argv);
	bool cmdNextInventory(int argc, const char **argv);
	bool cmdNextWeapon(int argc, const char **argv);
	bool cmdToggleCombat(int argc, const char **argv);
	bool cmdUseInventoryItem(int argc, const char **argv);
	bool cmdUseMedikit(int argc, const char **argv);
	bool cmdUseEnergyCube(int argc, const char **argv);
	bool cmdDetonateBomb(int argc, const char **argv);
	bool cmdDropWeapon(int argc, const char **argv);
	bool cmdStartSelection(int argc, const char **argv);
	bool cmdUseSelection(int argc, const char **argv);
	bool cmdGrabItems(int argc, const char **argv);

	// Object Manager
	bool cmdObjectTypes(int argc, const char **argv);
	bool cmdObjectInfo(int argc, const char **argv);

	// Quick Avatar Mover Process
	bool cmdStartQuickMoveUp(int argc, const char **argv);
	bool cmdStartQuickMoveDown(int argc, const char **argv);
	bool cmdStartQuickMoveLeft(int argc, const char **argv);
	bool cmdStartQuickMoveRight(int argc, const char **argv);
	bool cmdStartQuickMoveAscend(int argc, const char **argv);
	bool cmdStartQuickMoveDescend(int argc, const char **argv);
	bool cmdStopQuickMoveUp(int argc, const char **argv);
	bool cmdStopQuickMoveDown(int argc, const char **argv);
	bool cmdStopQuickMoveLeft(int argc, const char **argv);
	bool cmdStopQuickMoveRight(int argc, const char **argv);
	bool cmdStopQuickMoveAscend(int argc, const char **argv);
	bool cmdStopQuickMoveDescend(int argc, const char **argv);
	bool cmdToggleQuarterSpeed(int argc, const char **argv);
	bool cmdToggleClipping(int argc, const char **argv);

	// UCMachine
	bool cmdGetGlobal(int argc, const char **argv);
	bool cmdSetGlobal(int argc, const char **argv);
	bool cmdTracePID(int argc, const char **argv);
	bool cmdTraceObjID(int argc, const char **argv);
	bool cmdTraceClass(int argc, const char **argv);
	bool cmdTraceAll(int argc, const char **argv);
	bool cmdStopTrace(int argc, const char **argv);

	// Miscellaneous
	bool cmdToggleFastArea(int argc, const char **argv);
	bool cmdVerifyQuit(int argc, const char **argv);
	bool cmdU8ShapeViewer(int argc, const char **argv);
	bool cmdShowMenu(int argc, const char **argv);
	bool cmdToggleMinimap(int argc, const char **argv);
	bool cmdGenerateMinimap(int argc, const char **argv);
	bool cmdClearMinimap(int argc, const char **argv);
	bool cmdInvertScreen(int argc, const char **argv);
	bool cmdPlayMovie(int argc, const char **argv);
	bool cmdPlayMusic(int argc, const char **argv);
	bool cmdBenchmarkRenderSurface(int argc, const char **argv);

#ifdef DEBUG
	bool cmdVisualDebugPathfinder(int argc, const char **argv);
#endif

	void dumpCurrentMap(); // helper function

public:
	Debugger();
	~Debugger() override;

	void executeCommand(const Common::String &args);
	void executeCommand(const Common::Array<Common::String> &argv);
};

extern Debugger *g_debugger;

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
