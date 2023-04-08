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

#include "watchmaker/classes/do_player.h"
#include "watchmaker/t3d.h"
#include "watchmaker/globvar.h"
#include "watchmaker/define.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/utils.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/3d/t3d_mesh.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/animation.h"
#include "watchmaker/message.h"
#include "watchmaker/schedule.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/walk/walk.h"
#include "watchmaker/classes/do_sound.h"
#include "watchmaker/classes/do_string.h"
#include "watchmaker/ll/ll_mesh.h"

namespace Watchmaker {

// locals
#define BLEND_INC   65
t3dV3F SerraRect[2] = { { -6939.0f, 0.0f, -8396.0f }, { 6679.0f, 0.0f, -16561.0f } };
t3dV3F KreenRect1[2] = { { -1482.0f, 0.0f, -12162.0f }, { -1207.0f, 0.0f, -12586.0f } };
t3dV3F KreenRect2[2] = { { -1042.0f, 0.0f, -12162.0f }, { -766.0f, 0.0f, -12586.0f } };
t3dF32 KrennX = -600.0f;
t3dF32 KrennZ = -12570.0f;
t3dF32 CacciatoreXleft = -7450.0f;
t3dF32 CacciatoreXright = 8400.0f;
t3dF32 CacciatoreZright = -14701.0f;
t3dF32 r47CoordX = -2125.0f;

/* -----------------08/10/00 11.55-------------------
 *                  SetCurPlayerPosTo_9x()
 * --------------------------------------------------*/
void SetCurPlayerPosTo_9x(GameVars &gameVars, Init &init) {
	// Setto il player nella posizione 91 o 92 (o successive) se esistono
	t3dV3F dest;
	uint8 pos, start_pos, found_pos;

	pos = 0;
	start_pos = 0;
	found_pos = 0;

	if (CurPlayer == DARRELL)  start_pos = 91;
	if (CurPlayer == VICTORIA) start_pos = 92;

//		caso particolare nella r42, quando i personaggi potrebbero essere al di la' del portone
	if ((gameVars.getCurRoomId() == r42) && (init.Dialog[dR421_fine].flags & DIALOG_DONE)) {
		if (CurPlayer == DARRELL)  start_pos = 93;
	}
	if ((gameVars.getCurRoomId() == r42) && (init.Dialog[dR42_porta].flags & DIALOG_DONE)) {
		if (CurPlayer == VICTORIA)  start_pos = 94;
	}

//		caso particolare: quando con Victoria non posso entrare nel castello ma devo tirare il lucchetto, faccio
//		in modo che quando carico lei si posizioni vicino alla finestra e non a cozze
	if (bPorteEsternoBloccate && (gameVars.getCurRoomId() == rXT)) {
		if (CurPlayer == VICTORIA)  start_pos = 96;
	}

	for (pos = start_pos; pos <= 98; pos += 2) {
		if (!GetLightPosition(&dest, pos)) continue;

		found_pos = pos;
		break;
	}//for

	if (found_pos) {
		CharSetPosition(CurPlayer + ocDARRELL, found_pos, nullptr);
		warning("SETTATA POSITION: %d, %d", CurPlayer, start_pos);
	} else {
		t3dVectCopy(&Player->Pos, &t3dCurCamera->Target);
		t3dVectCopy(&Player->Mesh->Trasl, &t3dCurCamera->Target);
		warning("SETTATA POSITION SU CAMERATARGET: %d", CurPlayer);
	}
}

/* -----------------17/03/98 17.16-------------------
 *                      doMouse
 * --------------------------------------------------*/
void doPlayer(WGame &game) {
	switch (TheMessage->event) {
	case ME_PLAYERIDLE:
//			Parte animazione di idle
		if ((int32)TheTime > TheMessage->lparam[0]) {
			StopObjAnim(game, TheMessage->wparam1);
			//DebugString("Idle %d at %d",TheMessage->wparam1,TheMessage->lparam[0]);
		} else {
			TheMessage->flags |= MP_WAIT_RETRACE;
			//DebugString("NO %d",TheMessage->lparam[0]-TheTime);
			ReEvent();
		}
		break;

	case ME_PLAYERGOTO:
	case ME_PLAYERGOTOEXAMINE:
	case ME_PLAYERGOTOACTION:
	case ME_PLAYERGOTOEXIT:
	case ME_PLAYERGOTONOSKIP:
		if (CharNextFrame(game, ocCURPLAYER)) {
			TheMessage->flags |= MP_WAIT_RETRACE;
			ReEvent();
		} else {
//				se sono partito dalla prima persona, torna in prima persona
//				if ( TheMessage->bparam )
//					Event( EventClass::MC_CAMERA, ME_CAMERA3TO1, MP_DEFAULT, 0, 0, 0, NULL, NULL, NULL );

			if (Player)
				Player->Walk.NumPathNodes = Player->Walk.CurrentStep = Player->Walk.NumSteps = bNotSkippableWalk = 0;

			if ((!bDialogActive) || (TimeWalk == CurPlayer + ocDARRELL) || (TimeWalk == ocBOTH))
				AddWaitingMsgs(MP_WAIT_ACT);

			if (TheMessage->event == ME_PLAYERGOTOEXAMINE)
				Event(EventClass::MC_ACTION, ME_MOUSEEXAMINE, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, &TheMessage->lparam[0], nullptr, nullptr);
			else if (TheMessage->event == ME_PLAYERGOTOACTION)
				Event(EventClass::MC_ACTION, ME_MOUSEOPERATE, MP_DEFAULT, TheMessage->wparam1, TheMessage->wparam2, 0, &TheMessage->lparam[0], nullptr, nullptr);
			else if ((TheMessage->event == ME_PLAYERGOTO) && (TheMessage->lparam[1]))
				StartAnim(game, TheMessage->lparam[1]);

			/*??                if( TheMessage->lparam[0] )
			                {
			                    if( (bFirstPerson) && (Obj[TheMessage->lparam[0]].flags & HIDEIN1ST) )
			                        Event( EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, NULL, NULL, NULL );
			                    else if( !(bFirstPerson) && (Obj[TheMessage->lparam[0]].flags & HIDEIN3RD) )
			                        Event( EventClass::MC_CAMERA, ME_CAMERA3TO1, MP_DEFAULT, 0, 0, Obj[TheMessage->lparam[0]].pos, NULL, NULL, NULL );
			                }*/
		}
		break;
	}
}

/* -----------------02/10/98 10.17-------------------
 *                  ProcessCharacters
 * --------------------------------------------------*/
void ProcessCharacters(WGame &game) {
	int32 i, na, nf;
	uint8 cbi;
	t3dMESH *m;
	t3dCHARACTER *c;

	for (i = 0; i < T3D_MAX_CHARACTERS; i++)
		if (Character[i] && (Character[i] != Player) && (Character[i]->Walk.NumSteps))
			if (!CharNextFrame(game, i))
				if ((bDialogActive) && (TimeWalk == i))
					AddWaitingMsgs(MP_WAIT_ACT);

	for (i = 0; i < T3D_MAX_CHARACTERS; i++) {
		c = Character[i];
		if (c && (i != ocCURPLAYER) && !(c->Flags & T3D_CHARACTER_HIDE)) {
			nf = c->Walk.CurFrame;
			na = c->Walk.CurAction;
			if ((na == aWALK_START) || (na == aWALK_LOOP) || (na == aWALK_END)) {
				if ((nf == 81) || (nf == 171)) StartStepSound(game, &c->Pos, SOUND_PSX);
				else if ((nf == 36) || (nf == 126) || (nf == 216)) StartStepSound(game, &c->Pos, SOUND_PDX);
			} else if ((na == aBACK_START) || (na == aBACK_LOOP) || (na == aBACK_END)) {
				if ((nf == 37 + ActionStart[aBACK_START] - 3) || (nf == 162 + ActionStart[aBACK_START] - 3)) StartStepSound(game, &c->Pos, SOUND_PSX);
				else if (nf == 101 + ActionStart[aBACK_START] - 3) StartStepSound(game, &c->Pos, SOUND_PDX);
			} else if ((na == aRUN_START) || (na == aRUN_LOOP) || (na == aRUN_END)) {
				if (nf == 74 + ActionStart[aRUN_START] - 4) StartStepSound(game, &c->Pos, SOUND_PSX);
				else if (nf == 45 + ActionStart[aRUN_START] - 4) StartStepSound(game, &c->Pos, SOUND_PDX);
			}
		}
	}

//	Segue con la testa il mouse
	UpdateCharHead(ocCURPLAYER, &mPos);

//	Discesa del garage
	if (t3dCurRoom->name.equalsIgnoreCase("rxt.t3d")) {
		if ((m = LinkMeshToStr(game.init, "oxt-garage")) && (m->BBox[3].p.x - m->BBox[2].p.x) &&
		        (Player->Pos.x > m->BBox[2].p.x) && (Player->Pos.x < m->BBox[3].p.x) &&
		        (Player->Pos.z > m->BBox[2].p.z) && (Player->Pos.z < m->BBox[6].p.z)) {
			Player->Pos.y = ((Player->Pos.x - m->BBox[2].p.x) / (m->BBox[3].p.x - m->BBox[2].p.x)) * m->BBox[2].p.y;
			t3dVectCopy(&Player->Mesh->Trasl, &Player->Pos);
		}
	}

//	Parte RTV serra
	if (!(game.init.Dialog[dR1a1].flags & DIALOG_DONE) && !(bDialogActive) && (game._gameVars.getCurRoomId() == rXT)) {
		if ((Player->Mesh->Trasl.x > SerraRect[0].x) && (Player->Mesh->Trasl.x < SerraRect[1].x) &&
		        (Player->Mesh->Trasl.z > SerraRect[1].z) && (Player->Mesh->Trasl.z < SerraRect[0].z)) {
			if (!(LoaderFlags & T3D_DEBUGMODE)) {
				t3dF32 dist = SerraRect[1].x - Player->Mesh->Trasl.x;

				CharStop(ocCURPLAYER);

//				controllo da che parte far partire l'RTV
				if ((dist >= 0.0f) && (dist < 30.f))
					Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dRUN_R1a1_RIGHT, 0, 0, nullptr, nullptr, nullptr);
				else
					Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dRUN_R1a1_CENTER, 0, 0, nullptr, nullptr, nullptr);
			}
		}
	}

