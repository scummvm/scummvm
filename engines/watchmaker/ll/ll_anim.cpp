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

#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/ll/ll_sound.h"
#include "watchmaker/ll/ll_string.h"
#include "watchmaker/3d/animation.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/classes/do_camera.h"
#include "watchmaker/classes/do_dialog.h"
#include "watchmaker/classes/do_inv.h"
#include "watchmaker/classes/do_operate.h"
#include "watchmaker/classes/do_player.h"
#include "watchmaker/classes/do_sound.h"
#include "watchmaker/classes/do_string.h"
#include "watchmaker/classes/do_system.h"
#include "watchmaker/define.h"
#include "watchmaker/game.h"
#include "watchmaker/globvar.h"
#include "watchmaker/ll/ll_mesh.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/main.h"
#include "watchmaker/message.h"
#include "watchmaker/schedule.h"
#include "watchmaker/types.h"
#include "watchmaker/utils.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/walk/ball.h"
#include "watchmaker/walk/walk.h"
#include "watchmaker/windows_hacks.h"

// locals
#define MAX_ACTIVE_ANIMS            16
#define MAX_ATF_STACK               10              // stack di indici per ATF

namespace Watchmaker {

uint16 ATFStack[MAX_ACTIVE_ANIMS][MAX_ATF_STACK], CurATFStack[MAX_ACTIVE_ANIMS];

struct SSubAnim {
	int16 LastFrame;
	uint16 flags;
	t3dMESH *ptr;
};

struct SActiveAnim {
	struct SSubAnim sub[MAX_SUBANIMS];
	int32 index;
	int32 obj;
	int16 CurFrame, LastFrame;
	uint16 flags;
	uint16 LoopStart, LoopEnd, LoopMask;
} ActiveAnim[MAX_ACTIVE_ANIMS];
int32 CurActiveAnim = 0;

int32 FirstFrame, LastFrame, CurFrame, CurFlags;

char PushedRoomName[T3D_NAMELEN];
uint8 PushedRoomPos;

// Wide Screen Effect
extern int32 WideScreen_Y;
extern uint32 WideScreen_StartTime;
extern uint32 WideScreen_EndTime;

/* -----------------01/06/98 11.57-------------------
 *                  PopATF
 * --------------------------------------------------*/
uint16 PopATF() {
	uint16 v, a;

	if (CurATFStack[CurActiveAnim] > 0)
		CurATFStack[CurActiveAnim] --;
	else
		DebugLogFile("ATF Stack vuoto | AN %d, CF %d", ActiveAnim[CurActiveAnim].index, ActiveAnim[CurActiveAnim].CurFrame);

	v = ATFStack[CurActiveAnim][0];

	for (a = 0; a < MAX_ATF_STACK - 1; a++)
		ATFStack[CurActiveAnim][a] = ATFStack[CurActiveAnim][a + 1];

	return v;
}

/* -----------------01/06/98 11.58-------------------
 *                  PushATF
 * --------------------------------------------------*/
void PushATF(uint16 v) {
	ATFStack[CurActiveAnim][CurATFStack[CurActiveAnim]++] = v;

	if (CurATFStack[CurActiveAnim] >= MAX_ATF_STACK)
		DebugLogFile("ATF Stack pieno");
}

/* -----------------26/11/98 12.19-------------------
 *                  ProcessATFDo
 * --------------------------------------------------*/
void ProcessATFDO(WGame &game, int32 in) {
	t3dMESH *m;
	int32 i;
	Init &init = game.init;

	switch (in) {
	case fFADIN:
		_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND, 0, EFFECT_FADIN, nullptr, nullptr, nullptr);
		break;
	case fFADOUT:
		_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND, 0, EFFECT_FADOUT, nullptr, nullptr, nullptr);
		break;
	case fFADOUTWAIT2:
		_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND, FRAME_PER_SECOND * 2, EFFECT_FADOUT, nullptr, nullptr, nullptr);
		break;
	case fFADOUTWAIT5:
		_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND, FRAME_PER_SECOND * 5, EFFECT_FADOUT, nullptr, nullptr, nullptr);
		break;
	case fPUSH_ROOM:
		strcpy(PushedRoomName, t3dCurRoom->name.c_str());
		PushedRoomPos = PlayerPos[ocDARRELL + CurPlayer];
		break;
	case fPOP_ROOM:
		ChangeRoom(game, PushedRoomName, PushedRoomPos, 0);
		CharSetPosition(ocCURPLAYER, 91, nullptr);
		Character[CurPlayer + ocDARRELL]->Flags &= ~T3D_CHARACTER_HIDE;
		break;
	case fPOP_ROOM_dR015:
		ChangeRoom(game, PushedRoomName, PushedRoomPos, 0);
		Character[CurPlayer + ocDARRELL]->Flags &= ~T3D_CHARACTER_HIDE;
		_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR015, 0, 0, nullptr, nullptr, nullptr);
		break;
	case fSET_MOGLIE_GYM:
		i = ocMOGLIESUPERVISORE;
		if (!Character[i]) break;
		CharStop(i);
		/*          t3dReleaseCharacter( Character[i] );
		            Character[i] = nullptr;

		            for ( j=0; j<NumLoadedFiles; j++ )
		            {
		                if ( !strcasecmp( LoadedFiles[j].Name, "MoglieSwim.t3d" ) )
		                {
		                    t3dReleaseBody( LoadedFiles[j].b );
		                    LoadedFiles[j].b = nullptr;
		                }
		            }
		*/          bMoglieGym = 1;

		game.CheckAndLoadMoglieSupervisoreModel(i);

		/*          LoaderFlags |= T3D_PRELOADBASE;
		            LoaderFlags |= T3D_STATIC_SET1;
		            LoaderFlags |= T3D_STATIC_SET0;
		            rSetLoaderFlags( LoaderFlags );
		            if( !(Character[i] = t3dLoadCharacter( "MoglieGym.t3d", Character[i], (t3dU16)i ) ) )
		                DebugFile( "Error loading MoglieGym" );
		            LoaderFlags &= ~T3D_PRELOADBASE;
		            LoaderFlags &= ~T3D_STATIC_SET1;
		            LoaderFlags &= ~T3D_STATIC_SET0;
		            rSetLoaderFlags( LoaderFlags );*/
		break;
	case fSET_MOGLIE_SANGUE:
		i = ocMOGLIE_KIMONO;
		if (!Character[i]) break;
		CharStop(i);
		/*          t3dReleaseCharacter( Character[i] );
		            Character[i] = nullptr;

		            for ( j=0; j<NumLoadedFiles; j++ )
		            {
		                if ( !strcasecmp( LoadedFiles[j].Name, "MoglieKimono.t3d" ) )
		                {
		                    t3dReleaseBody( LoadedFiles[j].b );
		                    LoadedFiles[j].b = nullptr;
		                }
		            }
		*/          bMoglieSangue = 1;

		game.CheckAndLoadMoglieSupervisoreModel(i);

		/*          LoaderFlags |= T3D_PRELOADBASE;
		            LoaderFlags |= T3D_STATIC_SET1;
		            LoaderFlags |= T3D_STATIC_SET0;
		            rSetLoaderFlags( LoaderFlags );
		            if( !(Character[i] = t3dLoadCharacter( "MoglieKimonoSangue.t3d", Character[i], (t3dU16)i ) ) )
		                DebugFile( "Error loading MoglieSangue" );
		            LoaderFlags &= ~T3D_PRELOADBASE;
		            LoaderFlags &= ~T3D_STATIC_SET1;
		            LoaderFlags &= ~T3D_STATIC_SET0;
		            rSetLoaderFlags( LoaderFlags );*/
		break;
	case fSET_PLAYERSWITCH_ON:
		bNoPlayerSwitch = 0;
		break;
	case fSET_PLAYERSWITCH_OFF:
		bNoPlayerSwitch = 1;
		break;
	case fUSCITAPOZZO:
		if (IconInInv(init, i1dAUDIOCASSETTA) || IconInInv(init, i28WALKMANNASTRO) || IconInInv(init, i28WALKMANOK))
			_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR1a4, 0, 0, NULL, NULL, NULL);
		break;
	case f22STOPWAVES:
		if ((m = LinkMeshToStr(init, "p22-water01"))) ChangeMeshFlags(m, -1, T3D_MESH_WAVESTEXTURE);
		if ((m = LinkMeshToStr(init, "p22-fondopiscina"))) ChangeMeshFlags(m, -1, T3D_MESH_WAVESTEXTURE);
		if ((m = LinkMeshToStr(init, "p22-latopiscina01"))) ChangeMeshFlags(m, -1, T3D_MESH_WAVESTEXTURE);
		if ((m = LinkMeshToStr(init, "p22-latopiscina02"))) ChangeMeshFlags(m, -1, T3D_MESH_WAVESTEXTURE);
		if ((m = LinkMeshToStr(init, "p22-latopiscina03"))) ChangeMeshFlags(m, -1, T3D_MESH_WAVESTEXTURE);
		if ((m = LinkMeshToStr(init, "p22-latopiscina04"))) ChangeMeshFlags(m, -1, T3D_MESH_WAVESTEXTURE);
		if ((m = LinkMeshToStr(init, "p22-a-piscindown01"))) ChangeMeshFlags(m, -1, T3D_MESH_WAVESTEXTURE);

		if ((m = LinkMeshToStr(init, "pxt-_p22-water01"))) ChangeMeshFlags(m, -1, T3D_MESH_WAVESTEXTURE);
		if ((m = LinkMeshToStr(init, "pxt-_p22-fondopiscina"))) ChangeMeshFlags(m, -1, T3D_MESH_WAVESTEXTURE);
		if ((m = LinkMeshToStr(init, "pxt-_p22-latopiscina01"))) ChangeMeshFlags(m, -1, T3D_MESH_WAVESTEXTURE);
		if ((m = LinkMeshToStr(init, "pxt-_p22-latopiscina02"))) ChangeMeshFlags(m, -1, T3D_MESH_WAVESTEXTURE);
		if ((m = LinkMeshToStr(init, "pxt-_p22-latopiscina03"))) ChangeMeshFlags(m, -1, T3D_MESH_WAVESTEXTURE);
		break;
	case f23ENTRALIFT:
		in = aTO1PERSON;
		bLockCamera = true;
		CheckCharacterWithoutBounds(game, ocCURPLAYER, (const uint8 *)"\055\000", 0);
		_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERGOTO, MP_WAIT_ANIM, 0, 0, bFirstPerson, nullptr, &in, nullptr);
		bNoFirstPersonSwitch = true;
		bNoPlayerSwitch = true;
		break;
	case f23ESCELIFT:
		bLockCamera = false;
		CheckCharacterWithoutBounds(game, ocCURPLAYER, (const uint8 *)"\057\000", 0);
		_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERGOTO, MP_DEFAULT, 0, 0, bFirstPerson, nullptr, nullptr, nullptr);
		bNoFirstPersonSwitch = false;
		bNoPlayerSwitch = false;
		break;
	case f25CHECK:
		if (IconInInv(init, i25FIALABOK) && IconInInv(init, i25FIALAAOK)) {
			ReplaceIcon(init, i25FIALAAOK, i25FIALEABPIENE);
			KillIcon(init, i25FIALABOK);
		}
		break;
	case f25CHECKFORNO: {
		if (init.Obj[o25FIALABFORNO].flags & ON) {
			if (Forno25 == 8) {
				init.Obj[o25FIALABFORNO].flags &= ~ON;
				init.Obj[o25FIALABFORNOCALDA].flags |= ON;
			} else {
				init.Obj[o25FIALABFORNO].flags |= ON;
				init.Obj[o25FIALABFORNOCALDA].flags &= ~ON;
			}

			UpdateObjMesh(init, o25FIALABFORNO);
			UpdateObjMesh(init, o25FIALABFORNOCALDA);
		}
	}
	break;
	case f2OENTRALIFT:
		in = aTO1PERSON;
		bLockCamera = true;
		CheckCharacterWithoutBounds(game, ocCURPLAYER, (const uint8 *)"\051\052\000", 0);
		_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERGOTO, MP_WAIT_ANIM, 0, 0, bFirstPerson, nullptr, &in, nullptr);
		bNoFirstPersonSwitch = true;
		bNoPlayerSwitch = true;
		break;
	case f2OESCELIFT:
		bLockCamera = false;
		CheckCharacterWithoutBounds(game, ocCURPLAYER, (const uint8 *)"\054\000", 0);
		_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERGOTO, MP_DEFAULT, 0, 0, bFirstPerson, nullptr, nullptr, nullptr);
		bNoFirstPersonSwitch = false;
		bNoPlayerSwitch = false;
		break;
	case f2QENTRALAB:
		CheckCharacterWithoutBounds(game, ocCURPLAYER, (const uint8 *)"\051\052\000", 0);
		_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERGOTO, MP_WAIT_ANIM, 0, 0, bFirstPerson, nullptr, nullptr, nullptr);
		_vm->_messageSystem.doEvent(EventClass::MC_ANIM, ME_STARTANIM, MP_WAIT_PORTAL, a2Q14, 0, 0, nullptr, nullptr, nullptr);
		break;
	case f2QESCELAB:
		CheckCharacterWithoutBounds(game, ocCURPLAYER, (const uint8 *)"\051\052\000", 0);
		_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERGOTO, MP_WAIT_ANIM, 0, 0, bFirstPerson, nullptr, nullptr, nullptr);
		_vm->_messageSystem.doEvent(EventClass::MC_ANIM, ME_STARTANIM, MP_WAIT_PORTAL, a2Q14, 0, 0, nullptr, nullptr, nullptr);
		break;

	case f36MSD:
	case f36MSS:
	case f36PSD:
	case f36PSS:
	case f36MID:
	case f36MIS:
	case f36PID:
	case f36PIS:
		break;
	case fCALLOTHERPLAYER_START: {
		uint8 cindex;

		CurPlayer ^= 1;
		SetCurPlayerPosTo_9x(game._gameVars, init);
		CurPlayer ^= 1;

		CharStop((CurPlayer ^ 1) + ocDARRELL);

		Character[(CurPlayer ^ 1) + ocDARRELL]->Flags &= ~T3D_CHARACTER_HIDE;

		if (CurPlayer == DARRELL)
			StartAnim(game, aVICTORIA_ASCOLTA);
		else
			StartAnim(game, aDARRELL_ASCOLTA);

		cindex = game._cameraMan->GetCameraIndexUnderPlayer(ocDARRELL + (CurPlayer ^ 1));
		if (cindex != 255) {
			// equivale ad un "setcamera:"
			ForcedCamera = GetAlternateCamera(init, (uint8)(cindex + 1));
			bCutCamera = true;
			bAllowCalcCamera = false;

			// equivale ad un "settarget:"
			CameraTargetObj = ocDARRELL + (CurPlayer ^ 1);
			CameraTargetBone = 0;
		}
	}
	break;
	case fCALLOTHERPLAYER_END: {
		Character[(CurPlayer ^ 1) + ocDARRELL]->Flags |= T3D_CHARACTER_HIDE;

		CurPlayer ^= 1;
		UpdatePlayerStand(game, (uint8)(CurPlayer + ocDARRELL));
		CurPlayer ^= 1;
	}
	break;
	case fLAUNCHGAMEOVERMUSIC: {
		if (game.gameOptions.music_on && (!(LoaderFlags & T3D_NOMUSIC))) {
			if (mLoadMusic(".\\Midi\\GameOver.mid")) mPlayMusic(".\\Midi\\GameOver.mid");
		}
	}
	break;
	case fLAUNCHCREDITSMUSIC: {
		if (game.gameOptions.music_on && (!(LoaderFlags & T3D_NOMUSIC))) {
			if (mLoadMusic(".\\Midi\\Coda.mid")) mPlayMusic(".\\Midi\\Coda.mid");
		}
	}
	break;
	case fSTART_MAIN_MENU: {
#if 0
		StopPlayingGame(game);
#endif
		_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, 0, 0, tMAINMENU, nullptr, nullptr, nullptr);
	}
	break;
	case f35LIGHTS: {

		/*              t3dBODY *t;
		                t3dS32 i;

		                t = t3dCurRoom;
		                for( i=0; i<(t3dS32)t3dCurRoom->NumMeshes; i++ )
		                    if( ( t3dCurRoom->MeshTable[i].PortalList != NULL ) )
		                        if( strcasecmp( t3dCurRoom->MeshTable[i].PortalName, "r35" ) == 0 )
		                            t = t3dCurRoom->MeshTable[i].PortalList;

		                if( t->LightTable->Type & T3D_LIGHT_ALLLIGHTSOFF )
		                    t3dTurnOnAllLights( t );
		                else
		                    t3dTurnOffAllLights( t );*/
	}
	break;

	case fFORCE_DEFAULT_ANIM: {
		Player->Mesh->Flags |= T3D_MESH_DEFAULTANIM;
	}
	break;

	case f34CHECK:
		if ((init.Obj[o34FACCIABIANCA].flags & ON) && (init.Obj[o34FACCIANERA].flags & ON)) {
			init.Obj[o34SFERA].flags &= ~ON;
			init.Obj[o34SFERASU].flags |= ON;
		}
		break;

	case f3BCHECK:
		if ((init.Obj[o3BSESTERZOBRACCIALI].flags & ON) &&
		        (init.Obj[o3B2DOSIBRACCIALI].flags & ON) && (init.Obj[o3B2DOSI].flags & ON)) {
			init.Obj[o3BSESTERZOBRACCIALI].flags &= ~ ON;
			init.Obj[o3BSESTERZOBRACCIALI].flags |= NOUPDATE;
			init.Obj[o3B2DOSIBRACCIALI].flags &= ~ ON;
			init.Obj[o3B2DOSIBRACCIALI].flags |= NOUPDATE;
			init.Obj[o3B2DOSI].flags &= ~ ON;
			init.Obj[o3B2DOSI].flags |= NOUPDATE;
			init.Obj[o3BCOPPA].flags &= ~ ON;
			init.Obj[o3BCOPPABRACCIALI].flags &= ~ ON;
			init.Obj[o3BCOPPA].flags |= EXTRA;
			StartAnim(game, a3B8);
		}
		break;

	case f41WALK:
		CheckCharacterWithoutBounds(game, ocCURPLAYER, (const uint8 *)"\064\000", 0);
		_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERGOTO, MP_WAIT_ANIM, 0, 0, bFirstPerson, nullptr, nullptr, nullptr);
		_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_WAIT_ACT, dR411, 0, 0, nullptr, nullptr, nullptr);
		break;
		break;

	case f44UPDATE:
		UpdateSpecial(game, r44);
		if (!(init.Obj[o43CANCELLO01].flags & EXTRA) && (Comb44[0] == 4) && (Comb44[1] == 3) && (Comb44[2] == 6)) {
			init.Obj[o43CANCELLO01].flags &= ~ON;
			init.Obj[o43CANCELLO02].flags &= ~ON;
			init.Obj[o43CANCELLOALZATO03].flags &= ~ON;
			init.Obj[o43CANCELLOALZATO01].flags |= ON;
			init.Obj[o43CANCELLOALZATO02].flags |= ON;
			init.Obj[o43CANCELLO03].flags |= ON;
			UpdateObjMesh(init, o43CANCELLO01);
			UpdateObjMesh(init, o43CANCELLO02);
			UpdateObjMesh(init, o43CANCELLO03);
			UpdateObjMesh(init, o43CANCELLOALZATO01);
			UpdateObjMesh(init, o43CANCELLOALZATO02);
			UpdateObjMesh(init, o43CANCELLOALZATO03);

			init.Obj[o42LEVAMURATA].anim[VICTORIA] = a428;
			init.Obj[o43CANCELLO01].flags |= EXTRA;

			if (!(init.Obj[o43CANCELLO01].flags & EXTRA2)) {
				IncCurTime(game, 15);
				init.Obj[o43CANCELLO01].flags |= EXTRA2;
			}

			SetBndLevel(game, "r43.t3d", 1);
			StartSound(game, w428B);
		}
		break;

	case f45STARTTIMER:
		i = TheTime + 1000 * 20;
		_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_PLAYERTIMER, MP_WAIT_RETRACE, (int16)(CurPlayer + ocDARRELL), (int16)dR452, 0, &i, nullptr, nullptr);
