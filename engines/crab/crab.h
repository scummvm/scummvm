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

#ifndef CRAB_H
#define CRAB_H

#include "common/random.h"
#include "common/serializer.h"

#include "crab/detection.h"

namespace Graphics {
class ManagedSurface;
struct PixelFormat;
class Screen;
} // End of namespace Graphics

namespace Crab {

class App;
struct CrabGameDescription;

class LoadingScreen;

struct FilePaths;
struct TempValue;
class StringPool;
class ScreenSettings;

namespace pyrodactyl {
namespace event {
struct GameEventStore;
} // End of namespace event

namespace ui {
class SlideShow;
class OptionMenu;

template<typename FileType>
class FileMenu;

class SaveFileData;
} // End of namespace ui

namespace image {
class ImageManager;
} // End of namespace image

namespace input {
class Cursor;
class InputManager;
} // End of namespace input

namespace music {
class MusicManager;
} // End of namespace music

namespace text {
class TextManager;
} // End of namespace text

} // End of namespace pyrodactyl

enum DebugDraw {
	DRAW_TMX = 1 << 0,
	DRAW_PROP_BOUNDS = 1 << 1,
	DRAW_SPRITE_BOUNDS = 1 << 2,
	DRAW_PATHING = 1 << 3
};

class CrabEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	App *_app;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	Graphics::Screen *_screen = nullptr;
	Graphics::PixelFormat *_format = nullptr;

	pyrodactyl::image::ImageManager *_imageManager = nullptr;
	pyrodactyl::text::TextManager *_textManager = nullptr;
	pyrodactyl::input::InputManager *_inputManager = nullptr;
	pyrodactyl::music::MusicManager *_musicManager = nullptr;

	pyrodactyl::input::Cursor *_mouse = nullptr;
	pyrodactyl::event::GameEventStore *_eventStore = nullptr;

	// Should these really be inside the Engine class?
	LoadingScreen *_loadingScreen = nullptr;
	pyrodactyl::ui::SlideShow *_helpScreen = nullptr;
	pyrodactyl::ui::OptionMenu *_optionMenu = nullptr;
	pyrodactyl::ui::FileMenu<pyrodactyl::ui::SaveFileData> *_loadMenu = nullptr;

	FilePaths *_filePath = nullptr;
	TempValue *_tempData = nullptr;
	StringPool *_stringPool = nullptr;
	ScreenSettings *_screenSettings = nullptr;

	// Keeps a copy of latest screen for thumbnail
	Graphics::ManagedSurface *_thumbnail;

	// What components to draw lines for (if any)
	uint32 _debugDraw;

	Common::FSNode _gameDataDir;

	CrabEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~CrabEngine() override;
	void initializePath(const Common::FSNode &gamePath) override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsLoadingDuringRuntime) ||
			   (f == kSupportsSavingDuringRuntime) ||
			   (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently() override {
		return true;
	}

	bool canSaveGameStateCurrently() override;

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave) override;
	Common::Error loadGameState(int slot) override;
};

extern CrabEngine *g_engine;
#define SHOULD_QUIT ::Crab::g_engine->shouldQuit()

} // End of namespace Crab

#endif // CRAB_H
