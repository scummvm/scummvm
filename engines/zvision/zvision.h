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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#ifndef ZVISION_H
#define ZVISION_H

#include "common/random.h"
#include "common/events.h"

#include "engines/engine.h"

#include "zvision/detection.h"

#include "gui/debugger.h"
 
namespace ZVision {

struct ZVisionGameDescription;
class Console;
class ScriptManager;
class RenderManager;
 
// our engine debug channels
enum {
	kZDebugExample = 1 << 0,
	kZDebugExample2 = 1 << 1
	// next new channel must be 1 << 2 (4)
	// the current limitation is 32 debug channels (1 << 31 is the last one)
};
 
class ZVision : public Engine {
public:
	ZVision(OSystem *syst, const ZVisionGameDescription *gameDesc);
	~ZVision();

private:
	Console *_console;
	const ZVisionGameDescription *_gameDescription;
	const int _width;
	const int _height;

	// We need random numbers
	Common::RandomSource *_rnd;
	// Managers
	ScriptManager *_scriptManager;
	RenderManager *_renderManager;

	// To prevent allocation every time we process events
	Common::Event _event;

public:
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	virtual Common::Error run();
	ScriptManager *getScriptManager() const;
	RenderManager *getRenderManager() const;
	Common::RandomSource *getRandomSource() const;
	ZVisionGameId getGameId() const;

private:
	void initialize();

	void processEvents();
	void onMouseDown(const Common::Point &pos);
	void onMouseMove(const Common::Point &pos);
	void onKeyDown(uint keyCode);
};

} // End of namespace ZVision
 
#endif
