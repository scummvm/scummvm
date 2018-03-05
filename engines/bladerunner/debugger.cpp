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

#include "bladerunner/debugger.h"

#include "bladerunner/actor.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/boundingbox.h"
#include "bladerunner/font.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/game_flags.h"
#include "bladerunner/game_info.h"
#include "bladerunner/light.h"
#include "bladerunner/lights.h"
#include "bladerunner/regions.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/settings.h"
#include "bladerunner/set.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/vector.h"
#include "bladerunner/view.h"
#include "bladerunner/waypoints.h"
#include "bladerunner/zbuffer.h"

#include "common/debug.h"
#include "common/str.h"

#include "graphics/surface.h"

namespace BladeRunner {

Debugger::Debugger(BladeRunnerEngine *vm) : GUI::Debugger() {
	_vm = vm;

	_viewSceneObjects = false;
	_viewUI = false;
	_viewZBuffer = false;

	registerCmd("anim", WRAP_METHOD(Debugger, cmdAnimation));
	registerCmd("goal", WRAP_METHOD(Debugger, cmdGoal));
	registerCmd("draw", WRAP_METHOD(Debugger, cmdDraw));
	registerCmd("scene", WRAP_METHOD(Debugger, cmdScene));
	registerCmd("chapter", WRAP_METHOD(Debugger, cmdChapter));
	registerCmd("flag", WRAP_METHOD(Debugger, cmdFlag));
	registerCmd("var", WRAP_METHOD(Debugger, cmdVariable));
	registerCmd("say", WRAP_METHOD(Debugger, cmdSay));
}

Debugger::~Debugger() {
}

bool Debugger::cmdAnimation(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Get or set animation mode of the actor.\n");
		debugPrintf("Usage: %s <actorId> [<animationMode>]\n", argv[0]);
		return true;
	}

	int actorId = atoi(argv[1]);

	Actor *actor = nullptr;
	if (actorId >= 0 && actorId < (int)_vm->_gameInfo->getActorCount()) {
		actor = _vm->_actors[actorId];
	}

	if (actor == nullptr) {
		debugPrintf("Unknown actor %i\n", actorId);
		return true;
	}

	if (argc == 3) {
		int animationMode = atoi(argv[2]);
		debugPrintf("actorAnimationMode(%i) = %i\n", actorId, animationMode);
		actor->changeAnimationMode(animationMode);
		return false;
	}

	debugPrintf("actorAnimationMode(%i) = %i\n", actorId, actor->getAnimationMode());
	return true;
}

bool Debugger::cmdGoal(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Get or set goal of the actor.\n");
		debugPrintf("Usage: %s <actorId> [<goal>]\n", argv[0]);
		return true;
	}

	int actorId = atoi(argv[1]);

	Actor *actor = nullptr;
	if (actorId >= 0 && actorId < (int)_vm->_gameInfo->getActorCount()) {
		actor = _vm->_actors[actorId];
	}

	if (actor == nullptr) {
		debugPrintf("Unknown actor %i\n", actorId);
		return true;
	}

	if (argc == 3) {
		int goal = atoi(argv[2]);
		debugPrintf("actorGoal(%i) = %i\n", actorId, goal);
		actor->setGoal(goal);
		return false;
	}

	debugPrintf("actorGoal(%i) = %i\n", actorId, actor->getGoal());
	return true;
}

bool Debugger::cmdDraw(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Enables debug rendering of scene objects, ui elements, zbuffer or disables debug rendering.\n");
		debugPrintf("Usage: %s (obj | ui | zbuf | reset)\n", argv[0]);
		return true;
	}

	Common::String arg = argv[1];
	if (arg == "obj") {
		_viewSceneObjects = !_viewSceneObjects;
		debugPrintf("Drawing scene objects = %i\n", _viewSceneObjects);
	} else if (arg == "ui") {
		_viewUI = !_viewUI;
		debugPrintf("Drawing UI elements = %i\n", _viewUI);
	} else if (arg == "zbuf") {
		_viewZBuffer = !_viewZBuffer;
		debugPrintf("Drawing Z buffer = %i\n", _viewZBuffer);
	} else if (arg == "reset") {
		_viewSceneObjects = false;
		_viewUI = false;
		_viewZBuffer = false;
		debugPrintf("Drawing scene objects = %i\n", _viewSceneObjects);
		debugPrintf("Drawing UI elements = %i\n", _viewUI);
		debugPrintf("Drawing Z buffer = %i\n", _viewZBuffer);
	}

	return true;
}