//			Event( EventClass::MC_CAMERA, ME_CAMERA3TO1, MP_DEFAULT, 0, 0, 0, NULL, NULL, NULL );  //preferisco farlo rimanere in terza (ndFox)
		break;
	case f45STOPTIMER:
		game._messageSystem.removeEvent(EventClass::MC_SYSTEM, ME_PLAYERTIMER);
		break;

	case f41STARTTIMER:
		i = TheTime + 1000 * 20;
		_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_PLAYERTIMER, MP_WAIT_RETRACE, (int16)(CurPlayer + ocDARRELL), (int16)dR0010, 0, &i, nullptr, nullptr);
		break;
	case f41STOPTIMER:
		game._messageSystem.removeEvent(EventClass::MC_SYSTEM, ME_PLAYERTIMER);
		break;

	case fDISABLESAVE:
		bSaveDisabled = 1;
		break;
	case fENABLESAVE:
		bSaveDisabled = 0;
		break;

	case fSAVEPARTICULAR:
		//add automatic save
		break;

	case f49CHECK:
		if ((init.Obj[o49AMPOLLAAZZURRO].anim[CurPlayer] == aNULL) && (init.Obj[o49AMPOLLAVERDE].anim[CurPlayer] == aNULL) && (init.Obj[o49AMPOLLAGIALLO].anim[CurPlayer] == aNULL))
			_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR491, 0, 0, nullptr, nullptr, nullptr);
		break;

	case f4ASETBND44:
		SetBndLevel(game, "r44.t3d", t3dCurRoom->CurLevel);

		if (!(init.Obj[o4ALEVA].flags & EXTRA2)) {
			IncCurTime(game, 15);
			init.Obj[o4ALEVA].flags |= EXTRA2;
		}
		break;
	case fSTART_CACCIATORE:
		bCacciatore = true;
		break;
	case fSTOP_CACCIATORE:
		bCacciatore = false;
		break;

	case f42RESETCOMBINATION:
		for (i = 0; i < 12; i++) {
			Comb42[i] = 0;

			init.Obj[o42TASTO01SU + i].flags |= ON;
			UpdateObjMesh(init, o42TASTO01SU + i);

			init.Obj[o42TASTO01SU + i + 12].flags &= ~ON;
			UpdateObjMesh(init, o42TASTO01SU + i + 12);
		}

		break;

	/*  //Non piu' usati
	        case fUSE_DARRELL:
	            CurPlayer = DARRELL;
	            Character[ocCURPLAYER] = Character[ocDARRELL];
	            Player = Character[ocCURPLAYER];
	            Player->Flags &= ~T3D_CHARACTER_HIDE;
	            break;
	        case fUSE_VICTORIA:
	            CurPlayer = VICTORIA;
	            Character[ocCURPLAYER] = Character[ocVICTORIA];
	            Player = Character[ocCURPLAYER];
	            Player->Flags &= ~T3D_CHARACTER_HIDE;
	            break;*/

	case fPASSA_ICONE:
		for (i = 0; i < MAX_ICONS_IN_INV; i++) {
			if ((Inv[DARRELL][i]) && (Inv[DARRELL][i] != i00TELEFONO))
				Inv[VICTORIA][InvLen[VICTORIA]++] = Inv[DARRELL][i];

			Inv[DARRELL][i] = iNULL;
		}

		InvLen[DARRELL] = 1;
		Inv[DARRELL][0] = i00TELEFONO;

		InvBase[VICTORIA] = InvBase[DARRELL] = 0;

		bSezioneLabirinto = 1;
		break;

	case fGG_USER1:
		Freccia50->Flags &= ~T3D_CHARACTER_HIDE;
		break;
	case fGG_USER2:
		Ball[CurGopher].Tb = 1.0f;
		ChangePhys(&Ball[CurGopher], 0);
		Ball[CurGopher].TimeLeft -= 20.0f;
		bGolfMode = 2;
		break;

	case f47SETPDALOGS:
		if (
		    (init.PDALog[lPDA3_TITLE11_CORONA].flags & PDA_ON) ||
		    (init.PDALog[lPDA3_TITLE12_DUKES].flags & PDA_ON) ||
		    (init.PDALog[lPDA3_TITLE13_KRENN].flags & PDA_ON) ||
		    (init.PDALog[lPDA3_TITLE14_VALENCIA].flags & PDA_ON)
		) {
			if (!(init.PDALog[lPDA3_MENU15_CONCLUSIONS_ITEM1].flags & PDA_ON)) {
				init.PDALog[lPDA3_MENU15_CONCLUSIONS_ITEM1].flags |= (PDA_ON | PDA_UPDATE);
				init.PDALog[lPDA3_MENU15_CONCLUSIONS_ITEM1].time = t3dCurTime;
				_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND * 3, 0, EFFECT_DISPLAY_NEWLOGIMG, nullptr, nullptr, nullptr);
			}
			if (!(init.PDALog[lPDA3_MENU15_CONCLUSIONS_ITEM2].flags & PDA_ON)) {
				init.PDALog[lPDA3_MENU15_CONCLUSIONS_ITEM2].flags |= (PDA_ON | PDA_UPDATE);
				init.PDALog[lPDA3_MENU15_CONCLUSIONS_ITEM2].time = t3dCurTime;
				_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND * 3, 0, EFFECT_DISPLAY_NEWLOGIMG, nullptr, nullptr, nullptr);
			}
			if (!(init.PDALog[lPDA3_MENU15_CONCLUSIONS_ITEM3].flags & PDA_ON)) {
				init.PDALog[lPDA3_MENU15_CONCLUSIONS_ITEM3].flags |= (PDA_ON | PDA_UPDATE);
				init.PDALog[lPDA3_MENU15_CONCLUSIONS_ITEM3].time = t3dCurTime;
				_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND * 3, 0, EFFECT_DISPLAY_NEWLOGIMG, nullptr, nullptr, nullptr);
			}
			if (!(init.PDALog[lPDA3_MENU15_CONCLUSIONS_ITEM4].flags & PDA_ON)) {
				init.PDALog[lPDA3_MENU15_CONCLUSIONS_ITEM4].flags |= (PDA_ON | PDA_UPDATE);
				init.PDALog[lPDA3_MENU15_CONCLUSIONS_ITEM4].time = t3dCurTime;
				_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND * 3, 0, EFFECT_DISPLAY_NEWLOGIMG, nullptr, nullptr, nullptr);
			}
		}
		break;

	case fdR222CHECK: {
		//          nel caso in cui l'altro giocatore sia nella r2g lo faccio uscire
		if (PlayerStand[CurPlayer ^ 1].roomName.equalsIgnoreCase("r2g.t3d")) {
			if (CurPlayer == DARRELL)
				CharSetPosition((CurPlayer ^ 1) + ocDARRELL, 94, "r2h.t3d");
			else
				CharSetPosition((CurPlayer ^ 1) + ocDARRELL, 93, "r2h.t3d");
			CharStop((CurPlayer ^ 1) + ocDARRELL);
			Character[(CurPlayer ^ 1) + ocDARRELL]->Flags &= ~T3D_CHARACTER_HIDE;
			CurPlayer ^= 1;
			UpdatePlayerStand(game, (uint8)(CurPlayer + ocDARRELL));
			CurPlayer ^= 1;
		}
	}
	break;

	case fCPSPEAK_PORTACHIUSA1: {
		//"It seems to be locked."
		if (!bAnimWaitText && PlayerSpeak(game, init.Obj[o24PORTATRAMEZZOA].action[CurPlayer])) bAnimWaitText = true;
		break;
	}
	break;
	case fCPSPEAK_PORTACHIUSA2: {
		//"It's locked."
		if (!bAnimWaitText && PlayerSpeak(game, init.Obj[o29PORTA].action[CurPlayer])) bAnimWaitText = true;
		break;
	}
	break;

	case fSET_BLOCCO_PORTE_ESTERNO: {
		bPorteEsternoBloccate = 1;

		init.Obj[oXT1Ap22].flags |= DONE;
		init.Obj[oXT1Ap22].flags &= ~DOOR;

		init.Obj[oXT11p21].flags |= DONE;
		init.Obj[oXT11p21].flags &= ~DOOR;

		init.Obj[oXT18p25].flags |= DONE;
		init.Obj[oXT18p25].flags &= ~DOOR;

		init.Obj[oXT14p3C].flags |= DONE;
		init.Obj[oXT14p3C].flags &= ~DOOR;

		init.Obj[oXT1APOZZO].flags |= DONE;
		init.Obj[oXT1APOZZO].flags &= ~DOOR;
	}
	break;
	case fCLR_BLOCCO_PORTE_ESTERNO: {
		bPorteEsternoBloccate = 0;

		init.Obj[oXT1Ap22].flags &= ~DONE;
		init.Obj[oXT1Ap22].flags |= DOOR;

		init.Obj[oXT11p21].flags &= ~DONE;
		init.Obj[oXT11p21].flags |= DOOR;

		init.Obj[oXT18p25].flags &= ~DONE;
		init.Obj[oXT18p25].flags |= DOOR;

		init.Obj[oXT14p3C].flags &= ~DONE;
		init.Obj[oXT14p3C].flags |= DOOR;

		init.Obj[oXT1APOZZO].flags &= ~DONE;
		init.Obj[oXT1APOZZO].flags |= DOOR;
	}
	break;

	case fSTART_TITOLI_CODA: {
		TitoliCoda_ShowStatic(game, 1);
//				TitoliCoda_ShowScrolling( 1 );
	}
	break;

