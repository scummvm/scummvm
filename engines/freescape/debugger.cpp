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

#include "freescape/debugger.h"
#include "freescape/freescape.h"
#include "freescape/gfx.h"
#include "freescape/objects/object.h"
#include "freescape/area.h"
#include "math/ray.h"

namespace Freescape {
Debugger *g_debugger;

Debugger::Debugger(FreescapeEngine *vm) : GUI::Debugger(), _vm(vm) {
	g_debugger = this;
	// register commands
	registerCmd("info", WRAP_METHOD(Debugger, cmdInfo)); // inspect object under cursor
	registerCmd("bbox", WRAP_METHOD(Debugger, cmdShowBBox)); // toggle bboxes
	registerCmd("wire", WRAP_METHOD(Debugger, cmdWireframe)); // toggle wireframe
	registerCmd("normals", WRAP_METHOD(Debugger, cmdShowNormals)); // toggle normals
	registerCmd("iso", WRAP_METHOD(Debugger, cmdHighlightID)); // isolate object
	registerCmd("obj_pos", WRAP_METHOD(Debugger, cmdObjPos)); // get object pos
	registerCmd("obj_mov", WRAP_METHOD(Debugger, cmdSetObjPos)); // move object
	registerCmd("goto", WRAP_METHOD(Debugger, cmdGoto)); // teleport to a position
}

Debugger::~Debugger() {}

bool Debugger::cmdShowBBox(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: bbox <0/1> [id]\n");
		return true;
	}
	_vm->_gfx->_debugRenderBoundingBoxes = atoi(argv[1]);
	if (argc > 2)
		_vm->_gfx->_debugBoundingBoxFilterID = atoi(argv[2]);
	else
		_vm->_gfx->_debugBoundingBoxFilterID = -1;
	return true;
}

bool Debugger::cmdWireframe(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: wire <0/1>\n");
		return true;
	}
	_vm->_gfx->_debugRenderWireframe = atoi(argv[1]);
	return true;
}

bool Debugger::cmdShowNormals(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: normals <0/1>\n");
		return true;
	}
	_vm->_gfx->_debugRenderNormals = atoi(argv[1]);
	return true;
}

bool Debugger::cmdHighlightID(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: iso <id> (-1 for all)\n");
		return true;
	}
	_vm->_gfx->_debugHighlightObjectID = atoi(argv[1]);
	return true;
}

bool Debugger::cmdInfo(int argc, const char **argv) {
	if (!_vm->_currentArea)
		return true;
	Math::Vector3d pos = _vm->_position;
	Math::Vector3d dir = _vm->_cameraFront;

	// shoot a long raycast
	Math::Ray ray(pos, dir);
	Object *obj = _vm->_currentArea->checkCollisionRay(ray, 100000);

	if (obj) {
		debugPrintf("Object %d\n", obj->getObjectID());
		debugPrintf("Type: %d | Flags: %x\n", obj->getType(), obj->getObjectFlags());
		debugPrintf("Origin: %f %f %f\n", obj->getOrigin().x(), obj->getOrigin().y(), obj->getOrigin().z());
		debugPrintf("Size: %f %f %f\n", obj->getSize().x(), obj->getSize().y(), obj->getSize().z());
		if (obj->isGeometric()) {
			debugPrintf("BBox: min(%f %f %f) max(%f %f %f)\n",
						obj->_boundingBox.getMin().x(),
						obj->_boundingBox.getMin().y(),
						obj->_boundingBox.getMin().z(),
						obj->_boundingBox.getMax().x(),
						obj->_boundingBox.getMax().y(),
						obj->_boundingBox.getMax().z());
		}
		if (obj->_partOfGroup) {
			debugPrintf("Grouped: Yes (Group ID: %d)\n", obj->_partOfGroup->getObjectID());
		}
	} else {
		debugPrintf("Raycast hit nothing.\n");
	}
	return true;
}

bool Debugger::cmdObjPos(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: obj_pos <id>\n");
		return true;
	}
	int id = atoi(argv[1]);
	Object *obj = _vm->_currentArea->objectWithID(id);
	if (!obj) {
		debugPrintf("Object %d not found.\n", id);
		return true;
	}

	debugPrintf("Origin: %f %f %f\n", obj->getOrigin().x(), obj->getOrigin().y(), obj->getOrigin().z());
	return true;
}

bool Debugger::cmdSetObjPos(int argc, const char **argv) {
	if (argc < 5) {
		debugPrintf("Usage: obj_mov <id> <x> <y> <z>\n");
		return true;
	}
	int id = atoi(argv[1]);
	Object *obj = _vm->_currentArea->objectWithID(id);
	if (!obj) {
		debugPrintf("Object %d not found.\n", id);
		return true;
	}

	Math::Vector3d newPos(atof(argv[2]), atof(argv[3]), atof(argv[4]));
	obj->setOrigin(newPos);
	debugPrintf("Moved Object %d to %f %f %f\n", id, newPos.x(), newPos.y(), newPos.z());
	return true;
}

bool Debugger::cmdGoto(int argc, const char **argv) {
	if (argc < 4) {
		debugPrintf("Usage: goto <x> <y> <z>\n");
		return true;
	}
	float x = atof(argv[1]);
	float y = atof(argv[2]);
	float z = atof(argv[3]);
	_vm->_position = Math::Vector3d(x, y, z);
	debugPrintf("Teleported to %f %f %f\n", x, y, z);
	return true;
}

}