bool Debugger::cmdChapter(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Changes chapter of the game without changing scene.\n");
		debugPrintf("Usage: %s <chapter>\n", argv[0]);
		return true;
	}

	int chapter = atoi(argv[1]);
	if (chapter >= 1 && chapter <= 5) {
		_vm->_settings->setChapter(chapter);
	} else {
		debugPrintf("Chapter must be between 1 and 5\n");
	}

	return true;
}

bool Debugger::cmdFlag(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Get or set game flag (boolean value).\n");
		debugPrintf("Usage: %s <id> [<value>]\n", argv[0]);
		return true;
	}

	int flag = atoi(argv[1]);
	int flagCount = _vm->_gameInfo->getFlagCount();
	if (flag > 0 && flag < flagCount) {
		if (argc == 3) {
			int value = atoi(argv[2]);
			if (value == 0) {
				_vm->_gameFlags->reset(flag);
			} else {
				_vm->_gameFlags->set(flag);
			}
		}
		debugPrintf("flag(%i) = %i\n", flag, _vm->_gameFlags->query(flag));
	} else {
		debugPrintf("Flag id must be between 0 and %i\n", flagCount - 1);
	}

	return true;
}

bool Debugger::cmdSay(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Actor will say specified line.\n");
		debugPrintf("Usage: %s <actorId> <sentenceId>\n", argv[0]);
		return true;
	}

	int actorId = atoi(argv[1]);
	int sentenceId = atoi(argv[2]);

	Actor *actor = nullptr;
	if (actorId >= 0 && actorId < (int)_vm->_gameInfo->getActorCount()) {
		actor = _vm->_actors[actorId];
	}

	if (actor == nullptr) {
		debugPrintf("Unknown actor %i\n", actorId);
		return true;
	}

	actor->speechPlay(sentenceId, true);
	return false;
}

bool Debugger::cmdScene(int argc, const char **argv) {
	if (argc != 1 && argc != 3) {
		debugPrintf("Changes set and scene.\n");
		debugPrintf("Usage: %s [<setId> <sceneId>]\n", argv[0]);
		return true;
	}

	if (argc == 3) {
		int setId = atoi(argv[1]);
		int sceneId = atoi(argv[2]);
		_vm->_settings->setNewSetAndScene(setId, sceneId);
		return false;
	}

	debugPrintf("set = %i\nscene = %i\n", _vm->_scene->getSetId(), _vm->_scene->getSceneId());
	return true;
}

bool Debugger::cmdVariable(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Get or set game variable (integer).\n");
		debugPrintf("Usage: %s <id> [<value>]\n", argv[0]);
		return true;
	}

	int variable = atoi(argv[1]);
	int variableCount = _vm->_gameInfo->getGlobalVarCount();
	if (variable >= 0 && variable < variableCount) {
		if (argc == 3) {
			_vm->_gameVars[variable] = atoi(argv[2]);
		}
		debugPrintf("variable(%i) = %i\n", variable, _vm->_gameVars[variable]);
	} else {
		debugPrintf("Variable id must be between 0 and %i\n", variableCount - 1);
	}
	return true;
}

