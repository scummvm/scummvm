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
#include "mads/nebular/game_nebular.h"
#include "mads/nebular/nebular_scenes.h"
#include "mads/nebular/nebular_scenes1.h"
#include "mads/nebular/nebular_scenes2.h"
#include "mads/nebular/nebular_scenes3.h"
#include "mads/nebular/nebular_scenes4.h"
#include "mads/nebular/nebular_scenes8.h"

namespace MADS {

namespace Nebular {

SceneLogic *SceneFactory::createScene(MADSEngine *vm) {
	Scene &scene = vm->_game->_scene;

	scene.addActiveVocab(NOUN_DROP);
	scene.addActiveVocab(NOUN_DOLLOP);
	scene.addActiveVocab(NOUN_DASH);
	scene.addActiveVocab(NOUN_SPLASH);
	scene.addActiveVocab(NOUN_ALCOHOL);

	switch (scene._nextSceneId) {
	// Scene group #1
	case 101:
		return new Scene101(vm);
	case 102:
		return new Scene102(vm);
	case 103:
		return new Scene103(vm);
	case 104:
		return new Scene104(vm);
	case 105:
		return new Scene105(vm);
	case 106:
		return new Scene106(vm);
	case 107:
		return new Scene107(vm);
	case 108:
		return new Scene108(vm);
	case 109:
		return new Scene109(vm);
	case 110:
		return new Scene110(vm);
	case 111:
		return new Scene111(vm);
	case 112:
		return new Scene112(vm);

	// Scene group #2
	case 201:
		return new Scene201(vm);
	case 202:
		return new Scene202(vm);
	case 203:
		return new Scene203(vm);
	case 205:
		return new Scene205(vm);
	case 207:
		return new Scene207(vm);
	case 208:
		return new Scene208(vm);
	case 209:
		return new Scene209(vm);
	case 210:
		return new Scene210(vm);
	case 211:
		return new Scene211(vm);
	case 212:
		return new Scene212(vm);

	case 214:
		return new Scene214(vm);
	case 215:
		return new Scene215(vm);
	case 216:
		return new Scene216(vm);

	// Scene group #3
	case 301:
		return new Scene301(vm);
	case 302:
		return new Scene302(vm);
	case 303:
		return new Scene303(vm);
	case 304:
		return new Scene304(vm);
	case 307:
		return new Scene307(vm);
	case 308:
		return new Scene308(vm);
	case 309:
		return new Scene309(vm);
	case 310:
		return new Scene310(vm);
	case 311:
		return new Scene311(vm);
	case 313:
		return new Scene313(vm);
	case 316:
		return new Scene316(vm);
	case 318:
		return new Scene318(vm);
	case 319:
		return new Scene319(vm);
	case 320:
		return new Scene320(vm);
	case 321:
		return new Scene321(vm);
	case 351:
		return new Scene351(vm);
	case 352:
		return new Scene352(vm);
	case 353:
		return new Scene353(vm);
	case 354:
		return new Scene354(vm);
	case 357:
		return new Scene357(vm);
	case 358:
		return new Scene358(vm);
	case 359:
		return new Scene359(vm);
	case 360:
		return new Scene360(vm);
	case 361:
		return new Scene361(vm);
	case 366:
		return new Scene366(vm);
	case 387:
		return new Scene387(vm);
	case 388:
		return new Scene388(vm);
	case 389:
		return new Scene389(vm);
	case 390:
		return new Scene390(vm);
	case 391:
		return new Scene391(vm);
	case 399:
		return new Scene399(vm);

	// Scene group #4
	case 401:
		return new Scene401(vm);
	case 402:
		return new Scene402(vm);
	case 405:
		return new Scene405(vm);
	case 406:
		return new Scene406(vm);
	case 407:
		return new Scene407(vm);
	case 408:
		// TODO
	case 409:
		// TODO
	case 410:
		// TODO
	case 411:
		// TODO
	case 413:
		// TODO
		// Fall-through for unimplemented scenes in group #4
		error("Invalid scene %d called", scene._nextSceneId);

	// Scene group #5
	case 501:
		// TODO
	case 502:
		// TODO
	case 503:
		// TODO
	case 504:
		// TODO
	case 505:
		// TODO
	case 506:
		// TODO
	case 507:
		// TODO
	case 508:
		// TODO
	case 511:
		// TODO
	case 512:
		// TODO
	case 513:
		// TODO
	case 515:
		// TODO
	case 551:
		// TODO
		// Fall-through for unimplemented scenes in group #5
		error("Invalid scene %d called", scene._nextSceneId);

	// Scene group #6
	case 601:
		// TODO
	case 602:
		// TODO
	case 603:
		// TODO
	case 604:
		// TODO
	case 605:
		// TODO
	case 607:
		// TODO
	case 608:
		// TODO
	case 609:
		// TODO
	case 610:
		// TODO
	case 611:
		// TODO
	case 612:
		// TODO
	case 620:
		// TODO
		// Fall-through for unimplemented scenes in group #6
		error("Invalid scene %d called", scene._nextSceneId);

	// Scene group #7
	case 701:
		// TODO
	case 702:
		// TODO
	case 703:
		// TODO
	case 704:
		// TODO
	case 705:
		// TODO
	case 706:
		// TODO
	case 707:
		// TODO
	case 708:
		// TODO
	case 710:
		// TODO
	case 711:
		// TODO
	case 751:
		// TODO
	case 752:
		// TODO
		// Fall-through for unimplemented scenes in group #7
		error("Invalid scene %d called", scene._nextSceneId);

	// Scene group #8
	case 801:
		// TODO
	case 802:
		// TODO
	case 803:
		// TODO
		// Fall-through for unimplemented scenes in group #8
		error("Invalid scene %d called", scene._nextSceneId);
	case 804:
		return new Scene804(vm);
	case 805:
		// TODO
	case 806:
		// TODO
	case 807:
		// TODO
	case 808:
		// TODO
	case 810:
		// TODO
		// Fall-through for unimplemented scenes in group #8
		error("Invalid scene %d called", scene._nextSceneId);

	default:
		error("Invalid scene %d called", scene._nextSceneId);
	}
}

/*------------------------------------------------------------------------*/

NebularScene::NebularScene(MADSEngine *vm) : SceneLogic(vm),
		_globals(static_cast<GameNebular *>(vm->_game)->_globals),
		_game(*static_cast<GameNebular *>(vm->_game)),
		_action(vm->_game->_scene._action) {
}

Common::String NebularScene::formAnimName(char sepChar, int suffixNum) {
	return Resources::formatName(_scene->_currentSceneId, sepChar, suffixNum,
		EXT_NONE, "");
}

/*------------------------------------------------------------------------*/

void SceneInfoNebular::loadCodes(MSurface &depthSurface, int variant) {
	File f(Resources::formatName(RESPREFIX_RM, _sceneId, ".DAT"));
	MadsPack codesPack(&f);
	Common::SeekableReadStream *stream = codesPack.getItemStream(variant + 1);

	loadCodes(depthSurface, stream);

	delete stream;
	f.close();
}

void SceneInfoNebular::loadCodes(MSurface &depthSurface, Common::SeekableReadStream *stream) {
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

} // End of namespace Nebular

} // End of namespace MADS