//		talvolta capita che la moglie non parta piu' quando si risale dalla piscina, percio' ho aggiunto questi due case per sicurezza
	case fSTART_DISCESAPISCINA: {
		if (Character[ocMOGLIESUPERVISORE])
			Character[ocMOGLIESUPERVISORE]->Flags |= T3D_CHARACTER_DIARYDISABLE;
		StopDiary(game, r22, ocMOGLIESUPERVISORE, 0);
	}
	break;
	case fEND_DISCESAPISCINA: {
		if (Character[ocMOGLIESUPERVISORE])
			Character[ocMOGLIESUPERVISORE]->Flags &= ~T3D_CHARACTER_DIARYDISABLE;
		StartDiary(game, r22, nullptr);
	}
	break;
	case f48_STOPSOUNDS_ENDTRO: {
		StopAnim(game, a48PENDOLO);
	}
	break;
	}
}


/* -----------------13/05/98 17.01-------------------
 *                  ProcessATF
 * --------------------------------------------------*/
void ProcessATF(WGame &game, int32 an, int32 atf) {
	Init &init = game.init;
	struct SAnim *ha = &init.Anim[an];
	struct SActiveAnim *h;
	int32 obj, in, a;

	if (!an || !ha || !ha->active) return;
	h = &ActiveAnim[ha->active - 1];
	obj = h->obj;
	in = ha->atframe[atf].index;
//	if( h->sub[0].ptr )
//		DebugLogFile("ATF: %d (%d,%d)",ha->atframe[atf].type,h->CurFrame,h->sub[0].ptr->CurFrame);
	switch (ha->atframe[atf].type) {
	case ATF_DO:
		ProcessATFDO(game, in);
		break;
	case ATF_PUSH:
		PushATF((uint16)(in));
		break;
	case ATF_POP:
		PopATF();
		break;
	case ATF_JUMP:
		for (a = 0, obj = PopATF(); a < MAX_SUBANIMS; a++)
			if ((h->sub[a].ptr != nullptr) && (h->sub[a].ptr->CurFrame >= 0) && !(obj & (1 << a)))
				h->sub[a].ptr->CurFrame = in * 3;
		break;
	case ATF_JUMP_ALL:
		for (a = 0; a < MAX_SUBANIMS; a++)
			if ((h->sub[a].ptr != nullptr) && (h->sub[a].ptr->CurFrame >= 0))
				h->sub[a].ptr->CurFrame = in * 3;
		h->CurFrame = in * 3;
		break;
	case ATF_LOOP_LEN:
		h->LoopStart = h->CurFrame;
		h->LoopEnd = h->LoopStart + in * 3;
		break;
	case ATF_LOOP_END:
		h->LoopStart = h->CurFrame;
		h->LoopEnd = in * 3;
		break;
	case ATF_LOOP_MASK:
		h->LoopMask = in;
		break;
	case ATF_TEXT:
		if (!bAnimWaitText && PlayerSpeak(game, in)) bAnimWaitText = true;
		break;
	case ATF_POP_TEXT:
		if (!bAnimWaitText && PlayerSpeak(game, PopATF())) bAnimWaitText = true;
		break;
	case ATF_CHANGE_ROOM:
		ChangeRoom(game, (char *) ha->RoomName.rawArray(), (uint8)in, aNULL);
		break;
	case ATF_CHANGE_ROOM_AN:
		ChangeRoom(game, (char *) ha->RoomName.rawArray(), (uint8)in, PopATF());
		break;
	case ATF_SET_CAMERA:
		ForcedCamera = in;
		break;
	case ATF_SET_BND_LEVEL:
		SetBndLevel(game, (char *) ha->RoomName.rawArray(), in);
		break;
	case ATF_TO_1ST_SENT:
		ToFirstPersonSent = (in >= TEXT1) ? init.Obj[obj].text[in - TEXT1] : init.Obj[obj].action[CurPlayer];
		break;
	case ATF_TO_1ST_ANIM:
		// Does nothing
		break;
	case ATF_FROM_1ST_ANIM:
		FromFirstPersonAnim = in;
		break;
	case ATF_CUROBJ:
		CurObj = in;
		break;
	case ATF_CHANGE_PLAYER:
		ChangePlayer(game, (uint8)in);
		break;
	case ATF_NO_INPUT:
		bPlayerInAnim = in;
		break;
	case ATF_UPDATE_BBOX:
		UpdateBoundingBox(h->sub[in - 1].ptr);
		break;
	case ATF_RESET_BLEND:
		if (h->sub[in - 1].ptr) h->sub[in - 1].ptr->BlendPercent = 255;
		break;
	case ATF_CAMERA_TARGET:
		CameraTargetObj = in;
		break;
	case ATF_UPDATE_MAX_TARGET:
		game._cameraMan->GetCameraTarget(init, &t3dCurCamera->MaxTarget);
		break;
	case ATF_SUB_MUSIC:
		CurSubMusic = in;
		UpdateRoomVisibility(game);
		break;
	case ATF_EXIT:
		StopAnim(game, an);
		break;
	case ATF_LINK:
		for (a = 0, obj = h->LoopMask; a < MAX_SUBANIMS; a++)
			if (!bAnimWaitText && h->sub[a].ptr && (h->sub[a].ptr->CurFrame >= 0) && !(obj & (1 << a)))
				if ((h->sub[a].ptr->Flags & T3D_MESH_CHARACTER) && (bDialogActive)) {
					_vm->_messageSystem.addWaitingMsgs(MP_WAIT_LINK);
					h->sub[a].ptr->CurFrame = h->LoopEnd;
					h->sub[a].ptr->BlendPercent = 0;
					h->CurFrame = h->LoopEnd;
					h->LoopEnd = 0;
				}
		break;

	case ATFO_OFF:
		init.Obj[in].flags &= ~ON;
		break;
	case ATFO_ON:
		init.Obj[in].flags |= ON;
		break;
	case ATFO_HIDE:
		init.Obj[in].flags |= HIDE;
		break;
	case ATFO_UNHIDE:
		init.Obj[in].flags &= ~HIDE;
		break;
	case ATFOM_OFF:
		init.Obj[in].flags &= ~ON;
		UpdateObjMesh(init, in);
		break;
	case ATFOM_ON:
		init.Obj[in].flags |= ON;
		UpdateObjMesh(init, in);
		break;
	case ATFOM_HIDE:
		init.Obj[in].flags |= HIDE;
		UpdateObjMesh(init, in);
		break;
	case ATFOM_UNHIDE:
		init.Obj[in].flags &= ~HIDE;
		UpdateObjMesh(init, in);
		break;
	case ATFD_OFF:
		_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGEND, MP_DEFAULT, (int16)in, mQUIT, 0, nullptr, nullptr, nullptr);
		break;
	case ATFD_ON:
		_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_WAIT_ANIM, (int16)in, 0, 0, nullptr, nullptr, nullptr);
		break;
	case ATFI_OFF:
		KillIcon(init, (uint8)in);
		break;
	case ATFI_ON:
		AddIcon(init, (uint8)in);
		break;
	case ATFA_OFF:
		StopAnim(game, (int32)in);
		break;
	case ATFA_ON:
		StartAnim(game, (int32)in);
		break;
	case ATFA_OFF_POP:
		StopAnim(game, (int32)PopATF());
		break;
	case ATFA_ON_POP:
		StartAnim(game, (int32)PopATF());
		break;
	case ATFA_PAUSE:
		PauseAnim(init, (int32)in);
		break;
	case ATFA_CONTINUE:
		ContinueAnim(init, (uint8)in);
		break;
	case ATFS_SET_FLAG:
		init.Sound[PopATF()].flags |= in;
		break;
	case ATFS_CLR_FLAG:
		init.Sound[PopATF()].flags &= ~in;
		break;


	case ATFP_OFF:
		ChangeMeshFlags(h->sub[in - 1].ptr, +1, T3D_MESH_NOPORTALCHECK);
		UpdateRoomVisibility(game);
		break;
	case ATFP_ON:
		ChangeMeshFlags(h->sub[in - 1].ptr, -1, T3D_MESH_NOPORTALCHECK);
		UpdateRoomVisibility(game);
		break;
	case ATFM_OFF:
		ChangeMeshFlags(h->sub[in - 1].ptr, +1, T3D_MESH_HIDDEN);
		break;
	case ATFM_ON:
		ChangeMeshFlags(h->sub[in - 1].ptr, -1, T3D_MESH_HIDDEN);
		break;
	case ATFM_OFF_POP:
		ChangeMeshFlags(h->sub[PopATF() - 1].ptr, +1, T3D_MESH_HIDDEN);
		break;
	case ATFM_ON_POP:
		ChangeMeshFlags(h->sub[PopATF() - 1].ptr, -1, T3D_MESH_HIDDEN);
		break;
	case ATFM_HIDE:
		ChangeMeshFlags(h->sub[in - 1].ptr, +1, T3D_MESH_NOBOUNDBOX);
		break;
	case ATFM_UNHIDE:
		ChangeMeshFlags(h->sub[in - 1].ptr, -1, T3D_MESH_NOBOUNDBOX);
		break;
	case ATFM_SET_AVFLAG:
		ChangeMeshFlags(h->sub[in - 1].ptr, +1, T3D_MESH_ALWAYSVISIBLE);
		break;
	case ATFM_CLR_AVFLAG:
		ChangeMeshFlags(h->sub[in - 1].ptr, -1, T3D_MESH_ALWAYSVISIBLE);
		break;
	case ATFM_SETFRAME:
		SetMeshMaterialMovieFrame(h->sub[in - 1].ptr, 0, (int32)PopATF());
		break;
	case ATFM_INCFRAME:
		SetMeshMaterialMovieFrame(h->sub[in - 1].ptr, +1, 1);
		break;
	case ATFM_DECFRAME:
		SetMeshMaterialMovieFrame(h->sub[in - 1].ptr, -1, 1);
		break;
	case ATFM_START_MOVIE:
		SetMeshMaterialMovieFrame(h->sub[in - 1].ptr, 0, 1);
		ChangeMeshMaterialFlag(h->sub[in - 1].ptr, -1, T3D_MATERIAL_MOVIEPAUSED);
		break;
	case ATFM_STOP_MOVIE:
		ChangeMeshMaterialFlag(h->sub[in - 1].ptr, +1, T3D_MATERIAL_MOVIEPAUSED);
		break;
	case ATFH_ON:
		ChangeHaloesStatus(nullptr, +1);
		break;
	case ATFH_OFF:
		ChangeHaloesStatus(nullptr, -1);
		break;

	case ATFC_HIDE:
		if (Character[in])  Character[in]->Flags |= T3D_CHARACTER_HIDE;
		break;
	case ATFC_UNHIDE:
		if (Character[in])  Character[in]->Flags &= ~T3D_CHARACTER_HIDE;
		break;
	case ATFC_SETTO:
		CharSetPosition((int32)PopATF(), (uint8)in, nullptr);
		break;
	case ATFC_GOTO:
		CharGotoPosition(game, PopATF(), (uint8)in, 0, 0);
		break;
	case ATFC_RUNTO:
		CharGotoPosition(game, PopATF(), (uint8)in, 1, 0);
		break;
	case ATFC_BACKTO:
		CharGotoPosition(game, PopATF(), (uint8)in, 2, 0);
		break;
	case ATFC_GOTO_AN:
		CharGotoPosition(game, PopATF(), (uint8)in, 0, PopATF());
		break;
	case ATFC_RUNTO_AN:
		CharGotoPosition(game, PopATF(), (uint8)in, 1, PopATF());
		break;
	case ATFC_BACKTO_AN:
		CharGotoPosition(game, PopATF(), (uint8)in, 2, PopATF());
		break;
	case ATFC_FIX_POS:
		FixPos(in);
		break;
	case ATFC_SET_ALT0:
	case ATFC_SET_ALT1:
	case ATFC_SET_ALT2:
	case ATFC_SET_ALT3:
		if (in == ocCURPLAYER) in = CurPlayer + ocDARRELL;
		CurAlternate[in] = ha->atframe[atf].type - ATFC_SET_ALT0;
		break;
	case ATFPL_SETTO:
		CharSetPosition(ocCURPLAYER, (uint8)in, nullptr);
		break;
	case ATFPL_GOTO:
		CharGotoPosition(game, ocCURPLAYER, (uint8)in, 0, 0);
		break;
	case ATFPL_RUNTO:
		CharGotoPosition(game, ocCURPLAYER, (uint8)in, 1, 0);
		break;
	case ATFPL_BACKTO:
		CharGotoPosition(game, ocCURPLAYER, (uint8)in, 2, 0);
		break;
	case ATFPL_GOTO_AN:
		CharGotoPosition(game, ocCURPLAYER, (uint8)in, 0, PopATF());
		break;
	case ATFPL_RUNTO_AN:
		CharGotoPosition(game, ocCURPLAYER, (uint8)in, 1, PopATF());
		break;
	case ATFPL_BACKTO_AN:
		CharGotoPosition(game, ocCURPLAYER, (uint8)in, 2, PopATF());
		break;
	case ATFDM_OFF:
		init.DlgMenu[in].on = 0;
		break;
	case ATFDM_ON:
		init.DlgMenu[in].on = 1;
		break;
	case ATFS_OFF:
		StopSound(in);
		break;
	case ATFS_ON:
		StartSound(game, in);
		break;
	case ATFS_PSX:
		if (h->sub[in - 1].ptr) StartStepSound(game, &h->sub[in - 1].ptr->Trasl, SOUND_PSX);
		break;
	case ATFS_PDX:
		if (h->sub[in - 1].ptr) StartStepSound(game, &h->sub[in - 1].ptr->Trasl, SOUND_PDX);
		break;

	case ATFO_TEX:
		if (!bAnimWaitText && PlayerSpeak(game, init.Obj[PopATF()].examine[CurPlayer])) bAnimWaitText = true;
		break;
	case ATFO_TEXOP:
		if (!bAnimWaitText && PlayerSpeak(game, init.Obj[PopATF()].examine[CurPlayer ^ 1])) bAnimWaitText = true;
		break;
	case ATFO_TACT:
		if (!bAnimWaitText && PlayerSpeak(game, init.Obj[PopATF()].action[CurPlayer])) bAnimWaitText = true;
		break;
	case ATFO_TACTOP:
		if (!bAnimWaitText && PlayerSpeak(game, init.Obj[PopATF()].action[CurPlayer ^ 1])) bAnimWaitText = true;
		break;
	case ATFO_TEXT:
		if (!bAnimWaitText && PlayerSpeak(game, init.Obj[PopATF()].text[in - TEXT1])) bAnimWaitText = true;
		break;
	case ATFO_CEX:
		obj = PopATF();
		init.Obj[obj].examine[CurPlayer] = (in >= TEXT1) ? init.Obj[obj].text[in - TEXT1] : in;
		break;
	case ATFO_CACT:
		obj = PopATF();
		init.Obj[obj].action[CurPlayer] = (in >= TEXT1) ? init.Obj[obj].text[in - TEXT1] : in;
		break;
	case ATFO_CANIM:
		obj = PopATF();
		init.Obj[obj].anim[CurPlayer] = init.Obj[obj].anim[CurPlayer ^ 1] = (in == ANIM2) ? init.Obj[obj].anim2[CurPlayer] : in;
		break;
	case ATFO_CANIM2:
		obj = PopATF();
		init.Obj[obj].anim2[CurPlayer] = init.Obj[obj].anim2[CurPlayer ^ 1] = in;
		break;
	case ATFO_CPOS:
		init.Obj[PopATF()].pos = in;
		break;
	case ATFO_SET_FLAG:
		init.Obj[PopATF()].flags |= in;
		break;
	case ATFO_CLR_FLAG:
		init.Obj[PopATF()].flags &= ~in;
		break;
	case ATFCO_TEX:
		if (!bAnimWaitText && PlayerSpeak(game, init.Obj[obj].examine[CurPlayer])) bAnimWaitText = true;
		break;
	case ATFCO_TACT:
		if (!bAnimWaitText && PlayerSpeak(game, init.Obj[obj].action[CurPlayer])) bAnimWaitText = true;
		break;
	case ATFCO_TEXT:
		if (!bAnimWaitText && PlayerSpeak(game, init.Obj[obj].text[in - TEXT1])) bAnimWaitText = true;
		break;
	case ATFCO_CEX:
		init.Obj[obj].examine[CurPlayer] = (in >= TEXT1) ? init.Obj[obj].text[in - TEXT1] : in;
		break;
	case ATFCO_CACT:
		init.Obj[obj].action[CurPlayer] = (in >= TEXT1) ? init.Obj[obj].text[in - TEXT1] : in;
		break;
	case ATFCO_CANIM:
		init.Obj[obj].anim[CurPlayer] = init.Obj[obj].anim[CurPlayer ^ 1] = (in == ANIM2) ? init.Obj[obj].anim2[CurPlayer] : in;
		break;
	case ATFCO_CANIM2:
		init.Obj[obj].anim2[CurPlayer] = init.Obj[obj].anim2[CurPlayer ^ 1] = in;
		break;
	case ATFCO_CPOS:
		init.Obj[obj].pos = in;
		break;
	case ATFCO_SET_FLAG:
		init.Obj[obj].flags |= in;
		break;
	case ATFCO_CLR_FLAG:
		init.Obj[obj].flags &= ~in;
		break;
	case ATFI_TEX:
		if (!bAnimWaitText && PlayerSpeak(game, init.InvObj[in].examine[CurPlayer])) bAnimWaitText = true;
		break;
	case ATFI_TACT:
		if (!bAnimWaitText && PlayerSpeak(game, init.InvObj[in].action[CurPlayer])) bAnimWaitText = true;
		break;
	case ATFI_TEXT:
		if (!bAnimWaitText && PlayerSpeak(game, init.InvObj[PopATF()].text[in - TEXT1])) bAnimWaitText = true;
		break;
	case ATFI_CEX:
		obj = PopATF();
		init.InvObj[obj].examine[CurPlayer] = (in >= TEXT1) ? init.InvObj[obj].text[in - TEXT1] : in;
		break;
	case ATFI_CACT:
		obj = PopATF();
		init.InvObj[obj].action[CurPlayer] = (in >= TEXT1) ? init.InvObj[obj].text[in - TEXT1] : in;
		break;
	case ATFI_CANIM:
		obj = PopATF();
		init.InvObj[obj].anim[CurPlayer] = init.InvObj[obj].anim[CurPlayer ^ 1] = (in == ANIM2) ? init.InvObj[obj].anim2[CurPlayer] : in;
		break;
	case ATFI_CANIM2:
		obj = PopATF();
		init.InvObj[obj].anim2[CurPlayer] = init.InvObj[obj].anim2[CurPlayer ^ 1] = in;
		break;
	case ATFI_CUWOBJ:
		init.InvObj[PopATF()].uwobj = in;
		break;
	case ATFI_SET_FLAG:
		init.InvObj[PopATF()].flags |= in;
		break;
	case ATFI_CLR_FLAG:
		init.InvObj[PopATF()].flags &= ~in;
		break;
	case ATFI_REPLACE:
		ReplaceIcon(init, (uint8)PopATF(), (uint8)in);
		break;
	case ATFSA_START:
		for (a = 0; a < MAX_SUBANIMS; a++) if (h->sub[a].ptr && !(in & (1 << a))) {
				h->sub[a].ptr->CurFrame = 0;
				h->sub[a].ptr->Flags &= ~T3D_MESH_DEFAULTANIM;
			}
		break;
	case ATFSA_BLOCK:
		if (in) h->sub[in - 1].flags |= ANIM_BLOCK;
		else h->flags |= ANIM_BLOCK;
		break;

	case ATF_PUSH_USED:
		PushATF((uint16)UseWith[USED]);
		break;
	case ATF_PUSH_WITH:
		PushATF((uint16)UseWith[WITH]);
		break;

	case ATF_GG_MODE:
		bGolfMode = in;
		break;
	case ATF_VISIBILITY:
		UpdateRoomVisibility(game);
		break;

	case ATF_INC_TIME:
		IncCurTime(game, in);
		break;
	case ATF_DEC_TIME:
		DecCurTime(game, in);
		break;
	case ATF_SET_TIME:
		SetCurTime(game, in);
		break;
	case ATF_SET_STARTT:
		init.Diary[in].startt = t3dCurTime;
		break;
	case ATF_SET_ENDT:
		init.Diary[in].endt = t3dCurTime;
		break;

	case ATF_START_T2D:
		_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, 0, 0, (uint8)in, nullptr, nullptr, nullptr);
		break;

	case ATFO_CANIMD:
		obj = PopATF();
		init.Obj[obj].anim[DARRELL] = (in == ANIM2) ? init.Obj[obj].anim2[DARRELL] : in;
		break;
	case ATFO_CANIMV:
		obj = PopATF();
		init.Obj[obj].anim[VICTORIA] = (in == ANIM2) ? init.Obj[obj].anim2[VICTORIA] : in;
		break;
	case ATFO_CANIM2D:
		obj = PopATF();
		init.Obj[obj].anim2[DARRELL] = in;
		break;
	case ATFO_CANIM2V:
		obj = PopATF();
		init.Obj[obj].anim2[VICTORIA] = in;
		break;
	case ATFCO_CANIMD:
		init.Obj[obj].anim[DARRELL] = (in == ANIM2) ? init.Obj[obj].anim2[DARRELL] : in;
		break;
	case ATFCO_CANIMV:
		init.Obj[obj].anim[VICTORIA] = (in == ANIM2) ? init.Obj[obj].anim2[VICTORIA] : in;
		break;
	case ATFCO_CANIM2D:
		init.Obj[obj].anim2[DARRELL] = in;
		break;
	case ATFCO_CANIM2V:
		init.Obj[obj].anim2[VICTORIA] = in;
		break;
	case ATFI_CANIMD:
		obj = PopATF();
		init.InvObj[obj].anim[DARRELL] = (in == ANIM2) ? init.InvObj[obj].anim2[DARRELL] : in;
		break;
	case ATFI_CANIMV:
		obj = PopATF();
		init.InvObj[obj].anim[VICTORIA] = (in == ANIM2) ? init.InvObj[obj].anim2[VICTORIA] : in;
		break;
	case ATFI_CANIM2D:
		obj = PopATF();
		init.InvObj[obj].anim2[DARRELL] = in;
		break;
	case ATFI_CANIM2V:
		obj = PopATF();
		init.InvObj[obj].anim2[VICTORIA] = in;
		break;
	case ATFO_CANIMCP:
		obj = PopATF();
		init.Obj[obj].anim[CurPlayer] = (in == ANIM2) ? init.Obj[obj].anim2[CurPlayer] : in;
		break;
	case ATFO_CANIMOP:
		obj = PopATF();
		init.Obj[obj].anim[(CurPlayer ^ 1)] = (in == ANIM2) ? init.Obj[obj].anim2[(CurPlayer ^ 1)] : in;
		break;
	case ATFO_CANIM2CP:
		obj = PopATF();
		init.Obj[obj].anim2[CurPlayer] = in;
		break;
	case ATFO_CANIM2OP:
		obj = PopATF();
		init.Obj[obj].anim2[(CurPlayer ^ 1)] = in;
		break;
	case ATFCO_CANIMCP:
		init.Obj[obj].anim[CurPlayer] = (in == ANIM2) ? init.Obj[obj].anim2[CurPlayer] : in;
		break;
	case ATFCO_CANIMOP:
		init.Obj[obj].anim[(CurPlayer ^ 1)] = (in == ANIM2) ? init.Obj[obj].anim2[(CurPlayer ^ 1)] : in;
		break;
	case ATFCO_CANIM2CP:
		init.Obj[obj].anim2[CurPlayer] = in;
		break;
	case ATFCO_CANIM2OP:
		init.Obj[obj].anim2[(CurPlayer ^ 1)] = in;
		break;
	case ATFI_CANIMCP:
		obj = PopATF();
		init.InvObj[obj].anim[CurPlayer] = (in == ANIM2) ? init.InvObj[obj].anim2[CurPlayer] : in;
		break;
	case ATFI_CANIMOP:
		obj = PopATF();
		init.InvObj[obj].anim[(CurPlayer ^ 1)] = (in == ANIM2) ? init.InvObj[obj].anim2[(CurPlayer ^ 1)] : in;
		break;
	case ATFI_CANIM2CP:
		obj = PopATF();
		init.InvObj[obj].anim2[CurPlayer] = in;
		break;
	case ATFI_CANIM2OP:
		obj = PopATF();
		init.InvObj[obj].anim2[(CurPlayer ^ 1)] = in;
		break;
	case ATFC_HIDE_BND:
		if (Character[in])  Character[in]->Flags |= T3D_CHARACTER_BNDHIDE;
		break;
	case ATFC_UNHIDE_BND:
		if (Character[in])  Character[in]->Flags &= ~T3D_CHARACTER_BNDHIDE;
		break;
	case ATFI_ON_DAR:
		obj = CurPlayer;
		CurPlayer = DARRELL;
		AddIcon(init, (uint8)in);
		CurPlayer = obj;
		break;
	case ATFI_ON_VIC:
		obj = CurPlayer;
		CurPlayer = VICTORIA;
		AddIcon(init, (uint8)in);
		CurPlayer = obj;
		break;
	case ATFO_CEX_DLG:
		obj = PopATF();
		if (init.Dialog[init.Obj[obj].goroom].flags & DIALOG_DONE) init.Obj[obj].examine[CurPlayer ^ 1] = init.Obj[obj].examine[CurPlayer] = (in >= TEXT1) ? init.Obj[obj].text[in - TEXT1] : in;
		break;
	case ATFO_CACT_DLG:
		obj = PopATF();
		if (init.Dialog[init.Obj[obj].goroom].flags & DIALOG_DONE) init.Obj[obj].action[CurPlayer ^ 1] = init.Obj[obj].action[CurPlayer] = (in >= TEXT1) ? init.Obj[obj].text[in - TEXT1] : in;
		break;
	case ATFO_CEX_BOTH:
		obj = PopATF();
		init.Obj[obj].examine[CurPlayer] = init.Obj[obj].examine[CurPlayer ^ 1] = (in >= TEXT1) ? init.Obj[obj].text[in - TEXT1] : in;
		break;
	case ATFO_CACT_BOTH:
		obj = PopATF();
		init.Obj[obj].action[CurPlayer] = init.Obj[obj].action[CurPlayer ^ 1] = (in >= TEXT1) ? init.Obj[obj].text[in - TEXT1] : in;
		break;
	case ATFCO_CEX_BOTH:
		init.Obj[obj].examine[CurPlayer] = init.Obj[obj].examine[CurPlayer ^ 1] = (in >= TEXT1) ? init.Obj[obj].text[in - TEXT1] : in;
		break;
	case ATFCO_CACT_BOTH:
		init.Obj[obj].action[CurPlayer] = init.Obj[obj].action[CurPlayer ^ 1] = (in >= TEXT1) ? init.Obj[obj].text[in - TEXT1] : in;
		break;
	case ATFI_CEX_BOTH:
		obj = PopATF();
		init.InvObj[obj].examine[CurPlayer] = init.InvObj[obj].examine[CurPlayer ^ 1] = (in >= TEXT1) ? init.InvObj[obj].text[in - TEXT1] : in;
		break;
	case ATFI_CACT_BOTH:
		obj = PopATF();
		init.InvObj[obj].action[CurPlayer] = init.InvObj[obj].action[CurPlayer ^ 1] = (in >= TEXT1) ? init.InvObj[obj].text[in - TEXT1] : in;
		break;
	case ATFD_ON_DLG:
		_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, (int16)in, 0, 0, nullptr, nullptr, nullptr);
		break;
	case ATFS_PSX_STAIRS:
		if (h->sub[in - 1].ptr) StartStepSound(game, &h->sub[in - 1].ptr->Trasl, SOUND_PSX | SOUND_STAIRS);
		break;
	case ATFS_PDX_STAIRS:
		if (h->sub[in - 1].ptr) StartStepSound(game, &h->sub[in - 1].ptr->Trasl, SOUND_PDX | SOUND_STAIRS);
		break;
	case ATFO_SET_NOUPDATE:
		init.Obj[in].flags |= NOUPDATE;
		break;
	case ATFO_CLR_NOUPDATE:
		init.Obj[in].flags &= ~NOUPDATE;
		UpdateObjMesh(init, in);
		break;
	case ATFC_DIARY_ENABLE:
		if (Character[in]) Character[in]->Flags &= ~T3D_CHARACTER_DIARYDISABLE;
		break;
	case ATFC_DIARY_DISABLE:
		if (Character[in]) Character[in]->Flags |= T3D_CHARACTER_DIARYDISABLE;
		break;
	case ATFL_ON:
		if (!(init.PDALog[in].flags & PDA_ON)) {
			init.PDALog[in].flags |= (PDA_ON | PDA_UPDATE);
			init.PDALog[in].time = t3dCurTime;
			_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND * 3, 0, EFFECT_DISPLAY_NEWLOGIMG, nullptr, nullptr, nullptr);
		}
		break;
	case ATFC_ENABLE_SHADOWS:
		if (Character[in]) Character[in]->Flags |= T3D_CHARACTER_CASTREALTIMESHADOWS;
		break;
	case ATFC_DISABLE_SHADOWS:
		if (Character[in]) Character[in]->Flags &= ~T3D_CHARACTER_CASTREALTIMESHADOWS;
		break;
	case ATFD_ON_WAITTEXT:
		_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_WAIT_LINK, (int16)in, 0, 0, nullptr, nullptr, nullptr);
		break;
	case ATF_SET_NOSKIP:
		bNotSkippableSent = in;
		break;
	case ATF_SET_BASAMENTO:
		bPlayerSuBasamento = in ? 1 : 0;
		break;
	case ATFC_ENABLE_VOLUMETRIC_SHADOWS:
		if (Character[in]) Character[in]->Flags |= T3D_CHARACTER_VOLUMETRICLIGHTING;
		break;
	case ATFC_DISABLE_VOLUMETRIC_SHADOWS:
		if (Character[in]) Character[in]->Flags &= ~T3D_CHARACTER_VOLUMETRICLIGHTING;
		break;
	case ATF_START_WIDESCREEN:
		WideScreen_Y = 0;
		WideScreen_StartTime = TheTime;
		WideScreen_EndTime = TheTime + (uint32)in;
		bWideScreen = 1;
		break;
	case ATF_END_WIDESCREEN:
		WideScreen_Y = 0;
		bWideScreen = 0;
		break;
	case ATF_DONT_PLAY_STEPS:
		bDontPlaySteps =  in ? 1 : 0;
		break;
	case ATFO_CEXD:
		obj = PopATF();
		init.Obj[obj].examine[DARRELL] = (in >= TEXT1) ? init.Obj[obj].text[in - TEXT1] : in;
		break;
	case ATFO_CEXV:
		obj = PopATF();
		init.Obj[obj].examine[VICTORIA] = (in >= TEXT1) ? init.Obj[obj].text[in - TEXT1] : in;
		break;

	}
}

