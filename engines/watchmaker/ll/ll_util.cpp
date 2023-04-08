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
#define FORBIDDEN_SYMBOL_EXCEPTION_vsprintf

#include "watchmaker/ll/ll_util.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/ll/ll_mesh.h"
#include "watchmaker/ll/ll_regen.h"
#include "watchmaker/3d/animation.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/t3d_mesh.h"
#include "watchmaker/classes/do_camera.h"
#include "watchmaker/classes/do_operate.h"
#include "watchmaker/classes/do_system.h"
#include "watchmaker/define.h"
#include "watchmaker/globvar.h"
#include "watchmaker/main.h"
#include "watchmaker/message.h"
#include "watchmaker/renderer.h"
#include "watchmaker/schedule.h"
#include "watchmaker/sysdef.h"
#include "watchmaker/t2d/t2d_internal.h"
#include "watchmaker/types.h"
#include "watchmaker/utils.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/windows_hacks.h"

namespace Watchmaker {

/* -----------------17/03/98 16.19-------------------
 *                  LinkObjToMesh
 * --------------------------------------------------*/
uint16 LinkObjToMesh(WGame &game, t3dMESH *m, uint8 op) {
	uint16 a, b, c, i;
	t3dBODY *cr;
	int16 n;
	Init &init = game.init;

	if (!m) return (oNULL);

	NextPortalObj = oNULL;
//	Controllo tra gli altri personaggi
	for (c = ocCUOCO; c <= ocLASTCHAR; c++) {
		for (b = 0; b < MAX_OBJ_MESHLINKS; b++)
			if (Character[c] && Character[c]->Mesh)
				if ((init.Obj[c].meshlink[b][0] != 0) && (m->name.equalsIgnoreCase((const char *) init.Obj[c].meshlink[b])))
					return (c);
	}

//	Controllo in stanza attuale
	for (a = 0; a < MAX_OBJS_IN_ROOM; a++) {
		if (!(c = game.getCurRoom().objects[a])) continue;
		if ((init.Obj[c].flags & ON) && !(init.Obj[c].flags & HIDE) &&
		        (((bFirstPerson) && !(init.Obj[c].flags & HIDEIN1ST)) ||
		         (!(bFirstPerson) && !(init.Obj[c].flags & HIDEIN3RD)))) {
			for (b = 0; b < MAX_OBJ_MESHLINKS; b++)
				if ((init.Obj[c].meshlink[b][0] != 0) && (m->name.equalsIgnoreCase((const char *)init.Obj[c].meshlink[b])))
					for (i = 0; i < t3dCurRoom->NumMeshes(); i++)
						if ((m->name.equalsIgnoreCase(t3dCurRoom->MeshTable[i].name)))
							return (c);
		}
	}

//	Se non trova controlla in portali vicino
	for (i = 0; i < t3dCurRoom->NumMeshes(); i++) {
		if ((cr = t3dCurRoom->MeshTable[i].PortalList) != nullptr) {
			NextPortalAnim = aNULL;
			n = getRoomFromStr(init, cr->name);

			if ((op == ME_MRIGHT) || (op == ME_MLEFT)) {
				for (a = 0; a < MAX_ANIMS_IN_ROOM; a++) {
					b = game.getCurRoom().anims[a];
					if ((b == aNULL) || !(init.Anim[b].flags & ANIM_PORTAL_LINK)) continue;

					if (t3dCurRoom->MeshTable[i].name.equalsIgnoreCase((const char *)init.Anim[b].RoomName.rawArray())) {
						NextPortalAnim = b;
						break;
					}
				}
			}

			for (a = 0; a < MAX_OBJS_IN_ROOM; a++) {
				if (!(c = init.Room[n].objects[a])) continue;
				if ((init.Obj[c].flags & ON) && !(init.Obj[c].flags & HIDE) &&
				        (((bFirstPerson) && !(init.Obj[c].flags & HIDEIN1ST)) ||
				         (!(bFirstPerson) && !(init.Obj[c].flags & HIDEIN3RD)))) {
					for (b = 0; b < MAX_OBJ_MESHLINKS; b++) {
						if ((init.Obj[c].meshlink[b][0] != 0) && m->name.equalsIgnoreCase((const char *)init.Obj[c].meshlink[b])) {
							if ((op == ME_MRIGHT) || (op == ME_MLEFT))
								NextPortalObj = c;
							return (c);
						}
					}
				}
			}

			if ((op == ME_MRIGHT) || (op == ME_MLEFT)) {
				for (a = 0; a < cr->NumMeshes(); a++) {
					if (m->name.equalsIgnoreCase(cr->MeshTable[a].name)) {
						NextPortalObj = oNEXTPORTAL;
						return (oNULL);
					}
				}
			}
		}
	}

	if ((op == ME_MRIGHT) || (op == ME_MLEFT))
		NextPortalAnim = aNULL;
	return (oNULL);
}


/* -----------------16/04/98 11.33-------------------
 *                  WhatIObj
 * --------------------------------------------------*/
int32 WhatObj(WGame &game, int32 mx, int32 my, uint8 op) {
	t3dMESH *pm, *CurMesh;
	t3dF32 minz;
	int32 ret;
	uint16 n;

	op = ME_MRIGHT;

	ret = 0;
	CurMesh = nullptr;
	FloorHit = 0;
	minz = 999999999.9f;
	for (n = 0; n < t3d_NumMeshesVisible; n++) {                                                // Prima cerca in mesh associate
		if (!(pm = t3d_VisibleMeshes[n])) continue;

		if ((pm->BBoxAverageZ < minz) && ((pm->Flags & T3D_MESH_RAYBAN) || (LinkObjToMesh(game, pm, 0)))) {
			minz = pm->BBoxAverageZ;
			CurMesh = pm;
		}
	}
	if (CurMesh == nullptr) {                                                                           // Poi cerca nelle altre
		for (n = 0; n < t3d_NumMeshesVisible; n++) {
			if (!(pm = t3d_VisibleMeshes[n])) continue;

			if (!(pm->Flags & T3D_MESH_NOBOUNDBOX) && (pm->BBoxAverageZ < minz)) {
				minz = pm->BBoxAverageZ;
				CurMesh = pm;
			}
		}

		if (t3dCheckWithFloor() < minz)
			FloorHit = 1;
	}

	memset(ObjectUnderCursor, 0, 400);
	if (CurMesh || (FloorHit)) {
		if (!FloorHit) {
			strcpy(ObjectUnderCursor, CurMesh->name.c_str());
			ret = LinkObjToMesh(game, CurMesh, op);
			if (NextPortalObj)
				sprintf(ObjectUnderCursor, "NextPortalObj -> %s", CurMesh->name.c_str());

			mPos.x = CurMesh->Intersection.x;
			mPos.y = CurMesh->Intersection.y;
			mPos.z = CurMesh->Intersection.z;
		} else {
			if (CurMesh)
				sprintf(ObjectUnderCursor, "Floor Hit -> %s", CurMesh->name.c_str());
			else
				strcpy(ObjectUnderCursor, "Floor Hit");
			LinkObjToMesh(game, CurMesh, op);
			if (NextPortalObj)
				sprintf(ObjectUnderCursor, "NextPortalObj -> Floor Hit -> %s", CurMesh->name.c_str());

			mPos.x = FloorHitCoords.x;
			mPos.y = FloorHitCoords.y;
			mPos.z = FloorHitCoords.z;
		}
	}
	return (ret);
}

/* -----------------13/07/98 12.05-------------------
 *                  getRoomFromStr
 * --------------------------------------------------*/
int16 getRoomFromStr(Init &init, const Common::String &s) {
	char t[100]; //, *str;

	auto end = s.findLastOf(".-");
	if (end == s.npos) {
		end = s.size() - 1;
	}
	auto start = s.findLastOf("\\/");
	if (start == s.npos) {
		start = 0;
	}
	Common::String str = s.substr(start, end - start);

	for (int a = 0; a < MAX_ROOMS; a++)
		if (str.equalsIgnoreCase((const char *)init.Room[a].name))
			return a;

	return (0);
}

/* -----------------19/01/99 11.03-------------------
 *              CreateTooltipBitmap
 * --------------------------------------------------*/
int32 CreateTooltipBitmap(Renderer &renderer, char *tooltip, FontColor color, uint8 r, uint8 g, uint8 b) {
	int32 dimx, dimy, surf, enlarge = 5;
	char info[100];

	if (!tooltip || !strcmp(tooltip, "")) return -1;
	FontKind font = FontKind::Standard;

	renderer._fonts->getTextDim(tooltip, font, &dimx, &dimy);
	dimx += renderer.rFitX(enlarge * 2);
	dimy += renderer.rFitY(enlarge * 2);

	surf = rCreateSurface(dimx, dimy, 0);
	if (surf <= 0) return -1;

	strcpy(info, "tooltip: ");
	strncat(info, tooltip, 15);
	rSetBitmapName(surf, info);
	renderer.clearBitmap(surf, 0, 0, dimx, dimy, 18, 18, 18);          // Bordino nero
	renderer.clearBitmap(surf, 1, 1, dimx - 2, dimy - 2, r, g, b);     // Sfondo colorato
	renderer.printText(tooltip, surf, font, color, (uint16)renderer.rFitX(enlarge), (uint16)renderer.rFitY(enlarge));
	return surf;
}

/* -----------------27/10/98 15.31-------------------
 *                  LoadDDBitmap
 * --------------------------------------------------*/
int32 LoadDDBitmap(WGame &game, const char *n, uint8 flags) {
	auto name = game.workDirs.join(game.workDirs._miscDir, n);

	int rez = rLoadBitmapImage(game, name.c_str(), (uint8)(rBITMAPSURFACE | rSURFACEFLIP | flags));

	if (rez <= 0) {
		warning("Failed to load %s. Quitting ...", name.c_str());
		CloseSys(game);
	}

	return (rez);
}

/* -----------------13/05/98 10.59-------------------
 *                  LinkMeshToStr
 * --------------------------------------------------*/
t3dMESH *LinkMeshToStr(Init &init, const Common::String &str) {
	if (str.empty()) return nullptr;

//	Cerca tra le camere
	if (str.equalsIgnoreCase("camera"))
		return &init._globals._invVars.CameraDummy;
//	Cerca tra i personaggi
	for (uint16 i = 0; i < T3D_MAX_CHARACTERS; i++)
		if ((Character[i]) && (str.equalsIgnoreCase((char *)init.Obj[i].meshlink[0])))
			return Character[i]->Mesh;
//	Cerca nelle stanze caricate
	for (uint16 i = 0; i < NumLoadedFiles; i++) {
		if (LoadedFiles[i].b)
			for (uint16 j = 0; j < LoadedFiles[i].b->NumMeshes(); j++) {
				if (str.equalsIgnoreCase(LoadedFiles[i].b->MeshTable[j].name))
					return &LoadedFiles[i].b->MeshTable[j];
			}
	}

	return nullptr;
}

/* -----------------18/12/00 18.02-------------------
 *                  UpdateRoomInfo
 * --------------------------------------------------*/
void UpdateRoomInfo(WGame &game) {
	uint8 cr;

	if (!t3dCurRoom) return;

	cr = (uint8)getRoomFromStr(game.init, t3dCurRoom->name);
	if (!cr) return;

//	se � diversa dalla precedente...
	if (strcmp(RoomInfo.name, game.init.Room[cr].desc)) {
		game._messageSystem.removeEvent_bparam(EventClass::MC_SYSTEM, ME_STARTEFFECT, EFFECT_ROOMINFO);
		game._messageSystem.removeEvent_bparam(EventClass::MC_SYSTEM, ME_CONTINUEEFFECT, EFFECT_ROOMINFO);
		game._messageSystem.removeEvent_bparam(EventClass::MC_SYSTEM, ME_STOPEFFECT, EFFECT_ROOMINFO);
		_vm->_messageSystem.doEvent(EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND * 3, 0, EFFECT_ROOMINFO, nullptr, nullptr, nullptr);
	}
}

/* -----------------13/10/98 15.00-------------------
 *                  UpdateRoomVisibility
 * --------------------------------------------------*/
void UpdateRoomVisibility(WGame &game) {
	uint32 i, j;
	t3dBODY *pr;
	uint8 cr;

	Init &init = game.init;

	if (!t3dCurRoom) return;

//	Leva il flag visibile a tutte le stanze
	for (i = 0; i < MAX_ROOMS; i++) {
		if (init.Room[i].flags & ROOM_VISIBLE)
			init.Room[i].flags |= ROOM_OLDVISIBLE;
		init.Room[i].flags &= ~ROOM_VISIBLE;
	}

//	Aggiunge room attuale
	if (cr = (uint8)getRoomFromStr(init, t3dCurRoom->name))
		init.Room[cr].flags |= ROOM_VISIBLE;

	if (bShowRoomDescriptions)
		UpdateRoomInfo(game);
//	if( !( Room[cr].flags & ROOM_OLDVISIBLE ) )
//		Event( EventClass::MC_SYSTEM, ME_STARTEFFECT, MP_DEFAULT, FRAME_PER_SECOND*3, 0, EFFECT_ROOMINFO, NULL, NULL, NULL );

//		Prima volta che entra nella r45
	if ((cr == r45) && !(init.Room[cr].flags & ROOM_VISITED))
		init.Obj[o46SECONDODIAGRAMMA].flags |= EXTRA;

	if (!(LoaderFlags & T3D_DEBUGMODE)) {
//		Prima volta che entra nella r46
		if ((cr == r46) && !(init.Room[cr].flags & ROOM_VISITED))
			_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR212, 0, 0, NULL, NULL, NULL);
//		Prima volta che entra nella r47
		if ((cr == r47) && !(init.Room[cr].flags & ROOM_VISITED))
			_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR471, 0, 0, NULL, NULL, NULL);
//		Prima volta che entra nella r48
		if ((cr == r48) && !(init.Room[cr].flags & ROOM_VISITED))
			_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dR481, 0, 0, NULL, NULL, NULL);
	}
