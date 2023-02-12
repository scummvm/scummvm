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

#include "engines/wintermute/base/gfx/3dshadow_volume.h"
#include "engines/wintermute/base/gfx/xmaterial.h"
#include "engines/wintermute/base/gfx/xskinmesh_loader.h"
#include "engines/wintermute/base/gfx/xframe_node.h"
#include "engines/wintermute/base/gfx/xfile_loader.h"
#include "engines/wintermute/base/gfx/xskinmesh_loader.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/math/math_util.h"

namespace Wintermute {

// define constant to make it available to the linker
const uint32 XSkinMeshLoader::kNullIndex;

XSkinMeshLoader::XSkinMeshLoader(XMesh *mesh) {
	_mesh = mesh;

	_vertexData = nullptr;
	_vertexPositionData = nullptr;
	_vertexNormalData = nullptr;
	_vertexCount = 0;
}

XSkinMeshLoader::~XSkinMeshLoader() {
	delete[] _vertexData;
	delete[] _vertexPositionData;
	delete[] _vertexNormalData;
}

bool XSkinMeshLoader::parsePositionCoords(XMeshObject *mesh) {
	for (uint i = 0; i < _vertexCount; ++i) {
		_vertexPositionData[i * 3 + 0] = mesh->_vertices[i]._x;
		_vertexPositionData[i * 3 + 1] = mesh->_vertices[i]._y;
		_vertexPositionData[i * 3 + 2] = mesh->_vertices[i]._z;
		for (int j = 0; j < 3; ++j) {
			_vertexData[i * kVertexComponentCount + kPositionOffset + j] = _vertexPositionData[i * 3 + j];
		}

		_vertexPositionData[i * 3 + 2] *= -1.0f;
		_vertexData[i * kVertexComponentCount + kPositionOffset + 2] *= -1.0f;
	}

	return true;
}

bool XSkinMeshLoader::parseFaces(XMeshObject *mesh, int faceCount, Common::Array<int> &indexCountPerFace) {
	for (int i = 0; i < faceCount; ++i) {
		XMeshFace *face = &mesh->_faces[i];
		int indexCount = face->_numFaceVertexIndices;
		if (indexCount == 3) {
			uint16 index1 = face->_faceVertexIndices[0];
			uint16 index2 = face->_faceVertexIndices[1];
			uint16 index3 = face->_faceVertexIndices[2];

			_indexData.push_back(index3);
			_indexData.push_back(index2);
			_indexData.push_back(index1);

			indexCountPerFace.push_back(3);
		} else if (indexCount == 4) {
			uint16 index1 = face->_faceVertexIndices[0];
			uint16 index2 = face->_faceVertexIndices[1];
			uint16 index3 = face->_faceVertexIndices[2];
			uint16 index4 = face->_faceVertexIndices[3];

			_indexData.push_back(index3);
			_indexData.push_back(index2);
			_indexData.push_back(index1);

			_indexData.push_back(index4);
			_indexData.push_back(index3);
			_indexData.push_back(index1);

			indexCountPerFace.push_back(6);
		} else {
			warning("XMeshLoader::parseFaces faces with more than four vertices are not supported");
			return false;
		}
	}

	return true;
}

bool XSkinMeshLoader::parseTextureCoords(XFileData *xobj) {
	XMeshTextureCoordsObject *texCoords = xobj->getXMeshTextureCoordsObject();
	if (!texCoords)
		return false;
	// should be the same as _vertexCount
	int textureCoordCount = texCoords->_numTextureCoords;

	for (int i = 0; i < textureCoordCount; ++i) {
		_vertexData[i * kVertexComponentCount + kTextureCoordOffset + 0] = texCoords->_textureCoords[i]._u;
		_vertexData[i * kVertexComponentCount + kTextureCoordOffset + 1] = texCoords->_textureCoords[i]._v;
	}

	return true;
}

bool XSkinMeshLoader::parseNormalCoords(XFileData *xobj) {
	XMeshNormalsObject *normals = xobj->getXMeshNormalsObject();
	if (!normals)
		return false;
	// should be the same as _vertex count
	uint vertexNormalCount = normals->_numNormals;
	//assert(vertexNormalCount == _vertexCount);

	Common::Array<float> vertexNormalData;
	vertexNormalData.resize(3 * vertexNormalCount);

	for (uint i = 0; i < vertexNormalCount; ++i) {
		vertexNormalData[i * 3 + 0] = normals->_normals[i]._x;
		vertexNormalData[i * 3 + 1] = normals->_normals[i]._y;
		// mirror z coordinate to change to OpenGL coordinate system
		vertexNormalData[i * 3 + 2] = -normals->_normals[i]._z;
	}

	uint faceNormalCount = normals->_numFaceNormals;
	Common::Array<int> faceNormals;

	for (uint i = 0; i < faceNormalCount; ++i) {
		XMeshFace *normalFace = &normals->_faceNormals[i];
		int indexCount = normalFace->_numFaceVertexIndices;

		if (indexCount == 3) {
			uint16 index1 = normalFace->_faceVertexIndices[0];
			uint16 index2 = normalFace->_faceVertexIndices[1];
			uint16 index3 = normalFace->_faceVertexIndices[2];

			faceNormals.push_back(index3);
			faceNormals.push_back(index2);
			faceNormals.push_back(index1);
		} else if (indexCount == 4) {
			uint16 index1 = normalFace->_faceVertexIndices[0];
			uint16 index2 = normalFace->_faceVertexIndices[1];
			uint16 index3 = normalFace->_faceVertexIndices[2];
			uint16 index4 = normalFace->_faceVertexIndices[3];

			faceNormals.push_back(index3);
			faceNormals.push_back(index2);
			faceNormals.push_back(index1);

			faceNormals.push_back(index4);
			faceNormals.push_back(index3);
			faceNormals.push_back(index1);
		} else {
			warning("XMeshLoader::parseNormalCoords faces with more than four vertices are not supported");
			return false;
		}
	}

	assert(_indexData.size() == faceNormals.size());

	for (uint i = 0; i < faceNormals.size(); ++i) {
		uint16 vertexIndex = _indexData[i];
		int normalIndex = faceNormals[i];

		for (int j = 0; j < 3; ++j) {
			_vertexData[vertexIndex * kVertexComponentCount + kNormalOffset + j] = vertexNormalData[3 * normalIndex + j];
			_vertexNormalData[3 * vertexIndex + j] = vertexNormalData[3 * normalIndex + j];
		}
	}

	return true;
}

bool XSkinMeshLoader::parseMaterials(XFileData *xobj, BaseGame *inGame,
	                                 int faceCount, const Common::String &filename,
	                                 Common::Array<MaterialReference> &materialReferences,
									 const Common::Array<int> &indexCountPerFace) {
	XMeshMaterialListObject *materialList = xobj->getXMeshMaterialListObject();
	if (!materialList)
		return false;

	// there can be unused materials inside a .X file
	// so this piece of information is probably useless
	// should be the same as faceCount
	int faceMaterialCount = materialList->_numFaceIndexes;
	assert(faceMaterialCount == faceCount);

	_indexRanges.push_back(0);
	int currentMaterialIndex = materialList->_faceIndexes[0];
	_materialIndices.push_back(currentMaterialIndex);

	int currentIndex = indexCountPerFace[0];

	for (int i = 1; i < faceMaterialCount; ++i) {
		int currentMaterialIndexTmp = materialList->_faceIndexes[i];

		if (currentMaterialIndex != currentMaterialIndexTmp) {
			currentMaterialIndex = currentMaterialIndexTmp;
			_indexRanges.push_back(currentIndex);
			_materialIndices.push_back(currentMaterialIndex);
		}

		currentIndex += indexCountPerFace[i];
	}

	_indexRanges.push_back(currentIndex);
	_mesh->_numAttrs = _indexRanges.size() - 1;

	uint numChildren = 0;
	xobj->getChildren(numChildren);

	for (uint32 i = 0; i < numChildren; i++) {
		XFileData xchildData;
		XClassType objectType;
		bool res = xobj->getChild(i, xchildData);
		if (res) {
			res = xchildData.getType(objectType);
			if (res) {
				if (xchildData.isReference()) {
					Common::String materialReference;
					xchildData.getName(materialReference);
					for (uint32 j = 0; j < materialReferences.size(); j++) {
						if (materialReferences[j]._name == materialReference) {
							_mesh->_materials.add(materialReferences[j]._material);
							break;
						}
					}
				} else if (objectType == kXClassMaterial) {
					Material *mat = new Material(inGame);
					mat->loadFromX(&xchildData, filename);
					_mesh->_materials.add(mat);
					MaterialReference materialReference;
					materialReference._material = mat;
					materialReferences.push_back(materialReference);
				}
			}
		}
	}

	return true;
}

bool XSkinMeshLoader::parseSkinWeights(XFileData *xobj) {
	XSkinWeightsObject *skinWeights = xobj->getXSkinWeightsObject();
	if (!skinWeights)
		return false;

	_skinWeightsList.resize(_skinWeightsList.size() + 1);
	SkinWeights &currSkinWeights = _skinWeightsList.back();

	currSkinWeights._boneName = skinWeights->_transformNodeName;

	int weightCount = skinWeights->_numWeights;
	currSkinWeights._vertexIndices.resize(weightCount);
	currSkinWeights._vertexWeights.resize(weightCount);

	for (int i = 0; i < weightCount; ++i) {
		currSkinWeights._vertexIndices[i] = skinWeights->_vertexIndices[i];
	}

	for (int i = 0; i < weightCount; ++i) {
		currSkinWeights._vertexWeights[i] = skinWeights->_weights[i];
	}

	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			currSkinWeights._offsetMatrix(c, r) = skinWeights->_matrixOffset[r * 4 + c];
		}
	}

	// mirror at orign
	currSkinWeights._offsetMatrix(2, 3) *= -1.0f;

	// mirror base vectors
	currSkinWeights._offsetMatrix(2, 0) *= -1.0f;
	currSkinWeights._offsetMatrix(2, 1) *= -1.0f;

	// change handedness
	currSkinWeights._offsetMatrix(0, 2) *= -1.0f;
	currSkinWeights._offsetMatrix(1, 2) *= -1.0f;

	return true;
}

