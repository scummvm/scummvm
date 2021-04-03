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

#ifndef STARK_GFX_OPENGL_ACTOR_H
#define STARK_GFX_OPENGL_ACTOR_H

#include "engines/stark/gfx/renderentry.h"
#include "engines/stark/visual/actor.h"
#include "engines/stark/gfx/opengl.h"

#include "common/hashmap.h"
#include "common/hash-ptr.h"

#include "graphics/opengl/system_headers.h"

#if defined(USE_OPENGL_GAME)

namespace Stark {
namespace Gfx {

class OpenGLDriver;

#include "common/pack-start.h"

struct _ActorVertex {
	Math::Vector3d pos1;
	Math::Vector3d pos2;
	uint32 bone1;
	uint32 bone2;
	float boneWeight;
	Math::Vector3d normal;
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
} PACKED_STRUCT;
typedef _ActorVertex ActorVertex;

#include "common/pack-end.h"

class OpenGLActorRenderer : public VisualActor {
public:
	OpenGLActorRenderer(OpenGLDriver *gfx);
	virtual ~OpenGLActorRenderer();

	void render(const Math::Vector3d &position, float direction, const LightEntryArray &lights) override;

protected:
	typedef Common::HashMap<Face *, uint32 *> FaceBufferMap;

	OpenGLDriver *_gfx;

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

#endif // defined(USE_OPENGL_GAME)

#endif // STARK_GFX_OPENGL_ACTOR_H
