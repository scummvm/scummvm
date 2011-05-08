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

namespace Grim {

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

	int lightMode = lua_getnumber(lightModeObj);
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

static void L2_GetActiveCD() {
	// return current CD number 1 or 2, original can also avoid push any numer
	warning("L2_GetActiveCD: return const CD 1");
	lua_pushnumber(1);
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

STUB_FUNC2(L2_EngineDisplay)
STUB_FUNC2(L2_Load)
STUB_FUNC2(L2_Save)
STUB_FUNC2(L2_Remove)
STUB_FUNC2(L2_SetActorWalkChore)
STUB_FUNC2(L2_SetActorTurnChores)
STUB_FUNC2(L2_SetActorRestChore)
STUB_FUNC2(L2_SetActorMumblechore)
STUB_FUNC2(L2_SetActorTalkChore)
STUB_FUNC2(L2_SetActorWalkRate)
STUB_FUNC2(L2_GetActorWalkRate)
STUB_FUNC2(L2_SetActorTurnRate)
STUB_FUNC2(L2_SetSelectedActor)
STUB_FUNC2(L2_GetActorPos)
STUB_FUNC2(L2_GetActorPuckVector)
STUB_FUNC2(L2_GetActorYawToPoint)
STUB_FUNC2(L2_SetActorReflection)
STUB_FUNC2(L2_PutActorInSet)
STUB_FUNC2(L2_WalkActorVector)
STUB_FUNC2(L2_WalkActorForward)
STUB_FUNC2(L2_WalkActorTo)
STUB_FUNC2(L2_ActorLookAt)
STUB_FUNC2(L2_SetActorLookRate)
STUB_FUNC2(L2_GetActorLookRate)
STUB_FUNC2(L2_GetVisibleThings)
STUB_FUNC2(L2_SetActorHead)
STUB_FUNC2(L2_SetActorVisibility)
STUB_FUNC2(L2_SetActorFollowBoxes)
STUB_FUNC2(L2_ShutUpActor)
STUB_FUNC2(L2_IsActorInSector)
STUB_FUNC2(L2_GetActorSector)
STUB_FUNC2(L2_TurnActor)
STUB_FUNC2(L2_GetActorRot)
STUB_FUNC2(L2_SetActorRot)
STUB_FUNC2(L2_IsActorTurning)
STUB_FUNC2(L2_PlayActorChore)
STUB_FUNC2(L2_StopActorChore)
STUB_FUNC2(L2_IsActorResting)
STUB_FUNC2(L2_Exit)
STUB_FUNC2(L2_FunctionName)
STUB_FUNC2(L2_GetControlState)
STUB_FUNC2(L2_MakeCurrentSet)
STUB_FUNC2(L2_LockSet)
STUB_FUNC2(L2_UnLockSet)
STUB_FUNC2(L2_MakeCurrentSetup)
STUB_FUNC2(L2_GetCurrentSetup)
STUB_FUNC2(L2_StartFullscreenMovie)
STUB_FUNC2(L2_IsFullscreenMoviePlaying)
STUB_FUNC2(L2_StartMovie)
STUB_FUNC2(L2_StopMovie)
STUB_FUNC2(L2_PauseMovie)
STUB_FUNC2(L2_IsMoviePlaying)
STUB_FUNC2(L2_PlaySound)
STUB_FUNC2(L2_IsSoundPlaying)
STUB_FUNC2(L2_SetSoundPosition)
STUB_FUNC2(L2_FileFindFirst)
STUB_FUNC2(L2_FileFindNext)
STUB_FUNC2(L2_FileFindDispose)
STUB_FUNC2(L2_InputDialog)
STUB_FUNC2(L2_MakeSectorActive)
STUB_FUNC2(L2_GetCurrentScript)
STUB_FUNC2(L2_TurnActorTo)
STUB_FUNC2(L2_PerSecond)
STUB_FUNC2(L2_GetAngleBetweenActors)
STUB_FUNC2(L2_SetAmbientLight)
STUB_FUNC2(L2_ImStartSound)
STUB_FUNC2(L2_ImGetSfxVol)
STUB_FUNC2(L2_ImGetVoiceVol)
STUB_FUNC2(L2_ImGetMusicVol)
STUB_FUNC2(L2_ImSetState)
STUB_FUNC2(L2_ImSetSequence)
STUB_FUNC2(L2_LoadBundle)
STUB_FUNC2(L2_SetGamma)
STUB_FUNC2(L2_SetActorWalkDominate)
STUB_FUNC2(L2_RenderModeUser)
STUB_FUNC2(L2_DimScreen)
STUB_FUNC2(L2_Display)
STUB_FUNC2(L2_SetSpeechMode)
STUB_FUNC2(L2_GetSpeechMode)
STUB_FUNC2(L2_KillActorShadows)
STUB_FUNC2(L2_NewObjectState)
STUB_FUNC2(L2_SubmitSaveGameData)
STUB_FUNC2(L2_GetSaveGameData)
STUB_FUNC2(L2_SetTextSpeed)
STUB_FUNC2(L2_GetTextSpeed)
STUB_FUNC2(L2_JustLoaded)
STUB_FUNC2(L2_IsMessageGoing)
STUB_FUNC2(L2_SetSayLineDefaults)
STUB_FUNC2(L2_SetActorTalkColor)
STUB_FUNC2(L2_SayLine)
STUB_FUNC2(L2_MakeTextObject)
STUB_FUNC2(L2_GetTextObjectDimensions)
STUB_FUNC2(L2_ChangeTextObject)
STUB_FUNC2(L2_KillTextObject)
STUB_FUNC2(L2_ExpireText)
STUB_FUNC2(L2_GetColorComponents)
STUB_FUNC2(L2_GetTextCharPosition)
STUB_FUNC2(L2_SetOffscreenTextPos)
STUB_FUNC2(L2_DrawLine)
STUB_FUNC2(L2_ChangePrimitive)
STUB_FUNC2(L2_PurgePrimitiveQueue)
STUB_FUNC2(L2_concatFallback)
STUB_FUNC2(L2_typeOverride)


STUB_FUNC2(L2_SetActorInvClipNode)
STUB_FUNC2(L2_NukeResources)
STUB_FUNC2(L2_UnShrinkBoxes)
STUB_FUNC2(L2_ShrinkBoxes)
STUB_FUNC2(L2_ResetTextures)
STUB_FUNC2(L2_AttachToResources)
STUB_FUNC2(L2_DetachFromResources)
STUB_FUNC2(L2_IrisUp)
STUB_FUNC2(L2_IrisDown)
STUB_FUNC2(L2_FadeInChore)
STUB_FUNC2(L2_FadeOutChore)
STUB_FUNC2(L2_SetActorClipPlane)
STUB_FUNC2(L2_SetActorClipActive)
STUB_FUNC2(L2_SetActorCollisionScale)
STUB_FUNC2(L2_SetActorCollisionMode)
STUB_FUNC2(L2_FlushControls)
STUB_FUNC2(L2_ActorToClean)
STUB_FUNC2(L2_LightMgrStartup)
STUB_FUNC2(L2_SetLightIntensity)
STUB_FUNC2(L2_SetLightPosition)
STUB_FUNC2(L2_TurnLightOn)
STUB_FUNC2(L2_GetAngleBetweenVectors)
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
STUB_FUNC2(L2_IsPointInSector)
STUB_FUNC2(L2_SetActorFrustrumCull)
STUB_FUNC2(L2_DriveActorTo)
STUB_FUNC2(L2_GetActorRect)
STUB_FUNC2(L2_SetActorTimeScale)
STUB_FUNC2(L2_SetActorScale)
STUB_FUNC2(L2_GetTranslationMode)
STUB_FUNC2(L2_SetTranslationMode)
STUB_FUNC2(L2_PrintLine)
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
STUB_FUNC2(L2_PurgeText)

// Monkey specific opcodes:

STUB_FUNC2(L2_SetActiveCD)
STUB_FUNC2(L2_AreWeInternational)
STUB_FUNC2(L2_MakeScreenTextures)
STUB_FUNC2(L2_ThumbnailFromFile)
STUB_FUNC2(L2_ClearSpecialtyTexture)
STUB_FUNC2(L2_UnloadActor)
STUB_FUNC2(L2_PutActorInOverworld)
STUB_FUNC2(L2_RemoveActorFromOverworld)
STUB_FUNC2(L2_ClearOverworld)
STUB_FUNC2(L2_ToggleOverworld)
STUB_FUNC2(L2_ActorStopMoving)
STUB_FUNC2(L2_SetActorFOV)
STUB_FUNC2(L2_SetActorHeadLimits)
STUB_FUNC2(L2_ActorActivateShadow)
STUB_FUNC2(L2_EnableActorPuck)
STUB_FUNC2(L2_SetActorGlobalAlpha)
STUB_FUNC2(L2_SetActorLocalAlpha)
STUB_FUNC2(L2_SetActorSortOrder)
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
STUB_FUNC2(L2_LockChore)
STUB_FUNC2(L2_UnlockChore)
STUB_FUNC2(L2_LockChoreSet)
STUB_FUNC2(L2_UnlockChoreSet)
STUB_FUNC2(L2_LockBackground)
STUB_FUNC2(L2_UnLockBackground)
STUB_FUNC2(L2_EscapeMovie)
STUB_FUNC2(L2_StopAllSounds)
STUB_FUNC2(L2_LoadSound)
STUB_FUNC2(L2_FreeSound)
STUB_FUNC2(L2_PlayLoadedSound)
STUB_FUNC2(L2_SetGroupVolume)
STUB_FUNC2(L2_GetSoundVolume)
STUB_FUNC2(L2_SetSoundVolume)
STUB_FUNC2(L2_EnableAudioGroup)
STUB_FUNC2(L2_EnableVoiceFX)
STUB_FUNC2(L2_PlaySoundFrom)
STUB_FUNC2(L2_PlayLoadedSoundFrom)
STUB_FUNC2(L2_SetReverb)
STUB_FUNC2(L2_UpdateSoundPosition)
STUB_FUNC2(L2_ImSelectSet)
STUB_FUNC2(L2_ImStateHasLooped)
STUB_FUNC2(L2_ImStateHasEnded)
STUB_FUNC2(L2_ImPushState)
STUB_FUNC2(L2_ImPopState)
STUB_FUNC2(L2_ImFlushStack)
STUB_FUNC2(L2_ImGetMillisecondPosition)
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
	{ "Load", L2_Load },
	{ "Save", L2_Save },
	{ "remove", L2_Remove },
	{ "SetActorTimeScale", L2_SetActorTimeScale },
	{ "SetActorWalkChore", L2_SetActorWalkChore },
	{ "SetActorTurnChores", L2_SetActorTurnChores },
	{ "SetActorRestChore", L2_SetActorRestChore },
	{ "SetActorMumblechore", L2_SetActorMumblechore },
	{ "SetActorTalkChore", L2_SetActorTalkChore },
	{ "SetActorWalkRate", L2_SetActorWalkRate },
	{ "GetActorWalkRate", L2_GetActorWalkRate },
	{ "SetActorTurnRate", L2_SetActorTurnRate },
	{ "SetSelectedActor", L2_SetSelectedActor },
	{ "LoadActor", L1_LoadActor },
	{ "GetActorPos", L2_GetActorPos },
	{ "GetActorPuckVector", L2_GetActorPuckVector },
	{ "GetActorYawToPoint", L2_GetActorYawToPoint },
	{ "SetActorReflection", L2_SetActorReflection },
	{ "PutActorAt", L1_PutActorAt },
	{ "PutActorInSet", L2_PutActorInSet },
	{ "WalkActorVector", L2_WalkActorVector },
	{ "WalkActorForward", L2_WalkActorForward },
	{ "WalkActorTo", L2_WalkActorTo },
	{ "WalkActorToAvoiding", L2_WalkActorToAvoiding },
	{ "ActorLookAt", L2_ActorLookAt },
	{ "SetActorLookRate", L2_SetActorLookRate },
	{ "GetActorLookRate", L2_GetActorLookRate },
	{ "GetVisibleThings", L2_GetVisibleThings },
	{ "SetActorHead", L2_SetActorHead },
	{ "SetActorVisibility", L2_SetActorVisibility },
	{ "SetActorFollowBoxes", L2_SetActorFollowBoxes },
	{ "ShutUpActor", L2_ShutUpActor },
	{ "IsActorInSector", L2_IsActorInSector },
	{ "GetActorSector", L2_GetActorSector },
	{ "TurnActor", L2_TurnActor },
	{ "GetActorRot", L2_GetActorRot },
	{ "SetActorRot", L2_SetActorRot },
	{ "IsActorTurning", L2_IsActorTurning },
	{ "PlayActorChore", L2_PlayActorChore },
	{ "StopActorChore", L2_StopActorChore },
	{ "IsActorResting", L2_IsActorResting },
	{ "GetActorChores", L2_GetActorChores },
	{ "WorldToScreen", L2_WorldToScreen },
	{ "exit", L2_Exit },
	{ "FunctionName", L2_FunctionName },
	{ "EnableControl", L1_EnableControl },
	{ "DisableControl", L1_DisableControl },
	{ "GetControlState", L2_GetControlState },
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
	{ "StartFullscreenMovie", L2_StartFullscreenMovie },
	{ "IsFullscreenMoviePlaying", L2_IsFullscreenMoviePlaying },
	{ "StartMovie", L2_StartMovie },
	{ "StopMovie", L2_StopMovie },
	{ "PauseMovie", L2_PauseMovie },
	{ "IsMoviePlaying", L2_IsMoviePlaying },
	{ "PlaySound", L2_PlaySound },
	{ "IsSoundPlaying", L2_IsSoundPlaying },
	{ "SetSoundPosition", L2_SetSoundPosition },
	{ "FileFindFirst", L2_FileFindFirst },
	{ "FileFindNext", L2_FileFindNext },
	{ "FileFindDispose", L2_FileFindDispose },
	{ "InputDialog", L2_InputDialog },
	{ "GetSectorOppositeEdge", L2_GetSectorOppositeEdge },
	{ "MakeSectorActive", L2_MakeSectorActive },
	{ "GetCurrentScript", L2_GetCurrentScript },
	{ "GetCameraPosition", L2_GetCameraPosition },
	{ "SetCameraPosition", L2_SetCameraPosition },
	{ "GetCameraFOV", L2_GetCameraFOV },
	{ "SetCameraFOV", L2_SetCameraFOV },
	{ "GetCameraRoll", L2_GetCameraRoll },
	{ "GetCameraLookVector", L2_GetCameraLookVector },
	{ "TurnActorTo", L2_TurnActorTo },
	{ "PerSecond", L2_PerSecond },
	{ "GetAngleBetweenVectors", L2_GetAngleBetweenVectors },
	{ "GetAngleBetweenActors", L2_GetAngleBetweenActors },
	{ "SetAmbientLight", L2_SetAmbientLight },
	{ "TurnLightOn", L2_TurnLightOn },
	{ "SetLightPosition", L2_SetLightPosition },
	{ "LightMgrStartup", L2_LightMgrStartup },
	{ "ImStartSound", L2_ImStartSound },
	{ "ImGetSfxVol", L2_ImGetSfxVol },
	{ "ImGetVoiceVol", L2_ImGetVoiceVol },
	{ "ImGetMusicVol", L2_ImGetMusicVol },
	{ "ImSetState", L2_ImSetState },
	{ "ImSetSequence", L2_ImSetSequence },
	{ "LoadBundle", L2_LoadBundle },
	{ "SetGamma", L2_SetGamma },
	{ "SetActorWalkDominate", L2_SetActorWalkDominate },
	{ "RenderModeUser", L2_RenderModeUser },
	{ "DimScreen", L2_DimScreen },
	{ "Display", L2_Display },
	{ "SetSpeechMode", L2_SetSpeechMode },
	{ "GetSpeechMode", L2_GetSpeechMode },
	{ "KillActorShadows", L2_KillActorShadows },
	{ "NewObjectState", L2_NewObjectState },
	{ "SetActorCollisionMode", L2_SetActorCollisionMode },
	{ "SetActorCollisionScale", L2_SetActorCollisionScale },
	{ "SubmitSaveGameData", L2_SubmitSaveGameData },
	{ "GetSaveGameData", L2_GetSaveGameData },
	{ "SetTextSpeed", L2_SetTextSpeed },
	{ "GetTextSpeed", L2_GetTextSpeed },
	{ "JustLoaded", L2_JustLoaded },
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
	{ "IsMessageGoing", L2_IsMessageGoing },
	{ "SetSayLineDefaults", L2_SetSayLineDefaults },
	{ "SetActorTalkColor", L2_SetActorTalkColor },
	{ "SayLine", L2_SayLine },
	{ "MakeTextObject", L2_MakeTextObject },
	{ "GetTextObjectDimensions", L2_GetTextObjectDimensions },
	{ "GetFontDimensions", L2_GetFontDimensions },
	{ "ChangeTextObject", L2_ChangeTextObject },
	{ "KillTextObject", L2_KillTextObject },
	{ "ExpireText", L2_ExpireText },
	{ "PurgeText", L2_PurgeText },
	{ "MakeColor", L1_MakeColor },
	{ "GetColorComponents", L2_GetColorComponents },
	{ "GetTextCharPosition", L2_GetTextCharPosition },
	{ "LocalizeString", L1_LocalizeString },
	{ "SetOffscreenTextPos", L2_SetOffscreenTextPos }
};

struct luaL_reg monkeyPrimitivesOpcodes[] = {
	{ "DrawLine", L2_DrawLine },
	{ "ChangePrimitive", L2_ChangePrimitive },
	{ "KillPrimitive", L2_KillPrimitive },
	{ "PurgePrimitiveQueue", L2_PurgePrimitiveQueue }
};

struct luaL_reg monkeyMiscOpcodes[] = {
	{ "  concatfallback", L2_concatFallback },
	{ "  typeoverride", L2_typeOverride },
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
