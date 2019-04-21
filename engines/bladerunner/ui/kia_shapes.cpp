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

#include "bladerunner/ui/kia_shapes.h"

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

KIAShapes::KIAShapes(BladeRunnerEngine *vm) {
	_vm = vm;
	_isLoaded = false;
	for (uint i = 0; i < kShapeCount; ++i) {
		_shapes[i] = nullptr;
	}
}

KIAShapes::~KIAShapes() {
	unload();
}

void KIAShapes::load() {
	if (_isLoaded) {
		return;
	}

	for (uint i = 0; i < kShapeCount; ++i) {
		Shape *shape = new Shape(_vm);
		shape->open("kiaopt.shp", i);
		_shapes[i] = shape;
	}

	_isLoaded = true;
}

void KIAShapes::unload() {
	if (!_isLoaded) {
		return;
	}

	for (uint i = 0; i < kShapeCount; ++i) {
		delete _shapes[i];
		_shapes[i] = nullptr;
	}

	_isLoaded = false;
}

const Shape *KIAShapes::get(int shapeId) const {
	return _shapes[shapeId];
}

} // End of namespace BladeRunner
