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

#ifdef ENABLE_MADSV2

#include "common/config-manager.h"
#include "mads/mads.h"
#include "mads/compression.h"
#include "mads/resources.h"
#include "mads/scene.h"
#include "mads/forest/game_forest.h"
#include "mads/forest/forest_scenes.h"

namespace MADS {

namespace Forest {

SceneLogic *SceneFactory::createScene(MADSEngine *vm) {
	Scene &scene = vm->_game->_scene;

	// TODO
	//scene.addActiveVocab(NOUN_DROP);

	switch (scene._nextSceneId) {
	// Scene group #1 (Cornelius's House inside/outside)
	case 101:	// Cornelius's House inside
		return new DummyScene(vm);  // TODO
	case 103:	// Forest, tree
		return new DummyScene(vm);  // TODO
	case 104:	// Bedroom
		return new DummyScene(vm);  // TODO
	case 106:	// Cornelius's House outside
		return new DummyScene(vm);	// TODO
	case 107:	// Tree base
		return new DummyScene(vm);	// TODO
	case 199:	// Book
		return new DummyScene(vm);	// TODO

	// Scene group #2 (Dapplewood Forest Areas 1-4)
	case 201:	// 
		return new DummyScene(vm);	// TODO
	case 203:	// 
		return new DummyScene(vm);	// TODO
	case 204:	// 
		return new DummyScene(vm);	// TODO
	case 205:	// 
		return new DummyScene(vm);	// TODO
	case 210:	// 
		return new DummyScene(vm);	// TODO
	case 211:	// 
		return new DummyScene(vm);	// TODO
	case 220:	// 
		return new DummyScene(vm);	// TODO
	case 221:	// 
		return new DummyScene(vm);	// TODO

	// Scene group #3 (Dapplewood Nexus)
	case 301:	// 
		return new DummyScene(vm);	// TODO
	case 302:	// 
		return new DummyScene(vm);	// TODO
	case 303:	// 
		return new DummyScene(vm);	// TODO
	case 304:	// 
		return new DummyScene(vm);	// TODO
	case 305:	// 
		return new DummyScene(vm);	// TODO
	case 306:	// 
		return new DummyScene(vm);	// TODO
	case 307:	// 
		return new DummyScene(vm);	// TODO
	case 308:	//
		return new DummyScene(vm);	// TODO
	case 321:	//
		return new DummyScene(vm);	// TODO
	case 322:	//
		return new DummyScene(vm);	// TODO

	// Scene group #4 (Yellow Dragons' Lair)
	case 401:	// 
		return new DummyScene(vm);	// TODO
	case 402:	// 
		return new DummyScene(vm);	// TODO
	case 403:	// 
		return new DummyScene(vm);	// TODO
	case 404:	// 
		return new DummyScene(vm);	// TODO
	case 405:	// 
		return new DummyScene(vm);	// TODO
	case 420:	// 
		return new DummyScene(vm);	// TODO

	// Scene group #5 (Meadow & Backtracking)
	case 501:	// 
		return new DummyScene(vm);	// TODO
	case 503:	// 
		return new DummyScene(vm);	// TODO
	case 509:	// 
		return new DummyScene(vm);	// TODO
	case 510:	// 
		return new DummyScene(vm);	// TODO
	case 520:	// 
		return new DummyScene(vm);	// TODO

	default:
		error("Invalid scene %d called", scene._nextSceneId);
	}
}

/*------------------------------------------------------------------------*/

ForestScene::ForestScene(MADSEngine *vm) : SceneLogic(vm),
		_globals(static_cast<GameForest *>(vm->_game)->_globals),
		_game(*static_cast<GameForest *>(vm->_game)),
		_action(vm->_game->_scene._action) {
}

Common::Path ForestScene::formAnimName(char sepChar, int suffixNum) {
	return Resources::formatName(_scene->_currentSceneId, sepChar, suffixNum,
		EXT_NONE, "");
}

/*------------------------------------------------------------------------*/

void SceneInfoForest::loadCodes(BaseSurface &depthSurface, int variant) {
	Common::String ext = Common::String::format(".WW%d", variant);
	Common::Path fileName = Resources::formatName(RESPREFIX_RM, _sceneId, ext);
	if (!Common::File::exists(fileName))
		return;

	File f(fileName);
	MadsPack codesPack(&f);
	Common::SeekableReadStream *stream = codesPack.getItemStream(0);

	loadCodes(depthSurface, stream);

	delete stream;
	f.close();
}

void SceneInfoForest::loadCodes(BaseSurface &depthSurface, Common::SeekableReadStream *stream) {
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

} // End of namespace Forest

} // End of namespace MADS

#endif