//	Parte Morte Cacciatore se passa l'esterno
	if ((bCacciatore) && !(LoaderFlags & T3D_DEBUGMODE)) {
		if (Player->Mesh->Trasl.x < CacciatoreXleft) {
			bCacciatore = FALSE;
			CharStop(ocCURPLAYER);
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR0014_left, 0, 0, nullptr, nullptr, nullptr);
		}
		if ((Player->Mesh->Trasl.x > CacciatoreXright) && (Player->Mesh->Trasl.z > CacciatoreZright)) {
			bCacciatore = FALSE;
			CharStop(ocCURPLAYER);
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR0014_right, 0, 0, nullptr, nullptr, nullptr);
		}
	}

//	Quando Victoria si avvicina per la prima volta alla Room47 parte un RTV che mostra Krenn.
	if (
	    (game._gameVars.getCurRoomId() == r47)
	    && (CurPlayer == VICTORIA)
	    && (!(game.init.Dialog[dR482].flags & DIALOG_DONE))
	    && (!(LoaderFlags & T3D_DEBUGMODE))
	    && (!bDialogActive)) {
		if (Player->Mesh->Trasl.x > r47CoordX) {
			CharStop(ocCURPLAYER);
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR482, 0, 0, nullptr, nullptr, nullptr);
		}
	}

