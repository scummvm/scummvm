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

#include "watchmaker/classes/do_operate.h"
#include "watchmaker/t3d.h"
#include "watchmaker/define.h"
#include "watchmaker/globvar.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/ll/ll_mesh.h"
#include "watchmaker/message.h"
#include "watchmaker/schedule.h"
#include "watchmaker/classes/do_sound.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/classes/do_string.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/classes/do_inv.h"
#include "watchmaker/3d/t3d_body.h"

namespace Watchmaker {

/* -----------------14/04/00 16.25-------------------
 *                  UpdateSpecial
 * --------------------------------------------------*/
void UpdateSpecial(WGame &game, int32 room) {
	extern t3dBODY *t3dCurRoom;
	t3dMESH *m;
	Init &init = game.init;


	switch (room) {
	case r15:
		if (Player && Player->Mesh) {
			if ((Player->Mesh->Trasl.x > -4826.0f) && (Player->Mesh->Trasl.z > -3412.0f)) {
				DebugLogWindow("r15: set0");
				t3dCurRoom->CurLevel = 0;
				init.Obj[o15CREPAESTERNA].flags |= ON;
				init.Obj[o15BRECCIA].flags &= ~ON;
				init.Obj[o15COLONNINATORRE].flags |= ON;
				init.Obj[o15p2E].flags |= ON;
				init.Obj[o15p2F].flags |= ON;

				if ((m = LinkMeshToStr(init, "p15-portal_to_room31")))
					m->Flags |= T3D_MESH_VIEWONLYPORTAL;
			} else {
				DebugLogWindow("r15: set1");
				t3dCurRoom->CurLevel = 1;
				init.Obj[o15CREPAESTERNA].flags &= ~ON;
				init.Obj[o15BRECCIA].flags |= ON;
				init.Obj[o15COLONNINATORRE].flags &= ~ON;
				init.Obj[o15p2E].flags &= ~ON;
				init.Obj[o15p2F].flags &= ~ON;

				if ((m = LinkMeshToStr(init, "p15-portal_to_room31")))
					m->Flags &= ~T3D_MESH_VIEWONLYPORTAL;
			}

			CurFloorY = t3dCurRoom->PanelHeight[t3dCurRoom->CurLevel];
		}
		break;
	case r1F:
		if (Player && Player->Mesh) {
			if ((Player->Mesh->Trasl.x > -8767.0f)) {
				DebugLogWindow("r1F: set0");
				t3dCurRoom->CurLevel = 0;
				init.Obj[o1FDETRITI].anim[CurPlayer] = a1F5;
				init.Obj[o1FVARCO].anim[CurPlayer] = aNULL;
			} else {
				DebugLogWindow("r1F: set1");
				t3dCurRoom->CurLevel = 1;
				init.Obj[o1FVARCO].anim[CurPlayer] = a1F6;
				init.Obj[o1FDETRITI].anim[CurPlayer] = aNULL;
			}

			CurFloorY = t3dCurRoom->PanelHeight[t3dCurRoom->CurLevel];
		}
		break;

	case r1D:
		if ((m = LinkMeshToStr(init, "o1d-rotellina01")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb1D[0]) >= 0 ? (Comb1D[0]) : 0);
		if ((m = LinkMeshToStr(init, "o1d-rotellina02")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb1D[1]) >= 0 ? (Comb1D[1]) : 0);
		if ((m = LinkMeshToStr(init, "o1d-rotellina03")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb1D[2]) >= 0 ? (Comb1D[2]) : 0);
		if ((m = LinkMeshToStr(init, "o1d-rotellina04")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb1D[3]) >= 0 ? (Comb1D[3]) : 0);
		if ((m = LinkMeshToStr(init, "o1d-rotellina05")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb1D[4]) >= 0 ? (Comb1D[4]) : 0);
		break;

	case r25:
		if ((m = LinkMeshToStr(init, "o25-mikrondedisplayON")) && m->hasFaceMaterial())
			m->setMovieFrame((Forno25) >= 0 ? (Forno25) : 0);
		if ((m = LinkMeshToStr(init, "o25-freezervask02")) && m->hasFaceMaterial())
			m->setMovieFrame((Frigo25) >= 0 ? (Frigo25) : 0);

		break;

	case r2D:
		if ((m = LinkMeshToStr(init, "o2d-displayino01")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb2D[0] - o2D0 + 1) >= 0 ? (Comb2D[0] - o2D0 + 1) : 0);
		if ((m = LinkMeshToStr(init, "o2d-displayino02")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb2D[1] - o2D0 + 1) >= 0 ? (Comb2D[1] - o2D0 + 1) : 0);
		if ((m = LinkMeshToStr(init, "o2d-displayino03")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb2D[2] - o2D0 + 1) >= 0 ? (Comb2D[2] - o2D0 + 1) : 0);
		if ((m = LinkMeshToStr(init, "o2d-displayino04")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb2D[3] - o2D0 + 1) >= 0 ? (Comb2D[3] - o2D0 + 1) : 0);
		if ((m = LinkMeshToStr(init, "o2d-displayino05")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb2D[4] - o2D0 + 1) >= 0 ? (Comb2D[4] - o2D0 + 1) : 0);
		break;

	case r2Q:
		if ((m = LinkMeshToStr(init, "o2q-displayino01")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb2Q[0] - o2Q0 + 1) >= 0 ? (Comb2Q[0] - o2Q0 + 1) : 0);
		if ((m = LinkMeshToStr(init, "o2q-displayino02")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb2Q[1] - o2Q0 + 1) >= 0 ? (Comb2Q[1] - o2Q0 + 1) : 0);
		if ((m = LinkMeshToStr(init, "o2q-displayino03")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb2Q[2] - o2Q0 + 1) >= 0 ? (Comb2Q[2] - o2Q0 + 1) : 0);
		if ((m = LinkMeshToStr(init, "o2q-displayino04")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb2Q[3] - o2Q0 + 1) >= 0 ? (Comb2Q[3] - o2Q0 + 1) : 0);
		if ((m = LinkMeshToStr(init, "o2q-displayino05")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb2Q[4] - o2Q0 + 1) >= 0 ? (Comb2Q[4] - o2Q0 + 1) : 0);
		break;

	case r31:
		if ((m = LinkMeshToStr(init, (char *)init.Obj[o31LEVETTA1].meshlink[0]))) t3dSetSpecialAnimFrame(game, "r31-cles01.a3d", m, Comb31[0] + 1);
		if ((m = LinkMeshToStr(init, (char *)init.Obj[o31LEVETTA2].meshlink[0]))) t3dSetSpecialAnimFrame(game,  "r31-cles02.a3d", m, Comb31[1] + 1);
		if ((m = LinkMeshToStr(init, (char *)init.Obj[o31LEVETTA3].meshlink[0]))) t3dSetSpecialAnimFrame(game,  "r31-cles03.a3d", m, Comb31[2] + 1);
		if ((m = LinkMeshToStr(init, (char *)init.Obj[o31LEVETTA4].meshlink[0])))  t3dSetSpecialAnimFrame(game,  "r31-cles04.a3d", m, Comb31[3] + 1);
		if ((m = LinkMeshToStr(init, (char *)init.Obj[o31LEVETTA5].meshlink[0]))) t3dSetSpecialAnimFrame(game,  "r31-cles05.a3d", m, Comb31[4] + 1);
		break;

	case r38:
		if (Player && Player->Mesh) {
			if (Player->Mesh->Trasl.y < 2000.0f) {
				DebugLogWindow("r38: set0");
				init.Obj[o38SCALA].anim[CurPlayer] = a382;
				init.Obj[o38SCALA].pos = 2;
			} else {
				DebugLogWindow("r38: set1");
				init.Obj[o38SCALA].anim[CurPlayer] = a383;
				init.Obj[o38SCALA].pos = 3;
			}
		}
		break;

	case r44:
		if ((m = LinkMeshToStr(init, "o44-lancettaore")))  t3dSetSpecialAnimFrame(game, "r44-orologio-ore.a3d", m,     Comb44[0]);
		if ((m = LinkMeshToStr(init, "o44-lancettaminuti")))   t3dSetSpecialAnimFrame(game, "r44-orologio-minuti.a3d", m,  Comb44[1]);
		if ((m = LinkMeshToStr(init, "o44-indicatore")))       t3dSetSpecialAnimFrame(game, "r44-gradi.a3d", m,                Comb44[2] + 1);
		break;

	case r45:
		if ((m = LinkMeshToStr(init, "o45-contatore01")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb45[0]) >= 0 ? (Comb45[0]) : 0);
		if ((m = LinkMeshToStr(init, "o45-contatore02")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb45[1]) >= 0 ? (Comb45[1]) : 0);
		if ((m = LinkMeshToStr(init, "o45-contatore03")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb45[2]) >= 0 ? (Comb45[2]) : 0);
		if ((m = LinkMeshToStr(init, "o45-contatore04")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb45[3]) >= 0 ? (Comb45[3]) : 0);
		if ((m = LinkMeshToStr(init, "o45-contatore05")) && m->hasFaceMaterial())
			m->setMovieFrame((Comb45[4]) >= 0 ? (Comb45[4]) : 0);
		break;
	}
}

/* -----------------19/05/98 16.40-------------------
 *                  doOperate
 * --------------------------------------------------*/
void doOperate(WGame &game, int32 obj) {
	extern t3dBODY *t3dCurRoom;
	uint8 sent = FALSE;
	int32 sa = 0;
	Init &init = game.init;

	if (bPorteEsternoBloccate) {
		// non faccio entrare il giocatore nel castello
		if (obj == oXT1Ap22) {
			sa = aCURPLAYER_PARLA;
			PlayerSpeak(game, init.Obj[oXT1Ap22].text[0]);
			return;
		}

		if (obj == oXT11p21) {
			sa = aCURPLAYER_PARLA;
			PlayerSpeak(game, init.Obj[oXT1Ap22].text[0]);
			return;
		}

		if (obj == oXT18p25) {
			sa = aCURPLAYER_PARLA;
			PlayerSpeak(game, init.Obj[oXT1Ap22].text[0]);
			return;
		}

		if (obj == oXT14p3C) {
			sa = aCURPLAYER_PARLA;
			PlayerSpeak(game, init.Obj[oXT1Ap22].text[0]);
			return;
		}

		if (obj == oXT1APOZZO) {
			sa = aCURPLAYER_PARLA;
			PlayerSpeak(game, init.Obj[oXT1APOZZO].text[1]);
			return;
		}
	}

	switch (obj) {
	case oXT1APOZZO:
		if (init.Obj[obj].anim[CurPlayer]) {
			if (!(init.Obj[oXT1APOZZO].flags & EXTRA2)) {
				IncCurTime(game, 15);
				init.Obj[oXT1APOZZO].flags |= EXTRA2;
			}
			sa = init.Obj[obj].anim[CurPlayer];
		} else
			sent = TRUE;
		break;

	case  oXT14BASAMENTO:
		if ((WhichAnimChar(init, ocGIARDINIERE) == aGIA141) || (WhichAnimChar(init, ocGIARDINIERE) == aGIA142))
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dGIARDINIERE_INTERRUPT1, 0, 0, nullptr, nullptr, nullptr);
		else if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		else
			sent = TRUE;
		break;

	case o13SCRITTOIO:
	case o13MOBILE:
	case o13CREDENZA:
		if (WhichRoomChar(init, ocCUSTODE) == r13)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dCUSTODE_INTERRUPT3, 0, 0, nullptr, nullptr, nullptr);
		else if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		else
			sent = TRUE;
		break;

