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
#include "freescape/games/eclipse/eclipse.h"
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
	registerCmd("goto", WRAP_METHOD(Debugger, cmdGoto)); // teleport to area/entrance or position
	registerCmd("sort_order", WRAP_METHOD(Debugger, cmdSortOrder)); // print current draw order of objects
	registerCmd("occ", WRAP_METHOD(Debugger, cmdShowOcclusion)); // toggle occlussion boxes
	registerCmd("area", WRAP_METHOD(Debugger, cmdArea)); // show current area info
	registerCmd("pos", WRAP_METHOD(Debugger, cmdPos)); // show camera position and direction
	registerCmd("win", WRAP_METHOD(Debugger, cmdWin)); // trigger the current game's win condition
	registerCmd("ankh", WRAP_METHOD(Debugger, cmdAnkh)); // set ankh count (Total Eclipse only)
}

Debugger::~Debugger() {}

bool Debugger::cmdShowBBox(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: bbox <0/1>\n");
		return true;
	}
	_vm->_gfx->_debugRenderBoundingBoxes = atoi(argv[1]);
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
	_vm->_gfx->_debugHighlightObjectIDs.clear();
	if (argc < 2) {
		debugPrintf("Isolation cleared, drawing all objects.\n");
		debugPrintf("Usage: iso [id1] [id2] [id3] ...\n");
		return true;
	}
	debugPrintf("Isolating Objects: ");
	for (int i = 1; i < argc; i++) {
		int id = atoi(argv[i]);
		if (id == -1) {
			_vm->_gfx->_debugHighlightObjectIDs.clear();
			debugPrintf("All (Cleared)");
			break;
		}

		_vm->_gfx->_debugHighlightObjectIDs.push_back(id);
		debugPrintf("%d ", id);
	}
	debugPrintf("\n");
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
	if (argc == 3) {
		uint16 areaID = atoi(argv[1]);
		int entranceID = atoi(argv[2]);
		_vm->gotoArea(areaID, entranceID);
		debugPrintf("Jumped to area %d, entrance %d\n", areaID, entranceID);
		return true;
	}

	if (argc >= 4) {
		float x = atof(argv[1]);
		float y = atof(argv[2]);
		float z = atof(argv[3]);
		_vm->_position = Math::Vector3d(x, y, z);
		debugPrintf("Teleported to %f %f %f\n", x, y, z);
		return true;
	}

	debugPrintf("Usage: goto <area> <entrance>\n");
	debugPrintf("       goto <x> <y> <z>\n");
	return true;
}

bool Debugger::cmdSortOrder(int argc, const char** argv) {
	if (!_vm->_currentArea) {
		debugPrintf("No area loaded.\n");
		return true;
	}

	Common::Array<Object *> &objs = _vm->_currentArea->getSortedObjects();

	debugPrintf("Draw order (back-to-front, first drawn first):\n");
	debugPrintf("Total visible objects: %d\n", objs.size());
	for (uint i = 0; i < objs.size(); i++) {
		Object *obj = objs[i];
		debugPrintf("%d ", obj->getObjectID());
	}
	debugPrintf("\n");
	return true;
}

bool Debugger::cmdShowOcclusion(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: occ <0/1>\n");
		return true;
	}
	_vm->_gfx->_debugRenderOcclusionBoxes = atoi(argv[1]);
	return true;
}

static const char *objectTypeNames[] = {
	"Entrance",
	"Cube",
	"Sensor",
	"Rectangle",
	"EastPyramid",
	"WestPyramid",
	"UpPyramid",
	"DownPyramid",
	"NorthPyramid",
	"SouthPyramid",
	"Line",
	"Triangle",
	"Quadrilateral",
	"Pentagon",
	"Hexagon",
	"Group"
};

