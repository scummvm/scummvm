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

	out.open("ite.sav", File::kFileWriteMode);

	out.writeSint16LE(_actor->_actorsCount);
	out.writeSint16LE(_actor->_objsCount);
	out.writeSint16LE(_script->_commonBufferSize);
	out.writeSint16LE(_actor->getProtagState());

	// Surrounding scene
	out.writeSint32LE(_scene->getOutsetSceneNumber());
	out.writeSint32LE(0);

	// Inset scene
	out.writeSint32LE(_scene->currentSceneNumber());
	out.writeSint32LE(0);

	uint16 i;

	for (i = 0; i < _actor->_actorsCount; i++) {
		ActorData *a = _actor->_actors[i];

		out.writeSint32LE(a->sceneNumber);
		out.writeSint16LE(a->location.x);
		out.writeSint16LE(a->location.y);
		out.writeSint16LE(a->location.z);
		out.writeSint16LE(a->finalTarget.x);
		out.writeSint16LE(a->finalTarget.y);
		out.writeSint16LE(a->finalTarget.z);
		out.writeByte(a->currentAction);
		out.writeByte(a->facingDirection);
		out.writeSint16LE(a->targetObject);
		out.writeByte(a->flags & (kProtagonist | kFollower));
		out.writeByte(a->frameNumber);
		out.writeSint16LE(0);
	}
	
	for (i = 0; i < _actor->_objsCount; i++) {
		ObjectData *o = _actor->_objs[i];

		out.writeSint32LE(o->sceneNumber);
		out.writeSint32LE(o->id);
		out.writeSint16LE(o->location.x);
		out.writeSint16LE(o->location.y);
		out.writeSint16LE(o->location.z);
		out.writeSint16LE(o->nameIndex);
		if (o->sceneNumber == ITE_SCENE_INV) {
			out.writeSint16LE(_interface->inventoryItemPosition(_actor->objIndexToId(i)));
		} else {
			out.writeSint16LE(0);
		}
		out.writeByte(0);
	}
	
	for (i = 0; i < _script->_commonBufferSize; i++)
		out.writeByte(_script->_commonBuffer[i]);

	out.writeSint16LE(_isoMap->getMapPosition().x);
	out.writeSint16LE(_isoMap->getMapPosition().y);

	out.close();
}

void SagaEngine::load() {
	File in;
	int actorsCount, objsCount, commonBufferSize;
	int scenenum, inset;
	int mapx, mapy;

	in.open("ite.sav");

	if (!in.isOpen())
		return;

	actorsCount = in.readSint16LE();
	objsCount = in.readSint16LE();
	commonBufferSize = in.readSint16LE();
	_actor->setProtagState(in.readSint16LE());

	// Surrounding scene
	scenenum = in.readSint32LE();
	in.readSint32LE();

	// Inset scene
	inset = in.readSint32LE();
	in.readSint32LE();

	debug(0, "scene: %d out: %d", scenenum, inset);

	uint16 i;

	for (i = 0; i < actorsCount; i++) {
		ActorData *a = _actor->_actors[i];

		a->sceneNumber = in.readSint32LE();
		a->location.x = in.readSint16LE();
		a->location.y = in.readSint16LE();
		a->location.z = in.readSint16LE();
		a->finalTarget.x = in.readSint16LE();
		a->finalTarget.y = in.readSint16LE();
		a->finalTarget.z = in.readSint16LE();
		a->currentAction = in.readByte();
		a->facingDirection = in.readByte();
		a->targetObject = in.readSint16LE();
		a->flags = (a->flags & ~(kProtagonist | kFollower) | in.readByte());
		a->frameNumber = in.readByte();
		in.readSint16LE();
	}
	
	_interface->clearInventory();

	for (i = 0; i < objsCount; i++) {
		ObjectData *o = _actor->_objs[i];
		int pos;

		o->sceneNumber = in.readSint32LE();
		o->id = in.readSint32LE();
		o->location.x = in.readSint16LE();
		o->location.y = in.readSint16LE();
		o->location.z = in.readSint16LE();
		o->nameIndex = in.readSint16LE();
		
		pos = in.readSint16LE();
		if (o->sceneNumber == ITE_SCENE_INV) {
			_interface->addToInventory(_actor->objIndexToId(i), pos);
		}
		in.readByte();
	}
	
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
