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

#ifndef TETRAEDGE_TETRAEDGE_H
#define TETRAEDGE_TETRAEDGE_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"
#include "graphics/renderer.h"

#include "tetraedge/detection.h"

namespace Tetraedge {

struct TetraedgeGameDescription;

class Application;
class Game;
class TeCore;
class TeSoundManager;
class TeRenderer;
class TeResourceManager;
class TeInputMgr;

class TetraedgeEngine : public Engine {
public:
	enum TetraedgeGameType {
		kNone,
		kSyberia,
		kSyberia2,
		kAmerzone
	};

private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	TeCore *_core;
	Application *_application;
	Game *_game;
	TeSoundManager *_soundManager;
	TeRenderer *_renderer;
	TeResourceManager *_resourceManager;
	TeInputMgr *_inputMgr;
	enum TetraedgeGameType _gameType;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	TetraedgeEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~TetraedgeEngine() override;

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
		return
			(f == kSupportsLoadingDuringRuntime) ||
			(f == kSupportsSavingDuringRuntime) ||
			(f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	bool canSaveAutosaveCurrently() override;

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}

	static void getSavegameThumbnail(Graphics::Surface &thumb);

	Common::Error loadGameState(int slot) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	int getDefaultScreenWidth() const;
	int getDefaultScreenHeight() const;

	Application *getApplication();
	Game *getGame();
	TeCore *getCore();
	TeSoundManager *getSoundManager();
	TeRenderer *getRenderer();
	TeResourceManager *getResourceManager();
	TeInputMgr *getInputMgr();
	TetraedgeGameType gameType() const { return _gameType; }

	void openConfigDialog();
	bool onKeyUp(const Common::KeyState &state);

	static Common::StringArray splitString(const Common::String &text, char c);

	/* Pick the renderer type to use.
	   Currently will only return kRendererTypeOpenGL or kRendererTypeTinyGL */
	Graphics::RendererType preferredRendererType() const;

private:
	void configureSearchPaths();
	void registerConfigDefaults();
};

extern TetraedgeEngine *g_engine;

} // namespace Tetraedge

#endif