/* -----------------13/05/98 15.42-------------------
 *                  AtFrameNext
 * --------------------------------------------------*/
void AtFrame(WGame &game, int32 an) {
	struct SAnim *ha = &game.init.Anim[an];
	struct SActiveAnim *h;
	int32 a, nf, ac;

	if (!an) return;
	h = &ActiveAnim[ha->active - 1];

	for (a = 0; a < MAX_ATFRAMES; a++) {
		if (ha->atframe[a].type == 0) continue;
		ac = ha->atframe[a].anim;
		nf = ha->atframe[a].nframe;
		if ((nf < 0) &&  !ac) ac = 1;
//		Esegue ATFrame solo se:
		if ((ac && h->sub[ac - 1].ptr && ((h->sub[ac - 1].ptr->CurFrame == nf) || (!nf && (h->sub[ac - 1].ptr->CurFrame < 0)) ||
		                                  (h->sub[ac - 1].ptr->Anim.NumFrames && (nf < 0) && (h->sub[ac - 1].ptr->CurFrame == h->sub[ac - 1].ptr->Anim.NumFrames - 1 + nf)))) ||
		        (!ac && !nf && (h->CurFrame < 0)) ||
		        (!ac && (nf == h->CurFrame)) ||
		        ((nf == 10000) && PortalCrossed))
			ProcessATF(game, an, a);
	}
}


