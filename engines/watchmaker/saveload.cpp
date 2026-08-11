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

#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy

#include "watchmaker/saveload.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/t3d_mesh.h"
#include "watchmaker/classes/do_inv.h"
#include "watchmaker/classes/do_player.h"
#include "watchmaker/classes/do_sound.h"
#include "watchmaker/classes/do_system.h"
#include "watchmaker/define.h"
#include "watchmaker/globvar.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/ll/ll_string.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/schedule.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/windows_hacks.h"

#define WM_SAVEFILE_VERSION     0x17 // Don't know what changed though.

namespace Watchmaker {

char *TextPtr;

/* -----------------03/08/00 17.45-------------------
 *              ReplaceSaveLoadTexture
 * --------------------------------------------------*/
void ReplaceSaveLoadTexture(int32 ci) {
	/*  char str[255];
	    gTexture *t;
	    t3dMESH *m;

	    if( ( ci <= i00SAVEPOS01 ) || ( !InvObj[ci].meshlink[0] ) || ( ( m=LinkMeshToStr( InvObj[ci].meshlink ) ) == nullptr ) )
	        return;
	    if( ( !m->FList ) || ( !m->FList->mat ) || ( !m->FList->mat->Texture ) )
	        return ;

	    rReleaseBitmapDirect( m->FList->mat->Texture  );

	    sprintf( str, "WmSav%#02d.tga", ci-i00SAVEPOS01+1 );
	    t = (gTexture *)rLoadBitmapImage( str, (t3dU8)(rTEXTURESURFACE) );
	    m->FList->mat->Texture = t;
	    */
}

/* -----------------23/08/00 11.12-------------------
 *                  getNextSent
 * --------------------------------------------------*/
char *getNextSent() {
	while (*TextPtr) {
		*TextPtr = ~(*TextPtr);
		TextPtr++;
	}

	TextPtr++;
	return ((char *)(TextPtr));
}

void loadTextBucket(Common::SeekableReadStream &stream) {
	extern char *TextBucket;

	if (!TextBucket) TextBucket = (char *)calloc(1, TEXT_BUCKET_SIZE);

	stream.read(TextBucket, TEXT_BUCKET_SIZE);

	TextPtr = TextBucket;

	for (int a = 0; a < MAX_OBJ_NAMES; a++) {
		ObjName[a] = getNextSent();
	}
	for (int a = 0; a < MAX_SENTENCES; a++) {
		Sentence[a] = getNextSent();
	}
	for (int a = 0; a < MAX_SYS_SENTS; a++) {
		SysSent[a] = getNextSent();
	}
	for (int a = 0; a < MAX_TOOLTIP_SENTS; a++) {
		TooltipSent[a] = getNextSent();
	}
}

void loadAll(WorkDirs &workDirs, Init &init) {
	// TODO: Cwd
	auto stream = openFile(workDirs._gameDir +  WM_INIT_PACK_FILENAME);
	assert(stream);
	init.loadFromStream(*stream);

	loadTextBucket(*stream);
}

/* -----------------28/06/00 10.12-------------------
 *                      DataSave
 * --------------------------------------------------*/
// NOTA: se slot==255 forza il save (salva senza controllare le condizioni)
bool DataSave(const char *SaveName, uint8 slot) {
	warning("STUBBED: DataSave");
#if 0
	char str[T3D_NAMELEN];
	FILE *fhs;
	int32 i, j;
	uint16 WmVer;

	if (slot != 255) {
		if (!PlayerCanSave())   return FALSE;

		if (
		    (bPlayerInAnim && (bT2DActive != tOPTIONS))
		    || bUseWith || bDialogActive || bDialogMenuActive || bMovingCamera || bGolfActive || InvStatus
		)
			return FALSE;
	}

	CharStop_SaveStatus();
	bPauseAllAnims = TRUE;
	for (i = 0; i < T3D_MAX_CHARACTERS; i++)
		if (Character[i])
			CharStop(i);

	InitMessageSystem();
	ClearUseWith();
	ClearText();

	if (bSomeOneSpeak) bSkipTalk = TRUE;

	PlayerPos[CurPlayer + ocDARRELL] = 0;
	PlayerGotoPos[CurPlayer + ocDARRELL] = 0;
	game._messageSystem.removeEvent(EventClass::MC_PLAYER, ME_ALL);
	CharStop(ocCURPLAYER);

//	nascondo l'altro giocatore, cos� quando riparte il gioco StartDiary() vede che e' nascosto e controlla se deve essere visualizzato
//	(altrimenti quando carico e l'altro player non � HIDE, per via di alcune variabili puo' non essere disegnato)
	Character[(CurPlayer ^ 1) + ocDARRELL]->Flags |= T3D_CHARACTER_HIDE;

	//sprintf( str, "WmSav%#02d.tga", slot );
	//rGrabVideo( str, 1 );
	/*  memcpy( BackupInv, &Inv[CurPlayer], sizeof(t3dU8)*MAX_ICONS_IN_INV );
	    BackupInvLen = InvLen[CurPlayer];
	    BackupInvBase = InvBase[CurPlayer];

	    memset( &Inv[CurPlayer], 0, sizeof(t3dU8)*MAX_ICONS_IN_INV );
	    for ( i=0; i<MAX_SAVE_SLOTS; i++ ) Inv[CurPlayer][i] = i00SAVEPOS01+i;
	    InvLen[CurPlayer] = MAX_SAVE_SLOTS;
	    InvBase[CurPlayer] = 0;

	    InvStatus = INV_ON|INV_MODE2|INV_MODE5;
	*/
	/*
	#ifdef INTERNAL_TEST_VERSION
	    {//DEBUG: si backuppa fino a 20 files
	        char _str[T3D_NAMELEN];
	        int i;

	        for(i=20; i>=1; i--)
	        {
	            sprintf( str, "WmSav%#02d.%#03d", slot,i-1 );
	            sprintf( _str, "WmSav%#02d.%#03d", slot,i );

	            remove(_str);
	            rename(str,_str);
	        }

	        sprintf( str, "Wm%#02d.sav", slot );
	        sprintf( _str, "WmSav%#02d.%#03d", slot,0 );
	        remove(_str);
	        rename(str,_str);

	    }
	#endif
	*/
	snprintf(str, T3D_NAMELEN, "%sWm%#02d.sav", WmSavesDir, slot);

	if ((fhs = fopen(str, "wb")) == nullptr)
		return FALSE;

	WmVer = (t3dU16)WM_SAVEFILE_VERSION;
	fwrite(&WmVer,              sizeof(t3dU16),        1,              fhs);
	fwrite(SaveName,                sizeof(char),          T3D_NAMELEN,    fhs);
	fwrite(t3dCurRoom->Name,        sizeof(char),          T3D_NAMELEN,    fhs);

	for (i = 0; i < MAX_ROOMS; i++) {
		fwrite(&Room[i].flags,          sizeof(t3dU16),        1,              fhs);
	}
	for (i = 0; i < MAX_OBJS; i++) {
		fwrite(&Obj[i].name,            sizeof(t3dU16),        1,              fhs);
		fwrite(Obj[i].examine,          sizeof(t3dU16),        MAX_PLAYERS,    fhs);
		fwrite(Obj[i].action,           sizeof(t3dU16),        MAX_PLAYERS,    fhs);
		fwrite(Obj[i].anim,         sizeof(t3dU16),        MAX_PLAYERS,    fhs);
		fwrite(Obj[i].anim2,            sizeof(t3dU16),        MAX_PLAYERS,    fhs);
		fwrite(&Obj[i].goroom,          sizeof(t3dU8),        1,              fhs);
		fwrite(&Obj[i].ninv,            sizeof(t3dU8),        1,              fhs);
		fwrite(&Obj[i].flags,           sizeof(t3dU16),        1,              fhs);
		fwrite(&Obj[i].pos,         sizeof(t3dU8),        1,              fhs);
	}
	for (i = 0; i < MAX_ICONS; i++) {
		fwrite(&InvObj[i].name,     sizeof(t3dU16),        1,              fhs);
		fwrite(InvObj[i].examine,       sizeof(t3dU16),        MAX_PLAYERS,    fhs);
		fwrite(InvObj[i].action,        sizeof(t3dU16),        MAX_PLAYERS,    fhs);
		fwrite(&InvObj[i].flags,        sizeof(t3dU16),        1,              fhs);
		fwrite(&InvObj[i].uwobj,        sizeof(t3dU16),        1,              fhs);
		fwrite(InvObj[i].anim,          sizeof(t3dU16),        MAX_PLAYERS,    fhs);
		fwrite(InvObj[i].anim2,     sizeof(t3dU16),        MAX_PLAYERS,    fhs);
	}
	for (i = 0; i < MAX_SOUNDS; i++) {
		fwrite(&Sound[i].flags,         sizeof(t3dU8),        1,              fhs);
	}
	for (i = 0; i < MAX_DIALOGS; i++) {
		fwrite(&Dialog[i].flags,        sizeof(t3dU16),        1,              fhs);
	}
	for (i = 0; i < MAX_DIARIES; i++) {
		fwrite(&Diary[i].startt,        sizeof(t3dU16),        1,              fhs);
		fwrite(&Diary[i].endt,          sizeof(t3dU16),        1,              fhs);
		fwrite(&Diary[i].end_hideobj,   sizeof(t3dU16),        1,              fhs);
	}
	for (i = 0; i < MAX_DLG_MENUS; i++) {
		fwrite(&DlgMenu[i].on,          sizeof(t3dU8),        1,              fhs);
	}
	for (i = 0; i < MAX_PDALOGS; i++) {
		fwrite(&PDALog[i].time,     sizeof(t3dS32),        1,              fhs);
		fwrite(&PDALog[i].flags,        sizeof(t3dS32),        1,              fhs);
	}
	fwrite(&CurRoom,                sizeof(t3dS32),        1,              fhs);
	fwrite(&CurInvObj,              sizeof(t3dS32),        1,              fhs);
	fwrite(&BigInvObj,              sizeof(t3dS32),        1,              fhs);
	fwrite(&CurPlayer,              sizeof(t3dS32),        1,              fhs);
	fwrite(&CurSubMusic,            sizeof(t3dS32),        1,              fhs);
	fwrite(Inv,                     sizeof(t3dU8),        MAX_PLAYERS * MAX_ICONS_IN_INV,   fhs);
	fwrite(InvLen,                  sizeof(t3dU8),        MAX_PLAYERS,    fhs);
	fwrite(InvBase,             sizeof(t3dU8),        MAX_PLAYERS,    fhs);
	fwrite(PlayerPos,               sizeof(t3dU8),        T3D_MAX_CHARACTERS, fhs);
	fwrite(&CameraTargetObj,        sizeof(t3dS32),        1,              fhs);
	fwrite(&CameraTargetBone,       sizeof(t3dS32),        1,              fhs);

	fwrite(Comb2D,                  sizeof(t3dS32),        5,              fhs);
	fwrite(Comb2Q,                  sizeof(t3dS32),        5,              fhs);
	fwrite(Comb19,                  sizeof(t3dS32),        3,              fhs);
	fwrite(Comb1D,                  sizeof(t3dS32),        5,              fhs);
	fwrite(&Forno25,                sizeof(t3dS32),        1,              fhs);
	fwrite(&Frigo25,                sizeof(t3dS32),        1,              fhs);
	fwrite(Comb31,                  sizeof(t3dS32),        5,              fhs);
	fwrite(Comb33,                  sizeof(t3dS32),        4,              fhs);
	fwrite(Comb42,                  sizeof(t3dS32),        12,             fhs);
	fwrite(Comb44,                  sizeof(t3dS32),        3,              fhs);
	fwrite(Comb45,                  sizeof(t3dS32),        5,              fhs);
	fwrite(&bMoglieGym,         sizeof(t3dU8),     1,              fhs);
	fwrite(&bMoglieSangue,          sizeof(t3dU8),     1,              fhs);
	fwrite(&bNoPlayerSwitch,        sizeof(t3dU8),     1,              fhs);
	fwrite(&bPorteEsternoBloccate,  sizeof(t3dU8),     1,              fhs);
	fwrite(&bSezioneLabirinto,      sizeof(t3dU8),     1,              fhs);

	for (i = 0; i < MAX_MODIFIED_MESH; i++) {
		fwrite(&MMList[i],          sizeof(struct SMeshModifier),      1,      fhs);
	}
	fwrite(PlayerStand,             sizeof(struct SPlayerStand),   MAX_PLAYERS,    fhs);
	fwrite(&t3dCurTime,             sizeof(t3dS32),        1,              fhs);

	for (i = 0; i < T3D_MAX_CHARACTERS; i++) {
		if ((i == ocCURPLAYER) || (i == ocLASTCHAR)) continue;
		if (Character[i] && Character[i]->Mesh) {
			fwrite(&Character[i]->Mesh->Trasl,      sizeof(t3dV3F),        1,      fhs);
			fwrite(&Character[i]->Dir,              sizeof(t3dV3F),        1,      fhs);
			fwrite(&Character[i]->Flags,            sizeof(t3dU8),        1,      fhs);
			fwrite(&Character[i]->Walk.LookX,       sizeof(t3dF32),        1,      fhs);
			fwrite(&Character[i]->Walk.LookZ,       sizeof(t3dF32),        1,      fhs);
			fwrite(&Character[i]->Walk.CurX,        sizeof(t3dF32),        1,      fhs);
			fwrite(&Character[i]->Walk.CurZ,        sizeof(t3dF32),        1,      fhs);
			fwrite(&Character[i]->Walk.CurPanel,    sizeof(t3dS16),        1,      fhs);
			fwrite(&Character[i]->Walk.OldPanel,    sizeof(t3dS16),        1,      fhs);
		} else {
			//evito che i save non dipendano dal numero di personaggi presenti in memoria in quel momento
			BYTE x, k, len;
			len = sizeof(t3dV3F) * 2 + sizeof(t3dU8) + sizeof(t3dF32) * 4 + sizeof(t3dS16) * 2;
			for (k = 0, x = 0; k < len; k++) fwrite(&x,    1,  1,  fhs);
		}
	}

	for (i = 0; i < MAX_PLAYERS; i++) {
		for (j = 0; j < T3D_MAX_CHARACTERS; j++) {
			fwrite(&UsedDlgMenu[i][j],      sizeof(t3dU8),     MAX_DLG_MENUS,              fhs);
		}
	}

	fclose(fhs);
	bPauseAllAnims = FALSE;
	CharStop_LoadStatus();
#endif
	return (TRUE);
}

/* -----------------28/06/00 14.49-------------------
 *                  DataLoad
 * --------------------------------------------------*/
//se SaveName e' "" utilizzo lo slot
bool DataLoad(WGame &game, const Common::String &FileName, uint8 slot) {
	char str[T3D_NAMELEN] = {};
	uint16 WmVer = 0;

	if (!FileName.equalsIgnoreCase("WmStart.dat"))
		if (
		    (bPlayerInAnim && (bT2DActive != tOPTIONS))
		    || bUseWith || bDialogActive || bDialogMenuActive || bMovingCamera || bGolfActive || InvStatus
		)
			return FALSE;

//	resetto alcune variabili che potrebbero rimanere sporche dalla partita prima (uguale per StartPlayingGame(), DataLoad() )
	bCacciatore = 0;
	bSaveDisabled = 0;
	bNotSkippableSent = 0;
	bPorteEsternoBloccate = 0;
	bNoPlayerSwitch = 0;
	bDarkScreen = FALSE;
	bSuperView = 0;
	bSezioneLabirinto = 0;
	bSkipTalk = FALSE;
	bSomeOneSpeak = FALSE;
	bPlayerSpeak = FALSE;
	bWideScreen = 0;
	bTitoliCodaStatic = 0;
	bTitoliCodaScrolling = 0;
	strcpy(RoomInfo.name, "");

	StopDiary(game, 0, 0, 0);
	bPauseAllAnims = TRUE;

	for (int i = 0; i < T3D_MAX_CHARACTERS; i++)
		if (Character[i])
			CharStop(i);

	game._messageSystem.init();
	ClearUseWith();
	ClearText();

	/*  FreeKey();

	    mleft = mright = 0;
	    Mouse(3);
	    while( mleft || mright )
	        Mouse(3);

	    // ferma omino, animazioni, spegne scritte
	    memcpy( OldInv, Inventario, MAXICON );
	    memset( Inventario, 0, MAXICON );

	    OldIconBase = TheIconBase;
	    TheIconBase = 0;
	    OldInvLen = InventarioLen;
	    InventarioLen = MAXSAVEFILE;
	*/

	if (!FileName.empty())
		strcpy(str, FileName.c_str());
	else
		snprintf(str, T3D_NAMELEN, "%sWm%02d.sav", game.workDirs._savesDir.c_str(), slot);

	auto stream = openFile(FileName);
	if (!stream) {
		return false;
	}
	Init &init = game.init;

	WmVer = stream->readUint16LE();
	if (WmVer != WM_SAVEFILE_VERSION) {
		DebugLogFile("!!Invalid SaveGame Version: found %d, required %d", WmVer, WM_SAVEFILE_VERSION);
		return false;
	}
	stream->read(str, T3D_NAMELEN);
	stream->read(str, T3D_NAMELEN);

	for (int i = 0; i < MAX_ROOMS; i++) {
		init.Room[i].flags = stream->readUint16LE();
	}
	for (int i = 0; i < MAX_OBJS; i++) {
		init.Obj[i].name = stream->readUint16LE();
		init.Obj[i].examine.loadFromStream(*stream);
		init.Obj[i].action.loadFromStream(*stream);
		init.Obj[i].anim.loadFromStream(*stream);
		init.Obj[i].anim2.loadFromStream(*stream);
		init.Obj[i].goroom = stream->readByte();
		init.Obj[i].ninv = stream->readByte();
		init.Obj[i].flags = stream->readUint16LE();
		init.Obj[i].pos = stream->readByte();
	}
	// HACK? (Something is off between 0.92 and the saves in the retail, leading to a slight skew.
	stream->seek(92, SEEK_CUR);
	for (int i = 0; i < MAX_ICONS; i++) {
		init.InvObj[i].name = stream->readUint16LE();
		init.InvObj[i].examine.loadFromStream(*stream);
		init.InvObj[i].action.loadFromStream(*stream);
		init.InvObj[i].flags = stream->readUint16LE();
		init.InvObj[i].uwobj = stream->readUint16LE();
		init.InvObj[i].anim.loadFromStream(*stream);
		init.InvObj[i].anim2.loadFromStream(*stream);
	}
	for (int i = 0; i < MAX_SOUNDS; i++) {
		init.Sound[i].flags = stream->readByte();
	}
	for (int i = 0; i < MAX_DIALOGS; i++) {
		init.Dialog[i].flags = stream->readUint16LE();
	}
	for (int i = 0; i < MAX_DIARIES; i++) {
		init.Diary[i].startt = stream->readUint16LE();
		init.Diary[i].endt = stream->readUint16LE();
		init.Diary[i].end_hideobj = stream->readUint16LE();
		init.Diary[i].cur = 0;
		for (int j = 0; j < MAX_DIARY_ITEMS; j++) {
			init.Diary[i].item[j].on = FALSE;
			init.Diary[i].item[j].cur = 0;
		}
	}
	for (int i = 0; i < MAX_DLG_MENUS; i++) {
		init.DlgMenu[i].on = stream->readByte();
	}
	for (int i = 0; i < MAX_PDALOGS; i++) {
		init.PDALog[i].time = stream->readSint32LE();
		init.PDALog[i].flags = stream->readSint32LE();
	}
	game._gameVars.setCurRoomId(stream->readSint32LE());
	CurInvObj = stream->readSint32LE();
	BigInvObj = stream->readSint32LE();
	CurPlayer = stream->readSint32LE();
	CurSubMusic = stream->readSint32LE();
	for (int player = 0; player < MAX_PLAYERS; player++) {
		for (int icon = 0; icon < MAX_ICONS_IN_INV; icon++) {
			Inv[player][icon] = stream->readByte();
		}
	}
	for (int i = 0; i < MAX_PLAYERS; i++) {
		InvLen[i] = stream->readByte();
	}
	for (int i = 0; i < MAX_PLAYERS; i++) {
		InvBase[i] = stream->readByte();
	}
	for (int i = 0; i < T3D_MAX_CHARACTERS; i++) {
		PlayerPos[i] = stream->readByte();
	}
	CameraTargetObj = stream->readSint32LE();
	CameraTargetBone = stream->readSint32LE();

	for (int i = 0; i < 5; i++) {
		Comb2D[i] = stream->readSint32LE();
	}
	for (int i = 0; i < 5; i++) {
		Comb2Q[i] = stream->readSint32LE();
	}
	for (int i = 0; i < 3; i++) {
		Comb19[i] = stream->readSint32LE();
	}
	for (int i = 0; i < 5; i++) {
		Comb1D[i] = stream->readSint32LE();
	}
	Forno25 = stream->readSint32LE();
	Frigo25 = stream->readSint32LE();
	for (int i = 0; i < 5; i++) {
		Comb31[i] = stream->readSint32LE();
	}
	for (int i = 0; i < 4; i++) {
		Comb33[i] = stream->readSint32LE();
	}
	for (int i = 0; i < 12; i++) {
		Comb42[i] = stream->readSint32LE();
	}
	for (int i = 0; i < 3; i++) {
		Comb44[i] = stream->readSint32LE();
	}
	for (int i = 0; i < 5; i++) {
		Comb45[i] = stream->readSint32LE();
	}
	bMoglieGym = stream->readByte();
	bMoglieSangue = stream->readByte();
	bNoPlayerSwitch = stream->readByte();
	bPorteEsternoBloccate = stream->readByte();
	bSezioneLabirinto = stream->readByte();

	_vm->loadMeshModifiers(*stream);

	for (int i = 0; i < MAX_PLAYERS; i++) {
		PlayerStand[i] = SPlayerStand(*stream);
	}
	t3dCurTime = stream->readSint32LE();

	for (int i = 0; i < T3D_MAX_CHARACTERS; i++) {
		if ((i == ocCURPLAYER) || (i == ocLASTCHAR)) continue;
		if (Character[i] && Character[i]->Mesh) {
			Character[i]->Mesh->Trasl = t3dV3F(*stream);
			Character[i]->Dir = t3dV3F(*stream);
			Character[i]->Flags = stream->readByte();
			Character[i]->Walk.Look = PointXZ::readFromStream(*stream);
			Character[i]->Walk.Cur = PointXZ::readFromStream(*stream);
			Character[i]->Walk.CurPanel = stream->readSint16LE();
			Character[i]->Walk.OldPanel = stream->readSint16LE();
		} else {
			// I avoid that the saves do not depend on the number of characters present in memory at that moment
			//len = sizeof(t3dV3F) * 2 + sizeof(t3dU8) + sizeof(t3dF32) * 4 + sizeof(t3dS16) * 2;
			t3dV3F unused(*stream);
			t3dV3F unused2(*stream);
			stream->readByte();
			stream->readFloatLE();
			stream->readFloatLE();
			stream->readFloatLE();
			stream->readFloatLE();
			stream->readSint16LE();
			stream->readSint16LE();
		}
	}

	for (int i = 0; i < MAX_PLAYERS; i++) {
		for (int j = 0; j < T3D_MAX_CHARACTERS; j++) {
			for (int k = 0; k < MAX_DLG_MENUS; k++) {
				UsedDlgMenu[i][j][k] = stream->readByte();
			}
		}
	}

	// fclose(fhs);

//	t3dVectCopy( &tmp, &Character[ocDARRELL]->Mesh->Trasl );
//	t3dVectCopy( &tmp2, &Character[ocVICTORIA]->Mesh->Trasl );

	StopAllAnims(init);
	StopSounds();
	StopMusic();
	t3dResetPipeline();
	_vm->_roomManager->releaseLoadedFiles(T3D_STATIC_SET1);
	t3dRxt = nullptr;
	t3dSky = nullptr;
	rReleaseAllTextures(T3D_STATIC_SET0);
	LoaderFlags |= T3D_STATIC_SET0;
	rSetLoaderFlags(LoaderFlags);
	t3dCurRoom = nullptr;
	bPauseAllAnims = FALSE;

	ChangeRoom(game, str, 0, 0);

//	evito che i personaggi rimangano a mezz'aria quando il gioco si stoppa con i personaggi in dubbie posizioni
	Character[ocCURPLAYER]->Mesh->Flags |= T3D_MESH_DEFAULTANIM;
//	resetto solo il current player, visto che l'altro deve avere l'anim di stand
	if (CurPlayer == DARRELL)
		Character[ocDARRELL]->Mesh->Flags |= T3D_MESH_DEFAULTANIM;
	else
		Character[ocVICTORIA]->Mesh->Flags |= T3D_MESH_DEFAULTANIM;

	t3dResetMesh(LinkMeshToStr(init, "darrell"));
	t3dResetMesh(LinkMeshToStr(init, "victoria"));
//	t3dVectCopy( &Character[ocDARRELL]->Mesh->Trasl, &tmp );
//	t3dVectCopy( &Character[ocDARRELL]->Pos, &tmp );
//	t3dVectCopy( &Character[ocVICTORIA]->Mesh->Trasl, &tmp2 );
//	t3dVectCopy( &Character[ocVICTORIA]->Pos, &tmp2 );

//	CharSetPosition( ocDARRELL, 0, NULL );
//	CharSetPosition( ocVICTORIA, 0, NULL );

	return true;
}

} // End of namespace Watchmaker
