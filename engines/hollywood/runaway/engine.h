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

#ifndef HOLLYWOOD_RUNAWAY_ENGINE_H
#define HOLLYWOOD_RUNAWAY_ENGINE_H

#include "common/array.h"
#include "common/language.h"
#include "common/ptr.h"
#include "engines/engine.h"
#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/intro.h"
#include "hollywood/runaway/menu.h"
#include "hollywood/runaway/saveload.h"

namespace Common {
class Keymap;
}

namespace Graphics {
class ManagedSurface;
}

namespace Hollywood {

class RunawayMenu;
class RunawayInventory;
class RunawayScene;
class RunawayUI;

// Runaway shares the archive and text formats with Hollywood Monsters, but
// has its own RGB565 renderer, scene state and audio formats.
class RunawayEngine : public Engine {
	friend class RunawayConsole;

public:
	enum RunawayAction {
		kScrollLeft,
		kScrollRight,
		kScrollStart,
		kScrollEnd,
		kCancelAction,
		kSkip,
		kToggleInventory,
		kCycleAction,
		kPauseMenu
	};

	RunawayEngine(OSystem *syst, Common::Language language, RunawayGameType game);
	~RunawayEngine() override;

	Common::Error run() override;
	void initializePath(const Common::FSNode &gamePath) override;
	bool hasFeature(EngineFeature feature) const override;
	void syncSoundSettings() override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	bool copySaveThumbnail(Graphics::Surface &thumbnail) const;
	void getHotspotPositions(Common::Array<Graphics::HotspotInfo> &hotspots) override;
	Graphics::HotspotRenderer *createHotspotRenderer() override;
	bool hideCursorForHotspots() const override { return false; }

	static Common::Array<Common::Keymap *> initKeymaps();
	static void registerDefaultSettings();

private:
	RunawayScene *createScene(uint16 entryScene);
	Common::Error changeScene();
	bool sceneBusy() const;
	Common::Error playIntro(RunawayMenu &menu, byte chapter = 0, RunawayIntro::Mode mode = RunawayIntro::kCompleteIntro);
	Common::Error startChapter(RunawayMenu &menu, byte chapter);
	Common::Error selectChapter(RunawayMenu &menu);
	byte unlockedChapter() const;
	void unlockChapter(byte chapter);
	Common::Error playEnding(RunawayMenu &menu);
	int pauseMenu(RunawayMenu &menu, Graphics::ManagedSurface &screen, byte chapters = 0);
	RunawayMenu::Settings readSettings() const;
	void applySettings(const RunawayMenu::Settings &settings, bool restoreDefaults = false);
	void present(const Graphics::ManagedSurface &screen);
	void updateSoundListener();
	Common::Point updateUI(uint32 now);
	Common::Language _language;
	const RunawayGameType _game;
	const bool _museum;
	RunawayStrings _strings;
	RunawayAudio _audio;
	RunawaySaveState _state;
	Common::ScopedPtr<RunawayInventory> _inventory;
	Common::ScopedPtr<RunawayScene> _scene;
	Common::ScopedPtr<RunawayUI> _ui;
	RunawayPresentation::MouseState _presentationMouse;
	bool _inPresentation = false, _gameStarted = false, _loadedGame = false;
	bool _pendingDoorClose = false;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_ENGINE_H
