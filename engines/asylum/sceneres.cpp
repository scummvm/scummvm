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
    delete _actionList;
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

    _worldStats->_numChapter          = stream->readUint32LE();
    _worldStats->_xLeft               = stream->readUint32LE();
    _worldStats->_yTop                = stream->readUint32LE();
    _worldStats->_boundingRect.left   = stream->readUint32LE() & 0xFFFF;
	_worldStats->_boundingRect.top    = stream->readUint32LE() & 0xFFFF;
	_worldStats->_boundingRect.right  = stream->readUint32LE() & 0xFFFF;
	_worldStats->_boundingRect.bottom = stream->readUint32LE() & 0xFFFF;

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
    _worldStats->_commonRes.field_54           = stream->readUint32LE();
    _worldStats->_commonRes.field_58           = stream->readUint32LE();
    _worldStats->_commonRes.field_5C           = stream->readUint32LE();
    _worldStats->_commonRes.palette            = stream->readUint32LE();
    _worldStats->_commonRes.cellShadeMask1     = stream->readUint32LE();
    _worldStats->_commonRes.cellShadeMask2     = stream->readUint32LE();
    _worldStats->_commonRes.cellShadeMask3     = stream->readUint32LE();
    _worldStats->_commonRes.unused             = stream->readUint32LE();
    _worldStats->_commonRes.smallCurUp         = stream->readUint32LE();
    _worldStats->_commonRes.smallCurDown       = stream->readUint32LE();
    _worldStats->_commonRes.field_7C           = stream->readUint32LE();

    _worldStats->_width              = stream->readUint32LE();
    _worldStats->_height             = stream->readUint32LE();
    _worldStats->_field_88           = stream->readUint32LE();
    _worldStats->_field_8C           = stream->readUint32LE();
    _worldStats->_numActions         = stream->readUint32LE();
    _worldStats->_numBarriers        = stream->readUint32LE();
    _worldStats->_field_98           = stream->readUint32LE();
    _worldStats->_field_9C           = stream->readUint32LE();
    _worldStats->_field_A0           = stream->readUint32LE();
    _worldStats->_field_A4           = stream->readUint32LE();
    _worldStats->_field_A8           = stream->readUint32LE();
    _worldStats->_field_AC           = stream->readUint32LE();
    _worldStats->_field_B0           = stream->readUint32LE();
    _worldStats->_numActors          = stream->readUint32LE();   
    _worldStats->_stereoReversedFlag = stream->readUint32LE();

    for(int r=0; r < 6; r++) {
        _worldStats->_sceneRects[r].left   = stream->readUint32LE() & 0xFFFF;
	    _worldStats->_sceneRects[r].top    = stream->readUint32LE() & 0xFFFF;
	    _worldStats->_sceneRects[r].right  = stream->readUint32LE() & 0xFFFF;
	    _worldStats->_sceneRects[r].bottom = stream->readUint32LE() & 0xFFFF;
    }
    _worldStats->_sceneRectIdx  = stream->readByte();
    _worldStats->_field_11D[0]  = stream->readByte();
    _worldStats->_field_11D[1]  = stream->readByte();
    _worldStats->_field_11D[2]  = stream->readByte();

    _worldStats->_field_120     = stream->readUint32LE();
    _worldStats->_actionListIdx = stream->readUint32LE();

    for(int gr=0; gr < 100; gr++) {
        _worldStats->_grResId[gr] = stream->readUint32LE();
    }

    _worldStats->_sceneTitleGrResId  = stream->readUint32LE();
    _worldStats->_sceneTitlePalResId = stream->readUint32LE();

    // FIXME Jump unknown sound resource list
    // FIXME Jump resource list definitions

    stream->seek(0x6FA); // where actors definitions start

    for (uint32 a = 0; a < _worldStats->_numBarriers; a++) {
        BarrierItem barrier;
		int i;

		barrier.id    = stream->readUint32LE();
		barrier.resId = stream->readUint32LE();
		barrier.x     = stream->readUint32LE();
		barrier.y     = stream->readUint32LE();

		barrier.boundingRect.left   = stream->readUint32LE() & 0xFFFF;
		barrier.boundingRect.top    = stream->readUint32LE() & 0xFFFF;
		barrier.boundingRect.right  = stream->readUint32LE() & 0xFFFF;
		barrier.boundingRect.bottom = stream->readUint32LE() & 0xFFFF;

		barrier.field_20   = stream->readUint32LE();
		barrier.frameIdx   = stream->readUint32LE();
		barrier.frameCount = stream->readUint32LE();
		barrier.field_2C   = stream->readUint32LE();
		barrier.field_30   = stream->readUint32LE();
		barrier.field_34   = stream->readUint32LE();
		barrier.flags      = stream->readUint32LE();
		barrier.field_3C   = stream->readUint32LE();

		stream->read(barrier.name, sizeof(barrier.name));

		barrier.field_74 = stream->readUint32LE();
		barrier.field_78 = stream->readUint32LE();
		barrier.field_7C = stream->readUint32LE();
		barrier.field_80 = stream->readUint32LE();
		barrier.polyIdx  = stream->readUint32LE();
		barrier.flags2   = stream->readUint32LE();

		for (i = 0; i < 10; i++)
			barrier.gameFlags[i] = stream->readUint32LE();

		barrier.field_B4      = stream->readUint32LE();
		barrier.tickCount     = stream->readUint32LE();
		barrier.tickCount2    = stream->readUint32LE();
		barrier.field_C0      = stream->readUint32LE();
		barrier.field_C4      = stream->readUint32LE();
		barrier.actionListIdx = stream->readUint32LE();

		for (i = 0; i < 16; i++) {
			barrier.soundItems[i].resId   = stream->readUint32LE();
			barrier.soundItems[i].field_4 = stream->readUint32LE();
			barrier.soundItems[i].field_8 = stream->readUint32LE();
			barrier.soundItems[i].field_C = stream->readUint32LE();

		}

		for (i = 0; i < 50; i++) {
			barrier.frameSoundItems[i].resId    = stream->readUint32LE();
			barrier.frameSoundItems[i].frameIdx = stream->readUint32LE();
			barrier.frameSoundItems[i].index    = stream->readUint32LE();
			barrier.frameSoundItems[i].field_C  = stream->readUint32LE();
			barrier.frameSoundItems[i].field_10 = stream->readUint32LE();
			barrier.frameSoundItems[i].field_14 = stream->readUint32LE();
		}

		barrier.field_67C = stream->readUint32LE();
		barrier.soundX    = stream->readUint32LE();
		barrier.soundY    = stream->readUint32LE();
		barrier.field_688 = stream->readUint32LE();

		for (i = 0; i < 5; i++) {
			barrier.field_68C[i] = stream->readUint32LE();
		}

		barrier.soundResId = stream->readUint32LE();
		barrier.field_6A4  = stream->readUint32LE();

        _worldStats->_barriers.push_back(barrier);
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

    // FIXME Figure out all the actions definitions
    for (uint32 a = 0; a < _worldStats->_numActions; a++) {
        ActionItem action;
        memset(&action, 0, sizeof(ActionItem));

        stream->read(action.name,52);
        action.id             = stream->readUint32LE();
        stream->skip(0x14);
        action.actionListIdx1 = stream->readUint32LE();
        action.actionListIdx2 = stream->readUint32LE();
        action.actionType     = stream->readUint32LE();
        stream->skip(0x2C);
        action.polyIdx        = stream->readUint32LE();
        stream->skip(0x08);
        action.soundResId     = stream->readUint32LE();
        stream->skip(0x04);
        action.palCorrection  = stream->readUint32LE();
        stream->skip(0x14);
        action.soundVolume    = stream->readUint32LE();

        _worldStats->_actions.push_back(action);
    }
}

