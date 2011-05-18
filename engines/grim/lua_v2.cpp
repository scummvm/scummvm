/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"

#include "engines/grim/lua.h"
#include "engines/grim/lua/luadebug.h"
#include "engines/grim/lua/lauxlib.h"

#include "engines/grim/grim.h"
#include "engines/grim/actor.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/costume.h"

namespace Grim {

void L2_SetActorWalkRate() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object rateObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	if (!lua_isnumber(rateObj))
		return;

	Actor *actor = getactor(actorObj);
	float rate = lua_getnumber(rateObj);
	// const below only differ from grim
	actor->setWalkRate(rate * 3.279999971389771);
}

void L2_GetActorWalkRate() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	// const below only differ from grim
	lua_pushnumber(actor->getWalkRate() * 0.3048780560493469);
}

void L2_SetReverb() {
	lua_Object eaxObj = lua_getparam(1);
	lua_Object decayObj = lua_getparam(2);
	lua_Object mixObj = lua_getparam(3);
	lua_Object predelayObj = lua_getparam(4);
	lua_Object dampingObj = lua_getparam(5);

	if (!lua_isnumber(eaxObj))
		return;

	int eax = (int)lua_getnumber(eaxObj);
	int param = 0;
	float decay = -1;
	float mix = -1;
	float predelay = -1;
	float damping = -1;

	if (eax == 60) {
		param = 26;
	} else if (eax == 70) {
		param = 27;
	} else if (eax >= 0 && eax <= 25) {
		param = eax;
		// there is some table, initialy is like eax
	} else {
		return;
	}

	if (lua_isnumber(decayObj))
		decay = lua_getnumber(decayObj);
	if (lua_isnumber(mixObj))
		mix = lua_getnumber(mixObj);
	if (lua_isnumber(predelayObj))
		predelay = lua_getnumber(predelayObj);
	if (lua_isnumber(dampingObj))
		damping = lua_getnumber(dampingObj);

	warning("L2_SetReverb, eax: %d, decay: %f, mix: %f, predelay: %f, damping: %f", param, decay, mix, predelay, damping);
	// FIXME: func(param, decay, mix, predelay, damping);
}

void L2_LockBackground() {
	lua_Object filenameObj = lua_getparam(1);

	if (!lua_isstring(filenameObj)) {
		lua_pushnil();
		return;
	}
	const char *filename = lua_getstring(filenameObj);
	warning("L2_LockBackground, filename: %s", filename);
	// FIXME: implement missing rest part of code
}

void L2_LockChore() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object filenameObj = lua_getparam(2);

	if (!lua_isstring(nameObj) || !lua_isstring(filenameObj)) {
		lua_pushnil();
		return;
	}

	const char *name = lua_getstring(nameObj);
	const char *filename = lua_getstring(filenameObj);
	warning("L2_LockChore, name: %s, filename: %s", name, filename);
	// FIXME: implement missing rest part of code
}

void L2_SetActorSortOrder() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object modeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	if (!lua_isnumber(modeObj))
		return;

	Actor *actor = getactor(actorObj);
	int mode = (int)lua_getnumber(modeObj);
	warning("L2_SetActorSortOrder, actor: %s, mode: %d", actor->getName(), mode);
	// FIXME: actor->func(mode);
}

void L2_ActorActivateShadow() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object qualityObj = lua_getparam(2);
	lua_Object planeObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;
	int quality = (int)lua_getnumber(qualityObj);
	const char *plane = "NULL";
	if (lua_isstring(planeObj))
		plane = lua_getstring(planeObj);
	warning("L2_ActorActivateShadow, actor: %s, aquality: %d, plane: %s", actor->getName(), quality, plane);
	// FIXME: implement missing rest part of code
}

void L2_ActorStopMoving() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	warning("L2_ActorStopMoving, actor: %s", actor->getName());
	// FIXME: implement missing rest part of code
}

void L2_PutActorInOverworld() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	warning("L2_PutActorInOverworld, actor: %s", actor->getName());
	// FIXME: implement missing func
	//actor->func();
}

void L2_MakeScreenTextures() {
	lua_Object indexObj = lua_getparam(1);

	if (!lua_isnil(indexObj) && lua_isnumber(indexObj)) {
		int index = (int)lua_getnumber(indexObj);
		warning("L2_MakeScreenTextures, index: %d", index);
		// FIXME: implement missing function
//		if (func(index)) {
			lua_pushnumber(1.0);
			return;
//		}
	}
	lua_pushnil();
}