/* -----------------13/05/98 16.09-------------------
 *                  StartAnim
 * --------------------------------------------------*/
void StartAnim(WGame &game, int32 an) {
	struct SActiveAnim *h;
	int32 a, b;
	uint32 st;
	int8 cp;
	Init &init = game.init;

	if (!an) return;
	if ((an == aIDLE_DARRELL_1) || (an == aIDLE_VICTORIA_1)) {
		if (TheTime % 2) an += 1;
	}
	DebugLogFile("StartAnim %d (%s)", an, init.Anim[an].name[0].rawArray());

	if ((an == aTO1PERSON) && (bFirstPerson == 0) && !(InvStatus & INV_ON)) {
		if (bMovingCamera)
			_vm->_messageSystem.doEvent(EventClass::MC_CAMERA, ME_CAMERA3TO1, MP_WAIT_CAMERA, 0, 0, init.Obj[CurObj].pos, nullptr, nullptr, nullptr);
		else
			_vm->_messageSystem.doEvent(EventClass::MC_CAMERA, ME_CAMERA3TO1, MP_DEFAULT, 0, 0, init.Obj[CurObj].pos, nullptr, nullptr, nullptr);
		return;
	} else if ((an == aTO3PERSON) && (bFirstPerson) && (!bMovingCamera) && !(InvStatus & INV_ON)) {
		_vm->_messageSystem.doEvent(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_WAIT_CAMERA, 0, 0, 0, nullptr, nullptr, nullptr);
		return;
	} else if (bFirstPerson && ((init.Anim[an].obj < ocDARRELL) || (init.Anim[an].obj > oCAMERAMAX))) {
		_vm->_messageSystem.doEvent(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
		_vm->_messageSystem.doEvent(EventClass::MC_ANIM, ME_STARTANIM, MP_WAIT_CAMERA, (int16)an, 0, 0, nullptr, nullptr, nullptr);
		return ;
	} else if ((bMovingCamera) && (!bDialogActive) && ((init.Anim[an].obj < ocDARRELL) || (init.Anim[an].obj > oCAMERAMAX))) {
		_vm->_messageSystem.doEvent(EventClass::MC_ANIM, ME_STARTANIM, MP_WAIT_CAMERA, (int16)an, 0, 0, nullptr, nullptr, nullptr);
		return ;
	}

//	Se la stava gia' playando quitta
	for (b = 0; b < MAX_ACTIVE_ANIMS; b++)
		if (ActiveAnim[b].index == an) return;

//	Se esiste gia' un'animazione sullo stesso oggetto base, la termina
	for (b = 0; b < MAX_ACTIVE_ANIMS; b++)
		if ((ActiveAnim[b].index) && (ActiveAnim[b].CurFrame >= 0) && !(ActiveAnim[b].flags & ANIM_PAUSED) &&
		        (Common::String((const char *)init.Anim[ActiveAnim[b].index].meshlink[0].rawArray()).equalsIgnoreCase((const char *)init.Anim[an].meshlink[0].rawArray())))
			StopAnim(game, ActiveAnim[b].index);

//	Se trova uno slot vuoto
	for (b = 0; b < MAX_ACTIVE_ANIMS; b++)
		if (ActiveAnim[b].index == aNULL) break;
//	Troppe animazioni contemporanee
	if (b >= MAX_ACTIVE_ANIMS) return ;

	CurActiveAnim = b;
	h = &ActiveAnim[b];
	CurATFStack[CurActiveAnim] = 0;
	memset(ATFStack[CurActiveAnim], 0, MAX_ATF_STACK * sizeof(uint16));

	mHide = 1;
	if ((init.Anim[an].obj < ocDARRELL) || (init.Anim[an].obj > oCAMERAMAX)) ClearText();
	st = ReadTime();

	init.Anim[an].active = b + 1;
	h->index = an;
	if (init.Anim[an].cam) ForcedCamera = init.Anim[an].cam;
	h->obj = (init.Anim[an].obj ? init.Anim[an].obj : CurObj);

	cp = 99;
	h->flags = 0;
	h->CurFrame = 0;
	h->LastFrame = -3;
	h->LoopStart = (uint16)-1;
	h->LoopEnd = 0;
	h->LoopMask = 0;
	for (a = 0; a < MAX_SUBANIMS; a++) {
		h->sub[a].flags = 0;
		h->sub[a].LastFrame = -3;

		// se non c'e' un link skippa
		if (init.Anim[an].meshlink[a][0] == 0) continue;

		h->sub[a].ptr = LinkMeshToStr(init, (char *)init.Anim[an].meshlink[a].rawArray());
		if (h->sub[a].ptr != nullptr) h->sub[a].ptr->CurFrame = -2;
		else DebugLogFile("Mesh not Found: |%s|", (char *)init.Anim[an].meshlink[a].rawArray());

		// se non deve caricare file skippa
		if ((init.Anim[an].name[a][0] == '\0') || (h->sub[a].ptr == nullptr) /*|| (h->sub[a].ptr->Flags & T3D_MESH_HIDDEN) */)
			continue;
//		DebugFile("Mesh Found: |%s| %X",Anim[an].meshlink[a], h->sub[a].ptr);

		if (h->sub[a].ptr->Anim.BoneTable) h->sub[a].ptr->releaseAnim(0);
		if (t3dLoadAnimation(game, (const char *)init.Anim[an].name[a].rawArray(), h->sub[a].ptr, 0) <= 0) { //continue;
			DebugLogFile("Failed to load animation %s. Quitting ...", (char *)init.Anim[an].name[a].rawArray());
			CloseSys(game);
		}

		if (init.Anim[an].flags & ANIM_ABS) h->sub[a].ptr->Flags |= T3D_MESH_ABS_ANIM;
		else h->sub[a].ptr->Flags &= ~T3D_MESH_ABS_ANIM;

		if (h->sub[a].ptr == Character[ocDARRELL]->Mesh  && PlayerPos[ocDARRELL]) cp = PlayerPos[ocDARRELL];
		if (h->sub[a].ptr == Character[ocVICTORIA]->Mesh && PlayerPos[ocVICTORIA]) cp = PlayerPos[ocVICTORIA];
		for (b = ocCUOCO; b <= ocLASTCHAR; b++)
			if (Character[b] && Character[b]->Mesh)
				if (h->sub[a].ptr == Character[b]->Mesh && PlayerPos[b]) cp = PlayerPos[b];
//		if( h->sub[a].ptr->Flags & T3D_MESH_CHARACTER )
//			CheckStandFrame( h->sub[a].ptr, Anim[an].name[a]);

//		Copia la posizione iniziale della bone0 se e' l'omino
		if (h->sub[a].ptr->Flags & T3D_MESH_CHARACTER) {
			if (h->sub[a].ptr == Player->Mesh) {
				game._messageSystem.removeEvent(EventClass::MC_PLAYER, ME_ALL);
				CharSetPosition(ocCURPLAYER, cp, nullptr);
				Player->Walk.NumPathNodes = Player->Walk.CurrentStep = Player->Walk.NumSteps = 0;
				Player->Walk.CurAction = aNULL;
				bPlayerInAnim = true;
			}

			if (init.Anim[an].flags & ANIM_NO_START_BLEND)
				h->sub[a].ptr->BlendPercent = 255;
			else
				h->sub[a].ptr->BlendPercent = 0;
		} else
			h->sub[a].ptr->BlendPercent = 255;

		if (init.Anim[an].flags & ANIM_BKG)
			FixupAnim(h->sub[a].ptr, cp, (const char *)init.Anim[an].RoomName.rawArray());
		else
			FixupAnim(h->sub[a].ptr, cp, "");
//		if( h->sub[a].ptr->Flags & T3D_MESH_CHARACTER )
//			CheckStandFrame( h->sub[a].ptr, Anim[an].name[a]);
//		DebugLogFile("Animation Loaded: |%s|",Anim[an].name[a]);

//		Se il personaggio principale non deve uscire dai bounds
		if ((h->sub[a].ptr->Flags & T3D_MESH_CHARACTER) && (h->sub[a].ptr == Player->Mesh) && (init.Anim[an].flags & ANIM_CHECK_BOUNDS))
			ForceAnimInBounds(ocCURPLAYER);

//		se non ha partenze strane piu' avanti -> parte subito
		for (b = 0; b < MAX_ATFRAMES; b++)
			if (((init.Anim[an].atframe[b].type == ATFSA_START) && !(init.Anim[an].atframe[b].index & (1 << a))))
				break;
		if (b >= MAX_ATFRAMES)
			h->sub[a].ptr->CurFrame = 0;

//		Se e' un'animazione di una camera
		if (h->sub[a].ptr == &game.init._globals._invVars.CameraDummy)
			game._cameraMan->StartAnimCamera(game);
	}
	if (AnimAutoPush) {
		PushATF(AnimAutoPush);
		AnimAutoPush = 0;
	}
//	Esegue tutti i primi ATFRAME prima del 7
	if (init.Anim[an].flags & ANIM_SKIP_1ST_FRAME) {
		for (b = 1; b <= 7; b++) {
			for (a = 0; a < MAX_SUBANIMS; a++)
				if (h->sub[a].ptr)
					h->sub[a].ptr->CurFrame = b;
			h->CurFrame = b;
			AtFrame(game, an);
		}
	}
//	Leva il tempo perso nel caricare dal frame-rate
	LoadTime += (ReadTime() - st);
}

/* -----------------13/05/98 16.09-------------------
 *                  StopAnim
 * --------------------------------------------------*/
void StopAnim(WGame &game, int32 an) {
	struct SActiveAnim *h;
	int32 a, b, t;
	Init &init = game.init;

	if (!an || !init.Anim[an].active) return;

	h = &ActiveAnim[init.Anim[an].active - 1];
	DebugLogFile("StopAnim %d (%s)", an, init.Anim[an].name[0].rawArray());
	if (h->sub[0].ptr && (h->sub[0].ptr->CurFrame > 0) && !(h->sub[0].ptr->Flags & T3D_MESH_DEFAULTANIM)) {
		for (b = h->sub[0].ptr->CurFrame; b < h->sub[0].ptr->Anim.NumFrames; b++) {
			for (a = 0; a < MAX_SUBANIMS; a++) {
				if ((h->sub[a].ptr) && ((!(init.Anim[an].flags & ANIM_SKIP_LAST_FRAME) && (h->sub[a].ptr->CurFrame + 1) < h->sub[a].ptr->Anim.NumFrames) ||
				                        ((h->sub[a].ptr->CurFrame + 1 + 1 * 3) < h->sub[a].ptr->Anim.NumFrames))) {
					if ((h->sub[a].ptr->CurFrame + 10) >= h->sub[a].ptr->Anim.NumFrames) {
//						DebugLogFile("CalcMeshBones |%s| %d",h->sub[a].ptr->Name,h->sub[a].ptr->CurFrame);
						t3dCalcMeshBones(h->sub[a].ptr, 1);
					}

					if (init.Anim[an].name[a][0] != '\0') h->sub[a].ptr->CurFrame ++;
				}
			}
			if (h->CurFrame > 0)
				h->CurFrame ++;
			else
				h->CurFrame = b;
			AtFrame(game, an);
		}
	}

	h->CurFrame = -2;
	AtFrame(game, an);
	if (!an || !init.Anim[an].active) return;
	CurActiveAnim = 0;

	for (a = 0; a < MAX_SUBANIMS; a++) {
		if (h->sub[a].ptr) {
			if ((h->sub[a].flags & ANIM_BLOCK) || (init.Anim[an].flags & ANIM_BLOCK)) {
			}

			if (!(init.Anim[an].flags & ANIM_BLOCK_PLAYER) && (h->sub[a].ptr == Player->Mesh)) {
				if (!(init.Anim[an].flags & ANIM_NO_FIX_POS))
					FixPos(ocCURPLAYER);
				else {
					PlayerPos[CurPlayer + ocDARRELL] = 0;
					PlayerGotoPos[CurPlayer + ocDARRELL] = 0;
					CharStop(ocCURPLAYER);
				}

				if (init.Anim[an].flags & ANIM_NO_END_BLEND)
					h->sub[a].ptr->BlendPercent = 255;

				t = TheTime + PLAYER_IDLE_TIME;
				_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERIDLE, MP_WAIT_RETRACE, (int16)(CurPlayer + ocDARRELL), 0, 0, &t, nullptr, nullptr);
				bPlayerInAnim = false;
			}
			if (!(h->sub[a].flags & ANIM_BLOCK) && !(init.Anim[an].flags & ANIM_BLOCK) && !(init.Anim[an].flags & ANIM_BLOCK_PLAYER))
				h->sub[a].ptr->CurFrame = 0;
		}
		ActiveAnim[ init.Anim[an].active - 1 ].sub[a].ptr = nullptr;
		ActiveAnim[ init.Anim[an].active - 1 ].sub[a].LastFrame = -3;
	}
	ActiveAnim[ init.Anim[an].active - 1 ].index = aNULL;
	init.Anim[an].flags &= ~ANIM_PAUSED;
	init.Anim[an].active = 0;
	if (!bDialogActive) ForcedCamera = 0;

//	if( Anim[an].obj )
//		CharStop( Anim[an].obj );

//	Se sono in un diario faccio continuare il diario con questa animazione
	if (init.Anim[an].flags & ANIM_DIARY)
		ContinueDiary(game, an);
//	Se sono in un dialogo controlla che sia finita l'animazione di Time
	else if (bDialogActive) {
		if (an == TimeAnim)
			_vm->_messageSystem.addWaitingMsgs(MP_WAIT_ANIM);
	}
//	Altrimenti aggiungo tutte le anim
	else
		_vm->_messageSystem.addWaitingMsgs(MP_WAIT_ANIM);

}