	case o13SCALPELLO:
		if (WhichRoomChar(init, ocCUSTODE) == r13)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dCUSTODE_INTERRUPT1, 0, 0, nullptr, nullptr, nullptr);
		else if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		else
			sent = TRUE;
		break;

//        case o17COMPUTER:
//            Event( EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, 0, 0, tCOMPUTER, NULL, NULL, NULL );
//            break;

	case o19T01:
	case o19T02:
	case o19T03:
	case o19T04:
	case o19T05:
	case o19T06:
	case o19T07:
	case o19T08:
	case o19T09:
	case o19T10:
	case o19T11:
	case o19T12:
	case o19T13:
	case o19T14:
	case o19T15:
	case o19T16:
	case o19T17:
	case o19T18:
	case o19T19:
	case o19T20:
	case o19T21:
	case o19T22:
	case o19T23:
	case o19T24:
	case o19T25:
	case o19T26:
	case o19T27:
	case o19T28:
	case o19T29:
	case o19T30:
	case o19T31:
	case o19T32:
	case o19T33:
	case o19T34:
	case o19T35:
	case o19T36:
	case o19T37:
	case o19T38:
	case o19T39:
	case o19T40:
	case o19T41:
	case o19T42:
	case o19T43:
	case o19T44:
	case o19T45:
	case o19T46:
	case o19T47:
	case o19T48:
	case o19T49:
	case o19T50:
	case o19T51:
	case o19T52:
	case o19T53:
	case o19T54:
	case o19T55:
	case o19T56:
	case o19T57:
	case o19T58:
	case o19T59:
	case o19T60:
	case o19T61:
	case o19T62:
	case o19T63:
	case o19T64:
		StartSound(game, w197);
		Comb19[0] = Comb19[1];
		Comb19[1] = Comb19[2];
		Comb19[2] = obj;
		if ((Comb19[1] == o19T22) && (Comb19[2] == o19T26) && !(init.Obj[o37BACHECA].flags & EXTRA)) {
			Comb19[0] = Comb19[1] = Comb19[2] = 0;
			Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
			Event(EventClass::MC_STRING, ME_PLAYERSPEAK, MP_WAIT_CAMERA, init.Obj[o19SCACCHIERA].text[0], 0, 0, nullptr, nullptr, nullptr);
			break;
		}

		if ((Comb19[0] == o19T22) && (Comb19[1] == o19T26) && (Comb19[2] == o19T40)) {
			init.Obj[o19SCACCHIERA].anim[CurPlayer] = init.Obj[o19SCACCHIERA].anim[CurPlayer ^ 1] = a193;
			Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
			CurObj = o19SCACCHIERA;
			Event(EventClass::MC_ACTION, ME_MOUSEOPERATE, MP_WAIT_CAMERA, 0, 0, bFirstPerson, &CurObj, nullptr, nullptr);
		}
		break;

	case o1DSWITCH1:
	case o1DSWITCH2:
	case o1DSWITCH3:
	case o1DSWITCH4:
	case o1DSWITCH5:
		Comb1D[obj - o1DSWITCH1] ++;
		if (Comb1D[obj - o1DSWITCH1] > 10) Comb1D[obj - o1DSWITCH1] = 0;
		UpdateSpecial(game, r1D);
		break;

	case o1DTASTO:
		StartSound(game, w1D7);
		if (
		    IconInInv(init, i2cBANCONOTA1) && (!(init.Obj[o1DVALIGETTAAP].flags & ON))
		    && (Comb1D[0] == 4 + 1) && (Comb1D[1] == 1 + 1) && (Comb1D[2] == 7 + 1) && (Comb1D[3] == 3 + 1) && (Comb1D[4] == 0 + 1)
		) {
			init.Obj[o1DVALIGETTACH].anim[DARRELL] = init.Obj[o1DVALIGETTACH].anim[VICTORIA] = a1D8;
			init.Obj[o1DVALIGETTACH].pos = 8;
			CurObj = o1DVALIGETTACH;
			StartAnim(game, init.Obj[o1DVALIGETTACH].anim[CurPlayer]);
			if (!(init.Obj[o1DTASTO].flags & EXTRA2)) {
				IncCurTime(game, 15);
				init.Obj[o1DTASTO].flags |= EXTRA2;
				init.Obj[o1DTASTO].action[CurPlayer] = init.Obj[o1DTASTO].action[CurPlayer ^ 1] = init.Obj[o1DTASTO].text[0];
				sent = TRUE;
			}
		} else
			sent = TRUE;
		break;

	case o1DVALIGETTACH:
		if (IconInInv(init, i2cBANCONOTA1) && (Comb1D[0] == 4 + 1) && (Comb1D[1] == 1 + 1) && (Comb1D[2] == 7 + 1) && (Comb1D[3] == 3 + 1) && (Comb1D[4] == 0 + 1)) {
			init.Obj[o1DVALIGETTACH].anim[DARRELL] = init.Obj[o1DVALIGETTACH].anim[VICTORIA] = a1D8;
			init.Obj[o1DVALIGETTACH].pos = 8;

			if (!(init.Obj[o1DTASTO].flags & EXTRA2)) {
				IncCurTime(game, 15);
				init.Obj[o1DTASTO].flags |= EXTRA2;
				init.Obj[o1DTASTO].action[CurPlayer] = init.Obj[o1DTASTO].action[CurPlayer ^ 1] = init.Obj[o1DTASTO].text[0];
				sent = TRUE;
			}
		} else {
			init.Obj[o1DVALIGETTACH].anim[DARRELL] = init.Obj[o1DVALIGETTACH].anim[VICTORIA] = a1D6;
			init.Obj[o1DVALIGETTACH].pos = 6;
		}
		CurObj = o1DVALIGETTACH;
		StartAnim(game, init.Obj[o1DVALIGETTACH].anim[CurPlayer]);
		break;

	/*      case o1FPULSANTE:
	            if ( init.Obj[o1FLEVAGIU].flags & ON )
	                Event( EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR1F1, 0, 0, NULL, NULL, NULL );
	            else if( init.Obj[obj].anim[CurPlayer] )
	                sa = init.Obj[obj].anim[CurPlayer];
	            else
	                sent = TRUE;
	            break;
	*/
	case o23ATASTO1:
	case o23BTASTO1:
	case o23CTASTO1:
		bNoFirstPersonSwitch = FALSE;
		ChangeRoom(game, "r21-a.t3d", 46, a2111);
		if (obj == o23ATASTO1) Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, TRUE, nullptr, nullptr, nullptr);
		break;
	case o23ATASTO2:
	case o23BTASTO2:
	case o23CTASTO2:
		bNoFirstPersonSwitch = FALSE;
		ChangeRoom(game, "r26-b.t3d", 46, a268);
		if (obj == o23BTASTO2) Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, TRUE, nullptr, nullptr, nullptr);
		break;
	case o23ATASTO3:
	case o23BTASTO3:
	case o23CTASTO3:
		bNoFirstPersonSwitch = FALSE;
		ChangeRoom(game, "r2c.t3d", 46, a2C10);
		if (obj == o23CTASTO3) Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, TRUE, nullptr, nullptr, nullptr);
		break;
	case o25TASTOPIUFORNO:
		if (init.Obj[o25TASTOOFFFORNO].flags & ON) {
			if (Forno25 < 34) Forno25 ++;
			UpdateSpecial(game, r25);
		}
		break;
	case o25TASTOMENOFORNO:
		if (init.Obj[o25TASTOOFFFORNO].flags & ON) {
			if (Forno25 > 0) Forno25 --;
			UpdateSpecial(game, r25);
		}
		break;
	case o25TASTOPIUCONGELATORE:
	case o25TASTOMENOCONGELATORE:
		if (obj == o25TASTOPIUCONGELATORE) {
			if (Frigo25 < 8) Frigo25 ++;
		} else {
			if (Frigo25 > 0) Frigo25 --;
		}

		UpdateSpecial(game, r25);

