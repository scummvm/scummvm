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

#include "watchmaker/walk/ball.h"
#include "watchmaker/globvar.h"
#include "watchmaker/define.h"
#include "watchmaker/ll/ll_mesh.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/t3d_mesh.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/classes/do_camera.h"
#include "watchmaker/renderer.h"
#include "watchmaker/classes/do_keyboard.h"

namespace Watchmaker {

struct SPhys Ball[MAX_GOPHERS];
t3dV3F OldBallCameraSource, OldBallCameraTarget;
int32 ExplosionTimer;
t3dF32 IperLight, LastFloorY;

/* -----------------16/02/99 11.02-------------------
 *                  InitPhys
 * --------------------------------------------------*/
void InitPhys(struct SPhys *p) {
	p->Kg = 9.80665f;                                       // costante gravitazionale
	p->Kn = 6.0f * T3D_PI * 0.00023f;                       // coefficiente di attrito viscoso
	p->Kc = 0.70f;                                          // percentuale di energia cinetica che si conserva
	p->Kt = 0.50f;                                          // percentuale di energia cinetica che si conserva per terra
	p->Mp = 0.40f;                                          // massa palla
	p->Mb = 2.00f;                                          // massa braccio
	p->Rp = 0.13f;                                          // raggio palla
	p->Td = 0.0f;                                           // differenza di tempo trascorsa tra un istante e l'altro
	p->Ts = 0.0f;                                           // tempo trascorso dall'ultimo impatto
	p->Tb = 0.0f;                                           // tempo di carica del braccio
	p->InAir = 0;                                           // Se sta volando

	t3dVectInit(&p->Fv, 0.0f, 0.0f, 0.0f);               // forza del vento
	t3dVectFill(&p->V0, 0.0f);                           // velocita' iniziale palla
	t3dVectFill(&p->Vt, 0.0f);                           // velocita' attuale palla
	t3dUpdateArrow(Freccia50->Mesh, 0.0f);
}

/* -----------------16/02/99 11.05-------------------
 *                  ContinuePhys
 * --------------------------------------------------*/
void ContinuePhys(struct SPhys *p) {
	p->Ts += p->Td;
	if (!p->InAir) {                                                                         // se non vola, esce
		t3dVectCopy(&p->S0, &p->St);
		t3dVectCopy(&p->V0, &p->Vt);
		return ;
	}
	t3dVectFill(&p->At, 0.0f);
	if (p->MVt > 0.1f) {                                                                         // Attrito aria se vola
		p->At.x -= (p->Kn * p->Rp * p->V0.x / p->Mp);
		p->At.y -= (p->Kn * p->Rp * p->V0.y / p->Mp);
		p->At.z -= (p->Kn * p->Rp * p->V0.z / p->Mp);
	}

	p->At.y -= p->Kg;

	p->At.x += p->Fv.x / p->Mp;                                                                 // Vento
	p->At.y += p->Fv.y / p->Mp;
	p->At.z += p->Fv.z / p->Mp;

	p->MAt = (t3dF32)sqrt(p->At.x * p->At.x + p->At.y * p->At.y + p->At.z * p->At.z);

	p->Vt.x = p->V0.x + p->At.x * p->Td;
	p->Vt.y = p->V0.y + p->At.y * p->Td;
	p->Vt.z = p->V0.z + p->At.z * p->Td;
	p->MVt = (t3dF32)sqrt(p->Vt.x * p->Vt.x + p->Vt.y * p->Vt.y + p->Vt.z * p->Vt.z);

	p->St.x = p->S0.x + p->V0.x * p->Td + p->At.x * p->Td * p->Td / 2.0f;
	p->St.y = p->S0.y + p->V0.y * p->Td + p->At.y * p->Td * p->Td / 2.0f;
	p->St.z = p->S0.z + p->V0.z * p->Td + p->At.z * p->Td * p->Td / 2.0f;

//	if( p->St.y < CurFloorY ) p->St.y = CurFloorY;
//	DebugFile("%f;%f;%f;",p->St.x,p->St.y,p->St.z);

	t3dVectCopy(&p->S0, &p->St);
	t3dVectCopy(&p->V0, &p->Vt);
}


/* -----------------03/05/99 15.34-------------------
 *                  NextGopher
 * --------------------------------------------------*/
void NextGopher(WGame &game) {
	if (CurGopher >= 0) {
		StopObjAnim(game, ocCURPLAYER);
		CharStop(ocCURPLAYER);
	}
	CurGopher = ((CurGopher + 1) >= MAX_GOPHERS) ? 0 : CurGopher + 1;

	Character[ocCURPLAYER] = Character[CurGopher + 1];
	Player = Character[CurGopher + 1];
	Player->Flags &= ~T3D_CHARACTER_HIDE;
	Player->Mesh->Flags |= T3D_MESH_DEFAULTANIM;
//	Riposiziona sempre il personaggio sul suolo
	t3dClipToSurface(game.init, &Player->Mesh->Trasl);
	CurFloorY = Player->Mesh->Trasl.y;
	LastFloorY = Player->Mesh->Trasl.y;
	Ball[CurGopher].TimeLeft = 60.0f;
	t3dUpdateArrow(Freccia50->Mesh, 0.0f);
	game._renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);
	t3dVectFill(&OldBallCameraTarget, 0.0f);
	t3dVectFill(&OldBallCameraSource, 0.0f);
	bGolfMode = 0;
}