void L2_PutActorInSet() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object setObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	if (!lua_isstring(setObj) && !lua_isnil(setObj)) {
		lua_pushnil();
		return;
	}

	const char *set = lua_getstring(setObj);

	// FIXME verify adding actor to set
	if (!set) {
		actor->putInSet("");
	} else if (!actor->isInSet(set)) {
		actor->putInSet(set);
		lua_pushnumber(1.0);
		return;
	}
	lua_pushnil();
}

static void L2_LoadBundle() {
	lua_Object paramObj = lua_getparam(1);
	if (lua_isstring(paramObj) || lua_isnil(paramObj)) {
		const char *name = lua_getstring(paramObj);
		// FIXME: implement missing function
/*		if (!func(name))
			lua_pushnil();
		else*/
			lua_pushnumber(1.0);
		warning("L2_LoadBundle: stub, name: %s", name);
	}
}

void L2_AreWeInternational() {
	if (g_grim->getGameLanguage() != Common::EN_ANY)
		lua_pushnumber(1.0);
}

void L2_ImSetState() {
	lua_Object stateObj = lua_getparam(1);
	if (!lua_isnumber(stateObj))
		return;

	int state = (int)lua_getnumber(stateObj);
	warning("L2_ImSetState: stub, state: %d", state);
}

void L2_EnableVoiceFX() {
	lua_Object stateObj = lua_getparam(1);

	bool state = false;
	if (!lua_isnil(stateObj))
		state = true;

	// FIXME: func(state);
	warning("L2_EnableVoiceFX: implement opcode, state: %d", (int)state);
}

void L2_SetGroupVolume() {
	lua_Object groupObj = lua_getparam(1);
	lua_Object volumeObj = lua_getparam(2);

	if (!lua_isnumber(groupObj))
		return;
	int group = lua_getnumber(groupObj);

	int volume = 100;
	if (lua_isnumber(volumeObj))
		volume = (int)lua_getnumber(volumeObj);

	// FIXME: func(group, volume);
	warning("L2_SetGroupVolume: implement opcode, group: %d, volume %d", group, volume);
}

void L2_EnableAudioGroup() {
	lua_Object groupObj = lua_getparam(1);
	lua_Object stateObj = lua_getparam(2);

	if (!lua_isnumber(groupObj))
		return;
	int group = lua_getnumber(groupObj);

	bool state = false;
	if (!lua_isnil(stateObj))
		state = true;

	// FIXME: func(group, state);
	warning("L2_EnableAudioGroup: implement opcode, group: %d, state %d", group, (int)state);
}

void L2_ImSelectSet() {
	lua_Object qualityObj = lua_getparam(1);

	if (lua_isnumber(qualityObj)) {
		int quality = (int)lua_getnumber(qualityObj);
		// FIXME: func(quality);
		warning("L2_ImSelectSet: implement opcode, quality mode: %d", quality);
	}
}

void L2_PlayActorChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	lua_Object modeObj = lua_getparam(4);
	lua_Object paramObj = lua_getparam(5);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	if (!lua_isstring(choreObj) || !lua_isstring(costumeObj))
		lua_pushnil();

	bool mode = false;
	float param = 0.0;

	if (!lua_isnil(modeObj)) {
		if (lua_getnumber(modeObj) != 0.0)
			mode = true;
		if (!lua_isnil(paramObj))
			if (lua_isnumber(paramObj))
				param = lua_getnumber(paramObj);
	}

	const char *choreName = lua_getstring(choreObj);
	const char *costumeName = lua_getstring(costumeObj);

	warning("L2_PlayActorChore: implement opcode actor: %s, chore: %s, costume: %s, mode bool: %d, param: %f",
			actor->getName(), choreName, costumeName, (int)mode, param);
	// FIXME. code below is a hack, need proper implementation
	actor->setCostume(costumeName);
	Costume *costume = actor->getCurrentCostume();
	costume->playChore(choreName);
	pushbool(true);
}

static void L2_StopActorChores() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object paramObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	bool p = lua_isnil(paramObj) != 0;
	// I'm not fully sure about bool logic here
	//actor->func(p);
	warning("L2_StopActorChores: implement opcode... bool param: %d, actor: %s", (int)p, actor->getName());
}

