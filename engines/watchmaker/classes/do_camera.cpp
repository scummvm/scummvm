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

#include "watchmaker/classes/do_camera.h"
#include "watchmaker/t3d.h"
#include "watchmaker/utils.h"
#include "watchmaker/globvar.h"
#include "watchmaker/define.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/message.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/ll/ll_mouse.h"
#include "watchmaker/schedule.h"
#include "watchmaker/ll/ll_string.h"
#include "watchmaker/classes/do_string.h"
#include "watchmaker/3d/animation.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/walk/walkutil.h"
#include "watchmaker/renderer.h"

// locals
#define FIRST_PERSON_STEPS      8
#define CAMERA_CARRELLO_DIST    800.0f
#define MAX_CAMERA_MOVE         50.0f
#define MAX_CAMERA_ANGLE        2.0f
#define CAMERA_SUB_STEPS        3

namespace Watchmaker {

/* -----------------20/10/98 10.40-------------------
 *                  PickCamera
 * --------------------------------------------------*/
t3dCAMERA *CameraMan::PickCamera(t3dBODY *b, unsigned char in) {
	// TODO: This is just here until we know when we can expect a nullptr
	if (!b) {
		return nullptr;
	}
	return b->PickCamera(in);
}

t3dCAMERA *t3dBODY::PickCamera(uint8 in) {
	if (NumCameras() == 0) return nullptr;
//	in = 100;

	for (int i = 0; i < (int32)NumCameras(); i++)
		if (CameraTable[i].Index == (in + 1))
			return (&CameraTable[i]);
//	return( &b->CameraTable[in] );

	warning("Camera %d non trovata in %s", in + 1, name.c_str());
	return (&CameraTable[0]);
}


/* -----------------06/07/98 16.55-------------------
 *                  GetRealCharPos
 * --------------------------------------------------*/
void CameraMan::GetRealCharPos(Init &init, t3dV3F *Target, int32 oc, uint8 bn) {
	t3dCHARACTER *Ch = Character[oc];
	t3dMESH *mesh = Ch->Mesh, *m;
	t3dBONE *bone;
	int32 frame = mesh->CurFrame;
	t3dV3F tmp;

	if ((Target != nullptr) && (Ch != nullptr)) {
//		Discesa del garage
		if (t3dCurRoom->name.equalsIgnoreCase("rxt.t3d")) {
			if ((m = LinkMeshToStr(init, "oxt-garage")) && (m->BBox[3].p.x - m->BBox[2].p.x) &&
			        (Player->Pos.x > m->BBox[2].p.x) && (Player->Pos.x < m->BBox[3].p.x) &&
			        (Player->Pos.z > m->BBox[2].p.z) && (Player->Pos.z < m->BBox[6].p.z)) {
				Player->Mesh->Trasl.y = Player->Pos.y = ((Player->Pos.x - m->BBox[2].p.x) / (m->BBox[3].p.x - m->BBox[2].p.x)) * m->BBox[2].p.y;
				CurFloorY = Player->Pos.y;
			}
		}

		t3dVectCopy(Target, &mesh->Trasl);
		Target->y = CurFloorY + CHEST_HEIGHT;
		if ((bn) && (frame > 0)) {
			if (mesh->Flags & T3D_MESH_DEFAULTANIM)
				bone = &mesh->DefaultAnim.BoneTable[bn];
			else
				bone = &mesh->Anim.BoneTable[bn];

			if ((bone) && (bone->Trasl) && (bone->Matrix)) {
				Target->y = CurFloorY;
				t3dVectSub(&tmp, &bone->Trasl[frame], &bone->Trasl[1]);
				t3dVectTransform(&tmp, &tmp, &bone->Matrix[1]);
				t3dVectTransformInv(&tmp, &tmp, &bone->Matrix[frame]);
				t3dVectAdd(&tmp, &tmp, &bone->Trasl[frame]);
				t3dVectAdd(Target, Target, &tmp);
			}
		}
	}
}

/* -----------------21/08/98 14.38-------------------
 *      Dist Point C from AB -> Inters in I
 * --------------------------------------------------*/
uint8 DistPointRect(t3dV3F *i, t3dF32 *dist, t3dV3F *a, t3dV3F *b, t3dV3F *c) {
	t3dF32 d, r;

	d = ((b->x - a->x) * (b->x - a->x) + (b->z - a->z) * (b->z - a->z));
	r = ((a->z - c->z) * (a->z - b->z) - (a->x - c->x) * (b->x - a->x)) / d;

	if (r <= 0.0f) {                     // Prima di A
		t3dVectCopy(i, a);
		*dist = t3dVectDistance(c, i);
		return FALSE;
	} else if (r >= 1.0f) {              // Dopo B
		t3dVectCopy(i, b);
		*dist = t3dVectDistance(c, i);
		return FALSE;
	} else {                            // Tra A e B
		i->x = a->x + r * (b->x - a->x);
		i->y = a->y + r * (b->y - a->y);
		i->z = a->z + r * (b->z - a->z);
		*dist = t3dVectDistance(c, i);
		return TRUE;
	}
}


/* -----------------20/08/98 17.16-------------------
 *              HandleCameraCarrello
 * --------------------------------------------------*/
void CameraMan::HandleCameraCarrello(t3dBODY *croom) {
	t3dCAMERAPATH *cp = nullptr;
	t3dF32 dist, mindist;
	t3dV3F pt, i, b;
	int16 j;
	uint8 bcc = false;

	if (!Player || !croom || !t3dCurCamera) return;

//	Se e' appena partito il carrello
	if (!bCameraCarrello) {
		CameraCarrello = *t3dCurCamera;
		t3dCurCamera = &CameraCarrello;
	}

//	Cerca percorso carrello
	for (j = 0; j < CameraCarrello.NumAvailablePaths(); j++)
		if (CameraCarrello.CameraPaths[j].PathIndex & 0x80)
			break;
//	Puntatore al perscorso carrello, se non lo trova esce
	if ((j >= CameraCarrello.NumAvailablePaths()) ||
	        ((cp = &croom->CameraPath[(CameraCarrello.CameraPaths[j].PathIndex) & 0x7F]) == nullptr))
		return ;

//	Trova il punto  dell'omino che deve seguire il source del carrello
	t3dVectFill(&b, 0.0f);
	t3dVectFill(&pt, 0.0f);
	pt.z = (t3dF32)(cp->CarrelloDist);
	t3dVectTransform(&pt, &pt, &Player->Mesh->Matrix);
	t3dVectAdd(&pt, &pt, &t3dCurCamera->Target);

//	Cerca punto ottimale del carrello
	mindist = 9999999.9f;
	for (j = 0; j < cp->NumPoints() - 1; j++) {
		DistPointRect(&i, &dist, &cp->PList[j], &cp->PList[j + 1], &pt);
		if (dist < mindist) {
			t3dVectCopy(&b, &i);
			bcc = true;
			mindist = dist;
		}
	}

//	Se decide di spostare source camera carrello
	if (bcc) {
//		Se e' appena partito carrello, puo' fare scatto
		if (!bCameraCarrello)
			t3dVectCopy(&t3dCurCamera->Source, &b);
		else {
//			Se dorvebbe fare scatto, inizia a smussare
			if (t3dVectDistance(&t3dCurCamera->Source, &b) > MAX_CAMERA_MOVE) {
				t3dVectSub(&i, &b, &t3dCurCamera->Source);
				t3dVectNormalize(&i);
				i *= MAX_CAMERA_MOVE;
				t3dVectAdd(&t3dCurCamera->Source, &t3dCurCamera->Source, &i);
			} else
				t3dVectCopy(&t3dCurCamera->Source, &b);
		}
		bCameraCarrello = bcc;
	}
}

void CameraMan::MoveHeadAngles(t3dF32 diffx, t3dF32 diffy) {
	t3dF32 s;

	if (((diffx == 0) && (diffy == 0)) || (bLPressed) || (bRPressed) || (bDialogActive)) return;

	if (diffx < -10) diffx = -10;
	else if (diffx > 10) diffx = 10;
	if (diffy < -10) diffy = -10;
	else if (diffy > 10) diffy = 10;

	s = (t3dF32)bFirstPerson + 1.0f;
	if (diffx > 0) {
		if ((HeadAngles.x + diffx) >= MAX_HEAD_ANGLE_X * s) {
			diffx = MAX_HEAD_ANGLE_X * s - HeadAngles.x;
			HeadAngles.x = MAX_HEAD_ANGLE_X * s;
		} else
			HeadAngles.x += diffx;
	} else {
		if ((HeadAngles.x + diffx) < -MAX_HEAD_ANGLE_X * s) {
			diffx = -MAX_HEAD_ANGLE_X * s - HeadAngles.x;
			HeadAngles.x = -MAX_HEAD_ANGLE_X * s;
		} else
			HeadAngles.x += diffx;
	}

	if (diffy > 0) {
		if ((HeadAngles.y + diffy) >= MAX_HEAD_ANGLE_Y * s) {
			diffy = MAX_HEAD_ANGLE_Y * s - HeadAngles.y;
			HeadAngles.y = MAX_HEAD_ANGLE_Y * s;
		} else
			HeadAngles.y += diffy;
	} else {
		if ((HeadAngles.y + diffy) < -MAX_HEAD_ANGLE_Y * s) {
			diffy = -MAX_HEAD_ANGLE_Y * s - HeadAngles.y;
			HeadAngles.y = -MAX_HEAD_ANGLE_Y * s;
		} else
			HeadAngles.y += diffy;
	}

	CamAngleX = ((t3dF32)diffy / 180.0f * T3D_PI);
	CamAngleY = ((t3dF32)diffx / 180.0f * T3D_PI);
	if (bFirstPerson && !bLockCamera && ((CamAngleX != 0.0f) || (CamAngleY != 0.0f)))
		t3dRotateMoveCamera(t3dCurCamera, CamAngleX, CamAngleY, 0.0f);
}

void CameraMan::resetAngle() {
	CamAngleX = 0.0f;
	CamAngleY = 0.0f;
	t3dVectFill(&HeadAngles, 0.0f);
}

/* -----------------30/09/98 11.13-------------------
 *                  GetCameraTaget
 * --------------------------------------------------*/
void CameraMan::GetCameraTarget(Init &init, t3dV3F *Target) {
	int32 i;

	if (!Target) return;

	if (bFirstPerson)                                    //  Se sono in prima persona e' gia' precalcolato
		t3dVectCopy(Target, &FirstPersonTarget);
	else if ((Player) && (!CameraTargetObj))           //  Se non e' settato niente oppure e' il giocatore
		GetRealCharPos(init, Target, ocCURPLAYER, 0);
	else if ((CameraTargetObj == oCAMERAMAX) && (t3dCurCamera)) {
		if ((bAllowCalcCamera) && (bMovingCamera) && (CurCameraStep < NumCameraSteps)) {        // se li ha gia' precalcolati
			t3dVectCopy(Target, &CameraStep[CurCameraStep].Target);
			for (i = 0; i < CurCameraSubStep; i++)
				t3dVectAdd(Target, Target, &TargetBlend);
		} else
			t3dVectCopy(Target, &t3dCurCamera->MaxTarget);
	} else
		GetRealCharPos(init, Target, CameraTargetObj, (uint8)CameraTargetBone);
}

/* -----------------05/06/98 15.34-------------------
 *                  NextCameraStep
 * --------------------------------------------------*/
void CameraMan::NextCameraStep(WGame &game) {
	t3dBONE *bone;
	t3dV3F Target;
	int16 i;
	Init &init = game.init;

	if (bMovingCamera == 2) {
		if ((game.init._globals._invVars.CameraDummy.CurFrame >= 0) && (game.init._globals._invVars.CameraDummy.Anim.BoneTable)) {
			if ((bone = &game.init._globals._invVars.CameraDummy.Anim.BoneTable[33]) && (bone->Trasl))
				t3dVectCopy(&t3dCurCamera->Source, &bone->Trasl[game.init._globals._invVars.CameraDummy.CurFrame]);
			if ((bone = &game.init._globals._invVars.CameraDummy.Anim.BoneTable[34]) && (bone->Trasl))
				t3dVectCopy(&t3dCurCamera->Target, &bone->Trasl[game.init._globals._invVars.CameraDummy.CurFrame]);
		} else {
			t3dCurCamera = DestCamera;
			CurCameraSubStep = CurCameraStep = NumCameraSteps = 0;
			bMovingCamera = false;
			DestCamera = nullptr;
			t3dVectFill(&OldCameraTarget, 0.0f);
//			_vm->_messageSystem.addWaitingMsgs( MP_WAIT_CAMERA );

			GetCameraTarget(init, &t3dCurCamera->Target);
			game._renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);
		}
		return ;
	}

