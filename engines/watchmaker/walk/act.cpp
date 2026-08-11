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

#include "watchmaker/walk/act.h"
#include "watchmaker/t3d.h"
#include "watchmaker/globvar.h"
#include "watchmaker/define.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/walk/walk.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/3d/animation.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/message.h"
#include "watchmaker/schedule.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/walk/walkutil.h"

namespace Watchmaker {

int32 NumCurve;

void FixupCurAction(int32 oc);

/* -----------------28/04/98 17.51-------------------
 *                  SlideChar
 * --------------------------------------------------*/
void SlideChar(int32 oc) {
	t3dCHARACTER *Ch = Character[oc];
	t3dWALK *w = &Ch->Walk;
	t3dF32 r, len, x1, x2, z1, z2;
	int16 nf;
	t3dV3F v;

	if (/*!( w->Check & CLICKINTO ) &&*/ (w->CurPanel < 0)) {
		CharStop(oc);
		return ;
	}

	v.x = w->Look.x;
	v.y = 0.0f;
	v.z = w->Look.z;

	x1 = w->Panel[w->CurPanel].a.x;
	z1 = w->Panel[w->CurPanel].a.z;
	x2 = w->Panel[w->CurPanel].b.x;
	z2 = w->Panel[w->CurPanel].b.z;
	if ((len = (x1 - x2) * (x1 - x2) + (z1 - z2) * (z1 - z2)) == 0) {
		CharStop(oc);
		return ;
	}

	r = ((z1 - v.z) * (z1 - z2) - (x1 - v.x) * (x2 - x1)) / len;

	if (r > 1.0f) {  // a destra di 2
		x1 = x2;
		z1 = z2;
	} else if (r > 0.0f) { // dentro 1..2
		x1 += r * (x2 - x1);
		z1 += r * (z2 - z1);
	}

	nf = w->CurFrame + 1;
	if ((w->CurAction == aWALK_START) || (w->CurAction == aWALK_LOOP) || (w->CurAction == aWALK_END)) {
		if (nf >= (ActionStart[aWALK_LOOP] + ActionLen[aWALK_LOOP]))
			nf = ActionStart[aWALK_LOOP];
	} else if ((w->CurAction == aBACK_START) || (w->CurAction == aBACK_LOOP) || (w->CurAction == aBACK_END)) {
		if (nf >= (ActionStart[aBACK_LOOP] + ActionLen[aBACK_LOOP]))
			nf = ActionStart[aBACK_LOOP];
	} else if ((w->CurAction == aRUN_START) || (w->CurAction == aRUN_LOOP) || (w->CurAction == aRUN_END)) {
		if (nf >= (ActionStart[aRUN_LOOP] + ActionLen[aRUN_LOOP]))
			nf = ActionStart[aRUN_LOOP];
	}

	w->NumPathNodes = w->CurrentStep = 0;

	w->WalkSteps[0].curp = w->CurPanel;
	w->WalkSteps[0].Angle = SinCosAngle(Ch->Dir.x, Ch->Dir.z);
	w->WalkSteps[0].Frame = nf;

	w->WalkSteps[0].Pos.x = x1;
	w->WalkSteps[0].Pos.y = CurFloorY;
	w->WalkSteps[0].Pos.z = z1;

	w->WalkSteps[1].Act = 0;

	w->NumSteps = 1;
}

/* -----------------07/05/98 11.15-------------------
 *                  UpdateChar
 * --------------------------------------------------*/
void UpdateChar(WGame &game, int32 oc, t3dF32 Speed, t3dF32 Rot) {
	t3dCHARACTER *Char = Character[oc];
	t3dWALK *w = &Char->Walk;
	t3dV3F Pos, tmp;
	t3dM3X3F mx;

	if (!Char) return ;

	if ((Speed == 0.0f) && (w->NumSteps == 0) && ((w->CurAction != aROT_DX && w->CurAction != aROT_SX) || (Rot == 0.0f))) {
		if ((Char->Mesh->Flags & T3D_MESH_DEFAULTANIM))
			CharStop(oc);
		return;
	}

	_vm->_messageSystem.doEvent(EventClass::MC_MOUSE, ME_MOUSEHIDE, MP_DEFAULT, 0, 0, 0, NULL, NULL, NULL);
	if (Char && Speed)
		CharNextFrame(game, oc);

//	Ruota l'omino
	t3dVectCopy(&tmp, &Char->Dir);
	tmp.z = -tmp.z;
	tmp.y = 0.0f;
	t3dVectAdd(&tmp, &Char->Pos, &tmp);
	t3dMatView(&Char->Mesh->Matrix, &Char->Pos, &tmp);
	t3dMatRot(&mx, 0.0f, Rot, 0.0f);
	t3dMatMul(&Char->Mesh->Matrix, &mx, &Char->Mesh->Matrix);
	Char->Mesh->Matrix.Flags &= ~T3D_MATRIX_IDENTITY;

	t3dVectInit(&Char->Dir, 0.0f, 0.0f, -1.0f);
	t3dVectTransform(&Char->Dir, &Char->Dir, &Char->Mesh->Matrix);   //rotate by Character angle

	if (Speed) {
		FloorHit = 1;
		Pos.y = CurFloorY;
		if (Speed > 0.0f) {
			if (bFastWalk) {
				if ((w->CurFrame >= ActionStart[aRUN_END]) || (w->CurFrame < ActionStart[aRUN_START]))
					w->CurFrame = ActionStart[aRUN_START];
			} else if ((w->CurFrame >= ActionStart[aWALK_END]) || (w->CurFrame < ActionStart[aWALK_START]))
				w->CurFrame = ActionStart[aWALK_START];
		} else if ((Speed < 0.0f) && ((w->CurFrame >= ActionStart[aBACK_END]) || (w->CurFrame < ActionStart[aBACK_START])))
			w->CurFrame = ActionStart[aBACK_START];
		Speed = - Char->Mesh->DefaultAnim.Dist[w->CurFrame + 1] + Char->Mesh->DefaultAnim.Dist[w->CurFrame];
		FixupCurAction(ocCURPLAYER);

		tmp = Char->Dir * Speed;
		t3dVectAdd(&Pos, &Char->Pos, &tmp);
		PlayerPos[CurPlayer + ocDARRELL] = 0;
		PlayerGotoPos[CurPlayer + ocDARRELL] = 0;
		CheckCharacterWithBounds(game, oc, &Pos, 0, (uint8)((Speed < 0.0f) ? 2 : (bFastWalk ? 1 : 0)));
		if (!(Char->Walk.Check & CLICKINTO) && (Char->Walk.NumSteps)) {
//          fa solo 2 frames: il primo come l'attuale
			Char->Walk.WalkSteps[1].curp = Char->Walk.WalkSteps[Char->Walk.NumSteps - 1].curp;
			Char->Walk.WalkSteps[0].curp = Char->Walk.WalkSteps[Char->Walk.NumSteps - 1].curp;
			Char->Walk.WalkSteps[1].Angle = Char->Walk.WalkSteps[0].Angle;
			Char->Walk.WalkSteps[0].Angle = Char->Walk.WalkSteps[0].Angle;
			Char->Walk.WalkSteps[2].Act = 0;
			Char->Walk.NumSteps = 2;
			Char->Walk.CurrentStep = 0;
		} else
			SlideChar(oc);

		game._messageSystem.removeEvent(EventClass::MC_PLAYER, ME_ALL);
		_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERGOTO, MP_DEFAULT, 0, 0, 0, NULL, NULL, NULL);
	}
}

