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

#ifndef WATCHMAKER_GLOBVAR_H
#define WATCHMAKER_GLOBVAR_H

#include "watchmaker/struct.h"
#include "watchmaker/3d/t3d_mesh.h"

namespace Watchmaker {

#define MAX_PATH 256

// GAME RECT
struct GameRect {
	GameRect();
	SRect _bigIconRect;
	SRect _useIconRect;
	SRect _quitGameRect;
	SRect _closeInvRect;
	SRect _playerInvRect;
	SRect _invSaveRect;
	SRect _invLoadRect;
	SRect _invOptionsRect;
	SRect _inv1Up;
	SRect _inv1Down;
	SRect _diag2Up;
	SRect _diag2Down;
	SRect _callOtherPlayerRect;
};
extern struct SD3DRect D3DRectsList[];
extern struct SD3DTriangle D3DTrianglesList[];
extern struct SD3DBitmap D3DBitmapsList[];

// 2D INTERFACE
extern int32 MainDx, MainDy, MainBpp;
extern int32 IconsPics[MAX_ICONS];
extern int32 Console1, ConsoleFrecciaGiu, ConsoleFrecciaSu, Console2, Console3, Console4, Console5, Console6, ConsoleNoSwitchDar, ConsoleNoSwitchVic, ConsoleNoSave;
extern int32 ConsoleD1, ConsoleD2, StartPic, EndPic, NewLogImage, LoadingImage;
extern int32 CurMousePointer, MousePointerDefault, MousePointerPlus, TrecLogo;
extern int32 GopherMap, GopherPos[], GopherBall;
extern struct SRect MousePointerLim;

// 3D INTERFACE
extern t3dF32 CurFloorY;
extern uint8 PlayerPos[];
extern uint8 PlayerGotoPos[];
extern uint8 ForcedCamera, FastWalk;
extern uint16 AnimAutoPush;
extern int32 CameraTargetObj, CameraTargetBone;
extern int32 TimeWalk, TimeAnim;

// LOADER VARS
extern char LoaderName[];
extern char WmCfgName[];
extern uint32 LoaderFlags;
extern char WmGameDataPak_FilePath[];

// INV VARS
struct InvVars {
	t3dBODY *t3dIcons;
	t3dMESH CameraDummy;
	t3dCAMERA t3dIconCamera;
	t3dM3X3F BigIconM;
};

// MOUSE VARS
extern char ObjectUnderCursor[];
extern t3dV3F mPos;

extern struct SPlayerStand PlayerStand[];

// SENTENCES
extern char *ObjName[];
extern char *Sentence[];
extern char *SysSent[];
extern char *TooltipSent[];
extern struct SString TheString;
extern char CurDebugString[];
extern char TextLines[MAX_TEXT_LINES][MAX_TEXT_CHARS];

extern t3dCHARACTER *Player;
extern t3dCHARACTER *Character[];
extern t3dCHARSTOPSTATUS CharStopStatus[];
extern t3dCHARACTER *Lanc33[];
extern t3dCHARACTER *Sfera34;
extern t3dCHARACTER *Palla50;
extern t3dCHARACTER *Freccia50;
extern t3dCHARACTER *GopherMark[];
extern t3dCHARACTER *Explode50;
extern t3dMESH *t3dGolfSky;
extern int32 Comb2D[], Comb2Q[], Comb19[], Comb1D[], Forno25, Frigo25, Comb31[], Comb33[], Comb42[], Comb44[], Comb45[];
extern uint8 bMoglieGym, bMoglieSangue, bNoPlayerSwitch;

// MESSAGE SYSTEM
extern message *TheMessage;

// GAME VARS
extern int32 CurObj;
extern int32 NextPortalObj;
extern int32 NextPortalAnim;
extern int32 FromFirstPersonAnim;
extern int32 ToFirstPersonSent;
extern int32 CurInvObj;
extern int32 BigInvObj;
extern int32 CurPlayer;
extern int32 CurDialog, CurMenu, CurDlgItem;
extern int32 CurSubMusic;
extern int32 CurAlternate[];
extern int32 CurGopher;

extern uint32 TheTime;
extern uint32 NextRefresh, PaintTime, LoadTime;
extern t3dF32 CurFps, AvgFps;
extern t3dF32 FrameFactor;

// INV
extern uint8 Inv[MAX_PLAYERS][MAX_ICONS_IN_INV];
extern uint8 InvLen[], InvBase[];
extern uint8 InvStatus;
extern uint8 Diag2Base;

// USE WITH
extern int32 UseWith[];

// SEMAFORI
extern uint8 bSomeOneSpeak;
extern uint8 bPlayerSpeak;
extern uint8 bUseWith;
extern uint8 bSkipTalk;
extern uint8 bFirstPerson;
extern uint8 bDialogActive;
extern uint8 bDialogMenuActive;
extern uint8 bMovingCamera;
extern uint8 bCutCamera;
extern uint8 bAllowCalcCamera;
extern uint8 bAnimWaitText;
extern uint8 bPlayerInAnim;
extern uint8 bFastWalk;
extern uint8 bNotSkippableWalk;
extern uint8 bNoFirstPersonSwitch;
extern uint8 bClock33;
extern uint8 bShowInfo;
extern uint8 bShowBoundingBox;
extern uint8 bShowPanels;
extern uint8 bForceWire;
extern uint8 bPauseAllAnims;
extern uint8 bT2DActive;
extern uint8 bGolfActive;
extern uint8 bGolfMode;
extern uint8 bIngnoreDIKeyboard;
extern uint8 bDisableDiary;
extern uint8 bCacciatore;
extern uint8 bDarkScreen;
extern uint8 bSaveDisabled;
extern uint8 bLockCamera;
extern uint8 bMiscLoaded;
extern uint8 bStartMenuActive;
extern uint8 bShowOnlyLoadWindow;
extern uint8 bSbCacciatorehowOnlyLoadWindow;
extern uint8 bNotSkippableSent;
extern uint8 bPorteEsternoBloccate;
extern uint8 bSuperView;
extern uint8 bSezioneLabirinto;
extern uint8 bPlayerSuBasamento;
extern uint8 bWideScreen;
extern uint8 bTitoliCodaStatic;
extern uint8 bTitoliCodaScrolling;
extern uint8 bDontPlaySteps;
extern uint8 bShowRoomDescriptions;
extern uint8 bShowExtraLocalizationStrings;

// DIALOG VARS
extern uint8 UsedDlgMenu[MAX_PLAYERS][T3D_MAX_CHARACTERS][MAX_DLG_MENUS];

struct Globals {
	InvVars _invVars;
};

// INIT
struct Init : public SerializableAsset {
	SerializableArray<SRoom, MAX_ROOMS> Room;
	SerializableArray<SObject, MAX_OBJS> Obj;
	SerializableArray<SInvObject, MAX_ICONS> InvObj;
	SerializableArray<SSound, MAX_SOUNDS> Sound;
	SerializableArray<SMusic, MAX_MUSICS> Music;
	SerializableArray<SAnim, MAX_ANIMS> Anim;
	SerializableArray<SDialog, MAX_DIALOGS> Dialog;
	SerializableArray<SDiary, MAX_DIARIES> Diary;
	SerializableArray<SDlgMenu, MAX_DLG_MENUS> DlgMenu;
	SerializableArray<SDlgItem, MAX_DLG_ITEMS> DlgItem;