	if (!bMovingCamera || !Player) return;

	GetCameraTarget(init, &Target);

//	Se ha finito di muoversi
	if ((NumCameraSteps != 0) && (CurCameraStep >= NumCameraSteps)) {
		t3dCurCamera = DestCamera;
		t3dVectCopy(&t3dCurCamera->Target, &Target);
		game._renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);

		CurCameraStep = NumCameraSteps = 0;
		bMovingCamera = false;
		DestCamera = nullptr;
		_vm->_messageSystem.addWaitingMsgs(MP_WAIT_CAMERA);

//		if( (bFirstPerson) && ( ( CurRoom == r32 ) && ( PlayerPos[CurPlayer+ocDARRELL] == 6 ) ) )
//			PlayerSpeak( Obj[o32OROLOGIO].action[CurPlayer+ocDARRELL] );
		if ((bFirstPerson) && (ToFirstPersonSent)) {
			PlayerSpeak(game, ToFirstPersonSent);
			ToFirstPersonSent = 0;
		}

//		Se ho selezionato un carrello, riposiziona il source
		for (i = 0; i < t3dCurCamera->NumAvailablePaths(); i++)
			if (t3dCurCamera->CameraPaths[i].PathIndex & 0x80) {
				HandleCameraCarrello(t3dCurRoom);
				break;
			}
	} else {
		t3dCurCamera = &CameraStep[CurCameraStep];
		t3dVectCopy(&t3dCurCamera->Target, &Target);

		if ((!CurCameraSubStep) && (CurCameraStep + 1 < NumCameraSteps)) {
			t3dVectSub(&SourceBlend, &CameraStep[CurCameraStep + 1].Source, &CameraStep[CurCameraStep].Source);
			SourceBlend *= (1.0f / (t3dF32)CAMERA_SUB_STEPS);

			t3dVectSub(&TargetBlend, &CameraStep[CurCameraStep + 1].Target, &CameraStep[CurCameraStep].Target);
			TargetBlend *= (1.0f / (t3dF32)CAMERA_SUB_STEPS);
		} else if (CurCameraStep + 1 < NumCameraSteps)
			t3dVectAdd(&t3dCurCamera->Source, &t3dCurCamera->Source, &SourceBlend);

		game._renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);
		if (++CurCameraSubStep >= CAMERA_SUB_STEPS) {
			CurCameraStep ++;
			CurCameraSubStep = 0;
		}
	}
}


