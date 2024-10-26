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

#include "watchmaker/classes/do_keyboard.h"
#include "common/keyboard.h"
#include "watchmaker/3d/animation.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/t3d_mesh.h"
#include "watchmaker/classes/do_camera.h"
#include "watchmaker/classes/do_dialog.h"
#include "watchmaker/classes/do_player.h"
#include "watchmaker/define.h"
#include "watchmaker/game.h"
#include "watchmaker/globvar.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/ll/ll_mesh.h"
#include "watchmaker/ll/ll_mouse.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/main.h"
#include "watchmaker/schedule.h"
#include "watchmaker/t2d/t2d.h"
#include "watchmaker/utils.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/walk/walk.h"
#include "watchmaker/walk/walkutil.h"

namespace Watchmaker {

t3dF32 AngleX, AngleY, AngleSpeed;
char bFastAnim = 0;
int32 PlayAnim = 351;
char bBilinear = 1;
char bForceDebug = 0;
unsigned char KeyTable[Common::KEYCODE_LAST];

uint16 bnd_lev;

extern int16 NextDlg;   //from doDialog.c
extern uint8 tasti_per_sfx1;    //from main.c

void t3dLoadOutdoorLights(const char *pname, t3dBODY *b, int32 ora);

void ProcessKBInput() {
	// TODO: Currently we're polling this in the PollEvent flow.
	return;
#if 0
	int numKeys = 0;
	SDL_PumpEvents();
	auto keyState = SDL_GetKeyboardState(&numKeys);
	for (int i = 0; i < numKeys; i++) {
		KeyTable[i] = keyState[i];
	}

	for (int i = 0; i < numKeys; i++) {
		if (keyState[i])
			KeyTable[i] = 0x80;
		else if ((KeyTable[i] != 0) && (!keyState[i]))
			KeyTable[i] = 0x10;
		else
			KeyTable[i] = 0x00;
	}
#endif
}

bool KeyDown(Common::KeyCode key) {
	if (KeyTable[key] & 0x80)
		return TRUE;
	else
		return FALSE;
}

bool KeyUp(Common::KeyCode key) {
	if (KeyTable[key] & 0x10) {
		KeyTable[key] = 0;
		return TRUE;
	} else
		return FALSE;
}

void KeyClear(Common::KeyCode key) {
	KeyTable[key] = 0;
}

bool DInputExclusiveMouse() {
#if 0
	HWND hwnd = GetForegroundWindow();

	g_pMouse->lpVtbl->Unacquire(g_pMouse);
	if (FAILED(g_pMouse->lpVtbl->SetCooperativeLevel(g_pMouse, hwnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND)))
		return FALSE;
	g_pMouse->lpVtbl->Acquire(g_pMouse);
#endif
	LoaderFlags &= ~T3D_NONEXCLUSIVEMOUSE;

	return TRUE;
}

bool DInputNonExclusiveMouse() {
#if 0
	HWND hwnd = GetForegroundWindow();

	g_pMouse->lpVtbl->Unacquire(g_pMouse);
	if (FAILED(g_pMouse->lpVtbl->SetCooperativeLevel(g_pMouse, hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
		return FALSE;
	g_pMouse->lpVtbl->Acquire(g_pMouse);
#endif
	LoaderFlags |= T3D_NONEXCLUSIVEMOUSE;

	return TRUE;
}

/* -----------------25/08/98 10.42-------------------
 *                  HandleFirstPerson
 * --------------------------------------------------*/
void HandleFirstPersonView(WGame &game) {
	t3dF32 dist;
	t3dV3F d, n;

	if ((!Player) || (!t3dCurCamera) || (bLockCamera)) return;

	if (KeyDown(Common::KEYCODE_a)) {        // Alza testa
		if ((dist = CurFloorY + MAX_HEIGHT - (t3dCurCamera->Source.y + 10 * SCALEFACTOR)) > 0) {
			if (dist > 10 * SCALEFACTOR) dist = 10 * SCALEFACTOR;
			t3dVectInit(&d, 0.0f, dist, 0.0f);
			t3dMoveAndCheck1stCamera(t3dCurRoom, t3dCurCamera, &d);
		}
	} else if (KeyDown(Common::KEYCODE_z)) {     // Abbassa Testa
		if ((dist = CurFloorY + KNEE_HEIGHT - (t3dCurCamera->Source.y - 10 * SCALEFACTOR)) < 0) {
			if (dist < -10 * SCALEFACTOR) dist = -10 * SCALEFACTOR;
			t3dVectInit(&d, 0.0f, dist, 0.0f);
			t3dMoveAndCheck1stCamera(t3dCurRoom, t3dCurCamera, &d);
		}
	}

//	Se tengo premuto lo shift o un tasto del mouse
	if (KeyDown(Common::KEYCODE_LSHIFT) || KeyDown(Common::KEYCODE_RSHIFT) || ((bLPressed || bRPressed) && (mMove > 10))) {
		t3dVectSub(&d, &t3dCurCamera->Target, &t3dCurCamera->Source);
		d.y = 0.0f;
		t3dVectNormalize(&d);
		n.x = -d.z;
		n.y = 0.0f;
		n.z = d.x;

		dist = (t3dF32)((t3dF32)mMoveY / (t3dF32)(MainDy / 2)) * 100.0f;
		if (KeyDown(Common::KEYCODE_UP))
			d *= (5 * SCALEFACTOR);
		else if (KeyDown(Common::KEYCODE_DOWN))
			d *= (-5 * SCALEFACTOR);
		else if (((bLPressed) || (bRPressed)) && (mMoveY) && !bClock33)
			d *= (-dist * SCALEFACTOR);
		else
			t3dVectFill(&d, 0.0f);

		dist = (t3dF32)((t3dF32)mMoveX / (t3dF32)(MainDx / 2)) * 100.0f;
		if (KeyDown(Common::KEYCODE_LEFT))
			n *= (5 * SCALEFACTOR);
		else if (KeyDown(Common::KEYCODE_RIGHT))
			n *= (-5 * SCALEFACTOR);
		else if (((bLPressed) || (bRPressed)) && (mMoveX) && !bClock33)
			n *= (-dist * SCALEFACTOR);
		else
			t3dVectFill(&n, 0.0f);

		t3dVectAdd(&d, &d, &n);
		t3dMoveAndCheck1stCamera(t3dCurRoom, t3dCurCamera, &d);
	} else {
		int32 x, y;
		x = 0;
		y = 0;

		if (KeyDown(Common::KEYCODE_UP))
			y = -10;
		else if (KeyDown(Common::KEYCODE_DOWN))
			y = MainDy + 10;
		if (KeyDown(Common::KEYCODE_LEFT))
			x = -10;
		else if (KeyDown(Common::KEYCODE_RIGHT))
			x = MainDx + 10;

		if (x || y) {
			t3dF32 diffx, diffy;
			diffx = 0.f;
			diffy = 0.f;

			if (x > MainDx) diffx = (t3dF32)((t3dF32)(x - MainDx) / 3.0f);
			else if (x < 0) diffx = (t3dF32)((t3dF32)x / 3.0f);
			if (y > MainDy) diffy = (t3dF32)((t3dF32)(y - MainDy) / 3.0f);
			else if (y < 0) diffy = (t3dF32)((t3dF32)y / 3.0f);

			game._cameraMan->MoveHeadAngles(diffx, diffy);
		}
	}

//	Corregge Camera
	t3dVectCopy(&d, &Player->Mesh->Trasl);
	d.y = t3dCurCamera->Source.y;
	dist = KNEE_HEIGHT - t3dVectDistance(&t3dCurCamera->Source, &d);
	if (dist < 0.0f) {
		t3dVectSub(&d, &t3dCurCamera->Source, &d);
		d.y = 0.0f;
		t3dVectNormalize(&d);
		d *= dist;

		t3dVectAdd(&t3dCurCamera->Source, &t3dCurCamera->Source, &d);
		t3dVectAdd(&t3dCurCamera->Target, &t3dCurCamera->Target, &d);
	}
}


/* -----------------23/04/98 17.24-------------------
 *                  ProcessKeyboard
 * --------------------------------------------------*/

void ProcessKeyboard(WGame &game) {
	//warning("STUBBED: ProcessKeyboard");

	t3dF32 TurnSpeed, Speed = 1.0f, dist;
	t3dV3F cp, cd, ct;
	int32 a, b;

	AngleX = AngleY = AngleSpeed = 0.0f;
	TurnSpeed = 3.5f * FrameFactor;

	doT2DKeyboard(game);

	ProcessKBInput();

	if (bIngnoreDIKeyboard)
		return ;

	if (KeyUp(Common::KEYCODE_ESCAPE)) {
		if (LoaderFlags & T3D_DEBUGMODE) {
			CloseSys(game); // Quitta il gioco
		} else {
			// Skip Intro
			if ((CurDialog == dR000) || (CurDialog == dR111) || (CurDialog == dR211)) {
				NextDlg = dNULL;
				_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGEND_SKIPINTRO, MP_DEFAULT, (int16)CurDialog, mQUIT, 0, NULL, NULL, NULL);
			}

			// Skip Loghi
			if (CurDialog == dRLOGHI)
				_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGEND, MP_DEFAULT, (int16)CurDialog, mQUIT, 0, NULL, NULL, NULL);

			// Skip MorteWM
			if (CurDialog == dR391)
				_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGEND, MP_DEFAULT, (int16)CurDialog, mQUIT, 0, NULL, NULL, NULL);

			// Skip durante i crediti, quitta
			if (bTitoliCodaScrolling || bTitoliCodaStatic)
				game.CleanUpAndPostQuit();
		}
	}// Common::KEYCODE_ESCAPE

	//se ci sono i crediti ritorna (controlla solo l'ESC)
	if (bTitoliCodaStatic || bTitoliCodaScrolling)  return;

	if (KeyDown(Common::KEYCODE_LSHIFT) && KeyUp(Common::KEYCODE_d))
		bForceDebug ^= 1;

	/*  if( KeyDown(SDL_SCANCODE_LSHIFT) )           // Bomba il gioco
	        if( KeyUp(SDL_SCANCODE_F) )
	        {
	            t3dFree(t3dCurRoom->CameraTable);
	            t3dFree(t3dCurRoom->CameraTable);
	        }
	*/
	if (KeyUp(Common::KEYCODE_i))                    // Escono Informazioni
		bShowInfo ^= 1;

	if (KeyUp(Common::KEYCODE_g)) {
		error("TODO: Screenshot support");
#if 0
		FILE *fh;
		char str[32];
		int i;
		for (i = 1; i < 1000; i++) {
			sprintf(str, "Wm%#04d.tga", i);
			if ((fh = fopen(str, "rb")) == NULL)
				break;
			else
				fclose(fh);
		}
		rGrabVideo(str, 0);
#endif
	}

	if (KeyUp(Common::KEYCODE_END)) {
		if (!bFirstPerson)
			StartAnim(game, aGIRO);
	}

	if ((LoaderFlags & T3D_DEBUGMODE) || bForceDebug) {
		if (KeyUp(Common::KEYCODE_F5))
			DataSave("Prova Save", 0);
		if (KeyUp(Common::KEYCODE_F6))
			DataLoad(game, "", 0);

		if (KeyUp(Common::KEYCODE_w)) {                  // Modalita' wireframe
			bForceWire ^= 1;
			if (bForceWire)
				rSetRenderMode(rWIREFRAMEMODE);
			else
				rSetRenderMode(rSOLIDMODE);
		}

		if (KeyUp(Common::KEYCODE_b))                    // Escono BoundingBox
			bShowBoundingBox ^= 1;

		if (KeyUp(Common::KEYCODE_BACKSPACE))                 // Fa andare le animazioni piu' veloci
			bFastAnim ^= 1;

		if (KeyUp(Common::KEYCODE_s))
			bSkipTalk = TRUE;

		if (KeyUp(Common::KEYCODE_h))
			_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dPROVA, 0, 0, NULL, NULL, NULL);

		if (KeyUp(Common::KEYCODE_e))
			StartAnim(game, aFOX);

		if (KeyUp(Common::KEYCODE_j)) {
			if (Player && Player->Mesh) {
				ct = Player->Dir * HALF_STEP * 5.0f;
				t3dVectAdd(&Player->Mesh->Trasl, &Player->Mesh->Trasl, &ct);
				t3dVectCopy(&Player->Pos, &Player->Mesh->Trasl);
			}
		}

		if (KeyUp(Common::KEYCODE_p)) {
			if (KeyDown(Common::KEYCODE_LSHIFT) || KeyDown(Common::KEYCODE_RSHIFT)) {
				if (++bnd_lev > 5) bnd_lev = 0;
				SetBndLevel(game, NULL, bnd_lev);
				DebugLogWindow("BndLev %d", bnd_lev);
			} else
				bShowPanels ^= 1;   // Escono Pannelli

		}

		if (!tasti_per_sfx1) {
			if (KeyUp(Common::KEYCODE_m)) {
				if (KeyDown(Common::KEYCODE_LSHIFT))
					DInputNonExclusiveMouse();
				else if (KeyDown(Common::KEYCODE_RSHIFT))
					DInputExclusiveMouse();

			}

			if (KeyUp(Common::KEYCODE_o))
				bShowExtraLocalizationStrings ^= 1;
		} else {
			if (KeyUp(Common::KEYCODE_k)) {
				PlayAnim --;
				if (KeyDown(Common::KEYCODE_LCTRL) || KeyDown(Common::KEYCODE_RCTRL))
					PlayAnim -= 19;
				if (KeyDown(Common::KEYCODE_LSHIFT) || KeyDown(Common::KEYCODE_RSHIFT))
					PlayAnim -= 30;
				warning("PlayAnim %d '%s'\n", PlayAnim, game.init.Anim[PlayAnim].name[0].rawArray()); // TODO DebugString
			}
			if (KeyUp(Common::KEYCODE_l)) {
				PlayAnim ++;
				if (KeyDown(Common::KEYCODE_LCTRL) || KeyDown(Common::KEYCODE_RCTRL))
					PlayAnim += 19;
				if (KeyDown(Common::KEYCODE_LSHIFT) || KeyDown(Common::KEYCODE_RSHIFT))
					PlayAnim += 30;
				warning("PlayAnim %d '%s'\n", PlayAnim, game.init.Anim[PlayAnim].name[0].rawArray()); // TODO DebugString
			}
			if (KeyUp(Common::KEYCODE_m))
				StartAnim(game, PlayAnim);
		}


		if (KeyDown(Common::KEYCODE_LSHIFT)) {
			if (KeyUp(Common::KEYCODE_F1)) t3dLoadOutdoorLights("c:\\wm\\LMaps\\rxt.t3d", t3dRxt, 1030);
			if (KeyUp(Common::KEYCODE_F2)) t3dLoadOutdoorLights("c:\\wm\\LMaps\\rxt.t3d", t3dRxt, 1530);
			if (KeyUp(Common::KEYCODE_F3)) t3dLoadOutdoorLights("c:\\wm\\LMaps\\rxt.t3d", t3dRxt, 1930);
			if (KeyUp(Common::KEYCODE_F4)) t3dLoadOutdoorLights("c:\\wm\\LMaps\\rxt.t3d", t3dRxt, 2230);
		}

		if (KeyUp(Common::KEYCODE_F11)) IncCurTime(game, 5);
		if (KeyUp(Common::KEYCODE_F12)) IncCurTime(game, 100);

		if (KeyUp(Common::KEYCODE_1)) CharSetPosition(ocCURPLAYER, 1, NULL);
		if (KeyUp(Common::KEYCODE_2)) CharSetPosition(ocCURPLAYER, 2, NULL);
		if (KeyUp(Common::KEYCODE_3)) CharSetPosition(ocCURPLAYER, 3, NULL);
		if (KeyUp(Common::KEYCODE_4)) CharSetPosition(ocCURPLAYER, 4, NULL);
		if (KeyUp(Common::KEYCODE_5)) CharSetPosition(ocCURPLAYER, 5, NULL);
		if (KeyUp(Common::KEYCODE_6)) CharSetPosition(ocCURPLAYER, 6, NULL);
		if (KeyUp(Common::KEYCODE_7)) CharSetPosition(ocCURPLAYER, 7, NULL);
		if (KeyUp(Common::KEYCODE_8)) CharSetPosition(ocCURPLAYER, 8, NULL);
		if (KeyUp(Common::KEYCODE_9)) CharSetPosition(ocCURPLAYER, 9, NULL);
		if (KeyUp(Common::KEYCODE_KP0)) CharSetPosition(ocCURPLAYER, 10, NULL);
		if (KeyUp(Common::KEYCODE_KP1)) CharSetPosition(ocCURPLAYER, 11, NULL);
		if (KeyUp(Common::KEYCODE_KP2)) CharSetPosition(ocCURPLAYER, 12, NULL);
		if (KeyUp(Common::KEYCODE_KP3)) CharSetPosition(ocCURPLAYER, 13, NULL);
		if (KeyUp(Common::KEYCODE_KP4)) CharSetPosition(ocCURPLAYER, 14, NULL);
		if (KeyUp(Common::KEYCODE_KP5)) CharSetPosition(ocCURPLAYER, 15, NULL);
		if (KeyUp(Common::KEYCODE_KP6)) CharSetPosition(ocCURPLAYER, 16, NULL);
		if (KeyUp(Common::KEYCODE_KP7)) CharSetPosition(ocCURPLAYER, 17, NULL);
		if (KeyUp(Common::KEYCODE_KP8)) CharSetPosition(ocCURPLAYER, 18, NULL);
		if (KeyUp(Common::KEYCODE_KP9)) CharSetPosition(ocCURPLAYER, 19, NULL);

		if (KeyDown(Common::KEYCODE_LSHIFT))
			if (KeyUp(Common::KEYCODE_x)) {
				tasti_per_sfx1 ^= 1;
			}

		/*
		        if( KeyUp(SDL_SCANCODE_D) )
		            Event( EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, 0, 0, tDIARIO, NULL, NULL, NULL );

		        if( KeyUp(SDL_SCANCODE_K) )
		        {
		            PlayAnim --;
		            if( KeyDown(SDL_SCANCODE_LCONTROL) || KeyDown(SDL_SCANCODE_RCONTROL) )
		                PlayAnim -= 19;
		            if( KeyDown(SDL_SCANCODE_LSHIFT) || KeyDown(SDL_SCANCODE_RSHIFT) )
		                PlayAnim -= 30;
		            DebugString("PlayAnim %d '%s'",PlayAnim,Anim[PlayAnim].name[0]);
		        }
		        if( KeyUp(SDL_SCANCODE_L) )
		        {
		            PlayAnim ++;
		            if( KeyDown(SDL_SCANCODE_LCONTROL) || KeyDown(SDL_SCANCODE_RCONTROL) )
		                PlayAnim += 19;
		            if( KeyDown(SDL_SCANCODE_LSHIFT) || KeyDown(SDL_SCANCODE_RSHIFT) )
		                PlayAnim += 30;
		            DebugString("PlayAnim %d '%s'",PlayAnim,Anim[PlayAnim].name[0]);
		        }
		        if( KeyUp(SDL_SCANCODE_M) )
		            StartAnim( PlayAnim );


		        if ( KeyUp(SDL_SCANCODE_X) )
		            rSetBilinearFilterState( bBilinear ^= 1 );
		        if( KeyUp(SDL_SCANCODE_T) )                  // Scrive su file nome mesh
		            DebugFile("%s",ObjectUnderCursor);
		        if( KeyUp(SDL_SCANCODE_BACK) )               // Attiva modalita' frame-by-frame per le animazioni
		            bPauseAllAnims ^= 1;
		        if( KeyUp(SDL_SCANCODE_RETURN) )             // Avanza di un frame tutte le animazioni
		            ContinueAnim( -1 );
		        if( KeyUp(SDL_SCANCODE_S) )
		            Event( EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR1a1, 0, 0, NULL, NULL, NULL );
		        if( KeyUp(SDL_SCANCODE_N) )
		            Event( EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, 0, 0, tPDA, NULL, NULL, NULL );
		        if( KeyUp(SDL_SCANCODE_F) )
		            Event( EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, 0, 0, tCOMPUTER, NULL, NULL, NULL );
		        if( ( KeyDown(SDL_SCANCODE_LSHIFT) || KeyDown(SDL_SCANCODE_RSHIFT) ) && ( KeyDown(SDL_SCANCODE_F5) || KeyDown(SDL_SCANCODE_F6) ) )
		            Event( EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, 0, 0, tOPTIONS, NULL, NULL, NULL );

		        if( KeyUp(SDL_SCANCODE_C) )
		        {
		            int i,len;
		            char Name[200];
		            if ( t3dCurRoom->CameraGrid.Grid)
		                t3dFree(t3dCurRoom->CameraGrid.Grid);
		            t3dCurRoom->CameraGrid.Grid=NULL;

		            for ( i=0; i<t3dCurRoom->NumPaths; i++)
		            {
		                if ( t3dCurRoom->CameraPath[i].PList)
		                {
		                    t3dFree(t3dCurRoom->CameraPath[i].PList);
		                    t3dCurRoom->CameraPath[i].PList=NULL;
		                }
		            }
		            t3dFree(t3dCurRoom->CameraPath);
		            t3dCurRoom->CameraPath=NULL;

		            for ( i=0; i<t3dCurRoom->NumCameras; i++)
		            {
		                if ( t3dCurRoom->CameraTable[i].CameraPaths)
		                {
		                    t3dFree(t3dCurRoom->CameraTable[i].CameraPaths);
		                    t3dCurRoom->CameraTable[i].CameraPaths=NULL;
		                }
		            }
		            t3dFree(t3dCurRoom->CameraTable);
		            t3dCurRoom->CameraTable=NULL;

		            strcpy(Name,WmCamDir);
		            strcat(Name,t3dCurRoom->Name);
		            len=strlen(Name);
		            Name[len-3]='c';
		            Name[len-2]='a';
		            Name[len-1]='m';
		            LoadCameras(Name,t3dCurRoom);
		            t3dCurCamera = &t3dCurRoom->CameraTable[0];
		            BigInvObj ++;
		    //      DebugLogFile("%d: %s %s",BigInvObj,ObjName[InvObj[BigInvObj].name],InvObj[BigInvObj].meshlink);
		        }

		        if( KeyUp(SDL_SCANCODE_R) )
		            rSetFogMode( RGBA_MAKE( 70, 0, 0, 0), 4000.0f, 8000.0f, 0.1f, D3DFOG_LINEAR );
		*/
	}// fine tasti di debug
	else {
		if (KeyUp(Common::KEYCODE_d))
			bShowRoomDescriptions ^= 1;

		if (KeyUp(Common::KEYCODE_e))
			bShowExtraLocalizationStrings ^= 1;

		if (KeyUp(Common::KEYCODE_p))
			_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, 0, 0, tPDA, NULL, NULL, NULL);
	}

	if (KeyUp(Common::KEYCODE_F8) && PlayerCanSwitch(game._gameVars, 1) && !(InvStatus & INV_ON))