static void L2_SetActorLighting() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object lightModeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (lua_isnil(lightModeObj) || !lua_isnumber(lightModeObj))
		return;

	int lightMode = (int)lua_getnumber(lightModeObj);
	if (lightMode != 0) {
		if (lightMode == 1) {
			//FIXME actor->
			warning("L2_SetActorLighting: case param 1(LIGHT_FASTDYN), actor: %s", actor->getName());
		} else if (lightMode == 2) {
			//FIXME actor->
			warning("L2_SetActorLighting: case param 2(LIGHT_NORMDYN), actor: %s", actor->getName());
		} else {
			//FIXME actor->
			warning("L2_SetActorLighting: case param %d(LIGHT_NONE), actor: %s", lightMode, actor->getName());
		}
	} else {
		//FIXME actor->
		warning("L2_SetActorLighting: case param 0(LIGHT_STATIC), actor: %s", actor->getName());
	}
}

static void L2_GetCPUSpeed() {
	lua_pushnumber(500); // anything above 333 make best configuration
}

static void L2_SetActiveCD() {
	lua_Object cdObj = lua_getparam(1);
	int cd = (int)lua_getnumber(cdObj);

	if (cd == 1 || cd == 2) {
		warning("L2_GetActiveCD: set to CD: %d", cd);
		// FIXME
		lua_pushnumber(1.0);
	}
}

static void L2_GetActiveCD() {
	// FIXME: return current CD number 1 or 2, original can also avoid push any numer
	warning("L2_GetActiveCD: return const CD 1");
	lua_pushnumber(1);
}

static void L2_PurgeText() {
	// FIXME
	warning("L2_PurgeText: implement opcode");
}

void L2_ImFlushStack() {
	// FIXME
	warning("L2_ImFlushStack: implement opcode");
}

void L2_EngineDisplay() {
	// dummy
}

void L2_SetAmbientLight() {
	// dummy
}

void L2_Display() {
	// dummy
}

// Stub function for builtin functions not yet implemented
static void stubWarning(const char *funcName) {
	warning("Stub function: %s", funcName);
}

static void stubError(const char *funcName) {
	error("Stub function: %s", funcName);
}

#define STUB_FUNC(name) void name() { stubWarning(#name); }
#define STUB_FUNC2(name) void name() { stubError(#name); }

// Opcodes more or less differ to Grim L1_* opcodes
STUB_FUNC2(L2_SetActorWalkChore)
STUB_FUNC2(L2_SetActorTurnChores)
STUB_FUNC2(L2_SetActorRestChore)
STUB_FUNC2(L2_SetActorMumblechore)
STUB_FUNC2(L2_SetActorTalkChore)
STUB_FUNC2(L2_GetActorPos)
STUB_FUNC2(L2_WalkActorVector)
STUB_FUNC2(L2_ActorLookAt)
STUB_FUNC2(L2_SetActorLookRate)
STUB_FUNC2(L2_GetActorLookRate)
STUB_FUNC2(L2_GetVisibleThings)
STUB_FUNC2(L2_SetActorHead)
STUB_FUNC2(L2_GetActorRot)
STUB_FUNC2(L2_MakeCurrentSet)
STUB_FUNC2(L2_LockSet)
STUB_FUNC2(L2_UnLockSet)
STUB_FUNC2(L2_MakeCurrentSetup)
STUB_FUNC2(L2_GetCurrentSetup)
STUB_FUNC2(L2_StartMovie)
STUB_FUNC2(L2_PlaySound)
STUB_FUNC2(L2_IsSoundPlaying)
STUB_FUNC2(L2_MakeSectorActive)
STUB_FUNC2(L2_TurnActorTo)
STUB_FUNC2(L2_GetAngleBetweenActors)
STUB_FUNC2(L2_ImStartSound)
STUB_FUNC2(L2_ImGetSfxVol)
STUB_FUNC2(L2_ImGetVoiceVol)
STUB_FUNC2(L2_ImGetMusicVol)
STUB_FUNC2(L2_ImSetSequence)
STUB_FUNC2(L2_RenderModeUser)
STUB_FUNC2(L2_DimScreen)
STUB_FUNC2(L2_SayLine)
STUB_FUNC2(L2_MakeTextObject)
STUB_FUNC2(L2_GetTextObjectDimensions)
STUB_FUNC2(L2_ChangeTextObject)
STUB_FUNC2(L2_ExpireText)
STUB_FUNC2(L2_GetColorComponents)
STUB_FUNC2(L2_GetTextCharPosition)
STUB_FUNC2(L2_SetOffscreenTextPos)
STUB_FUNC2(L2_FadeInChore)
STUB_FUNC2(L2_FadeOutChore)
STUB_FUNC2(L2_SetLightPosition)
STUB_FUNC2(L2_GetAngleBetweenVectors)
STUB_FUNC2(L2_IsPointInSector)

