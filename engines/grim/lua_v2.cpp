/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

namespace Grim {


// Stub function for builtin functions not yet implemented
static void stubWarning(const char *funcName) {
	warning("Stub function: %s", funcName);
}

#define STUB_FUNC(name) static void name() { stubWarning(#name); }
STUB_FUNC(SetActorInvClipNode)
STUB_FUNC(NukeResources)
STUB_FUNC(UnShrinkBoxes)
STUB_FUNC(ShrinkBoxes)
STUB_FUNC(ResetTextures)
STUB_FUNC(AttachToResources)
STUB_FUNC(DetachFromResources)
STUB_FUNC(IrisUp)
STUB_FUNC(IrisDown)
STUB_FUNC(FadeInChore)
STUB_FUNC(FadeOutChore)
STUB_FUNC(SetActorClipPlane)
STUB_FUNC(SetActorClipActive)
STUB_FUNC(SetActorCollisionScale)
STUB_FUNC(SetActorCollisionMode)
STUB_FUNC(FlushControls)
STUB_FUNC(ActorToClean)
STUB_FUNC(LightMgrStartup)
STUB_FUNC(SetLightIntensity)
STUB_FUNC(SetLightPosition)
STUB_FUNC(TurnLightOn)
STUB_FUNC(GetAngleBetweenVectors)
STUB_FUNC(GetCameraLookVector)
STUB_FUNC(SetCameraRoll)
STUB_FUNC(SetCameraInterest)
STUB_FUNC(GetCameraPosition)
STUB_FUNC(SpewStartup)
STUB_FUNC(PreRender)
STUB_FUNC(GetSectorOppositeEdge)
STUB_FUNC(PreviousSetup)
STUB_FUNC(NextSetup)
STUB_FUNC(WorldToScreen)
STUB_FUNC(SetActorRoll)
STUB_FUNC(IsPointInSector)
STUB_FUNC(SetActorFrustrumCull)
STUB_FUNC(DriveActorTo)
STUB_FUNC(GetActorRect)
STUB_FUNC(SetActorTimeScale)
STUB_FUNC(SetActorScale)
STUB_FUNC(GetTranslationMode)
STUB_FUNC(SetTranslationMode)
STUB_FUNC(PrintLine)
STUB_FUNC(KillPrimitive)
STUB_FUNC(WalkActorToAvoiding)
STUB_FUNC(GetActorChores)
STUB_FUNC(SetCameraPosition)
STUB_FUNC(GetCameraFOV)
STUB_FUNC(SetCameraFOV)
STUB_FUNC(GetCameraRoll)
STUB_FUNC(ActorPuckOrient)
STUB_FUNC(GetMemoryUsage)
STUB_FUNC(GetFontDimensions)
STUB_FUNC(PurgeText)

// Monkey specific opcodes:

STUB_FUNC(SetActiveCD)
STUB_FUNC(GetActiveCD)
STUB_FUNC(AreWeInternational)
STUB_FUNC(MakeScreenTextures)
STUB_FUNC(ThumbnailFromFile)
STUB_FUNC(ClearSpecialtyTexture)
STUB_FUNC(UnloadActor)
STUB_FUNC(PutActorInOverworld)
STUB_FUNC(RemoveActorFromOverworld)
STUB_FUNC(ClearOverworld)
STUB_FUNC(ToggleOverworld)
STUB_FUNC(ActorStopMoving)
STUB_FUNC(SetActorFOV)
STUB_FUNC(SetActorLighting)
STUB_FUNC(SetActorHeadLimits)
STUB_FUNC(ActorActivateShadow)
STUB_FUNC(EnableActorPuck)
STUB_FUNC(SetActorGlobalAlpha)
STUB_FUNC(SetActorLocalAlpha)
STUB_FUNC(SetActorSortOrder)
STUB_FUNC(GetActorSortOrder)
STUB_FUNC(AttachActor)
STUB_FUNC(DetachActor)
STUB_FUNC(IsChoreValid)
STUB_FUNC(IsChorePlaying)
STUB_FUNC(IsChoreLooping)
STUB_FUNC(StopActorChores)
STUB_FUNC(PlayChore)
STUB_FUNC(StopChore)
STUB_FUNC(PauseChore)
STUB_FUNC(AdvanceChore)
STUB_FUNC(CompleteChore)
STUB_FUNC(LockChore)
STUB_FUNC(UnlockChore)
STUB_FUNC(LockChoreSet)
STUB_FUNC(UnlockChoreSet)
STUB_FUNC(LockBackground)
STUB_FUNC(UnLockBackground)
STUB_FUNC(EscapeMovie)
STUB_FUNC(StopAllSounds)
STUB_FUNC(LoadSound)
STUB_FUNC(FreeSound)
STUB_FUNC(PlayLoadedSound)
STUB_FUNC(SetGroupVolume)
STUB_FUNC(GetSoundVolume)
STUB_FUNC(SetSoundVolume)
STUB_FUNC(EnableAudioGroup)
STUB_FUNC(EnableVoiceFX)
STUB_FUNC(PlaySoundFrom)
STUB_FUNC(PlayLoadedSoundFrom)
STUB_FUNC(SetReverb)
STUB_FUNC(UpdateSoundPosition)
STUB_FUNC(ImSelectSet)
STUB_FUNC(ImStateHasLooped)
STUB_FUNC(ImStateHasEnded)
STUB_FUNC(ImPushState)
STUB_FUNC(ImPopState)
STUB_FUNC(ImFlushStack)
STUB_FUNC(ImGetMillisecondPosition)
STUB_FUNC(GetSectorName)
STUB_FUNC(GetCameraYaw)
STUB_FUNC(YawCamera)
STUB_FUNC(GetCameraPitch)
STUB_FUNC(PitchCamera)
STUB_FUNC(RollCamera)
STUB_FUNC(UndimAll)
STUB_FUNC(UndimRegion)
STUB_FUNC(GetCPUSpeed)
STUB_FUNC(NewLayer)
STUB_FUNC(FreeLayer)
STUB_FUNC(SetLayerSortOrder)
STUB_FUNC(SetLayerFrame)
STUB_FUNC(AdvanceLayerFrame)
STUB_FUNC(PushText)
STUB_FUNC(PopText)
STUB_FUNC(NukeAllScriptLocks)
STUB_FUNC(ToggleDebugDraw)
STUB_FUNC(ToggleDrawCameras)
STUB_FUNC(ToggleDrawLights)
STUB_FUNC(ToggleDrawSectors)
STUB_FUNC(ToggleDrawBBoxes)
STUB_FUNC(ToggleDrawFPS)
STUB_FUNC(ToggleDrawPerformance)
STUB_FUNC(ToggleDrawActorStats)
STUB_FUNC(SectEditSelect)
STUB_FUNC(SectEditPlace)
STUB_FUNC(SectEditDelete)
STUB_FUNC(SectEditInsert)
STUB_FUNC(SectEditSortAdd)
STUB_FUNC(SectEditForgetIt)
STUB_FUNC(FRUTEY_Begin)
STUB_FUNC(FRUTEY_End)

static void dummyHandler() {
}

struct luaL_reg mainOpcodes[] = {
	{ "EngineDisplay", EngineDisplay },
	{ "Load", Load },
	{ "Save", Save },
	{ "remove", lua_remove },
	{ "SetActorTimeScale", SetActorTimeScale },
	{ "SetActorWalkChore", SetActorWalkChore },
	{ "SetActorTurnChores", SetActorTurnChores },
	{ "SetActorRestChore", SetActorRestChore },
	{ "SetActorMumblechore", SetActorMumblechore },
	{ "SetActorTalkChore", SetActorTalkChore },
	{ "SetActorWalkRate", SetActorWalkRate },
	{ "GetActorWalkRate", GetActorWalkRate },
	{ "SetActorTurnRate", SetActorTurnRate },
	{ "SetSelectedActor", SetSelectedActor },
	{ "LoadActor", LoadActor },
	{ "GetActorPos", GetActorPos },
	{ "GetActorPuckVector", GetActorPuckVector },
	{ "GetActorYawToPoint", GetActorYawToPoint },
	{ "SetActorReflection", SetActorReflection },
	{ "PutActorAt", PutActorAt },
	{ "PutActorInSet", PutActorInSet },
	{ "WalkActorVector", WalkActorVector },
	{ "WalkActorForward", WalkActorForward },
	{ "WalkActorTo", WalkActorTo },
	{ "WalkActorToAvoiding", WalkActorToAvoiding },
	{ "ActorLookAt", ActorLookAt },
	{ "SetActorLookRate", SetActorLookRate },
	{ "GetActorLookRate", GetActorLookRate },
	{ "GetVisibleThings", GetVisibleThings },
	{ "SetActorHead", SetActorHead },
	{ "SetActorVisibility", SetActorVisibility },
	{ "SetActorFollowBoxes", SetActorFollowBoxes },
	{ "ShutUpActor", ShutUpActor },
	{ "IsActorInSector", IsActorInSector },
	{ "GetActorSector", GetActorSector },
	{ "TurnActor", TurnActor },
	{ "GetActorRot", GetActorRot },
	{ "SetActorRot", SetActorRot },
	{ "IsActorTurning", IsActorTurning },
	{ "PlayActorChore", PlayActorChore },
	{ "StopActorChore", StopActorChore },
	{ "IsActorResting", IsActorResting },
	{ "GetActorChores", GetActorChores },
	{ "WorldToScreen", WorldToScreen },
	{ "exit", Exit },
	{ "FunctionName", FunctionName },
	{ "EnableControl", EnableControl },
	{ "DisableControl", DisableControl },
	{ "GetControlState", GetControlState },
	{ "PrintError", PrintError },
	{ "PrintWarning", PrintWarning },
	{ "PrintDebug", PrintDebug },
	{ "MakeCurrentSet", MakeCurrentSet },
	{ "LockSet", LockSet },
	{ "UnLockSet", UnLockSet },
	{ "MakeCurrentSetup", MakeCurrentSetup },
	{ "GetCurrentSetup", GetCurrentSetup },
	{ "NextSetup", NextSetup },
	{ "PreviousSetup", PreviousSetup },
	{ "StartFullscreenMovie", StartFullscreenMovie },
	{ "IsFullscreenMoviePlaying", IsFullscreenMoviePlaying },
	{ "StartMovie", StartMovie },
	{ "StopMovie", StopMovie },
	{ "PauseMovie", PauseMovie },
	{ "IsMoviePlaying", IsMoviePlaying },
	{ "PlaySound", PlaySound },
	{ "IsSoundPlaying", IsSoundPlaying },
	{ "SetSoundPosition", SetSoundPosition },
	{ "FileFindFirst", luaFileFindFirst },
	{ "FileFindNext", luaFileFindNext },
	{ "FileFindDispose", FileFindDispose },
	{ "InputDialog", InputDialog },
	{ "GetSectorOppositeEdge", GetSectorOppositeEdge },
	{ "MakeSectorActive", MakeSectorActive },
	{ "GetCurrentScript", GetCurrentScript },
	{ "GetCameraPosition", GetCameraPosition },
	{ "SetCameraPosition", SetCameraPosition },
	{ "GetCameraFOV", GetCameraFOV },
	{ "SetCameraFOV", SetCameraFOV },
	{ "GetCameraRoll", GetCameraRoll },
	{ "GetCameraLookVector", GetCameraLookVector },
	{ "TurnActorTo", TurnActorTo },
	{ "PerSecond", PerSecond },
	{ "GetAngleBetweenVectors", GetAngleBetweenVectors },
	{ "GetAngleBetweenActors", GetAngleBetweenActors },
	{ "SetAmbientLight", SetAmbientLight },
	{ "TurnLightOn", TurnLightOn },
	{ "SetLightPosition", SetLightPosition },
	{ "LightMgrStartup", LightMgrStartup },
	{ "ImStartSound", ImStartSound },
	{ "ImGetSfxVol", ImGetSfxVol },
	{ "ImGetVoiceVol", ImGetVoiceVol },
	{ "ImGetMusicVol", ImGetMusicVol },
	{ "ImSetState", ImSetState },
	{ "ImSetSequence", ImSetSequence },
	{ "LoadBundle", LoadBundle },
	{ "SetGamma", SetGamma },
	{ "SetActorWalkDominate", SetActorWalkDominate },
	{ "RenderModeUser", RenderModeUser },
	{ "DimScreen", DimScreen },
	{ "Display", Display },
	{ "SetSpeechMode", SetSpeechMode },
	{ "GetSpeechMode", GetSpeechMode },
	{ "KillActorShadows", KillActorShadows },
	{ "NewObjectState", NewObjectState },
	{ "SetActorCollisionMode", SetActorCollisionMode },
	{ "SetActorCollisionScale", SetActorCollisionScale },
	{ "SubmitSaveGameData", SubmitSaveGameData },
	{ "GetSaveGameData", GetSaveGameData },
	{ "SetTextSpeed", SetTextSpeed },
	{ "GetTextSpeed", GetTextSpeed },
	{ "JustLoaded", JustLoaded },
	{ "UnShrinkBoxes", UnShrinkBoxes },
	{ "GetMemoryUsage", GetMemoryUsage },
	{ "dofile", new_dofile },

