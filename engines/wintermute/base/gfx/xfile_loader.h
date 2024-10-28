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
 * Partially based on XFile parser code from Wine sources.
 * Copyright 2008 Christian Costa
 */

#ifndef WINTERMUTE_XFILE_LOADER_H
#define WINTERMUTE_XFILE_LOADER_H

#include "common/str.h"
#include "common/stack.h"

namespace Wintermute {

enum XTokenType : uint16 {
	XTOKEN_ERROR        = 0xffff,
	XTOKEN_NONE         = 0,
	XTOKEN_NAME         = 1,
	XTOKEN_STRING       = 2,
	XTOKEN_INTEGER      = 3,
	XTOKEN_GUID         = 5,
	XTOKEN_INTEGER_LIST = 6,
	XTOKEN_FLOAT_LIST   = 7,
	XTOKEN_OBRACE       = 10,
	XTOKEN_CBRACE       = 11,
	XTOKEN_OPAREN       = 12,
	XTOKEN_CPAREN       = 13,
	XTOKEN_OBRACKET     = 14,
	XTOKEN_CBRACKET     = 15,
	XTOKEN_OANGLE       = 16,
	XTOKEN_CANGLE       = 17,
	XTOKEN_DOT          = 18,
	XTOKEN_COMMA        = 19,
	XTOKEN_SEMICOLON    = 20,
	XTOKEN_TEMPLATE     = 31,
	XTOKEN_WORD         = 40,
	XTOKEN_DWORD        = 41,
	XTOKEN_FLOAT        = 42,
	XTOKEN_DOUBLE       = 43,
	XTOKEN_CHAR         = 44,
	XTOKEN_UCHAR        = 45,
	XTOKEN_SWORD        = 46,
	XTOKEN_SDWORD       = 47,
	XTOKEN_VOID         = 48,
	XTOKEN_LPSTR        = 49,
	XTOKEN_UNICODE      = 50,
	XTOKEN_CSTRING      = 51,
	XTOKEN_ARRAY        = 52
};

#define XMAX_NAME_LEN   120
#define XMAX_STRING_LEN 500

struct XToken {
	XTokenType      _type;
	char            _textVal[XMAX_STRING_LEN];
	uint32          _integerVal;
	float           _floatVal;
};

struct XVector3 {
	float           _x;
	float           _y;
	float           _z;
};

struct XVector4 {
	float           _x;
	float           _y;
	float           _z;
	float           _w;
};

struct XCoords2d {
	float           _u;
	float           _v;
};

struct XMeshFace {
	uint32          _numFaceVertexIndices;
	uint32          _faceVertexIndices[4];
};

struct XTimedFloatKeys {
	float           _time;
	uint32          _numTfkeys;
	float           _tfkeys[16];
};

struct XIndexedColor {
	uint32          _index;
	float           _indexColorR;
	float           _indexColorG;
	float           _indexColorB;
	float           _indexColorA;
};

struct XVertexElement {
	uint32          _type;
	uint32          _method;
	uint32          _usage;
	uint32          _usageIndex;
};

struct XMeshMaterialListObject {
	uint32          _nMaterials;
	uint32          _numFaceIndexes;
	uint32          *_faceIndexes{};

	~XMeshMaterialListObject() {
		delete[] _faceIndexes;
	}
};

struct XVertexDuplicationIndicesObject {
	uint32          _nOriginalVertices;
	uint32          _numIndices;
	uint32          *_indices{};

	~XVertexDuplicationIndicesObject() {
		delete[] _indices;
	}
};

struct XSkinMeshHeaderObject{
	uint32          _nMaxSkinWeightsPerVertex;
	uint32          _nMaxSkinWeightsPerFace;
	uint32          _nBones;
};

struct XSkinWeightsObject {
	char            _transformNodeName[XMAX_NAME_LEN];
	uint32          _numVertexIndices;
	uint32          *_vertexIndices{};
	uint32          _numWeights;
	float           *_weights{};
	float           _matrixOffset[16];

	~XSkinWeightsObject() {
		delete[] _vertexIndices;
		delete[] _weights;
	}
};

struct XMeshObject {
	uint32          _numVertices;
    XVector3        *_vertices{};
	uint32          _numFaces;
	XMeshFace       *_faces{};

	~XMeshObject() {
		delete[] _vertices;
		delete[] _faces;
	}
};

struct XMeshNormalsObject {
	uint32          _numNormals;
    XVector3        *_normals{};
	uint32          _numFaceNormals;
	XMeshFace       *_faceNormals{};

	~XMeshNormalsObject() {
		delete[] _normals;
		delete[] _faceNormals;
	}
};

struct XMeshVertexColorsObject {
	uint32          _numVertexColors;
	XIndexedColor   *_vertexColors{};

	~XMeshVertexColorsObject() {
		delete[] _vertexColors;
	}
};

struct XMeshTextureCoordsObject {
	uint32          _numTextureCoords;
	XCoords2d       *_textureCoords{};

