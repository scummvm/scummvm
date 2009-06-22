/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/sceneres.h"

namespace Asylum {

SceneResource::SceneResource() {
}

SceneResource::~SceneResource() {
	delete _worldStats;
	delete _mainActor;
    for(uint i=0; i< _gamePolygons->_numEntries; i++) {
        delete[] _gamePolygons->_Polygons[i].points;
    }
    delete _gamePolygons;
}

bool SceneResource::load(uint8 sceneIdx) {
    char sceneTag[6];
    Common::File* fd        = new Common::File;
	Common::String filename = parseFilename(sceneIdx);

    if (!fd->exists(filename)) {
        printf("Scene file doesn't exist %s", filename.c_str());
        return false;
    }

    fd->open(filename);

    if (!fd->isOpen()) {
        printf("Failed to load scene file %s", filename.c_str());
        return false;
    }

    fd->read(sceneTag,6);

    if (Common::String(sceneTag,6) != "DFISCN") {
        printf("The file isn't recognized as scene %s", filename.c_str());
        return false;
    }

    loadWorldStats(fd);
    loadGamePolygons(fd);
    loadActionList(fd);

    fd->close();
    delete fd;

    return true;
}

// FIXME: load necessary World Stats content
void SceneResource::loadWorldStats(Common::SeekableReadStream *stream) {
    _worldStats = new WorldStats;

    _worldStats->_size       = stream->readUint32LE();
    _worldStats->_numEntries = stream->readUint32LE();
    _worldStats->_numChapter = stream->readUint32LE();

    stream->skip(24); // unused data

    // read common graphic resources
	_worldStats->_commonRes.backgroundImage    = stream->readUint32LE();
	_worldStats->_commonRes.curScrollUp        = stream->readUint32LE();
    _worldStats->_commonRes.curScrollUpLeft    = stream->readUint32LE();
    _worldStats->_commonRes.curScrollLeft      = stream->readUint32LE();
    _worldStats->_commonRes.curScrollDownLeft  = stream->readUint32LE();
    _worldStats->_commonRes.curScrollDown      = stream->readUint32LE();
    _worldStats->_commonRes.curScrollDownRight = stream->readUint32LE();
    _worldStats->_commonRes.curScrollRight     = stream->readUint32LE();
    _worldStats->_commonRes.curScrollUpRight   = stream->readUint32LE();
    _worldStats->_commonRes.curHand            = stream->readUint32LE();
    _worldStats->_commonRes.curMagnifyingGlass = stream->readUint32LE();
    _worldStats->_commonRes.curTalkNCP         = stream->readUint32LE();
    _worldStats->_commonRes.curGrabPointer     = stream->readUint32LE();
    _worldStats->_commonRes.curTalkNCP2        = stream->readUint32LE();
    _worldStats->_commonRes.unknown1           = stream->readUint32LE();
    _worldStats->_commonRes.unknown2           = stream->readUint32LE();
    _worldStats->_commonRes.unknown3           = stream->readUint32LE();
    _worldStats->_commonRes.palette            = stream->readUint32LE();
    _worldStats->_commonRes.cellShadeMask1     = stream->readUint32LE();
    _worldStats->_commonRes.cellShadeMask2     = stream->readUint32LE();
    _worldStats->_commonRes.cellShadeMask3     = stream->readUint32LE();
    _worldStats->_commonRes.unused             = stream->readUint32LE();
    _worldStats->_commonRes.smallCurUp         = stream->readUint32LE();
    _worldStats->_commonRes.smallCurDown       = stream->readUint32LE();
    _worldStats->_commonRes.unknown4           = stream->readUint32LE();

    _worldStats->_width  = stream->readUint32LE();
    _worldStats->_height = stream->readUint32LE();

    stream->skip(8); // unused data

    _worldStats->_numActions = stream->readUint32LE();
    _worldStats->_numActors  = stream->readUint32LE();

    stream->skip(20); // unused data

    // FIXME figure out what these 3 unknown fields are
    stream->skip(12);

    // FIXME Jump unknown resource list

    stream->seek(0x2C6);
    _worldStats->_loadingScreenGrResId  = stream->readUint32LE();
    _worldStats->_loadingScreenPalResId = stream->readUint32LE();

    // FIXME Jump unknown sound resource list
    // FIXME Jump resource list definitions

    stream->seek(0x6FA); // where actors definitions start

    for (uint32 a = 0; a < _worldStats->_numActors; a++) {
        ActorDefinitions actorDef;
		int i;

		actorDef.id    = stream->readUint32LE();
		actorDef.resId = stream->readUint32LE();
		actorDef.x     = stream->readUint32LE();
		actorDef.y     = stream->readUint32LE();

		actorDef.boundingBox.left   = stream->readUint32LE() & 0xFFFF;
		actorDef.boundingBox.top    = stream->readUint32LE() & 0xFFFF;
		actorDef.boundingBox.right  = stream->readUint32LE() & 0xFFFF;
		actorDef.boundingBox.bottom = stream->readUint32LE() & 0xFFFF;

		actorDef.field_20   = stream->readUint32LE();
		actorDef.frameIdx   = stream->readUint32LE();
		actorDef.frameCount = stream->readUint32LE();
		actorDef.field_2C   = stream->readUint32LE();
		actorDef.field_30   = stream->readUint32LE();
		actorDef.field_34   = stream->readUint32LE();
		actorDef.flags      = stream->readUint32LE();
		actorDef.field_3C   = stream->readUint32LE();

		stream->read(actorDef.name, sizeof(actorDef.name));

		actorDef.field_74 = stream->readUint32LE();
		actorDef.field_78 = stream->readUint32LE();
		actorDef.field_7C = stream->readUint32LE();
		actorDef.field_80 = stream->readUint32LE();
		actorDef.polyIdx  = stream->readUint32LE();
		actorDef.flags2   = stream->readUint32LE();

		for (i = 0; i < 10; i++)
			actorDef.gameFlags[i] = stream->readUint32LE();

		actorDef.field_B4      = stream->readUint32LE();
		actorDef.tickCount     = stream->readUint32LE();
		actorDef.tickCount2    = stream->readUint32LE();
		actorDef.field_C0      = stream->readUint32LE();
		actorDef.field_C4      = stream->readUint32LE();
		actorDef.actionListIdx = stream->readUint32LE();

		for (i = 0; i < 16; i++) {
			actorDef.soundItems[i].resId   = stream->readUint32LE();
			actorDef.soundItems[i].field_4 = stream->readUint32LE();
			actorDef.soundItems[i].field_8 = stream->readUint32LE();
			actorDef.soundItems[i].field_C = stream->readUint32LE();

		}

		for (i = 0; i < 50; i++) {
			actorDef.frameSoundItems[i].resId    = stream->readUint32LE();
			actorDef.frameSoundItems[i].frameIdx = stream->readUint32LE();
			actorDef.frameSoundItems[i].index    = stream->readUint32LE();
			actorDef.frameSoundItems[i].field_C  = stream->readUint32LE();
			actorDef.frameSoundItems[i].field_10 = stream->readUint32LE();
			actorDef.frameSoundItems[i].field_14 = stream->readUint32LE();
		}

		actorDef.field_67C = stream->readUint32LE();
		actorDef.soundX    = stream->readUint32LE();
		actorDef.soundY    = stream->readUint32LE();
		actorDef.field_688 = stream->readUint32LE();

		for (i = 0; i < 5; i++) {
			actorDef.field_68C[i] = stream->readUint32LE();
		}

		actorDef.soundResId = stream->readUint32LE();
		actorDef.field_6A4  = stream->readUint32LE();

        _worldStats->_actorsDef.push_back(actorDef);
    }

    // TODO grab Max actor definitions
    // TODO hardcoded for RES.005 ... need to verify if this
    // offset persists across the other files, and also how
    // scenes with multiple actors work
    stream->seek(0xA73B6);

    uint8 mainActorData[500];
    stream->read(mainActorData, 500);

    _mainActor = new MainActor(mainActorData);

    stream->seek(0xD6B5A); // where actors action definitions start

    // FIXME Figure out all the actor action definitions
    for (uint32 a=0; a < _worldStats->_numActions; a++) {
        ActorActionDefinitions actorActionDef;
        memset(&actorActionDef, 0, sizeof(ActorActionDefinitions));

        stream->read(actorActionDef.name,52);
        actorActionDef.id             = stream->readUint32LE();
        stream->skip(0x14);
        actorActionDef.actionListIdx1 = stream->readUint32LE();
        actorActionDef.actionListIdx2 = stream->readUint32LE();
        actorActionDef.actionType     = stream->readUint32LE();
        stream->skip(0x2C);
        actorActionDef.polyIdx        = stream->readUint32LE();
        stream->skip(0x08);
        actorActionDef.soundResId     = stream->readUint32LE();
        stream->skip(0x04);
        actorActionDef.palCorrection  = stream->readUint32LE();
        stream->skip(0x14);
        actorActionDef.soundVolume    = stream->readUint32LE();

        _worldStats->_actorsActionDef.push_back(actorActionDef);
    }
}

// FIXME: load necessary Game Polygons content
void SceneResource::loadGamePolygons(Common::SeekableReadStream *stream) {
    _gamePolygons = new GamePolygons;

    stream->seek(0xE8686); // jump to game Polygons data

    _gamePolygons->_size       = stream->readUint32LE();
    _gamePolygons->_numEntries = stream->readUint32LE();

    for (uint32 g = 0; g < _gamePolygons->_numEntries; g++) {
        PolyDefinitions poly;
        memset(&poly, 0, sizeof(PolyDefinitions));

        poly.numPoints = stream->readUint32LE();
        if(poly.numPoints > 0)
            poly.points = new Common::Point[poly.numPoints];
        for (uint32 i = 0; i < poly.numPoints; i++) {
			poly.points[i].y = stream->readUint32LE() & 0xFFFF;
			poly.points[i].x = stream->readUint32LE() & 0xFFFF;
		}
        stream->skip((Polygons_MAXSIZE - poly.numPoints) * 8);

        poly.boundingBox.left   = stream->readUint32LE() & 0xFFFF;
        poly.boundingBox.top    = stream->readUint32LE() & 0xFFFF;
        poly.boundingBox.right  = stream->readUint32LE() & 0xFFFF;
        poly.boundingBox.bottom = stream->readUint32LE() & 0xFFFF;

        _gamePolygons->_Polygons.push_back(poly);
    }
}

// TODO: load necessary Action List content
void SceneResource::loadActionList(Common::SeekableReadStream *stream) {

}

Common::String SceneResource::parseFilename(uint8 sceneIdx) {
	char filename[10];
	sprintf(filename, SCENEMASK, sceneIdx);

	return Common::String(filename);
}

} // end of namespace Asylum
