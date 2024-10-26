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
#define FORBIDDEN_SYMBOL_EXCEPTION_sprintf

#include "watchmaker/classes/do_system.h"
#include "watchmaker/types.h"
#include "watchmaker/define.h"
#include "watchmaker/globvar.h"
#include "watchmaker/message.h"
#include "watchmaker/utils.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/ll/ll_sound.h"
#include "watchmaker/schedule.h"
#include "watchmaker/classes/do_inv.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/main.h"
#include "watchmaker/classes/do_player.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/walk/ball.h"
#include "watchmaker/classes/do_keyboard.h"
#include "watchmaker/ll/ll_mouse.h"
#include "watchmaker/classes/do_camera.h"
#include "watchmaker/ll/ll_mesh.h"
#include "watchmaker/renderer.h"


#define DISTANCE_FACTOR 0.0039054571339752935096144661711453f

namespace Watchmaker {

// locals
int32 LastPaintTime, NumTimes, acc, RenderFps, MissFps = 10, OutTime, AccTime, UpdateListener = 5;
uint32 OldTime;
t3dF32 AccFps;
uint8 FrameCounter;
sListener CurListener;
//t3dF32 FpsTable[520];int FpsC=0;
extern char bFastAnim;
// intro text
#define INTRO_TEXT_BORDER_X     10
#define INTRO_TEXT_BORDER_Y     5
#define ALPHA_BASE              10
#define R_MIN                   0
#define G_MIN                   40
#define B_MIN                   50
#define R_BASE                  0
#define G_BASE                  176
#define B_BASE                  231

struct SDDBitmap T1, T2;
struct SD3DRect rT1, rT2;
char *T1t, *T2t;
int32 LastT1, LastT2;

// Titoli di coda
extern uint16 Credits_numRoles;
extern uint16 Credits_numNames;
STitoliCoda *TitoliCoda = nullptr;
STitoliCoda *TitoliCoda_End = nullptr;
int32 TitoliCoda_NumEntries;
int32 TitoliCoda_NumDeleted;
int32 TitoliCoda_Y;
uint32 TitoliCoda_nextupdatetime;
uint32 TitoliCoda_Endtime;  //solo se TitoliCoda_End esiste
uint32 TitoliCoda_timeinc;

// Wide Screen Effect
int32 WideScreen_Y;
uint32 WideScreen_StartTime;
uint32 WideScreen_EndTime;

SRoomInfo RoomInfo;


/* -----------------22/11/00 15.41-------------------
 *                  TitoliCoda_ShowScrolling
 * --------------------------------------------------*/
void TitoliCoda_ShowScrolling(WGame &game, char initialize) {
	int32 i, start_y, end_y, ys, ye, next_y, x, y, empty_x_space;
	int32 text_y_inc;  //distanza tra una riga e l'altra
	int32 additional_text_y_inc;   //distanza aggiuntiva tra un nome e il ruolo successivo
	uint16 j, k;
	int32 y_offs;  //offset verticale dall'inizio dello schermo (praticamente l'altezza delle bande orizzontali sopra e sotto)
	char visibile;
	STitoliCoda *c;
	FontKind f;
	int32 width, height, bpp;
	Init &init = game.init;

	rGetScreenInfos((unsigned int *)&width, (unsigned int *)&height, (unsigned int *)&bpp);

	f = FontKind::PDA;
	y_offs = (width - (width * 74 / 100)) / 2; //circa 1.35:1

	if (initialize) {
		TitoliCoda_NumEntries = 0;
		TitoliCoda_Endtime = 0;
		TitoliCoda_End = nullptr;
		TitoliCoda_timeinc = 36;

		auto windowInfo = game._renderer->getScreenInfos();

		if ((windowInfo.width >= 1024) || (windowInfo.height >= 768))
			TitoliCoda_timeinc = 36;
		else if ((windowInfo.width >= 800) || (windowInfo.height >= 600))
			TitoliCoda_timeinc = 38;
		else if ((windowInfo.width >= 640) || (windowInfo.height >= 480))
			TitoliCoda_timeinc = 56;

		DebugLogWindow("TitoliCoda_timeinc %d", TitoliCoda_timeinc);

		for (j = 0; j < Credits_numNames; j++)
			if (
			    (!(init._creditsNames[j].flags & CF_STATIC))
			    && (!(init._creditsNames[j].flags & CF_SPACE))
			) {
				if (init._creditsNames[j].flags & CF_IMG)
					TitoliCoda_NumEntries += 2; //extra per l'immagine
				else
					TitoliCoda_NumEntries++;
			}

		for (j = 0; j < Credits_numRoles; j++)
			if (
			    (!(init._creditsRoles[j].flags & CF_STATIC))
			    && (!(init._creditsRoles[j].flags & CF_IMG))
			    && (!(init._creditsRoles[j].flags & CF_IMG2))
			) TitoliCoda_NumEntries++;

		TitoliCoda = new STitoliCoda[TitoliCoda_NumEntries] {};
		if (!TitoliCoda) {
			warning("TitoliCoda_ShowScrolling() Can't allocate TitoliCoda");
			game.CleanUpAndPostQuit();
		}

		game._fonts.getTextDim("{", f, &x, &y);
		text_y_inc = y;
		additional_text_y_inc = (int32)((t3dF32)y * 50.f / 100.f);
		empty_x_space = x * 2;

		k = 65535;
		next_y = height;
		c = &TitoliCoda[0];
		for (j = 0; j < Credits_numNames; j++) {
			if (init._creditsNames[j].flags & CF_STATIC)    continue;

			if (init._creditsNames[j].flags & CF_SPACE) {
				next_y += text_y_inc * atoi(init._creditsNames[j].name);
				continue;
			}

			if (init._creditsNames[j].flags & CF_IMG) {

				c->s = init._creditsNames[j].name;
				game._fonts.getTextDim(c->s, f, &c->dx, &c->dy);
				c->px = width / 2 + empty_x_space;
				c->py = next_y + 32 - c->dy / 2;
				c->tnum = -1;

				c++;

				c->s = init._creditsRoles[init._creditsNames[j].role].role;
				c->px = width / 2 - 64 - empty_x_space;
				c->py = next_y;
				c->dx = c->dy = 64;
				c->tnum = -2;

				next_y += text_y_inc + 64;

				c++;

				continue;
			}

			if (init._creditsNames[j].flags & CF_IMG2) {
				c->s = init._creditsNames[j].name;
				c->px = (width - 128) / 2;
				c->py = next_y;
				c->dx = 128;
				c->dy = 64;
				c->tnum = -2;

				next_y += text_y_inc + 64;

				c++;

				continue;
			}

			if (!(init._creditsNames[j].flags & CF_TITLE) && (k != init._creditsNames[j].role)) {
				k = init._creditsNames[j].role;

				next_y += additional_text_y_inc;

				c->s = init._creditsRoles[k].role;
				game._fonts.getTextDim(c->s, f, &c->dx, &c->dy);
				c->px = width / 2 - c->dx - empty_x_space;
				c->py = next_y;
				c->tnum = -1;
				c++;
			}

			c->s = init._creditsNames[j].name;
			game._fonts.getTextDim(c->s, f, &c->dx, &c->dy);
			c->px = width / 2 + empty_x_space;
			c->py = next_y;
			next_y += text_y_inc;
			c->tnum = -1;

			if (init._creditsNames[j].flags & CF_TITLE)
				c->px = (width - c->dx) / 2;

			if (init._creditsNames[j].flags & CF_END)
				TitoliCoda_End = c;

			c++;
		}

		TitoliCoda_nextupdatetime = TheTime;
		TitoliCoda_NumDeleted = 0;
		TitoliCoda_Y = 0;

		bTitoliCodaScrolling = 1;
		return;
	}

	start_y =  TitoliCoda_Y + y_offs;
	end_y = height + TitoliCoda_Y - y_offs;

	c = &TitoliCoda[0];
	for (i = 0; i < TitoliCoda_NumEntries; i++, c++) {
		if (TitoliCoda_nextupdatetime < TheTime) {
			if (((c->py + c->dy) > start_y) && (c->py < end_y))
				visibile = 1;
			else
				visibile = 0;

			if (visibile && (c->tnum == -1)) {
				//Crea una surface che la contenga
				c->tnum = rCreateSurface(c->dx, c->dy, rBITMAPSURFACE);
				game._renderer->clearBitmap(c->tnum, 0, 0, c->dx, c->dy, 0, 0, 0);

				//Renderizza la scritta nella surface
				game._renderer->printText(c->s, c->tnum, f, WHITE_FONT, 0, 0);
				rSetBitmapName(c->tnum, "tit di coda");

				if (TitoliCoda_End == c) {
					TitoliCoda_Endtime = TheTime + 18000;
					if (!TitoliCoda_Endtime) TitoliCoda_Endtime = 1;
				}

				DebugLogFile("tcStart %s", c->s);
			}

			if (visibile && (c->tnum == -2)) {
				//Crea una surface che la contenga
				c->tnum = LoadDDBitmap(game, c->s, 0);

				//Renderizza la scritta nella surface
				rSetBitmapName(c->tnum, "tit di coda img");

				DebugLogFile("tcStart %s", c->s);
			}

			if ((!visibile) && (c->tnum != -1) && (c->tnum != -2)) {
				//rilascia la surface
				rReleaseBitmap(c->tnum);
				c->tnum = -1;

				TitoliCoda_NumDeleted ++;

				DebugLogFile("tcEnd %s", c->s);
			}
		}//update

		if ((c->tnum != -1) && (c->tnum != -2)) {
			ys = 0;
			if (c->py < start_y) ys = start_y - c->py;

			ye = 0;
			if ((c->py + c->dy) > end_y) ye = (c->py + c->dy) - end_y;

			game._renderer->_2dStuff.displayDDBitmap_NoFit(c->tnum, c->px, c->py - TitoliCoda_Y, 0, ys, c->dx, c->dy - ys - ye);
		}
	}//for

	if (TitoliCoda_nextupdatetime < TheTime) {
		if ((TitoliCoda_End) && ((TitoliCoda_End->py - TitoliCoda_Y) <= (height / 2))) {
			//non incremento
		} else {
			TitoliCoda_Y += 1;
		}

		TitoliCoda_nextupdatetime = TheTime + TitoliCoda_timeinc;
	}

	if ((TitoliCoda_NumDeleted == TitoliCoda_NumEntries) || (TitoliCoda_Endtime && (TitoliCoda_Endtime < TheTime))) {
		c = &TitoliCoda[0];

		//mi assicuro di rilasciare tutte le surfaces
		for (i = 0; i < TitoliCoda_NumEntries; i++, c++)
			if ((c->tnum != -1) && (c->tnum != -2)) {
				//rilascia la surface
				rReleaseBitmap(c->tnum);
				c->tnum = -1;

				DebugLogFile("tcEnd %s", c->s);
			}

		delete[] TitoliCoda;
		TitoliCoda = nullptr;

		bTitoliCodaScrolling = 0;
		DebugLogFile("TITOLI CODA FINITI");

		CloseSys(game);
	}
}

/* -----------------23/11/00 10.10-------------------
 *                  TitoliCoda_ShowStatic
 * --------------------------------------------------*/
void TitoliCoda_ShowStatic(WGame &game, char initialize) {
	int32 i, next_y, x, y;
	int32 text_y_inc;  //distanza tra una riga e l'altra
	int32 additional_text_y_inc;   //distanza aggiuntiva tra un ruolo e il nome che viene sotto
	uint16 j, k;
	int32 y_offs;  //offset verticale dall'inizio dello schermo (praticamente l'altezza delle bande orizzontali sopra e sotto)
	STitoliCoda *c;
	FontKind f;
	int32 width, height, bpp;
	uint32 next_time;
	uint32 cur_time = 0;
	uint32 time_inc;    //incremento tra l'apparizione di un ruolo e il successivo
	Init &init = game.init;

	rGetScreenInfos((unsigned int *)&width, (unsigned int *)&height, (unsigned int *)&bpp);

	f = FontKind::PDA;
	y_offs = (width - (width * 74 / 100)) / 2; //circa 1.35:1
	time_inc = 2500;

	if (initialize) {
		TitoliCoda_NumEntries = 0;
		for (j = 0; j < Credits_numNames; j++)
			if (init._creditsNames[j].flags & CF_STATIC) TitoliCoda_NumEntries++;
		for (j = 0; j < Credits_numRoles; j++)
			if (init._creditsRoles[j].flags & CF_STATIC) TitoliCoda_NumEntries++;

		TitoliCoda = new STitoliCoda[TitoliCoda_NumEntries] {};
		if (!TitoliCoda) {
			warning("TitoliCoda_ShowStatic() Can't allocate TitoliCoda");
			game.CleanUpAndPostQuit();
		}

		game._fonts.getTextDim("{", f, &x, &y);
		text_y_inc = y;
		additional_text_y_inc = (int32)((t3dF32)y * 40.f / 100.f);

		next_time = 500;
		k = 65535;
		next_y = 0;
		c = &TitoliCoda[0];
		for (j = 0; j < Credits_numNames; j++) {
			if (!(init._creditsNames[j].flags & CF_STATIC)) continue;

			//versione con ruoli
			if (k != init._creditsNames[j].role) {
				next_y = 0;
				k = init._creditsNames[j].role;

				c->s = init._creditsRoles[k].role;
				game._fonts.getTextDim(c->s, f, &c->dx, &c->dy);
				c->px = (width - c->dx) / 2;
				c->py = y_offs + ((height - y_offs * 2) - (c->dy * 4)) / 2; //4 numero di comodo
				c->tnum = -1;
				c->time = cur_time = next_time;
				next_time += time_inc + 500; //500: quando scompare una serie di nomi aspetto 500 prima che appaia la successiva
				next_y = c->py + text_y_inc + additional_text_y_inc;
				c++;
			}

			c->s = init._creditsNames[j].name;
			game._fonts.getTextDim(c->s, f, &c->dx, &c->dy);
			c->px = (width - c->dx) / 2;
			c->py = next_y;
			next_y += text_y_inc;
			c->tnum = -1;
			c->time = cur_time;
			c++;
			//versione con ruoli
			/*
			            //versione senza ruoli
			            c->s = init._creditsNames[j].name;
			            rGetTextDim( c->s, f->Table, &c->dx, &c->dy );
			            c->px = (width - c->dx)/2;
			            c->py = y_offs + ((height-y_offs*2)-(c->dy*4) )/2;  //4 numero di comodo
			            c->tnum = -1;
			            c->time = cur_time = next_time;
			            next_time += time_inc+300;
			            c++;
			            k = 65535;
			            //versione senza ruoli
			*/
		}

		TitoliCoda_NumDeleted = 0;
		TitoliCoda_nextupdatetime = 0;

		bTitoliCodaStatic = 1;
		return;
	}

	if (!TitoliCoda_nextupdatetime) {
		TitoliCoda_nextupdatetime = TheTime;
		return;
	}

	c = &TitoliCoda[0];
	cur_time = TheTime - TitoliCoda_nextupdatetime;
	for (i = 0; i < TitoliCoda_NumEntries; i++, c++) {
		if (
		    (c->time < cur_time)
		    && ((c->time + time_inc) > cur_time)
		    && (c->tnum == -1)
		) {
			//Crea una surface che la contenga
			c->tnum = rCreateSurface(c->dx, c->dy, rBITMAPSURFACE);
			game._renderer->clearBitmap(c->tnum, 0, 0, c->dx, c->dy, 0, 0, 0);

			//Renderizza la scritta nella surface
			game._renderer->printText(c->s, c->tnum, f, WHITE_FONT, 0, 0);
			rSetBitmapName(c->tnum, "tit di coda");

			DebugLogFile("tcStart %s", c->s);
		}

		if (((c->time + time_inc) < cur_time) && (c->tnum != -1)) {
			//rilascia la surface
			rReleaseBitmap(c->tnum);
			c->tnum = -1;

			TitoliCoda_NumDeleted ++;

			DebugLogFile("tcEnd %s", c->s);
		}

		if (c->tnum != -1)
			game._renderer->_2dStuff.displayDDBitmap_NoFit(c->tnum, c->px, c->py, 0, 0, c->dx, c->dy);
	}//for

	if (TitoliCoda_NumDeleted == TitoliCoda_NumEntries) {
		c = &TitoliCoda[0];

		//mi assicuro di rilasciare tutte le surfaces
		for (i = 0; i < TitoliCoda_NumEntries; i++, c++)
			if ((c->tnum != -1)) {
				//rilascia la surface
				rReleaseBitmap(c->tnum);
				c->tnum = -1;

				DebugLogFile("tcEnd %s", c->s);
			}

		delete[] TitoliCoda;
		TitoliCoda = nullptr;

		bTitoliCodaStatic = 0;
		TitoliCoda_ShowScrolling(game, 1);
		DebugLogFile("TITOLI CODA FINITI");
	}
}

/* -----------------15/11/00 17.48-------------------
 *                  PaintIntroText
 * --------------------------------------------------*/
void PaintIntroText(Renderer &renderer) {
	if (T1t) {
		DisplayD3DRect(renderer, rT1.px, rT1.py, rT1.dx, rT1.dy, rT1.r, rT1.g, rT1.b, rT1.a);
		renderer._2dStuff.displayDDText(T1t, FontKind::Computer, CYAN_FONT, T1.px, T1.py, 0, 0, 0, 0);
	}
	if (T2t) {
		DisplayD3DRect(renderer, rT2.px, rT2.py, rT2.dx, rT2.dy, rT2.r, rT2.g, rT2.b, rT2.a);
		renderer._2dStuff.displayDDText(T2t, FontKind::Computer, CYAN_FONT, T2.px, T2.py, 0, 0, 0, 0);
	}
}

/*-------------------------------------------------------------------------*/
/*                               PROCESSTIME                               */
/*-------------------------------------------------------------------------*/
void ProcessTime(WGame &game) {
	// warning("STUBBED ProcessTime");

//	LARGE_INTEGER pf,pt;
	TheTime = ReadTime();

//	QueryPerformanceFrequency(&pf);QueryPerformanceCounter(&pt);
//	DebugFile("%d %d %d",pf.LowPart,pt.LowPart,TheTime);

	if ((TheTime >= NextRefresh) && ((game._messageSystem.Game.len < 1) || (NumTimes > 7))) {
		if (!bGolfActive) {
			ProcessKeyboard(game);
			ProcessMouse(game);
		} else
			ProcessGopherKeyboard();

		if (!bStartMenuActive) {
			ProcessCharacters(game);
			ProcessAnims(game);
			if (!bGolfActive)
				game._cameraMan->ProcessCamera(game);
			else
				ProcessGopherCamera(game);
		}
//		Aggiunge eventi che devo essere lanciati alla fine del frame
		_vm->_messageSystem.addWaitingMsgs(MP_WAIT_RETRACE);
//		Azzera il numero di eventi inviati tra un refresh e l'altro
		NumTimes = 0;

		if (--MissFps < 0) {

			if (!bStartMenuActive) {
				if (t3dCurCamera && (--UpdateListener < 0)) {
					//CurListener.flDistanceFactor = DISTANCE_FACTOR;
					CurListener.flDistanceFactor = 1.0f;
					CurListener.flRolloff = 0.1f;
					CurListener.v3flFrontOrientation.x = t3dCurCamera->NormalizedDir.x;
					CurListener.v3flFrontOrientation.y = t3dCurCamera->NormalizedDir.y;
					CurListener.v3flFrontOrientation.z = t3dCurCamera->NormalizedDir.z;
					CurListener.v3flTopOrientation.x = 0.0f;
					CurListener.v3flTopOrientation.y = 1.0f;
					CurListener.v3flTopOrientation.z = 0.0f;
					CurListener.v3flPosition.x = t3dCurCamera->Source.x;
					CurListener.v3flPosition.y = t3dCurCamera->Source.y;
					CurListener.v3flPosition.z = t3dCurCamera->Source.z;
					sSetListener(&CurListener);
					//DebugLogFile("Updating Listener ");
					UpdateListener = 20;

					if (t3dCurRoom) {
						_vm->_roomManager->hideRoomMeshesMatching(t3dCurRoom->name);
					}
				}
			}
//
			if (Palla50 && OldTime && (TheTime != OldTime)) {
				Ball[CurGopher].Td = (TheTime - OldTime) / 1000.0f;
				UpdateBall(game, &Ball[CurGopher]);
			}
			OldTime = TheTime;

			if (bDarkScreen) DisplayD3DRect(*game._renderer, 0, 0, SCREEN_RES_X, SCREEN_RES_Y, 1, 1, 1, 1);
			PaintIntroText(*game._renderer);

			if (bTitoliCodaStatic)
				TitoliCoda_ShowStatic(game, 0);
			if (bTitoliCodaScrolling)
				TitoliCoda_ShowScrolling(game, 0);

			Render3DEnvironment(game);

			if (bWideScreen) {
				if (TheTime < WideScreen_EndTime) {
					t3dF32 yf;
					yf = ((t3dF32)SCREEN_RES_X - ((t3dF32)SCREEN_RES_X * 74.f / 100.f)) / 2.f; //circa 1.35:1
					yf *= ((t3dF32)TheTime - (t3dF32)WideScreen_StartTime) / ((t3dF32)WideScreen_EndTime - (t3dF32)WideScreen_StartTime);
					WideScreen_Y = (int32)yf;
				}

				DisplayD3DRect(*game._renderer, 0, 0, SCREEN_RES_X, WideScreen_Y, 1, 1, 1, 1);
				DisplayD3DRect(*game._renderer, 0, SCREEN_RES_Y - WideScreen_Y, SCREEN_RES_X, SCREEN_RES_Y, 1, 1, 1, 1);
			}
//			FrameCounter ++;

			if (!LastPaintTime) LastPaintTime = TheTime;
			LastPaintTime += LoadTime;
			LoadTime = 0;
			PaintTime = ReadTime();

			if (PaintTime <= (uint32)LastPaintTime) LastPaintTime = PaintTime - 1;
			if (PaintTime <= (uint32)TheTime) LastPaintTime = TheTime - 1;

			RenderFps = (int32)((1000.0f / (t3dF32)(PaintTime - LastPaintTime)) + 0.9f);
			OutTime = (PaintTime - LastPaintTime) - (1000L / FRAME_PER_SECOND) + AccTime;
			MissFps = (int32)((t3dF32)OutTime / (t3dF32)(1000L / FRAME_PER_SECOND));
			AccTime = (int32)((t3dF32)OutTime - (t3dF32)MissFps * (t3dF32)(1000L / FRAME_PER_SECOND));
//			FrameFactor = (t3dF32)RenderFps / (t3dF32)FRAME_PER_SECOND;
			FrameFactor = 30.0f / (t3dF32)FRAME_PER_SECOND;
			if (MissFps > 8) MissFps = 8;
			if (bFastAnim) MissFps *= 16;

			CurFps = 1000.0f / (t3dF32)(PaintTime - TheTime);
			AccFps += (1000.0f / (t3dF32)(PaintTime - LastPaintTime));
			if (++acc >= 100) {
				AvgFps = AccFps / (t3dF32)acc;
				AccFps = 0.0f;
				acc = 0;
			}

//			FpsTable[FpsC++]=tagfps;if(FpsC>=500){accfps=0.0f;for(acc=50;acc<FpsC;acc++){DebugFile("%f",FpsTable[acc]);accfps+=FpsTable[acc];}DebugFile("AVG %f",accfps/(t3dF32)(FpsC-50));CloseSys();}
//			DebugLogWindow("FPS %f",1000.0f/(float)(PaintTime-TheTime));
			LastPaintTime = PaintTime;
//			Se non sono riuscito a fare in tempo
			if ((PaintTime - TheTime) >= (1000L / FRAME_PER_SECOND) * 2)
				NextRefresh = PaintTime + 1;
			else if ((PaintTime - TheTime) >= (1000L / FRAME_PER_SECOND))
				NextRefresh = PaintTime + (1000L / FRAME_PER_SECOND) - ((PaintTime - TheTime) - (1000L / FRAME_PER_SECOND));
			else
				NextRefresh = TheTime + (1000L / FRAME_PER_SECOND);
		}
	}
	NumTimes ++;
}

/* -----------------16/10/98 15.13-------------------
 *                  InitMain
 * --------------------------------------------------*/
void InitMain(WGame &game) {
	int16 a;
	Init &init = game.init;

	CurPlayer = VICTORIA;
	for (a = 0; a < MAX_ICONS; a++) if ((init.InvObj[a].flags & ON) && (init.InvObj[a].flags & VIC)) AddIcon(init, (uint8)a);
	CurPlayer = DARRELL;
	for (a = 0; a < MAX_ICONS; a++) if ((init.InvObj[a].flags & ON) && !(init.InvObj[a].flags & VIC)) AddIcon(init, (uint8)a);
	InvBase[0] = InvBase[1] = 0;
	game._gameVars.setCurRoomId(getRoomFromStr(init, LoaderName));

//	InitMessageSystem();
	ProcessTime(game);

	_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_START, MP_DEFAULT, 0, 0, 0, nullptr, nullptr, nullptr);