//	Parte Morte Victoria se Krenn la vede
	if ((game._gameVars.getCurRoomId() == r48) && (!bDialogActive) && (!(LoaderFlags & T3D_DEBUGMODE))) {
		int whichanim;
		int beccato;

		whichanim = WhichAnimChar(game.init, ocKRENN);
		beccato = 0;

		if ((whichanim == aKRE481) || (whichanim == aKRE482) || (whichanim == aKRE483) || (whichanim == aKRE484)) {
			if (Player->Mesh->Trasl.z < KrennZ) {
				//se Krenn e' in movimento basta arrivare sotto la linea verticale delle colonne per beccarti
				beccato = 1;
			} else if (whichanim != aKRE481) {
				if ((Player->Mesh->Trasl.x > KrennX)    //oltrepassata l'ultima colonna
				        || (
				            (Player->Mesh->Trasl.x > KreenRect1[0].x) && (Player->Mesh->Trasl.x < KreenRect1[1].x)
				            && (Player->Mesh->Trasl.z < KreenRect1[0].z) && (Player->Mesh->Trasl.z > KreenRect1[1].z)
				        )
				        || (
				            (Player->Mesh->Trasl.x > KreenRect2[0].x) && (Player->Mesh->Trasl.x < KreenRect2[1].x)
				            && (Player->Mesh->Trasl.z < KreenRect2[0].z) && (Player->Mesh->Trasl.z > KreenRect2[1].z)
				        )
				   )
					beccato = 1;
			}
		}

		if (beccato) {
//			DebugLogFile("BECCATO");
			CharStop(ocCURPLAYER);
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR48KRENNSPARA, 0, 0, nullptr, nullptr, nullptr);
		}
	}