#if 0
//	Se ha cambiato stanza cambia anche l'environment
	sSetEnvironment(init.Room[cr].env);
//	Se ha cambiato stanza cambia anche la musica
	if (!(LoaderFlags & T3D_NOMUSIC) && (init.Room[cr].music != nNULL))
		PlayMusic(init.Room[cr].music, 3000, 3000);
#endif
	init.Room[cr].flags |= ROOM_VISITED;

//	Aggiorna oggetti speciali
	UpdateSpecial(game, cr);

//	Cerca nelle stanze visbili con ricorsione 2
	for (i = 0; i < t3dCurRoom->NumMeshes(); i++) {
		if (!(pr = t3dCurRoom->MeshTable[i].PortalList) || (t3dCurRoom->MeshTable[i].Flags & T3D_MESH_NOPORTALCHECK))
			continue;

		if (cr = (uint8)getRoomFromStr(init, pr->name))
			init.Room[cr].flags |= ROOM_VISIBLE;

		for (j = 0; j < pr->NumMeshes(); j++)
			if ((pr->MeshTable[j].PortalList) && !(pr->MeshTable[j].Flags & T3D_MESH_NOPORTALCHECK))
				if (cr = (uint8)getRoomFromStr(init, pr->MeshTable[j].PortalList->name))
					init.Room[cr].flags |= ROOM_VISIBLE;
	}