/* -----------------16/02/99 11.06-------------------
 *                  ChangePhys
 * --------------------------------------------------*/
void ChangePhys(struct SPhys *p, int flags) {
	p->Ts = 0.0f;
	if (flags == 1) {                                                                        // urto per terra
		if (!p->InAir) return;                                                               // se non vola, esce
		t3dVectCopy(&p->S0, &p->St);
		p->V0.x =  p->Vt.x * p->Kt;
		p->V0.y = -p->Vt.y * p->Kt;
		p->V0.z =  p->Vt.z * p->Kt;
		p->MVt = (t3dF32)sqrt(p->V0.x * p->V0.x + p->V0.y * p->V0.y + p->V0.z * p->V0.z);
	} else if (flags == 2) {                                                                     // riposizionamento della palla
		t3dVectFill(&p->V0, 0.0f);
		p->MVt = 0.0f;
		p->InAir = 0;
	} else {                                                                                    // urto col braccio
		t3dVectCopy(&p->S0, &p->St);
		p->Vb.x = p->Fb.x * p->Tb / p->Mb;
		p->Vb.y = p->Fb.y * p->Tb / p->Mb;
		p->Vb.z = p->Fb.z * p->Tb / p->Mb;

		p->V0.x = ((p->Mp - p->Mb) * p->Vt.x / (p->Mp + p->Mb) + 2.0f * p->Mb * p->Vb.x / (p->Mp + p->Mb)) * p->Kc;
		p->V0.y = ((p->Mp - p->Mb) * p->Vt.y / (p->Mp + p->Mb) + 2.0f * p->Mb * p->Vb.y / (p->Mp + p->Mb)) * p->Kc;
		p->V0.z = ((p->Mp - p->Mb) * p->Vt.z / (p->Mp + p->Mb) + 2.0f * p->Mb * p->Vb.z / (p->Mp + p->Mb)) * p->Kc;
		p->MVt = (t3dF32)sqrt(p->V0.x * p->V0.x + p->V0.y * p->V0.y + p->V0.z * p->V0.z);
		p->InAir = true;
	}
}

/* -----------------13/04/99 10.52-------------------
 *                  UpdateBall
 * --------------------------------------------------*/