/* -----------------28/05/98 10.38-------------------
 *                  CharStop
 * --------------------------------------------------*/
void CharStop(int32 oc) {
	t3dCHARACTER *Char = Character[oc];
	t3dV3F tmp;

	if (!Char) return;
	if ((Char->Mesh->CurFrame <= ActionStart[aSTAND]) && (Char->Mesh->CurFrame > 0)) return ;
	if (Char->Walk.CurAction != aSTAND)
		Char->Mesh->BlendPercent = 0;

	Char->Walk.NumPathNodes = Char->Walk.CurrentStep = Char->Walk.NumSteps = 0;
	Char->Walk.CurAction = aSTAND;
	Char->Walk.CurFrame = ActionStart[aSTAND];
	Char->Mesh->CurFrame = Char->Walk.CurFrame;
//	Char->Pos.y = Char->Mesh->Trasl.y = CurFloorY;

	t3dVectCopy(&tmp, &Char->Dir);
	tmp.z = -tmp.z;
	tmp.y = 0.0f;
	t3dVectAdd(&tmp, &Char->Pos, &tmp);
	t3dMatView(&Char->Mesh->Matrix, &Char->Pos, &tmp);
	Char->Mesh->Matrix.Flags &= ~T3D_MATRIX_IDENTITY;
}

/* -----------------28/05/98 10.38-------------------
 *                  CharSetPosition
 * --------------------------------------------------*/