//	Accende le animazioni di backgorund delle stanze che si vedono
//	DebugFile("RoomVisibility %s",t3dCurRoom->Name);
	for (i = 0; i < MAX_ROOMS; i++) {
		if (init.Room[i].flags & ROOM_VISIBLE) {
//			Aggiunge effetti di background
			for (j = 0; j < MAX_SOUNDS_IN_ROOM; j++)
				if ((init.Room[i].sounds[j]) && (init.Sound[init.Room[i].sounds[j]].flags & SOUND_ON)) {
					//se la pompa non e' attivata non attivo nemmeno il suono
					if ((init.Room[i].sounds[j] == wPOMPA) && (init.Obj[o2GOFF].flags & ON))  continue;

					//di notte nella r1c la fontana e' spenta
					if ((init.Room[i].sounds[j] == wFONTANA) &&
					        (((t3dCurTime >= 1300) && (t3dCurTime <= 1310)) || (t3dCurTime >= 1800))
					   )   continue;
#if 0
					StartSound(init.Room[i].sounds[j]);
#endif
				}

//			DebugFile("%d: %d %s",i,RoomVisibility[i],Room[RoomVisibility[i]].name);
			for (j = 0; j < MAX_ANIMS_IN_ROOM; j++)
				if ((init.Room[i].anims[j]) && (init.Anim[init.Room[i].anims[j]].flags & ANIM_ON))
					if (!(init.Anim[init.Room[i].anims[j]].active)) {
						if ((init.Anim[init.Room[i].anims[j]].obj != aNULL) && (Character[init.Anim[init.Room[i].anims[j]].obj])) {
							Character[init.Anim[init.Room[i].anims[j]].obj]->Flags &= ~T3D_CHARACTER_HIDE;
							CharSetPosition(init.Anim[init.Room[i].anims[j]].obj, init.Anim[init.Room[i].anims[j]].pos, (const char*)init.Anim[init.Room[i].anims[j]].RoomName.rawArray());
						}
						DebugLogWindow("Staring Bkg Anim %d | Obj %d Pos %d", init.Room[i].anims[j], init.Anim[init.Room[i].anims[j]].obj, init.Anim[init.Room[i].anims[j]].pos);
						StartAnim(game, init.Room[i].anims[j]);
					}

			StartDiary(game, i, NULL);
		}
//		Spegne le animazioni delle stanze che non si vedono piu'
		else if (init.Room[i].flags & ROOM_OLDVISIBLE) {
//			Leva effetti di background
			for (j = 0; j < MAX_SOUNDS_IN_ROOM; j++) {
#if 0
				if ((init.Room[i].sounds[j]) && (init.Sound[init.Room[i].sounds[j]].flags & SOUND_ON))
					StopSound(init.Room[i].sounds[j]);
#endif
			}

//			Se non e' piu' in vista
			for (j = 0; j < MAX_ANIMS_IN_ROOM; j++)
				if ((init.Room[i].anims[j]) && (init.Anim[init.Room[i].anims[j]].flags & ANIM_ON)) {
					if (Character[init.Anim[init.Room[i].anims[j]].obj]) Character[init.Anim[init.Room[i].anims[j]].obj]->Flags |= T3D_CHARACTER_HIDE;
					StopAnim(game, init.Room[i].anims[j]);
				}
			init.Room[i].flags &= ~ROOM_OLDVISIBLE;

			StopDiary(game, i, 0, 0);
		}
	}
}

