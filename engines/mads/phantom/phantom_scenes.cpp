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
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "mads/mads.h"
#include "mads/compression.h"
#include "mads/resources.h"
#include "mads/scene.h"
#include "mads/phantom/game_phantom.h"
#include "mads/phantom/phantom_scenes.h"

namespace MADS {

namespace Phantom {

SceneLogic *SceneFactory::createScene(MADSEngine *vm) {
	Scene &scene = vm->_game->_scene;

	// TODO
	//scene.addActiveVocab(NOUN_DROP);

	switch (scene._nextSceneId) {
	// Scene group #1 (theater, stage and dressing rooms)
	case 101:	// seats
		return new DummyScene(vm);	// TODO
	case 102:	// music stands
		return new DummyScene(vm);	// TODO
	case 103:	// below stage
		return new DummyScene(vm);	// TODO
	case 104:	// stage
		return new DummyScene(vm);	// TODO
	case 105:	// ground floor, storage room
		return new DummyScene(vm);	// TODO
	case 106:	// behind stage
		return new DummyScene(vm);	// TODO
	case 107:	// stage right wing
		return new DummyScene(vm);	// TODO
	case 108:	// stage left wing
		return new DummyScene(vm);	// TODO
	case 109:	// upper floor, staircase
		return new DummyScene(vm);	// TODO
	case 110:	// outside dressing rooms 1
		return new DummyScene(vm);	// TODO
	case 111:	// outside dressing rooms 2
		return new DummyScene(vm);	// TODO
	case 112:	// inside dressing room 1
		return new DummyScene(vm);	// TODO
	case 113:	// inside dressing room 2
		return new DummyScene(vm);	// TODO
	case 114:	// lower floor, storage room
		return new DummyScene(vm);	// TODO
	case 150:	// cutscene
		return new DummyScene(vm);	// TODO

	// Scene group #2 (theater entrance, offices, balcony)
	case 201:	// entrance / ticket office
		return new DummyScene(vm);	// TODO
	case 202:	// outside offices / paintings room
		return new DummyScene(vm);	// TODO
	case 203:	// office
		return new DummyScene(vm);	// TODO
	case 204:	// library
		return new DummyScene(vm);	// TODO
	case 205:	// upper floor, outside balcony boxes
		return new DummyScene(vm);	// TODO
	case 206:	// balcony box #1
		return new DummyScene(vm);	// TODO
	case 207:	// balcony box #2
		return new DummyScene(vm);	// TODO
	case 208:	// stage and balcony view
		return new DummyScene(vm);	// TODO
	case 250:	// cutscene
		return new DummyScene(vm);	// TODO

	// Scene group #3 (catwalks, chandelier, lake / catacombs entrance)
	case 301:	// catwalk #1 above stage
		return new DummyScene(vm);	// TODO
	case 302:	// catwalk #2 above stage
		return new DummyScene(vm);	// TODO
	case 303:	// above chandelier
		return new DummyScene(vm);	// TODO
	case 304:	// chandelier
		return new DummyScene(vm);	// TODO
	case 305:	// chandelier fight, phantom closeup
		return new DummyScene(vm);	// TODO
	case 306:	// chandelier #2
		return new DummyScene(vm);	// TODO
	case 307:	// catwalk #3 above stage
		return new DummyScene(vm);	// TODO
	case 308:	// hidden staircase behind balcony box
		return new DummyScene(vm);	// TODO
	case 309:	// lake and archway
		return new DummyScene(vm);	// TODO
	case 310:	// lake
		return new DummyScene(vm);	// TODO

	// Scene group #4 (labyrinth)
	case 401:	// labyrinth room, 3 exits
		return new DummyScene(vm);	// TODO
	case 403:	// labyrinth room (big), 4 exits + 1 bricked door, left
		return new DummyScene(vm);	// TODO
	case 404:	// labyrinth room, 3 exits
		return new DummyScene(vm);	// TODO
	case 406:	// labyrinth room, 2 exits
		return new DummyScene(vm);	// TODO
	case 407:	// catacomb room / lake
		return new DummyScene(vm);	// TODO
	case 408:	// catacomb corridor
		return new DummyScene(vm);	// TODO
	case 409:	// catacomb room, door with switch panel
		return new DummyScene(vm);	// TODO
	case 410:	// skull switch panel
		return new DummyScene(vm);	// TODO
	case 453:	// Labyrinth room (big), 4 exits + 1 bricked door, right
		return new DummyScene(vm);	// TODO
	case 456:	// Labyrinth room, 2 exits
		return new DummyScene(vm);	// TODO

	// Scene group #5 (Phantom's hideout)
	case 501:	// catacombs, outside phantom's hideout, lake and boat
		return new DummyScene(vm);	// TODO
	case 502:	// push panel trap
		return new DummyScene(vm);	// TODO
	case 504:	// Phantom's hideout, church organ
		return new DummyScene(vm);	// TODO
	case 505:	// Phantom's hideout, sarcophagus
		return new DummyScene(vm);	// TODO
	case 506:	// catacomb room with ramp
		return new DummyScene(vm);	// TODO

	default:
		error("Invalid scene %d called", scene._nextSceneId);
	}
}

/*------------------------------------------------------------------------*/

PhantomScene::PhantomScene(MADSEngine *vm) : SceneLogic(vm),
		_globals(static_cast<GamePhantom *>(vm->_game)->_globals),
		_game(*static_cast<GamePhantom *>(vm->_game)),
		_action(vm->_game->_scene._action) {
}

Common::String PhantomScene::formAnimName(char sepChar, int suffixNum) {
	return Resources::formatName(_scene->_currentSceneId, sepChar, suffixNum,
		EXT_NONE, "");
}

/*------------------------------------------------------------------------*/

void SceneInfoPhantom::loadCodes(MSurface &depthSurface, int variant) {
	File f(Resources::formatName(RESPREFIX_RM, _sceneId, ".DAT"));
	MadsPack codesPack(&f);
	Common::SeekableReadStream *stream = codesPack.getItemStream(variant + 1);

	loadCodes(depthSurface, stream);

	delete stream;
	f.close();
}

void SceneInfoPhantom::loadCodes(MSurface &depthSurface, Common::SeekableReadStream *stream) {
	byte *destP = depthSurface.getData();
	byte *endP = depthSurface.getBasePtr(0, depthSurface.h);

	byte runLength = stream->readByte();
	while (destP < endP && runLength > 0) {
		byte runValue = stream->readByte();

		// Write out the run length
		Common::fill(destP, destP + runLength, runValue);
		destP += runLength;

		// Get the next run length
		runLength = stream->readByte();
	}

	if (destP < endP)
		Common::fill(destP, endP, 0);
}

} // End of namespace Phantom

} // End of namespace MADS
