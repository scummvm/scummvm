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

#ifndef HOLLYWOOD_HOLLYWOOD_H
#define HOLLYWOOD_HOLLYWOOD_H

#include "common/language.h"
#include "common/platform.h"

#include "engines/engine.h"
#include "graphics/surface.h"
#include "hollywood/gameplay/cursor.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/music.h"

struct ADGameDescription;

namespace Common {
class Serializer;
}

namespace Hollywood {

enum HollywoodDebugChannels {
	kDebugGeneral = 1,
	kDebugResources,
	kDebugScene,
	kDebugPath
};

class ResourceManager;
class HollywoodFont;

class HollywoodEngine : public Engine {
public:
	HollywoodEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~HollywoodEngine() override;

	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	void syncSoundSettings() override;
	void syncSoundSettingsFromGameState();

	const ADGameDescription *getGameDescription() const { return _gameDescription; }
	const char *getGameId() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

	ResourceManager *resources() const { return _resources; }
	HollywoodFont *font() const { return _font; }
	HollywoodCursor *cursor() { return &_cursor; }
	MusicPlayer *introMusic() { return &_introMusic; }
	MusicPlayer *gameplayMusic() { return &_gameplayMusic; }
	GameplayState &gameState() { return _gameState; }
	const GameplayState &gameState() const { return _gameState; }
	bool subtitlesEnabled() const;
	bool restoredContentEnabled() const;
	void captureLastGameplayThumbnail();
	bool copyLastGameplayThumbnail(Graphics::Surface &thumbnail) const;
	bool isSceneRestartRequested() const { return _sceneRestartRequested; }
	void requestSceneRestart() { _sceneRestartRequested = true; }
	void clearSceneRestartRequest() { _sceneRestartRequested = false; }
	bool canSave() const { return _canSave; }
	void setCanSave(bool canSave) { _canSave = canSave; }

	enum {
		kScreenWidth = 640,
		kScreenHeight = 480,
		kSceneBufferWidth = 1024,
		kSceneBufferHeight = 480,
		kResourceChunkCount = 40,
		kSceneItemCount = 21
	};

private:
	Common::Error syncGameStream(Common::Serializer &s);
	void normalizeLoadedGameState();

	const ADGameDescription *_gameDescription;
	ResourceManager *_resources;
	HollywoodFont *_font;
	HollywoodCursor _cursor;
	MusicPlayer _introMusic;
	MusicPlayer _gameplayMusic;
	GameplayState _gameState;
	Graphics::Surface _lastGameplayThumbnail;
	bool _lastGameplayThumbnailValid;
	bool _sceneRestartRequested;
	bool _canSave;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_HOLLYWOOD_H