void UpdateBall(WGame &game, struct SPhys *p) {
	t3dV3F zero, tmp, start, end;
	int32 i;
	t3dMESH *m;
	Init &init = game.init;
	auto &renderer = *game._renderer;
	auto windowInfo = game._renderer->getScreenInfos();

//	Se ha finito il tempo, cambia talpa
	if ((p->TimeLeft < 0.0f) && ((bGolfMode == 0) || (bGolfMode == 1)))
		NextGopher(game);
//	Aggiorna la freccia
	UpdateArrow();
//	Riposiziona sempre il personaggio sul suolo
	t3dClipToSurface(init, &Player->Mesh->Trasl);
	CurFloorY = Player->Mesh->Trasl.y;
	LastFloorY = CurFloorY;
	IperLight = 0.0f;

//	Muove la palla
	if (bGolfMode != 5)
		ContinuePhys(p);
	Palla50->Mesh->Trasl = p->St * (2.55f * 100.0f);

//	Se vola e tocca il terreno ferma tutto
	if ((bGolfMode == 2) || (bGolfMode == 3) || (bGolfMode == 4)) {
		t3dVectCopy(&start, &Palla50->Mesh->Trasl);
		start.y = 260000.0f;
		t3dVectCopy(&end, &Palla50->Mesh->Trasl);
		end.y = -130000.0f;
		for (i = 0; i < 6; i++)
			if ((m = LinkMeshToStr(init, (const char *)init.Obj[oNEXTPORTAL].meshlink[i])) && (t3dVectMeshInters(m, start, end, &tmp)))
				break;
		if (tmp.y < 0.0f) tmp.y = 0.0f;
		if ((Palla50->Mesh->Trasl.y < (tmp.y + 0.01f)) && (p->MVt > 0.1)) {
			ExplosionTimer = 0;
			t3dVectCopy(&Explode50->Mesh->Trasl, &tmp);
			bGolfMode = 5;
		}

		t3dVectCopy(&zero, &p->Vt);
		t3dVectNormalize(&zero);
		if (bGolfMode == 3)
			if (((Palla50->Mesh->Trasl.y < (tmp.y + EYES_HEIGHT * 15.0f)) && (zero.y < 0.0f)) || (zero.y < -0.5f))
				bGolfMode = 4;
	}
//	Aggiorna i markers
	GopherMark[0]->Flags |= T3D_CHARACTER_HIDE;
	GopherMark[1]->Flags |= T3D_CHARACTER_HIDE;
	GopherMark[2]->Flags |= T3D_CHARACTER_HIDE;
	if ((bGolfMode == 0) || (bGolfMode == 1)) {
		for (i = 0; i < MAX_GOPHERS; i++) {
			if (i == CurGopher) continue;

			if (bGolfMode)
				t3dVectSub(&tmp, &Character[i + 1]->Mesh->Trasl, &Palla50->Mesh->Trasl);
			else
				t3dVectSub(&tmp, &Character[i + 1]->Mesh->Trasl, &Player->Mesh->Trasl);
			t3dVectNormalize(&tmp);
			tmp *= EYES_HEIGHT * 4.0f;
			if (bGolfMode)
				t3dVectAdd(&GopherMark[i]->Mesh->Trasl, &Palla50->Mesh->Trasl, &tmp);
			else
				t3dVectAdd(&GopherMark[i]->Mesh->Trasl, &Player->Mesh->Trasl, &tmp);
			GopherMark[i]->Mesh->Trasl.y += EYES_HEIGHT * 2.0f;
			GopherMark[i]->Flags &= ~T3D_CHARACTER_HIDE;
		}
	}
//	Aggiorna il cono visivo
	p->ViewCone.x1 = 130 + (int32)(Player->Mesh->Trasl.x / 255.0f * 0.341f) + windowInfo.width - renderer.getBitmapRealDimX(GopherMap);
	p->ViewCone.y1 = 146 - (int32)(Player->Mesh->Trasl.z / 255.0f * 0.341f);
	tmp = Player->Dir * 20.0f;
	zero = Player->Dir * 10.0f;
	p->ViewCone.x2 = p->ViewCone.x1 + (int32)(tmp.x + zero.z);
	p->ViewCone.y2 = p->ViewCone.y1 - (int32)(tmp.z - zero.x);
	p->ViewCone.x3 = p->ViewCone.x1 + (int32)(tmp.x - zero.z);
	p->ViewCone.y3 = p->ViewCone.y1 - (int32)(tmp.z + zero.x);
	DisplayD3DTriangle(*game._renderer, p->ViewCone.x1, p->ViewCone.y1, p->ViewCone.x2, p->ViewCone.y2, p->ViewCone.x3, p->ViewCone.y3, 125, 125, 125, 125);

//	Aggiorna la mappa e le scritte
	game._renderer->_2dStuff.displayDDBitmap(GopherMap, windowInfo.width - renderer.getBitmapRealDimX(GopherMap), 0, 0, 0, 0, 0);
	for (i = 0; i < MAX_GOPHERS; i++)
		game._renderer->_2dStuff.displayDDBitmap(GopherPos[i], windowInfo.width - renderer.getBitmapRealDimX(GopherMap) +
		                130 + (int32)(Character[i + 1]->Mesh->Trasl.x / 255.0f * 0.341f) - renderer.getBitmapRealDimX(GopherPos[i]) / 2,
		                146 - (int32)(Character[i + 1]->Mesh->Trasl.z / 255.0f * 0.341f) - renderer.getBitmapRealDimY(GopherPos[i]) / 2, 0, 0, 0, 0);
	if (bGolfMode)
		game._renderer->_2dStuff.displayDDBitmap(GopherBall, windowInfo.width - renderer.getBitmapRealDimX(GopherMap) +
		                130 + (int32)(Palla50->Mesh->Trasl.x / 255.0f * 0.341f) - renderer.getBitmapRealDimX(GopherBall) / 2,
		                146 - (int32)(Palla50->Mesh->Trasl.z / 255.0f * 0.341f) - renderer.getBitmapRealDimY(GopherBall) / 2, 0, 0, 0, 0);
	if ((bGolfMode == 0) || (bGolfMode == 1)) {
		DebugVideo(*game._renderer, 10, 32, "TimeLeft: %d", (int32)p->TimeLeft);
		p->TimeLeft -= p->Td;
		if ((Player->Mesh->Flags & T3D_MESH_DEFAULTANIM) && (Player->Mesh->CurFrame > 5) && (Player->Mesh->CurFrame < 271))
			p->TimeLeft -= 2.0f * p->Td;
	}
	if (bGolfMode == 1) {
		DebugVideo(*game._renderer, 10, 48, "Angle: %d %d", 45 - (int)(Ball[CurGopher].Angle.x * 180.0f / T3D_PI), (int)(Ball[CurGopher].Angle.y * 180.0f / T3D_PI));
		DebugVideo(*game._renderer, 10, 64, "Power: %d", (int)Ball[CurGopher].Angle.z);
	}
	game._renderer->_2dStuff.displayDDBitmap(EndPic, windowInfo.width - renderer.getBitmapRealDimX(EndPic) - 20, windowInfo.height - renderer.getBitmapRealDimY(EndPic) - 20, 0, 0, 0, 0);

//	Aggiorna la camera
	ProcessGopherCamera(game);
//	Controlla che la camera non finisca sotto terra
	t3dVectCopy(&tmp, &t3dCurCamera->Source);
	if (t3dClipToSurface(init, &tmp))
		if (t3dCurCamera->Source.y < (tmp.y + CHEST_HEIGHT))
			t3dCurCamera->Source.y = (tmp.y + CHEST_HEIGHT);

	t3dVectCopy(&OldBallCameraSource, &t3dCurCamera->Source);
	t3dVectCopy(&OldBallCameraTarget, &t3dCurCamera->Target);
//	Illumina quello che sta vicino alla camera
	t3dLightChar(Player->Mesh, &t3dCurCamera->Source);
	t3dLightChar(Freccia50->Mesh, &t3dCurCamera->Source);
	t3dLightRoom(init, t3dCurRoom, &t3dCurCamera->Target, 5000.0f, 15000.0f, IperLight);
}

