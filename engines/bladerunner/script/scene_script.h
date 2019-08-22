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

#ifndef BLADERUNNER_SCRIPT_SCENE_H
#define BLADERUNNER_SCRIPT_SCENE_H

#include "bladerunner/script/script.h"

namespace BladeRunner {

class BladeRunnerEngine;

class SceneScriptBase : public ScriptBase {
public:
	SceneScriptBase(BladeRunnerEngine *vm) : ScriptBase(vm) {}

	virtual void InitializeScene() = 0;
	virtual void SceneLoaded() = 0;
	virtual bool MouseClick(int x, int y) = 0;
	virtual bool ClickedOn3DObject(const char *objectName, bool combatMode) = 0;
	virtual bool ClickedOnActor(int actorId) = 0;
	virtual bool ClickedOnItem(int itemId, bool combatMode) = 0;
	virtual bool ClickedOnExit(int exitId) = 0;
	virtual bool ClickedOn2DRegion(int region) = 0;
	virtual void SceneFrameAdvanced(int frame) = 0;
	virtual void ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) = 0;
	virtual void PlayerWalkedIn() = 0;
	virtual void PlayerWalkedOut() = 0;
	virtual void DialogueQueueFlushed(int a1) = 0;
};

#define DECLARE_SCRIPT(name) \
class SceneScript##name : public SceneScriptBase { \
public: \
	SceneScript##name(BladeRunnerEngine *vm) \
		: SceneScriptBase(vm) \
	{} \
	void InitializeScene(); \
	void SceneLoaded(); \
	bool MouseClick(int x, int y); \
	bool ClickedOn3DObject(const char *objectName, bool combatMode); \
	bool ClickedOnActor(int actorId); \
	bool ClickedOnItem(int itemId, bool combatMode); \
	bool ClickedOnExit(int exitId); \
	bool ClickedOn2DRegion(int region); \
	void SceneFrameAdvanced(int frame); \
	void ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet); \
	void PlayerWalkedIn(); \
	void PlayerWalkedOut(); \
	void DialogueQueueFlushed(int a1); \
private:
#define END_SCRIPT };

DECLARE_SCRIPT(AR01)
	void dialogueWithFishDealerBuyGoldfish();
END_SCRIPT

DECLARE_SCRIPT(AR02)
	void dialogueWithInsectDealer1();
	void dialogueWithInsectDealerBuyBracelet();
	void dialogueWithInsectDealerBuySlug();
	void dialogueWithHassan();
	void dialogueWithHassanBuySnake();
END_SCRIPT

DECLARE_SCRIPT(BB01)
END_SCRIPT

DECLARE_SCRIPT(BB02)
END_SCRIPT

DECLARE_SCRIPT(BB03)
END_SCRIPT

DECLARE_SCRIPT(BB04)
END_SCRIPT

DECLARE_SCRIPT(BB05)
END_SCRIPT

DECLARE_SCRIPT(BB06)
END_SCRIPT

DECLARE_SCRIPT(BB07)
END_SCRIPT

DECLARE_SCRIPT(BB08)
END_SCRIPT

DECLARE_SCRIPT(BB09)
END_SCRIPT

DECLARE_SCRIPT(BB10)
END_SCRIPT

DECLARE_SCRIPT(BB11)
END_SCRIPT

DECLARE_SCRIPT(BB12)
END_SCRIPT

DECLARE_SCRIPT(BB51)
END_SCRIPT

DECLARE_SCRIPT(CT01)
	void dialogueWithHowieLee();
END_SCRIPT

DECLARE_SCRIPT(CT02)
	void dialogueWithZuben();
END_SCRIPT

DECLARE_SCRIPT(CT03)
END_SCRIPT

DECLARE_SCRIPT(CT04)
	void dialogueWithHomeless();
END_SCRIPT

DECLARE_SCRIPT(CT05)
END_SCRIPT

DECLARE_SCRIPT(CT06)
END_SCRIPT

DECLARE_SCRIPT(CT07)
END_SCRIPT

DECLARE_SCRIPT(CT08)
END_SCRIPT

DECLARE_SCRIPT(CT09)
END_SCRIPT

DECLARE_SCRIPT(CT10)
	void checkCabinet();
END_SCRIPT

DECLARE_SCRIPT(CT11)
END_SCRIPT

DECLARE_SCRIPT(CT12)
END_SCRIPT

DECLARE_SCRIPT(CT51)
END_SCRIPT

DECLARE_SCRIPT(DR01)
END_SCRIPT

DECLARE_SCRIPT(DR02)
END_SCRIPT

DECLARE_SCRIPT(DR03)
	void dialogueWithChew();
END_SCRIPT

DECLARE_SCRIPT(DR04)
	bool farEnoughFromExplosion();
END_SCRIPT

