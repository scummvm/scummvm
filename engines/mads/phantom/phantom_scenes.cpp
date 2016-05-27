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
#include "mads/phantom/game_phantom.h"
#include "mads/phantom/phantom_scenes.h"
#include "mads/phantom/phantom_scenes1.h"
#include "mads/phantom/phantom_scenes2.h"
#include "mads/phantom/phantom_scenes3.h"
#include "mads/phantom/phantom_scenes4.h"
#include "mads/phantom/phantom_scenes5.h"

namespace MADS {

namespace Phantom {

SceneLogic *SceneFactory::createScene(MADSEngine *vm) {
	Scene &scene = vm->_game->_scene;

	// When changing from one section to the other, set the scaling velocity
	if ((scene._nextSceneId / 100) != (scene._priorSceneId / 100))
		vm->_game->_player._scalingVelocity = true;

	switch (scene._nextSceneId) {
	// Scene group #1 (theater, stage and dressing rooms)
	case 101:	// seats
		return new Scene101(vm);
	case 102:	// music stands
		return new Scene102(vm);
	case 103:	// below stage
		return new Scene103(vm);
	case 104:	// stage
		return new Scene104(vm);
	case 105:	// ground floor, storage room
		return new Scene105(vm);
	case 106:	// behind stage
		return new Scene106(vm);
	case 107:	// stage right wing
		return new Scene107(vm);
	case 108:	// stage left wing
		return new Scene108(vm);
	case 109:	// upper floor, staircase
		return new Scene109(vm);
	case 110:	// outside dressing rooms 1
		return new Scene110(vm);
	case 111:	// outside dressing rooms 2
		return new Scene111(vm);
	case 112:	// inside dressing room 1
		return new Scene112(vm);
	case 113:	// inside dressing room 2
		return new Scene113(vm);
	case 114:	// lower floor, storage room
		return new Scene114(vm);
	case 150:	// cutscene
		return new Scene150(vm);

	// Scene group #2 (theater entrance, offices, balcony)
	case 201:	// entrance / ticket office
		return new Scene201(vm);
	case 202:	// outside offices / paintings room
		return new Scene202(vm);
	case 203:	// office
		return new Scene203(vm);
	case 204:	// library
		return new Scene204(vm);
	case 205:	// upper floor, outside balcony boxes
		return new Scene205(vm);
	case 206:	// balcony box #1
		return new Scene206(vm);
	case 207:	// balcony box #2
		return new Scene207(vm);
	case 208:	// stage and balcony view
		return new Scene208(vm);
	case 250:	// cutscene
		return new Scene250(vm);

	// Scene group #3 (catwalks, chandelier, lake / catacombs entrance)
	case 301:	// catwalk #1 above stage
		return new Scene301(vm);
	case 302:	// catwalk #2 above stage
		return new Scene302(vm);
	case 303:	// above chandelier
		return new Scene303(vm);
	case 304:	// chandelier
		return new Scene304(vm);
	case 305:	// chandelier fight, phantom closeup
		return new Scene305(vm);
	case 306:	// chandelier #2
		return new Scene306(vm);
	case 307:	// catwalk #3 above stage
		return new Scene307(vm);
	case 308:	// hidden staircase behind balcony box
		return new Scene308(vm);
	case 309:	// lake and archway
		return new Scene309(vm);
	case 310:	// lake
		return new Scene310(vm);

	// Scene group #4 (labyrinth)
	case 401:	// labyrinth room, 3 exits
		return new Scene401(vm);
	case 403:	// labyrinth room (big), 4 exits + 1 bricked door, left
		return new Scene403(vm);
	case 404:	// labyrinth room, 3 exits
		return new Scene404(vm);
	case 406:	// labyrinth room, 2 exits
		return new Scene406(vm);
	case 407:	// catacomb room / lake
		return new Scene407(vm);
	case 408:	// catacomb corridor
		return new Scene408(vm);
	case 409:	// catacomb room, door with switch panel
		return new Scene409(vm);
	case 410:	// skull switch panel
		return new Scene410(vm);
	case 453:	// Labyrinth room (big), 4 exits + 1 bricked door, right
		return new Scene453(vm);
	case 456:	// Labyrinth room, 2 exits
		return new Scene456(vm);

	// Scene group #5 (Phantom's hideout)
	case 501:	// catacombs, outside phantom's hideout, lake and boat
		return new Scene501(vm);
	case 502:	// push panel trap
		return new Scene502(vm);
	case 504:	// Phantom's hideout, church organ
		return new Scene504(vm);
	case 505:	// Phantom's hideout, sarcophagus
		return new Scene505(vm);
	case 506:	// catacomb room with ramp
		return new Scene506(vm);

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

void SceneInfoPhantom::loadCodes(BaseSurface &depthSurface, int variant) {
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

void SceneInfoPhantom::loadCodes(BaseSurface &depthSurface, Common::SeekableReadStream *stream) {
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

} // End of namespace Phantom

} // End of namespace MADS
