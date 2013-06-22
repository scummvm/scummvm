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

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"
#include "fullpipe/ngiarchive.h"
#include "fullpipe/statics.h"

namespace Fullpipe {

StaticANIObject::StaticANIObject() {
	_shadowsOn = 1;
	_field_30 = 0;
	_field_34 = 1;
	_initialCounter = 0;
	_messageQueueId = 0;
	_animExFlag = 0;
	_counter = 0;
	_movementObj = 0;
	_staticsObj = 0;
	_flags = 0;
	_callback1 = 0;
	_callback2 = 0;
	_sceneId = -1;
	_someDynamicPhaseIndex = -1;
}

bool StaticANIObject::load(MfcArchive &file) {
	GameObject::load(file);

	int count = file.readUint16LE();

	for (int i = 0; i < count; i++) {
		Statics *st = new Statics();
	  
		st->load(file);
		_staticsList.push_back(st);
	}

	count = file.readUint16LE();

	for (int i = 0; i < count; i++) {
	  	int movNum = file.readUint16LE();

		char *movname = genFileName(_id, movNum, "mov");

		Common::SeekableReadStream *f = g_fullpipe->_currArchive->createReadStreamForMember(movname);

		Movement *mov = new Movement();

		MfcArchive archive(f);

		mov->load(archive, this);

		_movements.push_back(mov);

		delete f;
		free(movname);
	}

	Common::Point pt;
	if (count) { // We have movements
		((Movement *)_movements[0])->getCurrDynamicPhaseXY(pt);
	} else {
		pt.x = pt.y = 100;
	}

	setOXY(pt.x, pt.y);

	return true;
}

void StaticANIObject::setOXY(int x, int y) {
}

Movement::Movement() {
}

bool Movement::load(MfcArchive &file) {
	warning("STUB: Movement::load");
	return true;
}
bool Movement::load(MfcArchive &file, StaticANIObject *ani) {
	return true;
}

Common::Point *Movement::getCurrDynamicPhaseXY(Common::Point &p) {
	p.x = _currDynamicPhase->_x;
	p.y = _currDynamicPhase->_y;

	return &p;
}

} // End of namespace Fullpipe