// Opcodes below are stubs in L1_* at that moment
STUB_FUNC2(L2_SetActorInvClipNode)
STUB_FUNC2(L2_NukeResources)
STUB_FUNC2(L2_UnShrinkBoxes)
STUB_FUNC2(L2_ShrinkBoxes)
STUB_FUNC2(L2_ResetTextures)
STUB_FUNC2(L2_AttachToResources)
STUB_FUNC2(L2_DetachFromResources)
STUB_FUNC2(L2_IrisUp)
STUB_FUNC2(L2_IrisDown)
STUB_FUNC2(L2_SetActorClipPlane)
STUB_FUNC2(L2_SetActorClipActive)
STUB_FUNC2(L2_SetActorCollisionScale)
STUB_FUNC2(L2_SetActorCollisionMode)
STUB_FUNC2(L2_FlushControls)
STUB_FUNC2(L2_ActorToClean)
STUB_FUNC2(L2_TurnLightOn)
STUB_FUNC2(L2_GetCameraLookVector)
STUB_FUNC2(L2_SetCameraRoll)
STUB_FUNC2(L2_SetCameraInterest)
STUB_FUNC2(L2_GetCameraPosition)
STUB_FUNC2(L2_SpewStartup)
STUB_FUNC2(L2_PreRender)
STUB_FUNC2(L2_GetSectorOppositeEdge)
STUB_FUNC2(L2_PreviousSetup)
STUB_FUNC2(L2_NextSetup)
STUB_FUNC2(L2_WorldToScreen)
STUB_FUNC2(L2_SetActorRoll)
STUB_FUNC2(L2_SetActorFrustrumCull)
STUB_FUNC2(L2_DriveActorTo)
STUB_FUNC2(L2_GetActorRect)
STUB_FUNC2(L2_SetActorTimeScale)
STUB_FUNC2(L2_GetTranslationMode)
STUB_FUNC2(L2_SetTranslationMode)
STUB_FUNC2(L2_KillPrimitive)
STUB_FUNC2(L2_WalkActorToAvoiding)
STUB_FUNC2(L2_GetActorChores)
STUB_FUNC2(L2_SetCameraPosition)
STUB_FUNC2(L2_GetCameraFOV)
STUB_FUNC2(L2_SetCameraFOV)
STUB_FUNC2(L2_GetCameraRoll)
STUB_FUNC2(L2_ActorPuckOrient)
STUB_FUNC2(L2_GetMemoryUsage)
STUB_FUNC2(L2_GetFontDimensions)