void CharSetPosition(int32 oc, uint8 pos, const char *room) {
	t3dBODY *OldCurRoom = t3dCurRoom;
	t3dCHARACTER *Char = Character[oc];
	t3dV3F tmp;

	CharStop(oc);
	if (pos == 99) return;
	if (room && (room[0] != '\0')) {
		t3dBODY *roomPtr = _vm->_roomManager->getRoomIfLoaded(room);
		if (roomPtr) {
			t3dCurRoom = roomPtr;
		}
	}

	if (pos) {
		GetLightDirection(&tmp, pos);
		if ((oc == ocCURPLAYER) || (oc == ocDARRELL && !CurPlayer) || (oc == ocVICTORIA && CurPlayer)) {
			if ((PlayerPos[CurPlayer + ocDARRELL] = GetLightPosition(&Char->Pos, pos)) == 0) {
				t3dCurRoom = OldCurRoom;
				return ;
			}
			PlayerGotoPos[CurPlayer + ocDARRELL] = 0;
		} else
			PlayerPos[oc] = GetLightPosition(&Char->Pos, pos);
	}

	t3dVectCopy(&Char->Mesh->Trasl, &Char->Pos);
//	Ruota l'omino
	t3dVectSub(&tmp, &tmp, &Char->Pos);
	tmp.z = -tmp.z;
	tmp.y = 0.0f;
	t3dVectAdd(&tmp, &Char->Pos, &tmp);
	t3dMatView(&Char->Mesh->Matrix, &Char->Pos, &tmp);
	Char->Mesh->Matrix.Flags &= ~T3D_MATRIX_IDENTITY;

	Char->Dir.x = 0.0f; //first frame direction
	Char->Dir.y = 0.0f;
	Char->Dir.z = -1.0f;
	t3dVectTransform(&Char->Dir, &Char->Dir, &Char->Mesh->Matrix);   //rotate by Character angle

	t3dCurRoom = OldCurRoom;
}

/* -----------------23/03/99 12.00-------------------
 *              CheckCharacterWithoutBounds
 * --------------------------------------------------*/
void CheckCharacterWithoutBounds(WGame &game, int32 oc, const uint8 *dpl, uint8 back) {
	t3dCHARACTER *Char = Character[oc];
	t3dWALK *w = &Char->Walk;
	t3dV3F tmp;
	uint8 dp;

	if (!Char) return;
	StopObjAnim(game, oc);

	// Reset some vars
	if (!(Char->Mesh->Flags & T3D_MESH_DEFAULTANIM))
		FixPos(oc);

	Char->Walk.Panel = t3dCurRoom->Panel[t3dCurRoom->CurLevel];
	Char->Walk.PanelNum = t3dCurRoom->NumPanels[t3dCurRoom->CurLevel];
	if (t3dCurRoom) {
		CurFloorY = t3dCurRoom->PanelHeight[t3dCurRoom->CurLevel];
	}

	Char->Mesh->Flags |= T3D_MESH_DEFAULTANIM;
	t3dVectCopy(&Char->Pos, &Char->Mesh->Trasl);

	w->NumPathNodes = 0;
	w->PathNode[w->NumPathNodes].pos.x = Char->Pos.x;
	w->PathNode[w->NumPathNodes].pos.z = Char->Pos.z;
	w->PathNode[w->NumPathNodes].oldp = w->CurPanel;
	w->PathNode[w->NumPathNodes].curp = -1;
	w->PathNode[w->NumPathNodes].dist = 0.0f;
	w->NumPathNodes ++;
	w->CurPanel = -1;
	w->Check = NOBOUNDCHECK | NOTSKIPPABLE;

	dp = 0;
	while (*dpl && GetLightPosition(&tmp, *dpl)) {
		dp = *dpl++;
		w->PathNode[w->NumPathNodes].pos.x = tmp.x;
		w->PathNode[w->NumPathNodes].pos.z = tmp.z;
		w->PathNode[w->NumPathNodes].oldp = -1;
		w->PathNode[w->NumPathNodes].curp = -1;
		w->PathNode[w->NumPathNodes].dist = t3dVectDistance(&tmp, &Char->Pos);
		w->NumPathNodes ++;

		Char->Walk.Cur.x = tmp.x;
		Char->Walk.Cur.z = tmp.z;
	}
	bNotSkippableWalk = TRUE;
	BuildStepList(oc, dp, back);
	_vm->_messageSystem.deleteWaitingMsgs(MP_WAIT_PORTAL);
	_vm->_messageSystem.deleteWaitingMsgs(MP_WAIT_ANIM);
	_vm->_messageSystem.deleteWaitingMsgs(MP_WAIT_ACT);
}

