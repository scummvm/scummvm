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

#include "asylum/scene.h"

namespace Asylum {

Scene::Scene() {
}

Scene::~Scene() {
    if (_worldStats)
        delete _worldStats;
}

bool Scene::load(uint8 sceneIdx) {
    char sceneTag[6];
    Common::File* fd = new Common::File;
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

    return true;
}

// FIXME: load necessary World Stats content
void Scene::loadWorldStats(Common::SeekableReadStream *stream) {
    _worldStats = new WorldStats;

    _worldStats->_size = stream->readUint32LE();
    _worldStats->_numEntries = stream->readUint32LE();
    _worldStats->_numChapter = stream->readUint32LE();
    
    stream->skip(24); // unused data
    
    // read common graphic resources
    stream->read(&_worldStats->_commonRes,sizeof(CommonResources));

    _worldStats->_width = stream->readUint32LE();
    _worldStats->_height = stream->readUint32LE();
    
    stream->skip(8); // unused data

    _worldStats->_numActions = stream->readUint32LE();
    _worldStats->_numActors = stream->readUint32LE();

    stream->skip(20); // unused data

    // FIXME figure out what these 3 unknown fields are
    stream->skip(12);

    // FIXME Jump unknown resource list

    stream->seek(0x2C6);
    _worldStats->_loadingScreenGrResId = stream->readUint32LE();
    _worldStats->_loadingScreenPalResId = stream->readUint32LE();

    // FIXME Jump unknown sound resource list
    // FIXME Jump resource list definitions

    stream->seek(0x6FA); // where actors definitions start

    // FIXME Figure out all the actor definitions
    for(uint32 a=0; a < _worldStats->_numActors; a++) {
        ActorDefinitions actorDef;
        memset(&actorDef, 0, sizeof(ActorDefinitions));

        actorDef.id = stream->readUint32LE();
        actorDef.graphicResId = stream->readUint32LE();
        actorDef.x = stream->readUint32LE();
        actorDef.y = stream->readUint32LE();
        stream->skip(0x30); 
        stream->read(actorDef.name, 52);       
        stream->skip(0x158);
        actorDef.soundResId = stream->readUint32LE();
        stream->skip(0x4D8);

        _worldStats->_actorsDef.push_back(actorDef);
    }
    
    // TODO grab Max actor definitions

    stream->seek(0xD6B5A); // where actors action definitions start

    // FIXME Figure out all the actor action definitions
    for(uint32 a=0; a < _worldStats->_numActions; a++) {
        ActorActionDefinitions actorActionDef;
        memset(&actorActionDef, 0, sizeof(ActorActionDefinitions));

        stream->read(actorActionDef.name,52);
        actorActionDef.id = stream->readUint32LE();
        stream->skip(0x14); 
        actorActionDef.actionListIdx1 = stream->readUint32LE();
        actorActionDef.actionListIdx2 = stream->readUint32LE();
        actorActionDef.actionType = stream->readUint32LE();
        stream->skip(0x2C); 
        actorActionDef.polyIdx = stream->readUint32LE();
        stream->skip(0x08); 
        actorActionDef.soundResId = stream->readUint32LE();
        stream->skip(0x04); 
        actorActionDef.palCorrection = stream->readUint32LE();
        stream->skip(0x14); 
        actorActionDef.soundVolume = stream->readUint32LE();

        _worldStats->_actorsActionDef.push_back(actorActionDef);
    }
}

// FIXME: load necessary Game Polygons content
void Scene::loadGamePolygons(Common::SeekableReadStream *stream) {
    _gamePolygons = new GamePolygons;

    stream->seek(0xE8686); // jump to game Polygons data

    _gamePolygons->_size = stream->readUint32LE();
    _gamePolygons->_numEntries = stream->readUint32LE();

    for(uint32 g=0; g < _gamePolygons->_numEntries; g++) {
        PolyDefinitions poly;
        memset(&poly, 0, sizeof(PolyDefinitions));
        poly.numPoints = stream->readUint32LE();
        stream->read(poly.points, sizeof(Common::Point) * Polygons_MAXSIZE);
        poly.boundingRect.top = stream->readUint32LE();
        poly.boundingRect.left = stream->readUint32LE();
        poly.boundingRect.bottom = stream->readUint32LE();
        poly.boundingRect.right = stream->readUint32LE();

        _gamePolygons->_Polygons.push_back(poly);
    }
}

// TODO: load necessary Action List content
void Scene::loadActionList(Common::SeekableReadStream *stream) {

}

Common::String Scene::parseFilename(uint8 sceneIdx) {
	char filename[8];
	sprintf(filename, SCENEMASK, sceneIdx);

	return Common::String(filename);
}

} // end of namespace Asylum
