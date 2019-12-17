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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/graphics/type_flags.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/graphics/anim_dat.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MainShapeArchive, ShapeArchive)


MainShapeArchive::~MainShapeArchive() {
	if (typeFlags) {
		delete typeFlags;
		typeFlags = 0;
	}

	if (animdat) {
		delete animdat;
		animdat = 0;
	}
}

void MainShapeArchive::loadTypeFlags(IDataSource *ds) {
	if (typeFlags) {
		delete typeFlags;
		typeFlags = 0;
	}

	typeFlags = new TypeFlags;
	typeFlags->load(ds);
}

ShapeInfo *MainShapeArchive::getShapeInfo(uint32 shapenum) {
	assert(typeFlags);

	return typeFlags->getShapeInfo(shapenum);
}

void MainShapeArchive::loadAnimDat(IDataSource *ds) {
	if (animdat) {
		delete animdat;
		animdat = 0;
	}

	animdat = new AnimDat;
	animdat->load(ds);
}

ActorAnim *MainShapeArchive::getAnim(uint32 shape) const {
	assert(animdat);

	return animdat->getAnim(shape);
}

AnimAction *MainShapeArchive::getAnim(uint32 shape, uint32 action) const {
	assert(animdat);

	return animdat->getAnim(shape, action);
}

} // End of namespace Ultima8
} // End of namespace Ultima