/* -----------------01/07/98 16.10-------------------
 *                  FixPos
 * --------------------------------------------------*/
void FixPos(int32 oc) {
	t3dCHARACTER *Ch = Character[oc];
	t3dMESH *mesh = Ch->Mesh;

	if (t3dCurRoom) {
		CurFloorY = t3dCurRoom->PanelHeight[t3dCurRoom->CurLevel];
	}
	mesh->Trasl.y = CurFloorY;
	t3dVectCopy(&Ch->Pos, &mesh->Trasl);
	t3dVectInit(&Ch->Dir, 0.0f, 0.0f, -1.0f);
	t3dVectTransform(&Ch->Dir, &Ch->Dir, &Ch->Mesh->Matrix);     //rotate by Character angle

	if (oc == ocCURPLAYER) {
		PlayerPos[CurPlayer + ocDARRELL] = 0;
		PlayerGotoPos[CurPlayer + ocDARRELL] = 0;
	} else {
		PlayerPos[oc] = 0;
		PlayerGotoPos[oc] = 0;
	}
	CharStop(oc);
}

/* -----------------27/04/98 15.12-------------------
 *                  BuildStepList
 * --------------------------------------------------*/
void BuildStepList(int32 oc, uint8 dp, uint8 back) {
	t3dCHARACTER *Ch = Character[oc];
	t3dWALK *w = &Ch->Walk;
	int16 LastStep, CurF;
	t3dF32 len, curlen, startpos, angle, approx;
	t3dV3F st, en, direction;
	t3dF32 lastangle, *dist, bc;
	int16 ws, wl, we;
	int32 i, j, na, nla, nca;

	if (!Ch || !w) return;

	for (int step = 0; step < T3D_MAX_WALKSTEPS; step++) {
		w->WalkSteps[step] = t3dSTEPS();
	}

	if (w->NumPathNodes < 2) {
		w->NumSteps = 0;
		w->CurrentStep = 0;
		w->NumPathNodes = 0;
		w->Check = 0;
		return ;
	}

	len = 0.0;
	curlen = 0.0;
	startpos = 0.0;
	lastangle = SinCosAngle(Ch->Dir.x, Ch->Dir.z);
//	calcola lunghezza totale del percorso
	for (i = 0; i < w->NumPathNodes - 1; i++) {
		st.x = w->PathNode[i].pos.x;
		st.y = 0.0;
		st.z = w->PathNode[i].pos.z;
		en.x = w->PathNode[i + 1].pos.x;
		en.y = 0.0;
		en.z = w->PathNode[i + 1].pos.z;

		len += t3dVectDistance(&st, &en);               // dist of two points
	}
//	Se il percorso e' piu' lungo di mezzo omino seleziona modalita' percorso lungo
	if (fabs(len) > (ONE_STEP)) w->Check |= LONGPATH;

//	Cerca di capire se gli conviene andare avanti o indietro
	if (back >= 10) {
		t3dVectInit(&st, w->PathNode[1].pos.x - w->PathNode[0].pos.x, 0.0f, w->PathNode[1].pos.z - w->PathNode[0].pos.z);

		if (!(w->Check & LONGPATH) && (fabs(t3dVectAngle(&Ch->Dir, &st)) > 145.0f)) {        // devo andare dietro
//			DebugFile("Back %f\n%f %f | %f %f\n",t3dVectAngle( &Ch->Dir, &st ),Ch->Dir.x,Ch->Dir.z,st.x,st.z);
			back = 2;

			if ((dp) && (GetLightDirection(&st, dp)) && (st.x != 0.0f) && (st.z != 0.0f) &&
			        (GetLightPosition(&en, dp)) && (en.x != 0.0f) && (en.z != 0.0f)) {
				t3dVectSub(&en, &st, &en);
				if (fabs(t3dVectAngle(&Ch->Dir, &en)) > 145.0f)
					back = 0;
			}
		} else {
			if (fabs(len) > 40.0f * ONE_STEP)
				back = 1;
			else
				back = 0;
		}

		if (bFastWalk && !back) back = 1;
	}
//	Seleziona azioni per camminare avanti o indietro
	if (back == 1) {
		ws = aRUN_START;
		wl = aRUN_LOOP;
		we = aRUN_END;
		bc = 1.0f;
	} else if (back == 2) {
		ws = aBACK_START;
		wl = aBACK_LOOP;
		we = aBACK_END;
		bc = -1.0f;
	} else {
		ws = aWALK_START;
		wl = aWALK_LOOP;
		we = aWALK_END;
		bc = 1.0f;
	}

	LastStep = 0;
	CurF = w->CurFrame;
	dist = Ch->Mesh->DefaultAnim.Dist;
	if ((CurF < ActionStart[ws]) || (CurF >= ActionStart[we])) CurF = ActionStart[ws];
	if (CurF <= ActionStart[ws]) startpos = bc * dist[ActionStart[aSTAND]];
	else startpos = bc * dist[CurF - 1];
//	decide quanti frame fare e quali frame fare
	while (curlen <= len) {
		if (CurF == ActionStart[we]) {
			startpos = (startpos - bc * dist[CurF]);
			CurF = ActionStart[wl];
			startpos = bc * dist[CurF] + startpos;
		}

		w->WalkSteps[LastStep].Act = wl;
		w->WalkSteps[LastStep].Frame = CurF;
		w->WalkSteps[LastStep++].Pos.y = curlen;
		curlen += startpos - bc * dist[CurF];
		startpos = bc * dist[CurF++];
		if (LastStep > T3D_MAX_WALKSTEPS - 5) break;
	}

	if (CurF == ActionStart[we]) CurF = ActionStart[wl];
	w->WalkSteps[LastStep].Act = wl;
	w->WalkSteps[LastStep].Frame = CurF;

	len = 0.0;
	curlen = 0.0;
	startpos = 0.0;
	w->NumSteps = 0;
	w->CurrentStep = 0;

	w->WalkSteps[w->NumSteps].Angle = lastangle;
	w->WalkSteps[w->NumSteps].Pos.x = w->PathNode[0].pos.x;
	w->WalkSteps[w->NumSteps].Pos.y = CurFloorY;
	w->WalkSteps[w->NumSteps].Pos.z = w->PathNode[0].pos.z;
	w->WalkSteps[w->NumSteps++].curp = w->OldPanel;

	for (i = 0; i < w->NumPathNodes - 1; i++) {
		st.x = w->PathNode[i].pos.x;
		st.y = 0.0;
		st.z = w->PathNode[i].pos.z;
		en.x = w->PathNode[i + 1].pos.x;
		en.y = 0.0;
		en.z = w->PathNode[i + 1].pos.z;

		len += t3dVectDistance(&st, &en);               // dist of two points
		t3dVectSub(&direction, &en, &st);
		t3dVectNormalize(&direction);               // normalize direction

		angle = SinCosAngle(bc * direction.x, bc * direction.z); //calc angles between new direction and first frame direction
		lastangle = angle;
		while ((w->WalkSteps[w->NumSteps].Pos.y <= len) && (w->NumSteps < LastStep)) {
			curlen = w->WalkSteps[w->NumSteps].Pos.y;

			t3dVectCopy(&w->WalkSteps[w->NumSteps].Pos, &st);
			w->WalkSteps[w->NumSteps].Pos.x += (curlen - startpos) * direction.x;
			w->WalkSteps[w->NumSteps].Pos.y = CurFloorY;
			w->WalkSteps[w->NumSteps].Pos.z += (curlen - startpos) * direction.z;

			w->WalkSteps[w->NumSteps].Angle = angle;
			w->WalkSteps[w->NumSteps].curp = w->PathNode[i].curp;

			w->NumSteps ++;
		}
		startpos = len;
	}

	if ((dp) && (GetLightDirection(&st, dp)) && (st.x != 0.0f) && (st.z != 0.0f)) {
		lastangle = SinCosAngle((st.x - w->Cur.x), (st.z - w->Cur.z));
//		DebugLogFile("LastPos %d | AN %d | %f %f", dp, lastangle, st.x, st.z );
	}

	w->WalkSteps[w->NumSteps].Angle = lastangle;
	w->WalkSteps[w->NumSteps].Pos.x = w->Cur.x;
	w->WalkSteps[w->NumSteps].Pos.y = CurFloorY;
	w->WalkSteps[w->NumSteps].Pos.z = w->Cur.z;
	w->WalkSteps[w->NumSteps++].curp = w->CurPanel;

	// arrotonda la fine
	if ((w->NumSteps > 2)) {
		// FIXME: The following code should be checked for correct intended logic as
		//        it's previous form had sequence point issues.
		angle = lastangle;
		lastangle = w->WalkSteps[w->NumSteps - 2].Angle;
		if (angle != lastangle) {
			approx = angle - lastangle;
			if (approx > T3D_PI) approx = -T3D_PI * 2 + approx;
			else if (approx < -T3D_PI) approx = T3D_PI * 2 + approx;
			approx /= 4.0f;

			if (--LastStep > 0) w->WalkSteps[LastStep].Angle = angle - approx * 1.0f;
			if (--LastStep > 0) w->WalkSteps[LastStep].Angle = angle - approx * 2.0f;
			if (--LastStep > 0) w->WalkSteps[LastStep].Angle = angle - approx * 3.0f;
		}
	}
	if (LastStep < 0) LastStep = 0;
	lastangle = w->WalkSteps[0].Angle;
	// fa le curve
	NumCurve = 14;
	for (i = 1; i < LastStep; i++) {
		angle = w->WalkSteps[i].Angle;
		// se ha fatto una curva
		if (angle != lastangle) {
			// calcola variazione di angoli
			approx = angle - lastangle;
			if (approx > T3D_PI) approx = -T3D_PI * 2 + approx;
			else if (approx < -T3D_PI) approx = T3D_PI * 2 + approx;

			// calcola in quanti frames puo' avvenire la variazione
			for (nla = 1; nla < (NumCurve / 2); nla++)
				if ((i - nla) > 0)
					if (w->WalkSteps[i - nla].Angle != lastangle)
						break;

			for (nca = 1; nca < (NumCurve / 2); nca++)
				if ((i + nca) < LastStep)
					if (w->WalkSteps[i + nca].Angle != angle)
						break;
			na = (nla - 1) + (nca - 1) + 1;

			// calcola il delta dell'andolo
			approx /= (float)(na);

			// aggiorna frames
			for (j = -(nla - 1), na = 0; j < nca; j++, na++)
				if (((i + j) > 0) && ((i + j) < LastStep))
					w->WalkSteps[i + j].Angle = lastangle + approx * (float)(na);
			i += (nca - 1);

			//DebugFile("%d: %6f %6f %6f",i,w->WalkSteps[i-1].Angle,w->WalkSteps[i].Angle,w->WalkSteps[i+1].Angle);
		}

		lastangle = angle;
	}
}

