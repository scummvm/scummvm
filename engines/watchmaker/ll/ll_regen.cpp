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

#include "watchmaker/ll/ll_regen.h"
#include "watchmaker/classes/do_camera.h"
#include "watchmaker/render.h"
#include "watchmaker/struct.h"
#include "watchmaker/sysdef.h"
#include "watchmaker/utils.h"
#include "watchmaker/t2d/t2d.h"
#include "watchmaker/extraLS.h"
#include "watchmaker/ll/ll_mouse.h"
#include "watchmaker/define.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/ll/ll_string.h"
#include "watchmaker/renderer.h"

// 2d-regen
#define MAX_PAINT_RECTS     MAX_DD_BITMAPS+MAX_REND_TEXTS
#define MAX_UPDATE_RECTS    MAX_DD_BITMAPS+MAX_REND_TEXTS

namespace Watchmaker {

struct SDDBitmap PaintRect[MAX_PAINT_RECTS];
struct SDDBitmap OldPaintRect[MAX_PAINT_RECTS];

// frame-rate
t3dF32 hi, lo, ofps;

#define WM_CUR_VERSION  "The Watchmaker v0.92"

/* -----------------30/10/98 12.19-------------------
 *                  IntersecateRect
 * --------------------------------------------------*/
int32 IntersecateRect(int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, int32 x4, int32 y4, SRect *r) {
	if ((x1 <= x4) && (x2 >= x3) && (y1 <= y4) && (y2 >= y3)) {
		if (x3 > x1)
			r->x1 = 0;
		else
			r->x1 = x1 - x3;

		if (x2 < x4)
			r->x2 = x2 - x3;
		else
			r->x2 = x4 - x3;

		if (y3 > y1)
			r->y1 = 0;
		else
			r->y1 = y1 - y3;

		if (y2 < y4)
			r->y2 = y2 - y3;
		else
			r->y2 = y4 - y3;

		if (!(r->x2 - r->x1) || !(r->y2 - r->y1))
			return 0;

		return 1;
	} else
		return 0;
}
/* -----------------30/10/98 11.41-------------------
 *                  Regen
 * --------------------------------------------------*/
void Regen(WGame &game) {
	struct SRect UpdateRect[MAX_UPDATE_RECTS], *p;
	struct SDDBitmap *n, *o;
	struct SRect r, ext;
	int32 a, b, upn;
	uint8 found, refresh[MAX_PAINT_RECTS];

#ifdef DEBUG_REGEN
	DebugFile("----- Nuovo Frame -----");
#endif
	upn = 0;
	memset(refresh, 0, sizeof(refresh));
	for (uint32 i = 0; i < ARRAYSIZE(UpdateRect); i++) UpdateRect[i].reset();
	// I take the Extends of what the engine drew
	rGetExtends(&ext.x1, &ext.y1, &ext.x2, &ext.y2);
	// 1 - Compare each element of PaintRect to all OlPaintRect elements
	for (a = 0; a < MAX_PAINT_RECTS; a++) {
		n = &PaintRect[a];
		if (!(n->dx) || !(n->dy))
			continue;
#ifdef DEBUG_REGEN
		DebugFile("%d: Controllo rect %d '%s': %d,%d %d,%d", a, n->tnum, rGetBitmapName(n->tnum), n->px + n->ox, n->py + n->oy, n->dx, n->dy);
#endif
		found = 0;
		for (b = 0; b < MAX_PAINT_RECTS; b++) {
			o = &OldPaintRect[b];
			if (!(o->dx) || !(o->dy))
				continue;

			if ((n->tnum == o->tnum) && (n->px == o->px) && (n->py == o->py) && (n->ox == o->ox) && (n->oy == o->oy) && (n->dx == o->dx) && (n->dy == o->dy)) {
#ifdef DEBUG_REGEN
				DebugFile("-> Gia' disegnato");
#endif
				refresh[b] = 1;
				found ++;
			}
		}
		// if it's a new rectangle, compile UpdateRect
		if (true || !found) { // HACK: Just always treat everything as new, since we're doing GL
#ifdef DEBUG_REGEN
			DebugFile("-> Nuovo rettangolo");
#endif
			UpdateRect[upn].x1 = n->px + n->ox;
			UpdateRect[upn].y1 = n->py + n->oy;
			UpdateRect[upn].x2 = n->px + n->ox + n->dx;
			UpdateRect[upn].y2 = n->py + n->oy + n->dy;
			upn ++;
		}
	}
	// 2 - If there are any reactangles that need to be deleted, compile UpdateRect
	for (a = 0; a < MAX_PAINT_RECTS; a++) {
		n = &OldPaintRect[a];
		if (!(n->dx) || !(n->dy))
			continue;

		if ((!refresh[a]) && (n->dx) && (n->dy)) {
#ifdef DEBUG_REGEN
			DebugFile("Non viene piu' ridisegnato %d '%s': %d,%d %d,%d", n->tnum, rGetBitmapName(n->tnum), n->px + n->ox, n->py + n->oy, n->dx, n->dy);
#endif
			UpdateRect[upn].x1 = n->px + n->ox;
			UpdateRect[upn].y1 = n->py + n->oy;
			UpdateRect[upn].x2 = n->px + n->ox + n->dx;
			UpdateRect[upn].y2 = n->py + n->oy + n->dy;
			upn ++;
		}
	}

	// 3 - For all UpdateRects delete the ScreenBuffer and copy what's left
	for (a = 0; a < upn; a++) {
		p = &UpdateRect[a];
		if (!(p->x2 - p->x1) || !(p->y2 - p->y1))
			continue;

		// Clear ScreenBuffer
		game._renderer->clearBitmap(BACK_BUFFER, (p->x1), (p->y1), (p->x2 - p->x1), (p->y2 - p->y1), 0, 0, 0);
	}

	for (a = 0; a < upn; a++) {
		p = &UpdateRect[a];
		if (!(p->x2 - p->x1) || !(p->y2 - p->y1))
			continue;

#ifdef DEBUG_REGEN
		DebugFile("Ridisegno quello che sta sopra a %d,%d %d,%d", p->x1, p->y1, p->x2 - p->x1, p->y2 - p->y1);
#endif
		found = 0;
		for (b = 0; b < MAX_PAINT_RECTS; b++) {
			n = &PaintRect[b];
			if (!(n->dx) || !(n->dy))
				continue;

			// If it intersects, copies intersection only
			if ((IntersecateRect(p->x1, p->y1, p->x2, p->y2, n->px + n->ox, n->py + n->oy, n->px + n->ox + n->dx, n->py + n->oy + n->dy, &r)) &&
			        (n->ox + r.x2 - r.x1) && (n->oy + r.y2 - r.y1)) {
#ifdef DEBUG_REGEN
				DebugFile("Copio %d '%s': P %d,%d O %d,%d D %d,%d", n->tnum, rGetBitmapName(n->tnum), (n->px + n->ox + r.x1), (n->py + n->oy + r.y1), (n->ox + r.x1), (n->oy + r.y1), (r.x2 - r.x1), (r.y2 - r.y1));
//				DebugLogWindow( "Copio %d '%s': P %d,%d O %d,%d D %d,%d", n->tnum, rGetBitmapName(n->tnum), (n->px+n->ox+r.x1), (n->py+n->oy+r.y1), (n->ox+r.x1), (n->oy+r.y1), (r.x2-r.x1), (r.y2-r.y1) );
#endif
				rBlitter(game, BACK_BUFFER, n->tnum, (n->px + n->ox + r.x1), (n->py + n->oy + r.y1), (n->ox + r.x1), (n->oy + r.y1), (r.x2 - r.x1), (r.y2 - r.y1));
				found ++;
			}
		}

		if (!found) {
			if (p) p->reset();
		}
	}

	// 4 - Copy on screen only the UpdateRects by stretching them.
	for (a = 0; a < MAX_PAINT_RECTS; a++) {
		n = &PaintRect[a];
		if (!(n->dx) || !(n->dy))
			continue;

		if ((n->px + n->ox) < ext.x1) ext.x1 = n->px + n->ox;
		if ((n->py + n->oy) < ext.y1) ext.y1 = n->py + n->oy;
		if ((n->px + n->ox + n->dx) > ext.x2) ext.x2 = n->px + n->ox + n->dx;
		if ((n->py + n->oy + n->dy) > ext.y2) ext.y2 = n->py + n->oy + n->dy;
	}
	for (a = 0; a < upn; a++) {
		p = &UpdateRect[a];
		if (!(p->x2 - p->x1) || !(p->y2 - p->y1))
			continue;

		if (p->x1 < ext.x1) ext.x1 = p->x1;
		if (p->y1 < ext.y1) ext.y1 = p->y1;
		if (p->x2 > ext.x2) ext.x2 = p->x2;
		if (p->y2 > ext.y2) ext.y2 = p->y2;
	}
	auto windowInfo = game._renderer->getScreenInfos();
	if (ext.x1 < 0) ext.x1 = 0;
	if (ext.y1 < 0) ext.y1 = 0;
	if (ext.x2 > (int32)windowInfo.width) ext.x2 = windowInfo.width;
	if (ext.y2 > (int32)windowInfo.height) ext.y2 = windowInfo.height;
#ifdef DEBUG_REGEN
	DebugFile("Aggiorna video %d,%d %d,%d", ext.x1, ext.y1, ext.x2 - ext.x1, ext.y2 - ext.y1);
//	DebugLogWindow( "Aggiorna video %d,%d %d,%d", ext.x1, ext.y1, ext.x2-ext.x1, ext.y2-ext.y1 );
#endif
	rUpdateExtends(ext.x1, ext.y1, ext.x2, ext.y2);
	game._renderer->blitScreenBuffer();
	rResetExtends();

	// 5 - Copy PaintRect to OldPaintRect
	memcpy(OldPaintRect, PaintRect, sizeof(OldPaintRect));
//	memset( OldPaintRect, 0, sizeof( OldPaintRect ) );
}


/* -----------------16/11/98 17.29-------------------
 *                  ResetScreenBuffer
 * --------------------------------------------------*/
void ResetScreenBuffer() {
	for (uint i = 0; i < ARRAYSIZE(OldPaintRect); i++) OldPaintRect[i].reset();

	if (!rClearBuffers(rCLEARSCREENBUFFER | rCLEARZBUFFER))
		warning("Unable to clear screenbuffer");
}

/* -----------------30/10/98 16.18-------------------
 *                  AfterRender
 * --------------------------------------------------*/
void AfterRender(WGame &game) {
	PaintT2D(*game._renderer);
	PaintInventory(game);
	PaintText(game);
}

/* -----------------31/10/98 15.56-------------------
 *                  AddPaintRect
 * --------------------------------------------------*/
void AddPaintRect(int32 tnum, int32 px, int32 py, int32 ox, int32 oy, int32 dx, int32 dy) {
	int32 a;
	for (a = 0; a < MAX_PAINT_RECTS; a++)
		if (!PaintRect[a].tnum)
			break;

	// ce ne sono troppe
	if (a >= MAX_PAINT_RECTS) {
		warning("Too many PaintRects!");
		return ;
	}

	PaintRect[a].tnum  = tnum;
	PaintRect[a].px    = (px);
	PaintRect[a].py    = (py);
	PaintRect[a].ox    = (ox);
	PaintRect[a].oy    = (oy);
	PaintRect[a].dx    = (dx);
	PaintRect[a].dy    = (dy);
}

void Renderer::add2DStuff() {
	// Insert pre-calculated images and texts
	_2dStuff.writeBitmapListTo(PaintRect);

	_2dStuff.garbageCollectPreRenderedText();

	// Put mouse over everything
	if ((!mHide) && (CurDialog <= dSUPERVISORE) && (!bTitoliCodaStatic) && (!bTitoliCodaScrolling)) {
		int32 cmx = mPosx - mHotspotX;
		int32 cmy = mPosy - mHotspotY;

		if (cmx >= MousePointerLim.x2)
			cmx = MousePointerLim.x2 - 1;
		else if (cmx <= MousePointerLim.x1)
			cmx = MousePointerLim.x1 + 1;

		if (cmy >= MousePointerLim.y2)
			cmy = MousePointerLim.y2 - 1;
		else if (cmy <= MousePointerLim.y1)
			cmy = MousePointerLim.y1 + 1;

		// Draw the current mouse pointer
		if (CurMousePointer > 0)
			AddPaintRect(CurMousePointer, (cmx), (cmy), 0, 0, this->getBitmapDimX(CurMousePointer), this->getBitmapDimY(CurMousePointer));
	}

	Regen(*_game);

	_2dStuff.clearBitmapList();
	_2dStuff.clearTextList();

	//check
	CheckExtraLocalizationStrings(*this, 0);
}

/* -----------------27/10/98 17.14-------------------
 *                  Add3DStuff
 * --------------------------------------------------*/
void Add3DStuff(WGame &game) {
	extern uint32 StatNumTris, StatNumVerts;
	struct SD3DRect *p;
	struct SD3DTriangle *t;
	int32 a, y = 0;
//	char *PauseAnimStr[] = { "", "(PAUSE)" };

	if (bShowBoundingBox) {
		t3dShowBoundingBox(t3dCurRoom);
		for (a = ocCUOCO; a < ocCURPLAYER; a++)
			if ((Character[a]) && (Character[a]->Body) && !(Character[a]->Flags & T3D_CHARACTER_HIDE))
				t3dShowBoundingBox(Character[a]->Body);
	}

	if (bShowPanels && Player && Player->Walk.Panel)
		t3dShowBounds(Player->Walk.Panel, Player->Walk.PanelNum);
	else if (bShowPanels)
		t3dShowBounds(t3dCurRoom->Panel[t3dCurRoom->CurLevel], t3dCurRoom->NumPanels[t3dCurRoom->CurLevel]);

	//DisplayDDBitmap( TrecLogo, 800-10-rGetBitmapRealDimX(TrecLogo),0, 0,0, 0,0 );
	if (bShowInfo) {
		//display version
		uint32 date = 0, time = 0, d = 0, m = 0, yy = 0, h = 0, min = 0;
		t3dForceNOFastFile(1);
		if (t3dGetFileDate(&date, &time, "wm.exe ")) {
			d = date - (date / 100) * 100;
			date /= 100;
			m = date - (date / 100) * 100;
			date /= 100;
			yy = date;

			time /= 100;
			min = time - (time / 100) * 100;
			time /= 100;
			h = time;
		} else {
			d = m = yy = h = min = 0;
		}
		t3dForceNOFastFile(0);
		DebugVideo(*game._renderer, 1, 600 - 20, "%s (%02d/%02d/%4d - %d.%d)", WM_CUR_VERSION, d, m, yy, h, min);

		if (LoaderFlags & T3D_DEBUGMODE) {
			auto windowInfo = game._renderer->getScreenInfos();
			if (CurFps > 100.0f) CurFps = 100.0f;
			if (AvgFps > 100.0f) AvgFps = 100.0f;
			if (!AvgFps) AvgFps = CurFps;
			if (AvgFps != ofps) {
				ofps = AvgFps;
				hi = 0.0f;
				lo = 999.0f;
			}
			if (CurFps > hi) hi = CurFps;
			if (CurFps < lo) lo = CurFps;
			DisplayD3DRect(*game._renderer, 1, 1, (int32)((t3dF32)windowInfo.width * (CurFps / 101.0f)), 13, 78, 78, 78, 228);
			DebugVideo(*game._renderer, 1, y += 16, "FPS: ( LOW %2d | AVG %2d | HI %2d ) TRI: %d VERT: %d", (int)lo, (int)AvgFps, (int)hi, StatNumTris, StatNumVerts);
			//      DebugVideo(1,y+=16,"(%d) CUR %f %s",(int)(1000.0f/CurFps),CurFps,PauseAnimStr[bPauseAllAnims]);
			DebugVideo(*game._renderer, 1, y += 16, "%d,%d: %s (%d %d) %X", game._gameVars.getCurRoomId(), CurObj, ObjectUnderCursor, NextPortalObj, NextPortalAnim, game.init.Obj[CurObj].flags);
			//      DebugVideo(1,y+=16,"Player: %d %d %d %d",Player->Mesh->BlendPercent,Player->Mesh->CurFrame,Player->Mesh->LastFrame,Player->Walk.CurAction);
			DebugVideo(*game._renderer, 1, y += 16, "DialogActive: %d   AnimWaitText: %d  PlayerInAnim: %d | %d", bDialogActive, bAnimWaitText, bPlayerInAnim, Player->Mesh->CurFrame);
			DebugVideo(*game._renderer, 1, y += 16, "CurCamera %d    CurTime %d (%f %f)", game._cameraMan->getCurCameraIndex() + 1, t3dCurTime, t3dCurCamera->Source.x, t3dCurCamera->Source.z);
//			DebugVideo(1,y+=16,"xy(%f %f)",Character[1]->Mesh->Trasl.x,Character[1]->Mesh->Trasl.y);
//			DebugVideo(1,y+=16,"xy(%f %f)",Character[2]->Dir.x,Character[2]->Dir.y);
			DebugVideo(*game._renderer, 1, y += 16, "bPlayerSuBasamento %d", bPlayerSuBasamento);
			DebugVideo(*game._renderer, 1, y += 16, "%f %f ", Player->Mesh->Trasl.y, Player->Pos.y);
//			DebugVideo(1,y+=16,"CurP %d",CurPlayer);
//			DebugVideo(1,y+=16,"InvStatus ON%d  (1)%d  (2)%d  (3)%d  (4)%d  (5)%d",InvStatus&1,InvStatus&2,InvStatus&4,InvStatus&8,InvStatus&16,InvStatus&32);
			DebugVideo(*game._renderer, 1, y += 16, "bMovingCamera%d", bMovingCamera);

//			DebugVideo(1,y+=16,"1-%s",PlayerStand[0].RoomName);
//			DebugVideo(1,y+=16,"2-%s",PlayerStand[1].RoomName);
			/*          if( Character[1]->CurRoom )
			            {
			                if( Character[1]->CurRoom->Name[0]=='\0' )  DebugVideo(1,y+=16,"1-NULLA");
			                else                                        DebugVideo(1,y+=16,"1-%s",Character[1]->CurRoom->Name);
			            }

			            if( Character[2]->CurRoom )
			            {
			                if( Character[2]->CurRoom->Name[0]=='\0' )  DebugVideo(1,y+=16,"2-NULLA");
			                else                                        DebugVideo(1,y+=16,"2-%s",Character[2]->CurRoom->Name);
			            }
			*/
			DebugVideo(*game._renderer, 1, windowInfo.height - 40, "%s", CurDebugString);
		} else {
//			extern t3dU8 t3dCurCameraIndex;
//			DebugVideo(1,y+=16,"CurCamera %d    CurTime %d",t3dCurCameraIndex+1,t3dCurTime);
//			extern t3dU8 t3dCurCameraIndex;

//			if( !AvgFps ) AvgFps = CurFps;
//			if( AvgFps != ofps ) { ofps = AvgFps; hi = 0.0f; lo = 999.0f; }
//			if( CurFps > hi ) hi = CurFps;
//			if( CurFps < lo ) lo = CurFps;
//			DebugVideo(1,y+=16,"mHide %d",mHide);
//			DebugVideo(1,1,"FPS: %3d TRI: %d VERT: %d",(int)AvgFps,StatNumTris,StatNumVerts);
//			DebugVideo(1,y+=16,"CurTime %d    CurCamera %d",t3dCurTime,t3dCurCameraIndex+1);
//			DebugVideo(1,y+=16,"CurDialog %d, obj %d",CurDialog,Dialog[CurDialog].obj);
//			DebugVideo(1,y+=16,"%d,%d: %s (%d %d) %X",CurRoom,CurObj,ObjectUnderCursor,NextPortalObj,NextPortalAnim,Obj[CurObj].flags); //_remove
		}
	}
//	Aggiunge i rettangoli D3D
	for (a = 0, p = &D3DRectsList[0]; a < MAX_D3D_RECTS; a++, p++)
		if (p->dx || p->dy)
			t3dAddQuad((t3dF32)(p->px), (t3dF32)(p->py), (t3dF32)(p->px + p->dx), (t3dF32)(p->py),
			           (t3dF32)(p->px), (t3dF32)(p->py + p->dy), (t3dF32)(p->px + p->dx), (t3dF32)(p->py + p->dy),
			           p->r, p->g, p->b, p->a);
	for (a = 0, p = &D3DRectsList[0]; a < MAX_D3D_RECTS; a++, p++)
		p->dx = p->dy = 0;
//	Aggiunge i triangoli D3D
	for (a = 0, t = &D3DTrianglesList[0]; a < MAX_D3D_TRIANGLES; a++, t++)
		if (t->x1 || t->y1 || t->x2 || t->y2)
			t3dAddTriangle((t3dF32)(t->x1), (t3dF32)(t->y1), (t3dF32)(t->x2), (t3dF32)(t->y2),
			               (t3dF32)(t->x3), (t3dF32)(t->y3), t->r, t->g, t->b, t->a);
	for (a = 0, t = &D3DTrianglesList[0]; a < MAX_D3D_TRIANGLES; a++, t++)
		t->x1 = t->y1 = t->x2 = t->y2 = t->x3 = t->y3 = 0;
}

} // End of namespace Watchmaker