/* -----------------12/06/00 10.12-------------------
 *                  SetBndLevel
 * --------------------------------------------------*/
bool SetBndLevel(WGame &game, const char *roomname, int32 lev) {
	t3dBODY *t;
	int32 i;

	t = nullptr;
	if (roomname && (roomname[0] != '\0')) {
		for (i = 0; i < NumLoadedFiles; i++)
			if ((LoadedFiles[i].b != nullptr) && LoadedFiles[i].b->name.equalsIgnoreCase(roomname))
				t = LoadedFiles[i].b;
	} else t = t3dCurRoom;

	if (!t) {
		DebugLogFile("SETBND FAILED: %s, %d", roomname, lev);
		return false;
	}

	if (lev >= t->NumLevels) {
		DebugLogFile("!!!! BND Lev too high: %s  max %d ask %d", t->name.c_str(), t->NumLevels, lev);
		return false;
	}

	t->CurLevel = (int16)lev;
	if (t == t3dCurRoom)
		CurFloorY = t->PanelHeight[t->CurLevel];

	_vm->addMeshModifier(t->name, MM_SET_BND_LEVEL, &lev);
	UpdateRoomVisibility(game);

	DebugLogFile("SETBND: %s, %d", t->name.c_str(), lev);

	if (Player) {
		Player->Walk.CurPanel = -1;
		Player->Walk.OldPanel = -1;
	}

	return true;
}