/* -----------------04/05/99 10.07-------------------
 *                  ProcessGopherKeyboard
 * --------------------------------------------------*/
void ProcessGopherKeyboard() {
	warning("Stubbed: ProcessGopherKeyboard");
#if 0
	t3dF32 AngleX, AngleY, AngleSpeed;
	t3dF32 TurnSpeed, Speed = 1.0f;
	t3dM3X3F mx;

	AngleX = AngleY = AngleSpeed = 0.0f;
	TurnSpeed = 3.5f * FrameFactor;

	ProcessKBInput();

	if (KeyDown(DIK_ESCAPE))             // Quitta il gioco
		CloseSys();
	if (KeyUp(DIK_W)) {                  // Modalita' wireframe
		bForceWire ^= 1;
		if (bForceWire)
			rSetRenderMode(rWIREFRAMEMODE);
		else
			rSetRenderMode(rSOLIDMODE);
	}
//	if( KeyUp(DIK_I) )                   // Escono Informazioni
//		bShowInfo ^= 1;
//	if( KeyUp(DIK_B) )                   // Escono BoundingBox
//		bShowBoundingBox ^= 1;
//	if( KeyUp(DIK_P) )                   // Escono Pannelli
//		bShowPanels ^= 1;
	if (KeyUp(DIK_RETURN))               // Avanza di un frame tutte le animazioni
		NextGopher();

	if (KeyUp(DIK_1)) CharSetPosition(ocCURPLAYER, 1, NULL);
	if (KeyUp(DIK_2)) CharSetPosition(ocCURPLAYER, 2, NULL);
	if (KeyUp(DIK_3)) CharSetPosition(ocCURPLAYER, 3, NULL);
	if (KeyUp(DIK_4)) CharSetPosition(ocCURPLAYER, 4, NULL);
	if (KeyUp(DIK_5)) CharSetPosition(ocCURPLAYER, 5, NULL);
	if (KeyUp(DIK_6)) CharSetPosition(ocCURPLAYER, 6, NULL);
	if (KeyUp(DIK_7)) CharSetPosition(ocCURPLAYER, 7, NULL);
	if (KeyUp(DIK_8)) CharSetPosition(ocCURPLAYER, 8, NULL);
	if (KeyUp(DIK_9)) CharSetPosition(ocCURPLAYER, 9, NULL);
	if (KeyUp(DIK_NUMPAD0)) CharSetPosition(ocCURPLAYER, 10, NULL);
	if (KeyUp(DIK_NUMPAD1)) CharSetPosition(ocCURPLAYER, 11, NULL);
	if (KeyUp(DIK_NUMPAD2)) CharSetPosition(ocCURPLAYER, 12, NULL);
	if (KeyUp(DIK_NUMPAD3)) CharSetPosition(ocCURPLAYER, 13, NULL);
	if (KeyUp(DIK_NUMPAD4)) CharSetPosition(ocCURPLAYER, 14, NULL);
	if (KeyUp(DIK_NUMPAD5)) CharSetPosition(ocCURPLAYER, 15, NULL);
	if (KeyUp(DIK_NUMPAD6)) CharSetPosition(ocCURPLAYER, 16, NULL);
	if (KeyUp(DIK_NUMPAD7)) CharSetPosition(ocCURPLAYER, 17, NULL);
	if (KeyUp(DIK_NUMPAD8)) CharSetPosition(ocCURPLAYER, 18, NULL);
	if (KeyUp(DIK_NUMPAD9)) CharSetPosition(ocCURPLAYER, 19, NULL);

	if (KeyUp(DIK_TAB)) {            // Cambia modalita'

		if (!bGolfMode) {
			t3dMatCopy(&Freccia50->Mesh->Matrix, &Player->Mesh->Matrix);
			t3dVectCopy(&Palla50->Mesh->Trasl, &Player->Mesh->Trasl);
			Palla50->Mesh->Flags |= T3D_MESH_HIDDEN;
			Palla50->Flags &= ~T3D_CHARACTER_HIDE;
			StartAnim(aGOPHER_ACTION);
			bGolfMode = 1;

			t3dVectScale(&Ball[CurGopher].S0, &Palla50->Mesh->Trasl,  1.0f / (2.55f * 100.0f));
			t3dVectCopy(&Ball[CurGopher].St, &Ball[CurGopher].S0);

			t3dVectInit(&Ball[CurGopher].Fb, 0.0f, 40.0f, -40.0f);
			if (Ball[CurGopher].Angle.z) {
				t3dVectNormalize(&Ball[CurGopher].Fb);
				t3dVectScale(&Ball[CurGopher].Fb, &Ball[CurGopher].Fb, Ball[CurGopher].Angle.z);
				t3dMatRot(&mx, -Ball[CurGopher].Angle.x, Ball[CurGopher].Angle.y, 0.0f);
				t3dVectTransform(&Ball[CurGopher].Fb, &Ball[CurGopher].Fb, &mx);
			}
			t3dVectTransform(&Ball[CurGopher].Fb, &Ball[CurGopher].Fb, &Freccia50->Mesh->Matrix);
			InitPhys(&Ball[CurGopher]);
		} else {
			Freccia50->Flags |= T3D_CHARACTER_HIDE;
			ContinueAnim(aGOPHER_ACTION);
		}
	}

	if (KeyDown(DIK_LEFT) && !bGolfMode) {       // Ruota a Destra
		AngleY = (-1) * TurnSpeed / 180.0f * T3D_PI;
		if ((Player->Walk.CurAction <= aSTAND) || (Player->Walk.CurAction == aROT_DX)) {
			PlayerGotoPos[CurPlayer] = 0;
			PlayerPos[CurPlayer] = 0;
			Player->Walk.CurAction = aROT_SX;
			Player->Walk.CurFrame = ActionStart[Player->Walk.CurAction];
			Player->Mesh->CurFrame = Player->Walk.CurFrame;
		} else if (Player->Walk.CurAction == aROT_SX) {
			PlayerGotoPos[CurPlayer] = 0;
			PlayerPos[CurPlayer] = 0;
			Player->Walk.CurFrame ++;
			if (Player->Walk.CurFrame >= ActionStart[Player->Walk.CurAction + 1] - 1)
				Player->Walk.CurFrame = ActionStart[Player->Walk.CurAction];
			Player->Mesh->CurFrame = Player->Walk.CurFrame;
		}
	} else if (KeyDown(DIK_RIGHT) && !bGolfMode) { // Ruota a Sinistra
		AngleY = TurnSpeed / 180.0f * T3D_PI;
		if ((Player->Walk.CurAction <= aSTAND) || (Player->Walk.CurAction == aROT_SX)) {
			PlayerGotoPos[CurPlayer] = 0;
			PlayerPos[CurPlayer] = 0;
			Player->Walk.CurAction = aROT_DX;
			Player->Walk.CurFrame = ActionStart[Player->Walk.CurAction];
			Player->Mesh->CurFrame = Player->Walk.CurFrame;
		} else if (Player->Walk.CurAction == aROT_DX) {
			PlayerGotoPos[CurPlayer] = 0;
			PlayerPos[CurPlayer] = 0;
			Player->Walk.CurFrame ++;
			if (Player->Walk.CurFrame >= ActionStart[Player->Walk.CurAction + 1] - 1)
				Player->Walk.CurFrame = ActionStart[Player->Walk.CurAction];
			Player->Mesh->CurFrame = Player->Walk.CurFrame;
		}
	}

	if (KeyDown(DIK_UP) && !bGolfMode)
		AngleSpeed = 20.0f;
	else if (KeyDown(DIK_DOWN) && !bGolfMode)
		AngleSpeed = -20.0f;

	if ((Player) && !bGolfMode)
		UpdateChar(ocCURPLAYER, AngleSpeed * Speed, AngleY);
#endif
}