	if (!rClearBuffers(rCLEARBACKBUFFER | rCLEARZBUFFER))   // Cancella buffers
		warning("Unable to clear backbuffers");

	T1.tnum = T2.tnum = -1;
	T1t = T2t = nullptr;
}

/* -----------------16/10/98 15.13-------------------
 *                  NextMessage
 * --------------------------------------------------*/
void NextMessage(WGame &game) {
	ProcessTime(game);
	game._messageSystem.scheduler();

	if (TheMessage) {
		if ((TheMessage->classe == EventClass::MC_SYSTEM) && (TheMessage->event == ME_QUIT))
			CloseSys(game);

		ProcessTheMessage(game);
	}
}

/* -----------------15/11/00 16.12-------------------
 *                  UpdateIntroText
 * --------------------------------------------------*/
void UpdateIntroText(uint8 com, int32 cur, int32 max, int32 orig) {
	int32 sx = 0, sy = 0, ix = 0, iy = 0, dx = 0;

	max --;
	if ((com == EFFECT_MOVEIN_T2) || (com == EFFECT_MOVEOUT_T2)) dx = rT2.dx;
	else dx = rT1.dx;

	switch (orig) {
	case 1:
		sx =   0;
		sy =  20;
		ix = +1;
		iy =  0;
		break;
	case 2:
		sx = 300;
		sy =   0;
		ix =  0;
		iy = +1;
		break;
	case 3:
		sx = 799 - dx;
		sy =  20;
		ix = -1;
		iy =  0;
		break;
	case 4:
		sx =   0;
		sy = 220;
		ix = +1;
		iy =  0;
		break;
	case 5:
		break;
	case 6:
		sx = 799 - dx;
		sy = 220;
		ix = -1;
		iy =  0;
		break;
	case 7:
		sx =   0;
		sy = 420;
		ix = +1;
		iy =  0;
		break;
	case 8:
		sx = 300;
		sy = 599;
		ix =  0;
		iy = -1;
		break;
	case 9:
		sx = 799 - dx;
		sy = 420;
		ix = -1;
		iy =  0;
		break;
	}

	switch (com) {
	case EFFECT_FADEIN_T1:
		rT1.r = (uint8)(R_MIN + (((R_BASE - R_MIN) * cur) / max));
		rT1.g = (uint8)(G_MIN + (((G_BASE - G_MIN) * cur) / max));
		rT1.b = (uint8)(B_MIN + (((B_BASE - B_MIN) * cur) / max));
		rT1.a = (uint8)(ALPHA_BASE + (((255 - ALPHA_BASE) * (max - cur)) / max));
		return ;
	case EFFECT_FADEOUT_T1:
		rT1.r = (uint8)(R_MIN + (((R_BASE - R_MIN) * (max - cur)) / max));
		rT1.g = (uint8)(G_MIN + (((G_BASE - G_MIN) * (max - cur)) / max));
		rT1.b = (uint8)(B_MIN + (((B_BASE - B_MIN) * (max - cur)) / max));
		rT1.a = (uint8)(ALPHA_BASE + (((255 - ALPHA_BASE) * (cur)) / max));
		return ;
	case EFFECT_FADEIN_T2:
		rT2.r = (uint8)(R_MIN + (((R_BASE - R_MIN) * cur) / max));
		rT2.g = (uint8)(G_MIN + (((G_BASE - G_MIN) * cur) / max));
		rT2.b = (uint8)(B_MIN + (((B_BASE - B_MIN) * cur) / max));
		rT2.a = (uint8)(ALPHA_BASE + (((255 - ALPHA_BASE) * (max - cur)) / max));
		return ;
	case EFFECT_FADEOUT_T2:
		rT2.r = (uint8)(R_MIN + (((R_BASE - R_MIN) * (max - cur)) / max));
		rT2.g = (uint8)(G_MIN + (((G_BASE - G_MIN) * (max - cur)) / max));
		rT2.b = (uint8)(B_MIN + (((B_BASE - B_MIN) * (max - cur)) / max));
		rT2.a = (uint8)(ALPHA_BASE + (((255 - ALPHA_BASE) * (cur)) / max));
		return ;

	case EFFECT_MOVEIN_T1:
		rT1.px = sx - (ix * rT1.dx) + ix * (((rT1.dx + 2 * INTRO_TEXT_BORDER_X) * (max - cur)) / max);
		rT1.py = sy - (iy * rT1.dy) + iy * (((rT1.dy + 2 * INTRO_TEXT_BORDER_Y) * (max - cur)) / max);
//			DebugLogFile("T1i %d %d %d %d",rT1.px,rT1.py,rT1.px+rT1.dx,rT1.py+rT1.dy);
		break;
	case EFFECT_MOVEOUT_T1:
		rT1.px = sx + (ix * (2 * INTRO_TEXT_BORDER_X)) - ix * (((rT1.dx + 2 * INTRO_TEXT_BORDER_X) * (max - cur)) / max);
		rT1.py = sy + (iy * (2 * INTRO_TEXT_BORDER_Y)) - iy * (((rT1.dy + 2 * INTRO_TEXT_BORDER_Y) * (max - cur)) / max);
//			DebugLogFile("T1o %d %d %d %d",rT1.px,rT1.py,rT1.px+rT1.dx,rT1.py+rT1.dy);
		break;
	case EFFECT_MOVEIN_T2:
		rT2.px = sx - (ix * rT2.dx) + ix * (((rT2.dx + 2 * INTRO_TEXT_BORDER_X) * (max - cur)) / max);
		rT2.py = sy - (iy * rT2.dy) + iy * (((rT2.dy + 2 * INTRO_TEXT_BORDER_Y) * (max - cur)) / max) + rT1.dy;
		break;
	case EFFECT_MOVEOUT_T2:
		rT2.px = sx + (ix * (2 * INTRO_TEXT_BORDER_X)) - ix * (((rT2.dx + 2 * INTRO_TEXT_BORDER_X) * (max - cur)) / max);
		rT2.py = sy + (iy * (2 * INTRO_TEXT_BORDER_Y)) - iy * (((rT2.dy + 2 * INTRO_TEXT_BORDER_Y) * (max - cur)) / max) + rT1.dy;
		break;
	}
	T1.px = rT1.px + INTRO_TEXT_BORDER_X;
	T1.py = rT1.py + INTRO_TEXT_BORDER_Y;
	T2.px = rT2.px + INTRO_TEXT_BORDER_X;
	T2.py = rT2.py + INTRO_TEXT_BORDER_Y;
}