// Monkey specific opcodes
STUB_FUNC2(L2_ThumbnailFromFile)
STUB_FUNC2(L2_ClearSpecialtyTexture)
STUB_FUNC2(L2_UnloadActor)
STUB_FUNC2(L2_RemoveActorFromOverworld)
STUB_FUNC2(L2_ClearOverworld)
STUB_FUNC2(L2_ToggleOverworld)
STUB_FUNC2(L2_SetActorFOV)
STUB_FUNC2(L2_SetActorHeadLimits)
STUB_FUNC2(L2_EnableActorPuck)
STUB_FUNC2(L2_SetActorGlobalAlpha)
STUB_FUNC2(L2_SetActorLocalAlpha)
STUB_FUNC2(L2_GetActorSortOrder)
STUB_FUNC2(L2_AttachActor)
STUB_FUNC2(L2_DetachActor)
STUB_FUNC2(L2_IsChoreValid)
STUB_FUNC2(L2_IsChorePlaying)
STUB_FUNC2(L2_IsChoreLooping)
STUB_FUNC2(L2_PlayChore)
STUB_FUNC2(L2_StopChore)
STUB_FUNC2(L2_PauseChore)
STUB_FUNC2(L2_AdvanceChore)
STUB_FUNC2(L2_CompleteChore)
STUB_FUNC2(L2_UnlockChore)
STUB_FUNC2(L2_LockChoreSet)
STUB_FUNC2(L2_UnlockChoreSet)
STUB_FUNC2(L2_UnLockBackground)
STUB_FUNC2(L2_EscapeMovie)
STUB_FUNC2(L2_StopAllSounds)
STUB_FUNC2(L2_LoadSound)
STUB_FUNC2(L2_FreeSound)
STUB_FUNC2(L2_PlayLoadedSound)
STUB_FUNC2(L2_GetSoundVolume)
STUB_FUNC2(L2_SetSoundVolume)
STUB_FUNC2(L2_PlaySoundFrom)
STUB_FUNC2(L2_PlayLoadedSoundFrom)
STUB_FUNC2(L2_UpdateSoundPosition)
STUB_FUNC2(L2_ImStateHasLooped)
STUB_FUNC2(L2_ImStateHasEnded)
STUB_FUNC2(L2_ImPushState)
STUB_FUNC2(L2_ImPopState)
STUB_FUNC2(L2_ImGetMillisecondPosition)
STUB_FUNC2(L2_ImSetMusicVol)
STUB_FUNC2(L2_ImSetSfxVol)
STUB_FUNC2(L2_ImSetVoiceVol)
STUB_FUNC2(L2_ImSetVoiceEffect)
STUB_FUNC2(L2_GetSectorName)
STUB_FUNC2(L2_GetCameraYaw)
STUB_FUNC2(L2_YawCamera)
STUB_FUNC2(L2_GetCameraPitch)
STUB_FUNC2(L2_PitchCamera)
STUB_FUNC2(L2_RollCamera)
STUB_FUNC2(L2_UndimAll)
STUB_FUNC2(L2_UndimRegion)
STUB_FUNC2(L2_NewLayer)
STUB_FUNC2(L2_FreeLayer)
STUB_FUNC2(L2_SetLayerSortOrder)
STUB_FUNC2(L2_SetLayerFrame)
STUB_FUNC2(L2_AdvanceLayerFrame)
STUB_FUNC2(L2_PushText)
STUB_FUNC2(L2_PopText)
STUB_FUNC2(L2_NukeAllScriptLocks)
STUB_FUNC2(L2_ToggleDebugDraw)
STUB_FUNC2(L2_ToggleDrawCameras)
STUB_FUNC2(L2_ToggleDrawLights)
STUB_FUNC2(L2_ToggleDrawSectors)
STUB_FUNC2(L2_ToggleDrawBBoxes)
STUB_FUNC2(L2_ToggleDrawFPS)
STUB_FUNC2(L2_ToggleDrawPerformance)
STUB_FUNC2(L2_ToggleDrawActorStats)
STUB_FUNC2(L2_SectEditSelect)
STUB_FUNC2(L2_SectEditPlace)
STUB_FUNC2(L2_SectEditDelete)
STUB_FUNC2(L2_SectEditInsert)
STUB_FUNC2(L2_SectEditSortAdd)
STUB_FUNC2(L2_SectEditForgetIt)
STUB_FUNC2(L2_FRUTEY_Begin)
STUB_FUNC2(L2_FRUTEY_End)