/* -----------------05/06/98 15.50-------------------
 *                      doCamera
 * --------------------------------------------------*/
void CameraMan::doCamera(WGame &game) {
	t3dV3F Dest, Dir, ct;
	t3dF32 dist;
	int16 row, col, i;
	Init &init = game.init;

	switch (TheMessage->event) {
	case ME_CAMERA3TO1:
		if (bMovingCamera) {
			TheMessage->flags |= MP_WAIT_CAMERA;
			ReEvent();
		}
		if (Player == nullptr) break;

		mHide = true;
		bFirstPerson = true;

		CharStop(ocCURPLAYER);
		game._messageSystem.removeEvent(EventClass::MC_PLAYER, ME_ALL);
		ClearText();

		LastCamera = t3dCurCamera;
		DestCamera = &FirstPersonCamera;
		DestCamera->Fov = CAMERA_FOV_1ST;
//			LastCamera->Fov = CAMERA_FOV;

//			Abilita la modalita' muovi camera
		bMovingCamera = true;

//			Seleziona il punto di destinazione del Source
		GetRealCharPos(init, &Dest, ocCURPLAYER, 0);
		Dest.y = CurFloorY + EYES_HEIGHT;
		t3dVectCopy(&FirstPersonCamera.Source, &Dest);
//			Trova il target a cui puntare
		t3dVectFill(&HeadAngles, 0.0f);
		CamAngleX = 0.0f;
		CamAngleY = 0.0f;
		t3dVectNormalize(&Player->Dir);
		FirstPersonTarget = Player->Dir * CHEST_HEIGHT;
		t3dVectAdd(&FirstPersonTarget, &FirstPersonTarget, &Player->Mesh->Trasl);
		FirstPersonTarget.y = CurFloorY + EYES_HEIGHT;
		if (TheMessage->bparam) {
			if (GetFullLightDirection(&FirstPersonTarget, TheMessage->bparam)) {
				Dest.y = FirstPersonTarget.y;
				FirstPersonCamera.Source.y = FirstPersonTarget.y;
//					HeadAngles.y = -(t3dF32)asin( (FirstPersonTarget.y-(CurFloorY+EYES_HEIGHT))/CHEST_HEIGHT )*180.0f/T3D_PI;
			}
		}
//			Calcola quanto deve muovere la camera
		dist = t3dVectDistance(&LastCamera->Source, &Dest);
		t3dVectSub(&Dir, &LastCamera->Source, &Dest);
		t3dVectNormalize(&Dir);
		Dir *= (dist / FIRST_PERSON_STEPS);

		CurCameraStep = 0;
		CurCameraSubStep = 0;
		NumCameraSteps = FIRST_PERSON_STEPS;
//			Copia tutti i passi del percorso partendo dall'arrivo
		for (i = NumCameraSteps - 1; i >= 0; i--) {
			t3dVectCopy(&CameraStep[i].Source, &Dest);
			t3dVectAdd(&Dest, &Dest, &Dir);
			CameraStep[i].Fov = LastCamera->Fov + (t3dF32)(((DestCamera->Fov - LastCamera->Fov) * (t3dF32)i) / (t3dF32)NumCameraSteps);
		}

//			Avanza al primo frame del percorso
		NextCameraStep(game);
		game._messageSystem.removeEvent(EventClass::MC_PLAYER, ME_PLAYERIDLE);

//			Sistema posizione del mouse e posizione della testa
		{
			auto info = game._renderer->getScreenInfos();
			mPosx = info.width / 2;
			mPosy = info.height / 2;
		}
		Player->Flags |= T3D_CHARACTER_ENABLEDINMIRROR;
		break;

	case ME_CAMERA1TO3:
		if (bMovingCamera) {
			TheMessage->flags |= MP_WAIT_CAMERA;
			ReEvent();
		}
		if (Player == nullptr) break;

		mHide = true;
		bFirstPerson = false;

//			CharStop( Player );
		ClearText();

		if (FromFirstPersonAnim)
			CharGotoPosition(game, ocCURPLAYER, init.Anim[FromFirstPersonAnim].pos, 0, FromFirstPersonAnim);
		DestCamera = LastCamera;
		if (TheMessage->bparam) DestCamera = t3dCurCamera;
		LastCamera = &FirstPersonCamera;
//			DestCamera->Fov = CAMERA_FOV;
		LastCamera->Fov = CAMERA_FOV_1ST;

//			Abilita la modalita' muovi camera
		bMovingCamera = true;

//			Seleziona il punto di destinazione del Source
		t3dVectCopy(&Dest, &Player->Mesh->Trasl);
		Dest.y = CurFloorY + EYES_HEIGHT;
		t3dVectCopy(&FirstPersonCamera.Source, &Dest);
//			Calcola quanto deve muovere la camera
		dist = t3dVectDistance(&DestCamera->Source, &Dest);
		t3dVectSub(&Dir, &DestCamera->Source, &Dest);
		t3dVectNormalize(&Dir);
		Dir *= (dist / FIRST_PERSON_STEPS);

		CurCameraStep = 0;
		CurCameraSubStep = 0;
		NumCameraSteps = FIRST_PERSON_STEPS;
//			Copia tutti i passi del percorso partendo dall'arrivo
		for (i = 0; i < NumCameraSteps; i++) {
			t3dVectCopy(&CameraStep[i].Source, &Dest);
			t3dVectAdd(&Dest, &Dest, &Dir);
			CameraStep[i].Fov = LastCamera->Fov + (t3dF32)(((DestCamera->Fov - LastCamera->Fov) * (t3dF32)i) / (t3dF32)NumCameraSteps);
		}
//			Azzera un po' di variabili
		t3dVectFill(&HeadAngles, 0.0f);
		CamAngleX = 0.0f;
		CamAngleY = 0.0f;
//			Avanza al primo frame del percorso
		NextCameraStep(game);

		FromFirstPersonAnim = aNULL;
		if (Player)
			Player->Flags &= ~T3D_CHARACTER_HIDE;
		Player->Flags &= ~T3D_CHARACTER_ENABLEDINMIRROR;
		break;

	case ME_CAMERAPLAYER:
		if (bMovingCamera) {
			TheMessage->flags |= MP_WAIT_CAMERA;
			ReEvent();
		}
		if ((Player == nullptr) || (t3dCurRoom->CameraGrid.Grid.empty())) break;

		GetCameraTarget(init, &ct);
		col = (int16)((ct.x - t3dCurRoom->CameraGrid.TopLeft.x) / t3dCurRoom->CameraGrid.CellDim.x);
		row = (int16)((ct.z - t3dCurRoom->CameraGrid.TopLeft.z) / t3dCurRoom->CameraGrid.CellDim.z);
		if (((col < 0 || row < 0) || (col >= t3dCurRoom->CameraGrid.Col) || (row >= t3dCurRoom->CameraGrid.Row))) return;

		t3dLastCameraIndex = t3dCurCameraIndex;
		t3dCurCameraIndex = t3dCurRoom->CameraGrid.Grid[col + row * t3dCurRoom->CameraGrid.Col];

		mHide = true;
		bFirstPerson = false;
		ClearText();

		LastCamera = t3dCurCamera;
		DestCamera = PickCamera(t3dCurRoom, t3dCurCameraIndex);
//			DestCamera->Fov = CAMERA_FOV;
//			LastCamera->Fov = CAMERA_FOV;

//			Abilita la modalita' muovi camera
		bMovingCamera = true;

//			Seleziona il punto di destinazione del Source
		t3dVectCopy(&Dest, &Player->Mesh->Trasl);
		Dest.y = CurFloorY + EYES_HEIGHT;

//			Calcola quanto deve muovere la camera
		dist = t3dVectDistance(&DestCamera->Source, &Dest);
		t3dVectSub(&Dir, &DestCamera->Source, &Dest);
		t3dVectNormalize(&Dir);
		Dir *= (dist / FIRST_PERSON_STEPS);

		CurCameraStep = 0;
		CurCameraSubStep = 0;
		NumCameraSteps = FIRST_PERSON_STEPS;
//			Copia tutti i passi del percorso partendo dall'arrivo
		for (i = 0; i < NumCameraSteps; i++) {
			t3dVectCopy(&CameraStep[i].Source, &Dest);
			t3dVectAdd(&Dest, &Dest, &Dir);
			CameraStep[i].Fov = LastCamera->Fov + (t3dF32)(((DestCamera->Fov - LastCamera->Fov) * (t3dF32)i) / (t3dF32)NumCameraSteps);
		}
//			Azzera un po' di variabili
		t3dVectFill(&HeadAngles, 0.0f);
		CamAngleX = 0.0f;
		CamAngleY = 0.0f;
//			Avanza al primo frame del percorso
		NextCameraStep(game);

		FromFirstPersonAnim = aNULL;
		if (Player)
			Player->Flags &= ~T3D_CHARACTER_HIDE;
		Player->Flags &= ~T3D_CHARACTER_ENABLEDINMIRROR;
		break;
	}
}

