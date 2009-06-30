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
    for(uint i=0; i< _gamePolygons->numEntries; i++) {
        delete[] _gamePolygons->polygons[i].points;
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

    _worldStats->size       = stream->readUint32LE();
    _worldStats->numEntries = stream->readUint32LE();

    _worldStats->numChapter          = stream->readUint32LE();
    _worldStats->xLeft               = stream->readUint32LE();
    _worldStats->yTop                = stream->readUint32LE();
    _worldStats->boundingRect.left   = stream->readUint32LE() & 0xFFFF;
	_worldStats->boundingRect.top    = stream->readUint32LE() & 0xFFFF;
	_worldStats->boundingRect.right  = stream->readUint32LE() & 0xFFFF;
	_worldStats->boundingRect.bottom = stream->readUint32LE() & 0xFFFF;

    // read common graphic resources
	_worldStats->commonRes.backgroundImage    = stream->readUint32LE();
	_worldStats->commonRes.curScrollUp        = stream->readUint32LE();
    _worldStats->commonRes.curScrollUpLeft    = stream->readUint32LE();
    _worldStats->commonRes.curScrollLeft      = stream->readUint32LE();
    _worldStats->commonRes.curScrollDownLeft  = stream->readUint32LE();
    _worldStats->commonRes.curScrollDown      = stream->readUint32LE();
    _worldStats->commonRes.curScrollDownRight = stream->readUint32LE();
    _worldStats->commonRes.curScrollRight     = stream->readUint32LE();
    _worldStats->commonRes.curScrollUpRight   = stream->readUint32LE();
    _worldStats->commonRes.curHand            = stream->readUint32LE();
    _worldStats->commonRes.curMagnifyingGlass = stream->readUint32LE();
    _worldStats->commonRes.curTalkNCP         = stream->readUint32LE();
    _worldStats->commonRes.curGrabPointer     = stream->readUint32LE();
    _worldStats->commonRes.curTalkNCP2        = stream->readUint32LE();
    _worldStats->commonRes.field_54           = stream->readUint32LE();
    _worldStats->commonRes.field_58           = stream->readUint32LE();
    _worldStats->commonRes.field_5C           = stream->readUint32LE();
    _worldStats->commonRes.palette            = stream->readUint32LE();
    _worldStats->commonRes.cellShadeMask1     = stream->readUint32LE();
    _worldStats->commonRes.cellShadeMask2     = stream->readUint32LE();
    _worldStats->commonRes.cellShadeMask3     = stream->readUint32LE();
    _worldStats->commonRes.unused             = stream->readUint32LE();
    _worldStats->commonRes.smallCurUp         = stream->readUint32LE();
    _worldStats->commonRes.smallCurDown       = stream->readUint32LE();
    _worldStats->commonRes.field_7C           = stream->readUint32LE();

    _worldStats->width              = stream->readUint32LE();
    _worldStats->height             = stream->readUint32LE();
    _worldStats->field_88           = stream->readUint32LE();
    _worldStats->field_8C           = stream->readUint32LE();
    _worldStats->numActions         = stream->readUint32LE();
    _worldStats->numBarriers        = stream->readUint32LE();
    _worldStats->field_98           = stream->readUint32LE();
    _worldStats->field_9C           = stream->readUint32LE();
    _worldStats->field_A0           = stream->readUint32LE();
    _worldStats->field_A4           = stream->readUint32LE();
    _worldStats->field_A8           = stream->readUint32LE();
    _worldStats->field_AC           = stream->readUint32LE();
    _worldStats->field_B0           = stream->readUint32LE();
    _worldStats->numActors          = stream->readUint32LE();
    _worldStats->stereoReversedFlag = stream->readUint32LE();

    for(int r=0; r < 6; r++) {
        _worldStats->sceneRects[r].left   = stream->readUint32LE() & 0xFFFF;
	    _worldStats->sceneRects[r].top    = stream->readUint32LE() & 0xFFFF;
	    _worldStats->sceneRects[r].right  = stream->readUint32LE() & 0xFFFF;
	    _worldStats->sceneRects[r].bottom = stream->readUint32LE() & 0xFFFF;
    }
    _worldStats->sceneRectIdx  = stream->readByte();
    _worldStats->field_11D[0]  = stream->readByte();
    _worldStats->field_11D[1]  = stream->readByte();
    _worldStats->field_11D[2]  = stream->readByte();

    _worldStats->field_120     = stream->readUint32LE();
    _worldStats->actionListIdx = stream->readUint32LE();

    for(int gr=0; gr < 100; gr++) {
        _worldStats->grResId[gr] = stream->readUint32LE();
    }

    _worldStats->sceneTitleGrResId  = stream->readUint32LE();
    _worldStats->sceneTitlePalResId = stream->readUint32LE();
    _worldStats->actorType          = stream->readUint32LE();

    for(int s=0; s < 50; s++) {
        _worldStats->soundResId[s] = stream->readUint32LE();
    }

    for(int s=0; s < 15; s++) {
        _worldStats->ambientSounds[s].field_0  = stream->readUint32LE();
        _worldStats->ambientSounds[s].flags    = stream->readUint32LE();
        _worldStats->ambientSounds[s].resId    = stream->readUint32LE();
        _worldStats->ambientSounds[s].field_C = stream->readUint32LE();
        _worldStats->ambientSounds[s].field_10 = stream->readUint32LE();
        _worldStats->ambientSounds[s].field_14 = stream->readUint32LE();

        for(int i=0; i < 6; i++)
            _worldStats->ambientSounds[s].flagNum[i] = stream->readUint32LE();

        _worldStats->ambientSounds[s].x = stream->readUint32LE();
        _worldStats->ambientSounds[s].y = stream->readUint32LE();
    }
    _worldStats->numAmbientSound   = stream->readUint32LE();

    _worldStats->musicStatus       = stream->readUint32LE();
    _worldStats->musicCurrentResId = stream->readUint32LE();
    _worldStats->musicFlag         = stream->readUint32LE();
    _worldStats->musicResId        = stream->readUint32LE();
    _worldStats->musicStatusExt    = stream->readUint32LE();

    for (uint32 a = 0; a < _worldStats->numBarriers; a++) {
        int i;
        BarrierItem barrier;

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

        _worldStats->barriers.push_back(barrier);
    }

    // need to jump all unused barriers data to where actors data start
    stream->seek(0xA6D7A);

    for (uint32 a = 0; a < _worldStats->numActors; a++) {
        int i;
        ActorItem actor;
        memset(&actor, 0, sizeof(ActorItem));

        actor.x0         = stream->readUint32LE();
        actor.y0         = stream->readUint32LE();
        actor.grResId    = stream->readUint32LE();
        actor.field_C    = stream->readUint32LE();
        actor.frameNum   = stream->readUint32LE();
        actor.frameCount = stream->readUint32LE();
        actor.x1         = stream->readUint32LE();
        actor.y1         = stream->readUint32LE();
        actor.x2         = stream->readUint32LE();
        actor.y2         = stream->readUint32LE();

        actor.boundingRect.left   = stream->readUint32LE() & 0xFFFF;
		actor.boundingRect.top    = stream->readUint32LE() & 0xFFFF;
		actor.boundingRect.right  = stream->readUint32LE() & 0xFFFF;
		actor.boundingRect.bottom = stream->readUint32LE() & 0xFFFF;

        actor.direction  = stream->readUint32LE();
        actor.field_3C   = stream->readUint32LE();
        actor.field_40   = stream->readUint32LE();
        actor.field_44   = stream->readUint32LE();
        actor.field_48   = stream->readUint32LE();
        actor.flags      = stream->readUint32LE();
        actor.field_50   = stream->readUint32LE();
        actor.field_54   = stream->readUint32LE();
        actor.field_58   = stream->readUint32LE();
        actor.field_5C   = stream->readUint32LE();
        actor.field_60   = stream->readUint32LE();
        actor.actionIdx3 = stream->readUint32LE();

        // TODO skip field_68 till field_617
        stream->skip(0x5B0);

        for(i=0; i < 8; i++) {
            actor.reaction[i] = stream->readUint32LE();
        }

        actor.field_638 = stream->readUint32LE();
        actor.field_63C = stream->readUint32LE();
        actor.field_640 = stream->readUint32LE();
        actor.field_644 = stream->readUint32LE();
        actor.field_648 = stream->readUint32LE();
        actor.field_64C = stream->readUint32LE();
        actor.field_650 = stream->readUint32LE();

        for(i=0; i < 55; i++) {
            actor.grResTable[i] = stream->readUint32LE();
        }

        stream->read(actor.name, sizeof(actor.name));

        for(i=0; i < 20; i++) {
            actor.field_830[i] = stream->readUint32LE();
        }
        for(i=0; i < 20; i++) {
            actor.field_880[i] = stream->readUint32LE();
        }
        for(i=0; i < 20; i++) {
            actor.field_8D0[i] = stream->readUint32LE();
        }
        
        actor.actionIdx2 = stream->readUint32LE();
        actor.field_924 = stream->readUint32LE();
        actor.tickValue1 = stream->readUint32LE();
        actor.field_92C = stream->readUint32LE();
        actor.flags2 = stream->readUint32LE();
        actor.field_934 = stream->readUint32LE();
        actor.field_938 = stream->readUint32LE();
        actor.soundResId = stream->readUint32LE();

        // TODO skip field_940 till field_978
        stream->skip(0x3C);

        actor.actionIdx1 = stream->readUint32LE();
        
        // TODO skip field_980 till field_9A0
        stream->skip(0x24);

        _worldStats->actors.push_back(actor);
    }

    // TODO Take this out, it shouldn't be here (TEST ONLY)
    stream->seek(0xA73B6);
    uint8 mainActorData[500];
    stream->read(mainActorData, 500);
    _mainActor = new MainActor(mainActorData);

    stream->seek(0xD6B5A); // where action items start

    // FIXME Figure out all the actions items
    for (uint32 a = 0; a < _worldStats->numActions; a++) {
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

        _worldStats->actions.push_back(action);
    }
}