struct luaL_reg monkeyMainOpcodes[] = {
	{ "EngineDisplay", L2_EngineDisplay },
	{ "Load", L1_Load },
	{ "Save", L1_Save },
	{ "remove", L1_Remove },
	{ "SetActorTimeScale", L2_SetActorTimeScale },
	{ "SetActorWalkChore", L2_SetActorWalkChore },
	{ "SetActorTurnChores", L2_SetActorTurnChores },
	{ "SetActorRestChore", L2_SetActorRestChore },
	{ "SetActorMumblechore", L2_SetActorMumblechore },
	{ "SetActorTalkChore", L2_SetActorTalkChore },
	{ "SetActorWalkRate", L2_SetActorWalkRate },
	{ "GetActorWalkRate", L2_GetActorWalkRate },
	{ "SetActorTurnRate", L1_SetActorTurnRate },
	{ "SetSelectedActor", L1_SetSelectedActor },
	{ "LoadActor", L1_LoadActor },
	{ "GetActorPos", L2_GetActorPos },
	{ "GetActorPuckVector", L1_GetActorPuckVector },
	{ "GetActorYawToPoint", L1_GetActorYawToPoint },
	{ "SetActorReflection", L1_SetActorReflection },
	{ "PutActorAt", L1_PutActorAt },
	{ "PutActorInSet", L2_PutActorInSet },
	{ "WalkActorVector", L2_WalkActorVector },
	{ "WalkActorForward", L1_WalkActorForward },
	{ "WalkActorTo", L1_WalkActorTo },
	{ "WalkActorToAvoiding", L2_WalkActorToAvoiding },
	{ "ActorLookAt", L2_ActorLookAt },
	{ "SetActorLookRate", L2_SetActorLookRate },
	{ "GetActorLookRate", L2_GetActorLookRate },
	{ "GetVisibleThings", L2_GetVisibleThings },
	{ "SetActorHead", L2_SetActorHead },
	{ "SetActorVisibility", L1_SetActorVisibility },
	{ "SetActorFollowBoxes", L1_SetActorFollowBoxes },
	{ "ShutUpActor", L1_ShutUpActor },
	{ "IsActorInSector", L1_IsActorInSector },
	{ "GetActorSector", L1_GetActorSector },
	{ "TurnActor", L1_TurnActor },
	{ "GetActorRot", L2_GetActorRot },
	{ "SetActorRot", L1_SetActorRot },
	{ "IsActorTurning", L1_IsActorTurning },
	{ "PlayActorChore", L2_PlayActorChore },
	{ "IsActorResting", L1_IsActorResting },
	{ "GetActorChores", L2_GetActorChores },
	{ "WorldToScreen", L2_WorldToScreen },
	{ "exit", L1_Exit },
	{ "FunctionName", L1_FunctionName },
	{ "EnableControl", L1_EnableControl },
	{ "DisableControl", L1_DisableControl },
	{ "GetControlState", L1_GetControlState },
	{ "PrintError", L1_PrintDebug },
	{ "PrintWarning", L1_PrintDebug },
	{ "PrintDebug", L1_PrintDebug },
	{ "MakeCurrentSet", L2_MakeCurrentSet },
	{ "LockSet", L2_LockSet },
	{ "UnLockSet", L2_UnLockSet },
	{ "MakeCurrentSetup", L2_MakeCurrentSetup },
	{ "GetCurrentSetup", L2_GetCurrentSetup },
	{ "NextSetup", L2_NextSetup },
	{ "PreviousSetup", L2_PreviousSetup },
	{ "StartMovie", L2_StartMovie },
	{ "StopMovie", L1_StopMovie },
	{ "PauseMovie", L1_PauseMovie },
	{ "IsMoviePlaying", L1_IsMoviePlaying },
	{ "PlaySound", L2_PlaySound },
	{ "IsSoundPlaying", L2_IsSoundPlaying },
	{ "FileFindFirst", L1_FileFindFirst },
	{ "FileFindNext", L1_FileFindNext },
	{ "FileFindDispose", L1_FileFindDispose },
	{ "InputDialog", L1_InputDialog },
	{ "GetSectorOppositeEdge", L2_GetSectorOppositeEdge },
	{ "MakeSectorActive", L2_MakeSectorActive },
	{ "GetCurrentScript", L1_GetCurrentScript },
	{ "GetCameraPosition", L2_GetCameraPosition },
	{ "SetCameraPosition", L2_SetCameraPosition },
	{ "GetCameraFOV", L2_GetCameraFOV },
	{ "SetCameraFOV", L2_SetCameraFOV },
	{ "GetCameraRoll", L2_GetCameraRoll },
	{ "GetCameraLookVector", L2_GetCameraLookVector },
	{ "TurnActorTo", L2_TurnActorTo },
	{ "PerSecond", L1_PerSecond },
	{ "GetAngleBetweenVectors", L2_GetAngleBetweenVectors },
	{ "GetAngleBetweenActors", L2_GetAngleBetweenActors },
	{ "SetAmbientLight", L2_SetAmbientLight },
	{ "TurnLightOn", L2_TurnLightOn },
	{ "SetLightPosition", L2_SetLightPosition },
	{ "LightMgrStartup", L1_LightMgrStartup },
	{ "ImStartSound", L2_ImStartSound },
	{ "ImGetSfxVol", L2_ImGetSfxVol },
	{ "ImGetVoiceVol", L2_ImGetVoiceVol },
	{ "ImGetMusicVol", L2_ImGetMusicVol },
	{ "ImSetState", L2_ImSetState },
	{ "ImSetSequence", L2_ImSetSequence },
	{ "ImSetMusicVol", L2_ImSetMusicVol },
	{ "ImSetSfxVol", L2_ImSetSfxVol },
	{ "ImSetVoiceVol", L2_ImSetVoiceVol },
	{ "ImSetVoiceEffect", L2_ImSetVoiceEffect },
	{ "LoadBundle", L2_LoadBundle },
	{ "SetGamma", L1_SetGamma },
	{ "SetActorWalkDominate", L1_SetActorWalkDominate },
	{ "RenderModeUser", L2_RenderModeUser },
	{ "DimScreen", L2_DimScreen },
	{ "Display", L2_Display },
	{ "SetSpeechMode", L1_SetSpeechMode },
	{ "GetSpeechMode", L1_GetSpeechMode },
	{ "SetActorCollisionMode", L2_SetActorCollisionMode },
	{ "SetActorCollisionScale", L2_SetActorCollisionScale },
	{ "SubmitSaveGameData", L1_SubmitSaveGameData },
	{ "GetSaveGameData", L1_GetSaveGameData },
	{ "SetTextSpeed", L1_SetTextSpeed },
	{ "GetTextSpeed", L1_GetTextSpeed },
	{ "JustLoaded", L1_JustLoaded },
	{ "UnShrinkBoxes", L2_UnShrinkBoxes },
	{ "GetMemoryUsage", L2_GetMemoryUsage },
	{ "dofile", L1_new_dofile },

