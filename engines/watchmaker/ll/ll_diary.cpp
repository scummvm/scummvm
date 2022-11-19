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

#define FORBIDDEN_SYMBOL_EXCEPTION_strcat
#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy

#include "watchmaker/game.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/ll/ll_mesh.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/define.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/3d/animation.h"

namespace Watchmaker {

char bDiariesStoppedByTimeInc = 0;

/* -----------------26/11/1999 16.19-----------------
 *                      StartDiary
 * --------------------------------------------------*/
void StartDiary(WGame &game, int32 room, t3dV3F *pos) {
	struct SDiary *d, *l;
	int32 tot_rand, cur_rand;
	int32 i, j, ca;
	t3dF32 acceptable_dist;
	char special_flag;
	uint8 cr;
	Init &init = game.init;

	if (t3dCurRoom) cr = (uint8)getRoomFromStr(init, t3dCurRoom->name);
	else cr = rNULL;

	if (bDisableDiary) return ;

//	DebugLogFile( "Parto Diario per room %d", room );

	for (i = 0, d = &init.Diary[0]; i < MAX_DIARIES; i++, d++) {
		if ((d->room != room) || (t3dCurTime < d->startt) || (d->endt && (t3dCurTime >= d->endt))) continue;
		if (d->item[d->cur].on) continue;
		if ((Character[d->obj]) && (Character[d->obj]->Flags & T3D_CHARACTER_DIARYDISABLE)) continue;
		if (init.Dialog[CurDialog].obj == d->obj) continue;  //se e' attivo un dialogo e si cerca di far partire il diario del personaggio che sta' gia' parlando

		for (j = 0, l = &init.Diary[0]; j < MAX_DIARIES; j++, l++)
			if ((l->obj == d->obj) && (l->item[l->cur].on))
				break;
		if (j < MAX_DIARIES) continue;

		if (cr != rNULL) {
			//se un diario e' fuori ma l'omino e' dentro non lo faccio partire
			if ((d->room == rXT) && (cr != rXT)) {
				DebugLogWindow("Skippato diario %d in rXT: obj %d room %d", i, d->obj, d->room);
				continue;
			}

			//se il diario e' nella r13 ma non siamo nella r13 skippo
			if ((d->room == r13) && (cr != r13)) {
				DebugLogWindow("Skippato diario %d in r13: obj %d room %d", i, d->obj, d->room);
				continue;
			}
		}

//		if( i==36)
//			DebugLogFile("START36 d->startt %d, d->endt %d, d->obj %d",d->startt,d->endt,d->obj);
//		DebugLogFile( "Diario valido num %d", i );

//		da usare per quei personaggi che, tra un diario e il successivo, cambiano posizione (sempre nel caso che un dialogo/rtv abbia incrementato il tempo e fatto cosi' switchare il diario)
		special_flag = 0;
		if (bDiariesStoppedByTimeInc && (d->obj == ocSERVETTA)) special_flag = 1;

		acceptable_dist = 40000.0f;

//		la servetta ha bisogno di piu' spazio vista la disposizione delle luci di posizione dei dialoghi e dei background
		if (d->obj == ocSERVETTA) acceptable_dist = 200000.0f;

//		quando il sup non c'� noi possiamo rubare la foto e quindi il portafoto diventa vuoto. Quando lui torna dobbiamo fare in modo che il portafoto sia rovesciato
		if ((d->obj == ocSUPERVISORE) && (d->room == r29) && (init.Obj[o29PORTAFOTOVUOTO].flags & ON) && (!(init.Obj[o29PORTAFOTOROVESCIATO].flags & ON))) {
			init.Obj[o29PORTAFOTOVUOTO].flags &= ~ON;
			UpdateObjMesh(init, o29PORTAFOTOVUOTO);
			init.Obj[o29PORTAFOTOROVESCIATO].flags |= ON;
			UpdateObjMesh(init, o29PORTAFOTOROVESCIATO);
		}

//		la domestica ha bisogno di piu' spazio in questa circostanza vista la disposizione delle luci di posizione dei dialoghi e dei background
		if ((d->obj == ocDOMESTICA) && ((d->room == r22) || (d->room == r26) || (d->room == r28)))        acceptable_dist = 200000.0f;

		tot_rand = 0;
		for (j = 0; j < MAX_ANIMS_PER_DIARY_ITEM; j++)
			if ((special_flag) || (pos == nullptr) || ((d->item[j].anim[0]) && (CompareLightPosition((char *) init.Anim[d->item[j].anim[0]].RoomName.rawArray(), init.Anim[d->item[j].anim[0]].pos, pos, acceptable_dist))))
				tot_rand += d->item[j].rand;

		if (!tot_rand) continue;
		game._rnd->setSeed((unsigned)t3dReadTime());
		cur_rand = game._rnd->getRandomNumber(tot_rand - 1);

//		DebugLogFile( "Random %d (%d)", cur_rand, tot_rand );

		tot_rand = 0;
		for (j = 0; j < MAX_ANIMS_PER_DIARY_ITEM; j++) {
			if ((special_flag) || (pos == nullptr) || ((d->item[j].anim[0]) && (CompareLightPosition((char *) init.Anim[d->item[j].anim[0]].RoomName.rawArray(), init.Anim[d->item[j].anim[0]].pos, pos, acceptable_dist)))) {
				tot_rand += d->item[j].rand;
//				DebugLogFile( "%d: %d (%d)", j, cur_rand, tot_rand );
				if (cur_rand < tot_rand)
					break;
			}
		}

		if (!(ca = d->item[j].anim[0]) || init.Anim[ca].active)
			continue;

//		DebugLogFile( "Parte anim %d char %d pos %d", ca, d->obj, Anim[ca].pos );
		Character[d->obj]->Flags &= ~T3D_CHARACTER_HIDE;

		d->cur = j;
		d->item[d->cur].on = TRUE;
		d->item[d->cur].cur = 0;
		d->item[d->cur].loopc = 0;
		if (d->end_hideobj) d->end_hideobj |= 0x8000; //se ha un oggetto assegnato setto anche il flag piu' alto

		init.Anim[ca].flags |= ANIM_DIARY;
		if ((d->item[d->cur].bnd < 255) && !(bPlayerSuBasamento && (d->room == rXT))) {
			d->item[d->cur].saved_bnd = GetBndLevel((char *) init.Anim[ca].RoomName.rawArray());
			SetBndLevel(game, (char *)init.Anim[ca].RoomName.rawArray(), d->item[d->cur].bnd);
		} else d->item[d->cur].saved_bnd = 255;

		if (d->obj) CharSetPosition(d->obj, init.Anim[ca].pos, (char *)init.Anim[ca].RoomName.rawArray());
		StartAnim(game, ca);
	}

	bDiariesStoppedByTimeInc = 0;

//	Fa partire animazione di stand dell'altro personaggio
	i = (CurPlayer ^ 1);
	if ((Character[i + ocDARRELL]->Flags & T3D_CHARACTER_HIDE) && (PlayerStand[i].cr == room)) {
		Character[i + ocDARRELL]->Flags &= ~T3D_CHARACTER_HIDE;
//		CharSetPosition( i+ocDARRELL, Anim[PlayerStand[i].an].pos, PlayerStand[i].RoomName );
		CharSetPosition(i + ocDARRELL, PlayerStand[i].pos, PlayerStand[i].roomName.c_str());
		StartAnim(game, PlayerStand[i].an);
	}
}

/* -----------------26/11/1999 16.39-----------------
 *                  StopDiary
 * --------------------------------------------------*/
void StopDiary(WGame &game, int32 room, int32 obj, uint8 only_overtime) {
	struct SDiary *d;
	int32 i, an;
	char is_overtime;
	uint8 cr;
	Init &init = game.init;

	if (t3dCurRoom) cr = (uint8)getRoomFromStr(init, t3dCurRoom->name);
	else cr = rNULL;

//	DebugLogFile( "Finisco Diario per room %d", room );

	for (i = 0, d = &init.Diary[0]; i < MAX_DIARIES; i++, d++) {
		if (obj && (obj != d->obj)) continue;

		if ((room > 0) && (d->room != room)) continue;

		if ((room < 0) && (room == cr)) continue;

		is_overtime = (t3dCurTime < d->startt) || ((d->endt != 0) && (t3dCurTime >= d->endt));

		if (only_overtime && (!is_overtime)) continue;

		if (is_overtime && (d->end_hideobj & 0x8000)) {  //se ha il bit settato significa che aveva un oggetto da nascondere e il diario era stato lanciato almeno una volta
			d->end_hideobj &= 0x7FFF; //rimuovo l'ultimo bit
			DebugLogFile("!! EndAnim !! per obj %d (%s)", d->end_hideobj, Character[d->obj]->Mesh->name.c_str());
			init.Obj[d->end_hideobj].flags &= ~ON;
			UpdateObjMesh(init, d->end_hideobj);
			d->end_hideobj = 0;

			//caso particolare dello sportello del bagno
			if ((d->room == r2M) && (d->obj == ocCUSTODE)) {
				init.Obj[o2MOGGETTICUSTODE_TOHIDE].flags |= ON;
				UpdateObjMesh(init, o2MOGGETTICUSTODE_TOHIDE);
			}

			//casi particolari delle porte dei diari della servetta
			if (i == eSERVETTA1) {
				init.Obj[o2CMEGABB_2R].flags &= ~ON;
				UpdateObjMesh(init, o2CMEGABB_2R);
//				stanze in cui la r2C e la r2R sono caricate
				if ((cr == r2C) || (cr == r2F) || (cr == r15) || (cr == r2E) || (cr == r2R) || (cr == r2T) || (cr == r2S) || (cr == r31)) {
					t3dResetMesh(LinkMeshToStr(init, "o2c-portacamere02"));
					t3dResetMesh(LinkMeshToStr(init, "o2r-portacamere2c01"));
				}
			}
			if (i == eSERVETTA2) {
				init.Obj[o2CMEGABB_2S].flags &= ~ON;
				UpdateObjMesh(init, o2CMEGABB_2S);
//				stanze in cui la r2C e la r2R sono caricate
				if ((cr == r2C) || (cr == r2F) || (cr == r15) || (cr == r2E) || (cr == r2R) || (cr == r2T) || (cr == r2S) || (cr == r31)) {
					t3dResetMesh(LinkMeshToStr(init, "o2c-portacamere06"));
					t3dResetMesh(LinkMeshToStr(init, "o2s-portacamere01"));
				}
			}
			if (i == eSERVETTA4) {
				init.Obj[o2PMEGABB_2D].flags &= ~ON;
				UpdateObjMesh(init, o2PMEGABB_2D);
//				stanze in cui la r2P e la r2D sono caricate
				if ((cr == r27) || (cr == r2P) || (cr == r2D)) {
					t3dResetMesh(LinkMeshToStr(init, "o2p-portasup"));
					t3dResetMesh(LinkMeshToStr(init, "o2d-portasup"));
				}
			}

		}//end_hideobj

//		da mettere dopo "end_hideobj", perche' anche se il diario non e' attivo gli oggetti li devo spegnere lo stesso
		if (!d->item[d->cur].on)    continue;

		an = d->item[d->cur].anim[d->item[d->cur].cur];
		init.Anim[an].flags &= ~ANIM_DIARY;
		StopAnim(game, an);
		if ((d->item[d->cur].saved_bnd < 255)) {
			if (!(bPlayerSuBasamento && (d->room == rXT)))
				SetBndLevel(game, (char *)init.Anim[an].RoomName.rawArray(), d->item[d->cur].saved_bnd);
			d->item[d->cur].saved_bnd = 255;
		}
		Character[d->obj]->Flags |= T3D_CHARACTER_HIDE;
//		DebugLogWindow("%s HIDEEEEEEEE: Flags %d",Character[d->obj]->Mesh->Name,Character[d->obj]->Flags);
		d->item[d->cur].on = FALSE;
		d->item[d->cur].loopc = 0;
		d->item[d->cur].cur = 0;
		d->cur = 0;
	}

//	Termina animazione di stand dell'altro personaggio
	if (!obj) {
		i = (CurPlayer ^ 1);
		if (!(Character[i + ocDARRELL]->Flags & T3D_CHARACTER_HIDE) && (PlayerStand[i].cr == room)) {
			StopObjAnim(game, i + ocDARRELL);
			Character[i + ocDARRELL]->Flags |= T3D_CHARACTER_HIDE;
		}
	}
}

/* -----------------26/11/1999 16.49-----------------
 *                  ContinueDiary
 * --------------------------------------------------*/
void ContinueDiary(WGame &game, int32 an) {
	struct SDiary *d;
	int32 i, ca, pos;
	Init &init = game.init;

//	DebugLogFile( "Continuo Diario per anim %d", an );

	pos = -1;
	for (i = 0, d = &init.Diary[0]; i < MAX_DIARIES; i++, d++) {
		if ((!d->item[d->cur].on) || (d->item[d->cur].anim[d->item[d->cur].cur] != an)) continue;

		pos = init.Anim[an].pos;
		d->item[d->cur].cur ++;
		if (!(ca = d->item[d->cur].anim[d->item[d->cur].cur]) || init.Anim[ca].active) {
			if ((!d->item[d->cur].loop) || !(ca = d->item[d->cur].anim[0]) || init.Anim[ca].active ||
			        ((d->item[d->cur].loop > 0) && ((d->item[d->cur].loopc + 1) >= d->item[d->cur].loop))) {
				if (!d->item[d->cur].anim[d->item[d->cur].cur])
					d->item[d->cur].cur --; //altrimenti quando stoppo il tutto becca un item vuoto
				StopDiary(game, d->room, d->obj, 0);
				/*              d->item[d->cur].on = FALSE;
				                d->item[d->cur].cur = 0;
				                d->item[d->cur].loopc = 0;
				                d->cur = 0;*/
				if (Character[d->obj] && Character[d->obj]->Mesh)
					StartDiary(game, d->room, &Character[d->obj]->Mesh->Trasl);
				else
					StartDiary(game, d->room, nullptr);
				break;
			} else {
				if (d->item[d->cur].loop > 0) d->item[d->cur].loopc ++;
				d->item[d->cur].cur = 0;
				ca = d->item[d->cur].anim[d->item[d->cur].cur];
			}
		}

//		DebugLogFile( "Parte anim %d char %d pos %d", ca, d->obj, Anim[ca].pos );
		init.Anim[an].flags &= ~ANIM_DIARY;
		init.Anim[ca].flags |= ANIM_DIARY;
		CharSetPosition(d->obj, init.Anim[ca].pos, (char *)init.Anim[ca].RoomName.rawArray());
		StartAnim(game, ca);
		break;
	}
}

/* -----------------29/05/00 16.57-------------------
 *                  UpdateAllClocks
 * --------------------------------------------------*/
void UpdateAllClocks(WGame &game) {
	void t3dLoadOutdoorLights(const char *pname, t3dBODY * b, int32 ora);
	char str[255];
	t3dMESH *mesh;
	Init &init = game.init;

	const char *ClockMeshes[] = {
		"o21-a-ore01",
		"o21-a-minuti01",
		"o24-a-ore01",
		"o24-a-minuti01",
		"o2p-ore01",
		"o2p-minuti01",
		"o2s-ore01",
		"o2s-minuti01",
		nullptr
	};
	const char *ClockAnims[] = {
		"r21-a-orologio-ore.a3d",
		"r21-a-orologio-minuti.a3d",
		"r24-a-orologio-ore.a3d",
		"r24-a-orologio-minuti.a3d",
		"r2p-orologio-ore.a3d",
		"r2p-orologio-minuti.a3d",
		"r2s-orologio-ore.a3d",
		"r2s-orologio-minuti.a3d",
		nullptr
	};
	const char *ClockMeshes24[] = {
		"o48-lancettaore",
		"o48-lancettaore01",
		nullptr
	};
	const char *ClockAnims24[] = {
		"r48-ore.a3d",
		"r48-minuti.a3d",
		nullptr
	};
	int32 i, l[2];

	l[0] = (t3dCurTime / 100) % 12;
	l[1] = (t3dCurTime % 100) / 5;
	if (!l[0]) l[0] = 12;
	if (!l[1]) l[1] = 12;
//	DebugLogWindow("%d: %d %d",t3dCurTime,l[0],l[1]);

	for (i = 0;; i++) {
		if (ClockMeshes[i] == nullptr) break;
		if ((mesh = LinkMeshToStr(init, ClockMeshes[i])) == nullptr) continue;

		t3dSetSpecialAnimFrame(game, ClockAnims[i], mesh, l[i % 2]);
	}

	//orologio a 24ore (avanti di un'ora)
	l[0] = ((t3dCurTime + 100) / 100) % 24;
	l[1] = (t3dCurTime % 100) / 5;
	if (!l[0]) l[0] = 24;
	if (!l[1]) l[1] = 12;

	for (i = 0;; i++) {
		if (ClockMeshes24[i] == nullptr) break;
		if ((mesh = LinkMeshToStr(init, ClockMeshes24[i])) == nullptr) continue;

		t3dSetSpecialAnimFrame(game, ClockAnims24[i], mesh, l[i % 2]);
	}

	strcpy(str, game.workDirs._lightmapsDir.c_str());
	strcat(str, "rxt.t3d");
	if (t3dRxt)
		t3dLoadOutdoorLights(str, t3dRxt, t3dCurTime);
}

/* -----------------22/05/00 10.03-------------------
 *                  IncCurTime
 * --------------------------------------------------*/
void IncCurTime(WGame &game, int32 inc) {
	int32 h, m;
	Init &init = game.init;

	t3dCurTime += inc;
	h = (t3dCurTime / 100);
	m = (t3dCurTime % 100);

	while (m >= 60) {
		m -= 60;
		h ++;
	}

	t3dCurTime = h * 100 + m;
	UpdateAllClocks(game);

	if (bDialogActive && (init.Dialog[CurDialog].obj == ocSERVETTA)) {
		StopDiary(game, 0, init.Dialog[CurDialog].obj, 1);
		bDiariesStoppedByTimeInc = 1;
	}

//	stoppo tutti i diari overtime di tutte le stanze esclusa la attuale
	StopDiary(game, -1, 0, 1);


	//la moglie del supervisore va in camera sua e non ci fa entrare in camera
	if ((t3dCurTime >= 1700) && (t3dCurTime < 1720)) {
		init.Obj[o2Pp2D].anim[CurPlayer] = a2P7;
		init.Obj[o2Pp2D].anim[CurPlayer ^ 1] = a2P7;
		init.Obj[o2Pp2D].pos = 2;
	}
	//il cuoco va a letto e noi non lo disturbiamo
	if ((t3dCurTime >= 1430) && (t3dCurTime < 1450)) {
		init.Obj[o2Cp2T].anim[CurPlayer] = a2C4_CUOCODORME;
		init.Obj[o2Cp2T].anim[CurPlayer ^ 1] = a2C4_CUOCODORME;
	}
	//il cuoco si alza e ci lascia entrare nella r2t
	if ((t3dCurTime >= 1715) && (t3dCurTime < 1735)) {
		init.Obj[o2Cp2T].anim[CurPlayer] = a2C4;
		init.Obj[o2Cp2T].anim[CurPlayer ^ 1] = a2C4;
	}
	//il supervisore ci da le foto
	if ((t3dCurTime >= 1900) && !(init.Dialog[dR008].flags & DIALOG_DONE)) {
		init.Obj[o2Qp29].anim[CurPlayer] = a2Q4b;
		init.Obj[o2Qp29].anim[CurPlayer ^ 1] = a2Q4b;
	}
	//la domestica ci cazzia
	if ((t3dCurTime >= 1715) && !(init.Dialog[dR015].flags & DIALOG_DONE)) {
		init.Obj[o25p2Q].anim[CurPlayer] = a2513_DOM;
		init.Obj[o25p2Q].anim[CurPlayer ^ 1] = a2513_DOM;

		init.Obj[o25p24].anim[CurPlayer] = a2512_DOM;
		init.Obj[o25p24].anim[CurPlayer ^ 1] = a2512_DOM;

		init.Obj[oXT11p21].anim[CurPlayer] = a111_DOM;
		init.Obj[oXT11p21].anim[CurPlayer ^ 1] = a111_DOM;

		init.Obj[oXT1Ap22].anim[CurPlayer] = a1A4_DOM;
		init.Obj[oXT1Ap22].anim[CurPlayer ^ 1] = a1A4_DOM;
	}
}

/* -----------------22/05/00 10.03-------------------
 *                  DecCurTime
 * --------------------------------------------------*/
void DecCurTime(WGame &game, int32 dec) {
	int32 h, m;

	t3dCurTime -= dec;
	h = (t3dCurTime / 100);
	m = (t3dCurTime % 100);

	while (m < 0) {
		m += 60;
		h --;
	}

	t3dCurTime = h * 100 + m;
	UpdateAllClocks(game);
}

/* -----------------29/05/00 15.40-------------------
 *                  SetCurTime
 * --------------------------------------------------*/
void SetCurTime(WGame &game, int32 set) {
	t3dCurTime = set;
	UpdateAllClocks(game);
}

/* -----------------02/06/00 10.10-------------------
 *                  WhichRoomChar
 * --------------------------------------------------*/
int32 WhichRoomChar(Init &init, int32 ch) {
	struct SDiary *d;
	int32 i;

	for (i = 0, d = &init.Diary[0]; i < MAX_DIARIES; i++, d++) {
		if ((d->obj != ch) || (t3dCurTime < d->startt) || (d->endt && (t3dCurTime >= d->endt))) continue;
		return d->room;
	}
	return rNULL;
}

/* -----------------02/06/00 10.26-------------------
 *                  WhichAnimChar
 * --------------------------------------------------*/
int32 WhichAnimChar(Init &init, int32 ch) {
	struct SDiary *d;
	int32 i;

	for (i = 0, d = &init.Diary[0]; i < MAX_DIARIES; i++, d++) {
		if ((d->obj != ch) || (t3dCurTime < d->startt) || (d->endt && (t3dCurTime >= d->endt))) continue;
		if (!d->item[d->cur].on) continue;
		return d->item[d->cur].anim[d->item[d->cur].cur];
	}
	return 0;
}

/* -----------------02/06/00 10.26-------------------
 *                  WhichPosChar
 * --------------------------------------------------*/
uint8 WhichPosChar(Init &init, int32 ch) {
	struct SDiary *d;
	int32 i;

	for (i = 0, d = &init.Diary[0]; i < MAX_DIARIES; i++, d++) {
		if ((d->obj != ch) || (t3dCurTime < d->startt) || (d->endt && (t3dCurTime >= d->endt))) continue;
		if (!d->item[d->cur].on) continue;
		return init.Anim[d->item[d->cur].anim[d->item[d->cur].cur]].pos;
	}
	return 0;
}

} // End of namespace Watchmaker
