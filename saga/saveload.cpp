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

void SagaEngine::save() {
	File out;

	out.open("iteSCUMMVM.sav", File::kFileWriteMode);
	//TODO: version number

	out.writeSint16LE(_script->_commonBufferSize);

	// Surrounding scene
	out.writeSint32LE(_scene->getOutsetSceneNumber());
	out.writeSint32LE(0);

	// Inset scene
	out.writeSint32LE(_scene->currentSceneNumber());
	out.writeSint32LE(0);

	uint16 i;

	_actor->saveState(out);
	
	for (i = 0; i < _script->_commonBufferSize; i++)
		out.writeByte(_script->_commonBuffer[i]);

	out.writeSint16LE(_isoMap->getMapPosition().x);
	out.writeSint16LE(_isoMap->getMapPosition().y);

	out.close();
}

void SagaEngine::load() {
	File in;
	int  commonBufferSize;
	int scenenum, inset;
	int mapx, mapy;

	in.open("iteSCUMMVM.sav");

	if (!in.isOpen())
		return;

	commonBufferSize = in.readSint16LE();

	// Surrounding scene
	scenenum = in.readSint32LE();
	in.readSint32LE();

	// Inset scene
	inset = in.readSint32LE();
	in.readSint32LE();

	debug(0, "scene: %d out: %d", scenenum, inset);

	uint16 i;

	
	_interface->clearInventory(); //TODO: interface load-save-state

	_actor->loadState(in);
	
	for (i = 0; i < commonBufferSize; i++)
		_script->_commonBuffer[i] = in.readByte();

	mapx = in.readSint16LE();
	mapy = in.readSint16LE();

	in.close();

	_isoMap->setMapPosition(mapx, mapy);

	// FIXME: When save/load screen will be implemented we should
	// call these after that screen left by user
	_interface->draw();

	// FIXME: hmmm... now we always require actorsEntrance to be defined
	// so no way to restore at arbitrary position
	_scene->clearSceneQueue();
	_scene->changeScene(scenenum, 0);

	if (inset != scenenum) {
		_render->drawScene();
		_scene->clearSceneQueue();
		_scene->changeScene(inset, 0);
	}
}

} // End of namespace Saga
