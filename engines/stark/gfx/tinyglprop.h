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

#ifndef STARK_GFX_TINYGL_RENDERED_H
#define STARK_GFX_TINYGL_RENDERED_H

#include "engines/stark/model/model.h"
#include "engines/stark/visual/prop.h"
#include "engines/stark/gfx/tinygl.h"

#include "graphics/tinygl/tinygl.h"

#include "common/hashmap.h"
#include "common/hash-ptr.h"

namespace Stark {

namespace Gfx {

class Driver;

struct _PropVertex {
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	float stexS;
	float stexT;
	float texS;
	float texT;
	float r;
	float g;
	float b;
};
typedef _PropVertex PropVertex;

class TinyGLPropRenderer : public VisualProp {
public:
	explicit TinyGLPropRenderer(TinyGLDriver *gfx);
	~TinyGLPropRenderer() override;

	void render(const Math::Vector3d &position, float direction, const LightEntryArray &lights) override;

protected:
	typedef Common::HashMap<const Face *, uint32 *> FaceBufferMap;

	TinyGLDriver *_gfx;

	bool _modelIsDirty;
	PropVertex *_faceVBO;
	FaceBufferMap _faceEBO;

	void clearVertices();
	void uploadVertices();
	PropVertex *createFaceVBO();
	uint32 *createFaceEBO(const Face *face);
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // STARK_GFX_TINYGL_S_RENDERED_H
