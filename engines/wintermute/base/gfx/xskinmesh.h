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
 * Based on skin and mesh code from Wine sources.
 * Copyright (C) 2005 Henri Verbeet
 * Copyright (C) 2006 Ivan Gyurdiev
 * Copyright (C) 2009 David Adam
 * Copyright (C) 2010 Tony Wasserka
 * Copyright (C) 2011 Dylan Smith
 * Copyright (C) 2011 Michael Mc Donnell
 * Copyright (C) 2013 Christian Costa
 */

#ifndef WINTERMUTE_XSKINMESH_H
#define WINTERMUTE_XSKINMESH_H

#include "engines/wintermute/base/gfx/xbuffer.h"
#include "engines/wintermute/base/gfx/xfile_loader.h"
#include "engines/wintermute/base/gfx/xmath.h"

namespace Wintermute {

#define DXFVF_XYZ             0x0002
#define DXFVF_NORMAL          0x0010
#define DXFVF_DIFFUSE         0x0040
#define DXFVF_TEX1            0x0100

typedef enum {
	DXDECLUSAGE_NORMAL        = 3,
	DXDECLUSAGE_TEXCOORD      = 5,
	DXDECLUSAGE_TANGENT       = 6,
	DXDECLUSAGE_BINORMAL      = 7,
} DXDECLUSAGE;

typedef enum {
	DXDECLTYPE_FLOAT2         =  1,
	DXDECLTYPE_FLOAT3         =  2,
} DXDECLTYPE;

#if defined(SCUMMVM_USE_PRAGMA_PACK)
#pragma pack(4)
#endif

struct DXAttributeRange {
	uint32 _attribId;
	uint32 _faceStart;
	uint32 _faceCount;
	uint32 _vertexStart;
	uint32 _vertexCount;
};

struct DXAttributeRangeTable {
	uint32 _size;
	DXAttributeRange *_ptr;
};

typedef union {
	struct {
		float _r;
		float _g;
		float _b;
		float _a;
	} color;
	float _data[4];
} DXColorValue;

typedef struct {
	DXColorValue   _diffuse;
	DXColorValue   _ambient;
	DXColorValue   _specular;
	DXColorValue   _emissive;
	float          _power;
	char           _textureFilename[XMAX_NAME_LEN];
} DXMaterial;

struct DXBone {
	char *_name;
	DXMatrix _transform;
	uint32 _numInfluences;
	uint32 *_vertices;
	float *_weights;
};

#if defined(SCUMMVM_USE_PRAGMA_PACK)
#pragma pack()
#endif

class DXSkinInfo {
	uint32 _fvf;
	uint32 _numVertices{};
	uint32 _numBones{};
	DXBone *_bones{};

public:
	~DXSkinInfo() { destroy(); }
	bool create(uint32 vertexCount, uint32 fvf, uint32 boneCount);
	void destroy();
	uint32 getNumBones() { return _numBones; }
	bool setBoneName(uint32 boneIdx, const char *name);
	char *getBoneName(uint32 boneIdx) { return _bones[boneIdx]._name; }
	bool setBoneInfluence(uint32 boneIdx, uint32 numInfluences, const uint32 *vertices, const float *weights);
	DXBone *getBone(uint32 boneIdx);
	bool setBoneOffsetMatrix(uint32 boneIdx, const float *boneTransform);
	DXMatrix *getBoneOffsetMatrix(uint32 boneIdx) { return &_bones[boneIdx]._transform; }
	bool updateSkinnedMesh(const DXMatrix *boneTransforms, void *srcVertices, void *dstVertices);
};

class DXMesh {
	uint32 _numFaces;
	uint32 _numVertices;
	uint32 _fvf;
	uint32 _vertexSize;
	DXBuffer _vertexBuffer;
	DXBuffer _indexBuffer;
	DXBuffer _attribBuffer;
	DXAttributeRangeTable _attribTable;

	struct DXVertexMetadata {
		float _key;
		uint32 _vertexIndex;
		uint32 _firstSharedIndex;
	};

	static int compareVertexKeys(const void *a, const void *b);

public:
	~DXMesh() { destroy(); }
	bool create(uint32 numFaces, uint32 numVertices, uint32 fvf);
	void destroy();
	bool cloneMesh(DXMesh **cloneMeshOut);
	uint32 getNumFaces() { return _numFaces; }
	uint32 getNumVertices() { return _numVertices; }
	uint32 getFVF() { return _fvf; }
	DXBuffer getVertexBuffer() { return _vertexBuffer; }
	DXBuffer getIndexBuffer() { return _indexBuffer; }
	DXBuffer getAtribBuffer() { return _attribBuffer; }
	DXAttributeRangeTable *getAttributeTable() { return &_attribTable; }
	bool generateAdjacency(uint32 *adjacency);
};

bool DXLoadSkinMesh(XFileData *fileData, DXBuffer &materialsOut, uint32 &numMaterialsOut, DXSkinInfo **skinInfoOut, DXMesh **meshOut);
uint32 DXGetFVFVertexSize(uint32 fvf);
bool DXComputeBoundingBox(DXVector3 *pfirstposition, uint32 numvertices, uint32 dwstride, DXVector3 *pmin, DXVector3 *pmax);

} // namespace Wintermute

#endif