void Debugger::drawBBox(Vector3 start, Vector3 end, View *view, Graphics::Surface *surface, int color) {
	Vector3 bfl = view->calculateScreenPosition(Vector3(start.x, start.y, start.z));
	Vector3 bfr = view->calculateScreenPosition(Vector3(start.x, end.y, start.z));
	Vector3 bbr = view->calculateScreenPosition(Vector3(end.x, end.y, start.z));
	Vector3 bbl = view->calculateScreenPosition(Vector3(end.x, start.y, start.z));

	Vector3 tfl = view->calculateScreenPosition(Vector3(start.x, start.y, end.z));
	Vector3 tfr = view->calculateScreenPosition(Vector3(start.x, end.y, end.z));
	Vector3 tbr = view->calculateScreenPosition(Vector3(end.x, end.y, end.z));
	Vector3 tbl = view->calculateScreenPosition(Vector3(end.x, start.y, end.z));

	surface->drawLine(bfl.x, bfl.y, bfr.x, bfr.y, color);
	surface->drawLine(bfr.x, bfr.y, bbr.x, bbr.y, color);
	surface->drawLine(bbr.x, bbr.y, bbl.x, bbl.y, color);
	surface->drawLine(bbl.x, bbl.y, bfl.x, bfl.y, color);

	surface->drawLine(tfl.x, tfl.y, tfr.x, tfr.y, color);
	surface->drawLine(tfr.x, tfr.y, tbr.x, tbr.y, color);
	surface->drawLine(tbr.x, tbr.y, tbl.x, tbl.y, color);
	surface->drawLine(tbl.x, tbl.y, tfl.x, tfl.y, color);

	surface->drawLine(bfl.x, bfl.y, tfl.x, tfl.y, color);
	surface->drawLine(bfr.x, bfr.y, tfr.x, tfr.y, color);
	surface->drawLine(bbr.x, bbr.y, tbr.x, tbr.y, color);
	surface->drawLine(bbl.x, bbl.y, tbl.x, tbl.y, color);
}

