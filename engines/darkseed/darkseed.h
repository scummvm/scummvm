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

#ifndef DARKSEED_H
#define DARKSEED_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/screen.h"

#include "darkseed/animation.h"
#include "darkseed/console.h"
#include "darkseed/cursor.h"
#include "darkseed/cutscene.h"
#include "darkseed/detection.h"
#include "darkseed/inventory.h"
#include "darkseed/nsp.h"
#include "darkseed/objects.h"
#include "darkseed/player.h"
#include "darkseed/room.h"
#include "darkseed/sound.h"
#include "darkseed/sprites.h"
#include "darkseed/tostext.h"
#include "darkseed/usecode.h"

namespace Darkseed {

struct DarkseedGameDescription;

enum ActionMode : uint8 {
	kPointerAction = 0,
	kHandAction = 2,
	kLookAction = 3,
	kUseStickAction = 19,
	kUseHammerAction = 27,
};

enum class FadeDirection : uint8 {
	NONE,
	IN,
	OUT
};

class DarkseedEngine : public Engine {
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Pic *_fullscreenPic = nullptr;
	bool _timeAdvanceEventSelected = false;
	uint8 _delbertspeech = 0;
	int16 _yvec = 0; //delbert throw stick related.
	bool _normalWorldSpritesLoaded = true;
	bool _redrawFrame = true;
	bool _restartGame = false;

	FadeDirection _fadeDirection = FadeDirection::NONE;
	uint8 _fadeStepCounter = 0;
	Pal _fadeTempPalette;
	Pal _fadeTargetPalette;

protected:
	// Engine APIs
	Common::Error run() override;
public:
	Pic _frame;

	bool _ct_voice_status = false;
	bool _isRightMouseClicked = false;
	bool _isLeftMouseClicked = false;

	Sound *_sound = nullptr;
	Nsp _baseSprites;
	Cursor _cursor;
	Graphics::Screen *_screen = nullptr;
	TosText *_tosText = nullptr;
	Console *_console = nullptr;
	Room *_room = nullptr;
	int _actionMode = kPointerAction;
	Player *_player = nullptr;
	Sprites _sprites;
	Objects _objectVar;
	Inventory _inventory;
	UseCode *_useCode = nullptr;
	Cutscene _cutscene;
	Animation *_animation = nullptr;

	uint8 _currentDay = 1;
	int _currentTimeInSeconds = 0x7e8e;
	int _fttime = 0;

	uint8 _previousRoomNumber = 0;
	uint16 _targetRoomNumber = 0;

	uint16 _headAcheMessageCounter = 0;
	uint8 _headacheMessageIdx = 0;

	int _sprite_y_scaling_threshold_maybe = 0xf0;
	int _scaledWalkSpeed_maybe = 0;
	uint16 _scaledSpriteWidth = 0;
	uint16 _scaledSpriteHeight = 0;

	int _frameBottom = 0;

	// Unknown variables
	bool _doorEnabled = false;
	bool _useDoorTarget = false;

	int16 _counter_2c85_888b = 0;

	uint8 _targetPlayerDirection = 0; // related to changing rooms.

	uint8 _systemTimerCounter = 0;

	bool _debugShowWalkPath = false;
	int _phoneStatus = 0;

	int16 _soundTimer = 0;
	bool _printedcomeheredawson = false;
	void zeroMouseButtons();

	void gotoNextMorning();

	void playDayChangeCutscene();

	void wonGame();

	void removeFullscreenPic();

	void wait();

	void waitForSpeech();

	void syncSoundSettings() override;

public:
	DarkseedEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~DarkseedEngine() override;

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

	bool isCdVersion() const {
		return getFeatures() & ADGF_CD;
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg) override {
		return !_animation->_isPlayingAnimation_maybe && !_player->_isAutoWalkingToBed && !_player->_heroWaiting;
	}

	bool canSaveGameStateCurrently(Common::U32String *msg) override {
		return !_animation->_isPlayingAnimation_maybe && !_player->_isAutoWalkingToBed && !_player->_heroWaiting;
	}

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}

	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		Common::Error syncResult = syncGame(s);
		if (syncResult.getCode() == Common::kNoError) {
			changeToRoom(_room->_roomNumber);
		}
		return syncResult;
	}

	Common::Path getRoomFilePath(const Common::Path &filename) const;
	Common::Path getPictureFilePath(const Common::Path &filename) const;

	void fadeIn(const Pal &palette);
	void fadeOut();
	bool fadeStep();

	void restartGame();
	void newGame();

	void updateDisplay();
	void debugTeleportToRoom(int newRoomNumber, int entranceNumber);
	void showFullscreenPic(const Common::Path &filename);
	void drawFullscreenPic();
	void lookCode(int objNum);
	void handleObjCollision(int targetObjNum);
	void playSound(uint8 sfxId, uint8 unk1, int16 unk2);
	void nextFrame(int nspAminIdx);

	void throwmikeinjail();
	void runObjects();
	void getPackageObj(int packageType);
	void printTime();
	void changeToRoom(int newRoomNumber, bool placeDirectly = false);
	void waitxticks(int ticks);

private:
	void updateBaseSprites();
	void gameLoop();
	void updateEvents();
	void handleInput();
	void handlePointerAction();
	void loadRoom(int roomNumber);

	void gotoSleepInJail();

	void updateHeadache();
	void closeShops();
	void initDelbertAtSide();
	void movePlayerToDelbert();
	void delbertThrowStick(int16 spriteNum);
	void leavePackage();
};

extern DarkseedEngine *g_engine;
#define SHOULD_QUIT ::Darkseed::g_engine->shouldQuit()

} // End of namespace Darkseed

#endif // DARKSEED_H
