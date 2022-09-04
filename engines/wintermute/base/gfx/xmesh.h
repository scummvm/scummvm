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

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_XMESH_H
#define WINTERMUTE_XMESH_H

#include "engines/wintermute/base/base_named_object.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/coll_templ.h"

#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Wintermute {

class BaseSprite;
class FrameNode;
class Material;
class XModel;
class ShadowVolume;
class VideoTheoraPlayer;
class XFileLexer;

struct SkinWeights {
	Common::String _boneName;
	Math::Matrix4 _offsetMatrix;
	BaseArray<uint32> _vertexIndices;
	BaseArray<float> _vertexWeights;
};

class XMesh : public BaseNamedObject {
public:
	XMesh(BaseGame *inGame);
	virtual ~XMesh();

	virtual bool loadFromX(const Common::String &filename, XFileLexer &lexer, Common::Array<MaterialReference> &materialReferences);
	bool findBones(FrameNode *rootFrame);
	virtual bool update(FrameNode *parentFrame);
	virtual bool render(XModel *model) = 0;
	virtual bool renderFlatShadowModel() = 0;
	bool updateShadowVol(ShadowVolume *shadow, Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth);

	bool pickPoly(Math::Vector3d *pickRayOrig, Math::Vector3d *pickRayDir);

	Math::Vector3d _BBoxStart;
	Math::Vector3d _BBoxEnd;

	bool setMaterialSprite(const Common::String &matName, BaseSprite *sprite);
	bool setMaterialTheora(const Common::String &matName, VideoTheoraPlayer *theora);

	bool invalidateDeviceObjects();
	bool restoreDeviceObjects();

protected:
	static const int kVertexComponentCount = 8;
	static const int kPositionOffset = 5;
	static const int kTextureCoordOffset = 0;
	static const int kNormalOffset = 2;

	// anything which does not fit into 16 bits would we fine
	static const uint32 kNullIndex = 0xFFFFFFFF;

	bool parsePositionCoords(XFileLexer &lexer);
	bool parseFaces(XFileLexer &lexer, int faceCount, Common::Array<int> &indexCountPerFace);
	bool parseTextureCoords(XFileLexer &lexer);
	bool parseNormalCoords(XFileLexer &lexer);
	bool parseMaterials(XFileLexer &lexer, int faceCount, const Common::String &filename, Common::Array<MaterialReference> &materialReferences, const Common::Array<int> &indexCountPerFace);
	bool parseSkinWeights(XFileLexer &lexer);
	bool parseVertexDeclaration(XFileLexer &lexer);

	void updateBoundingBox();

	bool generateAdjacency();
	bool adjacentEdge(uint16 index1, uint16 index2, uint16 index3, uint16 index4);

	float *_vertexData;
	float *_vertexPositionData;
	float *_vertexNormalData;
	uint32 _vertexCount;
	Common::Array<uint16> _indexData;

	BaseArray<Math::Matrix4 *> _boneMatrices;
	BaseArray<SkinWeights> skinWeightsList;

	Common::Array<uint32> _adjacency;

	BaseArray<Material *> _materials;
	BaseArray<int> _indexRanges;
	BaseArray<int> _materialIndices;
	uint32 _numAttrs;

	// Wintermute3D used the ID3DXSKININFO interface
	// we will only store, whether this mesh is skinned at all
	// and factor out the necessary computations into some functions
	bool _skinnedMesh;
};

} // namespace Wintermute

#endif