/* -----------------04/05/99 10.29-------------------
 *                  UpdateArrow
 * --------------------------------------------------*/
void UpdateArrow() {
	warning("Stubbed: UpdateArrow");
#if 0
	t3dF32 AngleX, AngleY, AngleSpeed;
	t3dM3X3F mx, mv;
	t3dV3F zero;

	if (bGolfMode != 1) {
		KeyClear(DIK_SPACE);
		return;
	}

	AngleX = AngleY = 0.0f;
	AngleSpeed = t3dVectMod(&Ball[CurGopher].Fb);
	t3dVectNormalize(&Ball[CurGopher].Fb);

	if ((Ball[CurGopher].Angle.x > -0.35f) && KeyDown(DIK_UP))     AngleX -= 0.01f;
	if ((Ball[CurGopher].Angle.x < 0.35f) && KeyDown(DIK_DOWN))    AngleX += 0.01f;
	if ((Ball[CurGopher].Angle.y < 0.25f) && KeyDown(DIK_RIGHT))   AngleY += 0.01f;
	if ((Ball[CurGopher].Angle.y > -0.25f) && KeyDown(DIK_LEFT))   AngleY -= 0.01f;
	if ((AngleSpeed < 90.0f) && KeyDown(DIK_A))            AngleSpeed += 1.0f;
	if ((AngleSpeed > 41.0f) &&  KeyDown(DIK_Z))           AngleSpeed -= 1.0f;
	Ball[CurGopher].Angle.x += AngleX;
	Ball[CurGopher].Angle.y += AngleY;
	Ball[CurGopher].Angle.z = AngleSpeed;

	t3dVectFill(&zero, 0.0f);
	t3dMatView(&mv, &zero, &Ball[CurGopher].Fb, 0.0f);
	t3dMatRot(&mx, AngleX, AngleY, 0.0f);
	t3dVectTransform(&Ball[CurGopher].Fb, &Ball[CurGopher].Fb, &mv);
	t3dVectTransform(&Ball[CurGopher].Fb, &Ball[CurGopher].Fb, &mx);
	t3dVectTransformInv(&Ball[CurGopher].Fb, &Ball[CurGopher].Fb, &mv);
	t3dVectScale(&Ball[CurGopher].Fb, &Ball[CurGopher].Fb, AngleSpeed);

	t3dMatViewAlt(&Freccia50->Mesh->Matrix, &Ball[CurGopher].Fb);

	if (KeyUp(DIK_SPACE)) {
		Freccia50->Flags |= T3D_CHARACTER_HIDE;
		Player->Mesh->CurFrame = 271;
		Palla50->Mesh->CurFrame = 271;
		ContinueAnim(aGOPHER_ACTION);
	}
#endif
}