//			se la fiala e' OK ma il frigo non e' a -25 la fiala salta
		if ((init.Obj[o25FIALAACONGELATOREFREDDA].flags & ON) && (Frigo25 != 3)) {
			init.Obj[o25FIALAACONGELATOREFREDDA].flags &= ~ON;
			init.Obj[o25FIALAACONGELATORE].flags |= ON;
			UpdateObjMesh(init, o25FIALAACONGELATORE);
			UpdateObjMesh(init, o25FIALAACONGELATOREFREDDA);
		}

//			se la fiala non e' OK ed il frigo e' chiuso...
		if ((init.Obj[o25CONGELATORE2CH].flags & ON) && (init.Obj[o25FIALAACONGELATORE].flags & ON) && (Frigo25 == 3)) {
			init.Obj[o25FIALAACONGELATORE].flags &= ~ON;
			init.Obj[o25FIALAACONGELATOREFREDDA].flags |= ON;
			UpdateObjMesh(init, o25FIALAACONGELATORE);
			UpdateObjMesh(init, o25FIALAACONGELATOREFREDDA);
		}

		break;
	case o25CONGELATORE2AP:
		if ((init.Obj[o25FIALAACONGELATORE].flags & ON) && (Frigo25 == 3)) {
			init.Obj[o25FIALAACONGELATORE].flags &= ~ON;
			init.Obj[o25FIALAACONGELATOREFREDDA].flags |= ON;
			UpdateObjMesh(init, o25FIALAACONGELATORE);
			UpdateObjMesh(init, o25FIALAACONGELATOREFREDDA);
		}

		if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		else
			sent = TRUE;
		break;

	case o25FORNOCH:
	case o25FORNOAP:
		if (WhichRoomChar(init, ocCUOCO) == r25)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dCUOCO_INTERRUPT1, 0, 0, nullptr, nullptr, nullptr);
		else if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		else
			sent = TRUE;
		break;

	case o25MOBILETTO:
		if (WhichRoomChar(init, ocCUOCO) == r25)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dCUOCO_INTERRUPT2, 0, 0, nullptr, nullptr, nullptr);
		else if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		else
			sent = TRUE;
		break;

	case o25SCAFFALEMAGAZZINO:
		if (WhichRoomChar(init, ocDOMESTICA) == r25)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dDOMESTICA_INTERRUPT1, 0, 0, nullptr, nullptr, nullptr);
		else if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		else
			sent = TRUE;
		break;

	case o28LIBRO:
	case o28ANTECXCH:
	case o28ANTEDXCH:
	case o28ANTESXCH:
		if (WhichRoomChar(init, ocDOMESTICA) == r28)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dDOMESTICA_INTERRUPT2, 0, 0, nullptr, nullptr, nullptr);
		else if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		else
			sent = TRUE;
		break;

	case o29COMPUTER:
		if (WhichRoomChar(init, ocSUPERVISORE) == r29)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dSUPERVISORE_INTERRUPT1, 0, 0, nullptr, nullptr, nullptr);
		else {
			if (init.Obj[obj].flags & EXTRA)
				Event(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, 0, 0, tSCANNER, nullptr, nullptr, nullptr);
			else
				sent = TRUE;
		}
		break;

	case o29SCHEDARIO:
	case o29PORTAFOTOCONJUDE:
	case o29SCANNER:
		if (WhichRoomChar(init, ocSUPERVISORE) == r29)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dSUPERVISORE_INTERRUPT1, 0, 0, nullptr, nullptr, nullptr);
		else if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		else
			sent = TRUE;
		break;

	case o29PORTA:
		if (WhichRoomChar(init, ocSUPERVISORE) == r29)
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dSUPERVISORE_INTERRUPT3, 0, 0, nullptr, nullptr, nullptr);
		else if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		else
			sent = TRUE;
		break;

	case o2DTASTIERINO:
		if (init.Obj[obj].anim[CurPlayer] == aTO1PERSON)
			sa = init.Obj[obj].anim[CurPlayer];
		else if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		UpdateSpecial(game, r2D);
		break;

	case o2D0:
	case o2D1:
	case o2D2:
	case o2D3:
	case o2D4:
	case o2D5:
	case o2D6:
	case o2D7:
	case o2D8:
	case o2D9:
		StartSound(game, w2D8);
		Comb2D[0] = Comb2D[1];
		Comb2D[1] = Comb2D[2];
		Comb2D[2] = Comb2D[3];
		Comb2D[3] = Comb2D[4];
		Comb2D[4] = obj;
		UpdateSpecial(game, r2D);
		break;

	case o2DC:
		StartSound(game, w2D9);
		Comb2D[4] = Comb2D[3];
		Comb2D[3] = Comb2D[2];
		Comb2D[2] = Comb2D[1];
		Comb2D[1] = Comb2D[0];
		Comb2D[0] = 0;
		UpdateSpecial(game, r2D);
		break;

	case o2DE:
		if ((Comb2D[0] == o2D1) && (Comb2D[1] == o2D2) && (Comb2D[2] == o2D4) && (Comb2D[3] == o2D8) && (Comb2D[4] == o2D0)) {
			init.Obj[o2DTASTIERINO].anim[CurPlayer] = init.Obj[o2DTASTIERINO].anim[CurPlayer ^ 1] = a2D11;
			Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
			CurObj = o2DTASTIERINO;
			Event(EventClass::MC_ACTION, ME_MOUSEOPERATE, MP_WAIT_CAMERA, 0, 0, bFirstPerson, &CurObj, nullptr, nullptr);
			StartSound(game, w2D10);
		} else
			StartSound(game, w2D9B);
		Comb2D[0] = Comb2D[1] = Comb2D[2] = Comb2D[3] = Comb2D[4] = 0;
		UpdateSpecial(game, r2D);
		break;

	case o2GFILTRODXAP:
		if ((init.Obj[o2GOFF].flags & EXTRA) && (init.Obj[obj].anim[CurPlayer])) {
			sa = init.Obj[obj].anim[CurPlayer];
			if (!(init.Obj[o2GFILTROSXAP].flags & ON))
				init.Obj[o2GOFF].flags &= ~EXTRA;
		} else
			sent = TRUE;
		break;
	case o2GFILTROSXAP:
		if ((init.Obj[o2GOFF].flags & EXTRA) && (init.Obj[obj].anim[CurPlayer])) {
			sa = init.Obj[obj].anim[CurPlayer];
			if (!(init.Obj[o2GFILTRODXAP].flags & ON))
				init.Obj[o2GOFF].flags &= ~EXTRA;
		} else
			sent = TRUE;
		break;

	case o2GOFF:
		if ((init.Obj[o2GFILTRODXCH].flags & ON) && (init.Obj[o2GFILTROSXCH].flags & ON)) {
			if ((!(init.InvObj[i2iDETERSIVO].flags & USEWITH)) && (!(init.InvObj[i2iDETERSIVO].flags & EXTRA2))) {
				init.InvObj[i2iDETERSIVO].flags |= EXTRA2;
				StartAnim(game, a2G9_parte_rtv);
			} else StartAnim(game, a2G9);
		} else
			PlayerSpeak(game, init.Obj[obj].text[0]);

		break;

	case o2OATASTOU:
	case o2OBTASTOU:
	case o2OCTASTOU:
	case o2ODTASTOU:
	case o2OETASTOU:
		bNoFirstPersonSwitch = FALSE;
		ChangeRoom(game, "r2h.t3d", 43, a2H7);
		if (obj == o2OATASTOU) Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, TRUE, nullptr, nullptr, nullptr);
		break;
	case o2OATASTO0:
	case o2OBTASTO0:
	case o2OCTASTO0:
	case o2ODTASTO0:
	case o2OETASTO0:
		bNoFirstPersonSwitch = FALSE;
		ChangeRoom(game, "r2m-a.t3d", 43, a2M1);
		if (obj == o2OBTASTO0) Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, TRUE, nullptr, nullptr, nullptr);
		break;
	case o2OATASTO1:
	case o2OBTASTO1:
	case o2OCTASTO1:
	case o2ODTASTO1:
	case o2OETASTO1:
		bNoFirstPersonSwitch = FALSE;
		ChangeRoom(game, "r2b-a.t3d", 43, a2B14);
		if (obj == o2OCTASTO1) Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, TRUE, nullptr, nullptr, nullptr);
		break;
	case o2OATASTO2:
	case o2OBTASTO2:
	case o2OCTASTO2:
	case o2ODTASTO2:
	case o2OETASTO2:
		bNoFirstPersonSwitch = FALSE;
		ChangeRoom(game, "r2p.t3d", 43, a2P6);
		if (obj == o2ODTASTO2) Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, TRUE, nullptr, nullptr, nullptr);
		break;
	case o2OATASTO3:
	case o2OBTASTO3:
	case o2OCTASTO3:
	case o2ODTASTO3:
	case o2OETASTO3:
		bNoFirstPersonSwitch = FALSE;
		ChangeRoom(game, "r2c.t3d", 43, a2C9);
		if (obj == o2OETASTO3) Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, TRUE, nullptr, nullptr, nullptr);
		break;

	case o2Pp2D:
		if ((init.Obj[o24BOCCIOLO].flags & EXTRA) && !(init.Dialog[dR2P1].flags & DIALOG_DONE))
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR2P1, 0, 0, nullptr, nullptr, nullptr);
		else if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		else
			sent = TRUE;
		break;

	case o2QTASTIERINO:
		if (init.Obj[obj].anim[CurPlayer] == aTO1PERSON)
			sa = init.Obj[obj].anim[CurPlayer];
		else if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];

		Comb2Q[0] = Comb2Q[1] = Comb2Q[2] = Comb2Q[3] = Comb2Q[4] = 0;
		UpdateSpecial(game, r2Q);
		break;

	case o2Q0:
	case o2Q1:
	case o2Q2:
	case o2Q3:
	case o2Q4:
	case o2Q5:
	case o2Q6:
	case o2Q7:
	case o2Q8:
	case o2Q9:
		StartSound(game, w2Q8);
		Comb2Q[0] = Comb2Q[1];
		Comb2Q[1] = Comb2Q[2];
		Comb2Q[2] = Comb2Q[3];
		Comb2Q[3] = Comb2Q[4];
		Comb2Q[4] = obj;
		UpdateSpecial(game, r2Q);
		break;

	case o2QC:
		StartSound(game, w2Q9);
		Comb2Q[4] = Comb2Q[3];
		Comb2Q[3] = Comb2Q[2];
		Comb2Q[2] = Comb2Q[1];
		Comb2Q[1] = Comb2Q[0];
		Comb2Q[0] = 0;
		UpdateSpecial(game, r2Q);
		break;

	case o2QE:
		if (IconInInv(init, i29FOTOJUDE1) && (Comb2Q[1] == o2Q9) && (Comb2Q[2] == o2Q7) && (Comb2Q[3] == o2Q9) && (Comb2Q[4] == o2Q5)) {
			init.Obj[o2QTASTIERINO].anim[CurPlayer] = init.Obj[o2QTASTIERINO].anim[CurPlayer ^ 1] = a2Q11;
			Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
			CurObj = o2QTASTIERINO;
			Event(EventClass::MC_ACTION, ME_MOUSEOPERATE, MP_WAIT_CAMERA, 0, 0, bFirstPerson, &CurObj, nullptr, nullptr);
			StartSound(game, w2Q10);
		} else
			StartSound(game, w2Q9B);
		Comb2Q[0] = Comb2Q[1] = Comb2Q[2] = Comb2Q[3] = Comb2Q[4] = 0;
		UpdateSpecial(game, r2Q);
		break;

	case o31LEVA:
		if ((init.Obj[o31DISCONUOVO].flags & ON) && (init.Obj[o31INGRALEGNO].flags & ON))
			sa = a312;
		else
			sa = a311;
		break;

	case o31LEVETTA1:
	case o31LEVETTA2:
	case o31LEVETTA3:
	case o31LEVETTA4:
	case o31LEVETTA5:
		if (init.Obj[o31CRISTALLOATTIVATO].flags & ON) {
			StartSound(game, w3118);
			if (++Comb31[obj - o31LEVETTA1] > 7) Comb31[obj - o31LEVETTA1] = 0;
			UpdateSpecial(game, r31);
			if ((Comb31[0] == 1) && (Comb31[1] == 6) && (Comb31[2] == 3) && (Comb31[3] == 5) && (Comb31[4] == 7)) {
				Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
				Event(EventClass::MC_ANIM, ME_STARTANIM, MP_WAIT_CAMERA, a319, 0, bFirstPerson, &CurObj, nullptr, nullptr);
			}
		} else
			sa = a3110;
		break;

	case o32LEVA: {
		if (!t3dCurRoom->name.equalsIgnoreCase(PlayerStand[CurPlayer ^ 1].roomName))
			sa = a323;
		else {
			if (CurPlayer == DARRELL) Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR321_DAR, 0, 0, nullptr, nullptr, nullptr);
			if (CurPlayer == VICTORIA) Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR321_VIC, 0, 0, nullptr, nullptr, nullptr);
			sa = aNULL;
			sent = FALSE;
		}
	}
	break;

	case o36PIATTODX:
		if (init.Obj[o36SESTERZODX].flags & ON) {
			if (init.Obj[o36INCENSOSX].flags & ON)
				sa = a36PSDM;
			else
				sa = a36PSDB;
		} else
			sent = TRUE;
		break;
	case o36PIATTOSX:
		if (init.Obj[o36SESTERZOSX].flags & ON) {
			if (init.Obj[o36INCENSODX].flags & ON)
				sa = a36PSSM;
			else
				sa = a36PSSB;
		} else
			sent = TRUE;
		break;

	case  o39ANGELO:
		if (((init.Obj[o39CRISTALLO].flags & ON) || (init.Obj[o39CRISTALLOATTIVATO].flags & ON)) && (init.Obj[o39SFERA].flags & ON) && (init.Obj[o39COLLANA].flags & ON)) {
			if (!(init.Obj[o39ANGELO].flags & EXTRA2)) {
				IncCurTime(game, 20);
				init.Obj[o39ANGELO].flags |= EXTRA2;
			}
			sa = a393;
		} else if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		else
			sent = TRUE;
		break;

	case o3BCOPPABRACCIALI:
		if (init.Obj[o3BSESTERZOBRACCIALI].flags & ON) {
			sa = a3B19;
			if (init.Obj[o3B1DOSEBRACCIALI].flags & ON) {
				init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPABRACCIALI].text[3];
			} else if (init.Obj[o3B2DOSIBRACCIALI].flags & ON) {
				init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPABRACCIALI].text[4];
			} else
				init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer] = init.Obj[o3BCOPPABRACCIALI].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPABRACCIALI].text[0];
		} else if ((init.Obj[o3B1DOSEBRACCIALI].flags & ON) || (init.Obj[o3B2DOSIBRACCIALI].flags & ON)) //se c'e' un incenso gli faccio dire che non puo' prenderli a mani nude
			PlayerSpeak(game, init.Obj[o3BCOPPABRACCIALI].text[1]);
		else
			sa = a3B24; //quando non ci sono incensi parte l'animazione che fa dondolare la cup
		break;

	case o3BCOPPA:
		if (init.Obj[o3BSESTERZO].flags & ON) {
			sa = a3B20;
			if (init.Obj[o3B1DOSE].flags & ON) {
				init.Obj[o3BCOPPA].examine[CurPlayer] = init.Obj[o3BCOPPA].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPA].text[3];
			} else if (init.Obj[o3B2DOSI].flags & ON) {
				init.Obj[o3BCOPPA].examine[CurPlayer] = init.Obj[o3BCOPPA].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPA].text[4];
			} else
				init.Obj[o3BCOPPA].examine[CurPlayer] = init.Obj[o3BCOPPA].examine[CurPlayer ^ 1] = init.Obj[o3BCOPPA].text[0];
		} else if ((init.Obj[o3B1DOSE].flags & ON) || (init.Obj[o3B2DOSI].flags & ON))   //se c'e' un incenso gli faccio dire che non puo' prenderli a mani nude
			PlayerSpeak(game, init.Obj[o3BCOPPA].text[1]);
		else
			sa = a3B25; //quando non ci sono incensi parte l'animazione che fa dondolare la cup
		break;

	case o42TASTO01SU:
	case o42TASTO02SU:
	case o42TASTO03SU:
	case o42TASTO04SU:
	case o42TASTO05SU:
	case o42TASTO06SU:
	case o42TASTO07SU:
	case o42TASTO08SU:
	case o42TASTO09SU:
	case o42TASTO10SU:
	case o42TASTO11SU:
	case o42TASTO12SU:
		StartSound(game, w422);
		Comb42[obj - o42TASTO01SU] = 1;
		init.Obj[obj].flags &= ~ON;
		init.Obj[obj + 12].flags |= ON;
		UpdateObjMesh(init, obj);
		UpdateObjMesh(init, obj + 12);
		if ((Comb42[0] == 0) && (Comb42[1] == 1) && (Comb42[2] == 0) && (Comb42[3] == 1) &&
		        (Comb42[4] == 1) && (Comb42[5] == 1) && (Comb42[6] == 1) && (Comb42[7] == 1) &&
		        (Comb42[8] == 0) && (Comb42[9] == 0) && (Comb42[10] == 1) && (Comb42[11] == 0) && IconInInv(init, i41OGGETTO)) {
			Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
			if (!(init.Dialog[dR421_fine].flags & DIALOG_DONE)) {
				if (CurPlayer == DARRELL)
					Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_WAIT_CAMERA, dR421_DAR, 0, 0, nullptr, nullptr, nullptr);
				if (CurPlayer == VICTORIA)
					Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_WAIT_CAMERA, dR421_VIC, 0, 0, nullptr, nullptr, nullptr);
			} else
				Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_WAIT_CAMERA, dR42_porta, 0, 0, nullptr, nullptr, nullptr);
		}
		break;
	case o42TASTO01GIU:
	case o42TASTO02GIU:
	case o42TASTO03GIU:
	case o42TASTO04GIU:
	case o42TASTO05GIU:
	case o42TASTO06GIU:
	case o42TASTO07GIU:
	case o42TASTO08GIU:
	case o42TASTO09GIU:
	case o42TASTO10GIU:
	case o42TASTO11GIU:
	case o42TASTO12GIU:
		StartSound(game, w424);
		Comb42[obj - o42TASTO01GIU] = 0;
		init.Obj[obj].flags &= ~ON;
		init.Obj[obj - 12].flags |= ON;
		UpdateObjMesh(init, obj);
		UpdateObjMesh(init, obj - 12);
		if ((Comb42[0] == 0) && (Comb42[1] == 1) && (Comb42[2] == 0) && (Comb42[3] == 1) &&
		        (Comb42[4] == 1) && (Comb42[5] == 1) && (Comb42[6] == 1) && (Comb42[7] == 1) &&
		        (Comb42[8] == 0) && (Comb42[9] == 0) && (Comb42[10] == 1) && (Comb42[11] == 0) && IconInInv(init, i41OGGETTO)) {
			Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
			if (!(init.Dialog[dR421_fine].flags & DIALOG_DONE)) {
				if (CurPlayer == DARRELL)
					Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_WAIT_CAMERA, dR421_DAR, 0, 0, nullptr, nullptr, nullptr);
				if (CurPlayer == VICTORIA)
					Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_WAIT_CAMERA, dR421_VIC, 0, 0, nullptr, nullptr, nullptr);
			} else
				Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_WAIT_CAMERA, dR42_porta, 0, 0, nullptr, nullptr, nullptr);
		}
		break;

	case o48KRENNSVENUTO:
		if (!(init.Dialog[dR48_chiavi].flags & DIALOG_DONE))
			Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR48_chiavi, 0, 0, nullptr, nullptr, nullptr);
		else
			Event(EventClass::MC_STRING, ME_PLAYERSPEAK, MP_DEFAULT, init.Obj[o48KRENNSVENUTO].examine[VICTORIA], 0, 0, nullptr, nullptr, nullptr);
		break;

	case o44LEVAORE:
	case o44LEVAMINUTI:
	case o44LEVAGRADI:
		sa = init.Obj[obj].anim[CurPlayer];
		Comb44[obj - o44LEVAORE] ++;
		if ((obj == o44LEVAORE) && (Comb44[obj - o44LEVAORE] > 12)) Comb44[obj - o44LEVAORE] = 1;
		else if ((obj == o44LEVAMINUTI) && (Comb44[obj - o44LEVAORE] > 12)) Comb44[obj - o44LEVAORE] = 1;
		else if ((obj == o44LEVAGRADI) && (Comb44[obj - o44LEVAORE] > 10)) Comb44[obj - o44LEVAORE] = 0;
		break;

	case o45INDICATORE1:
	case o45INDICATORE2:
	case o45INDICATORE3:
	case o45INDICATORE4:
	case o45INDICATORE5:
		if (++Comb45[obj - o45INDICATORE1] > 11) Comb45[obj - o45INDICATORE1] = 0;
		UpdateSpecial(game, r45);
		if ((Comb45[0] == 8) && (Comb45[1] == 1) && (Comb45[2] == 10) && (Comb45[3] == 5) && (Comb45[4] == 3)) {
			StartSound(game, wB451);
			Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
			SetMeshMaterialMovieFrame(LinkMeshToStr(init, "o45-ingranaggiBEK02"), 0, 1);
			SetMeshMaterialMovieFrame(LinkMeshToStr(init, "o45-ingranaggiBEK03"), 0, 1);
			SetMeshMaterialMovieFrame(LinkMeshToStr(init, "o45-ingranaggiBEK04"), 0, 1);
			SetMeshMaterialMovieFrame(LinkMeshToStr(init, "o45-ingranaggiBEK05"), 0, 1);
			SetMeshMaterialMovieFrame(LinkMeshToStr(init, "p45-ingras01"), 0, 1);

			//spengo l'ingranaggio abbassato
			init.Obj[o45INGRANAGGI].flags &= ~ON;
			UpdateObjMesh(init, o45INGRANAGGI);
			init.Obj[o45SANGUE].flags &= ~ON;
			UpdateObjMesh(init, o45SANGUE);

			//accendo l'ingranaggio alzato
			ChangeMeshFlags(LinkMeshToStr(init, "o45-ingranaggioALZATO"), -1, T3D_MESH_HIDDEN);
			ChangeMeshFlags(LinkMeshToStr(init, "o45-ingranaggioALZATO01"), -1, T3D_MESH_HIDDEN);
			if (init.Dialog[dR451].flags & DIALOG_DONE) ChangeMeshFlags(LinkMeshToStr(init, "o45-macchiedisangALZATO"), -1, T3D_MESH_HIDDEN);

			CurObj = oNULL;

			init.Sound[wB451].flags &= ~SOUND_ON;
			StopSound(wB451);
			SetBndLevel(game, nullptr, 1);
			SetBndLevel(game, "r47.t3d", 0);

			if (!(init.Obj[o45ALLOGGIAMENTO].flags & EXTRA)) {
				IncCurTime(game, 15);
				init.Obj[o45ALLOGGIAMENTO].flags |= EXTRA;
			}
		}
		if ((Comb45[0] == 0) && (Comb45[1] == 0) && (Comb45[2] == 0) && (Comb45[3] == 0) && (Comb45[4] == 0)) {
			StartSound(game, wB451);
			Event(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
			SetMeshMaterialMovieFrame(LinkMeshToStr(init, "o45-ingranaggiBEK02"), 0, 0xFFFF);
			SetMeshMaterialMovieFrame(LinkMeshToStr(init, "o45-ingranaggiBEK03"), 0, 0xFFFF);
			SetMeshMaterialMovieFrame(LinkMeshToStr(init, "o45-ingranaggiBEK04"), 0, 0xFFFF);
			SetMeshMaterialMovieFrame(LinkMeshToStr(init, "o45-ingranaggiBEK05"), 0, 0xFFFF);
			SetMeshMaterialMovieFrame(LinkMeshToStr(init, "p45-ingras01"), 0, 0xFFFF);
			CurObj = oNULL;

			//accendo l'ingranaggio abbassato
			init.Obj[o45INGRANAGGI].flags |= ON;
			UpdateObjMesh(init, o45INGRANAGGI);
			if (init.Dialog[dR451].flags & DIALOG_DONE) {
				init.Obj[o45SANGUE].flags |= ON;
				UpdateObjMesh(init, o45SANGUE);
			}

			//spengo l'ingranaggio alzato
			ChangeMeshFlags(LinkMeshToStr(init, "o45-ingranaggioALZATO"), +1, T3D_MESH_HIDDEN);
			ChangeMeshFlags(LinkMeshToStr(init, "o45-ingranaggioALZATO01"), +1, T3D_MESH_HIDDEN);
			ChangeMeshFlags(LinkMeshToStr(init, "o45-macchiedisangALZATO"), +1, T3D_MESH_HIDDEN);

			init.Sound[wB451].flags |= SOUND_ON;
			StartSound(game, wB451);
			SetBndLevel(game, nullptr, 0);
			SetBndLevel(game, "r47.t3d", 1);
			game._messageSystem.removeEvent(EventClass::MC_PLAYER, ME_PLAYERTIMER);

			if (init.Dialog[dR471].flags & DIALOG_DONE)
				Event(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR451, 0, 0, nullptr, nullptr, nullptr);
		}
		break;

	/*      //lo lanciamo da init
	        case o46CARTELLINA5:
	            if( !( Dialog[dR461].flags & DIALOG_DONE ) )
	                Event( EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR461, 0, 0, NULL, NULL, NULL );
	            else
	                sent = TRUE;
	            break;
	*/
	case o46SECONDODIAGRAMMA:
//			se il flag EXTRA2 non e' settato vuol dire che non si e' ancora entrati nella r48 e non si
//			puo' conoscere il significato del disegno
		if (!(init.Obj[o46SECONDODIAGRAMMA].flags & EXTRA2)) {
			PlayerSpeak(game, init.Obj[o13QUADRO1].action[CurPlayer]);
			return;
		} else    sent = TRUE;
		break;

	default:
		if (init.Obj[obj].anim[CurPlayer])
			sa = init.Obj[obj].anim[CurPlayer];
		else
			sent = TRUE;
		break;
	}

	if (sa == aTO1PERSON)
		ToFirstPersonSent = init.Obj[obj].action[CurPlayer];
	else if ((sent) && (init.Obj[obj].action[CurPlayer]))
		PlayerSpeak(game, init.Obj[obj].action[CurPlayer]);
	if (sa)
		StartAnim(game, sa);
}

} // End of namespace Watchmaker
