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

#ifndef TWINE_CONSOLE_H
#define TWINE_CONSOLE_H

#include "gui/debugger.h"
#include "twine/scene/gamestate.h"

namespace TwinE {

class TwinEEngine;

class TwinEConsole : public GUI::Debugger {
private:
	using Super = GUI::Debugger;
	TwinEEngine *_engine;

	bool doToggleSceneryView(int argc, const char **argv);
	bool doPlayVideo(int argc, const char **argv);
	bool doPlayMidi(int argc, const char **argv);
	bool doPlayMusic(int argc, const char **argv);
	bool doPrintGameFlag(int argc, const char **argv);
	bool doPrintInventoryFlag(int argc, const char **argv);
	bool doPrintHolomapFlag(int argc, const char **argv);
	bool doSetHeroPosition(int argc, const char **argv);
	bool doGiveItem(int argc, const char **argv);
	bool doSetLife(int argc, const char **argv);
	bool doGiveAllItems(int argc, const char **argv);
	bool doChangeScene(int argc, const char **argv);
	bool doToggleAutoAggressive(int argc, const char **argv);
	bool doGiveKey(int argc, const char **argv);
	bool doGiveGas(int argc, const char **argv);
	bool doGiveKashes(int argc, const char **argv);
	bool doToggleZoneRendering(int argc, const char **argv);
	bool doToggleClipRendering(int argc, const char **argv);
	bool doToggleActorRendering(int argc, const char **argv);
	bool doToggleTrackRendering(int argc, const char **argv);
	bool doToggleGodMode(int argc, const char **argv);
	bool doToggleEnhancements(int argc, const char **argv);
	bool doToggleFreeCamera(int argc, const char **argv);
	bool doToggleSceneRendering(int argc, const char **argv);
	bool doSetTrackObject(int argc, const char **argv);
	bool doChangeChapter(int argc, const char **argv);
	bool doSkipSceneActorsBut(int argc, const char **argv);
	bool doSetGameFlag(int argc, const char **argv);
	bool doSetInventoryFlag(int argc, const char **argv);
	bool doSetHolomapFlag(int argc, const char **argv);
	bool doAddMagicPoints(int argc, const char **argv);
	bool doDumpFile(int argc, const char **argv);
	bool doSetHolomapTrajectory(int argc, const char **argv);

protected:
	void preEnter() override;
	void postEnter() override;

public:
	TwinEConsole(TwinEEngine *engine);
	~TwinEConsole() override;

	bool exec(const char *file) {
		const char *argv[] = {"", file};
		return cmdExecFile(2, argv);
	}
};

} // End of namespace TwinE

#endif // TWINE_CONSOLE_H
