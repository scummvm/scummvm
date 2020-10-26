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
		_faceTextureName(' '),
		_castsShadow(false) {
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

void VisualActor::setTexture(Gfx::TextureSet *texture) {
	_textureSet = texture;
}

void VisualActor::setTextureFacial(Gfx::TextureSet *textureFacial) {
	_textureSetFacial = textureFacial;
}

void VisualActor::setNewFace(char shape) {
	_faceTextureName = shape;
}

const Gfx::Texture *VisualActor::resolveTexture(const Material *material) const {
	const Gfx::Texture *texture = nullptr;
	// Emma's face material is incorrectly named "faceEmma".
	// This workaround enables Emma's lipsync, which does not work in the original game engine.
	if (_textureSetFacial && (material->name == "face" || material->name == "faceEmma")) {
		texture = _textureSetFacial->getTexture(Common::String::format("%c.bmp", _faceTextureName));

		if (!texture) {
			// Default face texture in case the requested shape was not found
			texture = _textureSetFacial->getTexture("i.bmp");
		}
	}

	if (!texture) {
		texture = _textureSet->getTexture(material->texture);
	}

	return texture;
}

void VisualActor::setTime(uint32 time) {
	_time = time;
}

Math::Matrix4 VisualActor::getModelMatrix(const Math::Vector3d &position, float direction) const {
	Math::Matrix4 modelMatrix;
	modelMatrix.setPosition(position);

	Math::Angle swayAngle = StarkScene->getSwayAngle();
	if (swayAngle != 0) {
		Math::Quaternion swayRotation = Math::Quaternion(StarkScene->getSwayDirection(), swayAngle / 2.0);
		modelMatrix = modelMatrix * swayRotation.toMatrix();
	}

	float floatOffset = StarkScene->getFloatOffset();
	if (floatOffset != 0) {
		Math::Matrix4 floatTranslation;
		floatTranslation.setPosition(Math::Vector3d(0, 0, floatOffset));
		modelMatrix = modelMatrix * floatTranslation;
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

Common::Rect VisualActor::getBoundingRect(const Math::Vector3d &position3d, float direction) const {
	Math::Matrix4 modelMatrix = getModelMatrix(position3d, direction);

	Math::AABB modelSpaceBB = _model->getBoundingBox();
	Math::Vector3d min = modelSpaceBB.getMin();
	Math::Vector3d max = modelSpaceBB.getMax();

	Math::Vector3d verts[8];
	verts[0].set(min.x(), min.y(), min.z());
	verts[1].set(max.x(), min.y(), min.z());
	verts[2].set(min.x(), max.y(), min.z());
	verts[3].set(min.x(), min.y(), max.z());
	verts[4].set(max.x(), max.y(), min.z());
	verts[5].set(max.x(), min.y(), max.z());
	verts[6].set(min.x(), max.y(), max.z());
	verts[7].set(max.x(), max.y(), max.z());

	Common::Rect boundingRect;
	for (int i = 0; i < 8; ++i) {
		modelMatrix.transform(&verts[i], true);
		Common::Point point = StarkScene->convertPosition3DToGameScreenOriginal(verts[i]);

		if (i == 0) {
			boundingRect.top = point.y;
			boundingRect.bottom = point.y;
			boundingRect.left = point.x;
			boundingRect.right = point.x;
		} else {
			if (boundingRect.left > point.x) {
				boundingRect.left = point.x;
			}
			if (boundingRect.right < point.x) {
				boundingRect.right = point.x;
			}
			if (boundingRect.top > point.y) {
				boundingRect.top = point.y;
			}
			if (boundingRect.bottom < point.y) {
				boundingRect.bottom = point.y;
			}
		}
	}

	return boundingRect;
}

} // End of namespace Stark