void Debugger::drawSceneObjects() {
	//draw scene objects
	int count = _vm->_sceneObjects->_count;
	if (count > 0) {
		for (int i = 0; i < count; i++) {
			SceneObjects::SceneObject *sceneObject = &_vm->_sceneObjects->_sceneObjects[_vm->_sceneObjects->_sceneObjectsSortedByDistance[i]];

			const BoundingBox *bbox = sceneObject->boundingBox;
			Vector3 a, b;
			bbox->getXYZ(&a.x, &a.y, &a.z, &b.x, &b.y, &b.z);
			Vector3 pos = _vm->_view->calculateScreenPosition(0.5 * (a + b));
			int color;

			switch (sceneObject->type) {
			case kSceneObjectTypeUnknown:
				break;
			case kSceneObjectTypeActor:
				color = 0x7C00; // 11111 00000 00000;
				drawBBox(a, b, _vm->_view, &_vm->_surfaceFront, color);
				_vm->_surfaceFront.frameRect(*sceneObject->screenRectangle, color);
				_vm->_mainFont->drawColor(_vm->_textActorNames->getText(sceneObject->id - kSceneObjectOffsetActors), _vm->_surfaceFront, pos.x, pos.y, color);
				break;
			case kSceneObjectTypeItem:
				color = 0x03E0; // 00000 11111 00000
				char itemText[40];
				drawBBox(a, b, _vm->_view, &_vm->_surfaceFront, color);
				sprintf(itemText, "item %i", sceneObject->id - kSceneObjectOffsetItems);
				_vm->_surfaceFront.frameRect(*sceneObject->screenRectangle, color);
				_vm->_mainFont->drawColor(itemText, _vm->_surfaceFront, pos.x, pos.y, color);
				break;
			case kSceneObjectTypeObject:
				color = 0x3DEF; //01111 01111 01111;
				//if (sceneObject->_isObstacle)
				//	color += 0b100000000000000;
				if (sceneObject->isClickable) {
					color = 0x03E0; // 00000 11111 00000;
				}
				drawBBox(a, b, _vm->_view, &_vm->_surfaceFront, color);
				_vm->_surfaceFront.frameRect(*sceneObject->screenRectangle, color);
				_vm->_mainFont->drawColor(_vm->_scene->objectGetName(sceneObject->id - kSceneObjectOffsetObjects), _vm->_surfaceFront, pos.x, pos.y, color);
				break;
			}
		}
	}

	//draw regions
	for (int i = 0; i < 10; i++) {
		Regions::Region *region = &_vm->_scene->_regions->_regions[i];
		if (!region->present) continue;
		_vm->_surfaceFront.frameRect(region->rectangle, 0x001F); // 00000 00000 11111
	}

	for (int i = 0; i < 10; i++) {
		Regions::Region *region = &_vm->_scene->_exits->_regions[i];
		if (!region->present) continue;
		_vm->_surfaceFront.frameRect(region->rectangle, 0x7FFF); // 11111 11111 11111
	}

	//draw walkboxes
	for (int i = 0; i < _vm->_scene->_set->_walkboxCount; i++) {
		Set::Walkbox *walkbox = &_vm->_scene->_set->_walkboxes[i];

		for (int j = 0; j < walkbox->vertexCount; j++) {
			Vector3 start = _vm->_view->calculateScreenPosition(walkbox->vertices[j]);
			Vector3 end = _vm->_view->calculateScreenPosition(walkbox->vertices[(j + 1) % walkbox->vertexCount]);
			_vm->_surfaceFront.drawLine(start.x, start.y, end.x, end.y, 0x7FE0); // 11111 11111 00000
			Vector3 pos = _vm->_view->calculateScreenPosition(0.5 * (start + end));
			_vm->_mainFont->drawColor(walkbox->name, _vm->_surfaceFront, pos.x, pos.y, 0x7FE0); // 11111 11111 00000
		}
	}

	// draw lights
	for (int i = 0; i < (int)_vm->_lights->_lights.size(); i++) {
		Light *light = _vm->_lights->_lights[i];
		Matrix4x3 m = light->_matrix;
		m = invertMatrix(m);
		//todo do this properly
		Vector3 posOrigin = m * Vector3(0.0f, 0.0f, 0.0f);
		float t = posOrigin.y;
		posOrigin.y = posOrigin.z;
		posOrigin.z = -t;

		Vector3 posTarget = m * Vector3(0.0f, 0.0f, -100.0f);
		t = posTarget.y;
		posTarget.y = posTarget.z;
		posTarget.z = -t;

		Vector3 size = Vector3(5.0f, 5.0f, 5.0f);
		int colorR = (light->_color.r * 31.0f);
		int colorG = (light->_color.g * 31.0f);
		int colorB = (light->_color.b * 31.0f);
		int color = (colorR << 10) + (colorG << 5) + colorB;

		drawBBox(posOrigin - size, posOrigin + size, _vm->_view, &_vm->_surfaceFront, color);

		Vector3 posOriginT = _vm->_view->calculateScreenPosition(posOrigin);
		Vector3 posTargetT = _vm->_view->calculateScreenPosition(posTarget);
		_vm->_surfaceFront.drawLine(posOriginT.x, posOriginT.y, posTargetT.x, posTargetT.y, color);
		_vm->_mainFont->drawColor(light->_name, _vm->_surfaceFront, posOriginT.x, posOriginT.y, color);
	}

	//draw waypoints
	for(int i = 0; i < _vm->_waypoints->_count; i++) {
		Waypoints::Waypoint *waypoint = &_vm->_waypoints->_waypoints[i];
		if(waypoint->setId != _vm->_scene->getSetId())
			continue;
		Vector3 pos = waypoint->position;
		Vector3 size = Vector3(5.0f, 5.0f, 5.0f);
		int color = 0x7FFF; // 11111 11111 11111
		drawBBox(pos - size, pos + size, _vm->_view, &_vm->_surfaceFront, color);
		Vector3 spos = _vm->_view->calculateScreenPosition(pos);
		char waypointText[40];
		sprintf(waypointText, "waypoint %i", i);
		_vm->_mainFont->drawColor(waypointText, _vm->_surfaceFront, spos.x, spos.y, color);
	}

#if 0
	//draw aesc
	for (uint i = 0; i < _screenEffects->_entries.size(); i++) {
		ScreenEffects::Entry &entry = _screenEffects->_entries[i];
		int j = 0;
		for (int y = 0; y < entry.height; y++) {
			for (int x = 0; x < entry.width; x++) {
				Common::Rect r((entry.x + x) * 2, (entry.y + y) * 2, (entry.x + x) * 2 + 2, (entry.y + y) * 2 + 2);

				int ec = entry.data[j++];
				Color256 color = entry.palette[ec];
				int bladeToScummVmConstant = 256 / 16;

				Graphics::PixelFormat _pixelFormat = createRGB555();
				int color555 = _pixelFormat.RGBToColor(
					CLIP(color.r * bladeToScummVmConstant, 0, 255),
					CLIP(color.g * bladeToScummVmConstant, 0, 255),
					CLIP(color.b * bladeToScummVmConstant, 0, 255));
				_surfaceFront.fillRect(r, color555);
			}
		}
	}
#endif
}

} // End of namespace BladeRunner