/* -----------------17/12/00 17.59-------------------
 *                  PrintLoading
 * --------------------------------------------------*/
void PrintLoading(void) {
	warning("STUBBED: PrintLoading");
#if 0
//	Stampa la scritta loading

	DisplayDDBitmap(LoadingImage, 800 - 103 - 4, 600 - 85 - 4,  0, 0, 0, 0);
	Add2DStuff();
	rShowFrame();

//	rBlitSetStandardFont( StandardFont.Color[WHITE_FONT], StandardFont.Table );
//	DebugQuick(10,-1,"Loading...");
//	rBlitSetStandardFont( 0, NULL );
#endif
}


/* -----------------01/09/98 16.52--------------------
 *                  ChangeRoom
 * --------------------------------------------------*/
void ChangeRoom(WGame &game, Common::String n, uint8 pos, int32 an) {
	t3dBODY *t;
	int32 i, j, k;

	if (n.equalsIgnoreCase("r21.t3d"))
		n = "r21-a.t3d";

	bFirstPerson = false;
	if (t3dCurRoom && t3dCurRoom->name.equalsIgnoreCase(n)) {
		game.UpdateAll();
		if (pos) CharSetPosition(ocCURPLAYER, pos, nullptr);
		if (an) StartAnim(game, an);
		return ;
	}

	PrintLoading();

	StopAllAnims(game.init);
//	StopMusic();

//	quando si cambia piano (e quindi si cancella quello attuale e si carica il nuovo) stoppo il diario in modo che
//	se esso aveva delle modifiche da fare al piano corrente le fa subito, e non dopo quando il piano non e' piu' disponibile
	StopDiary(game, 0, 0, 0);

	t = nullptr;
//	Prima lo cerca tra le stanze in memoria
	for (i = 0; i < NumLoadedFiles; i++)
		if ((LoadedFiles[i].b != nullptr) && LoadedFiles[i].b->name.equalsIgnoreCase(n))
			t = LoadedFiles[i].b;
//	Se non lo trova tra le stanze in memoria
	if (!t) {
		t3dResetPipeline();
		for (i = 0; i < NumLoadedFiles; i++) {
			if (LoadedFiles[i].b && !(LoadedFiles[i].Flags & T3D_STATIC_SET0)) {
				t3dReleaseBody(LoadedFiles[i].b);
				LoadedFiles[i] = RecStruct(); // TODO: Deduplicate this
			}
		}

		for (i = 0; i < T3D_MAX_CHARACTERS; i++) {
			if (Character[i]) {
				for (j = 0; j < T3D_MAX_SHADOWBOX_PER_CHAR; j++) {
					if (Character[i]->ShadowBox[j]) {
						for (k = 0; k < MAX_SHADOWS_PER_LIGHT; k++) {
							Character[i]->ShadowBox[j]->ShadowsList[k].ProjectiveTexture.clear();
						}
					}
				}
				Character[i]->Walk = t3dWALK();
			}
		}

		rReleaseAllTextures(T3D_STATIC_SET0);
		rReleaseAllBitmaps(T3D_STATIC_SET0);

//		t3dFree( StandardFont.Table );
		ReleasePreloadedAnims();
		t3dReleaseParticles();

		t3dCurRoom = nullptr;

		if (!game.LoadAndSetup(n, 0)) CloseSys(game);
		PortalCrossed = t3dCurRoom;

		ResetScreenBuffer();    //resetto in modo che la scritta di loading non faccia casini
	} else {
		ResetScreenBuffer();    //resetto in modo che la scritta di loading non faccia casini

		t3dResetPipeline();
		t3dCurRoom = t;
		PortalCrossed = t3dCurRoom;
		t3dCurCamera = &t3dCurRoom->CameraTable[0];
		t3dVectCopy(&t3dCurCamera->Target, &Player->Mesh->Trasl);
		ResetCameraSource();
		ResetCameraTarget();
		CurFloorY = t3dCurRoom->PanelHeight[t3dCurRoom->CurLevel];
	}

	CameraTargetObj = 0;
	CameraTargetBone = 0;
	ForcedCamera = 0;

	game.UpdateAll();
	if (pos)
		CharSetPosition(ocCURPLAYER, pos, nullptr);
	ProcessCamera(game);
	if (an)
		StartAnim(game, an);
}