	// Monkey specific opcodes:

	{ "SetActiveCD", L2_SetActiveCD },
	{ "GetActiveCD", L2_GetActiveCD },
	{ "AreWeInternational", L2_AreWeInternational },
	{ "MakeScreenTextures", L2_MakeScreenTextures },
	{ "ThumbnailFromFile", L2_ThumbnailFromFile },
	{ "ClearSpecialtyTexture", L2_ClearSpecialtyTexture },
	{ "UnloadActor", L2_UnloadActor },
	{ "PutActorInOverworld", L2_PutActorInOverworld },
	{ "RemoveActorFromOverworld", L2_RemoveActorFromOverworld },
	{ "ClearOverworld", L2_ClearOverworld },
	{ "ToggleOverworld", L2_ToggleOverworld },
	{ "ActorStopMoving", L2_ActorStopMoving },
	{ "SetActorFOV", L2_SetActorFOV },
	{ "SetActorLighting", L2_SetActorLighting },
	{ "SetActorHeadLimits", L2_SetActorHeadLimits },
	{ "ActorActivateShadow", L2_ActorActivateShadow },
	{ "EnableActorPuck", L2_EnableActorPuck },
	{ "SetActorGlobalAlpha", L2_SetActorGlobalAlpha },
	{ "SetActorLocalAlpha", L2_SetActorLocalAlpha },
	{ "SetActorSortOrder", L2_SetActorSortOrder },
	{ "GetActorSortOrder", L2_GetActorSortOrder },
	{ "AttachActor", L2_AttachActor },
	{ "DetachActor", L2_DetachActor },
	{ "IsChoreValid", L2_IsChoreValid },
	{ "IsChorePlaying", L2_IsChorePlaying },
	{ "IsChoreLooping", L2_IsChoreLooping },
	{ "StopActorChores", L2_StopActorChores },
	{ "PlayChore", L2_PlayChore },
	{ "StopChore", L2_StopChore },
	{ "PauseChore", L2_PauseChore },
	{ "AdvanceChore", L2_AdvanceChore },
	{ "CompleteChore", L2_CompleteChore },
	{ "LockChore", L2_LockChore },
	{ "UnlockChore", L2_UnlockChore },
	{ "LockChoreSet", L2_LockChoreSet },
	{ "UnlockChoreSet", L2_UnlockChoreSet },
	{ "LockBackground", L2_LockBackground },
	{ "UnLockBackground", L2_UnLockBackground },
	{ "EscapeMovie", L2_EscapeMovie },
	{ "StopAllSounds", L2_StopAllSounds },
	{ "LoadSound", L2_LoadSound },
	{ "FreeSound", L2_FreeSound },
	{ "PlayLoadedSound", L2_PlayLoadedSound },
	{ "SetGroupVolume", L2_SetGroupVolume },
	{ "GetSoundVolume", L2_GetSoundVolume },
	{ "SetSoundVolume", L2_SetSoundVolume },
	{ "EnableAudioGroup", L2_EnableAudioGroup },
	{ "EnableVoiceFX", L2_EnableVoiceFX },
	{ "PlaySoundFrom", L2_PlaySoundFrom },
	{ "PlayLoadedSoundFrom", L2_PlayLoadedSoundFrom },
	{ "SetReverb", L2_SetReverb },
	{ "UpdateSoundPosition", L2_UpdateSoundPosition },
	{ "ImSelectSet", L2_ImSelectSet },
	{ "ImStateHasLooped", L2_ImStateHasLooped },
	{ "ImStateHasEnded", L2_ImStateHasEnded },
	{ "ImPushState", L2_ImPushState },
	{ "ImPopState", L2_ImPopState },
	{ "ImFlushStack", L2_ImFlushStack },
	{ "ImGetMillisecondPosition", L2_ImGetMillisecondPosition },
	{ "GetSectorName", L2_GetSectorName },
	{ "GetCameraYaw", L2_GetCameraYaw },
	{ "YawCamera", L2_YawCamera },
	{ "GetCameraPitch", L2_GetCameraPitch },
	{ "PitchCamera", L2_PitchCamera },
	{ "RollCamera", L2_RollCamera },
	{ "UndimAll", L2_UndimAll },
	{ "UndimRegion", L2_UndimRegion },
	{ "GetCPUSpeed", L2_GetCPUSpeed },
	{ "NewLayer", L2_NewLayer },
	{ "FreeLayer", L2_FreeLayer },
	{ "SetLayerSortOrder", L2_SetLayerSortOrder },
	{ "SetLayerFrame", L2_SetLayerFrame },
	{ "AdvanceLayerFrame", L2_AdvanceLayerFrame },
	{ "PushText", L2_PushText },
	{ "PopText", L2_PopText },
	{ "NukeAllScriptLocks", L2_NukeAllScriptLocks },
	{ "ToggleDebugDraw", L2_ToggleDebugDraw },
	{ "ToggleDrawCameras", L2_ToggleDrawCameras },
	{ "ToggleDrawLights", L2_ToggleDrawLights },
	{ "ToggleDrawSectors", L2_ToggleDrawSectors },
	{ "ToggleDrawBBoxes", L2_ToggleDrawBBoxes },
	{ "ToggleDrawFPS", L2_ToggleDrawFPS },
	{ "ToggleDrawPerformance", L2_ToggleDrawPerformance },
	{ "ToggleDrawActorStats", L2_ToggleDrawActorStats },
	{ "SectEditSelect", L2_SectEditSelect },
	{ "SectEditPlace", L2_SectEditPlace },
	{ "SectEditDelete", L2_SectEditDelete },
	{ "SectEditInsert", L2_SectEditInsert },
	{ "SectEditSortAdd", L2_SectEditSortAdd },
	{ "SectEditForgetIt", L2_SectEditForgetIt },
	{ "FRUTEY_Begin", L2_FRUTEY_Begin },
	{ "FRUTEY_End", L2_FRUTEY_End }
};