/* -----------------16/12/1999 17.39-----------------
 *                  StopAllAnims
 * --------------------------------------------------*/
void StopAllAnims(Init &init) {
	struct SActiveAnim *h;
	int32 i, j;

	for (i = 0; i < MAX_ACTIVE_ANIMS; i++) {
		h = &ActiveAnim[i];
		for (j = 0; j < MAX_SUBANIMS; j++) {
			if (h->sub[j].ptr) h->sub[j].ptr->CurFrame = 0;
			if (h->sub[j].ptr && init.Anim[ActiveAnim[i].index].flags & ANIM_NO_END_BLEND) h->sub[j].ptr->BlendPercent = 255;
			h->sub[j].ptr = nullptr;
			h->sub[j].LastFrame = -3;
		}
		init.Anim[ActiveAnim[i].index].active = 0;
		ActiveAnim[i].index = aNULL;
	}
	bPlayerInAnim = FALSE;
}

/* -----------------15/10/98 10.23-------------------
 *                  PauseAnim
 * --------------------------------------------------*/
void PauseAnim(Init &init, int32 an) {
	struct SActiveAnim *h;
	int32 b;

//	DebugLogFile("Pause %d",an);

	if (an == aNULL) return;

	for (b = 0; b < MAX_ACTIVE_ANIMS; b++) {
		if ((an > 0) && (ActiveAnim[b].index != an)) continue;

		h = &ActiveAnim[b];
		if (h->flags & ANIM_PAUSED) continue;

		h->flags |= ANIM_PAUSED;
		init.Anim[an].flags |= ANIM_PAUSED;
		DebugLogFile("An %d -> %X", an, init.Anim[an].flags);
	}
}

