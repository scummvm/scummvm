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

#ifndef STARK_GFX_TINYGL_ACTOR_H
#define STARK_GFX_TINYGL_ACTOR_H

#include "engines/stark/gfx/renderentry.h"
#include "engines/stark/visual/actor.h"
#include "engines/stark/gfx/tinygl.h"

#include "graphics/tinygl/tinygl.h"

#include "common/hashmap.h"
#include "common/hash-ptr.h"

namespace Stark {
namespace Gfx {

class TinyGLDriver;

struct _ActorVertex {
	float pos1x;
	float pos1y;
	float pos1z;
	float pos2x;
	float pos2y;
	float pos2z;
	uint32 bone1;
	uint32 bone2;
	float boneWeight;
	float normalx;
	float normaly;
	float normalz;
	float texS;
	float texT;
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	float sx;
	float sy;
	float sz;
	float r;
	float g;
	float b;
};
typedef _ActorVertex ActorVertex;

class TinyGLActorRenderer : public VisualActor {
public:
	TinyGLActorRenderer(TinyGLDriver *gfx);
	virtual ~TinyGLActorRenderer();

	void render(const Math::Vector3d &position, float direction, const LightEntryArray &lights) override;

protected:
	typedef Common::HashMap<Face *, uint32 *> FaceBufferMap;

	TinyGLDriver *_gfx;

	ActorVertex *_faceVBO;
	FaceBufferMap _faceEBO;

	void clearVertices();
	void uploadVertices();
	ActorVertex *createModelVBO(const Model *model);
	uint32 *createFaceEBO(const Face *face);
	void setLightArrayUniform(const LightEntryArray &lights);

	Math::Vector3d getShadowLightDirection(const LightEntryArray &lights, const Math::Vector3d &actorPosition, Math::Matrix3 worldToModelRot);

	bool getPointLightContribution(LightEntry *light, const Math::Vector3d &actorPosition,
			Math::Vector3d &direction, float weight = 1.0f);
	bool getDirectionalLightContribution(LightEntry *light, Math::Vector3d &direction);
	bool getSpotLightContribution(LightEntry *light, const Math::Vector3d &actorPosition, Math::Vector3d &direction);
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // STARK_GFX_TINYGL_ACTOR_H