/* -----------------21/08/00 15.48-------------------
 *                  GetBndLevel
 * --------------------------------------------------*/
int32 GetBndLevel(char *roomname) {
	t3dBODY *t;
	int32 i;

	t = nullptr;
	if (roomname && (roomname[0] != '\0')) {
		for (i = 0; i < NumLoadedFiles; i++)
			if ((LoadedFiles[i].b != nullptr) && LoadedFiles[i].b->name.equalsIgnoreCase(roomname))
				t = LoadedFiles[i].b;
	} else t = t3dCurRoom;

	if (!t) return FALSE;

	return (int32)t->CurLevel;
}

/* -----------------23/10/98 18.17-------------------
 *                  CheckRect
 * --------------------------------------------------*/
bool CheckRect(Renderer &renderer, struct SRect p, int32 cmx, int32 cmy) {
	return ((cmx >= renderer.rFitX(p.x1)) && (cmx < renderer.rFitX(p.x2)) && (cmy >= renderer.rFitY(p.y1)) && (cmy < renderer.rFitY(p.y2)));
}

/* -----------------03/05/99 17.31-------------------
 *                  DisplayD3DTriangle
 * --------------------------------------------------*/
void DisplayD3DTriangle(Renderer &renderer, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r, uint8 g, uint8 b, uint8 al) {
	int32 a;
	for (a = 0; a < MAX_D3D_TRIANGLES; a++) {
		if ((D3DTrianglesList[a].x1 == renderer.rFitX(x1)) && (D3DTrianglesList[a].y1 == renderer.rFitY(y1)) &&
		        (D3DTrianglesList[a].x2 == renderer.rFitX(x2)) && (D3DTrianglesList[a].y2 == renderer.rFitY(y2)) &&
		        (D3DTrianglesList[a].x3 == renderer.rFitX(x3)) && (D3DTrianglesList[a].y3 == renderer.rFitY(y3)))
			break;
		if (!D3DTrianglesList[a].x1 && !D3DTrianglesList[a].y1 && !D3DTrianglesList[a].x2 && !D3DTrianglesList[a].y2)
			break;
	}

	if (a >= MAX_D3D_TRIANGLES) {
		warning("Too many D3D Triangles!");
		return ;
	}

	D3DTrianglesList[a].x1 = renderer.rFitX(x1);
	D3DTrianglesList[a].y1 = renderer.rFitY(y1);
	D3DTrianglesList[a].x2 = renderer.rFitX(x2);
	D3DTrianglesList[a].y2 = renderer.rFitY(y2);
	D3DTrianglesList[a].x3 = renderer.rFitX(x3);
	D3DTrianglesList[a].y3 = renderer.rFitY(y3);

	D3DTrianglesList[a].r = r;
	D3DTrianglesList[a].g = g;
	D3DTrianglesList[a].b = b;
	D3DTrianglesList[a].a = al;
}


