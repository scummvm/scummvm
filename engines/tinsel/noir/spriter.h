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
 * Spriter - 3D Renderer
 */

#ifndef	TINSEL_SPRITER_H
#define	TINSEL_SPRITER_H

#include "tinsel/dw.h"
#include "common/rect.h"
#include "common/stack.h"
#include "common/str.h"

#include "math/vector3d.h"
#include "math/vector2d.h"
#include "math/matrix4.h"

#if defined(USE_TINYGL)
#include "graphics/tinygl/tinygl.h"
#endif

namespace Tinsel {

typedef Common::FixedStack<Math::Matrix4, 30> MatrixStack;

enum RenderProgramOp : uint16 {
	MATRIX_DUPLICATE    = 1,
	MATRIX_REMOVE       = 2,
	UNUSED              = 3,
	TRANSFORM           = 4,
	TRANSLATE_X         = 5,
	TRANSLATE_Y         = 6,
	TRANSLATE_Z         = 7,
	TRANSLATE_XYZ       = 8,
	ROTATE_X            = 9,
	ROTATE_Y            = 10,
	ROTATE_Z            = 11,
	ROTATE_XYZ          = 17,
	STOP                = 16,
};

struct AnimationInfo {
	Common::String name;

	uint meshNum;

	uint translateTablesHunk;
	uint translateTables;
	uint translateNum;

	uint rotateTablesHunk;
	uint rotateTables;
	uint rotateNum;

	uint scaleTablesHunk;
	uint scaleTables;
	uint scaleNum;

	uint maxFrame;
};

struct MeshInfo {
	uint meshTablesHunk;
	uint meshTables;

	uint programHunk;
	uint program;
};

struct Hunk {
	Common::Array<uint8> data;
	Common::Array<uint> mappingIdx;
	uint size;
	uint flags;
};

typedef Common::Array<Hunk> Hunks;

typedef Common::Array<Math::Vector3d> Vectors;

struct Primitive {
	uint indices[8];
	uint color;
	Math::Vector2d uv[4];
	uint texture;
};

enum MeshPartType {
	MESH_PART_TYPE_COLOR,
	MESH_PART_TYPE_SOLID,
	MESH_PART_TYPE_TEXTURE,
};

struct MeshPart {
	MeshPartType type;
	uint cull;
	uint numVertices;
	Common::Array<Primitive> primitives;
};

struct Mesh {
	Common::Array<Math::Vector3d> vertices;
	Common::Array<Math::Vector3d> normals;
	Common::Array<MeshPart> parts;
	Common::Array<MeshPart> parts2;
};

struct Meshes {
	uint vertexCount;
	uint normalCount;

	Common::Array<Mesh> meshes;
};

typedef  Common::Array<Vectors> AnimationData;

struct ModelTables {
	Vectors translations;
	Vectors rotations;
	Vectors scales;
	Meshes  meshes;
};

enum ModelFlags {
	MODEL_HAS_TRANSLATION_TABLE = 1,
	MODEL_HAS_SCALE_TABLE = 2,
	MODEL_HAS_ROTATION_TABLE = 4
};

struct Model {
	Hunks hunks;
	Hunks hunksOverlay; // The game script can load additional data to supplement the main model. E.g. a special animation.
	uint animationCount;
	uint field_0xe;
	uint field_0xf;
	uint8* program;

	// animation tables
	AnimationData startTranslateTables;
	AnimationData startRotateTables;
	AnimationData startScaleTables;
	AnimationData endTranslateTables;
	AnimationData endRotateTables;
	AnimationData endScaleTables;
	int startFrame;
	int endFrame;

	uint flags;
	uint field_0x32;
	uint field_0x33;

	ModelTables tables;

	Math::Vector3d position;
	Math::Vector3d rotation;
	Math::Vector3d scale;

	uint time; // interpolant
};

struct Viewport {
	int ap;
	float width;
	float height;

	Common::Rect rect;
};

struct View {
	int centerX;
	int centerY;

	Common::Rect viewRect;
	Common::Rect screenRect;

	Viewport viewport;

	Math::Vector3d position;
	Math::Vector3d rotation;
};

class Spriter {
private:
	MatrixStack _modelMatrix;
	MatrixStack* _currentMatrix;

	Common::Array<AnimationInfo> _animMain;
	AnimationInfo _animShadow;

	MeshInfo _meshMain;
	Common::Array<MeshInfo> _meshShadow;

	View _view;

	Common::Array<uint8> _palette;
	Common::Array<uint8> _textureData;

	bool _textureGenerated;
	uint _texture[4];

	bool _modelIdle;

	uint _animId;
	uint _animSpeed;
	uint _animDelay;
	uint _animDelayMax;

	uint _sequencesCount;

	uint _direction;

public:
	Model _modelMain;
	Model _modelShadow;

public:
	Spriter();
	virtual ~Spriter();

	void Init(int width, int height);
	void SetCamera(int rotX, int rotY, int rotZ, int posX, int posY, int posZ, int cameraAp);
	void TransformSceneXYZ(int x, int y, int z, int& xOut, int& yOut);
	void Load(const Common::String& modelName, const Common::String& textureName);

	void SetPalette(SCNHANDLE hPalette);

	void SetSequence(uint animId, uint delay);
	Common::Rect Draw(int direction, int x, int y, int z, int tDelta);

private:
	const Math::Matrix4& MatrixCurrent() const;

	void MatrixReset();

	void MatrixPop();
	void MatrixPush();
	void MatrixTranslate(float x, float y, float z);
	void MatrixScale(float x, float y, float z);
	void MatrixRotateX(float angle);
	void MatrixRotateY(float angle);
	void MatrixRotateZ(float angle);

	void SetViewport(int ap);

	// Loading of the model
	void LoadH(const Common::String& modelName);
	void LoadGBL(const Common::String& modelName);
	void LoadRBH(const Common::String& modelName, Hunks& hunks);
	void LoadVMC(const Common::String& textureName);

	void UpdateTextures();

	Meshes LoadMeshes(const Hunks &hunks, uint hunk, uint offset, int frame);
	template<bool convert>
	AnimationData LoadAnimationData(const Hunks &hunks, uint hunk, uint offset);
	void InitModel(Model& model, MeshInfo& meshInfo, Common::Array<AnimationInfo>& animInfo, uint flags);

	// Processing of the model
	void RunRenderProgram(Model &model, bool preprocess);

	void FindSimilarVertices(Mesh& mesh, Vectors& vertices, Common::Array<uint16>& sameVertices) const;
	void MergeVertices(Mesh& mesh, Common::Array<uint16>& sameVertices);

	void TransformMesh(Mesh& mesh, Vectors& vertices);
	void CalculateNormals(Mesh& mesh, Vectors& vertices, Vectors &normals);

	// Rendering
	void RenderModel(Model& model);
	void RenderMesh(Mesh& mesh, Vectors& vertices, Vectors &normals);
	void RenderMeshPartColor(MeshPart& part, Vectors& vertices, Vectors &normals);
	void RenderMeshPartTexture(MeshPart& part, Vectors& vertices, Vectors &normals);

	// Animation
	bool SetStartFrame(Model &model, const AnimationInfo &anim, int frame);
	bool SetEndFrame(Model &model, const AnimationInfo &anim, int frame);
};

} // End of namespace Tinsel

#endif	// TINSEL_SPRITER_H
