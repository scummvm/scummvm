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

#include "watchmaker/struct.h"
#include "watchmaker/sysdef.h"
#include "watchmaker/define.h"
#include "watchmaker/message.h"
#include "watchmaker/globvar.h"
#include "watchmaker/3d/t3d_mesh.h"

namespace Watchmaker {

// GAME RECT
GameRect::GameRect() {
	_bigIconRect = {INV_MARG_DX + 30, INV_MARG_UP - 12, 775, 587 /*INV_MARG_DOWN+10 */};
	_useIconRect = {720, 517, 787, 587};
	_closeInvRect = {667, 13, 782, 73};
	_quitGameRect = {563, 13, 667, 73};
	_playerInvRect = {22, 13, 200, 73};
	_invSaveRect = {223, 13, 284, 73};
	_invLoadRect = {332, 13, 401, 73};
	_invOptionsRect = {437, 13, 523, 73};
	_inv1Up = {3 + 16, 67 + 56, 3 + 28, 67 + 96};
	_inv1Down = {3 + 205, 67 + 411, 3 + 224, 67 + 444};
	_diag2Up = {768, 510, 797, 537};
	_diag2Down = {768, 568, 797, 595};
	_callOtherPlayerRect = {22, 13, 22 + 192, 13 + 60};
}
struct SD3DRect D3DRectsList[MAX_D3D_RECTS];
struct SD3DTriangle D3DTrianglesList[MAX_D3D_TRIANGLES];
struct SD3DBitmap D3DBitmapsList[MAX_D3D_BITMAPS];
struct SDDBitmap DDBitmapsList[MAX_DD_BITMAPS];
struct SDDText DDTextsList[MAX_DD_TEXTS];
struct SDDText RendText[MAX_REND_TEXTS];

// 2D INTERFACE
int32 MainDx, MainDy, MainBpp;
int32 IconsPics[MAX_ICONS];
int32 Console1, ConsoleFrecciaGiu, ConsoleFrecciaSu, Console2, Console3, Console4, Console5, Console6, ConsoleNoSwitchDar, ConsoleNoSwitchVic, ConsoleNoSave;
int32 ConsoleD1, ConsoleD2, StartPic, EndPic, NewLogImage, LoadingImage;
int32 CurMousePointer, MousePointerDefault, MousePointerPlus, TrecLogo;
int32 GopherMap, GopherPos[MAX_GOPHERS], GopherBall;
struct SRect MousePointerLim;

// LOADER VARS
char LoaderName[T3D_NAMELEN] = "rxt.t3d";
char WmCfgName[T3D_NAMELEN] = "wm.cfg";
uint32 LoaderFlags = 0;
char WmGameDataPak_FilePath[MAX_PATH] = WM_GAMEDATA_PACK_FILENAME;

// SENTENCES
char *ObjName[MAX_OBJ_NAMES];
char *Sentence[MAX_SENTENCES];
char *SysSent[MAX_SYS_SENTS];
char *TooltipSent[MAX_TOOLTIP_SENTS];
struct SString TheString = { "", 0, 0 };
char CurDebugString[MAX_STRING_LEN];
char TextLines[MAX_TEXT_LINES][MAX_TEXT_CHARS];

// MESSAGE SYSTEM
message *TheMessage;

// GAME VARS
int32 CurObj;
int32 NextPortalObj;
int32 NextPortalAnim;
int32 FromFirstPersonAnim;
int32 ToFirstPersonSent;
int32 CurInvObj;
int32 BigInvObj;
int32 CurPlayer = DARRELL;
int32 CurDialog, CurMenu, CurDlgItem;
int32 CurSubMusic;
int32 CurAlternate[T3D_MAX_CHARACTERS];
int32 CurGopher;

// TIME
uint32 TheTime;
uint32 NextRefresh, PaintTime, LoadTime;
t3dF32 CurFps, AvgFps;
t3dF32 FrameFactor;

// INV
uint8 Inv[MAX_PLAYERS][MAX_ICONS_IN_INV];
uint8 InvLen[MAX_PLAYERS], InvBase[MAX_PLAYERS];
uint8 InvStatus = INV_OFF;
uint8 Diag2Base;

// USE WITH
int32 UseWith[2];

// SEMAFORI
uint8 bSomeOneSpeak;
uint8 bPlayerSpeak;
uint8 bUseWith;
uint8 bSkipTalk;
uint8 bFirstPerson;
uint8 bDialogActive;
uint8 bDialogMenuActive;
uint8 bMovingCamera;
uint8 bCutCamera;
uint8 bAllowCalcCamera;
uint8 bAnimWaitText;
uint8 bPlayerInAnim;
uint8 bFastWalk;
uint8 bNotSkippableWalk;
uint8 bNoFirstPersonSwitch;
uint8 bClock33;
uint8 bShowInfo;
uint8 bShowBoundingBox;
uint8 bShowPanels;
uint8 bForceWire;
uint8 bPauseAllAnims;
uint8 bT2DActive;
uint8 bGolfActive = false; // HACK
uint8 bGolfMode;
uint8 bIngnoreDIKeyboard;
uint8 bDisableDiary;
uint8 bNoPlayerSwitch;
uint8 bCacciatore;
uint8 bDarkScreen;
uint8 bSaveDisabled;
uint8 bLockCamera;
uint8 bMiscLoaded;
uint8 bStartMenuActive;
uint8 bShowOnlyLoadWindow;
uint8 bNotSkippableSent;
uint8 bPorteEsternoBloccate;
uint8 bSuperView;
uint8 bSezioneLabirinto;
uint8 bPlayerSuBasamento;
uint8 bWideScreen;
uint8 bTitoliCodaStatic;
uint8 bTitoliCodaScrolling;
uint8 bDontPlaySteps;
uint8 bShowRoomDescriptions;
uint8 bShowExtraLocalizationStrings;

// 3D INTERFACE
uint8 PlayerPos[T3D_MAX_CHARACTERS];
uint8 PlayerGotoPos[T3D_MAX_CHARACTERS];
uint8 ForcedCamera, FastWalk;
uint16 AnimAutoPush;
int32 CameraTargetObj, CameraTargetBone;
int32 TimeWalk, TimeAnim;

// MOUSE VARS
char ObjectUnderCursor[MAX_STRING_LEN];
t3dV3F mPos;

// 3D MESH MODIFIER
struct SPlayerStand PlayerStand[MAX_PLAYERS];

// CHARACTERS VARS
t3dCHARACTER *Player;
t3dCHARACTER *Character[T3D_MAX_CHARACTERS];
t3dCHARSTOPSTATUS CharStopStatus[T3D_MAX_CHARACTERS];
t3dCHARACTER *Lanc33[4];
t3dCHARACTER *Palla50;
t3dCHARACTER *Freccia50;
t3dCHARACTER *GopherMark[MAX_GOPHERS];
t3dCHARACTER *Explode50;
t3dMESH *t3dGolfSky;
int32 Comb2D[5], Comb2Q[5], Comb19[3], Comb1D[5], Forno25, Frigo25, Comb31[5], Comb33[4], Comb42[12], Comb44[3], Comb45[5];
uint8 bMoglieGym, bMoglieSangue;

// DIALOG VARS
uint8 UsedDlgMenu[MAX_PLAYERS][T3D_MAX_CHARACTERS][MAX_DLG_MENUS];  //DlgMenu che sono gia' stati utilizzati dal personaggio

// OPTIONS VARS
struct SGameOptions GameOptions;

} // End of namespace Watchmaker