/* -----------------15/10/98 10.23-------------------
 *                  ContinueAnim
 * --------------------------------------------------*/
void ContinueAnim(Init &init, int32 an) {
	struct SActiveAnim *h;
	int32 b;

//	DebugLogFile("Con %d",an);

	if (an == aNULL) return;

	for (b = 0; b < MAX_ACTIVE_ANIMS; b++) {
		if ((an > 0) && (ActiveAnim[b].index != an)) continue;

		h = &ActiveAnim[b];
		if (!(h->flags & ANIM_PAUSED)) continue;

		h->flags &= ~ANIM_PAUSED;
		init.Anim[an].flags &= ~ANIM_PAUSED;
	}
}

/* -----------------06/01/99 17.05-------------------
 *                  StopObjAnim
 * --------------------------------------------------*/
void StopObjAnim(WGame &game, int32 obj) {
	if (!obj) return;

	if (obj == ocCURPLAYER) {
		obj = ocDARRELL + CurPlayer;
	}

	Init &init = game.init;
//	Se esiste gia' un'animazione sullo stesso oggetto base, la termina
	for (int32 b = 0; b < MAX_ACTIVE_ANIMS; b++)
		if ((ActiveAnim[b].index) && (ActiveAnim[b].CurFrame >= 0)/* && !( ActiveAnim[b].flags & ANIM_PAUSED )*/ &&
		        ((Common::String((const char *)init.Anim[ActiveAnim[b].index].meshlink[0].rawArray()).equalsIgnoreCase((const char *)init.Obj[obj].meshlink[0])) || (ActiveAnim[b].obj == obj) ||
		         ((CurPlayer == obj - ocDARRELL) && (Common::String((const char *)init.Anim[ActiveAnim[b].index].meshlink[0].rawArray()).equalsIgnoreCase((const char *)init.Obj[ocCURPLAYER].meshlink[0]))
		          && (ActiveAnim[b].sub[0].ptr) && Player && Player->Mesh && (ActiveAnim[b].sub[0].ptr == Player->Mesh))))
			StopAnim(game, ActiveAnim[b].index);
}


