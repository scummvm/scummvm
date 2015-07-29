/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/visual/actor.h"

#include "engines/stark/model/model.h"
#include "engines/stark/model/skeleton.h"
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

namespace Stark {

VisualActor::VisualActor() :
		Visual(TYPE),
		_model(nullptr),
		_time(0),
		_meshIsDirty(true) {
}

VisualActor::~VisualActor() {
}

void VisualActor::setModel(Model *model) {
	if (_model == model) {
		return; // Nothing to do
	}

	_model = model;
	_meshIsDirty = true;
}

void VisualActor::setAnim(SkeletonAnim *anim) {
	_model->setAnim(anim);
}

void VisualActor::setTexture(Gfx::TextureSet *texture) {
	_model->setTextureSet(texture);
}

void VisualActor::setTime(uint32 time) {
	_time = time;
}

Math::Matrix4 VisualActor::getModelMatrix(const Math::Vector3d& position, float direction) {
	Math::Matrix4 posMatrix;
	posMatrix.setPosition(position);

	Math::Matrix4 rot1;
	rot1.buildAroundX(90);

	Math::Matrix4 rot2;
	rot2.buildAroundY(90 - (_model->getFacingDirection() + direction));

	Math::Matrix4 scale;
	scale.setValue(2, 2, -1.0f);

	return posMatrix * rot1 * rot2 * scale;

}

} // End of namespace Stark
