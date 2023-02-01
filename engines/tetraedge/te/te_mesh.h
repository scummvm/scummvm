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

class TeModel;
class TeModelVertexAnimation;

class TeMesh : public Te3DObject2 {
public:
	TeMesh();

	virtual ~TeMesh() {};

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
	void facesPerMaterial(uint idx, unsigned short value);
	unsigned short facesPerMaterial(uint idx) const { return _faceCounts[idx]; }
	void forceMatrix(const TeMatrix4x4 &matrix);
	byte getFaceMaterial(uint idx);
	virtual uint32 getTexEnvMode() const = 0;
	virtual TeMesh::Mode getMode() const = 0;
	virtual void setMode(enum Mode mode) = 0;
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

	void optimizeVerticies();
	void resizeUpdatedTables(uint newSize);

	void setColor(const TeColor &col) override;
	void setColor(uint idx, const TeColor &col);
	void setConf(uint vertexCount, uint indexCount, enum Mode mode, uint materialCount, uint materialIndexCount);
	void setIndex(uint idx, uint val);
	void setNormal(uint idx, const TeVector3f32 &val);
	void setTextureUV(uint idx, const TeVector2f32 &val);
	void setVertex(uint idx, const TeVector3f32 &val);
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

	void setShouldDraw(bool val) { _shouldDraw = val; }
	virtual void setglTexEnvBlend() = 0;
	void setHasAlpha(bool val) { _hasAlpha = val; }

	Common::Array<TeMaterial> &materials() { return _materials; }
	void setUpdatedVertex(uint idx, const TeVector3f32 &val) { _updatedVerticies[idx] = val; }
	void setUpdatedNormal(uint idx, const TeVector3f32 &val) { _updatedNormals[idx] = val; }

	const TeVector3f32 &preUpdatedVertex(uint idx) const { return _verticies[idx]; }
	const TeVector3f32 &preUpdatedNormal(uint idx) const { return _normals[idx]; }

	static TeMesh *makeInstance();

protected:
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

	bool _matrixForced;
	TeMatrix4x4 _forcedMatrix;
	bool _hasAlpha;
	uint _initialMaterialIndexCount;
	bool _drawWires;
	bool _shouldDraw;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_MESH_H