void SceneResource::loadGamePolygons(Common::SeekableReadStream *stream) {
    _gamePolygons = new GamePolygons;

    stream->seek(0xE8686); // jump to game Polygons data

    _gamePolygons->_size       = stream->readUint32LE();
    _gamePolygons->_numEntries = stream->readUint32LE();

    for (uint32 g = 0; g < _gamePolygons->_numEntries; g++) {
        PolyDefinitions poly;
        memset(&poly, 0, sizeof(PolyDefinitions));

        poly.numPoints = stream->readUint32LE();
        if (poly.numPoints > 0)
            poly.points = new Common::Point[poly.numPoints];
        for (uint32 i = 0; i < poly.numPoints; i++) {
			poly.points[i].y = stream->readUint32LE() & 0xFFFF;
			poly.points[i].x = stream->readUint32LE() & 0xFFFF;
		}
        stream->skip((Polygons_MAXSIZE - poly.numPoints) * 8);

        poly.boundingRect.left   = stream->readUint32LE() & 0xFFFF;
        poly.boundingRect.top    = stream->readUint32LE() & 0xFFFF;
        poly.boundingRect.right  = stream->readUint32LE() & 0xFFFF;
        poly.boundingRect.bottom = stream->readUint32LE() & 0xFFFF;

        _gamePolygons->_Polygons.push_back(poly);
    }
}

// FIXME: load necessary Action List content
void SceneResource::loadActionList(Common::SeekableReadStream *stream) {
    _actionList = new ActionList;

    stream->seek(0xE868E + _gamePolygons->_size * _gamePolygons->_numEntries); // jump to action list data

    _actionList->_size       = stream->readUint32LE();
    _actionList->_numEntries = stream->readUint32LE();

    for (uint32 a = 0; a < _actionList->_numEntries; a++) {
        ActionDefinitions action;
        memset(&action, 0, sizeof(ActionDefinitions));

        for (uint32 c = 0; c < Commands_MAXSIZE; c++) {
            ActionCommand command;
            memset(&command, 0, sizeof(ActionCommand));

            command.unknown = stream->readUint32LE();
            command.opcode  = stream->readUint32LE(); // command type
            command.param1  = stream->readUint32LE(); // command parameters
            command.param2  = stream->readUint32LE();
            command.param3  = stream->readUint32LE();
            command.param4  = stream->readUint32LE();
            command.param5  = stream->readUint32LE();
            command.param6  = stream->readUint32LE();
            command.param7  = stream->readUint32LE();
            command.param8  = stream->readUint32LE();
            command.param9  = stream->readUint32LE();

            action.commands[c] = command;
        }

        action.field_1BAC = stream->readUint32LE();
        action.field_1BB0 = stream->readUint32LE();
        action.counter    = stream->readUint32LE();

        _actionList->_Actions.push_back(action);
    }
}

Common::String SceneResource::parseFilename(uint8 sceneIdx) {
	char filename[10];
	sprintf(filename, SCENEMASK, sceneIdx);

	return Common::String(filename);
}

} // end of namespace Asylum
