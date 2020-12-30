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

#ifndef TWINE_CONSOLE_H
#define TWINE_CONSOLE_H

#include "gui/debugger.h"
#include "twine/scene/gamestate.h"

namespace TwinE {

class TwinEEngine;

class TwinEConsole : public GUI::Debugger {
private:
	TwinEEngine *_engine;

	bool doPrintGameFlag(int argc, const char **argv);
	bool doSetHeroPosition(int argc, const char **argv);
	bool doGiveAllItems(int argc, const char **argv);
	bool doChangeScene(int argc, const char **argv);
	bool doListMenuText(int argc, const char **argv);
	bool doToggleDebug(int argc, const char **argv);
	bool doGiveKey(int argc, const char **argv);
	bool doToggleZoneRendering(int argc, const char **argv);
	bool doToggleClipRendering(int argc, const char **argv);
	bool doToggleFreeCamera(int argc, const char **argv);
	bool doToggleSceneChanges(int argc, const char **argv);
	bool doSkipSceneActorsBut(int argc, const char **argv);
	bool doSetGameFlag(int argc, const char **argv);
	bool doSetInventoryFlag(int argc, const char **argv);
	bool doAddMagicPoints(int argc, const char **argv);
public:
	TwinEConsole(TwinEEngine *engine);
	~TwinEConsole() override;

};

} // End of namespace TwinE

#endif // TWINE_CONSOLE_H
