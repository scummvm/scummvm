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

#include "watchmaker/classes/do_sound.h"
#include "watchmaker/utils.h"
#include "watchmaker/types.h"
#include "watchmaker/globvar.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/define.h"
#include "watchmaker/ll/ll_sound.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/3d/t3d_mesh.h"
#include "watchmaker/work_dirs.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/game.h"
#include "watchmaker/walk/walkutil.h"

#define METER2UNIT 256.051971816707218167072181680248f

#define PG 3.14159265358979323846f
#define GradToRad ((2.0f*PG)/360.0f)

#define gCOS(G) cos(G*GradToRad)
#define gSIN(G) sin(G*GradToRad)

namespace Watchmaker {

/* -----------------28/06/00 16.33-------------------
 *                  InitMusic
 * --------------------------------------------------*/
bool InitMusic() {
	warning("STUBBED InitMusic");
#if 0
	if (!mInitMusicSystem())
		return false;
	if (!sInitSoundSystem(hWnd))
		return false;

	sSetDistanceFactor(METER2UNIT);
#endif
	return true;
}

bool ListenerUpdate(void) {
	sListener CurListener;

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
	return sSetListener(&CurListener);
}

/* -----------------06/04/00 11.52-------------------
 *                  PlaySound
 * --------------------------------------------------*/
bool StartSpeech(WGame &game, int32 n) {
	sSound CurSound;

	if (game.gameOptions.speech_on == FALSE) return true;

	snprintf(CurSound.name, SOUND_NAME_LEN, "%ss%04d.wav", game.workDirs._speechDir.c_str(), n);

	CurSound.lIndex = MAX_SOUNDS + n;
	CurSound.dwLooped = FALSE;
	CurSound.flMinDistance = 1000 * METER2UNIT;
	CurSound.flMaxDistance = 1001 * METER2UNIT;
	//CurSound.flMinDistance = 1*METER2UNIT;
	//CurSound.flMaxDistance = 1*METER2UNIT;
	CurSound.v3flPosition.x = 0;
	CurSound.v3flPosition.y = 0;
	CurSound.v3flPosition.z = 0;

	/*  Queste righe dovrebbero dare spazialita' al suono, ma visto che non funziona lo playamo diffuso
	    if( Anim[TimeAnim].obj )
	    {
	        CurSound.v3flPosition.x = Character[Anim[TimeAnim].obj]->Pos.x;
	        CurSound.v3flPosition.y = Character[Anim[TimeAnim].obj]->Pos.y;
	        CurSound.v3flPosition.z = Character[Anim[TimeAnim].obj]->Pos.z;
	        DebugLogWindow("StartSpeech(%d): %s",n,Character[Anim[TimeAnim].obj]->Body->Name);
	    }else
	    {
	        CurSound.v3flPosition.x = Character[CurPlayer+ocCURPLAYER]->Pos.x;
	        CurSound.v3flPosition.y = Character[CurPlayer+ocCURPLAYER]->Pos.y;
	        CurSound.v3flPosition.z = Character[CurPlayer+ocCURPLAYER]->Pos.z;
	        DebugLogWindow("StartSpeech(%d): %s",n,Character[CurPlayer+ocCURPLAYER]->Body->Name);
	    }*/


//	12 Ore
	CurSound.v3flConeOrientation.x = 1;
	CurSound.v3flConeOrientation.y = 0;
	CurSound.v3flConeOrientation.z = 0;

	CurSound.dwConeInsideAngle = 360;
	CurSound.dwConeOutsideAngle = 360;
	CurSound.dwConeOutsideVolume = 0;

	CurSound.dwFlags = SOUND_SPEECH;
	ListenerUpdate();
	return (sStartSoundDiffuse(&CurSound));
//	return( sStartSound( &CurSound, FALSE ) );
}

/* -----------------06/04/00 11.52-------------------
 *                  PlaySound
 * --------------------------------------------------*/
bool StartSound(WGame &game, int32 index) {
	sSound CurSound;
	t3dMESH *m;
	Init &init = game.init;

	if (game.gameOptions.sound_on == FALSE) return TRUE;

	snprintf(CurSound.name, SOUND_NAME_LEN, "%s%s", game.workDirs._wavDir.c_str(), init.Sound[index].name);
	CurSound.lIndex = index;
	CurSound.dwLooped = (init.Sound[index].flags & SOUND_LOOP);
	CurSound.flMinDistance = init.Sound[index].MinDist * METER2UNIT;
	CurSound.flMaxDistance = init.Sound[index].MaxDist * METER2UNIT;

	m = LinkMeshToStr(init, (char *)init.Sound[index].meshlink[0].rawArray());
	if (m) {
		CurSound.v3flPosition.x = m->Pos.x;
		CurSound.v3flPosition.y = m->Pos.y;
		CurSound.v3flPosition.z = m->Pos.z;
	} else {
		CurSound.v3flPosition.x = 0.f;
		CurSound.v3flPosition.y = 0.f;
		CurSound.v3flPosition.z = 0.f;
	}

//	360 gradi
//	CurSound.v3flConeOrientation.x = gCOS(Sound[index].Angle);
//	CurSound.v3flConeOrientation.y = 0;
//	CurSound.v3flConeOrientation.z = gSIN(Sound[index].Angle);
//	12 Ore
	CurSound.v3flConeOrientation.x = (float) - gCOS((double)(init.Sound[index].Angle * 30.0f));
	CurSound.v3flConeOrientation.y = 0.0f;
	CurSound.v3flConeOrientation.z = (float) - gSIN((double)(init.Sound[index].Angle * 30.0f));

	CurSound.dwConeInsideAngle = init.Sound[index].ConeInside;
	CurSound.dwConeOutsideAngle = init.Sound[index].ConeOutside;
	CurSound.dwConeOutsideVolume = init.Sound[index].ConeOutsideVolume;

	CurSound.dwFlags = init.Sound[index].flags;

	if (!sStartSound(&CurSound, FALSE)) return FALSE;
	if (!ListenerUpdate()) return FALSE;
	return TRUE;

}

/* -----------------06/04/00 11.52-------------------
 *                  StopSound
 * --------------------------------------------------*/
bool StopSound(int32 index) {
	return (sStopSound(index));
}

/* -----------------05/06/00 12.18-------------------
 *                  StartStepSound
 * --------------------------------------------------*/
bool StartStepSound(WGame &game, t3dV3F *pos, uint8 side) {
	double pgon[4][2];
	int32 cs, ts, index;
	sSound CurSound;
	Init &init = game.init;

	if (game.gameOptions.sound_on == FALSE) return TRUE;

	if (!pos) return FALSE;

	double px = (double)pos->x;
	double pz = (double)pos->z;
	if ((pos == &Player->Mesh->Trasl) && (side & SOUND_STAIRS)) {
		px += (double)(Player->Dir.x * HALF_STEP);
		pz += (double)(Player->Dir.z * HALF_STEP);
		side &= ~SOUND_STAIRS;
	}

	index = cs = wNULL;
	for (int32 i = 0; i < MAX_SOUNDS_IN_ROOM; i++) {
		if (((cs = game.getCurRoom().sounds[i]) != wNULL) && (init.Sound[cs].flags & side)) {
			bool found = false;
			for (int32 j = 0; j < MAX_SOUND_MESHLINKS; j++) {
				if (init.Sound[cs].meshlink[j][0] == '\0')
					break;

				auto it = t3dCurRoom->MeshTable.begin();
				for (; it != t3dCurRoom->MeshTable.end(); ++it)
					if (it->name.equalsIgnoreCase((const char *)init.Sound[cs].meshlink[j].rawArray())) {
						found = true;
						break;
					}
			}

			if ((init.Sound[cs].meshlink[0][0] == '\0') || (!found)) {                               // se non ci sono meshlink o tutti i meshlinks sono in un'altra stanza
				index = cs;                                                                     // prende questo suono e continua la ricerca
				continue;
			}

			ts = wNULL;
			for (int32 j = 0; j < MAX_SOUND_MESHLINKS; j++) {
				if (init.Sound[cs].meshlink[j][0] == '\0')
					break;

				auto it = t3dCurRoom->MeshTable.begin();
				for (; it != t3dCurRoom->MeshTable.end(); ++it)
					if (it->name.equalsIgnoreCase((const char *)init.Sound[cs].meshlink[j].rawArray()))
						break;

				if (it != t3dCurRoom->MeshTable.end()) {
					auto &m = *it;
					pgon[0][0] = (double)m.BBox[0].p.x;
					pgon[0][1] = (double)m.BBox[0].p.z;

					pgon[1][0] = (double)m.BBox[4].p.x;
					pgon[1][1] = (double)m.BBox[4].p.z;

					pgon[2][0] = (double)m.BBox[5].p.x;
					pgon[2][1] = (double)m.BBox[5].p.z;

					pgon[3][0] = (double)m.BBox[1].p.x;
					pgon[3][1] = (double)m.BBox[1].p.z;

					if (PointInside2DRectangle(pgon, px, pz)) {
						ts = cs;
						break;
					} else
						ts = -1;
				}
			}

			if (ts > wNULL) {
				index = ts;
				break;
			}
		}
	}

	if (index == wNULL)
		return FALSE;

	snprintf(CurSound.name, SOUND_NAME_LEN, "%s%s", game.workDirs._wavDir.c_str(), init.Sound[index].name);
	CurSound.lIndex = index;
	CurSound.dwLooped = (init.Sound[index].flags & SOUND_LOOP);
	CurSound.flMinDistance = init.Sound[index].MinDist * METER2UNIT;
	CurSound.flMaxDistance = init.Sound[index].MaxDist * METER2UNIT;
	if (pos != nullptr) {
		CurSound.v3flPosition.x = pos->x;
		CurSound.v3flPosition.y = pos->y;
		CurSound.v3flPosition.z = pos->z;
	}

	//12 Ore
	CurSound.v3flConeOrientation.x = (float) - gCOS((double)(init.Sound[index].Angle * 30));
	CurSound.v3flConeOrientation.y = 0;
	CurSound.v3flConeOrientation.z = (float) - gSIN((double)(init.Sound[index].Angle * 30));

	CurSound.dwConeInsideAngle = init.Sound[index].ConeInside;
	CurSound.dwConeOutsideAngle = init.Sound[index].ConeOutside;
	CurSound.dwConeOutsideVolume = init.Sound[index].ConeOutsideVolume;

	CurSound.dwFlags = init.Sound[index].flags;

	if (!ListenerUpdate()) return FALSE;
	if (!bDontPlaySteps) {
		if (!sStartSound(&CurSound, FALSE)) return FALSE;
	}
	//if (!sStartInstantSound( &CurSound ) ) return FALSE;
	//if (!ListenerUpdate()) return FALSE;
	return TRUE;
}

/* -----------------28/06/00 16.33-------------------
 *  Ferma tutti i midi in Play
 * --------------------------------------------------*/
bool StopMusic() {
	if (!mStopMusic()) return FALSE;
	//if( !sStopAllSounds() ) return FALSE;
	warning("STUBBED: StopMusic");
#if 0
	CurPlayIndex = -1;
	CurPlaySubMusic = -1;
	CurMusicName[0] = 0;
#endif
	return TRUE;
}

/* -----------------28/06/00 16.33-------------------
 * Ferma il MIDI con un fade di FadeOutTime ms,
 * fa partire il MIDI index con un fade di FadeInTime ms.
 * --------------------------------------------------*/

bool PlayMusic(int32 index, uint32 FadeOutTime, uint32 FadeInTime) {
	warning("STUBBED: PlayMusic");
#if 0
	char FileName[MAX_PATH];

	if (GameOptions.music_on == FALSE) return TRUE;

	if ((CurPlayIndex == index) && (CurPlaySubMusic == CurSubMusic)) return TRUE;
	if (!strcasecmp(CurMusicName, Music[index].name[CurSubMusic])) return TRUE;

	if ((Music[index].vol[CurSubMusic] != 0) &&
	        (Music[index].vol[CurSubMusic] != mGetAllVolume())) {
		if (!mSetAllVolume((BYTE)Music[index].vol[CurSubMusic])) return FALSE;
	}
	snprintf(FileName, MAX_PATH, "%s%s", WmMidiDir, Music[index].name[CurSubMusic]);
	if (!mLoadMusic(FileName)) {
		if (CurPlayIndex == -1) return TRUE;
		mInstantFadeOut(NULL, FadeOutTime);
		CurPlayIndex = -1;
		CurPlaySubMusic = -1;
		CurMusicName[0] = '\0';
		return TRUE;
	}

	//if (!mPlayMusic(FileName))
	if (!mCrossFade(FileName, FadeOutTime, FadeInTime)) {
		CurPlayIndex = -1;
		CurPlaySubMusic = -1;
		CurMusicName[0] = '\0';
		return FALSE;
	}

	DebugLogFile("PM %s", FileName);

	CurPlayIndex = index;
	CurPlaySubMusic = CurSubMusic;
	strcpy(CurMusicName, Music[index].name[CurSubMusic]);

#endif
	return TRUE;
}

bool StopSounds() {
	if (!sStopAllSounds()) return false;
	return true;
}

} // End of namespace Watchmaker