/* -----------------09/11/98 10.27-------------------
 *                  ResetCameraSource
 * --------------------------------------------------*/
void CameraMan::ResetCameraSource() {
	resetLastCameraIndex();
	t3dCurCameraIndex = 255;
}

/* -----------------09/11/98 10.27-------------------
 *                  ResetCameraTarget
 * --------------------------------------------------*/
void CameraMan::ResetCameraTarget() {
	t3dVectFill(&OldCameraTarget, 0.0f);
}

/* -----------------09/11/98 10.32-------------------
 *                  ClipCameraMove
 * --------------------------------------------------*/
uint8 ClipCameraMove(t3dV3F *NewT, t3dV3F *OldT, t3dV3F *Source) {
	t3dV3F n, o;
	t3dF32 a, d, l;

	t3dVectSub(&n, NewT, Source);
	t3dVectSub(&o, OldT, Source);
	t3dVectNormalize(&n);
	t3dVectNormalize(&o);

	a = t3dVectAngle(&o, &n);

	if (a > MAX_CAMERA_ANGLE) d = (MAX_CAMERA_ANGLE * T3D_PI) / 180.0f;
	else if (a < -MAX_CAMERA_ANGLE) d = -(MAX_CAMERA_ANGLE * T3D_PI) / 180.0f;
	else return 0;

	a = (a * T3D_PI) / 180.0f;
	t3dVectSub(&o, NewT, OldT);
	t3dVectNormalize(&o);
	l = (t3dVectDistance(NewT, OldT) * d) / a;
	o *= l;
	t3dVectAdd(NewT, OldT, &o);

	/*  t3dVectSub( &o, OldT, Source );
	    t3dVectNormalize( &o );
	    l = t3dVectDistance( OldT, Source );
	//  l += ( ( l - t3dVectDistance( NewT, Source ) ) * d ) / a;
	    t3dVectScale( &o, &o, l );

	    t3dMatRot( &m, 0.0f, d, 0.0f );
	    t3dVectTransform( NewT, &o, &m );
	    t3dVectAdd( NewT, NewT, Source );

	    NewT->y += ( ( NewT->y - OldT->y ) * d ) / a;
	*/
	return 1;
}