//		&& (  (InvStatus & (INV_ON|INV_MODE2)) != (INV_ON|INV_MODE2)  ) )
	{
		KeyClear(Common::KEYCODE_F8);
		if (CurPlayer == DARRELL) a = ocVICTORIA;
		else a = ocDARRELL;
		if (a == (ocDARRELL + CurPlayer)) return ;
		_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND / 3, 0, EFFECT_FADOUT, NULL, NULL, NULL);
		if (bMovingCamera)
			_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_CHANGEPLAYER, MP_WAITA | MP_WAIT_CAMERA, (int16)a, 0, 0, NULL, NULL, NULL);
		else
			_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_CHANGEPLAYER, MP_WAITA, (int16)a, 0, 0, NULL, NULL, NULL);
	}

	if ((bPlayerInAnim) || (bNotSkippableWalk) || (bDialogActive)) {
		KeyClear(Common::KEYCODE_F1);
		KeyClear(Common::KEYCODE_F2);
		KeyClear(Common::KEYCODE_F3);
		KeyClear(Common::KEYCODE_TAB);
		KeyClear(Common::KEYCODE_SPACE);
		KeyClear(Common::KEYCODE_LCTRL);
		KeyClear(Common::KEYCODE_RCTRL);
		KeyClear(Common::KEYCODE_END);
		return;
	}

	if (KeyUp(Common::KEYCODE_F1) && !(InvStatus & INV_ON) && (bT2DActive == tNULL) && PlayerCanSave()) {
		rGrabVideo("temp.tmp", 1);
		_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, MPX_START_T2D_SAVE, 0, tOPTIONS, NULL, NULL, NULL);
	}
	if (KeyUp(Common::KEYCODE_F2) && !(InvStatus & INV_ON) && (bT2DActive == tNULL)) {
		rGrabVideo("temp.tmp", 1);
		_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, MPX_START_T2D_LOAD, 0, tOPTIONS, NULL, NULL, NULL);
	}
	if (KeyUp(Common::KEYCODE_F3) && !(InvStatus & INV_ON) && (bT2DActive == tNULL)) {
		rGrabVideo("temp.tmp", 1);
		_vm->_messageSystem.doEvent(EventClass::MC_T2D, ME_T2DSTART, MP_DEFAULT, MPX_START_T2D_OPTIONS, 0, tOPTIONS, NULL, NULL, NULL);
	}


