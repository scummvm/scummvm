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

#include "common/ptr.h"
#include "engines/engine.h"

struct ADGameDescription;

namespace Ripper {

class CursorManager;
class CyberManager;
class InputManager;
class Inventory;
class MediaPlayer;
class Milestones;
class ModalDialogManager;
class ResourceManager;
class SceneAudioManager;
class ScriptManager;
class RipperSettings;
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
	CursorManager *getCursor() const { return _cursor.get(); }
	CyberManager *getCyber() const { return _cyber.get(); }
	InputManager *getInput() const { return _input.get(); }
	Inventory *getInventory() const { return _inventory.get(); }
	MediaPlayer *getMedia() const { return _media.get(); }
	Milestones *getMilestones() const { return _milestones.get(); }
	ModalDialogManager *getModalDialog() const { return _modalDialog.get(); }
	ResourceManager *getResources() const { return _resources.get(); }
	SceneAudioManager *getSceneAudio() const { return _sceneAudio.get(); }
	ScriptManager *getScripts() const { return _scripts.get(); }
	RipperSettings *getSettings() const { return _settings.get(); }
	ToolbarManager *getToolbar() const { return _toolbar.get(); }
	WacManager *getWac() const { return _wac.get(); }
	WorldMap *getWorldMap() const { return _worldMap.get(); }
	bool isPuzzleHelpEnabled() const { return _puzzleHelpEnabled; }
	void setPuzzleHelpEnabled(bool enabled) { _puzzleHelpEnabled = enabled; }

private:
	void pauseEngineIntern(bool pause) override;
	void registerSearchPaths();
	void pumpEvents();

	const ADGameDescription *const _gameDescription;
	Common::ScopedPtr<CursorManager> _cursor;
	Common::ScopedPtr<CyberManager> _cyber;
	Common::ScopedPtr<InputManager> _input;
	Common::ScopedPtr<Inventory> _inventory;
	Common::ScopedPtr<SceneAudioManager> _sceneAudio;
	Common::ScopedPtr<MediaPlayer> _media;
	Common::ScopedPtr<Milestones> _milestones;
	Common::ScopedPtr<ModalDialogManager> _modalDialog;
	Common::ScopedPtr<ResourceManager> _resources;
	Common::ScopedPtr<ScriptManager> _scripts;
	Common::ScopedPtr<RipperSettings> _settings;
	Common::ScopedPtr<ToolbarManager> _toolbar;
	Common::ScopedPtr<WacManager> _wac;
	Common::ScopedPtr<WorldMap> _worldMap;
	bool _gameplayStarted;
	bool _puzzleHelpEnabled;
};

} // End of namespace Ripper

#endif // RIPPER_RIPPER_H
