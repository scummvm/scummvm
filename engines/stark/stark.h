/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_H
#define STARK_H

#include "engines/advancedDetector.h"
#include "engines/engine.h"

namespace Stark {

/*
enum StarkGameID {
	GID_TLJ = 0,
	GID_DREAM
};
*/

enum StarkGameFeatures {
	GF_DVD =  (1 << 31)
};

class Console;
class GfxDriver;
class Scene;
class Global;
class ArchiveLoader;
class StateProvider;
class ResourceProvider;

class StarkEngine : public Engine {
public:
	StarkEngine(OSystem *syst, const ADGameDescription *gameDesc);
	virtual ~StarkEngine();

protected:
	// Engine APIs
	virtual Common::Error run();
	virtual GUI::Debugger *getDebugger() { return (GUI::Debugger *)_console; }
	bool hasFeature(EngineFeature f) const;
	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &desc);

private:
	void mainLoop();
	void updateDisplayScene();

	GfxDriver *_gfx;
	Console *_console;

	Global *_global;
	ArchiveLoader *_archiveLoader;
	StateProvider *_stateProvider;
	ResourceProvider *_resourceProvider;

	const ADGameDescription *_gameDescription;

	Scene *_scene;
};

} // End of namespace Stark

#endif // STARK_H