/* -----------------23/03/99 12.03-------------------
 *                  CheckPathNodes
 * --------------------------------------------------*/
int32 CheckPathNodes(int32 oc) {
	t3dCHARACTER *Ch = Character[oc];
	t3dWALK *w = &Ch->Walk;
	int32 i;
	uint16 b;

	if (!Ch || !w) return FALSE;

//	se interseca almeno un baffo si ferma!!
	for (i = 1; i < w->NumPathNodes; i++) {
		for (b = 0; b < w->PanelNum; b++) {
			PointResult res = IntersLineLine(w->Panel[b].backA, w->Panel[b].backB,
			                   w->PathNode[i - 1].pos, w->PathNode[i].pos);
			if (res.isValid) {
				w->NumPathNodes = i - 1;
				w->CurPanel = w->PathNode[i - 1].curp;
				w->NumSteps = 0;
				w->CurrentStep = 0;
				w->NumPathNodes = 0;
				w->Check = 0;
				return FALSE;
			}
		}
	}
	return TRUE;
}

/* -----------------17/03/98 16.30-------------------
 *              CheckCharacterWithBounds
 * --------------------------------------------------*/
bool CheckCharacterWithBounds(WGame &game, int32 oc, t3dV3F *Pos, uint8 dp, uint8 back) {
	t3dCHARACTER *Char = Character[oc];
	int32 st;
	t3dV3F tmp;
	t3dPAN *p;

	if (!Char) return FALSE;
	StopObjAnim(game, oc);

	Char->Walk.Cur.x = Pos->x;
	Char->Walk.Cur.z = Pos->z;

	// Reset some vars
	if (!(Char->Mesh->Flags & T3D_MESH_DEFAULTANIM))
		FixPos(oc);

	Char->Walk.Panel = t3dCurRoom->Panel[t3dCurRoom->CurLevel];
	Char->Walk.PanelNum = t3dCurRoom->NumPanels[t3dCurRoom->CurLevel];
	if (t3dCurRoom) {
		CurFloorY = t3dCurRoom->PanelHeight[t3dCurRoom->CurLevel];
	}

	for (int i = 0; i < T3D_MAX_CHARACTERS; i++) {
		// If the character is not hidden and has panels, adds them
		if (Character[i] && (Character[i] != Char) && !(Character[i]->Flags & (T3D_CHARACTER_HIDE | T3D_CHARACTER_BNDHIDE)) && (p = Character[i]->Body->Panel[0])) {
			st = Char->Walk.PanelNum;
			for (int j = 0; j < Character[i]->Body->NumPanels[0]; j++, p++, Char->Walk.PanelNum++) {
				tmp.x = p->a.x;
				tmp.y = CurFloorY;
				tmp.z = p->a.z;
				t3dVectTransform(&tmp, &tmp, &Character[i]->Mesh->Matrix);
				t3dVectAdd(&tmp, &tmp, &Character[i]->Mesh->Trasl);
				Char->Walk.Panel[Char->Walk.PanelNum].a.x = tmp.x;
				Char->Walk.Panel[Char->Walk.PanelNum].a.z = tmp.z;

				tmp.x = p->b.x;
				tmp.y = CurFloorY;
				tmp.z = p->b.z;
				t3dVectTransform(&tmp, &tmp, &Character[i]->Mesh->Matrix);
				t3dVectAdd(&tmp, &tmp, &Character[i]->Mesh->Trasl);
				Char->Walk.Panel[Char->Walk.PanelNum].b.x = tmp.x;
				Char->Walk.Panel[Char->Walk.PanelNum].b.z = tmp.z;

				tmp.x = p->backA.x;
				tmp.y = CurFloorY;
				tmp.z = p->backA.z;
				t3dVectTransform(&tmp, &tmp, &Character[i]->Mesh->Matrix);
				t3dVectAdd(&tmp, &tmp, &Character[i]->Mesh->Trasl);
				Char->Walk.Panel[Char->Walk.PanelNum].backA.x = tmp.x;
				Char->Walk.Panel[Char->Walk.PanelNum].backA.z = tmp.z;

				tmp.x = p->backB.x;
				tmp.y = CurFloorY;
				tmp.z = p->backB.z;
				t3dVectTransform(&tmp, &tmp, &Character[i]->Mesh->Matrix);
				t3dVectAdd(&tmp, &tmp, &Character[i]->Mesh->Trasl);
				Char->Walk.Panel[Char->Walk.PanelNum].backB.x = tmp.x;
				Char->Walk.Panel[Char->Walk.PanelNum].backB.z = tmp.z;

				Char->Walk.Panel[Char->Walk.PanelNum].near1 = p->near1 + st;
				Char->Walk.Panel[Char->Walk.PanelNum].near2 = p->near2 + st;
			}
		}
	}

	Char->Mesh->Flags |= T3D_MESH_DEFAULTANIM;
	t3dVectCopy(&Char->Pos, &Char->Mesh->Trasl);
	FindPath(oc, t3dCurCamera);    // Calc path
	if (CheckPathNodes(oc)) BuildStepList(oc, dp, back);
	_vm->_messageSystem.deleteWaitingMsgs(MP_WAIT_PORTAL);
	_vm->_messageSystem.deleteWaitingMsgs(MP_WAIT_ANIM);
	_vm->_messageSystem.deleteWaitingMsgs(MP_WAIT_ACT);
	if (Char->Walk.NumSteps >= 2) return TRUE;
	else return FALSE;
}

