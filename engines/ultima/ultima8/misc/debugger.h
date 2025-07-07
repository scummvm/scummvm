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
	bool cmdAvatarInStasis(int argc, const char **argv);
	bool cmdShowEditorItems(int argc, const char **argv);
	bool cmdShowTouchingItems(int argc, const char **argv);
	bool cmdCloseItemGumps(int argc, const char **argv);

	bool cmdCameraOnAvatar(int argc, const char **argv);

	// Audio Process
	bool cmdListSFX(int argc, const char **argv);
	bool cmdStopSFX(int argc, const char **argv);
	bool cmdPlaySFX(int argc, const char **argv);

	// Cheats
	bool cmdCheatMode(int argc, const char **argv);
	bool cmdCheatItems(int argc, const char **argv);
	bool cmdCheatEquip(int argc, const char **argv);
	bool cmdMaxStats(int argc, const char **argv);
	bool cmdHeal(int argc, const char **argv);
	bool cmdInvincibility(int argc, const char **argv);
	bool cmdHackMover(int argc, const char **argv);

	// Game Map Gump
	bool cmdHighlightItems(int argc, const char **argv);
	bool cmdFootpads(int argc, const char **argv);
	bool cmdGridlines(int argc, const char **argv);
	bool cmdDumpMap(int argc, const char **argvv);
	bool cmdDumpAllMaps(int argc, const char **argv);
	bool cmdIncrementSortOrder(int argc, const char **argv);
	bool cmdDecrementSortOrder(int argc, const char **argv);

	// Kernel
	bool cmdProcessTypes(int argc, const char **argv);
	bool cmdListProcesses(int argc, const char **argv);
	bool cmdProcessInfo(int argc, const char **argv);
	bool cmdFrameByFrame(int argc, const char **argv);
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
	bool cmdCombat(int argc, const char **argv);
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
	bool cmdQuickMover(int argc, const char **argv);
	bool cmdClipping(int argc, const char **argv);

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
	bool cmdVisualDebugPathfinder(int argc, const char **argv);

	void dumpCurrentMap(); // helper function

public:
	Debugger();
	~Debugger() override;
};

extern Debugger *g_debugger;

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