/* -----------------05/06/98 10.36-------------------
 *                  ProcessCamera
 * --------------------------------------------------*/
void CameraMan::ProcessCamera(WGame &game) {
	t3dCAMERAPATH *cp = nullptr;
	t3dBODY *croom;
	int16 row, col, i;
	t3dV3F Dest, Dir;
	t3dF32 dist;
	int32 a, b;
	t3dV3F ct;
	int8 cd;
	uint8 cc;
	int32 foxOldRoom;
	Init &init = game.init;

	if (bMovingCamera && !PortalCrossed) {       // Se la camera si sta muovendo
		NextCameraStep(game);
		return ;
	}
	if (bFirstPerson && !PortalCrossed) {    // se sono in prima persona non devo cambiare camera
		return;
	}

	if (PortalCrossed != nullptr) {                 // Se ho cambiato stanza
		croom = PortalCrossed;                  // Nuova stanza
		t3dVectFill(&OldCameraTarget, 0.0f);
		bForceDirectCamera = TRUE;
		t3dCurCameraIndex = 255;
		resetLastCameraIndex();
		_vm->_messageSystem.addWaitingMsgs(MP_WAIT_PORTAL);
//		DebugLogFile("PortalCrossed %s",PortalCrossed->Name);
//		PortalCrossed = nullptr;
		if (bMovingCamera) {
			CurCameraStep = NumCameraSteps = 0;
			bMovingCamera = FALSE;
			DestCamera = nullptr;
			_vm->_messageSystem.addWaitingMsgs(MP_WAIT_CAMERA);
		}
		if (bFirstPerson)
			_vm->_messageSystem.doEvent(EventClass::MC_CAMERA, ME_CAMERA3TO1, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);
//		Fa partire animazione di link portali se l'ho attraversato con i tasti
		for (i = 0; i < (int16)t3dCurRoom->NumMeshes(); i++) {
			if (t3dCurRoom->MeshTable[i].PortalList == PortalCrossed) {
				for (a = 0; a < MAX_ANIMS_IN_ROOM; a++) {
					b = game.getCurRoom().anims[a];
					if ((b == aNULL) || !(init.Anim[b].flags & ANIM_PORTAL_LINK)) continue;

					if (t3dCurRoom->MeshTable[i].name.equalsIgnoreCase((char *)init.Anim[b].RoomName.rawArray()) && (PlayerGotoPos[CurPlayer + ocDARRELL] != init.Anim[b].pos)) {
						if (!(init.Anim[b].flags & ANIM_NULL))
							CharGotoPosition(game, ocCURPLAYER, init.Anim[b].pos, 10, b);
						break;
					}
				}
			}
		}
		foxOldRoom = game._gameVars.getCurRoomId();
		game._gameVars.setCurRoomId(getRoomFromStr(init, croom->name));
		t3dCurRoom = PortalCrossed;
		PortalCrossed = nullptr;
		UpdateRoomVisibility(game);

		if (Player && t3dCurRoom) {
			Player->Walk.Panel = t3dCurRoom->Panel[t3dCurRoom->CurLevel];
			Player->Walk.PanelNum = t3dCurRoom->NumPanels[t3dCurRoom->CurLevel];
			Player->Walk.CurPanel = -1;
			Player->Walk.OldPanel = -1;
			for (a = 0; a < Player->Walk.NumSteps; a++)
				Player->Walk.WalkSteps[a].curp = -1;
			CurFloorY = t3dCurRoom->PanelHeight[t3dCurRoom->CurLevel];
		}


//		Parte Morte Victoria se esce dalla r49 per andare nella r48 prima di aver attivato le leylines
		if (
		    (game._gameVars.getCurRoomId() == r48) && (foxOldRoom == r49)
		    && (!(init.Dialog[dR491].flags & DIALOG_DONE))
		    && (!(LoaderFlags & T3D_DEBUGMODE))
		    && (!bDialogActive)
		) {
//			DebugLogFile("BECCATO222");
			CharStop(ocCURPLAYER);
			_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR48KRENNSPARA, 0, 0, nullptr, nullptr, nullptr);
		}

//		per sicurezza faccio sparire la bottiglia
		if (game._gameVars.getCurRoomId() == r25) {
			t3dMESH *m = LinkMeshToStr(init, "o25-fiaschetta");
			if (m) m->Flags |= T3D_MESH_HIDDEN;
		}

	} else {
		croom = t3dCurRoom;                     //  Stanza attuale
		if (ForcedCamera && bCutCamera)      // Se ho settato un camera-cut
			bForceDirectCamera = TRUE;
		else
			bForceDirectCamera = FALSE;
	}

