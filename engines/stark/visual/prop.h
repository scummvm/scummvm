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

#ifndef STARK_VISUAL_PROP_H
#define STARK_VISUAL_PROP_H

#include "math/matrix4.h"
#include "math/ray.h"
#include "math/vector3d.h"

#include "engines/stark/visual/visual.h"
#include "engines/stark/gfx/renderentry.h"

namespace Stark {

namespace Formats {
class BiffMesh;
}

namespace Gfx {
class TextureSet;
}

class VisualProp : public Visual {
public:
	static const VisualType TYPE = Visual::kRendered;

	VisualProp();
	~VisualProp() override;

	void setModel(Formats::BiffMesh *model);
	void setTexture(Gfx::TextureSet *texture);

	bool intersectRay(const Math::Ray &ray, const Math::Vector3d &position, float direction);
	virtual void render(const Math::Vector3d &position, float direction, const Gfx::LightEntryArray &lights) = 0;

protected:
	Formats::BiffMesh *_model;
	Gfx::TextureSet *_texture;
	Math::AABB _boundingBox;

	Math::Matrix4 getModelMatrix(const Math::Vector3d& position, float direction);
};

} // End of namespace Stark

#endif // STARK_VISUAL_PROP_H
