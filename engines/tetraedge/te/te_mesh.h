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

#ifndef TETRAEDGE_TE_TE_MESH_H
#define TETRAEDGE_TE_TE_MESH_H

#include "common/array.h"
#include "common/ptr.h"

#include "tetraedge/te/te_3d_object2.h"
#include "tetraedge/te/te_3d_texture.h"
#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_intrusive_ptr.h"
#include "tetraedge/te/te_vector2f32.h"
#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_matrix4x4.h"
#include "tetraedge/te/te_material.h"
#include "tetraedge/te/te_model_vertex_animation.h"

namespace Tetraedge {


class TeMesh : public Te3DObject2 {
public:
	TeMesh();
	TeMesh(const TeMesh &other) = default;

	enum Mode {
		MeshMode_None = 0,
		MeshMode_Points,
		MeshMode_Lines,
		MeshMode_LineLoop,
		MeshMode_LineStrip,
		MeshMode_Triangles,
		MeshMode_TriangleStrip,
		MeshMode_TriangleFan
	};

	void attachMaterial(uint idx, const TeMaterial &material);
	void boundingBox(TeVector3f32 &boxmin, TeVector3f32 boxmax);
	void checkArrays() {};
	void clearColors() { _colors.clear(); }
	TeColor color(uint idx) const { return _colors[idx]; }
	void copy(const TeMesh &other);
	void create();
	void defaultMaterial(const TeIntrusivePtr<Te3DTexture> &texture);
	void destroy();
	void draw() override;
	void facesPerMaterial(uint idx, unsigned short value);
	unsigned short facesPerMaterial(uint idx) const { return _faceCounts[idx]; }
	void forceMatrix(const TeMatrix4x4 &matrix);
	byte getFaceMaterial(uint idx);
	TeMesh::Mode getMode() const;
	bool hasAlpha(uint idx);
	bool hasColor() const { return !_colors.empty(); }
	bool hasUvs() const { return !_uvs.empty(); }
	unsigned short index(uint num) const { return _indexes[num]; }
	TeMaterial *material(uint idx);
	const TeMaterial *material(uint idx) const;
	void materialIndex(uint idx, byte val);
	byte materialIndex(uint idx) const { return _materialIndexes[idx]; }
	void matrixIndex(uint num, unsigned short val);
	unsigned short matrixIndex(uint num) const { return _matricies[num]; }
	TeVector3f32 normal(uint idx) const;

	TeMesh &operator=(const TeMesh &other) = default;

	void optimizeVerticies();
	void resizeUpdatedTables(unsigned long newSize);

	void setColor(const TeColor &col) override;
	void setColor(uint idx, const TeColor &col);
	void setConf(unsigned long vertexCount, unsigned long indexCount, enum Mode mode, unsigned int materialCount, unsigned int materialIndexCount);
	void setIndex(unsigned int idx, unsigned int val);
	void setNormal(unsigned int idx, const TeVector3f32 &val);
	void setTextureUV(unsigned int idx, const TeVector2f32 &val);
	void setVertex(unsigned int idx, const TeVector3f32 &val);
	void sortFaces();

	void update(const Common::Array<TeMatrix4x4> *matricies1, const Common::Array<TeMatrix4x4> *matricies2);
	void update(TeIntrusivePtr<TeModelVertexAnimation> vertexanim);
	void updateTo(const Common::Array<TeMatrix4x4> *matricies1, const Common::Array<TeMatrix4x4> *matricies2,
				Common::Array<TeVector3f32> &verts, Common::Array<TeVector3f32> &normals);

	TeVector2f32 textureUV(uint idx) const { return _uvs[idx]; }
	TeVector3f32 vertex(uint idx) const;

	uint numIndexes() const { return _indexes.size(); }
	uint numVerticies() const { return _verticies.size(); }
	bool shouldDrawMaybe() const { return _shouldDraw; }
	uint gltexenvMode() const { return _gltexEnvMode; }

	void setShouldDraw(bool val) { _shouldDraw = val; }
	void setglTexEnv(unsigned int val) { _gltexEnvMode = val; }
	void setHasAlpha(bool val) { _hasAlpha = val; }

	Common::Array<TeMaterial> &materials() { return _materials; }

private:
	Common::Array<unsigned char> _materialIndexes;
	Common::Array<TeVector3f32> _verticies;
	Common::Array<TeVector3f32> _normals;
	Common::Array<TeVector3f32> _updatedVerticies;
	Common::Array<TeVector3f32> _updatedNormals;
	Common::Array<TeVector2f32> _uvs;
	Common::Array<unsigned short> _indexes;
	Common::Array<unsigned short> _faceCounts;
	Common::Array<unsigned short> _matricies;
	Common::Array<TeColor> _colors;
	Common::Array<TeMaterial> _materials;

	unsigned int _glMeshMode;

	bool _matrixForced;
	TeMatrix4x4 _forceMatrix;
	bool _hasAlpha;
	uint _initialMaterialIndexCount;
	bool _drawWires;
	bool _shouldDraw;

	unsigned int _gltexEnvMode;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_MESH_H
