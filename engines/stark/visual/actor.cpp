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
#include "engines/stark/model/animhandler.h"
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"

namespace Stark {

VisualActor::VisualActor() :
		Visual(TYPE),
		_animHandler(nullptr),
		_model(nullptr),
		_textureSet(nullptr),
		_textureSetFacial(nullptr),
		_time(0),
		_modelIsDirty(true),
		_faceTextureName(' ') {
}

VisualActor::~VisualActor() {
}

void VisualActor::setModel(Model *model) {
	if (_model == model) {
		return; // Nothing to do
	}

	_model = model;
	_modelIsDirty = true;
}

void VisualActor::setAnimHandler(AnimHandler *animHandler) {
	_animHandler = animHandler;
}

void VisualActor::setAnim(SkeletonAnim *anim) {
	_animHandler->setAnim(anim);
}

void VisualActor::setTexture(Gfx::TextureSet *texture) {
	_textureSet = texture;
}

void VisualActor::setTextureFacial(Gfx::TextureSet *textureFacial) {
	_textureSetFacial = textureFacial;
}

void VisualActor::setNewFace(char shape) {
	_faceTextureName = shape;
}

const Gfx::Texture *VisualActor::resolveTexture(const MaterialNode *material) const {
	const Gfx::Texture *texture = nullptr;
	if (_textureSetFacial && material->_name == "face") {
		texture = _textureSetFacial->getTexture(Common::String::format("%c.bmp", _faceTextureName));

		if (!texture) {
			// Default face texture in case the requested shape was not found
			texture = _textureSetFacial->getTexture("i.bmp");
		}
	}

	if (!texture) {
		texture = _textureSet->getTexture(material->_texName);
	}

	return texture;
}

void VisualActor::setTime(uint32 time) {
	_time = time;
}

Math::Matrix4 VisualActor::getModelMatrix(const Math::Vector3d &position, float direction) {
	Math::Matrix4 modelMatrix;
	modelMatrix.setPosition(position);

	Math::Angle swayAngle = StarkScene->getSwayAngle();
	if (swayAngle != 0) {
		Math::Quaternion swayRotation = Math::Quaternion(StarkScene->getSwayDirection(), swayAngle / 2.0);
		modelMatrix = modelMatrix * swayRotation.toMatrix();
	}

	Math::Matrix4 rot1;
	rot1.buildAroundX(90);

	Math::Matrix4 rot2;
	rot2.buildAroundY(270 - direction);

	Math::Matrix4 scale;
	scale.setValue(2, 2, -1.0f);

	return modelMatrix * rot1 * rot2 * scale;
}

bool VisualActor::intersectRay(const Math::Ray &ray, const Math::Vector3d &position, float direction) {
	Math::Matrix4 inverseModelMatrix = getModelMatrix(position, direction);
	inverseModelMatrix.inverse();

	// Build an object local ray from the world ray
	Math::Ray localRay = ray;
	localRay.transform(inverseModelMatrix);

	return _model->intersectRay(localRay);
}

void VisualActor::resetBlending() {
	if (_animHandler) {
		_animHandler->resetBlending();
	}
}

} // End of namespace Stark