DECLARE_SCRIPT(DR05)
END_SCRIPT

DECLARE_SCRIPT(DR06)
END_SCRIPT

DECLARE_SCRIPT(HC01)
	void dialogueWithIzo();
	void takePhotoAndRunAway();
END_SCRIPT

DECLARE_SCRIPT(HC02)
END_SCRIPT

DECLARE_SCRIPT(HC03)
END_SCRIPT

DECLARE_SCRIPT(HC04)
	void dialogueWithIsabella();
END_SCRIPT

DECLARE_SCRIPT(HF01)
	void dialogueWithMiaAndMurray();
	void talkWithDektora();
	void talkWithLucy();
END_SCRIPT

DECLARE_SCRIPT(HF02)
END_SCRIPT

DECLARE_SCRIPT(HF03)
	void dialogueWithLucy();
END_SCRIPT

DECLARE_SCRIPT(HF04)
END_SCRIPT

DECLARE_SCRIPT(HF05)
	void talkWithCrazyLegs1();
	void talkWithCrazyLegs2();
	void dialogueWithCrazylegs1();
	void dialogueWithCrazylegs2();
	void talkWithCrazylegs3(int actorId);
	void talkWithDektora();
	void talkWithLucy();
	void policeAttack();
	void addAmbientSounds();
	int getAffectionTowardsActor();
	int getCompanionActor();
END_SCRIPT

DECLARE_SCRIPT(HF06)
	void steelInterruption();
	void addAmbientSounds();
END_SCRIPT

DECLARE_SCRIPT(HF07)
	int getAffectionTowardsActor();
END_SCRIPT

DECLARE_SCRIPT(KP01)
END_SCRIPT

DECLARE_SCRIPT(KP02)
END_SCRIPT

DECLARE_SCRIPT(KP03)
	void saveSteele();
END_SCRIPT

DECLARE_SCRIPT(KP04)
END_SCRIPT

DECLARE_SCRIPT(KP05)
END_SCRIPT

DECLARE_SCRIPT(KP06)
END_SCRIPT

DECLARE_SCRIPT(KP07)
END_SCRIPT

DECLARE_SCRIPT(MA01)
END_SCRIPT

DECLARE_SCRIPT(MA02)
	void talkWithRajif();
	bool isPhoneRinging();
	void selectNextTvNews();
END_SCRIPT

//MA03 does not exists

DECLARE_SCRIPT(MA04)
	bool isPhoneRinging();
	bool isPhoneMessageWaiting();
	void phoneCallWithDektora();
	void phoneCallWithLucy();
	void phoneCallWithSteele();
	void phoneCallWithClovis();
	void turnOnTV();
	void sleep();
END_SCRIPT

DECLARE_SCRIPT(MA05)
	bool isPhoneRinging();
END_SCRIPT

DECLARE_SCRIPT(MA06)
	bool isElevatorOnDifferentFloor();
	void activateElevator();
END_SCRIPT

DECLARE_SCRIPT(MA07)
END_SCRIPT

DECLARE_SCRIPT(MA08)
END_SCRIPT

DECLARE_SCRIPT(NR01)
END_SCRIPT

DECLARE_SCRIPT(NR02)
	void playNextMusic();
END_SCRIPT

DECLARE_SCRIPT(NR03)
	void rotateActorOnTable(int frame);
	void playNextMusic();
END_SCRIPT

DECLARE_SCRIPT(NR04)
	void dialogueWithEarlyQ();
	void druggedEffect(int frame);
	void playNextMusic();
END_SCRIPT

DECLARE_SCRIPT(NR05)
	void rotateActorOnTable(int frame);
	void talkToBartender();
	void talkToEarlyQ();
	void rotateActorOnGround(int actorId);
	void playNextMusic();
END_SCRIPT

DECLARE_SCRIPT(NR06)
	void playNextMusic();
END_SCRIPT

DECLARE_SCRIPT(NR07)
	void dektoraRunAway();
	void callHolloway();
	void clickedOnVase();
	void talkAboutBelt1();
	void talkAboutBelt2();
	void talkAboutVoightKampff();
	void talkAboutSteele();
	void talkAboutMoonbus();
	void talkAboutBlackSedan();
	void talkAboutScorpions();
END_SCRIPT

DECLARE_SCRIPT(NR08)
	void playNextMusic();
END_SCRIPT

DECLARE_SCRIPT(NR09)
	void playNextMusic();
END_SCRIPT

DECLARE_SCRIPT(NR10)
END_SCRIPT

DECLARE_SCRIPT(NR11)
	void actorSweepArea(int actorId, signed int frame);
	void untargetEverything();
END_SCRIPT

DECLARE_SCRIPT(PS01)
END_SCRIPT

DECLARE_SCRIPT(PS02)
	void activateElevator();
END_SCRIPT