/* -----------------24/04/98 10.33-------------------
 *                  DisplayD3DRect
 * --------------------------------------------------*/
void DisplayD3DRect(Renderer &renderer, int32 px, int32 py, int32 dx, int32 dy, uint8 r, uint8 g, uint8 b, uint8 al) {
	int32 a;
	for (a = 0; a < MAX_D3D_RECTS; a++) {
		if ((D3DRectsList[a].px == renderer.rFitX(px)) && (D3DRectsList[a].py == renderer.rFitY(py)) &&
		        (D3DRectsList[a].dx == renderer.rFitX(dx)) && (D3DRectsList[a].dy == renderer.rFitY(dy)))
			break;
		if (!D3DRectsList[a].dx && !D3DRectsList[a].dy)
			break;
	}

	if (a >= MAX_D3D_RECTS) {
		warning("Too many D3D Rects!");
		return ;
	}

	D3DRectsList[a].px = renderer.rFitX(px);
	D3DRectsList[a].py = renderer.rFitY(py);
	D3DRectsList[a].dx = renderer.rFitX(px + dx) - renderer.rFitX(px);
	D3DRectsList[a].dy = renderer.rFitY(py + dy) - renderer.rFitY(py);

	D3DRectsList[a].r = r;
	D3DRectsList[a].g = g;
	D3DRectsList[a].b = b;
	D3DRectsList[a].a = al;
}

/* -----------------24/04/98 10.33-------------------
 *                  DisplayDDBitmap
 * --------------------------------------------------*/
void DisplayDDBitmap(Renderer &renderer, int32 tnum, int32 px, int32 py, int32 ox, int32 oy, int32 dx, int32 dy) {
	int32 a;
	for (a = 0; a < MAX_DD_BITMAPS; a++)
		if (!DDBitmapsList[a].tnum)
			break;

	if (a >= MAX_DD_BITMAPS) {
		warning("Too many DD Bitmaps!");
		return ;
	}

	DDBitmapsList[a].tnum  = tnum;
	DDBitmapsList[a].px    = renderer.rFitX(px);
	DDBitmapsList[a].py    = renderer.rFitY(py);
	DDBitmapsList[a].ox    = renderer.rFitX(px + ox) - renderer.rFitX(px);
	DDBitmapsList[a].oy    = renderer.rFitY(py + oy) - renderer.rFitY(py);
	DDBitmapsList[a].dx    = renderer.rFitX(px + dx) - renderer.rFitX(px);
	DDBitmapsList[a].dy    = renderer.rFitY(py + dy) - renderer.rFitY(py);
	if (dx <= 0) DDBitmapsList[a].dx += renderer.getBitmapDimX(tnum) - DDBitmapsList[a].ox;
	if (dy <= 0) DDBitmapsList[a].dy += renderer.getBitmapDimY(tnum) - DDBitmapsList[a].oy;
}

/* -----------------22/11/00 12.15-------------------
 *                  DisplayDDBitmap_NoFit
 * --------------------------------------------------*/
void DisplayDDBitmap_NoFit(Renderer &renderer, int32 tnum, int32 px, int32 py, int32 ox, int32 oy, int32 dx, int32 dy) {
	int32 a;
	for (a = 0; a < MAX_DD_BITMAPS; a++)
		if (!DDBitmapsList[a].tnum)
			break;

	if (a >= MAX_DD_BITMAPS) {
		warning("Too many DD Bitmaps!");
		return ;
	}

	DDBitmapsList[a].tnum  = tnum;
	DDBitmapsList[a].px    = (px);
	DDBitmapsList[a].py    = (py);
	DDBitmapsList[a].ox    = (px + ox) - (px);
	DDBitmapsList[a].oy    = (py + oy) - (py);
	DDBitmapsList[a].dx    = (px + dx) - (px);
	DDBitmapsList[a].dy    = (py + dy) - (py);
	if (dx <= 0) DDBitmapsList[a].dx += renderer.getBitmapDimX(tnum) - DDBitmapsList[a].ox;
	if (dy <= 0) DDBitmapsList[a].dy += renderer.getBitmapDimY(tnum) - DDBitmapsList[a].oy;
}

/* -----------------15/11/00 12.16-------------------
 *                  RendDDText
 * --------------------------------------------------*/
