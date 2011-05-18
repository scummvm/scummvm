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
 */

#ifndef GRIM_LUA_HH
#define GRIM_LUA_HH

#include "engines/grim/lua/lua.h"

namespace Grim {

class Actor;
class Color;
class Costume;
class Font;
class ObjectState;
class PrimitiveObject;
class TextObject;
class TextObjectDefaults;
class TextObjectCommon;

extern int refSystemTable;
extern int refTypeOverride;
extern int refOldConcatFallback;
extern int refTextObjectX;
extern int refTextObjectY;
extern int refTextObjectFont;
extern int refTextObjectWidth;
extern int refTextObjectHeight;
extern int refTextObjectFGColor;
extern int refTextObjectBGColor;
extern int refTextObjectFXColor;
extern int refTextObjectHIColor;
extern int refTextObjectDuration;
extern int refTextObjectCenter;
extern int refTextObjectLJustify;
extern int refTextObjectRJustify;
extern int refTextObjectVolume;
extern int refTextObjectBackground;
extern int refTextObjectPan;

// Helpers
bool getbool(int num);
void pushbool(bool val);
void pushobject(int id, int32 tag);
int getobject(lua_Object obj);
Actor *getactor(lua_Object obj);
TextObject *gettextobject(lua_Object obj);
Font *getfont(lua_Object obj);
Color *getcolor(lua_Object obj);
PrimitiveObject *getprimitive(lua_Object obj);
ObjectState *getobjectstate(lua_Object obj);
byte clamp_color(int c);
bool findCostume(lua_Object costumeObj, Actor *actor, Costume **costume);

Common::String parseMsgText(const char *msg, char *msgId);
void parseSayLineTable(lua_Object paramObj, bool *background, int *vol, int *pan, int *x, int *y);
PointerId saveCallback(int32 /*tag*/, PointerId ptr, SaveSint32 /*savedState*/);
PointerId restoreCallback(int32 /*tag*/, PointerId ptr, RestoreSint32 /*savedState*/);
void setTextObjectParams(TextObjectCommon *textObject, lua_Object tableObj);
void dummyHandler();

void setFrameTime(float frameTime);
void setMovieTime(float movieTime);
void registerGrimOpcodes();
void registerMonkeyOpcodes();
void registerLua();

// Opcodes
void L1_new_dofile();
void L1_PrintDebug();
void L1_PrintError();
void L1_PrintWarning();
void L1_FunctionName();
void L1_CheckForFile();
void L1_MakeColor();
void L1_GetColorComponents();
void L1_ReadRegistryValue();
void L1_WriteRegistryValue();
void L1_LoadActor();
void L1_GetActorTimeScale();
void L1_SetSelectedActor();
void L1_GetCameraActor();
void L1_SetSayLineDefaults();
void L1_SetActorTalkColor();
void L1_GetActorTalkColor();
void L1_SetActorRestChore();
void L1_SetActorWalkChore();
void L1_SetActorTurnChores();
void L1_SetActorTalkChore();
void L1_SetActorMumblechore();
void L1_SetActorVisibility();
void L1_SetActorScale();
void L1_PutActorAt();
void L1_GetActorPos();
void L1_SetActorRot();
void L1_GetActorRot();
void L1_IsActorTurning();
void L1_GetAngleBetweenActors();
void L1_GetAngleBetweenVectors();
void L1_GetActorYawToPoint();
void L1_PutActorInSet();
void L1_SetActorWalkRate();
void L1_GetActorWalkRate();
void L1_SetActorTurnRate();
void L1_WalkActorForward();
void L1_SetActorReflection();
void L1_GetActorPuckVector();
void L1_WalkActorTo();
void L1_ActorToClean();
void L1_IsActorMoving();
void L1_Is3DHardwareEnabled();
void L1_SetHardwareState();
void L1_SetVideoDevices();
void L1_GetVideoDevices();
void L1_EnumerateVideoDevices();
void L1_Enumerate3DDevices();
void L1_IsActorResting();
void L1_GetActorNodeLocation();
void L1_SetActorWalkDominate();
void L1_SetActorColormap();
void L1_TurnActor();
void L1_PushActorCostume();
void L1_SetActorCostume();
void L1_GetActorCostume();
void L1_PopActorCostume();
void L1_GetActorCostumeDepth();
void L1_PrintActorCostumes();
void L1_LoadCostume();
void L1_PlayActorChore();
void L1_CompleteActorChore();
void L1_PlayActorChoreLooping();
void L1_SetActorChoreLooping();
void L1_StopActorChore();
void L1_FadeOutChore();
void L1_FadeInChore();
void L1_IsActorChoring();
void L1_ActorLookAt();
void L1_TurnActorTo();
void L1_PointActorAt();
void L1_WalkActorVector();
void L1_RotateVector();
void L1_SetActorPitch();
void L1_SetActorLookRate();
void L1_GetActorLookRate();
void L1_SetActorHead();
void L1_PutActorAtInterest();
void L1_SetActorFollowBoxes();
void L1_SetActorConstrain();
void L1_GetVisibleThings();
void L1_SetShadowColor();
void L1_KillActorShadows();
void L1_SetActiveShadow();
void L1_SetActorShadowPoint();
void L1_SetActorShadowPlane();
void L1_AddShadowPlane();
void L1_ActivateActorShadow();
void L1_SetActorShadowValid();
void L1_TextFileGetLine();
void L1_TextFileGetLineCount();
void L1_LocalizeString();
void L1_SayLine();
void L1_PrintLine();
void L1_InputDialog();
void L1_IsMessageGoing();
void L1_ShutUpActor();
void L1_GetPointSector();
void L1_GetActorSector();
void L1_IsActorInSector();
void L1_IsPointInSector();
void L1_MakeSectorActive();
void L1_LockSet();
void L1_UnLockSet();
void L1_MakeCurrentSet();
void L1_MakeCurrentSetup();
void L1_GetCurrentSetup();
void L1_GetShrinkPos();
void L1_ImStartSound();
void L1_ImStopSound();
void L1_ImStopAllSounds();
void L1_ImPause();
void L1_ImResume();
void L1_ImSetVoiceEffect();
void L1_ImSetMusicVol();
void L1_ImGetMusicVol();
void L1_ImSetVoiceVol();
void L1_ImGetVoiceVol();
void L1_ImSetSfxVol();
void L1_ImGetSfxVol();
void L1_ImSetParam();
void L1_ImGetParam();
void L1_ImFadeParam();
void L1_ImSetState();
void L1_ImSetSequence();
void L1_SaveIMuse();
void L1_RestoreIMuse();
void L1_SetSoundPosition();
void L1_IsSoundPlaying();
void L1_PlaySoundAt();
void L1_FileFindDispose();
void L1_FileFindNext();
void L1_FileFindFirst();
void L1_PerSecond();
void L1_EnableControl();
void L1_DisableControl();
void L1_GetControlState();
void L1_GetImage();
void L1_FreeImage();
void L1_BlastImage();
void L1_CleanBuffer();
void L1_Exit();
void L1_KillTextObject();
void L1_ChangeTextObject();
void L1_GetTextSpeed();
void L1_SetTextSpeed();
void L1_MakeTextObject();
void L1_GetTextObjectDimensions();
void L1_ExpireText();
void L1_GetTextCharPosition();
void L1_BlastText();
void L1_SetOffscreenTextPos();
void L1_SetSpeechMode();
void L1_GetSpeechMode();
void L1_StartFullscreenMovie();
void L1_StartMovie();
void L1_IsFullscreenMoviePlaying();
void L1_IsMoviePlaying();
void L1_StopMovie();
void L1_PauseMovie();
void L1_PurgePrimitiveQueue();
void L1_DrawPolygon();
void L1_DrawLine();
void L1_ChangePrimitive();
void L1_DrawRectangle();
void L1_BlastRect();
void L1_KillPrimitive();
void L1_DimScreen();
void L1_DimRegion();
void L1_GetDiskFreeSpace();
void L1_NewObjectState();
void L1_FreeObjectState();
void L1_SendObjectToBack();
void L1_SendObjectToFront();
void L1_SetObjectType();
void L1_GetCurrentScript();
void L1_ScreenShot();
void L1_GetSaveGameImage();
void L1_SubmitSaveGameData();
void L1_GetSaveGameData();
void L1_Load();
void L1_Save();
void L1_Remove();
void L1_LockFont();
void L1_EnableDebugKeys();
void L1_LightMgrSetChange();
void L1_SetAmbientLight();
void L1_SetLightIntensity();
void L1_SetLightPosition();
void L1_RenderModeUser();
void L1_SetGamma();
void L1_Display();
void L1_EngineDisplay();
void L1_ForceRefresh();
void L1_JustLoaded();
void L1_PlaySound();
void L1_SetEmergencyFont();
void L1_LoadBundle();
void L1_LightMgrStartup();
void L1_typeOverride();
void L1_concatFallback();

} // end of namespace Grim

#endif