//	Se non c'e' la griglia o non c'e' il personaggio o la camera attuale
	if ((croom->CameraGrid.Grid.empty()) || (Player == nullptr) || (t3dCurCamera == nullptr)) return;
	GetCameraTarget(init, &ct);                         // Si calcola quale sarebbe il Target giusto per la camera

//	Se non devo spostare la camera esco subito
	if ((ct == OldCameraTarget) && (!ForcedCamera || (t3dCurCameraIndex == ForcedCamera - 1))) {
		if ((CamAngleX != 0.0f) || (CamAngleY != 0.0f))
			t3dRotateMoveCamera(t3dCurCamera, CamAngleX, CamAngleY, 0.0f);
		return ;
	}
	if (!bFirstPerson) t3dVectFill(&HeadAngles, 0.0f);
//	Mi calcolo in che casella sta l'omino
	col = (int16)((ct.x - croom->CameraGrid.TopLeft.x) / croom->CameraGrid.CellDim.x);
	row = (int16)((ct.z - croom->CameraGrid.TopLeft.z) / croom->CameraGrid.CellDim.z);
//	Se la camera si muoverebbe troppo, riduce il movimento
	if (!bForceDirectCamera) ClipCameraMove(&ct, &t3dCurCamera->Target, &t3dCurCamera->Source);
//  Se sono in un dialogo elimina la gestione del trova-camera
	if (!ForcedCamera && bDialogActive) return ;
//	Aggiorno Camera
	t3dVectCopy(&t3dCurCamera->Target, &ct);
	t3dVectCopy(&OldCameraTarget, &ct);
//	Se e' un carrello, aggiorno carrello
	if (bCameraCarrello) HandleCameraCarrello(croom);         // Se carrello attivo, riposiziona il source
//	Se non e' valida esco
	if (!ForcedCamera && ((col < 0 || row < 0) || (col >= croom->CameraGrid.Col) || (row >= croom->CameraGrid.Row))) return;
//	Prendo l'indice della nuova stanza e se e' in una zona libera esce
	cc = croom->CameraGrid.Grid[col + row * croom->CameraGrid.Col];
//	Se e' in zona libera e la vecchia telecamera era in zona libera, ricerca a spirale
	if ((cc == 255) && (t3dLastCameraIndex == 255)) {
		for (a = 1; a < 10; a++) {
			for (b = -a; b <= a; b++) {
				if ((col + b) >= 0 || (row + a) >= 0 || (col + b) < croom->CameraGrid.Col || (row + a) < croom->CameraGrid.Row)
					if ((cc = croom->CameraGrid.Grid[col + b + (row + a) * croom->CameraGrid.Col]) != 255) break;
				if ((col + b) >= 0 || (row - a) >= 0 || (col + b) < croom->CameraGrid.Col || (row - a) < croom->CameraGrid.Row)
					if ((cc = croom->CameraGrid.Grid[col + b + (row - a) * croom->CameraGrid.Col]) != 255) break;
				if ((col + a) >= 0 || (row + b) >= 0 || (col + a) < croom->CameraGrid.Col || (row + b) < croom->CameraGrid.Row)
					if ((cc = croom->CameraGrid.Grid[col + a + (row + b) * croom->CameraGrid.Col]) != 255) break;
				if ((col - a) >= 0 || (row + b) >= 0 || (col - a) < croom->CameraGrid.Col || (row + b) < croom->CameraGrid.Row)
					if ((cc = croom->CameraGrid.Grid[col - a + (row + b) * croom->CameraGrid.Col]) != 255) break;
			}
			if (cc != 255) break;
		}
	}