bool Debugger::cmdArea(int argc, const char **argv) {
	if (!_vm->_currentArea) {
		debugPrintf("No area loaded.\n");
		return true;
	}

	Area *area = _vm->_currentArea;
	debugPrintf("Area ID: %d\n", area->getAreaID());
	debugPrintf("Area name: %s\n", area->_name.c_str());
	debugPrintf("Area flags: %04x\n", area->getAreaFlags());
	debugPrintf("Scale: %d\n", area->getScale());
	debugPrintf("Sky color: %d | Ground color: %d\n", area->_skyColor, area->_groundColor);
	debugPrintf("Ink: %d | Paper: %d\n", area->_inkColor, area->_paperColor);
	debugPrintf("Color cycling: %s\n", area->_colorCycling ? "yes" : "no");
	debugPrintf("Outside: %s\n", area->isOutside() ? "yes" : "no");
	debugPrintf("\n");

	ObjectMap *objectsByID = area->getObjectsByID();
	debugPrintf("Objects (%d):\n", objectsByID->size());
	for (auto &it : *objectsByID) {
		Object *obj = it._value;
		int type = obj->getType();
		const char *typeName = (type >= 0 && type <= 15) ? objectTypeNames[type] : "Unknown";
		debugPrintf("  ID: %3d | Type: %-14s | Flags: %04x", obj->getObjectID(), typeName, obj->getObjectFlags());
		if (obj->isInvisible()) {
			debugPrintf(" [invisible]");
		}
		if (obj->isDestroyed()) {
			debugPrintf(" [destroyed]");
		}
		if (obj->isGeometric()) {
			Math::Vector3d origin = obj->getOrigin();
			Math::Vector3d size = obj->getSize();
			debugPrintf(" | Pos: (%.0f, %.0f, %.0f) Size: (%.0f, %.0f, %.0f)",
				origin.x(), origin.y(), origin.z(),
				size.x(), size.y(), size.z());
		}
		debugPrintf("\n");
	}

	ObjectMap *entrancesByID = area->getEntrancesByID();
	if (entrancesByID->size() > 0) {
		debugPrintf("\nEntrances (%d):\n", entrancesByID->size());
		for (auto &it : *entrancesByID) {
			Object *obj = it._value;
			Math::Vector3d origin = obj->getOrigin();
			debugPrintf("  ID: %3d | Pos: (%.0f, %.0f, %.0f)\n",
				obj->getObjectID(), origin.x(), origin.y(), origin.z());
		}
	}

	return true;
}

bool Debugger::cmdPos(int argc, const char **argv) {
	Math::Vector3d pos = _vm->_position;
	Math::Vector3d front = _vm->_cameraFront;
	debugPrintf("Position: (%.2f, %.2f, %.2f)\n", pos.x(), pos.y(), pos.z());
	debugPrintf("Direction: (%.2f, %.2f, %.2f)\n", front.x(), front.y(), front.z());
	debugPrintf("Yaw: %.2f | Pitch: %.2f | Roll: %d\n", _vm->_yaw, _vm->_pitch, _vm->_roll);
	return true;
}

bool Debugger::cmdWin(int argc, const char **argv) {
	if (argc > 1) {
		debugPrintf("Usage: win\n");
		return true;
	}

	if (!_vm->triggerWinCondition()) {
		debugPrintf("No win condition is available for this game or platform.\n");
		return true;
	}

	debugPrintf("Triggered the win condition.\n");
	return true;
}

bool Debugger::cmdAnkh(int argc, const char **argv) {
	if (!_vm->isEclipse()) {
		debugPrintf("This command is only available in Total Eclipse.\n");
		return true;
	}

	if (argc < 2) {
		debugPrintf("Current ankhs: %d\n", _vm->_gameStateVars[kVariableEclipseAnkhs]);
		debugPrintf("Usage: ankh <count>\n");
		return true;
	}

	int count = atoi(argv[1]);
	_vm->_gameStateVars[kVariableEclipseAnkhs] = count;
	debugPrintf("Ankhs set to %d\n", count);
	return true;
}

}