	~XMeshTextureCoordsObject() {
		delete[] _textureCoords;
	}
};

struct XMaterialObject {
	float           _colorR;
	float           _colorG;
	float           _colorB;
	float           _colorA;
	float           _power;
	float           _specularR;
	float           _specularG;
	float           _specularB;
	float           _emissiveR;
	float           _emissiveG;
	float           _emissiveB;
};

struct XTextureFilenameObject {
	char            _filename[XMAX_NAME_LEN];
};

struct XAnimTicksPerSecondObject {
	uint32          _animTicksPerSecond;
};

struct XAnimationSetObject{
};

struct XAnimationObject{
};

struct XAnimationKeyObject {
	uint32          _keyType;
	uint32          _numKeys;
	XTimedFloatKeys *_keys{};

	~XAnimationKeyObject() {
		delete[] _keys;
	}
};

struct XAnimationOptionsObject {
	uint32          _openclosed;
	uint32          _positionquality;
};

struct XFrameObject {
};

struct XFrameTransformMatrixObject {
	float           _frameMatrix[16];
};

struct XDeclDataObject {
	uint32          _numElements;
	XVertexElement  *_elements{};
	uint32          _numData;
	uint32          *_data{};

	~XDeclDataObject() {
		delete[] _elements;
		delete[] _data;
	}
};

struct XFVFDataObject {
	uint32          _dwFVF;
	uint32          _numData;
	uint32          *_data{};

	~XFVFDataObject() {
		delete[] _data;
	}
};

enum XClassType {
	kXClassUnknown = 0,
	kXClassAnimTicksPerSecond,
	kXClassFrameTransformMatrix,
	kXClassFrame,
	kXClassMesh,
	kXClassMeshNormals,
	kXClassMeshVertexColors,
	kXClassMeshTextureCoords,
	kXClassMeshMaterialList,
	kXClassVertexDuplicationIndices,
	kXClassMaterial,
	kXClassTextureFilename,
	kXClassSkinMeshHeader,
	kXClassSkinWeights,
	kXClassAnimationSet,
	kXClassAnimation,
	kXClassAnimationKey,
	kXClassAnimationOptions,
	kXClassDeclData,
	kXClassFVFData,
};

class XFileEnumObject;

class XObject {
	friend class XFileLoader;
	friend class XFileData;
	friend class XFileEnumObject;

private:

	Common::String _name;
	XClassType _classType{};
	void *_object{};
	XObject *_targetObject{};
	Common::Stack<XObject *> _children;

public:

	void deinit() {
		switch (_classType) {
		case kXClassAnimTicksPerSecond:
			delete (XAnimTicksPerSecondObject *)_object;
			break;
		case kXClassAnimationKey:
			delete (XAnimationKeyObject *)_object;
			break;
		case kXClassAnimation:
			delete (XAnimationObject *)_object;
			break;
		case kXClassAnimationOptions:
			delete (XAnimationOptionsObject *)_object;
			break;
		case kXClassAnimationSet:
			delete (XAnimationSetObject *)_object;
			break;
		case kXClassDeclData:
			delete (XDeclDataObject *)_object;
			break;
		case kXClassFrame:
			delete (XFrameObject *)_object;
			break;
		case kXClassFrameTransformMatrix:
			delete (XFrameTransformMatrixObject *)_object;
			break;
		case kXClassFVFData:
			delete (XFVFDataObject *)_object;
			break;
		case kXClassMaterial:
			delete (XMaterialObject *)_object;
			break;
		case kXClassMesh:
			delete (XMeshObject *)_object;
			break;
		case kXClassMeshMaterialList:
			delete (XMeshMaterialListObject *)_object;
			break;
		case kXClassMeshNormals:
			delete (XMeshNormalsObject *)_object;
			break;
		case kXClassMeshVertexColors:
			delete (XMeshVertexColorsObject *)_object;
			break;
		case kXClassMeshTextureCoords:
			delete (XMeshTextureCoordsObject *)_object;
			break;
		case kXClassSkinMeshHeader:
			delete (XSkinMeshHeaderObject *)_object;
			break;
		case kXClassSkinWeights:
			delete (XSkinWeightsObject *)_object;
			break;
		case kXClassVertexDuplicationIndices:
			delete (XVertexDuplicationIndicesObject *)_object;
			break;
		case kXClassTextureFilename:
			delete (XTextureFilenameObject *)_object;
			break;
		case kXClassUnknown:
			break;
		}
	}
};

class XFileLoader {
	friend class XFileEnumObject;

private:

	const int kCabBlockSize = 0x8000;
	const int kCabInputmax = kCabBlockSize + 12;

	bool _initialised{};
	XToken _currentToken{};
	byte *_decompBuffer{};
	byte *_buffer{};
	uint32 _bufferLeft;
	bool _isText;
	uint32 _listNbElements;
	bool _listTypeFloat;
	bool _listSeparator;
	bool _tokenPresent;

