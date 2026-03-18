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
#include "harvester/startup_script.h"

struct ADGameDescription;

namespace Harvester {

class ResourceManager;
class RuntimeEntityManager;
class Script;
class Art;
class Text;

struct StartupSaveRoomState {
	Common::String entranceName;
	Common::String roomName;
	Common::String musicPath;
	int playerX = 0;
	int playerY = 0;
	int playerZ = 0;
	int playerFacing = -1;
	bool valid = false;

	void clear() {
		entranceName.clear();
		roomName.clear();
		musicPath.clear();
		playerX = 0;
		playerY = 0;
		playerZ = 0;
		playerFacing = -1;
		valid = false;
	}
};

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
	RuntimeEntityManager *getRuntimeEntities() const { return _runtimeEntities; }
	Graphics::Screen *getScreen() const { return _screen; }
	int getDisplayWidth() const { return _displayWidth; }
	int getDisplayHeight() const { return _displayHeight; }
	Script *getStartupScript() const { return _startupScript; }
	Art *getStartupArt() const { return _startupArt; }
	Text *getStartupText() const { return _startupText; }
	void setDisplayMode(int width, int height);
	bool isGoreEnabled() const;
	int getStartupFxVolumeLevel() const;
	int getStartupMusicVolumeLevel() const;
	int getStartupGammaLevel() const;
	float getStartupGammaBrightnessScale() const;
	void setStartupFxVolumeLevel(int level);
	void setStartupMusicVolumeLevel(int level);
	void setStartupGammaLevel(int level);
	const Common::String &getStartupMusicPath() const { return _startupMusicPath; }
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

private:
	void syncStartupSaveRoomState(Common::Serializer &s, StartupSaveRoomState &state);
	void stopStartupSoundHandle(Audio::SoundHandle &handle);
	bool validateStartupLoadedSoundSlot(int slot) const;
	void applyStartupMixerLevels();

	const ADGameDescription *const _gameDescription;
	Common::RandomSource _randomSource;
	Audio::SoundHandle _startupMusicHandle;
	Audio::SoundHandle _startupSingleSoundHandle;
	Audio::SoundHandle _startupSpeechHandle;
	Common::String _startupMusicPath;
	int _startupSoundSlotIndex = -1;
	Common::String _startupSoundPaths[8];
	Audio::SoundHandle _startupSoundHandles[8];
	Common::String _startupLoadedSoundPaths[4];
	Common::Array<byte> _startupLoadedSoundData[4];
	Audio::SoundHandle _startupLoadedSoundHandles[4];
	Graphics::Screen *_screen = nullptr;
	int _displayWidth = 0;
	int _displayHeight = 0;
	ResourceManager *_resources = nullptr;
	RuntimeEntityManager *_runtimeEntities = nullptr;
	Script *_startupScript = nullptr;
	Art *_startupArt = nullptr;
	Text *_startupText = nullptr;
	StartupSaveRoomState _currentStartupSaveRoomState;
	StartupSaveRoomState _pendingLoadedStartupSaveRoomState;
};

extern HarvesterEngine *g_engine;
#define SHOULD_QUIT ::Harvester::g_engine->shouldQuit()

} // End of namespace Harvester

#endif // HARVESTER_H
