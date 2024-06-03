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

#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/gfx/main_shape_archive.h"
#include "ultima/ultima8/gfx/type_flags.h"
#include "ultima/ultima8/gfx/anim_dat.h"

namespace Ultima {
namespace Ultima8 {

MainShapeArchive::~MainShapeArchive() {
	if (_typeFlags) {
		delete _typeFlags;
	}

	if (_animDat) {
		delete _animDat;
	}
}

void MainShapeArchive::loadTypeFlags(Common::SeekableReadStream *rs) {
	if (_typeFlags) {
		delete _typeFlags;
		_typeFlags = nullptr;
	}

	_typeFlags = new TypeFlags;
	_typeFlags->load(rs);
}

void MainShapeArchive::loadDamageDat(Common::SeekableReadStream *rs) {
	assert(_typeFlags);
	_typeFlags->loadDamageDat(rs);
}

const ShapeInfo *MainShapeArchive::getShapeInfo(uint32 shapenum) {
	assert(_typeFlags);

	return _typeFlags->getShapeInfo(shapenum);
}

void MainShapeArchive::loadAnimDat(Common::SeekableReadStream *rs) {
	if (_animDat) {
		delete _animDat;
		_animDat = nullptr;
	}

	_animDat = new AnimDat;
	_animDat->load(rs);
}

const ActorAnim *MainShapeArchive::getAnim(uint32 shape) const {
	assert(_animDat);

	return _animDat->getAnim(shape);
}

const AnimAction *MainShapeArchive::getAnim(uint32 shape, uint32 action) const {
	assert(_animDat);

	return _animDat->getAnim(shape, action);
}

} // End of namespace Ultima8
} // End of namespace Ultima
