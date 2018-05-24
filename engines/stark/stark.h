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

namespace Common {
class RandomSource;
}

namespace Stark {

namespace Gfx {
class Driver;
class FrameLimiter;
}

class ArchiveLoader;
class Console;
class DialogPlayer;
class Diary;
class FontProvider;
class Global;
class GameInterface;
class Scene;
class StateProvider;
class StaticProvider;
class ResourceProvider;
class UserInterface;
class Settings;

class StarkEngine : public Engine {
public:
	StarkEngine(OSystem *syst, const ADGameDescription *gameDesc);
	virtual ~StarkEngine();

	/** Build a save file name for the specified target and slot */
	static Common::String formatSaveName(const char *target, int slot);

protected:
	// Engine APIs
	virtual Common::Error run() override;
	virtual GUI::Debugger *getDebugger() override { return (GUI::Debugger *)_console; }
	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc) override;
	virtual void pauseEngineIntern(bool pause) override;

private:
	void mainLoop();
	void updateDisplayScene();
	void processEvents();

	Gfx::Driver *_gfx;
	Gfx::FrameLimiter *_frameLimiter;
	Console *_console;

	// Services
	ArchiveLoader *_archiveLoader;
	DialogPlayer *_dialogPlayer;
	Diary *_diary;
	Global *_global;
	GameInterface *_gameInterface;
	StateProvider *_stateProvider;
	StaticProvider *_staticProvider;
	ResourceProvider *_resourceProvider;
	FontProvider *_fontProvider;
	Settings *_settings;
	Common::RandomSource *_randomSource;

	const ADGameDescription *_gameDescription;

	UserInterface *_userInterface;
	Scene *_scene;

	// Double click handling
	static const uint _doubleClickDelay = 500; // ms
	uint _lastClickTime;
};

} // End of namespace Stark

#endif // STARK_H
