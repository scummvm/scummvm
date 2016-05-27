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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "mads/mads.h"
#include "mads/compression.h"
#include "mads/resources.h"
#include "mads/scene.h"
#include "mads/dragonsphere/game_dragonsphere.h"
#include "mads/dragonsphere/dragonsphere_scenes.h"
#include "mads/dragonsphere/dragonsphere_scenes1.h"

namespace MADS {

namespace Dragonsphere {

SceneLogic *SceneFactory::createScene(MADSEngine *vm) {
	Scene &scene = vm->_game->_scene;

	// TODO
	//scene.addActiveVocab(NOUN_DROP);

	switch (scene._nextSceneId) {
	// Scene group #1 (Castle, river and caves)
	case 101:	// king's bedroom
		return new Scene101(vm);
	case 102:	// queen's bedroom
		return new Scene102(vm);
	case 103:	// outside king's bedroom
		return new Scene103(vm);
	case 104:	// fireplace / bookshelf
		return new Scene104(vm);
	case 105:	// dining room
		return new Scene105(vm);
	case 106:	// throne room
		return new DummyScene(vm);	// TODO
	case 107:	// council chamber
		return new DummyScene(vm);	// TODO
	case 108:	// dungeon, cell entrance
		return new DummyScene(vm);	// TODO
	case 109:	// cell
		return new DummyScene(vm);	// TODO
	case 110:	// outside castle, merchants and well
		return new DummyScene(vm);	// TODO
	case 111:	// Dragonsphere closeup
		return new DummyScene(vm);	// TODO
	case 112:	// well descend
		return new DummyScene(vm);	// TODO
	case 113:	// bottom of well, river and trap door
		return new DummyScene(vm);	// TODO
	case 114:	// cave
		return new DummyScene(vm);	// TODO
	case 115:	// cave with passageway to west
		return new DummyScene(vm);	// TODO
	case 116:	// cave with pedestral
		return new DummyScene(vm);	// TODO
	case 117:	// river
		return new DummyScene(vm);	// TODO
	case 118:	// castle courtyard and gate
		return new DummyScene(vm);	// TODO
	case 119:	// castle stairs
		return new DummyScene(vm);	// TODO
	case 120:	// map
		return new DummyScene(vm);	// TODO

	// Scene group #2 (Slathan ni Patan, land of shapeshifters)
	case 201:	// guardhouse, entrance to Slathan ni Patan
		return new DummyScene(vm);	// TODO
	case 203:	// forest
		return new DummyScene(vm);	// TODO
	case 204:	// cave
		return new DummyScene(vm);	// TODO
	case 205:	// outside village
		return new DummyScene(vm);	// TODO
	case 206:	// village
		return new DummyScene(vm);	// TODO

	// Scene group #3 (Brynn-Fann, Land of faeries)
	case 301:	// maze entrance
		return new DummyScene(vm);	// TODO
	case 302:	// maze
		return new DummyScene(vm);	// TODO
	case 303:	// toads
		return new DummyScene(vm);	// TODO

	// Scene group #4 (The Desert)
	case 401:	// desert
		return new DummyScene(vm);	// TODO
	case 402:	// desert
		return new DummyScene(vm);	// TODO
	case 403:	// desert
		return new DummyScene(vm);	// TODO
	case 404:	// desert with dunes
		return new DummyScene(vm);	// TODO
	case 405:	// oasis
		return new DummyScene(vm);	// TODO
	case 406:	// inside tent
		return new DummyScene(vm);	// TODO
	case 407:	// gem sack closeup
		return new DummyScene(vm);	// TODO
	case 408:	// spirit plane
		return new DummyScene(vm);	// TODO
	case 409:	// spirit plane top down view, disks
		return new DummyScene(vm);	// TODO
	case 410:	// snake pit and spirit tree
		return new DummyScene(vm);	// TODO
	case 411:	// nest
		return new DummyScene(vm);	// TODO
	case 412:	// desert
		return new DummyScene(vm);	// TODO
	case 454:	// cutscene
		return new DummyScene(vm);	// TODO

	// Scene group #5 (The Mountain)
	case 501:	// base of mountain / wall
		return new DummyScene(vm);	// TODO
	case 502:	// base of mountain
		return new DummyScene(vm);	// TODO
	case 503:	// waterfall
		return new DummyScene(vm);	// TODO
	case 504:	// hermit's cave
		return new DummyScene(vm);	// TODO
	case 505:	// rock trees
		return new DummyScene(vm);	// TODO
	case 506:	// nest
		return new DummyScene(vm);	// TODO
	case 507:	// above nest
		return new DummyScene(vm);	// TODO
	case 508:	// ledge, right
		return new DummyScene(vm);	// TODO
	case 509:	// ledge, left
		return new DummyScene(vm);	// TODO
	case 510:	// nest top down view and pillars
		return new DummyScene(vm);	// TODO
	case 511:	// pillars
		return new DummyScene(vm);	// TODO
	case 512:	// nest
		return new DummyScene(vm);	// TODO
	case 557:	// cutscene
		return new DummyScene(vm);	// TODO

	// Scene group #6 (The Tower)
	case 601:	// ??? (tile count mismatch)
		return new DummyScene(vm);	// TODO
	case 603:	// eye chamber
		return new DummyScene(vm);	// TODO
	case 604:	// room of magic
		return new DummyScene(vm);	// TODO
	case 605:	// science room
		return new DummyScene(vm);	// TODO
	case 606:	// doorway
		return new DummyScene(vm);	// TODO
	case 607:	// prison
		return new DummyScene(vm);	// TODO
	case 609:	// stone pedestral chamber
		return new DummyScene(vm);	// TODO
	case 612:	// infernal machine room
		return new DummyScene(vm);	// TODO
	case 613:	// room with lava floor
		return new DummyScene(vm);	// TODO
	case 614:	// sorcerer's room
		return new DummyScene(vm);	// TODO

	default:
		error("Invalid scene %d called", scene._nextSceneId);
	}
}

/*------------------------------------------------------------------------*/

DragonsphereScene::DragonsphereScene(MADSEngine *vm) : SceneLogic(vm),
		_globals(static_cast<GameDragonsphere *>(vm->_game)->_globals),
		_game(*static_cast<GameDragonsphere *>(vm->_game)),
		_action(vm->_game->_scene._action) {
}

Common::String DragonsphereScene::formAnimName(char sepChar, int suffixNum) {
	return Resources::formatName(_scene->_currentSceneId, sepChar, suffixNum,
		EXT_NONE, "");
}

/*------------------------------------------------------------------------*/

void SceneInfoDragonsphere::loadCodes(BaseSurface &depthSurface, int variant) {
	Common::String ext = Common::String::format(".WW%d", variant);
	Common::String fileName = Resources::formatName(RESPREFIX_RM, _sceneId, ext);
	if (!Common::File::exists(fileName))
		return;

	File f(fileName);
	MadsPack codesPack(&f);
	Common::SeekableReadStream *stream = codesPack.getItemStream(0);

	loadCodes(depthSurface, stream);

	delete stream;
	f.close();
}

void SceneInfoDragonsphere::loadCodes(BaseSurface &depthSurface, Common::SeekableReadStream *stream) {
	byte *destP = (byte *)depthSurface.getPixels();
	byte *walkMap = new byte[stream->size()];
	stream->read(walkMap, stream->size());

	for (int y = 0; y < 156; ++y) {
		for (int x = 0; x < 320; ++x) {
			int offset = x + (y * 320);
			if ((walkMap[offset / 8] << (offset % 8)) & 0x80)
				*destP++ = 1;		// walkable
			else
				*destP++ = 0;
		}
	}

	delete[] walkMap;
}

} // End of namespace Dragonsphere

} // End of namespace MADS
