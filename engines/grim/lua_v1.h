/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_LUA_V1
#define GRIM_LUA_V1

#include "engines/grim/lua.h"

// Windows.h badness: Remove #defines to the following Win32 API MultiByte/Unicode functions.
#ifdef GetDiskFreeSpace
#undef GetDiskFreeSpace
#endif

#ifdef PlaySound
#undef PlaySound
#endif

namespace Grim {

class Lua_V1 : public LuaBase {
public:
	typedef Lua_V1 LuaClass;
	void registerOpcodes() override;
	void boot() override;
	void postRestoreHandle() override;

protected:
	// Opcodes
	DECLARE_LUA_OPCODE(new_dofile);
	DECLARE_LUA_OPCODE(PrintDebug);
	DECLARE_LUA_OPCODE(PrintError);
	DECLARE_LUA_OPCODE(PrintWarning);
	DECLARE_LUA_OPCODE(FunctionName);
	DECLARE_LUA_OPCODE(CheckForFile);
	DECLARE_LUA_OPCODE(MakeColor);
	DECLARE_LUA_OPCODE(GetColorComponents);
	DECLARE_LUA_OPCODE(ReadRegistryValue);
	DECLARE_LUA_OPCODE(WriteRegistryValue);
	DECLARE_LUA_OPCODE(LoadActor);
	DECLARE_LUA_OPCODE(GetActorTimeScale);
	DECLARE_LUA_OPCODE(SetSelectedActor);
	DECLARE_LUA_OPCODE(GetCameraActor);
	DECLARE_LUA_OPCODE(SetSayLineDefaults);
	DECLARE_LUA_OPCODE(SetActorTalkColor);
	DECLARE_LUA_OPCODE(GetActorTalkColor);
	DECLARE_LUA_OPCODE(SetActorRestChore);
	DECLARE_LUA_OPCODE(SetActorWalkChore);
	DECLARE_LUA_OPCODE(SetActorTurnChores);
	DECLARE_LUA_OPCODE(SetActorTalkChore);
	DECLARE_LUA_OPCODE(SetActorMumblechore);
	DECLARE_LUA_OPCODE(SetActorVisibility);
	DECLARE_LUA_OPCODE(SetActorScale);
	DECLARE_LUA_OPCODE(SetActorTimeScale);
	DECLARE_LUA_OPCODE(SetActorCollisionMode);
	DECLARE_LUA_OPCODE(SetActorCollisionScale);
	DECLARE_LUA_OPCODE(PutActorAt);
	DECLARE_LUA_OPCODE(GetActorPos);
	DECLARE_LUA_OPCODE(SetActorRot);
	DECLARE_LUA_OPCODE(GetActorRot);
	DECLARE_LUA_OPCODE(IsActorTurning);
	DECLARE_LUA_OPCODE(GetAngleBetweenActors);
	DECLARE_LUA_OPCODE(GetAngleBetweenVectors);
	DECLARE_LUA_OPCODE(GetActorYawToPoint);
	DECLARE_LUA_OPCODE(PutActorInSet);
	DECLARE_LUA_OPCODE(SetActorWalkRate);
	DECLARE_LUA_OPCODE(GetActorWalkRate);
	DECLARE_LUA_OPCODE(SetActorTurnRate);
	DECLARE_LUA_OPCODE(WalkActorForward);
	DECLARE_LUA_OPCODE(SetActorReflection);
	DECLARE_LUA_OPCODE(GetActorPuckVector);
	DECLARE_LUA_OPCODE(ActorPuckOrient);
	DECLARE_LUA_OPCODE(WalkActorTo);
	DECLARE_LUA_OPCODE(ActorToClean);
	DECLARE_LUA_OPCODE(IsActorMoving);
	DECLARE_LUA_OPCODE(Is3DHardwareEnabled);
	DECLARE_LUA_OPCODE(SetHardwareState);
	DECLARE_LUA_OPCODE(SetVideoDevices);
	DECLARE_LUA_OPCODE(GetVideoDevices);
	DECLARE_LUA_OPCODE(EnumerateVideoDevices);
	DECLARE_LUA_OPCODE(Enumerate3DDevices);
	DECLARE_LUA_OPCODE(IsActorResting);
	DECLARE_LUA_OPCODE(GetActorNodeLocation);
	DECLARE_LUA_OPCODE(SetActorWalkDominate);
	DECLARE_LUA_OPCODE(SetActorColormap);
	DECLARE_LUA_OPCODE(TurnActor);
	DECLARE_LUA_OPCODE(PushActorCostume);
	DECLARE_LUA_OPCODE(SetActorCostume);
	DECLARE_LUA_OPCODE(GetActorCostume);
	DECLARE_LUA_OPCODE(PopActorCostume);
	DECLARE_LUA_OPCODE(GetActorCostumeDepth);
	DECLARE_LUA_OPCODE(PrintActorCostumes);
	DECLARE_LUA_OPCODE(LoadCostume);
	DECLARE_LUA_OPCODE(PlayActorChore);
	DECLARE_LUA_OPCODE(CompleteActorChore);
	DECLARE_LUA_OPCODE(PlayActorChoreLooping);
	DECLARE_LUA_OPCODE(SetActorChoreLooping);
	DECLARE_LUA_OPCODE(StopActorChore);
	DECLARE_LUA_OPCODE(FadeOutChore);
	DECLARE_LUA_OPCODE(FadeInChore);
	DECLARE_LUA_OPCODE(IsActorChoring);
	DECLARE_LUA_OPCODE(ActorLookAt);
	DECLARE_LUA_OPCODE(TurnActorTo);
	DECLARE_LUA_OPCODE(PointActorAt);
	DECLARE_LUA_OPCODE(WalkActorVector);
	DECLARE_LUA_OPCODE(RotateVector);
	DECLARE_LUA_OPCODE(SetActorPitch);
	DECLARE_LUA_OPCODE(SetActorLookRate);
	DECLARE_LUA_OPCODE(GetActorLookRate);
	DECLARE_LUA_OPCODE(SetActorHead);
	DECLARE_LUA_OPCODE(PutActorAtInterest);
	DECLARE_LUA_OPCODE(SetActorFollowBoxes);
	DECLARE_LUA_OPCODE(SetActorConstrain);
	DECLARE_LUA_OPCODE(GetVisibleThings);
	DECLARE_LUA_OPCODE(SetShadowColor);
	DECLARE_LUA_OPCODE(KillActorShadows);
	DECLARE_LUA_OPCODE(SetActiveShadow);
	DECLARE_LUA_OPCODE(SetActorShadowPoint);
	DECLARE_LUA_OPCODE(SetActorShadowPlane);
	DECLARE_LUA_OPCODE(AddShadowPlane);
	DECLARE_LUA_OPCODE(ActivateActorShadow);
	DECLARE_LUA_OPCODE(SetActorShadowValid);
	DECLARE_LUA_OPCODE(TextFileGetLine);
	DECLARE_LUA_OPCODE(TextFileGetLineCount);
	DECLARE_LUA_OPCODE(LocalizeString);
	DECLARE_LUA_OPCODE(SayLine);
	DECLARE_LUA_OPCODE(PrintLine);
	DECLARE_LUA_OPCODE(InputDialog);
	DECLARE_LUA_OPCODE(IsMessageGoing);
	DECLARE_LUA_OPCODE(ShutUpActor);
	DECLARE_LUA_OPCODE(GetPointSector);
	DECLARE_LUA_OPCODE(GetActorSector);
	DECLARE_LUA_OPCODE(IsActorInSector);
	DECLARE_LUA_OPCODE(IsPointInSector);
	DECLARE_LUA_OPCODE(MakeSectorActive);
	DECLARE_LUA_OPCODE(LockSet);
	DECLARE_LUA_OPCODE(UnLockSet);
	DECLARE_LUA_OPCODE(MakeCurrentSet);
	DECLARE_LUA_OPCODE(MakeCurrentSetup);
	DECLARE_LUA_OPCODE(GetCurrentSetup);
	DECLARE_LUA_OPCODE(ShrinkBoxes);
	DECLARE_LUA_OPCODE(UnShrinkBoxes);
	DECLARE_LUA_OPCODE(GetShrinkPos);
	DECLARE_LUA_OPCODE(ImStartSound);
	DECLARE_LUA_OPCODE(ImStopSound);
	DECLARE_LUA_OPCODE(ImStopAllSounds);
	DECLARE_LUA_OPCODE(ImPause);
	DECLARE_LUA_OPCODE(ImResume);
	DECLARE_LUA_OPCODE(ImSetVoiceEffect);
	DECLARE_LUA_OPCODE(ImSetMusicVol);
	DECLARE_LUA_OPCODE(ImGetMusicVol);
	DECLARE_LUA_OPCODE(ImSetVoiceVol);
	DECLARE_LUA_OPCODE(ImGetVoiceVol);
	DECLARE_LUA_OPCODE(ImSetSfxVol);
	DECLARE_LUA_OPCODE(ImGetSfxVol);
	DECLARE_LUA_OPCODE(ImSetParam);
	DECLARE_LUA_OPCODE(ImGetParam);
	DECLARE_LUA_OPCODE(ImFadeParam);
	DECLARE_LUA_OPCODE(ImSetState);
	DECLARE_LUA_OPCODE(ImSetSequence);
	DECLARE_LUA_OPCODE(SaveIMuse);
	DECLARE_LUA_OPCODE(RestoreIMuse);
	DECLARE_LUA_OPCODE(SetSoundPosition);
	DECLARE_LUA_OPCODE(IsSoundPlaying);
	DECLARE_LUA_OPCODE(PlaySoundAt);
	DECLARE_LUA_OPCODE(FileFindDispose);
	DECLARE_LUA_OPCODE(FileFindNext);
	DECLARE_LUA_OPCODE(FileFindFirst);
	DECLARE_LUA_OPCODE(PerSecond);
	DECLARE_LUA_OPCODE(EnableControl);
	DECLARE_LUA_OPCODE(DisableControl);
	DECLARE_LUA_OPCODE(GetControlState);
	DECLARE_LUA_OPCODE(GetImage);
	DECLARE_LUA_OPCODE(FreeImage);
	DECLARE_LUA_OPCODE(BlastImage);
	DECLARE_LUA_OPCODE(CleanBuffer);
	DECLARE_LUA_OPCODE(Exit);
	DECLARE_LUA_OPCODE(KillTextObject);
	DECLARE_LUA_OPCODE(ChangeTextObject);
	DECLARE_LUA_OPCODE(GetTextSpeed);
	DECLARE_LUA_OPCODE(SetTextSpeed);
	DECLARE_LUA_OPCODE(MakeTextObject);
	DECLARE_LUA_OPCODE(GetTextObjectDimensions);
	DECLARE_LUA_OPCODE(ExpireText);
	DECLARE_LUA_OPCODE(GetTextCharPosition);
	DECLARE_LUA_OPCODE(BlastText);
	DECLARE_LUA_OPCODE(SetOffscreenTextPos);
	DECLARE_LUA_OPCODE(SetSpeechMode);
	DECLARE_LUA_OPCODE(GetSpeechMode);
	DECLARE_LUA_OPCODE(StartFullscreenMovie);
	DECLARE_LUA_OPCODE(StartMovie);
	DECLARE_LUA_OPCODE(IsFullscreenMoviePlaying);
	DECLARE_LUA_OPCODE(IsMoviePlaying);
	DECLARE_LUA_OPCODE(StopMovie);
	DECLARE_LUA_OPCODE(PauseMovie);
	DECLARE_LUA_OPCODE(PurgePrimitiveQueue);
	DECLARE_LUA_OPCODE(DrawPolygon);
	DECLARE_LUA_OPCODE(DrawLine);
	DECLARE_LUA_OPCODE(ChangePrimitive);
	DECLARE_LUA_OPCODE(DrawRectangle);
	DECLARE_LUA_OPCODE(BlastRect);
	DECLARE_LUA_OPCODE(KillPrimitive);
	DECLARE_LUA_OPCODE(DimScreen);
	DECLARE_LUA_OPCODE(DimRegion);
	DECLARE_LUA_OPCODE(GetDiskFreeSpace);
	DECLARE_LUA_OPCODE(NewObjectState);
	DECLARE_LUA_OPCODE(FreeObjectState);
	DECLARE_LUA_OPCODE(SendObjectToBack);
	DECLARE_LUA_OPCODE(SendObjectToFront);
	DECLARE_LUA_OPCODE(SetObjectType);
	DECLARE_LUA_OPCODE(GetCurrentScript);
	DECLARE_LUA_OPCODE(ScreenShot);
	DECLARE_LUA_OPCODE(GetSaveGameImage);
	DECLARE_LUA_OPCODE(SubmitSaveGameData);
	DECLARE_LUA_OPCODE(GetSaveGameData);
	DECLARE_LUA_OPCODE(Load);
	DECLARE_LUA_OPCODE(Save);
	DECLARE_LUA_OPCODE(Remove);
	DECLARE_LUA_OPCODE(LockFont);
	DECLARE_LUA_OPCODE(EnableDebugKeys);
	DECLARE_LUA_OPCODE(LightMgrSetChange);
	DECLARE_LUA_OPCODE(SetAmbientLight);
	DECLARE_LUA_OPCODE(SetLightIntensity);
	DECLARE_LUA_OPCODE(SetLightPosition);
	DECLARE_LUA_OPCODE(TurnLightOn);
	DECLARE_LUA_OPCODE(RenderModeUser);
	DECLARE_LUA_OPCODE(PreRender);
	DECLARE_LUA_OPCODE(IrisUp);
	DECLARE_LUA_OPCODE(IrisDown);
	DECLARE_LUA_OPCODE(SetGamma);
	DECLARE_LUA_OPCODE(Display);
	DECLARE_LUA_OPCODE(EngineDisplay);
	DECLARE_LUA_OPCODE(ForceRefresh);
	DECLARE_LUA_OPCODE(JustLoaded);
	DECLARE_LUA_OPCODE(PlaySound);
	DECLARE_LUA_OPCODE(SetEmergencyFont);
	DECLARE_LUA_OPCODE(LoadBundle);
	DECLARE_LUA_OPCODE(LightMgrStartup);
	DECLARE_LUA_OPCODE(GetSectorOppositeEdge);
	DECLARE_LUA_OPCODE(SetActorInvClipNode);
	DECLARE_LUA_OPCODE(NukeResources);
	DECLARE_LUA_OPCODE(ResetTextures);
	DECLARE_LUA_OPCODE(AttachToResources);
	DECLARE_LUA_OPCODE(DetachFromResources);
	DECLARE_LUA_OPCODE(SetActorClipPlane);
	DECLARE_LUA_OPCODE(SetActorClipActive);
	DECLARE_LUA_OPCODE(FlushControls);
	DECLARE_LUA_OPCODE(GetCameraLookVector);
	DECLARE_LUA_OPCODE(SetCameraRoll);
	DECLARE_LUA_OPCODE(SetCameraInterest);
	DECLARE_LUA_OPCODE(GetCameraPosition);
	DECLARE_LUA_OPCODE(SpewStartup);
	DECLARE_LUA_OPCODE(PreviousSetup);
	DECLARE_LUA_OPCODE(NextSetup);
	DECLARE_LUA_OPCODE(WorldToScreen);
	DECLARE_LUA_OPCODE(SetActorRoll);
	DECLARE_LUA_OPCODE(SetActorFrustrumCull);
	DECLARE_LUA_OPCODE(DriveActorTo);
	DECLARE_LUA_OPCODE(GetActorRect);
	DECLARE_LUA_OPCODE(GetTranslationMode);
	DECLARE_LUA_OPCODE(SetTranslationMode);
	DECLARE_LUA_OPCODE(WalkActorToAvoiding);
	DECLARE_LUA_OPCODE(GetActorChores);
	DECLARE_LUA_OPCODE(SetCameraPosition);
	DECLARE_LUA_OPCODE(GetCameraFOV);
	DECLARE_LUA_OPCODE(SetCameraFOV);
	DECLARE_LUA_OPCODE(GetCameraRoll);
	DECLARE_LUA_OPCODE(GetMemoryUsage);
	DECLARE_LUA_OPCODE(GetFontDimensions);
	DECLARE_LUA_OPCODE(PurgeText);
};

}

#endif