	// Monkey specific opcodes:

	{ "SetActiveCD", SetActiveCD },
	{ "GetActiveCD", GetActiveCD },
	{ "AreWeInternational", AreWeInternational },
	{ "MakeScreenTextures", MakeScreenTextures },
	{ "ThumbnailFromFile", ThumbnailFromFile },
	{ "ClearSpecialtyTexture", ClearSpecialtyTexture },
	{ "UnloadActor", UnloadActor },
	{ "PutActorInOverworld", PutActorInOverworld },
	{ "RemoveActorFromOverworld", RemoveActorFromOverworld },
	{ "ClearOverworld", ClearOverworld },
	{ "ToggleOverworld", ToggleOverworld },
	{ "ActorStopMoving", ActorStopMoving },
	{ "SetActorFOV", SetActorFOV },
	{ "SetActorLighting", SetActorLighting },
	{ "SetActorHeadLimits", SetActorHeadLimits },
	{ "ActorActivateShadow", ActorActivateShadow },
	{ "EnableActorPuck", EnableActorPuck },
	{ "SetActorGlobalAlpha", SetActorGlobalAlpha },
	{ "SetActorLocalAlpha", SetActorLocalAlpha },
	{ "SetActorSortOrder", SetActorSortOrder },
	{ "GetActorSortOrder", GetActorSortOrder },
	{ "AttachActor", AttachActor },
	{ "DetachActor", DetachActor },
	{ "IsChoreValid", IsChoreValid },
	{ "IsChorePlaying", IsChorePlaying },
	{ "IsChoreLooping", IsChoreLooping },
	{ "StopActorChores", StopActorChores },
	{ "PlayChore", PlayChore },
	{ "StopChore", StopChore },
	{ "PauseChore", PauseChore },
	{ "AdvanceChore", AdvanceChore },
	{ "CompleteChore", CompleteChore },
	{ "LockChore", LockChore },
	{ "UnlockChore", UnlockChore },
	{ "LockChoreSet", LockChoreSet },
	{ "UnlockChoreSet", UnlockChoreSet },
	{ "LockBackground", LockBackground },
	{ "UnLockBackground", UnLockBackground },
	{ "EscapeMovie", EscapeMovie },
	{ "StopAllSounds", StopAllSounds },
	{ "LoadSound", LoadSound },
	{ "FreeSound", FreeSound },
	{ "PlayLoadedSound", PlayLoadedSound },
	{ "SetGroupVolume", SetGroupVolume },
	{ "GetSoundVolume", GetSoundVolume },
	{ "SetSoundVolume", SetSoundVolume },
	{ "EnableAudioGroup", EnableAudioGroup },
	{ "EnableVoiceFX", EnableVoiceFX },
	{ "PlaySoundFrom", PlaySoundFrom },
	{ "PlayLoadedSoundFrom", PlayLoadedSoundFrom },
	{ "SetReverb", SetReverb },
	{ "UpdateSoundPosition", UpdateSoundPosition },
	{ "ImSelectSet", ImSelectSet },
	{ "ImStateHasLooped", ImStateHasLooped },
	{ "ImStateHasEnded", ImStateHasEnded },
	{ "ImPushState", ImPushState },
	{ "ImPopState", ImPopState },
	{ "ImFlushStack", ImFlushStack },
	{ "ImGetMillisecondPosition", ImGetMillisecondPosition },
	{ "GetSectorName", GetSectorName },
	{ "GetCameraYaw", GetCameraYaw },
	{ "YawCamera", YawCamera },
	{ "GetCameraPitch", GetCameraPitch },
	{ "PitchCamera", PitchCamera },
	{ "RollCamera", RollCamera },
	{ "UndimAll", UndimAll },
	{ "UndimRegion", UndimRegion },
	{ "GetCPUSpeed", GetCPUSpeed },
	{ "NewLayer", NewLayer },
	{ "FreeLayer", FreeLayer },
	{ "SetLayerSortOrder", SetLayerSortOrder },
	{ "SetLayerFrame", SetLayerFrame },
	{ "AdvanceLayerFrame", AdvanceLayerFrame },
	{ "PushText", PushText },
	{ "PopText", PopText },
	{ "NukeAllScriptLocks", NukeAllScriptLocks },
	{ "ToggleDebugDraw", ToggleDebugDraw },
	{ "ToggleDrawCameras", ToggleDrawCameras },
	{ "ToggleDrawLights", ToggleDrawLights },
	{ "ToggleDrawSectors", ToggleDrawSectors },
	{ "ToggleDrawBBoxes", ToggleDrawBBoxes },
	{ "ToggleDrawFPS", ToggleDrawFPS },
	{ "ToggleDrawPerformance", ToggleDrawPerformance },
	{ "ToggleDrawActorStats", ToggleDrawActorStats },
	{ "SectEditSelect", SectEditSelect },
	{ "SectEditPlace", SectEditPlace },
	{ "SectEditDelete", SectEditDelete },
	{ "SectEditInsert", SectEditInsert },
	{ "SectEditSortAdd", SectEditSortAdd },
	{ "SectEditForgetIt", SectEditForgetIt },
	{ "FRUTEY_Begin", FRUTEY_Begin },
	{ "FRUTEY_End", FRUTEY_End }
};

struct luaL_reg textOpcodes[] = {
	{ "IsMessageGoing", IsMessageGoing },
	{ "SetSayLineDefaults", SetSayLineDefaults },
	{ "SetActorTalkColor", SetActorTalkColor },
	{ "SayLine", SayLine },
	{ "MakeTextObject", MakeTextObject },
	{ "GetTextObjectDimensions", GetTextObjectDimensions },
	{ "GetFontDimensions", GetFontDimensions },
	{ "ChangeTextObject", ChangeTextObject },
	{ "KillTextObject", KillTextObject },
	{ "ExpireText", ExpireText },
	{ "PurgeText", PurgeText },
	{ "MakeColor", MakeColor },
	{ "GetColorComponents", GetColorComponents },
	{ "GetTextCharPosition", GetTextCharPosition },
	{ "LocalizeString", LocalizeString },
	{ "SetOffscreenTextPos", SetOffscreenTextPos }
};

struct luaL_reg primitivesOpcodes[] = {
	{ "DrawLine", DrawLine },
	{ "ChangePrimitive", ChangePrimitive },
	{ "KillPrimitive", KillPrimitive },
	{ "PurgePrimitiveQueue", PurgePrimitiveQueue }
};

struct luaL_reg miscOpcodes[] = {
	{ "  concatfallback", concatFallback },
	{ "  typeoverride", typeOverride },
	{ "  dfltcamera", dummyHandler },
	{ "  dfltcontrol", dummyHandler },
};

} // end of namespace Grim
