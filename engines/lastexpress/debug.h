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

#ifndef LASTEXPRESS_DEBUG_H
#define LASTEXPRESS_DEBUG_H

#include "gui/debugger.h"

namespace LastExpress {

#ifdef USE_IMGUI
void onImGuiInit();
void onImGuiRender();
void onImGuiCleanup();
#endif

enum {
	kLastExpressDebugGraphics = 1,
	kLastExpressDebugResource,
	kLastExpressDebugCursor,
	kLastExpressDebugSound,
	kLastExpressDebugSubtitle,
	kLastExpressDebugSavegame,
	kLastExpressDebugLogic,
	kLastExpressDebugScenes,
	kLastExpressDebugUnknown,
};

enum LastExpressDebugChannels {
	kDebugConsole = 0
};

class LastExpressEngine;

class Debugger : public GUI::Debugger {
public:
	Debugger(LastExpressEngine *engine);
	~Debugger() override;

private:
	LastExpressEngine *_engine;

	bool cmdHelp(int argc, const char **argv);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_DEBUG_H