//	Se resta sempre in una zona libera esce
	if (!ForcedCamera && (cc == 255)) return;
//	Mi salvo l'ultima posizione della camera e la posizione attuale
	t3dLastCameraIndex = t3dCurCameraIndex;
//	Se obbligo a prendre una camera particolare per l'animazione
	if (ForcedCamera) t3dCurCameraIndex = ForcedCamera - 1;
	else t3dCurCameraIndex = cc;
//	Se non deve cambiare camera ed e' nella stessa stanza esce
	if ((t3dCurCameraIndex == t3dLastCameraIndex) && !(bForceDirectCamera && !bCutCamera)) return;
	bCameraCarrello = FALSE;
//	Puntatore alla vecchia e alla nuova camera
	LastCamera = t3dCurCamera;
	DestCamera = PickCamera(croom, t3dCurCameraIndex);

//	DestCamera->Fov = CAMERA_FOV;
//	LastCamera->Fov = CAMERA_FOV;

	i = 255;
//	Cerca il percorso che mi porta la veccia camera nella nuova camera
	if (LastCamera->NumAvailablePaths() < 255)
		for (i = 0; i < LastCamera->NumAvailablePaths(); i++)
			if (LastCamera->CameraPaths[i].NumCamera == t3dCurCameraIndex)
				break;

//	Se non lo trova o se deve cambiare la camera senza fare il percorso e non puo' calcolarselo
	if (((i >= LastCamera->NumAvailablePaths()) || (!LastCamera->NumAvailablePaths()) || bForceDirectCamera) && !bAllowCalcCamera) {
//		Setta la nuova camera ed esce
		t3dCurCamera = PickCamera(croom, t3dCurCameraIndex);
		GetCameraTarget(init, &t3dCurCamera->Target);
//		t3dCurCamera->Fov = CAMERA_FOV;
		game._renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);
		DestCamera = nullptr;
		bMovingCamera = FALSE;
		bForceDirectCamera = FALSE;
//		Se ho selezionato un carrello, riposiziona il source
		for (i = 0; i < t3dCurCamera->NumAvailablePaths(); i++)
			if (t3dCurCamera->CameraPaths[i].PathIndex & 0x80) {
				HandleCameraCarrello(croom);
				break;
			}
		return ;
	}

//	Se non trova percorsi e puo' calcolarsi il percorso
	if ((bAllowCalcCamera) && ((i >= LastCamera->NumAvailablePaths()) || (!LastCamera->NumAvailablePaths()))) {
		t3dVectCopy(&Dest, &DestCamera->Source);
		dist = t3dVectDistance(&LastCamera->Source, &Dest);              // Calcola quanto deve muovere la camera
		t3dVectSub(&Dir, &LastCamera->Source, &Dest);
		t3dVectNormalize(&Dir);

		CurCameraStep = 0;
		CurCameraSubStep = 0;
		NumCameraSteps = (int16)(dist / MAX_CAMERA_MOVE) + 1;
		if (NumCameraSteps > MAX_CAMERA_STEPS) NumCameraSteps = MAX_CAMERA_STEPS - 1;
		Dir *= (dist / NumCameraSteps);
//		Copia tutti i passi del percorso partendo dall'arrivo
		for (i = NumCameraSteps - 1; i >= 0; i--) {
			t3dVectCopy(&CameraStep[i].Source, &Dest);
			t3dVectAdd(&Dest, &Dest, &Dir);
			CameraStep[i].Fov = LastCamera->Fov + (t3dF32)(((DestCamera->Fov - LastCamera->Fov) * (t3dF32)i) / (t3dF32)NumCameraSteps);
		}

		if (CameraTargetObj == oCAMERAMAX) {                             // Se deve mantenere il target di max
			t3dVectCopy(&Dest, &DestCamera->MaxTarget);
			dist = t3dVectDistance(&LastCamera->Target, &Dest);          // Calcola quanto deve muovere il target
			t3dVectSub(&Dir, &LastCamera->Target, &Dest);
			t3dVectNormalize(&Dir);
			Dir *= (dist / NumCameraSteps);
			for (i = NumCameraSteps - 1; i >= 0; i--) {
				t3dVectCopy(&CameraStep[i].MaxTarget, &Dest);
				t3dVectCopy(&CameraStep[i].Target, &Dest);
				t3dVectAdd(&Dest, &Dest, &Dir);
			}
		}
	} else {
		cp = &croom->CameraPath[(LastCamera->CameraPaths[i].PathIndex) & 0x7F];
		cd = LastCamera->CameraPaths[i].Direction;

		CurCameraStep = 0;
		CurCameraSubStep = 0;
		NumCameraSteps = cp->NumPoints();
		if (NumCameraSteps > MAX_CAMERA_STEPS) NumCameraSteps = MAX_CAMERA_STEPS - 1;
		for (i = 0; i < NumCameraSteps; i++) {           // Copia tutti i passi del percorso
			if (cd == 0)
				t3dVectCopy(&CameraStep[i].Source, &cp->PList[cp->NumPoints() - 1 - i]);
			else
				t3dVectCopy(&CameraStep[i].Source, &cp->PList[i]);

			CameraStep[i].Fov = LastCamera->Fov + (t3dF32)(((DestCamera->Fov - LastCamera->Fov) * (t3dF32)i) / (t3dF32)NumCameraSteps);
		}
	}
	bMovingCamera = TRUE;                           // Abilita la modalita' muovi camera
	NextCameraStep(game);                               // Avanza al primo frame del percorso
}