/* -----------------04/05/99 10.37-------------------
 *                  ProcessGopherCamera
 * --------------------------------------------------*/
void ProcessGopherCamera(WGame &game) {
	t3dV3F zero, tmp;
	t3dF32 dist, dist2;
	struct SPhys *p = &Ball[CurGopher];

//	Gestione della telecamera
	if (!bGolfMode) { // inquadra omino
		t3dVectInit(&zero, -Player->Dir.x, 0.0f, -Player->Dir.z);
		t3dVectNormalize(&zero);
		zero *= (EYES_HEIGHT * 2.0f);
		zero.y = CHEST_HEIGHT;

		game._cameraMan->GetCameraTarget(game.init, &t3dCurCamera->Target);
		t3dCurCamera->Target.y = LastFloorY + CHEST_HEIGHT;
		t3dVectAdd(&t3dCurCamera->Source,  &t3dCurCamera->Target, &zero);
		Palla50->Flags |= T3D_CHARACTER_HIDE;
		Freccia50->Flags |= T3D_CHARACTER_HIDE;
		Explode50->Flags |= T3D_CHARACTER_HIDE;
		InitPhys(p);
	} else if (bGolfMode == 1) { // inquadra palla
		t3dVectInit(&zero, -p->Fb.x, 0.0f, -p->Fb.z);
		t3dVectNormalize(&zero);
		zero *= (EYES_HEIGHT * 2.0f);
		zero.y = CHEST_HEIGHT;

		t3dVectCopy(&t3dCurCamera->Target, &Palla50->Mesh->Trasl);
		t3dCurCamera->Target.y += CHEST_HEIGHT;
		t3dVectAdd(&t3dCurCamera->Source,  &t3dCurCamera->Target, &zero);
		Explode50->Flags |= T3D_CHARACTER_HIDE;
		t3dVectCopy(&Freccia50->Mesh->Trasl, &Palla50->Mesh->Trasl);
		t3dUpdateArrow(Freccia50->Mesh, Ball[CurGopher].Angle.z * 15.0f);
	} else if (bGolfMode == 2) { // inquadra pallina che vola
		t3dVectCopy(&t3dCurCamera->Target, &Palla50->Mesh->Trasl);
		t3dVectFill(&OldBallCameraTarget, 0.0f);
		t3dVectFill(&OldBallCameraSource, 0.0f);
		Freccia50->Flags |= T3D_CHARACTER_HIDE;

		if (p->Ts > 0.5f) {
			bGolfMode = 3;
			game._renderer->setCurCameraViewport(45.0f, bSuperView);
		}
	} else if (bGolfMode == 3) { // inquadra terra
		t3dVectCopy(&tmp, &p->Vt);
		t3dVectNormalize(&tmp);

		t3dVectInit(&zero, p->Vt.x, -15.0f, p->Vt.z);
		t3dVectNormalize(&zero);

		if (tmp.y < zero.y)
			t3dVectInit(&zero, p->Vt.x, p->Vt.y, p->Vt.z);
		else
			t3dVectInit(&zero, p->Vt.x, -15.0f, p->Vt.z);

		zero *= 500.0f;
		t3dVectCopy(&t3dCurCamera->Source, &Palla50->Mesh->Trasl);
		t3dVectAdd(&t3dCurCamera->Target,  &t3dCurCamera->Source, &zero);
		if (t3dCurCamera->Target.y < CurFloorY) {
			dist = t3dVectMod(&zero) * ((CurFloorY - t3dCurCamera->Source.y) / zero.y);
			t3dVectNormalize(&zero);
			zero *= dist;
			t3dVectAdd(&t3dCurCamera->Target,  &t3dCurCamera->Source, &zero);
		}
		t3dVectFill(&OldBallCameraTarget, 0.0f);
		t3dVectFill(&OldBallCameraSource, 0.0f);
		Freccia50->Flags |= T3D_CHARACTER_HIDE;
	} else if (bGolfMode == 4) { // ferma la camera
		t3dVectCopy(&t3dCurCamera->Target, &Palla50->Mesh->Trasl);
		t3dVectFill(&OldBallCameraTarget, 0.0f);
		t3dVectFill(&OldBallCameraSource, 0.0f);
	} else if (bGolfMode == 5) {
		Palla50->Flags |= T3D_CHARACTER_HIDE;
		Freccia50->Flags |= T3D_CHARACTER_HIDE;
		Explode50->Flags &= ~T3D_CHARACTER_HIDE;

		if (ExplosionTimer > 200) {
//			NextGopher();
			game._renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);
			t3dVectFill(&OldBallCameraTarget, 0.0f);
			t3dVectFill(&OldBallCameraSource, 0.0f);
			bGolfMode = 0;
		} else if (ExplosionTimer > 100) {
			t3dUpdateExplosion(Explode50->Mesh, IperLight = (20.0f - (t3dF32)(ExplosionTimer++ -100) / 20.0f));
			Explode50->Mesh->Trasl.y -= HALF_STEP / 3.0f;
		} else
			t3dUpdateExplosion(Explode50->Mesh, IperLight = ((t3dF32)(ExplosionTimer++) / 5.0f));
		IperLight *= 100.0f;
		IperLight += 2000.0f;
		IperLight = 0.0f;
	}