DECLARE_SCRIPT(PS03)
END_SCRIPT

DECLARE_SCRIPT(PS04)
	void dialogueWithGuzza();
END_SCRIPT

DECLARE_SCRIPT(PS05)
	void selectNextTvNews();
	void turnOnTV();
END_SCRIPT

DECLARE_SCRIPT(PS06)
END_SCRIPT

DECLARE_SCRIPT(PS07)
END_SCRIPT

// PS08 does not exits

DECLARE_SCRIPT(PS09)
	void dialogueWithGrigorian();
END_SCRIPT

DECLARE_SCRIPT(PS10)
	void removeTargets();
	public:
	static int getPoliceMazePS10TargetCount();
END_SCRIPT

DECLARE_SCRIPT(PS11)
	void removeTargets();
	public:
	static int getPoliceMazePS11TargetCount();
END_SCRIPT

DECLARE_SCRIPT(PS12)
	void removeTargets();
	public:
	static int getPoliceMazePS12TargetCount();
END_SCRIPT

DECLARE_SCRIPT(PS13)
	void removeTargets();
	public:
	static int getPoliceMazePS13TargetCount();
END_SCRIPT

DECLARE_SCRIPT(PS14)
END_SCRIPT

DECLARE_SCRIPT(PS15)
END_SCRIPT

DECLARE_SCRIPT(RC01)
	void interrogateCrowd();
	void walkToCenter();
END_SCRIPT

DECLARE_SCRIPT(RC02)
	void dialogueWithRunciter();
END_SCRIPT

DECLARE_SCRIPT(RC03)
	void talkWithSteele();
END_SCRIPT

DECLARE_SCRIPT(RC04)
	void dialogueWithBulletBob();
END_SCRIPT

DECLARE_SCRIPT(RC51)
END_SCRIPT

DECLARE_SCRIPT(TB02)
	void dialogueWithTyrellGuard();
	void dialogueWithSteele();
END_SCRIPT

DECLARE_SCRIPT(TB03)
END_SCRIPT

DECLARE_SCRIPT(TB05)
END_SCRIPT

DECLARE_SCRIPT(TB06)
END_SCRIPT

DECLARE_SCRIPT(TB07)
	void McCoyTalkWithRachaelAndTyrell();
END_SCRIPT

DECLARE_SCRIPT(UG01)
END_SCRIPT

DECLARE_SCRIPT(UG02)
	bool walkToCenter();
END_SCRIPT

DECLARE_SCRIPT(UG03)
END_SCRIPT

DECLARE_SCRIPT(UG04)
END_SCRIPT

DECLARE_SCRIPT(UG05)
	int getAffectionTowardsActor();
	void endGame();
END_SCRIPT

DECLARE_SCRIPT(UG06)
END_SCRIPT

DECLARE_SCRIPT(UG07)
END_SCRIPT

DECLARE_SCRIPT(UG08)
END_SCRIPT

DECLARE_SCRIPT(UG09)
END_SCRIPT

DECLARE_SCRIPT(UG10)
END_SCRIPT

// UG11 does not exists

DECLARE_SCRIPT(UG12)
END_SCRIPT

DECLARE_SCRIPT(UG13)
	void talkAboutGuzza();
	void dialogueWithHomeless1();
	void dialogueWithHomeless2();
END_SCRIPT

DECLARE_SCRIPT(UG14)
END_SCRIPT

DECLARE_SCRIPT(UG15)
END_SCRIPT

DECLARE_SCRIPT(UG16)
	void dialogueWithLuther();
END_SCRIPT

DECLARE_SCRIPT(UG17)
END_SCRIPT

DECLARE_SCRIPT(UG18)
	void talkWithGuzza();
	void talkWithClovis();
END_SCRIPT

DECLARE_SCRIPT(UG19)
END_SCRIPT

#undef DECLARE_SCRIPT
#undef END_SCRIPT

class SceneScript {
protected:
	BladeRunnerEngine *_vm;
	int                _inScriptCounter;
	SceneScriptBase   *_currentScript;

public:
	int _mouseX;
	int _mouseY;

	SceneScript(BladeRunnerEngine *vm);
	~SceneScript();

	bool open(const Common::String &name);

	void initializeScene();
	void sceneLoaded();
	bool mouseClick(int x, int y);
	bool clickedOn3DObject(const char *objectName, bool combatMode);
	bool clickedOnActor(int actorId);
	bool clickedOnItem(int itemId, bool combatMode);
	bool clickedOnExit(int exitId);
	bool clickedOn2DRegion(int region);
	void sceneFrameAdvanced(int frame);
	void actorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet);
	void playerWalkedIn();
	void playerWalkedOut();
	void dialogueQueueFlushed(int a1);
	bool isInsideScript() const { return _inScriptCounter > 0; }
};

} // End of namespace BladeRunner

#endif