/* -----------------16/12/00 15.20-------------------
 *                  GetCameraIndexUnderPlayer
 * --------------------------------------------------*/
uint8 CameraMan::GetCameraIndexUnderPlayer(int32 pl) {
	t3dBODY *croom;
	int16 row, col;
	int32 a, b;
	t3dV3F ct;
	uint8 cc;

	croom = t3dCurRoom;                     //  Stanza attuale
	if ((!croom) || (!Character[pl])) return (255);

//	Se non c'e' la griglia o non c'e' il personaggio o la camera attuale
	if (croom->CameraGrid.Grid.empty()) return (255);

	t3dVectCopy(&ct, &Character[pl]->Mesh->Trasl);
//	Mi calcolo in che casella sta l'omino
	col = (int16)((ct.x - croom->CameraGrid.TopLeft.x) / croom->CameraGrid.CellDim.x);
	row = (int16)((ct.z - croom->CameraGrid.TopLeft.z) / croom->CameraGrid.CellDim.z);
//	Aggiorno Camera
//	Se non e' valida esco
	if (((col < 0 || row < 0) || (col >= croom->CameraGrid.Col) || (row >= croom->CameraGrid.Row))) return (255);
//	Prendo l'indice della nuova stanza e se e' in una zona libera esce
	cc = croom->CameraGrid.Grid[col + row * croom->CameraGrid.Col];
//	Se e' in zona libera e la vecchia telecamera era in zona libera, ricerca a spirale
	if (cc == 255) {
		for (a = 1; a < 10; a++) {
			for (b = -a; b <= a; b++) {
				if ((col + b) >= 0 || (row + a) >= 0 || (col + b) < croom->CameraGrid.Col || (row + a) < croom->CameraGrid.Row)
					if ((cc = croom->CameraGrid.Grid[col + b + (row + a) * croom->CameraGrid.Col]) != 255) break;
				if ((col + b) >= 0 || (row - a) >= 0 || (col + b) < croom->CameraGrid.Col || (row - a) < croom->CameraGrid.Row)
					if ((cc = croom->CameraGrid.Grid[col + b + (row - a) * croom->CameraGrid.Col]) != 255) break;
				if ((col + a) >= 0 || (row + b) >= 0 || (col + a) < croom->CameraGrid.Col || (row + b) < croom->CameraGrid.Row)
					if ((cc = croom->CameraGrid.Grid[col + a + (row + b) * croom->CameraGrid.Col]) != 255) break;
				if ((col - a) >= 0 || (row + b) >= 0 || (col - a) < croom->CameraGrid.Col || (row + b) < croom->CameraGrid.Row)
					if ((cc = croom->CameraGrid.Grid[col - a + (row + b) * croom->CameraGrid.Col]) != 255) break;
			}
			if (cc != 255) break;
		}
	}

	return (cc);
}

/* -----------------08/10/98 18.08-------------------
 *                  StartAnimCamera
 * --------------------------------------------------*/
void CameraMan::StartAnimCamera(WGame &game) {
	DebugLogFile("StartAnimCamera");
	t3dLastCameraIndex = t3dCurCameraIndex;
	if (ForcedCamera) t3dCurCameraIndex = ForcedCamera - 1;

	LastCamera = t3dCurCamera;
	DestCamera = PickCamera(t3dCurRoom, t3dCurCameraIndex);
	AnimCamera = *DestCamera;
	t3dCurCamera = &AnimCamera;

	game._renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);

	bCameraCarrello = FALSE;
	bCutCamera = FALSE;
	bAllowCalcCamera = FALSE;
	bMovingCamera = 2;

	NextCameraStep(game);
}

/* -----------------09/11/98 10.32-------------------
 *                  ClipGolfCameraMove
 * --------------------------------------------------*/
uint8 CameraMan::ClipGolfCameraMove(t3dV3F *NewT, t3dV3F *OldT, t3dV3F *Source) {
	t3dV3F n, o;
	t3dF32 a, d, l;

	t3dVectSub(&n, NewT, Source);
	t3dVectSub(&o, OldT, Source);
	t3dVectNormalize(&n);
	t3dVectNormalize(&o);

	a = t3dVectAngle(&o, &n);

	if (a > MAX_CAMERA_ANGLE / 3) d = (MAX_CAMERA_ANGLE / 3 * T3D_PI) / 180.0f;
	else if (a < -MAX_CAMERA_ANGLE / 3) d = -(MAX_CAMERA_ANGLE / 3 * T3D_PI) / 180.0f;
	else return 0;

	a = (a * T3D_PI) / 180.0f;
	t3dVectSub(&o, NewT, OldT);
	t3dVectNormalize(&o);
	l = (t3dVectDistance(NewT, OldT) * d) / a;
	o *= l;
	t3dVectAdd(NewT, OldT, &o);

	/*  t3dVectSub( &o, OldT, Source );
	    t3dVectNormalize( &o );
	    l = t3dVectDistance( OldT, Source );
	//  l += ( ( l - t3dVectDistance( NewT, Source ) ) * d ) / a;
	    t3dVectScale( &o, &o, l );

	    t3dMatRot( &m, 0.0f, d, 0.0f );
	    t3dVectTransform( NewT, &o, &m );
	    t3dVectAdd( NewT, NewT, Source );

	    NewT->y += ( ( NewT->y - OldT->y ) * d ) / a;
	*/
	return 1;
}

} // End of namespace Watchmaker