	// Sentences?
	SerializableArray<SPDALog, MAX_PDALOGS> PDALog;

	SerializableDynamicArray<SCreditsName> _creditsNames;
	SerializableDynamicArray<SCreditsRole> _creditsRoles;

	Globals _globals;

	void loadFromStream(Common::SeekableReadStream &stream) {
		warning("Init");
		Room.loadFromStream(stream);
		Obj.loadFromStream(stream);
		InvObj.loadFromStream(stream);
		Sound.loadFromStream(stream);
		Music.loadFromStream(stream);
		Anim.loadFromStream(stream);
		Dialog.loadFromStream(stream);
		Diary.loadFromStream(stream);
		DlgMenu.loadFromStream(stream);
		DlgItem.loadFromStream(stream);

		PDALog.loadFromStream(stream);

		uint16 numNames = stream.readUint16LE();
		uint16 numRoles = stream.readUint16LE();
		_creditsNames = SerializableDynamicArray<SCreditsName>(numNames);
		_creditsRoles = SerializableDynamicArray<SCreditsRole>(numRoles);
		_creditsNames.loadFromStream(stream);
		_creditsRoles.loadFromStream(stream);

		treatCredits();
	}
private:
	void treatCredits() {
		for (uint32 i = 0; i < _creditsNames.size(); i++) {
			char *c = _creditsNames[i].name;
			for (int k = 0; k < 32; k++, c++) {
				(*c) ^= 0xA0;
			}
		}

		for (uint32 i = 0; i < _creditsRoles.size(); i++) {
			char *c = _creditsRoles[i].role;
			for (int k = 0; k < 48; k++, c++) {
				(*c) ^= 0xC4;
			}
		}
	}
};

} // End of namespace Watchmaker

#endif // WATCHMAKER_GLOBVAR_H