/* -----------------29/09/98 15.09-------------------
 *                  CharGotoPosition
 * --------------------------------------------------*/
bool CharGotoPosition(WGame &game, int32 oc, uint8 pos, uint8 back, int32 anim) {
	t3dV3F tmp;
	uint8 cp;

	if ((oc == ocCURPLAYER) || (oc == ocDARRELL && !CurPlayer) || (oc == ocVICTORIA && CurPlayer)) {
		if ((PlayerPos[CurPlayer + ocDARRELL] == pos) || (PlayerGotoPos[CurPlayer + ocDARRELL] == pos)) return FALSE;
		if ((cp = PlayerGotoPos[CurPlayer + ocDARRELL] = GetLightPosition(&tmp, pos)) == 0) return FALSE;
		game._messageSystem.removeEvent(EventClass::MC_PLAYER, ME_ALL);

		if (bFirstPerson)
			_vm->_messageSystem.doEvent(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
		_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERGOTO, MP_DEFAULT, 0, 0, bFirstPerson, nullptr, nullptr, nullptr);
	} else if ((cp = PlayerGotoPos[oc] = GetLightPosition(&tmp, pos)) == 0) return FALSE;

	FloorHit = 1;
	if (!CheckCharacterWithBounds(game, oc, &tmp, cp, back)) return FALSE;
	if (anim)
		Character[oc]->Walk.WalkSteps[Character[oc]->Walk.NumSteps].Act = (int16)anim;
	return TRUE;
}

