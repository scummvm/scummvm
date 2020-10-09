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

#include "engines/stark/visual/prop.h"

#include "engines/stark/formats/biffmesh.h"
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

namespace Stark {

VisualProp::VisualProp() :
		Visual(TYPE),
		_model(nullptr),
		_texture(nullptr) {
}

VisualProp::~VisualProp() {
	delete _model;
	delete _texture;
}

void VisualProp::setModel(Formats::BiffMesh *model) {
	assert(!_model);

	_model = model;

	const Common::Array<Formats::BiffMesh::Vertex> &vertices = _model->getVertices();
	for (uint i = 0; i < vertices.size(); i++) {
		_boundingBox.expand(vertices[i].position);
	}
}

void VisualProp::setTexture(Gfx::TextureSet *texture) {
	assert(!_texture);

	_texture = texture;
}

Math::Matrix4 VisualProp::getModelMatrix(const Math::Vector3d& position, float direction) {
	Math::Matrix4 posMatrix;
	posMatrix.setPosition(position);

	Math::Matrix4 rot1;
	rot1.buildAroundX(90);

	Math::Matrix4 rot2;
	rot2.buildAroundY(270 - direction);

	Math::Matrix4 modelTransform = _model->getTransform();

	return posMatrix * rot1 * rot2 * modelTransform;
}

bool VisualProp::intersectRay(const Math::Ray &ray, const Math::Vector3d &position, float direction) {
	Math::Matrix4 inverseModelMatrix = getModelMatrix(position, direction);
	inverseModelMatrix.inverse();

	// Build an object local ray from the world ray
	Math::Ray localRay = ray;
	localRay.transform(inverseModelMatrix);

	return localRay.intersectAABB(_boundingBox);
}

} // End of namespace Stark
