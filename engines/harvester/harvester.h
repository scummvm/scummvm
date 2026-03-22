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

#ifndef HARVESTER_H
#define HARVESTER_H

#include "common/array.h"
#include "audio/mixer.h"
#include "common/error.h"
#include "common/random.h"
#include "common/serializer.h"
#include "engines/engine.h"
#include "graphics/screen.h"
#include "harvester/saveload.h"
#include "harvester/script.h"

struct ADGameDescription;

namespace Harvester {

class ResourceManager;
class RuntimeEntityManager;
class MediaManager;
class Script;
class Art;
class Text;
class Flow;

class HarvesterEngine : public Engine {
public:
	HarvesterEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~HarvesterEngine() override;

	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;

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

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	ResourceManager *getResources() const { return _resources; }
	MediaManager *getMedia() const { return _media; }
	RuntimeEntityManager *getRuntimeEntities() const;
	Graphics::Screen *getScreen() const;
	int getDisplayWidth() const;
	int getDisplayHeight() const;
	Script *getStartupScript() const { return _startupScript; }
	Art *getStartupArt() const;
	Text *getStartupText() const;
	void setDisplayMode(int width, int height);
	bool isGoreEnabled() const;
	int getStartupFxVolumeLevel() const;
	int getStartupMusicVolumeLevel() const;
	int getStartupGammaLevel() const;
	float getStartupGammaBrightnessScale() const;
	void setStartupFxVolumeLevel(int level);
	void setStartupMusicVolumeLevel(int level);
	void setStartupGammaLevel(int level);
	const Common::String &getStartupMusicPath() const;
	bool isStartupMusicPlaying() const;
	bool playStartupMusic(const Common::String &path);
	void pauseStartupMusic(bool paused);
	void stopStartupMusic();
	bool executeStartupAudioCommand(const StartupAudioCommand &command);
	bool playStartupSound(const Common::String &path);
	bool playStartupSingleSound(const Common::String &path);
	void stopStartupSingleSound();
	bool isStartupSingleSoundPlaying() const;
	bool playStartupSpeech(const Common::String &path);
	void stopStartupSpeech();
	bool isStartupSpeechPlaying() const;
	bool loadStartupSound(int slot, const Common::String &path);
	bool playStartupLoadedSound(int slot);
	bool deleteStartupLoadedSound(int slot);
	void stopStartupSound();
	bool isRoomDebugEnabled() const { return _roomDebugEnabled; }
	bool toggleRoomDebugEnabled();
	void captureCurrentStartupSaveRoomState(const Common::String &entranceName,
		const Common::String &roomName, int playerX, int playerY, int playerZ, int playerFacing,
		const Common::String &musicPath);
	void clearCurrentStartupSaveRoomState();
	bool hasCurrentStartupSaveRoomState() const { return _currentStartupSaveRoomState.valid; }
	bool hasPendingLoadedStartupSaveRoomState() const { return _pendingLoadedStartupSaveRoomState.valid; }
	const StartupSaveRoomState &getPendingLoadedStartupSaveRoomState() const {
		return _pendingLoadedStartupSaveRoomState;
	}
	void clearPendingLoadedStartupSaveRoomState();
	bool hasPendingLoadedDialogueStateBlob() const { return !_pendingLoadedDialogueStateBlob.empty(); }
	const Common::Array<byte> &getPendingLoadedDialogueStateBlob() const { return _pendingLoadedDialogueStateBlob; }
	uint32 getPendingLoadedDialogueStateBlobVersion() const { return _pendingLoadedDialogueStateBlobVersion; }
	void clearPendingLoadedDialogueStateBlob() {
		_pendingLoadedDialogueStateBlob.clear();
		_pendingLoadedDialogueStateBlobVersion = 0;
	}

private:
	void applyStartupMixerLevels();

	const ADGameDescription *const _gameDescription;
	Common::RandomSource _randomSource;
	ResourceManager *_resources = nullptr;
	MediaManager *_media = nullptr;
	Script *_startupScript = nullptr;
	StartupSaveRoomState _currentStartupSaveRoomState;
	StartupSaveRoomState _pendingLoadedStartupSaveRoomState;
	Common::Array<byte> _pendingLoadedDialogueStateBlob;
	uint32 _pendingLoadedDialogueStateBlobVersion = 0;
	Flow *_activeFlow = nullptr;
	bool _roomDebugEnabled = false;
};

extern HarvesterEngine *g_engine;
#define SHOULD_QUIT ::Harvester::g_engine->shouldQuit()

} // End of namespace Harvester

#endif // HARVESTER_H