//	Dice la frase dell'eclisse se non l'ha gi� detta
	if ((game._gameVars.getCurRoomId() == rXT) && (t3dCurTime >= 1300) && (t3dCurTime <= 1310) && (!(game.init.Obj[oADDTEXTOBJ].flags & EXTRA))) {
		game.init.Obj[oADDTEXTOBJ].flags |= EXTRA;
		PlayerSpeak(game, game.init.Obj[oADDTEXTOBJ].examine[CurPlayer]);
	}

//	Porta gradualmente il Blending a zero
	cbi = (uint8)((t3dF32)BLEND_INC * FrameFactor);
	if (bDialogActive) cbi /= 5;
	for (i = 0; i < T3D_MAX_CHARACTERS; i++) {
		if (Character[i] && (i != ocCURPLAYER) && (Character[i]->Mesh->BlendPercent != 255)) {
			if ((Character[i]->Mesh->BlendPercent + cbi) < 255)
				Character[i]->Mesh->BlendPercent += cbi;
			else
				Character[i]->Mesh->BlendPercent = 255;
//			Character[i]->Mesh->BlendPercent = 255;
		}
	}
}


/* -----------------17/08/00 17.31-------------------
 *                  UpdatePlayerStand
 * --------------------------------------------------*/
void UpdatePlayerStand(WGame &game, uint8 oc) {
	int32 a, b, panims[MAX_ANIMS_IN_ROOM], pc, na;
	t3dF32 mindist, dist;
	t3dV3F pos;
	Init &init = game.init;
	auto curRoom = game._gameVars.getCurRoomId();

	na = 0;
	pc = 0;
	mindist = 9999999.0f;
	memset(panims, 0, sizeof(panims));
	for (a = 0; a < MAX_ANIMS_IN_ROOM; a++) {
		b = game.getCurRoom().anims[a];
		if ((b == aNULL) || !(init.Anim[b].flags & ANIM_STAND) || (init.Anim[b].obj != (ocDARRELL + CurPlayer)))
			continue;

//		caso particolare nel rXT, quando i personaggi potrebbero teletrasportarsi in mezzo all'RTV della serra (dove appunto le luci 93 e 94 sono messe)
		if ((curRoom == rXT) && (!(init.Dialog[dR1a1].flags & DIALOG_DONE)))
			if ((init.Anim[b].pos == 93) || (init.Anim[b].pos == 94)) continue;

		if (!(GetLightPosition(&pos, init.Anim[b].pos)))
			continue;

		if ((dist = t3dVectDistance(&Player->Mesh->Trasl, &pos)) < mindist) {
			mindist = dist;
			na = b;
		}
		warning("Found %d, dist %f, good %d", b, dist, na);

		panims[pc] = b ;
		pc ++;
	}

//	Ferma il primo personaggio e salva le posizioni
	game._messageSystem.removeEvent(EventClass::MC_PLAYER, ME_ALL);
//	Salva informazioni
	{
//		DebugLogFile("NO");
		PlayerStand[CurPlayer].pos = init.Anim[na].pos;

		PlayerStand[CurPlayer].cr = curRoom;
		PlayerStand[CurPlayer].bnd = t3dCurRoom->CurLevel;
		if ((curRoom == r15) || (curRoom == r1F)) PlayerStand[CurPlayer].bnd = 0;
		PlayerStand[CurPlayer].roomName = t3dCurRoom->name;

		if (CurPlayer == DARRELL)
			PlayerStand[CurPlayer].an = aIDLE_DARRELL_1;
		else
			PlayerStand[CurPlayer].an = aIDLE_VICTORIA_1;
	}
}

