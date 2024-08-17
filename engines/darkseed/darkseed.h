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
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "console.h"
#include "cursor.h"
#include "darkseed/detection.h"
#include "inventory.h"
#include "nsp.h"
#include "objects.h"
#include "player.h"
#include "room.h"
#include "sound.h"
#include "sprites.h"
#include "tostext.h"
#include "usecode.h"

namespace Darkseed {

struct DarkseedGameDescription;

enum ActionMode {
	PointerAction = 0,
	HandAction = 2,
	LookAction = 3,
	Unk19Action = 19,
	Unk27Action = 27,
};

class DarkseedEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Pic _frame;
	Pic *_fullscreenPic = nullptr;
	bool _timeAdvanceEventSelected = false;
	uint8 _delbertspeech = 0;
	int16 _yvec = 0; //delbert throw stick related.

protected:
	// Engine APIs
	Common::Error run() override;
public:
	bool _FrameAdvanced = false;
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
	int _actionMode = PointerAction;
	Player *_player = nullptr;
	Sprites _sprites;
	Objects _objectVar;
	Inventory _inventory;
	UseCode *_useCode = nullptr;

	uint8 _currentDay = 1;
	int _currentTimeInSeconds = 0x7e8e;
	int _fttime = 0;

	uint8 _previousRoomNumber = 0;
	uint16 targetRoomNumber = 0;

	bool isPlayingAnimation_maybe = false;
	uint16 otherNspAnimationType_maybe = 0;
	uint16 headAcheMessageCounter = 0;
	uint8 headacheMessageIdx = 0;

	int sprite_y_scaling_threshold_maybe = 0xf0;
	int scaledWalkSpeed_maybe = 0;
	uint8 scaledSpriteWidth = 0;
	uint8 scaledSpriteHeight = 0;

	int nsp_sprite_scaling_y_position = 0;

	bool player_sprite_related_2c85_82f3 = false;
	int _frameBottom = 0;
	bool isAnimFinished_maybe = false;
	bool animFrameChanged = false;
	char prefsCutsceneId = 'I';
	int animIndexTbl[30];
	int spriteAnimCountdownTimer[30];

	// Unknown variables
	bool _doorEnabled = false;
	bool _scaleSequence = false;
	bool useDoorTarget = false;

	int16 counter_2c85_888b = 0;

	uint8 targetPlayerDirection = 0; // related to changing rooms.

	uint8 systemTimerCounter = 0;

	bool _debugShowWalkPath = false;
	int _phoneStatus = 0;

	int16 _SoundTimer = 0;
	bool _printedcomeheredawson = false;
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

	bool isCdVersion() {
		return getFeatures() & ADGF_CD;
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg) override {
		return !isPlayingAnimation_maybe && !_player->_isAutoWalkingToBed && !_player->_herowaiting;
	}

	bool canSaveGameStateCurrently(Common::U32String *msg) override {
		return !isPlayingAnimation_maybe && !_player->_isAutoWalkingToBed && !_player->_herowaiting;
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

	Common::Path getRoomFilePath(const Common::Path &filename);
	Common::Path getPictureFilePath(const Common::Path &filename);

	void fadeIn();
	void fadeOut();

	void updateDisplay();
	void setupOtherNspAnimation(int nspAnimIdx, int animId);
	void debugTeleportToRoom(int newRoomNumber, int entranceNumber);
	void showFullscreenPic(const Common::Path &filename);
	void lookCode(int objNum);
	void handleObjCollision(int targetObjNum);
	void playCutscene(const Common::String cutsceneId);
	void playSound(int16 unk, uint8 unk1, int16 unk2);
	void nextFrame(int nspAminIdx);
	void stuffPlayer();

	void throwmikeinjail();
	void runObjects();
	void getPackageObj(int packageType);
private:
	void updateAnimation();
	void advanceAnimationFrame(int nspAminIdx);
	void fadeInner(int startValue, int endValue, int increment);
	void gameloop();
	void updateEvents();
	void handleInput();
	void wait();
	void handlePointerAction();
	void loadRoom(int roomNumber);
	void changeToRoom(int newRoomNumber);

	void wongame();
	void printTime();
	void keeperanim();
	void sargoanim();

	void gotosleepinjail();
	void gotonextmorning();

	void updateHeadache();
	void playDayChangeCutscene();
	void closeShops();
	void initDelbertAtSide();
	void moveplayertodelbert();
	void rundrekethsequence();
	void delthrowstick(int16 spriteNum);
	void leavepackage();
};

extern DarkseedEngine *g_engine;
#define SHOULD_QUIT ::Darkseed::g_engine->shouldQuit()

} // End of namespace Darkseed

#endif // DARKSEED_H