/* -----------------30/04/98 10.43-------------------
 *                  FixupCurAction
 * --------------------------------------------------*/
void FixupCurAction(int32 oc) {
	t3dCHARACTER *Char = Character[oc];
	t3dWALK *w = &Char->Walk;

	w->CurAction = 1;
	while (ActionStart[w->CurAction]) {
		if (ActionStart[w->CurAction] > w->CurFrame)
			break;
		else
			w->CurAction ++;
	}
	w->CurAction --;
}

/* -----------------27/04/98 18.29-------------------
 *                  CharNextFrame
 * --------------------------------------------------*/
uint8 CharNextFrame(WGame &game, int32 oc) {
	t3dCHARACTER *Char = Character[oc];
	int32 an, na, nf;

	if (Char == nullptr)
		return false;

	if (Char->Walk.CurrentStep < Char->Walk.NumSteps) {
		t3dVectCopy(&Char->Mesh->Trasl, &Char->Walk.WalkSteps[Char->Walk.CurrentStep].Pos);
		t3dVectCopy(&Char->Pos, &Char->Walk.WalkSteps[Char->Walk.CurrentStep].Pos);

		na = Char->Walk.WalkSteps[Char->Walk.CurrentStep].Act;
		nf = Char->Walk.WalkSteps[Char->Walk.CurrentStep].Frame;
		Char->Walk.CurFrame = nf;
		Char->Mesh->CurFrame = Char->Walk.CurFrame;
		Char->Walk.CurAction = na;
		Char->Walk.OldPanel = Char->Walk.CurPanel;
		Char->Walk.CurPanel = Char->Walk.WalkSteps[Char->Walk.CurrentStep].curp;

		t3dMatRot(&Char->Mesh->Matrix, 0.0f, Char->Walk.WalkSteps[Char->Walk.CurrentStep].Angle + T3D_PI, 0.0f);
		Char->Mesh->Matrix.Flags &= ~T3D_MATRIX_IDENTITY;

		// rotate direction vector of the actor
		Char->Dir.x = 0.0f; //first frame direction
		Char->Dir.y = 0.0f;
		Char->Dir.z = -1.0f;
		t3dVectTransform(&Char->Dir, &Char->Dir, &Char->Mesh->Matrix);   //rotate by Character angle

		Char->Walk.CurrentStep ++;
		if (FastWalk && ((Char->Walk.NumSteps - Char->Walk.CurrentStep) > 5))
			Char->Walk.CurrentStep += 2;
		FixupCurAction(oc);
		if (oc == ocCURPLAYER)
			PlayerPos[CurPlayer + ocDARRELL] = 0;
		else
			PlayerPos[oc] = 0;

		return true;
	} else {
		if (Player == Char) {
			an = TheTime + PLAYER_IDLE_TIME;
			_vm->_messageSystem.doEvent(EventClass::MC_PLAYER, ME_PLAYERIDLE, MP_WAIT_RETRACE, (int16)(CurPlayer + ocDARRELL), 0, 0, &an, nullptr, nullptr);
			PlayerPos[CurPlayer + ocDARRELL] = PlayerGotoPos[CurPlayer + ocDARRELL];
			PlayerGotoPos[CurPlayer + ocDARRELL] = 0;
		}
		an = Char->Walk.WalkSteps[Char->Walk.NumSteps].Act;
		Char->Walk.WalkSteps[0].reset();
		Char->Walk.NumSteps = 0;
		if (an) StartAnim(game, an);
		return false;
	}
}

/* -----------------28/04/98 17.51-------------------
 *                  UpdateLook
 * --------------------------------------------------*/
void UpdateLook(int32 oc) {
	/*  t3dCHARACTER *Char=Character[oc];
	    t3dWALK *w=&Char->Walk;
	    t3dF32 a1,a2;

	    a1 = SinCosAngle( w->Panel[w->CurPanel].x1 - w->CurX, w->Panel[w->CurPanel].z1 - w->CurZ );
	    a2 = SinCosAngle( w->LookX - w->CurX, w->LookZ - w->CurZ );

	    if ( fabs( a1-a2 ) < T3D_PI )
	    {
	        w->LookX = w->Panel[w->CurPanel].x1;
	        w->LookZ = w->Panel[w->CurPanel].z1;
	    }
	    else
	    {
	        w->LookX = w->Panel[w->CurPanel].x2;
	        w->LookZ = w->Panel[w->CurPanel].z2;
	    }
	*/
}

} // End of namespace Watchmaker