void SceneResource::loadGamePolygons(Common::SeekableReadStream *stream) {
    _gamePolygons = new GamePolygons;

    stream->seek(0xE8686); // jump to game Polygons data

    _gamePolygons->size       = stream->readUint32LE();
    _gamePolygons->numEntries = stream->readUint32LE();

    for (uint32 g = 0; g < _gamePolygons->numEntries; g++) {
        PolyDefinitions poly;
        memset(&poly, 0, sizeof(PolyDefinitions));

        poly.numPoints = stream->readUint32LE();
        if (poly.numPoints > 0)
            poly.points = new Common::Point[poly.numPoints];
        for (uint32 i = 0; i < poly.numPoints; i++) {
			poly.points[i].x = stream->readUint32LE() & 0xFFFF;
			poly.points[i].y = stream->readUint32LE() & 0xFFFF;
		}
        stream->skip((Polygons_MAXSIZE - poly.numPoints) * 8);

        poly.boundingRect.left   = stream->readUint32LE() & 0xFFFF;
        poly.boundingRect.top    = stream->readUint32LE() & 0xFFFF;
        poly.boundingRect.right  = stream->readUint32LE() & 0xFFFF;
        poly.boundingRect.bottom = stream->readUint32LE() & 0xFFFF;

        _gamePolygons->polygons.push_back(poly);
    }
}

