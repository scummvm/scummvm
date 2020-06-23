/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_MESH_X_H
#define WINTERMUTE_MESH_X_H

#include "engines/wintermute/base/base_named_object.h"
#include "engines/wintermute/coll_templ.h"
#include "graphics/opengl/system_headers.h"
#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Wintermute {

class BaseSprite;
class FrameNode;
class Material;
class ModelX;
class ShadowVolume;
class VideoTheoraPlayer;
class XFileLexer;

struct SkinWeights {
	Common::String _boneName;
	Math::Matrix4 _offsetMatrix;
	BaseArray<uint32> _vertexIndices;
	BaseArray<float> _vertexWeights;
};

class MeshX : public BaseNamedObject {
public:
	MeshX(BaseGame *inGame);
	virtual ~MeshX();

	bool loadFromX(const Common::String &filename, XFileLexer &lexer);
	bool findBones(FrameNode *rootFrame);
	bool update(FrameNode *parentFrame);
	bool render(ModelX *model);
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

	bool parsePositionCoords(XFileLexer &lexer);
	bool parseFaces(XFileLexer &lexer, int faceCount);
	bool parseTextureCoords(XFileLexer &lexer);
	bool parseNormalCoords(XFileLexer &lexer);
	bool parseMaterials(XFileLexer &lexer, int faceCount, const Common::String &filename);
	bool parseSkinWeights(XFileLexer &lexer);

	void updateBoundingBox();

	bool generateMesh();
	uint32 _numAttrs;
	uint32 _maxFaceInfluence;

	float *_vertexData;
	float *_vertexPositionData;
	uint32 _vertexCount;
	uint16 *_indexData;
	uint32 _indexCount;

	BaseArray<Math::Matrix4 *> _boneMatrices;
	BaseArray<SkinWeights> skinWeightsList;

	uint32 *_skinAdjacency;
	uint32 *_adjacency;

	BaseArray<Material *> _materials;
	BaseArray<int> _indexRanges;

	// Wintermute3D used the ID3DXSKININFO interface
	// we will only store, whether this mesh is skinned at all
	// and factor out the necessary computations into some functions
	bool _skinnedMesh;
};

} // namespace Wintermute

#endif