//	Se Premo Control e sono vicino ad una porta o ad una scala
	if ((Player) && ((KeyUp(Common::KEYCODE_LCTRL)) || (KeyUp(Common::KEYCODE_RCTRL)))) {
		KeyClear(Common::KEYCODE_LCTRL);
		KeyClear(Common::KEYCODE_RCTRL);
		if (bSomeOneSpeak) bSkipTalk = TRUE;

		game._cameraMan->GetRealCharPos(game.init, &ct, ocCURPLAYER, 0);
		ct.y = CurFloorY;
		for (a = 0; a < MAX_OBJS_IN_ROOM; a++) {
			b = game.getCurRoom().objects[a];
			if (((game.init.Obj[b].flags & DOOR) || (bPorteEsternoBloccate && (game.init.Obj[b].flags & DONE))) && (game.init.Obj[b].flags & ON) && !(game.init.Obj[b].flags & HIDE) && (game.init.Obj[b].pos) && (GetLightPosition(&cp, game.init.Obj[b].pos))) {
				GetLightDirection(&cd, game.init.Obj[b].pos);
				dist = t3dVectDistance(&ct, &cp);
//				Se sono abbastanza vicino a luce posizione
				if (dist < (CHEST_HEIGHT)) {
					t3dVectSub(&cd, &cd, &cp);
					dist = t3dVectAngle(&cd, &Player->Dir);
					if ((dist < 60.0f) && (dist > -60.0f)) {
						CurObj = b;
						_vm->_messageSystem.doEvent(EventClass::MC_MOUSE, ME_MRIGHT, MP_DEFAULT, 0, 0, 0, &CurObj, NULL, NULL);
					}
				}
			}
		}
	}

	if (KeyUp(Common::KEYCODE_SPACE) && (!IsPlayerInPool())) {               // Cambia tra 3a e 1a persona
		if (bSomeOneSpeak) bSkipTalk = TRUE;

		if ((bFirstPerson == 0) && (!bMovingCamera) && (!bNoFirstPersonSwitch)/* && !( InvStatus & INV_ON )*/)
			_vm->_messageSystem.doEvent(EventClass::MC_CAMERA, ME_CAMERA3TO1, MP_DEFAULT, 0, 0, 0, NULL, NULL, NULL);
		else if ((!bMovingCamera) && (!bNoFirstPersonSwitch)/* && !( InvStatus & INV_ON )*/)
			_vm->_messageSystem.doEvent(EventClass::MC_CAMERA, ME_CAMERA1TO3, MP_DEFAULT, 0, 0, 0, NULL, NULL, NULL);
	}

	if (KeyUp(Common::KEYCODE_TAB) && !bLockCamera)              // Fa uscire l'inventario
		_vm->_messageSystem.doEvent(EventClass::MC_INVENTORY, ME_INVSWITCH, MP_DEFAULT, 0, 0, 0, NULL, NULL, NULL);

	if (KeyDown(Common::KEYCODE_LEFT) && !(InvStatus & INV_ON) && !(bFirstPerson)) {      // Ruota a Destra
		AngleY = (-1) * TurnSpeed / 180.0f * T3D_PI;
		if ((Player->Walk.CurAction <= aSTAND) || (Player->Walk.CurAction == aROT_DX)) {
			PlayerGotoPos[CurPlayer + ocDARRELL] = 0;
			PlayerPos[CurPlayer + ocDARRELL] = 0;
			Player->Walk.CurAction = aROT_SX;
			Player->Walk.CurFrame = ActionStart[Player->Walk.CurAction];
			Player->Mesh->CurFrame = Player->Walk.CurFrame;
		} else if (Player->Walk.CurAction == aROT_SX) {
			PlayerGotoPos[CurPlayer + ocDARRELL] = 0;
			PlayerPos[CurPlayer + ocDARRELL] = 0;
			Player->Walk.CurFrame ++;
			if (Player->Walk.CurFrame >= ActionStart[Player->Walk.CurAction + 1] - 1)
				Player->Walk.CurFrame = ActionStart[Player->Walk.CurAction];
			Player->Mesh->CurFrame = Player->Walk.CurFrame;
		}
	} else if (KeyDown(Common::KEYCODE_RIGHT) && !(InvStatus & INV_ON) && !(bFirstPerson)) { // Ruota a Sinistra
		AngleY = TurnSpeed / 180.0f * T3D_PI;
		if ((Player->Walk.CurAction <= aSTAND) || (Player->Walk.CurAction == aROT_SX)) {
			PlayerGotoPos[CurPlayer + ocDARRELL] = 0;
			PlayerPos[CurPlayer + ocDARRELL] = 0;
			Player->Walk.CurAction = aROT_DX;
			Player->Walk.CurFrame = ActionStart[Player->Walk.CurAction];
			Player->Mesh->CurFrame = Player->Walk.CurFrame;
		} else if (Player->Walk.CurAction == aROT_DX) {
			PlayerGotoPos[CurPlayer + ocDARRELL] = 0;
			PlayerPos[CurPlayer + ocDARRELL] = 0;
			Player->Walk.CurFrame ++;
			if (Player->Walk.CurFrame >= ActionStart[Player->Walk.CurAction + 1] - 1)
				Player->Walk.CurFrame = ActionStart[Player->Walk.CurAction];
			Player->Mesh->CurFrame = Player->Walk.CurFrame;
		}
	}

	if (KeyDown(Common::KEYCODE_UP) && !(InvStatus & INV_ON) && !(bFirstPerson)) {
		AngleSpeed = 20.0f;
	} else if (KeyDown(Common::KEYCODE_DOWN) && !(InvStatus & INV_ON) && !(bFirstPerson)) {
		AngleSpeed = -20.0f;
	}

	if (KeyDown(Common::KEYCODE_LSHIFT) || KeyDown(Common::KEYCODE_RSHIFT)) // || (GetKeyState(Common::KEYCODE_CAPSLOCK) & 0x1)) TODO: Allow for Caps-lock for fast walk
		bFastWalk = TRUE;
	else
		bFastWalk = FALSE;

	if (!t3dCurCamera) return;

	if ((bFirstPerson) && !(bMovingCamera))
		HandleFirstPersonView(game);

	if (bFirstPerson) {
		_vm->_messageSystem.doEvent(EventClass::MC_MOUSE, ME_MOUSEUPDATE, MP_DEFAULT, 0, 0, 0, NULL, NULL, NULL);
		AngleX = AngleY = AngleSpeed = 0.0f;
		if (Player)
			Player->Flags |= T3D_CHARACTER_HIDE;
	} else if (!(InvStatus & INV_ON) && (Player)) {
		UpdateChar(game, ocCURPLAYER, AngleSpeed * Speed, AngleY);
		AngleX = AngleY = AngleSpeed = 0.0f;
	}
}