	Common::Stack<XObject *> _xobjects;

public:

	XFileLoader();
	~XFileLoader();
	bool load(byte *buffer, uint32 bufferSize);
	bool createEnumObject(XFileEnumObject &xobj);

private:

	void init();
	void deinit();

	FORCEINLINE bool readChar(char &c);
	FORCEINLINE void rewindBytes(uint32 size);
	bool readBytes(void *data, uint32 size);
	bool readLE16(uint16 *data);
	bool readLE32(uint32 *data);
	bool readBE32(uint32 *data);

	FORCEINLINE bool getInteger(uint32 &value);
	FORCEINLINE bool getFloat(float &value);
	FORCEINLINE bool getString(char *str, uint maxLen);
	FORCEINLINE bool skipSemicolonComma();

	FORCEINLINE bool isSpace(char c);
	FORCEINLINE bool isOperator(char c);
	FORCEINLINE bool isSeparator(char c);
	FORCEINLINE bool isPrimitiveType(XTokenType token);
	FORCEINLINE bool isGuid();
	FORCEINLINE bool isName();
	FORCEINLINE bool isFloat();
	FORCEINLINE bool isInteger();
	FORCEINLINE bool isString();
	FORCEINLINE bool isKeyword(const char *keyword, uint len);
	FORCEINLINE XTokenType getKeywordToken();
	FORCEINLINE XTokenType checkToken();
	XTokenType getToken();
	void parseToken();

	bool decompressMsZipData();

	bool parseHeader();

	bool parseTemplate();
	bool parseTemplateParts();
	bool parseTemplateOptionInfo();
	bool parseTemplateMembersList();
	XObject *resolveChildObject(XObject *object, const Common::String &referenceName);
	bool resolveObject(XObject *referenceObject, const Common::String &referenceName);
	bool parseObject(XObject *object);
	bool parseChildObjects(XObject *object);
	bool parseObjectParts(XObject *object);
};

class XFileData {
	friend class XFileEnumObject;

private:

	XObject *_xobject{};
	bool _reference{};

public:

	bool getChild(uint id, XFileData &child) {
		if (_xobject) {
			if (id < _xobject->_children.size()) {
				child._xobject = _xobject->_children[id];
				if (child._xobject->_targetObject) {
					child._xobject = child._xobject->_targetObject;
					child._reference = true;
				}
				return true;
			}
		}
		return false;
	}

	bool getChildren(uint32 &num) {
		if (_xobject) {
			num = _xobject->_children.size();
			return true;
		}
		return false;
	}

	bool getName(Common::String &name) {
		if (_xobject) {
			name = _xobject->_name;
			return true;
		}
		return false;
	}

	bool getType(XClassType &classType) {
		if (_xobject) {
			classType = _xobject->_classType;
			return true;
		}
		return false;
	}

	bool isReference() {
		if (_xobject) {
			return _reference;
		}
		return false;
	}

#define GET_OBJECT_FUNC(objectName) \
	objectName *get ## objectName() { \
		if (_xobject) \
			return static_cast<objectName *>(_xobject->_object); \
		else \
			return nullptr; \
	}

	GET_OBJECT_FUNC(XAnimTicksPerSecondObject)
	GET_OBJECT_FUNC(XAnimationKeyObject)
	GET_OBJECT_FUNC(XAnimationObject)
	GET_OBJECT_FUNC(XAnimationOptionsObject)
	GET_OBJECT_FUNC(XAnimationSetObject)
	GET_OBJECT_FUNC(XDeclDataObject)
	GET_OBJECT_FUNC(XFrameObject)
	GET_OBJECT_FUNC(XFrameTransformMatrixObject)
	GET_OBJECT_FUNC(XFVFDataObject)
	GET_OBJECT_FUNC(XMaterialObject)
	GET_OBJECT_FUNC(XMeshObject)
	GET_OBJECT_FUNC(XMeshMaterialListObject)
	GET_OBJECT_FUNC(XMeshNormalsObject)
	GET_OBJECT_FUNC(XMeshVertexColorsObject)
	GET_OBJECT_FUNC(XMeshTextureCoordsObject)
	GET_OBJECT_FUNC(XSkinMeshHeaderObject)
	GET_OBJECT_FUNC(XSkinWeightsObject)
	GET_OBJECT_FUNC(XVertexDuplicationIndicesObject)
	GET_OBJECT_FUNC(XTextureFilenameObject)
};

class XFileEnumObject {
	friend class XFileLoader;

private:

	XFileLoader *_file{};

public:

	bool getChild(uint id, XFileData &child) {
		if (_file) {
			if (id < _file->_xobjects.size()) {
				child._xobject = _file->_xobjects[id];
				return true;
			}
		}
		return false;
	}

	bool getChildren(uint32 &num) {
		if (_file) {
			num = _file->_xobjects.size();
			return true;
		}
		return false;
	}
};

} // namespace Wintermute

#endif
