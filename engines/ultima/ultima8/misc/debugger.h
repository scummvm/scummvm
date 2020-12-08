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

#ifndef ULTIMA_ULTIMA8_ENGINE_DEBUGGER_H
#define ULTIMA_ULTIMA8_ENGINE_DEBUGGER_H

#include "ultima/ultima8/misc/common_types.h"
#include "ultima/shared/engine/debugger.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/misc.h"
#include "common/debug.h"
#include "common/stream.h"

namespace Ultima {
namespace Ultima8 {

class ConsoleStream : public Common::WriteStream {
private:
	Std::Precision _precision;
public:
	ConsoleStream() : Common::WriteStream(), _precision(Std::dec) {
	}

	int32 pos() const override {
		return 0;
	}

	void Print(const char *fmt, ...) {
		va_list argptr;
		va_start(argptr, fmt);
		Common::String str = Common::String::vformat(fmt, argptr);
		va_end(argptr);

		write(str.c_str(), str.size());
	}

	ConsoleStream &operator<<(const char *s) {
		write(s, strlen(s));
		return *this;
	}

	ConsoleStream &operator<<(const void *ptr) {
		Common::String str = Common::String::format("%p", ptr);
		write(str.c_str(), str.size());
		return *this;
	}

	ConsoleStream &operator<<(const Common::String &str) {
		write(str.c_str(), str.size());
		return *this;
	}

	ConsoleStream &operator<<(Std::Precision p) {
		_precision = p;
		return *this;
	}

	ConsoleStream &operator<<(int val) {
		Common::String str = Common::String::format(
			(_precision == Std::hex) ? "%x" : "%d", val);
		write(str.c_str(), str.size());
		return *this;
	}
};

template<class T>
class console_ostream : public ConsoleStream {
	uint32 write(const void *dataPtr, uint32 dataSize) override {
		Common::String str((const char *)dataPtr, (const char *)dataPtr + dataSize);
		debugN(MM_INFO, "%s", str.c_str());
		return dataSize;
	}
};

template<class T>
class console_err_ostream : public ConsoleStream {
private:
	Common::String _line;
public:
	uint32 write(const void *dataPtr, uint32 dataSize) override {
		_line += Common::String((const char *)dataPtr, dataSize);

		size_t lineEnd;
		while ((lineEnd = _line.find('\n')) != Common::String::npos) {
			if (lineEnd > 0)
				warning("%s", Common::String(_line.c_str(), lineEnd).c_str());

			_line = Common::String(_line.c_str() + lineEnd + 1);
		}

		return dataSize;
	}
};

// Standard Output Stream Object
extern console_ostream<char> *ppout;
// Error Output Stream Object
extern console_err_ostream<char> *pperr;

#define pout (*ppout)
#define perr (*pperr)


/**
 * Debugger base class
 */
class Debugger : public Shared::Debugger {
public:
	typedef Common::String ArgsType;
	typedef Std::vector<ArgsType> ArgvType;
private:
	// Standard Output Stream Object
	console_ostream<char> _strOut;
	// Error Output Stream Object
	console_err_ostream<char> _errOut;
private:
	const char *strBool(bool flag) {
		return flag ? "true" : "false";
	}

	// Engine
	bool cmdSaveGame(int argc, const char **argv);
	bool cmdLoadGame(int argc, const char **argv);
	bool cmdNewGame(int argc, const char **argv);
	bool cmdQuit(int argc, const char **argv);
	bool cmdChangeGame(int argc, const char **argv);
	bool cmdListGames(int argc, const char **argv);
	bool cmdSetVideoMode(int argc, const char **argv);
	bool cmdEngineStats(int argc, const char **argv);
	bool cmdToggleAvatarInStasis(int argc, const char **argv);
	bool cmdTogglePaintEditorItems(int argc, const char **argv);
	bool cmdToggleShowTouchingItems(int argc, const char **argv);
	bool cmdCloseItemGumps(int argc, const char **argv);
	bool cmdMemberVar(int argc, const char **argv);

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
	bool cmdAttack(int argc, const char **argv);

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
	bool cmdToggleHighlightItems(int argc, const char **argv);
	bool cmdDumpMap(int argc, const char **argvv);
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
	bool cmdDetonateBomb(int argc, const char **argv);
	bool cmdStartSelection(int argc, const char **argv);
	bool cmdUseSelection(int argc, const char **argv);

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
#ifdef DEBUG
	bool cmdTracePID(int argc, const char **argv);
	bool cmdTraceObjID(int argc, const char **argv);
	bool cmdTraceClass(int argc, const char **argv);
	bool cmdTraceAll(int argc, const char **argv);
	bool cmdTraceEvents(int argc, const char **argv);
	bool cmdStopTrace(int argc, const char **argv);
#endif

	// Miscellaneous
	bool cmdToggleFastArea(int argc, const char **argv);
	bool cmdVerifyQuit(int argc, const char **argv);
	bool cmdU8ShapeViewer(int argc, const char **argv);
	bool cmdShowMenu(int argc, const char **argv);
	bool cmdGenerateWholeMap(int argc, const char **argv);
	bool cmdToggleMinimap(int argc, const char **argv);
	bool cmdInvertScreen(int argc, const char **argv);
	bool cmdPlayMovie(int argc, const char **argv);
	bool cmdPlayMusic(int argc, const char **argv);

#ifdef DEBUG
	bool cmdVisualDebugPathfinder(int argc, const char **argv);
#endif

public:
	Debugger();
	~Debugger() override;

	void executeCommand(const ArgsType &args);
	void executeCommand(const ArgvType &argv);
};

extern Debugger *g_debugger;

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
