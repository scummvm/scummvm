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
	virtual bool ClickedOn3DObject(const char *objectName, bool attack) = 0;
	virtual bool ClickedOnActor(int actorId) = 0;
	virtual bool ClickedOnItem(int itemId, bool a2) = 0;
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
	bool ClickedOn3DObject(const char *objectName, bool attack); \
	bool ClickedOnActor(int actorId); \
	bool ClickedOnItem(int itemId, bool a2); \
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
END_SCRIPT

DECLARE_SCRIPT(AR02)
	void sub_402694();
	void sub_402AE0();
	void sub_402CE4();
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
	void sub_401844();
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
	void sub_401B18();
END_SCRIPT

DECLARE_SCRIPT(DR04)
	bool sub_401160();
END_SCRIPT

DECLARE_SCRIPT(DR05)
END_SCRIPT

DECLARE_SCRIPT(DR06)
END_SCRIPT

DECLARE_SCRIPT(HC01)
	void sub_402384();
	void sub_40346C();
END_SCRIPT

DECLARE_SCRIPT(HC02)
END_SCRIPT

DECLARE_SCRIPT(HC03)
END_SCRIPT

DECLARE_SCRIPT(HC04)
	void sub_401B90();
END_SCRIPT

DECLARE_SCRIPT(HF01)
	void sub_4026B4();
	void sub_4032DC();
	void sub_403484();
END_SCRIPT

DECLARE_SCRIPT(HF02)
END_SCRIPT

DECLARE_SCRIPT(HF03)
	void sub_401C80();
END_SCRIPT

DECLARE_SCRIPT(HF04)
END_SCRIPT

DECLARE_SCRIPT(HF05)
	void sub_402370();
	void sub_402970();
	void sub_402AE4();
	void sub_403738();
	void sub_403A34(int actorId);
	void sub_403F0C();
	void sub_40410C();
	void sub_4042E4();
	void sub_404474();
	int sub_404858();
	int sub_4048C0();
END_SCRIPT

DECLARE_SCRIPT(HF06)
	void sub_401EF4();
	void sub_4023E0();
END_SCRIPT

DECLARE_SCRIPT(HF07)
	int sub_401864();
END_SCRIPT

DECLARE_SCRIPT(KP01)
END_SCRIPT

DECLARE_SCRIPT(KP02)
END_SCRIPT

DECLARE_SCRIPT(KP03)
	void sub_401E54();
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
	void dialogueWithRajif();
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
	void sub_402134();
END_SCRIPT

DECLARE_SCRIPT(NR03)
	void sub_40259C(int frame);
	void sub_402994();
END_SCRIPT

DECLARE_SCRIPT(NR04)
	void sub_401DB0();
	void sub_402860(int frame);
	void sub_402960();
END_SCRIPT

DECLARE_SCRIPT(NR05)
	void rotateActorOnTable(int frame);
	void sub_4020B4();
	void sub_4022DC();
	void rotateActorOnGround(int actorId);
	void sub_402B9C();
END_SCRIPT

DECLARE_SCRIPT(NR06)
	void sub_401BAC();
END_SCRIPT

DECLARE_SCRIPT(NR07)
	void sub_4018D4();
	void sub_401A10();
	void sub_401C60();
	void sub_401EF4();
	void sub_4020F0();
	void sub_402284();
	void sub_402510();
	void sub_402614();
	void sub_402738();
	void sub_4028FC();
END_SCRIPT

DECLARE_SCRIPT(NR08)
	void sub_4021B4();
END_SCRIPT

DECLARE_SCRIPT(NR09)
	void sub_40172C();
END_SCRIPT

DECLARE_SCRIPT(NR10)
END_SCRIPT

DECLARE_SCRIPT(NR11)
	void sub_4027D0(int actorId, signed int frame);
	void sub_4028EC();
END_SCRIPT

DECLARE_SCRIPT(PS01)
END_SCRIPT

DECLARE_SCRIPT(PS02)
	void activateElevator();
END_SCRIPT

DECLARE_SCRIPT(PS03)
END_SCRIPT

DECLARE_SCRIPT(PS04)
	void sub_4017E4();
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
	void dialogueWithGregorian();
END_SCRIPT

DECLARE_SCRIPT(PS10)
	void removeTargets();
END_SCRIPT

DECLARE_SCRIPT(PS11)
	void sub_402744();
END_SCRIPT

DECLARE_SCRIPT(PS12)
	void sub_4028C4();
END_SCRIPT

DECLARE_SCRIPT(PS13)
	void sub_40267C();
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
	void sub_402834();
END_SCRIPT

DECLARE_SCRIPT(RC04)
	void sub_401DF4();
END_SCRIPT

DECLARE_SCRIPT(RC51)
END_SCRIPT

DECLARE_SCRIPT(TB02)
	void sub_402644();
	void sub_402B50();
END_SCRIPT

DECLARE_SCRIPT(TB03)
END_SCRIPT

DECLARE_SCRIPT(TB05)
END_SCRIPT

DECLARE_SCRIPT(TB06)
END_SCRIPT

DECLARE_SCRIPT(TB07)
	void sub_401B0C();
END_SCRIPT

DECLARE_SCRIPT(UG01)
END_SCRIPT

DECLARE_SCRIPT(UG02)
	bool sub_402354();
END_SCRIPT

DECLARE_SCRIPT(UG03)
END_SCRIPT

DECLARE_SCRIPT(UG04)
END_SCRIPT

DECLARE_SCRIPT(UG05)
	int sub_4021B0();
	void sub_402218();
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
	void sub_40223C();
	void sub_4023D8();
	void sub_4025E0();
	void sub_402960();
	int sub_402AD0();
	void sub_402AD4();
	void sub_402E24();
END_SCRIPT

DECLARE_SCRIPT(UG14)
END_SCRIPT

DECLARE_SCRIPT(UG15)
END_SCRIPT

DECLARE_SCRIPT(UG16)
	void sub_401D78();
END_SCRIPT

DECLARE_SCRIPT(UG17)
END_SCRIPT

DECLARE_SCRIPT(UG18)
	void sub_402734();
	void sub_402DE8();
	void sub_402F8C();
	void sub_403114();
	void sub_403278();
	void sub_403588();
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
	SceneScript(BladeRunnerEngine *vm);
	~SceneScript();

	bool open(const Common::String &name);

	void initializeScene();
	void sceneLoaded();
	bool mouseClick(int x, int y);
	bool clickedOn3DObject(const char *objectName, bool attack);
	bool clickedOnActor(int actorId);
	bool clickedOnItem(int itemId, bool a2);
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
