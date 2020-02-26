/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TEENAGENT_TEENAGENT_H
#define TEENAGENT_TEENAGENT_H

#include "engines/engine.h"

#include "audio/mixer.h"

#include "common/random.h"
#include "common/array.h"

#include "gui/debugger.h"

#include "teenagent/console.h"
#include "teenagent/dialog.h"

struct ADGameDescription;

namespace Audio {
class AudioStream;
}

namespace Common {
struct Point;
}

/**
 * This is the namespace of the TeenAgent engine.
 *
 * Status of this engine: Complete
 *
 * Games using this engine:
 * - Teen Agent
 */
namespace TeenAgent {

struct Object;
struct UseHotspot;
class Scene;
class MusicPlayer;
class Resources;
class Inventory;
class Pack;

// Engine Debug Flags
enum {
	kDebugActor     = (1 << 0),
	kDebugAnimation = (1 << 1),
	kDebugCallbacks = (1 << 2),
	kDebugDialog    = (1 << 3),
	kDebugFont      = (1 << 4),
	kDebugInventory = (1 << 5),
	kDebugMusic     = (1 << 6),
	kDebugObject    = (1 << 7),
	kDebugPack      = (1 << 8),
	kDebugScene     = (1 << 9),
	kDebugSurface   = (1 << 10)
};

const uint16 kScreenWidth = 320;
const uint16 kScreenHeight = 200;

class TeenAgentEngine : public Engine {
public:
	TeenAgentEngine(OSystem *system, const ADGameDescription *gd);
	~TeenAgentEngine();

	Common::Error run() override;
	Common::String getSaveStateName(int slot) const override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool canLoadGameStateCurrently() override { return true; }
	bool canSaveGameStateCurrently() override { return !_sceneBusy; }
	bool hasFeature(EngineFeature f) const override;

	void init();

	enum Action { kActionNone, kActionExamine, kActionUse };

	void examine(const Common::Point &point, Object *object);
	void use(Object *object);
	inline void cancel() { _action = kActionNone; }

	bool processCallback(uint16 addr);
	inline Scene *getScene() { return scene; }

	bool showLogo();
	bool showCDLogo();
	bool showMetropolis();
	int skipEvents() const;

	Common::String parseMessage(uint16 addr);

	//event driven:
	void displayMessage(uint16 addr, byte color = textColorMark, uint16 x = 0, uint16 y = 0);
	void displayMessage(const Common::String &str, byte color = textColorMark, uint16 x = 0, uint16 y = 0);
	void displayAsyncMessage(uint16 addr, uint16 x, uint16 y, uint16 firstFrame, uint16 lastFrame, byte color = textColorMark);
	void displayAsyncMessageInSlot(uint16 addr, byte slot, uint16 firstFrame, uint16 lastFrame, byte color = textColorMark);
	void displayCredits(uint16 addr, uint16 timer = 0);
	void displayCutsceneMessage(uint16 addr, uint16 x, uint16 y);
	void moveTo(const Common::Point &dst, byte o, bool warp = false);
	void moveTo(uint16 x, uint16 y, byte o, bool warp = false);
	void moveTo(Object *obj);
	void moveRel(int16 x, int16 y, byte o, bool warp = false);
	void playActorAnimation(uint16 id, bool async = false, bool ignore = false);
	void playAnimation(uint16 id, byte slot, bool async = false, bool ignore = false, bool loop = false);
	void loadScene(byte id, const Common::Point &pos, byte o = 0);
	void loadScene(byte id, uint16 x, uint16 y, byte o = 0);
	void enableOn(bool enable = true);
	void setOns(byte id, byte value, byte sceneId = 0);
	void setLan(byte id, byte value, byte sceneId = 0);
	void setFlag(uint16 addr, byte value);
	byte getFlag(uint16 addr);
	void reloadLan();
	void rejectMessage();

	void playMusic(byte id); //schedules play
	void playSound(byte id, byte skipFrames);
	void playSoundNow(Pack *pack, byte id);
	void enableObject(byte id, byte sceneId = 0);
	void disableObject(byte id, byte sceneId = 0);
	void hideActor();
	void showActor();
	void waitAnimation();
	void waitLanAnimationFrame(byte slot, uint16 frame);
	void setTimerCallback(uint16 addr, uint16 frames);
	void shakeScreen();
	void displayCredits();
	void fadeIn();
	void fadeOut();
	void wait(uint16 frames);

	Common::RandomSource _rnd;

	Resources *res;
	Scene *scene;
	Inventory *inventory;
	MusicPlayer *music;
	Dialog *dialog;

	void setMusic(byte id);

private:
	void processObject();
	bool trySelectedObject();

	bool _sceneBusy;
	Action _action;
	Object *_dstObject;

	Audio::AudioStream *_musicStream;
	Audio::SoundHandle _musicHandle, _soundHandle;
	const ADGameDescription *_gameDescription;

	uint _markDelay, _gameDelay;

	Common::Array<Common::Array<UseHotspot> > _useHotspots;

	void fnIntro();
	void fnPoleClimbFail();
	void fnGotAnchor();
	void fnGetOutOfLake();
	void fnGuardDrinking();
	void fnEgoDefaultPosition();
	void fnEnterCave();
	void fnEgoScaredBySpider();
	void fnMoveToLadderAndLeaveCellar();
	void fnLeaveCellar();
	void fnPutRockInHole();
	void fnEgoBottomRightTurn();
	bool fnCheckingDrawers();
	void fnDrawerOpenMessage();
	bool fnRobotSafeAlreadyUnlockedCheck();
	void fnRobotSafeUnlockCheck();
	bool fnMansionIntrusionAttempt();
	void fnSecondMansionIntrusion();
	void fnThirdMansionIntrusion();
	void fnFourthMansionIntrusion();
	void fnFifthMansionIntrusion();
	void fnSixthMansionIntrusion();
	void fnTooDark();
	bool fnIsCookGone();
	void fnEgoSuspiciousPosition();
	void fnGivingFlowerToOldLady();
	void fnGiveAnotherFlowerToOldLady();
	void fnGivingFlowerToAnne();
	void fnGiveAnotherFlowerToAnne();
};

} // End of namespace TeenAgent

#endif
