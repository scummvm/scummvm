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

#ifndef WATCHMAKER_LOADER_H
#define WATCHMAKER_LOADER_H

#include "watchmaker/t3d.h"
#include "watchmaker/types.h"
#include "watchmaker/work_dirs.h"

namespace Watchmaker {

#define SCALEFACTOR     1.0f//(1.0f/7.0f)

#define MAX_MIRRORS     10

#define T3D_GENERATESHADOWMAPS  (1<<0)      //Generate shadow maps
#define T3D_NOLIGHTMAPS         (1<<1)      //view without lightmaps
#define T3D_NORECURSION         (1<<2)      //do not recurse sub-rooms mirrors, reflections
#define T3D_HALFTEXTURESIZE     (1<<3)      //Half the texture dimension when load .t3d texture
#define T3D_FULLSCREEN          (1<<4)      //run in fullscreen
#define T3D_FASTRENDERING       (1<<5)      //render shadow/light maps with minimum dimension
#define T3D_OUTDOORLIGHTS       (1<<6)      //load .oli files for outdoor lights informations
#define T3D_NOVOLUMETRICLIGHTS  (1<<7)      //do not load .vol file
#define T3D_NOBOUNDS            (1<<8)      //do not load .vol file
#define T3D_NOCAMERAS           (1<<9)      //do not load .vol file
#define T3D_NONEXCLUSIVEMOUSE   (1<<10)     // mouse is not in exclusive mode
#define T3D_RECURSIONLEVEL1     (1<<12)     // one recursion level only
#define T3D_SKY                 (1<<13)     // if the sky is visible
#define T3D_PRELOAD_RXT         (1<<14)     // preload extern
#define T3D_STATIC_SET0         (1<<15)     // static loaded elements
#define T3D_STATIC_SET1         (1<<16)     // static loaded elements
#define T3D_NOSHADOWS           (1<<17)     // do not calc shadows
#define T3D_NOICONS             (1<<18)     // do not load icons
#define T3D_NOSOUND             (1<<19)     // do not use sound system
#define T3D_PRELOADBASE         (1<<20)     // preload basic elements
#define T3D_NOMUSIC             (1<<21)     // do not use music system
#define T3D_DEBUGMODE           (1<<22)     // debug mode
#define T3D_FASTFILE            (1<<23)     // fastfile
#define T3D_HIPOLYPLAYERS       (1<<24)     // hipoly players (darrell e victoria hipoly)
#define T3D_HIPOLYCHARACTERS    (1<<25)     // hipoly characters (gli abitanti del castello hipoly)

// TODO: Unglobalize
extern t3dV3F   CharCorrection;
extern t3dF32   CurFloorY;
extern int32   t3dCurTime, t3dCurOliSet;

struct RecStruct {
	Common::String name;
	t3dBODY *b = nullptr;
	uint32 Flags = 0;
};

void t3dOptimizeMaterialList(t3dBODY *b);
void t3dFinalizeMaterialList(t3dBODY *b);
void t3dPrecalcLight(t3dBODY *b, unsigned char *sun);
void t3dLoadSky(WGame &game, t3dBODY *b);

Common::String constructPath(const Common::String &prefix, const Common::String &filename, const char *suffix = nullptr);

class RoomManager {
public:
	virtual ~RoomManager() {}
	virtual void addToLoadList(t3dMESH *m, const Common::String &pname, uint32 LoaderFlags) = 0;
	virtual t3dBODY* loadRoom(const Common::String &pname, t3dBODY *b, uint16 *NumBody, uint32 LoaderFlags) = 0;
	static RoomManager *create(WGame *game);
	virtual t3dBODY *getRoomIfLoaded(const Common::String &roomname) = 0;
	virtual t3dMESH *linkMeshToStr(Init &init, const Common::String &str) = 0;
	virtual void hideRoomMeshesMatching(const Common::String &pname) = 0;
	virtual void releaseBody(const Common::String &name, const Common::String &altName) = 0;
	virtual void releaseLoadedFiles(uint32 exceptFlag) = 0;
	virtual t3dBODY *checkIfAlreadyLoaded(const Common::String &Name) = 0;
	virtual Common::Array<t3dBODY*> getLoadedFiles() = 0;
};

} // End of namespace Watchmaker

#endif // WATCHMAKER_LOADER_H