void SceneResource::loadActionList(Common::SeekableReadStream *stream) {
    _actionList = new ActionList;

    stream->seek(0xE868E + _gamePolygons->size * _gamePolygons->numEntries); // jump to action list data

    _actionList->size       = stream->readUint32LE();
    _actionList->numEntries = stream->readUint32LE();

    for (uint32 a = 0; a < _actionList->numEntries; a++) {
        ActionDefinitions action;
        memset(&action, 0, sizeof(ActionDefinitions));

        for (uint32 c = 0; c < Commands_MAXSIZE; c++) {
            ActionCommand command;
            memset(&command, 0, sizeof(ActionCommand));

            command.numLines = stream->readUint32LE();
            command.opcode   = stream->readUint32LE(); // command type
            command.param1   = stream->readUint32LE(); // command parameters
            command.param2   = stream->readUint32LE();
            command.param3   = stream->readUint32LE();
            command.param4   = stream->readUint32LE();
            command.param5   = stream->readUint32LE();
            command.param6   = stream->readUint32LE();
            command.param7   = stream->readUint32LE();
            command.param8   = stream->readUint32LE();
            command.param9   = stream->readUint32LE();

            action.commands[c] = command;
        }

        action.field_1BAC = stream->readUint32LE();
        action.field_1BB0 = stream->readUint32LE();
        action.counter    = stream->readUint32LE();

        _actionList->actions.push_back(action);
    }
}

Common::String SceneResource::parseFilename(uint8 sceneIdx) {
	char filename[10];
	sprintf(filename, SCENEMASK, sceneIdx);

	return Common::String(filename);
}

} // end of namespace Asylum
