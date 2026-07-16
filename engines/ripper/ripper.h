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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_RIPPER_H
#define RIPPER_RIPPER_H

#include "engines/engine.h"

struct ADGameDescription;

namespace Ripper {

class CursorManager;
class InputManager;
class MediaPlayer;
class Milestones;
class ResourceManager;
class ScriptManager;
class ToolbarManager;
class WacManager;
class WorldMap;

class RipperEngine : public Engine {
public:
	RipperEngine(OSystem *system, const ADGameDescription *gameDescription);
	~RipperEngine() override;

	Common::Error run() override;
	bool hasFeature(EngineFeature feature) const override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveAutosaveCurrently() override { return false; }
	int getAutosaveSlot() const override { return 20; }
	CursorManager *getCursor() const { return _cursor; }
	InputManager *getInput() const { return _input; }
	MediaPlayer *getMedia() const { return _media; }
	Milestones *getMilestones() const { return _milestones; }
	ResourceManager *getResources() const { return _resources; }
	ScriptManager *getScripts() const { return _scripts; }
	ToolbarManager *getToolbar() const { return _toolbar; }
	WacManager *getWac() const { return _wac; }
	WorldMap *getWorldMap() const { return _worldMap; }

private:
	void registerSearchPaths();
	void pumpEvents();

	const ADGameDescription *const _gameDescription;
	CursorManager *_cursor;
	InputManager *_input;
	MediaPlayer *_media;
	Milestones *_milestones;
	ResourceManager *_resources;
	ScriptManager *_scripts;
	ToolbarManager *_toolbar;
	WacManager *_wac;
	WorldMap *_worldMap;
	bool _gameplayStarted;
};

} // End of namespace Ripper

#endif // RIPPER_RIPPER_H
