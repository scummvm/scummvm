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

#include "common/system.h"
#include "common/events.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "chewy/cursor.h"
#include "chewy/graphics.h"
#include "chewy/scene.h"
#include "chewy/resource.h"
#include "chewy/text.h"
#include "chewy/video/cfo_decoder.h"

namespace Chewy {

Scene::Scene(ChewyEngine *vm) : _vm(vm) {
}

Scene::~Scene() {
}

void Scene::change(uint scene) {
	_curScene = scene;
	_vm->_cursor->setCursor(0);
	_vm->_cursor->showCursor();
	
	draw();
}

void Scene::draw() {
	_vm->_graphics->drawImage("episode1.tgp", _curScene);
	_vm->_graphics->drawSprite("det1.taf", 0, 200, 100);
	_vm->_graphics->loadFont("6x8.tff");
	_vm->_graphics->drawText("This is a test", 200, 80);
}

} // End of namespace Chewy