int32 RendDDText(Renderer &renderer, char *text, FontKind font, FontColor color) {
	struct SDDText *r;
	int32 c, tdx, tdy;
	char info[100];

	if ((!text) || (text[0] == '\0')) return -1;

	for (c = 0, r = &RendText[0]; c < MAX_REND_TEXTS; c++, r++) {
		if (r->text[0]) continue;
//		Prende dimesioni della scritta da renderizzare
		renderer._fonts->getTextDim(text, font, &tdx, &tdy);
//		Crea una surface che la contenga
		r->tnum = rCreateSurface(tdx, tdy, rBITMAPSURFACE);
		renderer.clearBitmap(r->tnum, 0, 0, tdx, tdy, 0, 0, 0);
//		Renderizza la scritta nella surface
//DebugLogWindow("Creo testo %s | %d %d",text,tdx,tdy );
		renderer.printText(text, r->tnum,  font, color, 0, 0);
		strcpy(info, "text: ");
		strncat(info, text, 15);
//DebugLogWindow("Creato %s",info);
		rSetBitmapName(r->tnum, info);
		strcpy(r->text, text);
		r->color = color;
		r->font = font;
		return r->tnum;
	}

	return -1;
}

/* -----------------24/04/98 10.33-------------------
 *                  DisplayDDText
 * --------------------------------------------------*/
void DisplayDDText(Renderer &renderer, char *text, FontKind font, FontColor color, int32 px, int32 py, int32 ox, int32 oy, int32 dx, int32 dy) {
	struct SDDText *t, *r;
	int32 a, c;

	if ((!text) || (text[0] == '\0')) return;

	for (a = 0; a < MAX_DD_TEXTS; a++)
		if (!DDTextsList[a].text[0])
			break;

	if (a >= MAX_DD_TEXTS) {
		warning("Too many DD Texts!");
		return ;
	}

	strcpy(DDTextsList[a].text, text);
	DDTextsList[a].tnum  = -1;
	DDTextsList[a].font  = font;
	DDTextsList[a].color = color;

	t = &DDTextsList[a];
//	Prova a cercare tra le scritte prenrenderizzate
	for (c = 0, r = &RendText[0]; c < MAX_REND_TEXTS; c++, r++) {
		if (!r->text[0]) continue;

		if (!strcmp(t->text, r->text) && (t->color == r->color) && (t->font == r->font)) {
			DisplayDDBitmap(renderer, r->tnum, px, py, ox, oy, dx, dy);
			break;
		}
	}
//	se non ho prerenderizzato la scritta, la renderizzo ora
	if (c >= MAX_REND_TEXTS) {
		/*      if( ( r->tnum = RendDDText( t->text, t->font, t->color ) ) > 0 )
		//          Aggiunge il bitmap con la scritta pre-renderizzata da visualizzare
		            DisplayDDBitmap( r->tnum, px, py, ox, oy, dx, dy );*/

		int32 tn;
		if ((tn = RendDDText(renderer, t->text, t->font, t->color)) > 0)
//          Aggiunge il bitmap con la scritta pre-renderizzata da visualizzare
			DisplayDDBitmap(renderer, tn, px, py, ox, oy, dx, dy);
	}
}


/* -----------------15/01/99 18.15-------------------
 *              GetDDBitmapExtends
 * --------------------------------------------------*/
void GetDDBitmapExtends(Renderer &renderer, struct SRect *r, struct SDDBitmap *b) {
	if (!r || !b) return ;
	r->x1 = b->px;
	r->y1 = b->py;
	// TODO: Somehow the original engine was able to handle this
	// even though rGetBitmapRealDimX used b->tnum as index
	// into gBitmapList, which becomes problematic when using
	// the upper bits for flagging. For now let's mask out
	// the high-bit.
	uint32 mask = T2D_BM_OFF ^ 0xFFFFFFFF;
	r->x2 = r->x1 + renderer.getBitmapRealDimX(b->tnum & mask);
	r->y2 = r->y1 + renderer.getBitmapRealDimY(b->tnum & mask);
}

/* -----------------05/11/98 10.36-------------------
 *                  DebugVideo
 * --------------------------------------------------*/
void DebugVideo(Renderer &renderer, int32 px, int32 py, const char *format, ...) {
	char str[500];
	va_list args;

	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);

	DisplayDDText(renderer, str, FontKind::Standard, WHITE_FONT, px, py, 0, 0, 0, 0);
//	rPrintText( str, 0, StandardFont.Color[WHITE_FONT], StandardFont.Table, px, py );
}

} // End of namespace Watchmaker