/* -----------------28/09/98 17.18-------------------
 *                  doClock32
 * --------------------------------------------------*/
void doClock33(WGame &game, int32 obj, t3dV3F *mp) {
	t3dMESH *l;
	t3dV3F tmp, pos;
	int32 i, r;

	if (!bClock33) {
		bClock33 = (uint8)(obj - o33LANCETTAHSX + 1);
		if (game._gameVars.getCurRoomId() == r33)
			CharSetPosition(ocCURPLAYER, game.init.Obj[o33OROLOGIO].pos, nullptr);
	}

	obj = (int32)bClock33 + o33LANCETTAHSX - 1;

//	Trova il puntatore alla mesh
	if (!(l = LinkMeshToStr(game.init, (char *)game.init.Obj[obj].meshlink[0]))) return;
//	Trova il punto centrale attorno a cui ruotare
	pos.x = l->Pos.x;
	pos.y = 350.0f;
	pos.z = l->Pos.z;
//	Trova direzione della lancetta in base al mouse
	t3dVectSub(&tmp, mp, &pos);
	tmp.z = 0.0f;
	r = (int32)(SinCosAngle(-tmp.x, tmp.y) * 180.0f / T3D_PI) / 30;
	Comb33[bClock33 - 1] = (int32)r;
//	Azzera Bounding box (tornano in posizione centrale)
	for (i = 0; i < 8; i++) {
		t3dVectSub(&tmp, &l->BBox[i].p, &pos);
		t3dVectTransformInv(&l->BBox[i].p, &tmp, &Lanc33[obj - o33LANCETTAHSX]->Mesh->Matrix);
	}

	t3dMatRot(&Lanc33[obj - o33LANCETTAHSX]->Mesh->Matrix, 0.0f, 0.0f, T3D_PI * (t3dF32)r / 6.0f);

	for (i = 0; i < 8; i++) {
		t3dVectTransform(&tmp, &l->BBox[i].p, &Lanc33[obj - o33LANCETTAHSX]->Mesh->Matrix);
		t3dVectAdd(&l->BBox[i].p, &tmp, &pos);
	}

	t3dPlaneNormal(&l->BBoxNormal[0], &l->BBox[0].p, &l->BBox[2].p, &l->BBox[1].p);      //front
	t3dPlaneNormal(&l->BBoxNormal[1], &l->BBox[4].p, &l->BBox[5].p, &l->BBox[6].p);      //back
	t3dPlaneNormal(&l->BBoxNormal[2], &l->BBox[4].p, &l->BBox[0].p, &l->BBox[5].p);      //Up
	t3dPlaneNormal(&l->BBoxNormal[3], &l->BBox[6].p, &l->BBox[7].p, &l->BBox[2].p);      //Down
	t3dPlaneNormal(&l->BBoxNormal[4], &l->BBox[4].p, &l->BBox[6].p, &l->BBox[0].p);      //Left
	t3dPlaneNormal(&l->BBoxNormal[5], &l->BBox[5].p, &l->BBox[1].p, &l->BBox[7].p);      //Right
}

} // End of namespace Watchmaker