/* -----------------14/05/98 11.41-------------------
 *                  ProcessAnims
 * --------------------------------------------------*/
void ProcessAnims(WGame &game) {
	struct SActiveAnim *h;
	int32 a, b, c, an;

	for (b = 0; b < MAX_ACTIVE_ANIMS; b++) {
//		se non e' attiva skippa
		if ((an = ActiveAnim[b].index) == aNULL) continue;

		h = &ActiveAnim[b];
		if (h->flags & ANIM_PAUSED) {
//			se l'animazione e' stata pausata, ma le animazioni sono state riattivate riattivo l'animazione
//			sempre che non sia una che deve essere pausata come tapullo
			if ((!bPauseAllAnims))
				ContinueAnim(game.init, an);
			continue;
		}
		CurActiveAnim = b;

		h->LastFrame = h->CurFrame;
		h->CurFrame ++;
		c = 0;
		for (a = 0; a < MAX_SUBANIMS; a++) {
			if ((h->sub[a].ptr == nullptr) || (h->sub[a].ptr->CurFrame < 0)) continue;
			if (h->sub[a].ptr->Flags & T3D_MESH_DEFAULTANIM) continue;
			if (game.init.Anim[an].name[a][0] == '\0') continue;

			if (h->sub[a].ptr == Player->Mesh)
				bPlayerInAnim = TRUE;

			FirstFrame = (game.init.Anim[an].flags & ANIM_SKIP_1ST_FRAME) ? 1 + 1 * 3 : 1;
			LastFrame = (game.init.Anim[an].flags & ANIM_SKIP_LAST_FRAME) ? h->sub[a].ptr->Anim.NumFrames - 1 - 1 * 3 : h->sub[a].ptr->Anim.NumFrames - 1;
			CurFrame = h->sub[a].ptr->CurFrame;
			CurFlags = h->sub[a].ptr->Flags;

//			Spegne il loop se e' un personaggio in un dialogo e non c'e' frase sullo schermo e ha raggiunto la fine del loop
			if ((CurFlags & T3D_MESH_CHARACTER) && (bDialogActive) && !(bAnimWaitText) && ((CurFrame + 1) >= h->LoopEnd))
				h->LoopEnd = 0;

			h->sub[a].LastFrame = CurFrame;
//			Se arriva alla fine del loop o dell'anim reinizia
			if ((h->LoopEnd > 0) && !(h->LoopMask & (1 << a)) &&
			        ((CurFrame >= h->LoopEnd) || (((CurFrame) >= LastFrame) && (h->LoopEnd >= 10000))))
				h->sub[a].ptr->CurFrame = CurFrame = h->LoopStart;

//			Calcola le bones dell'ultimo frame
			if (CurFrame >= LastFrame) t3dCalcMeshBones(h->sub[a].ptr, 1);

			h->sub[a].ptr->CurFrame = CurFrame = CurFrame + 1;
			if (CurFrame > LastFrame) {
				if ((h->sub[a].flags & ANIM_BLOCK) || (game.init.Anim[an].flags & ANIM_BLOCK)) {
					h->sub[a].ptr->CurFrame = CurFrame = LastFrame;
					if (!(CurFlags & T3D_MESH_CHARACTER)) {
						UpdateBoundingBox(h->sub[a].ptr);
						_vm->addMeshModifier(h->sub[a].ptr->name, MM_ANIM_BLOCK, game.init.Anim[an].name[a].rawArray());
					}
				} else
					h->sub[a].ptr->CurFrame = -3;

				if (!(game.init.Anim[an].flags & ANIM_BLOCK_PLAYER)) {
					if ((CurFlags & T3D_MESH_CHARACTER) && (h->sub[a].ptr == Player->Mesh)) {
						if (!(game.init.Anim[an].flags & ANIM_NO_FIX_POS))
							FixPos(ocCURPLAYER);
						else {
							PlayerPos[CurPlayer + ocDARRELL] = 0;
							PlayerGotoPos[CurPlayer + ocDARRELL] = 0;
							CharStop(ocCURPLAYER);
						}
						bPlayerInAnim = FALSE;
					}
				}

				if ((h->sub[a].flags & ANIM_BLOCK) || (game.init.Anim[an].flags & ANIM_BLOCK)) {
				} else {
					h->sub[a].ptr->releaseAnim(0);
					h->sub[a].ptr->Flags |= T3D_MESH_DEFAULTANIM;
					h->sub[a].ptr->Flags &= ~T3D_MESH_ABS_ANIM;
				}

				if (game.init.Anim[an].flags & ANIM_NO_END_BLEND)
					h->sub[a].ptr->BlendPercent = 255;
			} else {
				c++;
				if (CurFlags & T3D_MESH_CHARACTER) {
					t3dVectCopy(&h->sub[a].ptr->Trasl, &h->sub[a].ptr->Anim.BoneTable[0].Trasl[h->sub[a].ptr->CurFrame]);
					t3dMatCopy(&h->sub[a].ptr->Matrix, &h->sub[a].ptr->Anim.BoneTable[0].Matrix[h->sub[a].ptr->CurFrame]);
				}
			}
		}
//		esegue atframe
		AtFrame(game, ActiveAnim[b].index);

		if (ActiveAnim[b].index && !(game.init.Anim[ActiveAnim[b].index].flags & (ANIM_NULL | ANIM_PAUSED)) && (!c))
			StopAnim(game, ActiveAnim[b].index);
//		Avanza si un solo frame e poi va in pausa
		if (bPauseAllAnims) PauseAnim(game.init, an);

	}
}

/* -----------------05/10/00 18.25-------------------
 *                  StopPlayingGame
 * --------------------------------------------------*/
void StopPlayingGame(WGame &game) {
	int32 i;
	Init &init = game.init;
//
	StopDiary(game, 0, 0, 0);
	bPauseAllAnims = TRUE;

	for (i = 0; i < T3D_MAX_CHARACTERS; i++)
		if (Character[i])
			CharStop(i);

	game._messageSystem.init();
	ClearUseWith();
	ClearText();
//

	StopAllAnims(init);
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

//	tapullo biblico per non far quittare il gioco quando si torna al MainMenu e si fa StartNew
	init.Dialog[dR000].flags &= ~ DIALOG_DONE;
	init.Dialog[dR111].flags &= ~ DIALOG_DONE;
	init.Dialog[dR211].flags &= ~ DIALOG_DONE;

//	evito che i personaggi rimangano a mezz'aria quando il gioco si stoppa con i personaggi in dubbie posizioni
	Character[ocCURPLAYER]->Mesh->Flags |= T3D_MESH_DEFAULTANIM;
	Character[ocDARRELL]->Mesh->Flags |= T3D_MESH_DEFAULTANIM;
	Character[ocVICTORIA]->Mesh->Flags |= T3D_MESH_DEFAULTANIM;

	t3dResetMesh(LinkMeshToStr(init, "darrell"));
	t3dResetMesh(LinkMeshToStr(init, "victoria"));

	PlayerStand[DARRELL].an = PlayerStand[VICTORIA].an = aNULL;
}

} // End of namespace Watchmaker