/* -----------------22/10/98 17.29-------------------
 *                  ChangePlayer
 * --------------------------------------------------*/
void ChangePlayer(WGame &game, uint8 oc) {
	int32 a, b,/*panims[MAX_ANIMS_IN_ROOM],pc,*/na;
//	t3dF32 mindist,dist;
	t3dV3F pos;
	Init &init = game.init;

	if ((oc - ocDARRELL) == CurPlayer)
		return ;

//	caso particolare: sono nella r2a e faccio andare il submusic 1 (ovvero frigo.mid), il problema e' che le altre
//	stanze non hanno submusic 1 e quindi se cambio personaggio devo resettare submusic a 0 (cosa che generalmente
//	fa l'animazione di uscita dalla porta, che in questo caso non viene pero' chiamata)
	if (t3dCurRoom->name.equalsIgnoreCase("r2a.t3d") && CurSubMusic)
		CurSubMusic = 0;

	bFirstPerson = false;

//	Aggiorna la variabile di cambio personaggio
	UpdatePlayerStand(game, oc);
//	ferma sia darrell sia victoria
	StopObjAnim(game, ocDARRELL);
	StopObjAnim(game, ocVICTORIA);

//	cerca tutte le possibili posizioni di cambio
	CharSetPosition(ocCURPLAYER, PlayerStand[CurPlayer].pos, nullptr);
	StartAnim(game, PlayerStand[CurPlayer].an);
//	DebugLogFile("---");
//	Cambia personaggio
	bPlayerInAnim = false;
	CurPlayer = oc - ocDARRELL;
	Character[ocCURPLAYER] = Character[oc];
	Player = Character[oc];
	Player->Flags &= ~T3D_CHARACTER_HIDE;
//	Resetta telecamera
	CameraTargetObj = ocCURPLAYER;
	CameraTargetBone = 0;

	auto curRoom = game._gameVars.getCurRoomId();
//	Se il secondo personaggio non esisteva
	if (PlayerStand[CurPlayer].cr == rNULL) {
//		cerca una animazione adatta nella stessa stanza
		for (a = 0; a < MAX_ANIMS_IN_ROOM; a++) {
			b = game.getCurRoom().anims[a];
			if ((b == aNULL) || !(init.Anim[b].flags & ANIM_STAND) || (init.Anim[b].obj != (ocDARRELL + CurPlayer)))
				continue;

			if (!(GetLightPosition(&pos, init.Anim[b].pos)))
				continue;

			na = b;
			PlayerStand[CurPlayer].pos = init.Anim[na].pos;
			PlayerStand[CurPlayer].cr = curRoom;
			PlayerStand[CurPlayer].bnd = t3dCurRoom->CurLevel;
			PlayerStand[CurPlayer].roomName = t3dCurRoom->name.c_str();

			if (CurPlayer == DARRELL)
				PlayerStand[CurPlayer].an = aIDLE_DARRELL_1;
			else
				PlayerStand[CurPlayer].an = aIDLE_VICTORIA_1;

			break;
		}
	}
//	Cambia stanza andandosi a leggere la posizione e l'animazione che aveva prima
	warning("Changing Room to |%s| pos: %d an: %d", PlayerStand[CurPlayer].roomName.c_str(), PlayerStand[CurPlayer].pos, PlayerStand[CurPlayer].an);
	ChangeRoom(game, PlayerStand[CurPlayer].roomName, PlayerStand[CurPlayer].pos, 0);
	if ((curRoom == r15) || (curRoom == r1F))
		SetBndLevel(game, nullptr, PlayerStand[CurPlayer].bnd);
	CharStop(ocCURPLAYER);
	Player->Mesh->Flags |= T3D_MESH_DEFAULTANIM;

	if (Player && t3dCurRoom) {
		Player->Walk.Panel = t3dCurRoom->Panel[t3dCurRoom->CurLevel];
		Player->Walk.PanelNum = t3dCurRoom->NumPanels[t3dCurRoom->CurLevel];
		if (&t3dCurRoom->PanelHeight[t3dCurRoom->CurLevel])
			Player->Mesh->Trasl.y = Player->Pos.y = CurFloorY = t3dCurRoom->PanelHeight[t3dCurRoom->CurLevel];
	}

//	Inizia il Fade
//	Event( EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND/3, 0, EFFECT_FADIN, NULL, NULL, NULL );
	Event(EventClass::MC_CAMERA, ME_CAMERAPLAYER, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
}

/* -----------------30/10/00 18.26-------------------
 *                  IsPlayerInPool()
 * --------------------------------------------------*/
int IsPlayerInPool() {
	if (!t3dCurRoom) return (0);
	if (!Character[ocCURPLAYER]->CurRoom->name.equalsIgnoreCase("r22.t3d")) return (0);
	if (!t3dCurRoom->name.equalsIgnoreCase("r22.t3d")) return (0);
	if (t3dCurRoom->CurLevel != 1) return (0);

	return (1);
}

/* -----------------31/10/00 09.26-------------------
 *                  PlayerCanSwitch()
 * --------------------------------------------------*/
int PlayerCanSwitch(GameVars &gameVars, char no_inventory) {
	auto curRoom = gameVars.getCurRoomId();
	if (bNoPlayerSwitch)        return (0);
	if (curRoom == r15)     return (0);
	if (curRoom == r31)     return (0);
	if (curRoom == r1F)     return (0);
	if (curRoom == r1D)     return (0);
	if (curRoom == r42)     return (0);
	if (IsPlayerInPool())       return (0);
	if (bPlayerSuBasamento) return (0);

// significa che la richiesta viene fatta quando l'inventario "grosso" non e' aperto e quindi bisogna controllare
// dei parametri in piu' (parametri che quando l'inventario e' aperto sono sicuramente giusti)
	if (no_inventory) {
		if (bPlayerInAnim)                          return (0);
		if (bNotSkippableWalk)                      return (0);
		if (bDialogActive)                          return (0);
	}

	return (1);
}

/* -----------------31/10/00 09.26-------------------
 *                  PlayerCanCall()
 * --------------------------------------------------*/
int PlayerCanCall(GameVars &gameVars) {
	auto curRoom = gameVars.getCurRoomId();
	if (bNoPlayerSwitch)        return (0);
	if (curRoom == r15)     return (0);
	if (curRoom == r31)     return (0);
	if (curRoom == r1F)     return (0);
	if (curRoom == r1D)     return (0);
	if (curRoom == r42)     return (0);
	if (IsPlayerInPool())       return (0);
	if (bPlayerSuBasamento) return (0);
	if (bSezioneLabirinto)      return (0);

	if (curRoom == rXT) return (0);
	if (t3dCurRoom->name.equalsIgnoreCase(PlayerStand[CurPlayer ^ 1].roomName)) return (0);

	return (1);
}

/* -----------------31/10/00 09.26-------------------
 *                  PlayerCanSave()
 * --------------------------------------------------*/
int PlayerCanSave() {
	if (bSaveDisabled)          return (0);
	if (bCacciatore)            return (0);
	if (IsPlayerInPool())       return (0);
	if (bPlayerSuBasamento) return (0);
	if (bLockCamera)            return (0);

	return (1);
}

} // End of namespace Watchmaker
