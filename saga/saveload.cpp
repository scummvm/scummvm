/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/file.h"

#include "saga/saga.h"
#include "saga/actor.h"
#include "saga/isomap.h"
#include "saga/resnames.h"
#include "saga/script.h"
#include "saga/interface.h"
#include "saga/scene.h"
#include "saga/render.h"

namespace Saga {

void SagaEngine::save(const char *fileName) {
	Common::File out;

	out.open(fileName, Common::File::kFileWriteMode);
	//TODO: version number

	// Surrounding scene
	out.writeSint32LE(_scene->getOutsetSceneNumber());

	// Inset scene
	out.writeSint32LE(_scene->currentSceneNumber());

	uint16 i;

	_interface->saveState(out);

	_actor->saveState(out);
	
	out.writeSint16LE(_script->_commonBufferSize);

	for (i = 0; i < _script->_commonBufferSize; i++)
		out.writeByte(_script->_commonBuffer[i]);

	out.writeSint16LE(_isoMap->getMapPosition().x);
	out.writeSint16LE(_isoMap->getMapPosition().y);

	out.close();
}

void SagaEngine::load(const char *fileName) {
	Common::File in;
	int  commonBufferSize;
	int sceneNumber, insetSceneNumber;
	int mapx, mapy;
	uint16 i;

	in.open(fileName);

	if (!in.isOpen())
		return;


	// Surrounding scene
	sceneNumber = in.readSint32LE();

	// Inset scene
	insetSceneNumber = in.readSint32LE();

	debug(0, "scene: #%d inset scene: #%d", sceneNumber, insetSceneNumber);

	
	_interface->loadState(in);

	_actor->loadState(in);
	
	commonBufferSize = in.readSint16LE();
	for (i = 0; i < commonBufferSize; i++)
		_script->_commonBuffer[i] = in.readByte();

	mapx = in.readSint16LE();
	mapy = in.readSint16LE();

	in.close();

	_isoMap->setMapPosition(mapx, mapy);

	_scene->clearSceneQueue();
	_scene->changeScene(sceneNumber, ACTOR_NO_ENTRANCE, kTransitionNoFade);

	if (insetSceneNumber != sceneNumber) {
		_render->drawScene();
		_scene->clearSceneQueue();
		_scene->changeScene(insetSceneNumber, ACTOR_NO_ENTRANCE, kTransitionNoFade);
	}

	// FIXME: When save/load screen will be implemented we should
	// call these after that screen left by user
	_interface->draw();
}

} // End of namespace Saga