bool XSkinMeshLoader::parseVertexDeclaration(XFileData *xobj) {
	XDeclDataObject *declData = xobj->getXDeclDataObject();
	if (!declData)
		return false;

	int vertexElementCount = declData->_numElements;

	// size of a vertex measured in four byte blocks
	int vertexSize = 0;
	int normalOffset = -1;
	int textureOffset = -1;

	for (int i = 0; i < vertexElementCount; ++i) {
		int type = declData->_elements[i]._type;
		int method = declData->_elements[i]._method;
		int usage = declData->_elements[i]._usage;
		int usageIndex = declData->_elements[i]._usageIndex;

		debug(2, "Vertex Element: Type: %i, Method: %i, Usage: %i, Usage index: %i", type, method, usage, usageIndex);

		// we only care about normal vectors and texture coords
		switch (usage) {
		case 3:
			normalOffset = vertexSize;
			break;
		case 5:
			textureOffset = vertexSize;
			break;
		}

		// This is a first guess, based on
		// https://docs.microsoft.com/en-us/windows/win32/direct3d9/vertexelement
		switch (type) {
		case 0:
			vertexSize += 1;
			warning("D3DDECLTYPE_FLOAT1 encountered in .X model");
			break;
		case 1:
			vertexSize += 2;
			break;
		case 2:
			vertexSize += 3;
			break;
		case 3:
			vertexSize += 4;
			warning("D3DDECLTYPE_FLOAT4 encountered in .X model");
			break;
		case 4:
			vertexSize += 1;
			warning("D3DDECLTYPE_D3DCOLOR encountered in .X model");
			break;
		case 5:
			vertexSize += 1;
			warning("D3DDECLTYPE_UBYTE4 encountered in .X model");
			break;
		case 6:
			vertexSize += 2;
			warning("D3DDECLTYPE_SHORT2 encountered in .X model");
			break;
		case 7:
			vertexSize += 4;
			warning("D3DDECLTYPE_SHORT4 encountered in .X model");
			break;
		case 8:
			vertexSize += 1;
			warning("D3DDECLTYPE_UBYTE4N encountered in .X model");
			break;
		case 9:
			vertexSize += 2;
			warning("D3DDECLTYPE_SHORT2N encountered in .X model");
			break;
		case 10:
			vertexSize += 4;
			warning("D3DDECLTYPE_SHORT4N encountered in .X model");
			break;
		case 11:
			vertexSize += 2;
			warning("D3DDECLTYPE_USHORT2N encountered in .X model");
			break;
		case 12:
			vertexSize += 4;
			warning("D3DDECLTYPE_USHORT4N encountered in .X model");
			break;
		case 13:
			vertexSize += 3;
			warning("D3DDECLTYPE_UDEC3 encountered in .X model");
			break;
		case 14:
			vertexSize += 3;
			warning("D3DDECLTYPE_DEC3N encountered in .X model");
			break;
		case 15:
			vertexSize += 2;
			warning("D3DDECLTYPE_FLOAT16_2 encountered in .X model");
			break;
		case 16:
			vertexSize += 4;
			warning("D3DDECLTYPE_FLOAT16_4 encountered in .X model");
			break;
		default:
			warning("Unknown type in vertex declaration encountered");
			break;
		}
	}

	int dataSize = declData->_numData;
	Common::Array<uint32> data;
	data.reserve(dataSize);

	for (int i = 0; i < dataSize; ++i) {
		data.push_back(declData->_data[i]);
	}

	assert(dataSize % vertexSize == 0);
	assert(dataSize / vertexSize == static_cast<int>(_vertexCount));

	for (uint i = 0; i < _vertexCount; ++i) {
		if (normalOffset != -1) {
			float *vertexNormalData = reinterpret_cast<float *>(data.data() + vertexSize * i + normalOffset);

			for (int j = 0; j < 3; ++j) {
				_vertexNormalData[3 * i + j] = vertexNormalData[j];
				_vertexData[kVertexComponentCount * i + kNormalOffset + j] = vertexNormalData[j];
			}
		}

		if (textureOffset != -1) {
			float *vertexTextureCoordsData = reinterpret_cast<float *>(data.data() + vertexSize * i + textureOffset);

			_vertexData[kVertexComponentCount * i + kTextureCoordOffset + 0] = vertexTextureCoordsData[0];
			_vertexData[kVertexComponentCount * i + kTextureCoordOffset + 1] = vertexTextureCoordsData[1];
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XSkinMeshLoader::generateAdjacency(Common::Array<uint32> &adjacency) {
	adjacency = Common::Array<uint32>(_indexData.size(), XSkinMeshLoader::kNullIndex);

	for (uint32 i = 0; i < _indexData.size() / 3; ++i) {
		for (uint32 j = i + 1; j < _indexData.size() / 3; ++j) {
			for (int edge1 = 0; edge1 < 3; ++edge1) {
				uint16 index1 = _indexData[i * 3 + edge1];
				uint16 index2 = _indexData[i * 3 + (edge1 + 1) % 3];

				for (int edge2 = 0; edge2 < 3; ++edge2) {
					uint16 index3 = _indexData[j * 3 + edge2];
					uint16 index4 = _indexData[j * 3 + (edge2 + 1) % 3];

					if (adjacency[i * 3 + edge1] == XSkinMeshLoader::kNullIndex && adjacency[j * 3 + edge2] == XSkinMeshLoader::kNullIndex && adjacentEdge(index1, index2, index3, index4)) {
						adjacency[i * 3 + edge1] = j;
						adjacency[j * 3 + edge2] = i;
						break;
					}
				}
			}
		}
	}

	return true;
}

bool XSkinMeshLoader::adjacentEdge(uint16 index1, uint16 index2, uint16 index3, uint16 index4) {
	Math::Vector3d vertex1(_vertexPositionData + 3 * index1);
	Math::Vector3d vertex2(_vertexPositionData + 3 * index2);
	Math::Vector3d vertex3(_vertexPositionData + 3 * index3);
	Math::Vector3d vertex4(_vertexPositionData + 3 * index4);

	// wme uses a function from the D3DX library, which takes in an epsilon for floating point comparison
	// wme passes in zero, so we just do a direct comparison
	if (vertex1 == vertex3 && vertex2 == vertex4) {
		return true;
	} else if (vertex1 == vertex4 && vertex2 == vertex3) {
		return true;
	}

	return false;
}

} // namespace Wintermute