//	Smooth della camera
	dist2 = t3dVectDistance(&t3dCurCamera->Source, &t3dCurCamera->Target);
	dist = t3dVectDistance(&t3dCurCamera->Target, &OldBallCameraTarget);
	if ((dist > 20.0f) && (dist < HALF_STEP * 30.0f) && OldBallCameraTarget.z) {
		t3dVectSub(&tmp, &t3dCurCamera->Target, &OldBallCameraTarget);
		t3dVectNormalize(&tmp);
		tmp *= 20.0f;
		t3dVectAdd(&t3dCurCamera->Target, &OldBallCameraTarget, &tmp);
	}
	dist = t3dVectDistance(&t3dCurCamera->Source, &OldBallCameraSource);
	if ((dist < HALF_STEP * 30.0f) && OldBallCameraSource.z) {
		game._cameraMan->ClipGolfCameraMove(&t3dCurCamera->Source, &OldBallCameraSource, &t3dCurCamera->Target);
		t3dVectSub(&tmp, &t3dCurCamera->Source, &t3dCurCamera->Target);
		t3dVectNormalize(&tmp);
		tmp *= dist2;
		t3dVectAdd(&t3dCurCamera->Source, &t3dCurCamera->Target, &tmp);
	}

	t3dVectCopy(&OldBallCameraSource, &t3dCurCamera->Source);
	t3dVectCopy(&OldBallCameraTarget, &t3dCurCamera->Target);
}

} // End of namespace Watchmaker
