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
#include "sprites.h"
#include "tostext.h"

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

protected:
	// Engine APIs
	Common::Error run() override;
public:
	bool _isRightMouseClicked = false;
	bool _isLeftMouseClicked = false;

	Nsp _baseSprites;
	Cursor _cursor;
	Graphics::Screen *_screen = nullptr;
	TosText *_tosText = nullptr;
	Console *_console = nullptr;
	Room *_room = nullptr;
	ActionMode _actionMode = PointerAction;
	Player *_player = nullptr;
	Sprites _sprites;
	Objects _objectVar;
	Inventory _inventory;

	int _currentDay = 1;
	int _currentTimeInSeconds = 0x7e8e;

	uint16 _previousRoomNumber = 0;
	uint16 targetRoomNumber = 0;

	bool isPlayingAnimation_maybe = false;
	uint16 otherNspAnimationType_maybe = 0;
	bool isWearingHeadBand = false;
	uint16 headAcheMessageCounter = 0;
	uint16 trunkPushCounter = 0;

	int sprite_y_scaling_threshold_maybe = 0xf0;
	int scaledWalkSpeed_maybe = 0;
	uint8 scaledSpriteWidth = 0;
	uint8 scaledSpriteHeight = 0;

	int nsp_sprite_scaling_y_position = 0;

	bool player_sprite_related_2c85_82f3 = false;
	int DAT_2c85_8326_blit_related = 0;
	bool isAnimFinished_maybe = false;
	bool animFrameChanged = false;
	char prefsCutsceneId = 'I';
	int animIndexTbl[20];
	int spriteAnimCountdownTimer[20];

	// Unknown variables
	bool _doorEnabled = false;
	uint16 DAT_2c85_825c = 0;
	uint16 DAT_2c85_819c = 0;
	bool _scaleSequence = false;
	bool useDoorTarget = false;

	uint16 DAT_2c85_81e0 = 0;

	int16 counter_2c85_888b = 0;

	uint8 targetPlayerDirection = 0; // related to changing rooms.

	uint8 systemTimerCounter = 0;

	bool _debugShowWalkPath = false;
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

	bool canLoadGameStateCurrently() override {
		return true;
	}
	bool canSaveGameStateCurrently() override {
		return true;
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
		return syncGame(s);
	}

	Common::String getRoomFilePath(const Common::String &filename);
	Common::String getPictureFilePath(const Common::String &filename);

	void fadeIn();
	void fadeOut();

	void updateDisplay();
	void setupOtherNspAnimation(int nspAnimIdx, int animId);
	void debugTeleportToRoom(int newRoomNumber, int entranceNumber);

private:
	void updateAnimation();
	void advanceAnimationFrame(int nspAminIdx);
	void fadeInner(int startValue, int endValue, int increment);
	void gameloop();
	void updateEvents();
	void handleInput();
	void wait();
	void handlePointerAction();
	void changeToRoom(int newRoomNumber);
	void handleObjCollision(int objNum);

	void eyeCode(int objNum);
	void wongame();
	void getPackage(int state);
};

extern DarkseedEngine *g_engine;
#define SHOULD_QUIT ::Darkseed::g_engine->shouldQuit()

} // End of namespace Darkseed

#endif // DARKSEED_H
