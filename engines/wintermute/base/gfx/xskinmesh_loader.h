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

#ifndef WINTERMUTE_XSKINMESH_LOADER_H
#define WINTERMUTE_XSKINMESH_LOADER_H

#include "engines/wintermute/base/gfx/xmodel.h"

#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Wintermute {

class Material;
class XModel;
class XMesh;
class ShadowVolume;
class SkinMeshHelper;
class VideoTheoraPlayer;
struct XMeshObject;

struct SkinWeights {
	Common::String _boneName;
	Math::Matrix4 _offsetMatrix;
	BaseArray<uint32> _vertexIndices;
	BaseArray<float> _vertexWeights;
};

class XSkinMeshLoader {
	friend class XMesh;
	friend class XMeshOpenGL;
	friend class XMeshOpenGLShader;
	friend class SkinMeshHelper;

public:
	XSkinMeshLoader(XMesh *mesh, XMeshObject *meshObject);
	virtual ~XSkinMeshLoader();
	void loadMesh(const Common::String &filename, XFileData *xobj, Common::Array<MaterialReference> &materialReferences);

protected:
	static const int kVertexComponentCount = 8;
	static const int kPositionOffset = 5;
	static const int kTextureCoordOffset = 0;
	static const int kNormalOffset = 2;
	
	// anything which does not fit into 16 bits would we fine
	static const uint32 kNullIndex = 0xFFFFFFFF;
	
	bool generateAdjacency(Common::Array<uint32> &adjacency);
	bool adjacentEdge(uint16 index1, uint16 index2, uint16 index3, uint16 index4);

public:
	bool parsePositionCoords(XMeshObject *mesh);
	bool parseFaces(XMeshObject *mesh, int faceCount, Common::Array<int> &indexCountPerFace);
	bool parseTextureCoords(XFileData *xobj);
	bool parseNormalCoords(XFileData *xobj);
	bool parseMaterials(XFileData *xobj, BaseGame *inGame, int faceCount, const Common::String &filename,
                            Common::Array<MaterialReference> &materialReferences, const Common::Array<int> &indexCountPerFace);
	bool parseSkinWeights(XFileData *xobj);
	bool parseVertexDeclaration(XFileData *xobj);
	
protected:

	float *_vertexData;
	float *_vertexPositionData;
	float *_vertexNormalData;
	uint32 _vertexCount;
	Common::Array<uint16> _indexData;
	
	BaseArray<Math::Matrix4 *> _boneMatrices;
	BaseArray<SkinWeights> _skinWeightsList;
	
	BaseArray<int> _indexRanges;
	BaseArray<int> _materialIndices;

	XMesh *_mesh;
	XMeshObject *_meshObject;
};

} // namespace Wintermute

#endif