struct luaL_reg monkeyTextOpcodes[] = {
	{ "IsMessageGoing", L1_IsMessageGoing },
	{ "SetSayLineDefaults", L1_SetSayLineDefaults },
	{ "SetActorTalkColor", L1_SetActorTalkColor },
	{ "SayLine", L2_SayLine },
	{ "MakeTextObject", L2_MakeTextObject },
	{ "GetTextObjectDimensions", L2_GetTextObjectDimensions },
	{ "GetFontDimensions", L2_GetFontDimensions },
	{ "ChangeTextObject", L2_ChangeTextObject },
	{ "KillTextObject", L1_KillTextObject },
	{ "ExpireText", L2_ExpireText },
	{ "PurgeText", L2_PurgeText },
	{ "MakeColor", L1_MakeColor },
	{ "GetColorComponents", L2_GetColorComponents },
	{ "GetTextCharPosition", L2_GetTextCharPosition },
	{ "LocalizeString", L1_LocalizeString },
	{ "SetOffscreenTextPos", L2_SetOffscreenTextPos }
};

struct luaL_reg monkeyPrimitivesOpcodes[] = {
	{ "DrawLine", L1_DrawLine },
	{ "ChangePrimitive", L1_ChangePrimitive },
	{ "KillPrimitive", L2_KillPrimitive },
	{ "PurgePrimitiveQueue", L1_PurgePrimitiveQueue }
};

struct luaL_reg monkeyMiscOpcodes[] = {
	{ "  concatfallback", L1_concatFallback },
	{ "  typeoverride", L1_typeOverride },
	{ "  dfltcamera", dummyHandler },
	{ "  dfltcontrol", dummyHandler },
};

void registerMonkeyOpcodes() {
	// Register main opcodes functions
	luaL_openlib(monkeyMainOpcodes, ARRAYSIZE(monkeyMainOpcodes));

	// Register text opcodes functions
	luaL_openlib(monkeyTextOpcodes, ARRAYSIZE(monkeyTextOpcodes));

	// Register primitives opcodes functions
	luaL_openlib(monkeyPrimitivesOpcodes, ARRAYSIZE(monkeyPrimitivesOpcodes));

	// Register miscOpcodes opcodes functions
	luaL_openlib(monkeyMiscOpcodes, ARRAYSIZE(monkeyMiscOpcodes));
}

} // end of namespace Grim
