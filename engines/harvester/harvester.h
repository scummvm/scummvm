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
class EntityManager;
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
	bool isDemo() const;

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
	EntityManager *getRuntimeEntities() const;
	Graphics::Screen *getScreen() const;
	int getDisplayWidth() const;
	int getDisplayHeight() const;
	Script *getScript() const { return _script; }
	Art *getArt() const;
	Text *getText() const;
	void setDisplayMode(int width, int height);
	bool isGoreEnabled() const;
	bool shouldShowCdChangePrompts() const;
	int getFxVolumeLevel() const;
	int getMusicVolumeLevel() const;
	int getGammaLevel() const;
	float getGammaBrightnessScale() const;
	void setFxVolumeLevel(int level);
	void setMusicVolumeLevel(int level);
	void setGammaLevel(int level);
	const Common::String &getMusicPath() const;
	bool isMusicPlaying() const;
	bool playMusic(const Common::String &path);
	void pauseMusic(bool paused);
	void stopMusic();
	bool executeAudioCommand(const AudioCommand &command);
	bool playSound(const Common::String &path);
	bool playSingleSound(const Common::String &path);
	void stopSingleSound();
	bool isSingleSoundPlaying() const;
	bool playSpeech(const Common::String &path);
	void stopSpeech();
	bool isSpeechPlaying() const;
	bool loadSound(int slot, const Common::String &path);
	bool playLoadedSound(int slot);
	bool deleteLoadedSound(int slot);
	void stopSound();
	bool activateDisc(int discNumber);
	bool isCombatDebugEnabled() const { return _combatDebugEnabled; }
	bool toggleCombatDebugEnabled();
	bool isRoomDebugEnabled() const { return _roomDebugEnabled; }
	bool toggleRoomDebugEnabled();
	bool isPathfindingDebugEnabled() const { return _pathfindingDebugEnabled; }
	bool togglePathfindingDebugEnabled();
	bool isTimerDebugEnabled() const { return _timerDebugEnabled; }
	bool toggleTimerDebugEnabled();
	bool requestDebugCommand(const CommandRecord &command);
	bool requestDebugRoomChange(const Common::String &roomName);
	void captureCurrentSaveRoomState(const Common::String &entranceName,
		const Common::String &roomName, int playerX, int playerY, int playerZ,
		int playerFacing, int discNumber, const Common::String &musicPath);
	void clearCurrentSaveRoomState();
	bool hasCurrentSaveRoomState() const { return _currentSaveRoomState.valid; }
	const SaveRoomState &getCurrentSaveRoomState() const {
		return _currentSaveRoomState;
	}
	bool hasPendingLoadedSaveRoomState() const { return _pendingLoadedSaveRoomState.valid; }
	const SaveRoomState &getPendingLoadedSaveRoomState() const {
		return _pendingLoadedSaveRoomState;
	}
	int getPendingLoadedDisc() const { return _pendingLoadedDisc; }
	void clearPendingLoadedSaveRoomState();
	bool hasPendingLoadedDialogueStateBlob() const { return !_pendingLoadedDialogueStateBlob.empty(); }
	const Common::Array<byte> &getPendingLoadedDialogueStateBlob() const { return _pendingLoadedDialogueStateBlob; }
	uint32 getPendingLoadedDialogueStateBlobVersion() const { return _pendingLoadedDialogueStateBlobVersion; }
	void clearPendingLoadedDialogueStateBlob() {
		_pendingLoadedDialogueStateBlob.clear();
		_pendingLoadedDialogueStateBlobVersion = 0;
	}

private:
	void applyMixerLevels();

	const ADGameDescription *const _gameDescription;
	Common::RandomSource _randomSource;
	ResourceManager *_resources = nullptr;
	MediaManager *_media = nullptr;
	Script *_script = nullptr;
	SaveRoomState _currentSaveRoomState;
	SaveRoomState _pendingLoadedSaveRoomState;
	int _pendingLoadedDisc = 0;
	Common::Array<byte> _pendingLoadedDialogueStateBlob;
	uint32 _pendingLoadedDialogueStateBlobVersion = 0;
	Flow *_activeFlow = nullptr;
	bool _combatDebugEnabled = false;
	bool _roomDebugEnabled = false;
	bool _pathfindingDebugEnabled = false;
	bool _timerDebugEnabled = false;
	Common::String _emptyMusicPath;
};

// Global engine hook used by existing Harvester helpers and macros.
// It is assigned in HarvesterEngine::HarvesterEngine() and cleared in
// HarvesterEngine::~HarvesterEngine() so restart-to-launcher resets it.
extern HarvesterEngine *g_engine;
#define SHOULD_QUIT ::Harvester::g_engine->shouldQuit()

} // End of namespace Harvester

#endif // HARVESTER_H
