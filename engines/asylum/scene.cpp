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

Scene::Scene(AsylumEngine *vm): _vm(vm) {
}

Scene::~Scene() {
}

bool Scene::load(uint8 sceneIdx){
    char sceneTag[6];
    Common::File* fd = new Common::File;
	Common::String filename = parseFilename(sceneIdx);

    if(!fd->exists(filename)){
        printf("Scene file doesn't exist %s", filename.c_str());
        return false;
    }

    fd->open(filename);

    if(!fd->isOpen()){
        printf("Failed to load scene file %s", filename.c_str());
        return false;
    }

    fd->read(sceneTag,6);

    if(Common::String(sceneTag,6) != "DFISCN"){
        printf("The file isn't recognized as scene %s", filename.c_str());
        return false;
    }

    loadWorldStats(fd);

    // TODO: load Game Polies
    // TODO: load Action List

    return true;
}

// FIXME: load necessary World Stats content
void Scene::loadWorldStats(Common::SeekableReadStream *stream){
    _worldStats = new WorldStats;
    _worldStats->_size = stream->readUint32LE();
    _worldStats->_numEntries = stream->readUint32LE();
    _worldStats->_numChapter = stream->readUint32LE();
    
    stream->skip(24); //unUsed data
    
    for(int i=0; i < 25; i++){
        _worldStats->_commonGrResIdArray[i] = stream->readUint32LE();
    }

    _worldStats->_width = stream->readUint32LE();
    _worldStats->_height = stream->readUint32LE();
    
    stream->skip(8); //unUsed data

    _worldStats->_numActions = stream->readUint32LE();
    _worldStats->_numActors = stream->readUint32LE();

    stream->skip(20); //unUsed data

    // 3 unknown fields
}

Common::String Scene::parseFilename(uint8 sceneIdx) {
	char filename[8];
	sprintf(filename, SCENEMASK, sceneIdx);

	return Common::String(filename);
}

} // end of namespace Asylum
