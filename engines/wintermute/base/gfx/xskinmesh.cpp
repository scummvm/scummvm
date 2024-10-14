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

#include "engines/wintermute/base/gfx/xskinmesh.h"
#include "engines/wintermute/base/gfx/xmath.h"

#include "common/array.h"
#include "math/matrix4.h"

namespace Wintermute {

struct MeshData {
	uint32 _numVertices;
	uint32 _numPolyFaces;
	uint32 _numTriFaces;
	DXVector3 *_vertices;
	uint32 *_numTriPerFace;
	uint32 *_indices;
	uint32 _fvf;
	uint32 _numNormals;
	DXVector3 *_normals;
	uint32 *_normalIndices;
	DXVector3 *_vertexNormals;
	DXVector2 *_texCoords;
	DXColorValue *_vertexColors;
	uint32 _numMaterials;
	DXMaterial *_materials;
	uint32 *_materialIndices;
	DXSkinInfo *_skinInfo;
	uint32 _boneCount;
	uint32 _skinWeightsInfoCount;
};

uint32 DXGetFVFVertexSize(uint32 fvf) {
	uint32 size = 0;

	if (fvf & DXFVF_XYZ)
		size += sizeof(DXVector3);
	if (fvf & DXFVF_NORMAL)
		size += sizeof(DXVector3);
	if (fvf & DXFVF_DIFFUSE)
		size += sizeof(DXVector4);
	if (fvf & DXFVF_TEX1)
		size += sizeof(DXVector2);

	return size;
}

bool DXComputeBoundingBox(DXVector3 *pfirstposition, uint32 numvertices, uint32 dwstride, DXVector3 *pmin, DXVector3 *pmax) {
	DXVector3 vec;
	uint32 i;

	if (!pfirstposition || !pmin || !pmax )
		return false;

	*pmin = *pfirstposition;
	*pmax = *pmin;

	for (i = 0; i < numvertices; i++) {
		vec = *((DXVector3 *)((byte *)pfirstposition + dwstride * i));

		if (vec._x < pmin->_x)
			pmin->_x = vec._x;
		if (vec._x > pmax->_x)
			pmax->_x = vec._x;

		if (vec._y < pmin->_y)
			pmin->_y = vec._y;
		if (vec._y > pmax->_y)
			pmax->_y = vec._y;

		if (vec._z < pmin->_z)
			pmin->_z = vec._z;
		if (vec._z > pmax->_z)
			pmax->_z = vec._z;
	}

	return true;
}

static bool createMesh(uint32 numFaces, uint32 numVertices, uint32 fvf, DXMesh **mesh) {
	if (!mesh)
		return false;

	auto object = new DXMesh;
	if (!object)
		return false;

	if (!object->create(numFaces, numVertices, fvf)) {
		delete object;
		return false;
	}

	*mesh = object;

	return true;
}

static bool createSkinInfo(uint32 vertexCount, uint32 fvf, uint32 boneCount, DXSkinInfo **skinInfo) {
	if (!skinInfo)
		return false;

	auto skin = new DXSkinInfo();
	if (!skin)
		return false;

	if (!skin->create(vertexCount, fvf, boneCount)) {
		delete skin;
		return false;
	}

	*skinInfo = skin;

	return true;
}

bool DXMesh::create(uint32 numFaces, uint32 numVertices, uint32 fvf) {
	_numFaces = numFaces;
	_numVertices = numVertices;
	_fvf = fvf;
	_vertexSize = DXGetFVFVertexSize(fvf);
	_attribTable._size = 0;
	_attribTable._ptr = nullptr;
	_vertexBuffer = DXBuffer(numVertices * _vertexSize);
	_indexBuffer = DXBuffer(numFaces * 3 * sizeof(uint32));
	_attribBuffer = DXBuffer(numFaces * sizeof(uint32));
	if (!_vertexBuffer.ptr() || !_indexBuffer.ptr() || !_attribBuffer.ptr()) {
		destroy();
		return false;
	}

	return true;
}

void DXMesh::destroy() {
	_numFaces = 0;
	_numVertices = 0;
	_fvf = 0;
	_vertexSize = 0;
	_attribTable._size = 0;
	_vertexBuffer.free();
	_indexBuffer.free();
	_attribBuffer.free();
	delete[] _attribTable._ptr;
	_attribTable._ptr = nullptr;
}

int DXMesh::compareVertexKeys(const void *a, const void *b) {
	const DXVertexMetadata *left = static_cast<const DXVertexMetadata *>(a);
	const DXVertexMetadata *right = static_cast<const DXVertexMetadata *>(b);
	if (left->_key == right->_key)
		return 0;
	return left->_key < right->_key ? -1 : 1;
}

bool DXMesh::generateAdjacency(uint32 *adjacency) {
	uint32 i;

	if (!adjacency)
		return false;

	byte *vertices = const_cast<byte *>(_vertexBuffer.ptr());
	const uint32 *indices = (const uint32 *)_indexBuffer.ptr();

	uint32 bufferSize = _numFaces * 3 * sizeof(uint32) + _numVertices * sizeof(DXVertexMetadata);
	uint32 *sharedIndices = new uint32[bufferSize];
	if (!sharedIndices)
		return false;
	DXVertexMetadata *sortedVertices = (DXVertexMetadata *)(sharedIndices + _numFaces * 3);

	for (i = 0; i < _numVertices; i++) {
		DXVector3 *vertex = (DXVector3 *)(vertices + _vertexSize * i);
		sortedVertices[i]._firstSharedIndex = -1;
		sortedVertices[i]._key = vertex->_x + vertex->_y + vertex->_z;
		sortedVertices[i]._vertexIndex = i;
	}
	for (i = 0; i < _numFaces * 3; i++) {
		uint32 *firstSharedIndex = &sortedVertices[indices[i]]._firstSharedIndex;
		sharedIndices[i] = *firstSharedIndex;
		*firstSharedIndex = i;
		adjacency[i] = -1;
	}
	qsort(sortedVertices, _numVertices, sizeof(DXVertexMetadata), compareVertexKeys);

	for (i = 0; i < _numVertices; i++) {
		DXVertexMetadata *sortedVertexA = &sortedVertices[i];
		DXVector3 *vertex_a = (DXVector3 *)(vertices + sortedVertexA->_vertexIndex * _vertexSize);
		uint32 sharedIndexA = sortedVertexA->_firstSharedIndex;

		while (sharedIndexA != (uint32)-1) {
			uint32 j = i;
			uint32 sharedIndexB = sharedIndices[sharedIndexA];
			struct DXVertexMetadata *sortedVertexB = sortedVertexA;

			while (true) {
				while (sharedIndexB != (uint32)-1) {
					uint32 baseA = (sharedIndexA / 3) * 3;
					uint32 baseB = (sharedIndexB / 3) * 3;
					bool adjacent = true;
					int k;

					for (k = 0; k < 3; k++) {
						if (adjacency[baseB + k] == sharedIndexA / 3) {
							adjacent = true;
							break;
						}
					}
					if (!adjacent) {
						for (k = 1; k <= 2; k++) {
							uint32 vertex_index_a = baseA + (sharedIndexA + k) % 3;
							uint32 vertex_index_b = baseB + (sharedIndexB + (3 - k)) % 3;
							adjacent = indices[vertex_index_a] == indices[vertex_index_b];
							if (!adjacent) {
								DXVector3 delta = {0.0f, 0.0f, 0.0f};
								float lengthSq;

								DXVec3Subtract(&delta, (DXVector3 *)(vertices + indices[vertex_index_a] * _vertexSize), (DXVector3 *)(vertices + indices[vertex_index_b] * _vertexSize));
								lengthSq = DXVec3Length(&delta);
								adjacent = lengthSq == 0.0f;
							}
							if (adjacent) {
								uint32 adjA = baseA + 2 - (vertex_index_a + sharedIndexA + 1) % 3;
								uint32 adjB = baseB + 2 - (vertex_index_b + sharedIndexB + 1) % 3;
								if (adjacency[adjA] == (uint32)-1 && adjacency[adjB] == (uint32)-1) {
									adjacency[adjA] = baseB / 3;
									adjacency[adjB] = baseA / 3;
									break;
								}
							}
						}
					}

					sharedIndexB = sharedIndices[sharedIndexB];
				}
				while (++j < _numVertices) {
					DXVector3 *vertexB;

					sortedVertexB++;
					if (sortedVertexB->_key - sortedVertexA->_key > 0.0f) {
						j = _numVertices;
						break;
					}
					vertexB = (DXVector3 *)(vertices + sortedVertexB->_vertexIndex * _vertexSize);
					if (fabsf(vertex_a->_x - vertexB->_x) <= 0.0f &&
						fabsf(vertex_a->_y - vertexB->_y) <= 0.0f &&
						fabsf(vertex_a->_z - vertexB->_z) <= 0.0f) {
						break;
					}
				}
				if (j >= _numVertices)
					break;
				sharedIndexB = sortedVertexB->_firstSharedIndex;
			}

			sortedVertexA->_firstSharedIndex = sharedIndices[sortedVertexA->_firstSharedIndex];
			sharedIndexA = sortedVertexA->_firstSharedIndex;
		}
	}

	delete[] sharedIndices;
	return true;
}

bool DXMesh::cloneMesh(DXMesh **cloneMeshOut) {
	DXMesh *clonedMesh;

	if (!cloneMeshOut)
		return false;

	if (!createMesh(_numFaces, _numVertices, _fvf, &clonedMesh))
		return false;

	memcpy(clonedMesh->_vertexBuffer.ptr(), _vertexBuffer.ptr(), _vertexBuffer.size());
	memcpy(clonedMesh->_indexBuffer.ptr(), _indexBuffer.ptr(), _indexBuffer.size());
	memcpy(clonedMesh->_attribBuffer.ptr(), _attribBuffer.ptr(), _attribBuffer.size());

	if (_attribTable._size) {
		clonedMesh->_attribTable._size = _attribTable._size;
		clonedMesh->_attribTable._ptr = new DXAttributeRange[_attribTable._size];
		if (!clonedMesh->_attribTable._ptr) {
			delete clonedMesh;
			return false;
		}
		memcpy(clonedMesh->_attribTable._ptr, _attribTable._ptr, _attribTable._size * sizeof(DXAttributeRange));
	}

	*cloneMeshOut = clonedMesh;

	return true;
}

static uint32 countAttributes(const uint32 *attribBuffer, uint32 numFaces) {
	uint32 last_attribute = attribBuffer[0];
	uint32 attribTableSize = 1;
	for (uint32 i = 1; i < numFaces; i++) {
		if (attribBuffer[i] != last_attribute) {
			last_attribute = attribBuffer[i];
			attribTableSize++;
		}
	}
	return attribTableSize;
}

static void fillAttributeTable(const uint32 *attribBuffer, uint32 numfaces, const uint32 *indices, DXAttributeRange *attribTable) {
	uint32 attribTableSize = 0;
	uint32 lastAttribute = attribBuffer[0];
	uint32 minVertex, maxVertex, i, j;

	attribTable[0]._attribId = lastAttribute;
	attribTable[0]._faceStart = 0;
	minVertex = (uint32)-1;
	maxVertex = 0;
	for (i = 0; i < numfaces; i++) {
		if (attribBuffer[i] != lastAttribute) {
			lastAttribute = attribBuffer[i];
			attribTable[attribTableSize]._faceCount = i - attribTable[attribTableSize]._faceStart;
			attribTable[attribTableSize]._vertexStart = minVertex;
			attribTable[attribTableSize]._vertexCount = maxVertex - minVertex + 1;
			attribTableSize++;
			attribTable[attribTableSize]._attribId = attribBuffer[i];
			attribTable[attribTableSize]._faceStart = i;
			minVertex = (uint32)-1;
			maxVertex = 0;
		}
		for (j = 0; j < 3; j++) {
			uint32 vertex_index = indices[i * 3 + j];
			if (vertex_index < minVertex)
				minVertex = vertex_index;
			if (vertex_index > maxVertex)
				maxVertex = vertex_index;
		}
	}
	attribTable[attribTableSize]._faceCount = i - attribTable[attribTableSize]._faceStart;
	attribTable[attribTableSize]._vertexStart = minVertex;
	attribTable[attribTableSize]._vertexCount = maxVertex - minVertex + 1;
	attribTableSize++;
}

bool DXSkinInfo::create(uint32 vertexCount, uint32 fvf, uint32 boneCount) {
	_numVertices = vertexCount;
	_numBones = boneCount;
	_fvf = fvf;

	_bones = new DXBone[boneCount];
	if (!_bones) {
		return false;
	}
	memset(_bones, 0, boneCount * sizeof(DXBone));
	return true;
}

void DXSkinInfo::destroy() {
	delete[] _bones;
	_bones = nullptr;
}

bool DXSkinInfo::updateSkinnedMesh(const DXMatrix *boneTransforms, void *srcVertices, void *dstVertices) {
	uint32 vertexSize = DXGetFVFVertexSize(_fvf);
	uint32 normalOffset = sizeof(DXVector3);
	uint32 i, j;

	for (i = 0; i < _numVertices; i++) {
		DXVector3 *position = (DXVector3 *)((byte *)dstVertices + vertexSize * i);
		position->_x = 0.0f;
		position->_y = 0.0f;
		position->_z = 0.0f;
	}

	for (i = 0; i < _numBones; i++) {
		//DXMatrix boneInverse, matrix;

		Math::Matrix4 boneMatrix;
		boneMatrix.setData(boneTransforms[i]._m4x4);

		//DXMatrixInverse(&boneInverse, NULL, &_bones[i]._transform);
		//DXMatrixMultiply(&matrix, &boneTransforms[i], &boneInverse);
		//DXMatrixMultiply(&matrix, &matrix, &_bones[i]._transform);

		for (j = 0; j < _bones[i]._numInfluences; j++) {
			Math::Vector3d position;
			DXVector3 *positionSrc = (DXVector3 *)((byte *)srcVertices + vertexSize * _bones[i]._vertices[j]);
			DXVector3 *positionDst = (DXVector3 *)((byte *)dstVertices + vertexSize * _bones[i]._vertices[j]);
			float weight = _bones[i]._weights[j];

			position.set(positionSrc->_x, positionSrc->_y, positionSrc->_z);
			boneMatrix.transform(&position, true);
			//DXVec3TransformCoord(&position, positionSrc, &matrix);

			positionDst->_x += weight * position.x();
			positionDst->_y += weight * position.y();
			positionDst->_z += weight * position.z();
		}
	}

	if (_fvf & DXFVF_NORMAL) {
		for (i = 0; i < _numVertices; i++) {
			DXVector3 *normal = (DXVector3 *)((byte *)dstVertices + vertexSize * i + normalOffset);
			normal->_x = 0.0f;
			normal->_y = 0.0f;
			normal->_z = 0.0f;
		}

		for (i = 0; i < _numBones; i++) {
			//DXMatrix boneInverse, matrix;

			Math::Matrix4 boneMatrix;
			boneMatrix.setData(boneTransforms[i]._m4x4);
			boneMatrix.transpose();
			boneMatrix.inverse();

			//DXMatrixInverse(&boneInverse, nullptr, &_bones[i]._transform);
			//DXMatrixMultiply(&matrix, &_bones[i]._transform, &boneTransforms[i]);

			for (j = 0; j < _bones[i]._numInfluences; j++) {
				Math::Vector3d normal;
				DXVector3 *normalSrc = (DXVector3 *)((byte *)srcVertices + vertexSize * _bones[i]._vertices[j] + normalOffset);
				DXVector3 *normalDst = (DXVector3 *)((byte *)dstVertices + vertexSize * _bones[i]._vertices[j] + normalOffset);
				float weight = _bones[i]._weights[j];

				//DXVec3TransformNormal(&normal, normalSrc, &boneInverse);
				//DXVec3TransformNormal(&normal, &normal, &matrix);
				normal.set(normalSrc->_x, normalSrc->_y, normalSrc->_z);
				boneMatrix.transform(&normal, true);
				normalDst->_x += weight * normal.x();
				normalDst->_y += weight * normal.y();
				normalDst->_z += weight * normal.z();
			}
		}

		for (i = 0; i < _numVertices; i++) {
			DXVector3 *normalDest = (DXVector3 *)((byte *)dstVertices + (i * vertexSize) + normalOffset);
			if ((normalDest->_x != 0.0f) && (normalDest->_y != 0.0f) && (normalDest->_z != 0.0f)) {
				DXVec3Normalize(normalDest, normalDest);
			}
		}
	}

	return true;
}

bool DXSkinInfo::setBoneName(uint32 boneIdx, const char *name) {
	if (boneIdx >= _numBones || !name)
		return false;

	uint32 size = strlen(name) + 1;
	char *newName = new char[size];
	if (!newName)
		return false;
	memcpy(newName, name, size);
	delete[] _bones[boneIdx]._name;
	_bones[boneIdx]._name = newName;

	return true;
}

bool DXSkinInfo::setBoneInfluence(uint32 boneIdx, uint32 numInfluences, const uint32 *vertices, const float *weights) {
	DXBone *bone;
	uint32 *newVertices = NULL;
	float *newWeights = NULL;

	if (boneIdx >= _numBones || !vertices || !weights) {
		return false;
	}

	if (numInfluences) {
		newVertices = new uint32[numInfluences];
		if (!newVertices)
			return false;
		newWeights = new float[numInfluences];
		if (!newWeights) {
			delete[] newVertices;
			return false;
		}
		memcpy(newVertices, vertices, numInfluences * sizeof(*vertices));
		memcpy(newWeights, weights, numInfluences * sizeof(*weights));
	}
	bone = &_bones[boneIdx];
	bone->_numInfluences = numInfluences;
	delete[] bone->_vertices;
	delete[] bone->_weights;
	bone->_vertices = newVertices;
	bone->_weights = newWeights;

	return true;
}

DXBone *DXSkinInfo::getBone(uint32 boneIdx) {
	return &_bones[boneIdx];
}

bool DXSkinInfo::setBoneOffsetMatrix(uint32 boneIdx, const float *boneTransform) {
	if (boneIdx >= _numBones || !boneTransform)
		return false;

	for (int m = 0; m < 16; m++) {
		_bones[boneIdx]._transform._m4x4[m] = boneTransform[m];
	}

	// mirror at orign
	_bones[boneIdx]._transform._m[3][2] *= -1.0f;

	// mirror base vectors
	_bones[boneIdx]._transform._m[0][2] *= -1.0f;
	_bones[boneIdx]._transform._m[1][2] *= -1.0f;

	// change handedness
	_bones[boneIdx]._transform._m[2][0] *= -1.0f;
	_bones[boneIdx]._transform._m[2][1] *= -1.0f;

	return true;
}

static bool parseVertexDuplicationIndices(XFileData &fileData, struct MeshData */*meshData*/) {
	auto vertexDuplObj = fileData.getXVertexDuplicationIndicesObject();
	if (!vertexDuplObj) {
		return false;
	}

	// skipping this data
	return true;
}

static bool parseFVFData(XFileData &fileData, struct MeshData */*meshData*/) {
	auto vfvDataObj = fileData.getXFVFDataObject();
	if (!vfvDataObj) {
		return false;
	}

	// FVFData suppose contains proper layout for declared FVF
	// instead they are not complete and duplicated to already loaded
	// so skipping this data
	return true;
}

static bool parseDeclData(XFileData &fileData, struct MeshData *meshData) {
	uint32 i, vertexSize = 0;
	uint32 normalOffset = -1;
	uint32 textureOffset = -1;
	//uint32 tangentOffset = -1;
	//uint32 binormalOffset = -1;

	auto declObj = fileData.getXDeclDataObject();
	if (!declObj) {
		return false;
	}

	for (i = 0; i < declObj->_numElements; ++i) {
		switch (declObj->_elements[i]._usage) {
		case DXDECLUSAGE_NORMAL:
			assert(!(meshData->_fvf & DXFVF_NORMAL));
			normalOffset = vertexSize;
			meshData->_fvf |= DXFVF_NORMAL;
			break;
		case DXDECLUSAGE_TEXCOORD:
			assert(!(meshData->_fvf & DXFVF_TEX1));
			textureOffset = vertexSize;
			meshData->_fvf |= DXFVF_TEX1;
			break;
		case DXDECLUSAGE_TANGENT:
			//tangentOffset = vertexSize;
			break;
		case DXDECLUSAGE_BINORMAL:
			//binormalOffset = vertexSize;
			break;
		default:
			error("parseDeclData() Error: not handled DeclUsage: %d", declObj->_elements[i]._usage);
		}
		switch (declObj->_elements[i]._type) {
		case DXDECLTYPE_FLOAT2:
			vertexSize += 2;
			break;
		case DXDECLTYPE_FLOAT3:
			vertexSize += 3;
			break;
		default:
			error("parseDeclData() Error: not handled DeclType: %d", declObj->_elements[i]._type);
		}
	}

	if (vertexSize * meshData->_numVertices != declObj->_numData) {
		return false;
	}

	if (meshData->_fvf & DXFVF_NORMAL) {
		if (!meshData->_indices)
			return false;
		delete[] meshData->_normals;
		uint32 numFaceIndices = meshData->_numPolyFaces * 2 + meshData->_numTriFaces;
		meshData->_numNormals = meshData->_numVertices;
		meshData->_normals = new DXVector3[meshData->_numNormals];
		meshData->_normalIndices = new uint32[numFaceIndices];
		if (!meshData->_normals || !meshData->_normalIndices) {
			return false;
		}
		memcpy(meshData->_normalIndices, meshData->_indices, numFaceIndices * sizeof(uint32));
	}

	if (meshData->_fvf & DXFVF_TEX1) {
		delete[] meshData->_texCoords;
		meshData->_texCoords = new DXVector2[meshData->_numVertices];
		if (!meshData->_texCoords) {
			return false;
		}
	}

	for (i = 0; i < meshData->_numVertices; ++i) {
		if (meshData->_fvf & DXFVF_NORMAL) {
			float *vertexNormalData = reinterpret_cast<float *>(declObj->_data + vertexSize * i + normalOffset);
			meshData->_normals[i]._x = vertexNormalData[0];
			meshData->_normals[i]._y = vertexNormalData[1];
			meshData->_normals[i]._z = vertexNormalData[2];
			DXVec3Normalize(&meshData->_normals[i], &meshData->_normals[i]);
		}

		if (meshData->_fvf & DXFVF_TEX1) {
			float *vertexTextureCoordsData = reinterpret_cast<float *>(declObj->_data + vertexSize * i + textureOffset);
			meshData->_texCoords[i]._x = vertexTextureCoordsData[0];
			meshData->_texCoords[i]._y = vertexTextureCoordsData[1];
		}

		if (textureOffset != (uint32)-1) {
			//float *vertexTangentData = reinterpret_cast<float *>(declObj->_data + vertexSize * i + tangentOffset);
		}

		if (textureOffset != (uint32)-1) {
			//float *vertexBinormalData = reinterpret_cast<float *>(declObj->_data + vertexSize * i + binormalOffset);
		}
	}

	return true;
}

static bool parseSkinWeightsInfo(XFileData &fileData, struct MeshData *meshData) {
	uint32 influenceCount;
	const char *name;
	uint32 index = meshData->_skinWeightsInfoCount;

	if (!meshData->_skinInfo) {
		return false;
	}

	auto skinWeightsObj = fileData.getXSkinWeightsObject();
	if (!skinWeightsObj) {
		return false;
	}

	name = skinWeightsObj->_transformNodeName;
	influenceCount = skinWeightsObj->_numWeights;

	if (meshData->_skinInfo->setBoneName(index, name)) {
		if (meshData->_skinInfo->setBoneInfluence(index, influenceCount, skinWeightsObj->_vertexIndices, skinWeightsObj->_weights)) {
			if (meshData->_skinInfo->setBoneOffsetMatrix(index, skinWeightsObj->_matrixOffset)) {
				++meshData->_skinWeightsInfoCount;
				return true;
			}
		}
	}

	return false;
}

static bool parseSkinMeshHeader(XFileData &fileData, struct MeshData *meshData) {
	if (meshData->_skinInfo) {
		return false;
	}

	auto skinMeshHeaderObj = fileData.getXSkinMeshHeaderObject();
	if (!skinMeshHeaderObj) {
		return false;
	}

	meshData->_boneCount = skinMeshHeaderObj->_nBones;
	return createSkinInfo(meshData->_numVertices, meshData->_fvf, meshData->_boneCount, &meshData->_skinInfo);
}

static bool parseTextureFilename(XFileData &fileData, char *filenameOut) {
	auto textureNameObj = fileData.getXTextureFilenameObject();
	if (!textureNameObj) {
		return false;
	}

	Common::strlcpy(filenameOut, textureNameObj->_filename, XMAX_NAME_LEN);

	return true;
}

static bool parseMaterial(XFileData &fileData, DXMaterial *material) {
	XFileData child;
	XClassType type;
	uint32 nbChildren, i;

	auto materialObj = fileData.getXMaterialObject();
	if (!materialObj) {
		return false;
	}

	material->_diffuse.color._r = materialObj->_colorR;
	material->_diffuse.color._g = materialObj->_colorG;
	material->_diffuse.color._b = materialObj->_colorB;
	material->_diffuse.color._a = materialObj->_colorA;
	material->_power = materialObj->_power;
	material->_specular.color._r = materialObj->_specularR;
	material->_specular.color._g = materialObj->_specularG;
	material->_specular.color._b = materialObj->_specularB;
	material->_specular.color._a = 1.0f;
	material->_emissive.color._r = materialObj->_emissiveR;
	material->_emissive.color._g = materialObj->_emissiveG;
	material->_emissive.color._b = materialObj->_emissiveB;
	material->_emissive.color._a = 1.0f;
	material->_ambient.color._r = 0.0f;
	material->_ambient.color._g = 0.0f;
	material->_ambient.color._b = 0.0f;
	material->_ambient.color._a = 1.0f;
	material->_textureFilename[0] = '\0';

	if (!fileData.getChildren(nbChildren)) {
		return false;
	}

	for (i = 0; i < nbChildren; i++) {
		if (!fileData.getChild(i, child)) {
			return false;
		}
		if (!child.getType(type)) {
			return false;
		}

		if (type == XClassType::kXClassTextureFilename) {
			if (!parseTextureFilename(child, material->_textureFilename)) {
				return false;
			}
		}
	}
	return true;
}

static void destroyMaterials(struct MeshData *meshData) {
	delete[] meshData->_materials;
	meshData->_materials = nullptr;
	delete[] meshData->_materialIndices;
	meshData->_materialIndices = nullptr;
	meshData->_numMaterials = 0;
}

static bool parseMaterialList(XFileData &fileData, struct MeshData *meshData) {
	XFileData child;
	XClassType type;
	uint32 nbChildren, materialCount, i;

	destroyMaterials(meshData);

	auto materialListObj = fileData.getXMeshMaterialListObject();
	if (!materialListObj) {
		return false;
	}

	materialCount = materialListObj->_nMaterials;

	if (materialListObj->_numFaceIndexes != meshData->_numPolyFaces) {
		return false;
	}
	for (i = 0; i < meshData->_numPolyFaces; ++i) {
		if (materialListObj->_faceIndexes[i] >= materialCount) {
			return false;
		}
	}

	meshData->_materials = new DXMaterial[materialCount];
	meshData->_materialIndices = new uint32[meshData->_numPolyFaces];
	if (!meshData->_materials || !meshData->_materialIndices) {
		return false;
	}
	for (i = 0; i < meshData->_numPolyFaces; ++i) {
		meshData->_materialIndices[i] = materialListObj->_faceIndexes[i];
	}

	if (!fileData.getChildren(nbChildren)) {
		return false;
	}

	for (i = 0; i < nbChildren; i++) {
		if (!fileData.getChild(i, child)) {
			return false;
		}
		if (!child.getType(type)) {
			return false;
		}

		if (type == XClassType::kXClassMaterial) {
			if (meshData->_numMaterials >= materialCount) {
				return false;
			}
			if (!parseMaterial(child, &meshData->_materials[meshData->_numMaterials++])) {
				return false;
			}
		}
		// missing referenced material
		if (type == XClassType::kXClassUnknown) {
			materialCount--;
		}
	}
	if (materialCount != meshData->_numMaterials) {
		return false;
	}

	return true;
}

static bool parseTextureCoords(XFileData &fileData, struct MeshData *meshData) {
	uint32 i;

	delete[] meshData->_texCoords;
	meshData->_texCoords = nullptr;

	auto textureCordsObj = fileData.getXMeshTextureCoordsObject();
	if (!textureCordsObj) {
		return false;
	}

	if (textureCordsObj->_numTextureCoords != meshData->_numVertices) {
		return false;
	}

	meshData->_texCoords = new DXVector2[meshData->_numVertices];
	if (!meshData->_texCoords) {
		return false;
	}
	for (i = 0; i < meshData->_numVertices; i++) {
		meshData->_texCoords[i]._x = textureCordsObj->_textureCoords[i]._u;
		meshData->_texCoords[i]._y = textureCordsObj->_textureCoords[i]._v;
	}

	meshData->_fvf |= DXFVF_TEX1;

	return true;
}

static bool parseVertexColors(XFileData &fileData, struct MeshData *meshData) {
	uint32 i;

	delete[] meshData->_vertexColors;
	meshData->_vertexColors = nullptr;

	auto colorsObj = fileData.getXMeshVertexColorsObject();
	if (!colorsObj) {
		return false;
	}
	uint32 colorCount = colorsObj->_numVertexColors;

	meshData->_vertexColors = new DXColorValue[meshData->_numVertices];
	if (!meshData->_vertexColors) {
		return false;
	}

	for (i = 0; i < meshData->_numVertices; i++) {
		meshData->_vertexColors[i].color._r = 1.0f;
		meshData->_vertexColors[i].color._g = 1.0f;
		meshData->_vertexColors[i].color._b = 1.0f;
		meshData->_vertexColors[i].color._a = 0.0f;
	}

	for (i = 0; i < colorCount; ++i) {
		DXColorValue color;
		uint32 index = colorsObj->_vertexColors[i]._index;
		if (index >= meshData->_numVertices) {
			return false;
		}
		color.color._r = colorsObj->_vertexColors[i]._indexColorR;
		color.color._g = colorsObj->_vertexColors[i]._indexColorG;
		color.color._b = colorsObj->_vertexColors[i]._indexColorB;
		color.color._a = colorsObj->_vertexColors[i]._indexColorA;
		meshData->_vertexColors[index].color._r = MIN(1.0f, MAX(0.0f, color.color._r));
		meshData->_vertexColors[index].color._g = MIN(1.0f, MAX(0.0f, color.color._g));
		meshData->_vertexColors[index].color._b = MIN(1.0f, MAX(0.0f, color.color._b));
		meshData->_vertexColors[index].color._a = MIN(1.0f, MAX(0.0f, color.color._a));
	}

	meshData->_fvf |= DXFVF_DIFFUSE;

	return true;
}

static bool parseNormals(XFileData &fileData, struct MeshData *meshData) {
	uint32 numFaceIndices = meshData->_numPolyFaces * 2 + meshData->_numTriFaces;
	uint32 i, j;

	auto normalsObj = fileData.getXMeshNormalsObject();
	if (!normalsObj) {
		return false;
	}

	delete[] meshData->_normals;
	meshData->_normals = nullptr;

	meshData->_fvf |= DXFVF_NORMAL;
	meshData->_numNormals = normalsObj->_numNormals;
	meshData->_normals = new DXVector3[meshData->_numNormals];
	meshData->_normalIndices = new uint32[numFaceIndices];
	if (!meshData->_normals || !meshData->_normalIndices) {
		return false;
	}

	memcpy(meshData->_normals, normalsObj->_normals, meshData->_numNormals * sizeof(DXVector3));
	for (i = 0; i < meshData->_numNormals; i++) {
		DXVec3Normalize(&meshData->_normals[i], &meshData->_normals[i]);
	}

	if (normalsObj->_numFaceNormals != meshData->_numPolyFaces) {
		return false;
	}

	uint32 index = 0;
	for (i = 0; i < meshData->_numPolyFaces; i++) {
		uint32 count = normalsObj->_faceNormals[i]._numFaceVertexIndices;
		if (count != meshData->_numTriPerFace[i] + 2) {
			return false;
		}

		for (j = 0; j < count; j++) {
			uint32 normalIndex = normalsObj->_faceNormals[i]._faceVertexIndices[j];
			if (normalIndex >= meshData->_numNormals) {
				return false;
			}
			meshData->_normalIndices[index++] = normalIndex;
		}
	}

	return true;
}

static bool parseMesh(XFileData *fileData, struct MeshData *meshData) {
	XFileData child;
	XClassType type;
	uint32 childCount;
	uint32 i, j;
	bool result = true;

	meshData->_skinWeightsInfoCount = 0;

	XMeshObject *meshObj = fileData->getXMeshObject();
	if (!meshObj) {
		return false;
	}

	meshData->_numVertices = meshObj->_numVertices;
	meshData->_numPolyFaces = meshObj->_numFaces;
	meshData->_numTriFaces = 0;
	for (i = 0; i < meshData->_numPolyFaces; i++) {
		meshData->_numTriFaces += meshObj->_faces[i]._numFaceVertexIndices - 2;
	}

	meshData->_fvf = DXFVF_XYZ;

	meshData->_vertices = new DXVector3[meshData->_numVertices];
	meshData->_numTriPerFace = new uint32[meshData->_numPolyFaces];
	meshData->_indices = new uint32[meshData->_numTriFaces + meshData->_numPolyFaces * 2];
	if (!meshData->_vertices || !meshData->_numTriPerFace || !meshData->_indices) {
		return false;
	}

	memcpy(meshData->_vertices, meshObj->_vertices, meshData->_numVertices * sizeof(DXVector3));

	uint32 index = 0;
	for (i = 0; i < meshData->_numPolyFaces; i++) {
		uint32 count = meshObj->_faces[i]._numFaceVertexIndices;
		meshData->_numTriPerFace[i] = count - 2;
		for (j = 0; j < count; j++) {
			meshData->_indices[index++] = meshObj->_faces[i]._faceVertexIndices[j];
		}
	}

	if (!fileData->getChildren(childCount))
		return false;

	for (i = 0; i < childCount; i++) {
		if (!fileData->getChild(i, child))
			return false;
		if (!child.getType(type))
			return false;
		switch (type) {
		case XClassType::kXClassMeshNormals:
			result = parseNormals(child, meshData);
			break;
		case XClassType::kXClassMeshVertexColors:
			result = parseVertexColors(child, meshData);
			break;
		case XClassType::kXClassMeshTextureCoords:
			result = parseTextureCoords(child, meshData);
			break;
		case XClassType::kXClassMeshMaterialList:
			result = parseMaterialList(child, meshData);
			break;
		case XClassType::kXClassSkinMeshHeader:
			result = parseSkinMeshHeader(child, meshData);
			break;
		case XClassType::kXClassSkinWeights:
			result = parseSkinWeightsInfo(child, meshData);
			break;
		case XClassType::kXClassDeclData:
			result = parseDeclData(child, meshData);
			break;
		case XClassType::kXClassFVFData:
			result = parseFVFData(child, meshData);
			break;
		case XClassType::kXClassVertexDuplicationIndices:
			result = parseVertexDuplicationIndices(child, meshData);
			break;
		default:
			break;
		}
		if (!result)
			return false;
	}

	if (meshData->_skinInfo && (meshData->_skinWeightsInfoCount != meshData->_boneCount)) {
		return false;
	}

	if (!meshData->_skinInfo) {
		result = createSkinInfo(meshData->_numVertices, meshData->_fvf, meshData->_boneCount, &meshData->_skinInfo);
		if (!result)
			return false;
	}

	return true;
}

static void cleanupMeshData(MeshData *meshData, bool releaseSkin = true) {
	delete[] meshData->_vertices;
	delete[] meshData->_numTriPerFace;
	delete[] meshData->_indices;
	delete[] meshData->_normals;
	delete[] meshData->_normalIndices;
	delete[] meshData->_vertexNormals;
	destroyMaterials(meshData);
	delete[] meshData->_texCoords;
	delete[] meshData->_vertexColors;
	if (releaseSkin)
		delete meshData->_skinInfo;
}

bool DXLoadSkinMesh(XFileData *fileData, DXBuffer &materialsOut, uint32 &numMaterialsOut, DXSkinInfo **skinInfoOut, DXMesh **meshOut) {
	MeshData meshData{};
	DXMesh *mesh;
	uint32 i;

	bool result = parseMesh(fileData, &meshData);
	if (!result) {
		cleanupMeshData(&meshData, true);
		return false;
	}
	if (meshData._numVertices == 0) {
		createMesh(meshData._numTriFaces, meshData._numVertices, meshData._fvf, &mesh);
		*meshOut = mesh;
		numMaterialsOut = meshData._numMaterials;
		materialsOut = DXBuffer(meshData._numMaterials * sizeof(DXMaterial));
		*skinInfoOut = meshData._skinInfo;
		cleanupMeshData(&meshData, false);
		return true;
	}

	uint32 totalVertices = meshData._numVertices;
	if (meshData._fvf & DXFVF_NORMAL) {
		meshData._vertexNormals = new DXVector3[meshData._numVertices];
		memset(meshData._vertexNormals, 0, meshData._numVertices * sizeof(DXVector3));
		uint32 numFaceIndices = meshData._numPolyFaces * 2 + meshData._numTriFaces;
		for (i = 0; i < numFaceIndices; i++) {
			uint32 vertexIndex = meshData._indices[i];
			uint32 normalIndex = meshData._normalIndices[i];
			assert(vertexIndex < meshData._numVertices);
			assert(normalIndex < meshData._numNormals);
			meshData._vertexNormals[vertexIndex]._x = meshData._normals[normalIndex]._x;
			meshData._vertexNormals[vertexIndex]._y = meshData._normals[normalIndex]._y;
			meshData._vertexNormals[vertexIndex]._z = meshData._normals[normalIndex]._z;
		}
	}

	result = createMesh(meshData._numTriFaces, totalVertices, meshData._fvf, &mesh);
	if (!result) {
		cleanupMeshData(&meshData);
		delete mesh;
		return false;
	}

	float *vertices = (float *)mesh->getVertexBuffer().ptr();
	float *outPtr = vertices;
	for (i = 0; i < meshData._numVertices; i++) {
		if (meshData._fvf & DXFVF_XYZ) {
			*outPtr++ = meshData._vertices[i]._x;
			*outPtr++ = meshData._vertices[i]._y;
			*outPtr++ = meshData._vertices[i]._z;
		}
		if (meshData._fvf & DXFVF_NORMAL) {
			*outPtr++ = meshData._vertexNormals[i]._x;
			*outPtr++ = meshData._vertexNormals[i]._y;
			*outPtr++ = meshData._vertexNormals[i]._z;
		}
		if (meshData._fvf & DXFVF_DIFFUSE) {
			*outPtr++ = meshData._vertexColors[i].color._r;
			*outPtr++ = meshData._vertexColors[i].color._g;
			*outPtr++ = meshData._vertexColors[i].color._b;
			*outPtr++ = meshData._vertexColors[i].color._a;
		}
		if (meshData._fvf & (DXFVF_TEX1)) {
			*outPtr++ = meshData._texCoords[i]._x;
			*outPtr++ = meshData._texCoords[i]._y;
		}
	}


	uint32 *indices = (uint32 *)mesh->getIndexBuffer().ptr();
	uint32 *indexInPtr = meshData._indices;
	for (i = 0; i < meshData._numPolyFaces; i++) {
		uint32 count = meshData._numTriPerFace[i];
		uint32 firstIndex = *indexInPtr++;
		// 1 -> 1 -> 3
		// 2 -> 2 -> 2
		// 3 -> 3 -> 1
		// 1 -> 4 -> 6
		// 3 -> 5 -> 5
		// 4 -> 6 -> 4
		while (count--) {
			indices[2] = firstIndex;
			indices[1] = *indexInPtr++;
			indices[0] = *indexInPtr;
			indices += 3;
		}
		indexInPtr++;
	}

	if (meshData._materialIndices) {
		uint32 index = 0;
		uint32 *attribBuffer = (uint32 *)mesh->getAtribBuffer().ptr();
		for (i = 0; i < meshData._numPolyFaces; i++) {
			uint32 count = meshData._numTriPerFace[i];
			while (count--)
				attribBuffer[index++] = meshData._materialIndices[i];
		}

		uint32 attribTableSize = countAttributes(attribBuffer, meshData._numTriFaces);
		auto attribTable = new DXAttributeRange[attribTableSize];
		if (!attribTable) {
			cleanupMeshData(&meshData);
			delete mesh;
			return false;
		}
		auto rangeTable = mesh->getAttributeTable();
		rangeTable->_size = attribTableSize;
		rangeTable->_ptr = attribTable;

		indices = (uint32 *)mesh->getIndexBuffer().ptr();
		fillAttributeTable(attribBuffer, meshData._numTriFaces, indices, attribTable);
	}

	uint32 bufferSize = meshData._numMaterials * sizeof(DXMaterial);
	DXBuffer materials = DXBuffer(bufferSize);
	if (!materials.ptr()) {
		cleanupMeshData(&meshData);
		delete mesh;
		return false;
	}
	memcpy(materials.ptr(), meshData._materials, meshData._numMaterials * sizeof(DXMaterial));


	*meshOut = mesh;
	numMaterialsOut = meshData._numMaterials;
	materialsOut = materials;
	*skinInfoOut = meshData._skinInfo;

	cleanupMeshData(&meshData, false);

	return result;
}

} // namespace Wintermute
