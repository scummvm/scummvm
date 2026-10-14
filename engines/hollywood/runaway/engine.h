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
#include "hollywood/runaway/menu.h"

namespace Common {
class Keymap;
}

namespace Graphics {
class ManagedSurface;
}

namespace Hollywood {

class RunawayMenu;
class RunawayInventory;
class RunawayScene5030;
class RunawayUI;

// Runaway shares the archive and text formats with Hollywood Monsters, but
// has its own RGB565 renderer, scene state and audio formats.
class RunawayEngine : public Engine {
public:
	RunawayEngine(OSystem *syst, Common::Language language);
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

	static Common::Array<Common::Keymap *> initKeymaps();
	static void registerDefaultSettings();

private:
	Common::Error playIntro(RunawayMenu &menu);
	Common::Error playEnding(RunawayMenu &menu);
	void pauseMenu(RunawayMenu &menu, Graphics::ManagedSurface &screen);
	RunawayMenu::Settings readSettings() const;
	void applySettings(const RunawayMenu::Settings &settings, bool restoreDefaults = false);
	void present(const Graphics::ManagedSurface &screen);
	Common::Language _language;
	RunawayStrings _strings;
	Common::ScopedPtr<RunawayInventory> _inventory;
	Common::ScopedPtr<RunawayScene5030> _scene;
	Common::ScopedPtr<RunawayUI> _ui;
	bool _inPresentation = false, _gameStarted = false, _loadedGame = false;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_ENGINE_H
