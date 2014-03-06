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
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"
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

	// TODO: Implement all the game scenes
	assert(scene._nextSceneId == 804);

	return new Scene804(vm);
}

} // End of namespace Nebular

} // End of namespace MADS