/* -----------------07/01/99 10.55-------------------
 *                  doSystem
 * --------------------------------------------------*/
void doSystem(WGame &game) {
	switch (TheMessage->event) {
	case ME_QUIT:
		CloseSys(game);
		break;

	case ME_CHANGEPLAYER:
		ChangePlayer(game, (uint8)TheMessage->wparam1);
		break;

	case ME_STARTEFFECT:
		TheMessage->event = ME_CONTINUEEFFECT;
		TheMessage->flags |= MP_WAIT_RETRACE;
		TheMessage->lparam[1] = TheMessage->wparam1;
		switch (TheMessage->bparam) {
		case EFFECT_FADEOUT_T1:
			if (T1t) {
				TheMessage->lparam[0] = LastT1;
				ReEvent();
			} else
				_vm->_messageSystem.addWaitingMsgs(MP_WAITA);
			break;
		case EFFECT_FADEOUT_T2:
			if (T2t) {
				TheMessage->lparam[0] = LastT2;
				ReEvent();
			} else
				_vm->_messageSystem.addWaitingMsgs(MP_WAITA);
			break;

		case EFFECT_MOVEIN_T1:
			T1t = nullptr;
			if (TheMessage->lparam[2] > 0) {
				T1t = SysSent[TheMessage->lparam[2]];
				game._fonts.getTextDim(T1t, FontKind::Computer, &T1.dx, &T1.dy);
				rT1.dx = T1.dx + INTRO_TEXT_BORDER_X * 2;
				rT1.dy = T1.dy + INTRO_TEXT_BORDER_Y * 2;
				rT1.r = R_BASE;
				rT1.g = G_BASE;
				rT1.b = B_BASE;
				rT1.a = ALPHA_BASE;
				UpdateIntroText(TheMessage->bparam, TheMessage->wparam1, TheMessage->lparam[1], TheMessage->lparam[0]);
				ReEvent();
				LastT1 = TheMessage->lparam[0];
			} else
				_vm->_messageSystem.deleteWaitingMsgs(MP_WAITA);
			break;
		case EFFECT_MOVEIN_T2:
			T2t = nullptr;
			if (TheMessage->lparam[2] > 0) {
				T2t = SysSent[TheMessage->lparam[2]];
				game._fonts.getTextDim(T2t, FontKind::Computer, &T2.dx, &T2.dy);
				rT2.dx = T2.dx + INTRO_TEXT_BORDER_X * 2;
				rT2.dy = T2.dy + INTRO_TEXT_BORDER_Y * 2;
				rT2.r = R_BASE;
				rT2.g = G_BASE;
				rT2.b = B_BASE;
				rT2.a = ALPHA_BASE;
				UpdateIntroText(TheMessage->bparam, TheMessage->wparam1, TheMessage->lparam[1], TheMessage->lparam[0]);
				ReEvent();
				LastT2 = TheMessage->lparam[0];
			} else
				_vm->_messageSystem.deleteWaitingMsgs(MP_WAITA);
			break;
		case EFFECT_ROOMINFO: {
			int32 width, height, bpp, time;

			if ((RoomInfo.tnum) && (RoomInfo.tnum != -1)) {
				rReleaseBitmap(RoomInfo.tnum);
				RoomInfo.tnum = 0;
			}

			if (game.getCurRoom().desc[0] == '\0') {
				//termino l'effetto
				TheMessage->wparam1 = 1;
				break;
			}

			//RoomInfo.f = &StandardFont;
			//RoomInfo.f = &ComputerFont;
			RoomInfo.f = FontKind::PDA;

			strcpy(RoomInfo.name, game.getCurRoom().desc);

			time = t3dCurTime;
			if (time >= 1300) time -= 1200;
			snprintf(RoomInfo.fullstring, sizeof(RoomInfo.fullstring), "%s, %2d.%02dam", RoomInfo.name, time / 100, time - (time / 100) * 100);

			rGetScreenInfos((unsigned int *)&width, (unsigned int *)&height, (unsigned int *)&bpp);
			game._fonts.getTextDim(RoomInfo.fullstring, RoomInfo.f, &RoomInfo.dx, &RoomInfo.dy);
			RoomInfo.tnum = rCreateSurface(RoomInfo.dx, RoomInfo.dy, rBITMAPSURFACE);
			if ((!RoomInfo.tnum) || (RoomInfo.tnum == -1)) {
				DebugLogWindow("EFFECT_ROOMINFO: can't create surface");
				break;
			}
			game._renderer->clearBitmap(RoomInfo.tnum, 0, 0, RoomInfo.dx, RoomInfo.dy, 0, 0, 0);
			rSetBitmapName(RoomInfo.tnum, "RoomInfo");

			RoomInfo.letter_ptr = &RoomInfo.fullstring[0];
			RoomInfo.px = game._renderer->rInvFitX(4);
			RoomInfo.py = height - RoomInfo.dy - game._renderer->rInvFitY(4);
			TheMessage->wparam1 = (int16)(((t3dF32)FRAME_PER_SECOND / 8) * (t3dF32)strlen(RoomInfo.fullstring));
			TheMessage->wparam1 += FRAME_PER_SECOND * 2 + FRAME_PER_SECOND / 2; //dopo che la scritta � completata rimane tot secondi
			RoomInfo.t_next_letter = TheMessage->wparam1;

			ReEvent();
		}
		break;
		default:
			ReEvent();
			break;
		}
		break;

	case ME_CONTINUEEFFECT:
		TheMessage->wparam1 --;
		if (TheMessage->wparam1 > 0) {
			switch (TheMessage->bparam) {
			case EFFECT_FADIN:
				bDarkScreen = false;
				DisplayD3DRect(*game._renderer, 0, 0, SCREEN_RES_X, SCREEN_RES_Y, 1, 1, 1, (byte)((255 * (TheMessage->lparam[1] - TheMessage->wparam1)) / TheMessage->lparam[1]));
				break;
			case EFFECT_FADOUT:
				DisplayD3DRect(*game._renderer, 0, 0, SCREEN_RES_X, SCREEN_RES_Y, 1, 1, 1, (byte)((255 * (TheMessage->wparam1)) / TheMessage->lparam[1]));
				break;
			case EFFECT_WAIT:
				break;
			case EFFECT_WAITDARK:
				DisplayD3DRect(*game._renderer, 0, 0, SCREEN_RES_X, SCREEN_RES_Y, 1, 1, 1, 1);
				break;
			case EFFECT_ROOMINFO:
				if ((RoomInfo.t_next_letter > TheMessage->wparam1) && ((*RoomInfo.letter_ptr) != '\0')) {
					constexpr int nameSize = ARRAYSIZE(RoomInfo.fullstring);
					char name_backup[nameSize] = {};

					Common::strlcpy(name_backup, RoomInfo.fullstring, nameSize - 1);
					*(RoomInfo.letter_ptr + 1) = '\0';

					game._renderer->clearBitmap(RoomInfo.tnum, 0, 0, RoomInfo.dx, RoomInfo.dy, 0, 0, 0);
					game._renderer->printText(RoomInfo.fullstring, RoomInfo.tnum,  RoomInfo.f, FontColor::WHITE_FONT, 0, 0);
					game._fonts.getTextDim(RoomInfo.fullstring, RoomInfo.f, &RoomInfo._dx, &RoomInfo._dy);

					strcpy(RoomInfo.fullstring, name_backup);

					RoomInfo.t_next_letter -= FRAME_PER_SECOND / 8;
					if (RoomInfo.t_next_letter < 0) RoomInfo.t_next_letter = 0;
					RoomInfo.letter_ptr ++;
				}
				game._renderer->_2dStuff.displayDDBitmap_NoFit(RoomInfo.tnum, RoomInfo.px, RoomInfo.py, 0, 0, RoomInfo._dx, RoomInfo._dy);
				break;
			case EFFECT_FADEIN_T1:
			case EFFECT_FADEOUT_T1:
			case EFFECT_FADEIN_T2:
			case EFFECT_FADEOUT_T2:
			case EFFECT_MOVEIN_T1:
			case EFFECT_MOVEOUT_T1:
			case EFFECT_MOVEIN_T2:
			case EFFECT_MOVEOUT_T2:
				UpdateIntroText(TheMessage->bparam, TheMessage->wparam1, TheMessage->lparam[1], TheMessage->lparam[0]);
				break;
			case EFFECT_DISPLAY_NEWLOGIMG:
				game._renderer->_2dStuff.displayDDBitmap(NewLogImage, 800 - 60 - 8, 4,  0, 0, 0, 0);
				break;

			}
			TheMessage->flags |= MP_WAIT_RETRACE;
			ReEvent();
		} else if (TheMessage->wparam2) {
			if (TheMessage->bparam == EFFECT_MOVEIN_T1) TheMessage->bparam = EFFECT_FADEIN_T1;
			else if (TheMessage->bparam == EFFECT_MOVEIN_T2) TheMessage->bparam = EFFECT_FADEIN_T2;
			else if (TheMessage->bparam == EFFECT_FADEOUT_T1) TheMessage->bparam = EFFECT_MOVEOUT_T1;
			else if (TheMessage->bparam == EFFECT_FADEOUT_T2) TheMessage->bparam = EFFECT_MOVEOUT_T2;
			else {
				TheMessage->bparam = EFFECT_WAIT;
				bDarkScreen = true;
			}

			TheMessage->event = ME_CONTINUEEFFECT;
			TheMessage->flags |= MP_WAIT_RETRACE;
			TheMessage->wparam1 = TheMessage->wparam2;
			TheMessage->wparam2 = 0;
			TheMessage->lparam[1] = TheMessage->wparam1;
			ReEvent();
		} else {
			switch (TheMessage->bparam) {
			case EFFECT_MOVEOUT_T1:
				T1t = nullptr;
				break;
			case EFFECT_MOVEIN_T2:
				T2t = nullptr;
				break;
			}
			TheMessage->flags |= MP_CLEARA;
			TheMessage->event = ME_STOPEFFECT;
			ReEvent();
		}
		break;

	case ME_STOPEFFECT:
		switch (TheMessage->bparam) {
		case EFFECT_ROOMINFO:
			if ((RoomInfo.tnum) && (RoomInfo.tnum != -1)) {
				rReleaseBitmap(RoomInfo.tnum);
				RoomInfo.tnum = 0;
			}
			break;
		}
		break;

	case ME_PLAYERTIMER:
//			Parte RTV allo scadere del Timer
//			DebugLogFile("ttt %d, %d",TheTime,TheMessage->lparam[0]);
		if ((int32)TheTime > TheMessage->lparam[0]) {
			StopObjAnim(game, TheMessage->wparam1);
			CharStop(TheMessage->wparam1);
			_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, (int16)TheMessage->wparam2, 0, 0, nullptr, nullptr, nullptr);
			//DebugString("Idle %d at %d",TheMessage->wparam1,TheMessage->lparam[0]);
		} else {
			TheMessage->flags |= MP_WAIT_RETRACE;
			//DebugString("NO %d",TheMessage->lparam[0]-TheTime);
			ReEvent();
		}
		break;
	}
}

} // End of namespace Watchmaker
