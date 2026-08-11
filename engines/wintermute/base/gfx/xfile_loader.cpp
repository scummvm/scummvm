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

#include "common/endian.h"
#include "common/str.h"
#include "common/util.h"
#include "common/compression/deflate.h"

#include "wintermute/base/gfx/xfile_loader.h"

namespace Wintermute {

typedef struct {
	const char *className;
	const XClassType type;
} XClassEntries;

// strings must be in lower case
static const XClassEntries gXClasses[] = {
	{ "skinweights",                 kXClassSkinWeights },
	{ "frame",                       kXClassFrame },
	{ "frametransformmatrix",        kXClassFrameTransformMatrix },
	{ "animationkey",                kXClassAnimationKey },
	{ "animationoptions",            kXClassAnimationOptions },
	{ "mesh",                        kXClassMesh },
	{ "meshnormals",                 kXClassMeshNormals },
	{ "animation",                   kXClassAnimation },
	{ "animationset",                kXClassAnimationSet },
	{ "meshvertexcolors",            kXClassMeshVertexColors },
	{ "meshtexturecoords",           kXClassMeshTextureCoords },
	{ "meshmateriallist",            kXClassMeshMaterialList },
	{ "vertexduplicationindices",    kXClassVertexDuplicationIndices },
	{ "material",                    kXClassMaterial },
	{ "texturefilename",             kXClassTextureFilename },
	{ "xskinmeshheader",             kXClassSkinMeshHeader },
	{ "animtickspersecond",          kXClassAnimTicksPerSecond },
	{ "decldata",                    kXClassDeclData },
	{ "fvfdata",                     kXClassFVFData },
};

// first string expected in lower case
FORCEINLINE static int XFileLoader_strncmp(const char *s1, const char *s2, uint n) {
	byte l1, l2;
	do {
		if (n-- == 0)
			return 0;

		l1 = (byte)*s1++;
		l2 = (byte)*s2++;
		l2 = tolower(l2);
	} while (l1 == l2 && l1 != 0);
	return l1 - l2;
}

// first string expected in lower case
FORCEINLINE static int XFileLoader_strcmp(const char *s1, const char *s2) {
	byte l1, l2;
	do {
		l1 = (byte)*s1++;
		l2 = (byte)*s2++;
		l2 = tolower(l2);
	} while (l1 == l2 && l1 != 0);
	return l1 - l2;
}

XFileLoader::XFileLoader() {
	init();
}

XFileLoader::~XFileLoader() {
	deinit();
}

void XFileLoader::init() {
	deinit();

	_decompBuffer = nullptr;
	_bufferLeft = 0;
	_tokenPresent = false;
	_isText = false;
	_listSeparator = false;
	_listTypeFloat = false;
	_listNbElements = 0;

	_initialised = true;
}

void XFileLoader::deinit() {
	delete _decompBuffer;
	for (uint i = 0; i < _xobjects.size(); i++) {
		if (_xobjects[i]->_object && !_xobjects[i]->_targetObject) {
			_xobjects[i]->deinit();
		}
		_xobjects[i]->_object = nullptr;
		delete _xobjects[i];
	}
	_xobjects.clear();
}

bool XFileLoader::createEnumObject(XFileEnumObject &xobj) {
	if (_initialised) {
		xobj._file = this;
		return true;
	}
	return false;
}

bool XFileLoader::isSpace(char c) {
	switch (c) {
	case ' ':
	case '\t':
	case 0x0D:
	case 0x0A:
	case 0x00:
		return true;
	default:
		return false;
	}
	return false;
}

bool XFileLoader::isOperator(char c) {
	switch (c) {
	case ',':
	case ';':
	case '{':
	case '}':
	case '[':
	case ']':
	case '(':
	case ')':
	case '<':
	case '>':
		return true;
	default:
		return false;
	}
	return false;
}

bool XFileLoader::isSeparator(char c) {
	return isSpace(c) || isOperator(c);
}

bool XFileLoader::isPrimitiveType(XTokenType token) {
	switch (token) {
	case XTOKEN_DWORD:
	case XTOKEN_FLOAT:
	case XTOKEN_WORD:
	case XTOKEN_CSTRING:
	case XTOKEN_DOUBLE:
	case XTOKEN_CHAR:
	case XTOKEN_UCHAR:
	case XTOKEN_SWORD:
	case XTOKEN_SDWORD:
	case XTOKEN_VOID:
	case XTOKEN_LPSTR:
	case XTOKEN_UNICODE:
		return true;
	default:
		return false;
	}
}

bool XFileLoader::readChar(char &c) {
	if (_bufferLeft == 0)
		return false;
	c = *_buffer;
	_buffer += 1;
	_bufferLeft -= 1;
	return true;
}

bool XFileLoader::readBytes(void *data, uint32 size) {
	if (_bufferLeft < size)
		return false;
	memcpy(data, _buffer, size);
	_buffer += size;
	_bufferLeft -= size;
	return true;
}

bool XFileLoader::readLE16(uint16 *data) {
	if (_bufferLeft < 2)
		return false;
	*data = READ_LE_UINT16(_buffer);
	_buffer += 2;
	_bufferLeft -= 2;
	return true;
}

bool XFileLoader::readLE32(uint32 *data) {
	if (_bufferLeft < 4)
		return false;
	*data = READ_LE_UINT32(_buffer);
	_buffer += 4;
	_bufferLeft -= 4;
	return true;
}

bool XFileLoader::readBE32(uint32 *data) {
	if (_bufferLeft < 4)
		return false;
	*data = READ_BE_UINT32(_buffer);
	_buffer += 4;
	_bufferLeft -= 4;
	return true;
}

void XFileLoader::rewindBytes(uint32 size) {
	_buffer -= size;
	_bufferLeft += size;
}

static struct keywords {
	const char *keyword;
	const uint len;
	const XTokenType token;
} XKeywords[] = {
	{ "dword", sizeof("dword") - 1, XTOKEN_DWORD },
	{ "float", sizeof("float") - 1, XTOKEN_FLOAT },
	{ "array", sizeof("array") - 1, XTOKEN_ARRAY },
	{ "template", sizeof("template") - 1, XTOKEN_TEMPLATE },
	{ "word", sizeof("word") - 1, XTOKEN_WORD },
	{ "cstring", sizeof("cstring") - 1, XTOKEN_CSTRING },
	{ "char", sizeof("char") - 1, XTOKEN_CHAR },
	{ "uchar", sizeof("uchar") - 1, XTOKEN_UCHAR },
	{ "sword", sizeof("sword") - 1, XTOKEN_SWORD },
	{ "sdword", sizeof("sdword") - 1, XTOKEN_SDWORD },
	{ "void", sizeof("void") - 1, XTOKEN_VOID },
	{ "string", sizeof("string") - 1, XTOKEN_LPSTR },
};

// string expected in lower case
bool XFileLoader::isKeyword(const char *keyword, uint len) {
	if (XFileLoader_strncmp(keyword, (char *)_buffer, len)) {
		return false;
	}
	_buffer += len;
	_bufferLeft -= len;

	char tmp;
	if (!readChar(tmp))
		return true;
	if (isSeparator(tmp)) {
		rewindBytes(1);
		return true;
	}

	rewindBytes(len + 1);
	return false;
}

XTokenType XFileLoader::getKeywordToken() {
	for (int i = 0; i < ARRAYSIZE(XKeywords); i++) {
		if (isKeyword(XKeywords[i].keyword, XKeywords[i].len))
			return XKeywords[i].token;
	}
	return XTOKEN_NONE;
}

bool XFileLoader::isGuid() {
	if (_bufferLeft < 38 || *_buffer != '<')
		return false;

	char tmp[50];
	uint32 pos = 1;
	tmp[0] = '<';
	while (pos < sizeof(tmp) - 2 && *(_buffer + pos) != '>') {
		tmp[pos] = *(_buffer + pos);
		pos++;
	}
	tmp[pos++] = '>';
	tmp[pos] = 0;
	if (pos != 38) {
		warning("XFileLoader: Wrong guid %s (%d)", tmp, pos);
		return false;
	}
	_buffer += pos;
	_bufferLeft -= pos;
	return true;
}

bool XFileLoader::isName() {
	char tmp[XMAX_STRING_LEN];
	uint32 pos = 0;
	char c;
	bool error = false;

	while (pos < _bufferLeft && !isSeparator(c = *(_buffer + pos))) {
		if (!(((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_') || (c == '-')))
			error = true;
		if (pos < sizeof(tmp))
			tmp[pos] = c;
		pos++;
	}
	tmp[MIN(pos, (uint32)sizeof(tmp) - 1)] = 0;

	if (error) {
		warning("XFileLoader: Wrong name %s", tmp);
		return false;
	}

	_buffer += pos;
	_bufferLeft -= pos;

	Common::strlcpy(_currentToken._textVal, tmp, XMAX_STRING_LEN);

	return true;
}

bool XFileLoader::isFloat() {
	char tmp[XMAX_STRING_LEN];
	uint32 pos = 0;
	char c;
	bool dot = false;

	while (pos < _bufferLeft && !isSeparator(c = *(_buffer + pos))) {
		if (!((!pos && (c == '-')) || ((c >= '0') && (c <= '9')) || (!dot && (c == '.'))))
			return false;
		if (c == '.')
			dot = true;
		if (pos < sizeof(tmp))
			tmp[pos] = c;
		pos++;
	}
	tmp[MIN(pos, (uint32)sizeof(tmp) - 1)] = 0;

	_buffer += pos;
	_bufferLeft -= pos;

	_currentToken._floatVal = atof(tmp);

	return true;
}

bool XFileLoader::isInteger() {
	char tmp[XMAX_STRING_LEN];
	uint32 pos = 0;
	char c;

	while (pos < _bufferLeft && !isSeparator(c = *(_buffer + pos))) {
		if (!((c >= '0') && (c <= '9')))
			return false;
		if (pos < sizeof(tmp))
			tmp[pos] = c;
		pos++;
	}
	tmp[MIN(pos, (uint32)sizeof(tmp) - 1)] = 0;

	_buffer += pos;
	_bufferLeft -= pos;

	_currentToken._integerVal = atoi(tmp);

	return true;
}

bool XFileLoader::isString() {
	char tmp[XMAX_STRING_LEN];
	uint32 pos = 0;
	char c;
	bool ok = false;

	if (*_buffer != '"')
		return false;

	while ((pos + 1) < _bufferLeft) {
		c = *(_buffer + pos + 1);
		if (c == '"') {
			ok = true;
			break;
		}
		if (pos < sizeof(tmp))
			tmp[pos] = c;
		pos++;
	}
	tmp[MIN((int32)pos, (int32)sizeof(tmp) - 1)] = 0;

	if (!ok) {
		warning("XFileLoader: Wrong string %s", tmp);
		return false;
	}

	_buffer += pos + 2;
	_bufferLeft -= pos + 2;

	Common::strlcpy(_currentToken._textVal, tmp, XMAX_STRING_LEN);

	return true;
}

void XFileLoader::parseToken() {
	if (_isText) {
		char current;

		while (true) {
			if (!readChar(current)) {
				_currentToken._type = XTOKEN_NONE;
				return;
			}
			if (isSpace(current)) {
				continue;
			} else if (current == '/' || current == '#') {
				if (current == '/') {
					if (!readChar(current)) {
						_currentToken._type = XTOKEN_ERROR;
						return;
					}
					if (current != '/') {
						_currentToken._type = XTOKEN_ERROR;
						warning("XFileLoader: Unknown token %c", current);
						return;
					}
				}
				while (current != 0xA) {
					if (!readChar(current)) {
						_currentToken._type = XTOKEN_ERROR;
						return;
					}
				}
				continue;
			}
			break;
		}

		switch (current) {
		case ';':
			_currentToken._type = XTOKEN_SEMICOLON;
			return;
		case ',':
			_currentToken._type = XTOKEN_COMMA;
			return;
		case '{':
			_currentToken._type = XTOKEN_OBRACE;
			return;
		case '}':
			_currentToken._type = XTOKEN_CBRACE;
			return;
		case '(':
			_currentToken._type = XTOKEN_OPAREN;
			return;
		case ')':
			_currentToken._type = XTOKEN_CPAREN;
			return;
		case '[':
			_currentToken._type = XTOKEN_OBRACKET;
			return;
		case ']':
			_currentToken._type = XTOKEN_CBRACKET;
			return;
		case '>':
			_currentToken._type = XTOKEN_CANGLE;
			return;
		case '.':
			_currentToken._type = XTOKEN_DOT;
			return;
		default:
			rewindBytes(1);
			_currentToken._type = getKeywordToken();
			if (_currentToken._type != XTOKEN_NONE) {
				return;
			} else if (isGuid()) {
				_currentToken._type = XTOKEN_GUID;
			} else if (isInteger()) {
				_currentToken._type = XTOKEN_INTEGER;
			} else if (isFloat()) {
				_currentToken._type = XTOKEN_FLOAT;
			} else if (isString()) {
				_currentToken._type = XTOKEN_STRING;
			} else if (isName()) {
				_currentToken._type = XTOKEN_NAME;
			} else {
				_currentToken._type = XTOKEN_ERROR;
				warning("XFileLoader: Unknown token %c", current);
			}
		}
	} else {
		if (!_listNbElements) {
			uint16 type;
			if (!readLE16(&type)) {
				_currentToken._type = XTOKEN_NONE;
				return;
			}
			_currentToken._type = (XTokenType)type;

			if (_currentToken._type == XTOKEN_INTEGER_LIST) {
				if (!readLE32(&_listNbElements)) {
					_currentToken._type = XTOKEN_ERROR;
					return;
				}
				_currentToken._type = XTOKEN_INTEGER;
				_listTypeFloat = false;
			} else if (_currentToken._type == XTOKEN_FLOAT_LIST) {
				if (!readLE32(&_listNbElements)) {
					_currentToken._type = XTOKEN_ERROR;
					return;
				}
				_currentToken._type = XTOKEN_FLOAT;
				_listTypeFloat = true;
			}
		}

		if (_listNbElements) {
			if (_listSeparator) {
				_listNbElements--;
				_listSeparator = false;
				_currentToken._type = XTOKEN_COMMA;
			} else {
				uint32 value;
				if (!readLE32(&value)) {
					_currentToken._type = XTOKEN_ERROR;
					return;
				}
				_listSeparator = true;
				if (_listTypeFloat) {
					_currentToken._type = XTOKEN_FLOAT;
					_currentToken._floatVal = *(float *)&value;
				} else {
					_currentToken._type = XTOKEN_INTEGER;
					_currentToken._integerVal = value;
				}
			}
			return;
		}

		switch (_currentToken._type) {
		case XTOKEN_NAME: {
				uint32 count;
				if (!readLE32(&count)) {
					_currentToken._type = XTOKEN_ERROR;
					return;
				}
				char name[XMAX_NAME_LEN];
				if (!readBytes(name, count)) {
					_currentToken._type = XTOKEN_ERROR;
					return;
				}
				name[count] = 0;
				assert(count < XMAX_NAME_LEN);
				Common::strlcpy(_currentToken._textVal, name, XMAX_NAME_LEN);
			}
			break;
		case XTOKEN_INTEGER: {
				uint32 integer;
				if (!readLE32(&integer)) {
					_currentToken._type = XTOKEN_ERROR;
					return;
				}
				_currentToken._integerVal = integer;
			}
			break;
		case XTOKEN_GUID: {
				byte classId[16];
				if (!readBytes(&classId, 16)) {
					_currentToken._type = XTOKEN_ERROR;
					return;
				}
			}
			break;
		case XTOKEN_STRING: {
				uint32 count;
				if (!readLE32(&count)) {
					_currentToken._type = XTOKEN_ERROR;
					return;
				}
				char string[XMAX_NAME_LEN];
				if (!readBytes(string, count)) {
					_currentToken._type = XTOKEN_ERROR;
					return;
				}
				string[count] = 0;
				assert(count < XMAX_NAME_LEN);
				Common::strlcpy(_currentToken._textVal, string, XMAX_NAME_LEN);
			}
			break;
		case XTOKEN_COMMA:
			break;
		case XTOKEN_SEMICOLON:
			break;
		case XTOKEN_OBRACE:
			break;
		case XTOKEN_CBRACE:
			break;
		case XTOKEN_DWORD:
			break;
		case XTOKEN_FLOAT:
			break;
		case XTOKEN_OBRACKET:
			break;
		case XTOKEN_CBRACKET:
			break;
		case XTOKEN_ARRAY:
			break;
		case XTOKEN_WORD:
			break;
		case XTOKEN_CSTRING:
			break;
		case XTOKEN_OPAREN:
			break;
		case XTOKEN_CPAREN:
			break;
		case XTOKEN_OANGLE:
			break;
		case XTOKEN_CANGLE:
			break;
		case XTOKEN_DOT:
			break;
		case XTOKEN_TEMPLATE:
			break;
		case XTOKEN_DOUBLE:
			break;
		case XTOKEN_CHAR:
			break;
		case XTOKEN_UCHAR:
			break;
		case XTOKEN_SWORD:
			break;
		case XTOKEN_SDWORD:
			break;
		case XTOKEN_VOID:
			break;
		case XTOKEN_LPSTR:
			break;
		case XTOKEN_UNICODE:
			break;
		default:
			_currentToken._type = XTOKEN_ERROR;
			warning("XFileLoader::nextToken: Unknown token encountered");
			return;
		}
	}
}

XTokenType XFileLoader::getToken() {
	if (_tokenPresent) {
		_tokenPresent = false;
		return _currentToken._type;
	}

	parseToken();

	return _currentToken._type;
}

XTokenType XFileLoader::checkToken() {
	if (_tokenPresent)
		return _currentToken._type;

	parseToken();
	_tokenPresent = true;

	return _currentToken._type;
}

bool XFileLoader::skipSemicolonComma() {
	if (checkToken() != XTOKEN_COMMA && checkToken() != XTOKEN_SEMICOLON) {
		return false;
	}
	while (checkToken() == XTOKEN_SEMICOLON)
		getToken();
	if (checkToken() == XTOKEN_COMMA)
		getToken();
	return true;
}

bool XFileLoader::getInteger(uint32 &value) {
	if (getToken() != XTOKEN_INTEGER) {
		return false;
	}
	value = _currentToken._integerVal;
	return skipSemicolonComma();
}

bool XFileLoader::getFloat(float &value) {
	if (getToken() != XTOKEN_FLOAT) {
		return false;
	}
	value = _currentToken._floatVal;
	return skipSemicolonComma();
}

bool XFileLoader::getString(char *str, uint maxLen) {
	if (getToken() != XTOKEN_STRING) {
		return false;
	}
	uint len = strlen(_currentToken._textVal);
	assert(maxLen > len);
	Common::strlcpy(str, _currentToken._textVal, maxLen);
	return skipSemicolonComma();
}

bool XFileLoader::decompressMsZipData() {
	bool error = false;

	byte *compressedBlock = new byte[kCabInputmax];
	byte *decompressedBlock = new byte[kCabBlockSize];

	uint32 decompressedSize = 0;
	if (!readLE32(&decompressedSize)) {
		error = true;
	} else {
		if (decompressedSize < 16) {
			delete[] compressedBlock;
			delete[] decompressedBlock;
			return false;
		}
		decompressedSize -= 16;
	}

	uint32 decompressedPos = 0;
	byte *decompressedData = new byte[decompressedSize];
	if (!decompressedData)
		error = true;

	while (!error && _bufferLeft) {
		uint16 uncompressedLen, compressedLen;
		if (!readLE16(&uncompressedLen) || !readLE16(&compressedLen)) {
			error = true;
			break;
		}

		if (_bufferLeft == 0) {
			break;
		}

		if (compressedLen > kCabInputmax || uncompressedLen > kCabBlockSize) {
			error = true;
			break;
		}

		if (!readBytes(compressedBlock, compressedLen)) {
			error = true;
			break;
		}

		if (compressedBlock[0] != 'C' || compressedBlock[1] != 'K') {
			error = true;
			break;
		}

		const byte *dict = decompressedPos ? decompressedBlock : nullptr;
		bool decRes = Common::inflateZlibHeaderless(decompressedBlock, uncompressedLen, compressedBlock + 2, compressedLen - 2, dict, kCabBlockSize);
		if (!decRes) {
			error = true;
			break;
		}

		memcpy(decompressedData + decompressedPos, decompressedBlock, uncompressedLen);
		decompressedPos += uncompressedLen;
	}
	if (decompressedSize != decompressedPos)
		error = true;

	delete[] compressedBlock;
	delete[] decompressedBlock;

	if (!error) {
		_decompBuffer = _buffer = decompressedData;
		_bufferLeft = decompressedSize;
		return true;
	}

	delete[] decompressedData;

	warning("XFileLoader: decompressMsZipData: Error decompressing data!");
	return false;
}

bool XFileLoader::parseHeader() {
	uint32 header[4];

	for (int i = 0; i < 4; i++) {
		if (!readBE32(&header[i])) {
			warning("XFileLoader: bad file");
			return false;
		}
	}

	if (header[0] != MKTAG('x','o','f',' ')) {
		warning("XFileLoader: bad file");
		return false;
	}

	if (header[1] != MKTAG('0','3','0','2') &&
	    header[1] != MKTAG('0','3','0','3')) {
		warning("XFileLoader: bad version");
		return false;
	}

	if (header[2] != MKTAG('b','i','n',' ') &&
	    header[2] != MKTAG('t','x','t',' ') &&
	    header[2] != MKTAG('b','z','i','p') &&
	    header[2] != MKTAG('t','z','i','p')) {
		warning("XFileLoader: file type unknown");
		return false;
	}

	if (header[3] != MKTAG('0','0','3','2') &&
	    header[3] != MKTAG('0','0','6','4')) {
		warning("XFileLoader: bad float size");
		return false;
	}

	if (header[3] == MKTAG('0','0','6','4')) {
		warning("XFileLoader: double float size is not supported");
		return false;
	}

	_isText = header[2] == MKTAG('t','x','t',' ') ||
	          header[2] == MKTAG('t','z','i','p');

	if (header[2] == MKTAG('b','z','i','p') ||
	    header[2] == MKTAG('t','z','i','p')) {
		if (!decompressMsZipData()) {
			return false;
		}
	}

	return true;
}

bool XFileLoader::parseTemplateOptionInfo() {
	if (checkToken() == XTOKEN_DOT) {
		getToken();
		if (getToken() != XTOKEN_DOT)
			return false;
		if (getToken() != XTOKEN_DOT)
			return false;
	} else {
		while (1) {
			if (getToken() != XTOKEN_NAME)
				return false;
			if (checkToken() == XTOKEN_GUID)
				getToken();
			if (checkToken() != XTOKEN_COMMA)
				break;
			getToken();
		}
	}
	return true;
}

bool XFileLoader::parseTemplateMembersList() {
	while (true) {
		bool array = false;
		if (checkToken() == XTOKEN_ARRAY) {
			getToken();
			array = true;
		}

		if (checkToken() == XTOKEN_NAME) {
			getToken();
		} else if (isPrimitiveType(checkToken())) {
			getToken();
		} else
			break;

		if (getToken() != XTOKEN_NAME)
			return false;

		if (array) {
			while (checkToken() == XTOKEN_OBRACKET) {
				getToken();
				if (checkToken() == XTOKEN_INTEGER) {
					getToken();
				} else {
					if (getToken() != XTOKEN_NAME)
						return false;
				}
				if (getToken() != XTOKEN_CBRACKET)
					return false;
			}
		}
		if (getToken() != XTOKEN_SEMICOLON)
			return false;
	}
	return true;
}

bool XFileLoader::parseTemplateParts() {
	if (!parseTemplateMembersList())
		return false;
	if (checkToken() == XTOKEN_OBRACKET) {
		getToken();
		if (!parseTemplateOptionInfo())
			return false;
		if (getToken() != XTOKEN_CBRACKET)
			return false;
	}
	return true;
}

bool XFileLoader::parseTemplate() {
	if (getToken() != XTOKEN_TEMPLATE)
		return false;
	if (getToken() != XTOKEN_NAME)
		return false;
	if (getToken() != XTOKEN_OBRACE)
		return false;
	if (getToken() != XTOKEN_GUID)
		return false;
	if (!parseTemplateParts())
		return false;
	if (getToken() != XTOKEN_CBRACE)
		return false;
	return true;
}

bool XFileLoader::parseObjectParts(XObject *object) {
	switch (object->_classType) {
	case kXClassAnimTicksPerSecond: {
			auto objClass = new XAnimTicksPerSecondObject;
			if (!getInteger(objClass->_animTicksPerSecond)) {
				delete objClass;
				return false;
			}
			object->_object = objClass;
		}
		break;

	case kXClassFrame: {
			auto objClass = new XFrameObject;
			if (!parseChildObjects(object)) {
				delete objClass;
				return false;
			}
			object->_object = objClass;
		}
		break;

	case kXClassFrameTransformMatrix: {
			auto objClass = new XFrameTransformMatrixObject;
			for (int m = 0; m < 16; m++) {
				if (!getFloat(objClass->_frameMatrix[m])) {
					delete objClass;
					return false;
				}
			}
			object->_object = objClass;
		}
		break;

	case kXClassMesh: {
			auto objClass = new XMeshObject;
			if (!getInteger(objClass->_numVertices)) {
				delete objClass;
				return false;
			}

			objClass->_vertices = new XVector3[objClass->_numVertices];
			for (uint n = 0; n < objClass->_numVertices; n++) {
				if (!getFloat(objClass->_vertices[n]._x) ||
				    !getFloat(objClass->_vertices[n]._y) ||
				    !getFloat(objClass->_vertices[n]._z)) {
					delete objClass;
					return false;
				}
			}
			skipSemicolonComma();

			if (!getInteger(objClass->_numFaces)) {
				delete objClass;
				return false;
			}
			objClass->_faces = new XMeshFace[objClass->_numFaces];
			for (uint n = 0; n < objClass->_numFaces; n++) {
				if (!getInteger(objClass->_faces[n]._numFaceVertexIndices)) {
					delete objClass;
					return false;
				}
				assert(objClass->_faces[n]._numFaceVertexIndices == 3 ||
				       objClass->_faces[n]._numFaceVertexIndices == 4);

				for (uint f = 0; f < objClass->_faces[n]._numFaceVertexIndices; f++) {
					if (!getInteger(objClass->_faces[n]._faceVertexIndices[f])) {
						delete objClass;
						return false;
					}
				}
				skipSemicolonComma();
			}
			skipSemicolonComma();

			if (!parseChildObjects(object)) {
				delete objClass;
				return false;
			}

			object->_object = objClass;
			break;
		}

	case kXClassMeshNormals: {
			auto objClass = new XMeshNormalsObject;
			if (!getInteger(objClass->_numNormals)) {
				delete objClass;
				return false;
			}

			objClass->_normals = new XVector3[objClass->_numNormals];
			for (uint n = 0; n < objClass->_numNormals; n++) {
				if (!getFloat(objClass->_normals[n]._x) ||
				    !getFloat(objClass->_normals[n]._y) ||
				    !getFloat(objClass->_normals[n]._z)) {
					delete objClass;
					return false;
				}
			}
			skipSemicolonComma();

			if (!getInteger(objClass->_numFaceNormals)) {
				delete objClass;
				return false;
			}
			objClass->_faceNormals = new XMeshFace[objClass->_numFaceNormals];

			for (uint n = 0; n < objClass->_numFaceNormals; n++) {
				if (!getInteger(objClass->_faceNormals[n]._numFaceVertexIndices)) {
					delete objClass;
					return false;
				}
				assert(objClass->_faceNormals[n]._numFaceVertexIndices == 3 ||
				       objClass->_faceNormals[n]._numFaceVertexIndices == 4);

				for (uint f = 0; f < objClass->_faceNormals[n]._numFaceVertexIndices; f++) {
					if (!getInteger(objClass->_faceNormals[n]._faceVertexIndices[f])) {
						delete objClass;
						return false;
					}
				}
				skipSemicolonComma();
			}
			skipSemicolonComma();

			object->_object = objClass;
			break;
		}

	case kXClassMeshVertexColors: {
			auto objClass = new XMeshVertexColorsObject;
			if (!getInteger(objClass->_numVertexColors)) {
				delete objClass;
				return false;
			}

			objClass->_vertexColors = new XIndexedColor[objClass->_numVertexColors];
			for (uint n = 0; n < objClass->_numVertexColors; n++) {
				if (!getInteger(objClass->_vertexColors[n]._index) ||
				    !getFloat(objClass->_vertexColors[n]._indexColorR) ||
				    !getFloat(objClass->_vertexColors[n]._indexColorG) ||
				    !getFloat(objClass->_vertexColors[n]._indexColorB) ||
				    !getFloat(objClass->_vertexColors[n]._indexColorA)) {
					delete objClass;
					return false;
				}
			}
			skipSemicolonComma();

			object->_object = objClass;
			break;
		}

	case kXClassMeshTextureCoords: {
			auto objClass = new XMeshTextureCoordsObject;
			if (!getInteger(objClass->_numTextureCoords)) {
				delete objClass;
				return false;
			}

			objClass->_textureCoords = new XCoords2d[objClass->_numTextureCoords];
			for (uint n = 0; n < objClass->_numTextureCoords; n++) {
				if (!getFloat(objClass->_textureCoords[n]._u)) {
					delete objClass;
					return false;
				}
				if (!getFloat(objClass->_textureCoords[n]._v)) {
					delete objClass;
					return false;
				}
			}
			skipSemicolonComma();

			object->_object = objClass;
			break;
		}

	case kXClassVertexDuplicationIndices: {
			auto objClass = new XVertexDuplicationIndicesObject;
			if (!getInteger(objClass->_numIndices) ||
			    !getInteger(objClass->_nOriginalVertices)) {
				delete objClass;
				return false;
			}

			objClass->_indices = new uint32[objClass->_numIndices];
			for (uint n = 0; n < objClass->_numIndices; n++) {
				if (!getInteger(objClass->_indices[n])) {
					delete objClass;
					return false;
				}
			}
			skipSemicolonComma();

			object->_object = objClass;
			break;
		}

	case kXClassMeshMaterialList: {
			auto objClass = new XMeshMaterialListObject;
			if (!getInteger(objClass->_nMaterials) ||
			    !getInteger(objClass->_numFaceIndexes)) {
				delete objClass;
				return false;
			}

			objClass->_faceIndexes = new uint32[objClass->_numFaceIndexes];
			for (uint n = 0; n < objClass->_numFaceIndexes; n++) {
				if (!getInteger(objClass->_faceIndexes[n])) {
					delete objClass;
					return false;
				}
			}
			skipSemicolonComma();

			if (!parseChildObjects(object)) {
				delete objClass;
				return false;
			}

			object->_object = objClass;
			break;
		}

	case kXClassMaterial: {
			auto objClass = new XMaterialObject;
			if (!getFloat(objClass->_colorR) ||
			    !getFloat(objClass->_colorG) ||
			    !getFloat(objClass->_colorB) ||
			    !getFloat(objClass->_colorA) ||
			    !getFloat(objClass->_power) ||
			    !getFloat(objClass->_specularR) ||
			    !getFloat(objClass->_specularG) ||
			    !getFloat(objClass->_specularB) ||
			    !getFloat(objClass->_emissiveR) ||
			    !getFloat(objClass->_emissiveG) ||
			    !getFloat(objClass->_emissiveB)) {
				delete objClass;
				return false;
			}

			if (!parseChildObjects(object)) {
				delete objClass;
				return false;
			}
			object->_object = objClass;
			break;
		}

	case kXClassTextureFilename: {
			auto objClass = new XTextureFilenameObject;
			if (!getString((char *)objClass->_filename, XMAX_NAME_LEN)) {
				delete objClass;
				return false;
			}
			object->_object = objClass;
			break;
		}

	case kXClassSkinMeshHeader: {
			auto objClass = new XSkinMeshHeaderObject;
			if (!getInteger(objClass->_nMaxSkinWeightsPerVertex) ||
			    !getInteger(objClass->_nMaxSkinWeightsPerFace) ||
			    !getInteger(objClass->_nBones)) {
				delete objClass;
				return false;
			}
			object->_object = objClass;
			break;
		}

	case kXClassSkinWeights: {
			auto objClass = new XSkinWeightsObject;
			if (!getString(objClass->_transformNodeName, XMAX_NAME_LEN) ||
			    !getInteger(objClass->_numWeights)) {
				delete objClass;
				return false;
			}

			objClass->_vertexIndices = new uint32[objClass->_numWeights];
			for (uint n = 0; n < objClass->_numWeights; n++) {
				if (!getInteger(objClass->_vertexIndices[n])) {
					delete objClass;
					return false;
				}
			}
			skipSemicolonComma();

			objClass->_weights = new float[objClass->_numWeights];
			for (uint n = 0; n < objClass->_numWeights; n++) {
				if (!getFloat(objClass->_weights[n])) {
					delete objClass;
					return false;
				}
			}
			skipSemicolonComma();

			for (int m = 0; m < 16; m++) {
				if (!getFloat(objClass->_matrixOffset[m])) {
					delete objClass;
					return false;
				}
			}

			object->_object = objClass;
			break;
		}

	case kXClassAnimationSet: {
			auto objClass = new XAnimationSetObject;
			if (!parseChildObjects(object)) {
				delete objClass;
				return false;
			}
			object->_object = objClass;
		}
		break;

	case kXClassAnimation: {
			auto objClass = new XAnimationObject;
			if (!parseChildObjects(object)) {
				delete objClass;
				return false;
			}
			object->_object = objClass;
		}
		break;

	case kXClassAnimationKey: {
			auto objClass = new XAnimationKeyObject;
			if (!getInteger(objClass->_keyType) ||
			    !getInteger(objClass->_numKeys)) {
				delete objClass;
				return false;
			}
			if (objClass->_keyType > 4) {
				warning("XFileLoader: AnimationKey key type invalid");
				delete objClass;
				return false;
			}

			objClass->_keys = new XTimedFloatKeys[objClass->_numKeys];
			for (uint n = 0; n < objClass->_numKeys; n++) {
				if (checkToken() == XTOKEN_INTEGER) {
					uint32 timeVal;
					if (!getInteger(timeVal)) {
						delete objClass;
						return false;
					}
					objClass->_keys[n]._time = timeVal;
				} else if (checkToken() == XTOKEN_FLOAT) {
					if (!getFloat(objClass->_keys[n]._time)) {
						delete objClass;
						return false;
					}
				}

				if (!getInteger(objClass->_keys[n]._numTfkeys)) {
					delete objClass;
					return false;
				}
				for (uint f = 0; f < objClass->_keys[n]._numTfkeys; f++) {
					if (!getFloat(objClass->_keys[n]._tfkeys[f])) {
						delete objClass;
						return false;
					}
				}
				skipSemicolonComma();
			}
			skipSemicolonComma();

			object->_object = objClass;
			break;
		}

	case kXClassAnimationOptions: {
			auto objClass = new XAnimationOptionsObject;
			if (!getInteger(objClass->_openclosed) ||
			    !getInteger(objClass->_positionquality)) {
				delete objClass;
				return false;
			}

			object->_object = objClass;
			break;
		}

	case kXClassDeclData: {
			auto objClass = new XDeclDataObject;
			if (!getInteger(objClass->_numElements)) {
				delete objClass;
				return false;
			}

			objClass->_elements = new XVertexElement[objClass->_numElements];
			for (uint n = 0; n < objClass->_numElements; n++) {
				if (!getInteger(objClass->_elements[n]._type) ||
				    !getInteger(objClass->_elements[n]._method) ||
				    !getInteger(objClass->_elements[n]._usage) ||
				    !getInteger(objClass->_elements[n]._usageIndex)) {
					delete objClass;
					return false;
				}
			}
			skipSemicolonComma();

			if (!getInteger(objClass->_numData)) {
				delete objClass;
				return false;
			}
			objClass->_data = new uint32[objClass->_numData];
			for (uint n = 0; n < objClass->_numData; n++) {
				if (!getInteger(objClass->_data[n])) {
					delete objClass;
					return false;
				}
			}
			skipSemicolonComma();

			object->_object = objClass;
			break;
		}

	case kXClassFVFData: {
			auto objClass = new XFVFDataObject;
			if (!getInteger(objClass->_dwFVF) ||
			    !getInteger(objClass->_numData)) {
				delete objClass;
				return false;
			}

			objClass->_data = new uint32[objClass->_numData];
			for (uint n = 0; n < objClass->_numData; n++) {
				if (!getInteger(objClass->_data[n])) {
					delete objClass;
					return false;
				}
			}
			skipSemicolonComma();

			object->_object = objClass;
			break;
		}

	default:
		error("XFileLoader: Not implemented class %d", object->_classType);
	}

	return true;
}

XObject *XFileLoader::resolveChildObject(XObject *object, const Common::String &referenceName) {
	if (object->_name == referenceName) {
		return object;
	}
	for (uint i = 0; i < object->_children.size(); i++) {
		XObject *targetObject = resolveChildObject(object->_children[i], referenceName);
		if (targetObject)
			return targetObject;
	}
	return nullptr;
}

bool XFileLoader::resolveObject(XObject *referenceObject, const Common::String &referenceName) {
	bool found = false;
	for (uint i = 0; i < _xobjects.size(); i++) {
		XObject *targetObject = resolveChildObject(_xobjects[i], referenceName);
		if (targetObject) {
			referenceObject->_targetObject = targetObject;
			found = true;
			break;
		}
	}
	return found;
}

bool XFileLoader::parseChildObjects(XObject *object) {
	if (checkToken() != XTOKEN_NAME && checkToken() != XTOKEN_OBRACE) {
		return true;
	}

	while (true) {
		if (checkToken() == XTOKEN_OBRACE) {
			getToken();
			if (getToken() != XTOKEN_NAME)
				return false;
			XObject *child = new XObject();
			object->_children.push(child);
			if (!resolveObject(child, _currentToken._textVal)) {
				warning("XFileLoader: Referenced object doesn't exists \"%s\"", _currentToken._textVal);
			}
			if (getToken() != XTOKEN_CBRACE)
				return false;
		} else if (checkToken() == XTOKEN_NAME) {
			XObject *child = new XObject();
			object->_children.push(child);
			if (!parseObject(child))
				return false;
		} else if (checkToken() != XTOKEN_CBRACE) {
			return false;
		} else
			break;
	}

	return true;
}

bool XFileLoader::parseObject(XObject *object) {
	if (getToken() != XTOKEN_NAME)
		return false;

	for (uint i = 0; i < ARRAYSIZE(gXClasses); i++) {
		if (!XFileLoader_strcmp(gXClasses[i].className, _currentToken._textVal)) {
			object->_classType = gXClasses[i].type;
			break;
		}
	}
	if (object->_classType == kXClassUnknown) {
		error("XFileLoader: Unknown class \"%s\"", _currentToken._textVal);
		return false;
	}

	if (checkToken() == XTOKEN_NAME) {
		getToken();
		object->_name = _currentToken._textVal;
	}

	if (getToken() != XTOKEN_OBRACE)
		return false;

	if (checkToken() == XTOKEN_GUID) {
		getToken();
	}

	if (!parseObjectParts(object))
		return false;
	if (getToken() != XTOKEN_CBRACE)
		return false;

	checkToken();

	return true;
}

bool XFileLoader::load(byte *buffer, uint32 bufferSize) {
	if (!_initialised)
		return false;

	_buffer = buffer;
	_bufferLeft = bufferSize;

	if (!parseHeader())
		return false;

	while (_bufferLeft) {
		XTokenType token = checkToken();
		switch (token) {
		case XTOKEN_TEMPLATE:
			if (!parseTemplate()) {
				warning("XFileLoader: Template is not correct");
				return false;
			}
			break;
		case XTOKEN_NAME: {
				XObject *xobject = new XObject();
				_xobjects.push(xobject);
				if (!parseObject(xobject)) {
					warning("XFileLoader: Object is not correct");
					return false;
				}
			}
			break;
		default:
			warning("XFileLoader: Unexpected token");
			return false;
		}
	}
	return true;
}

} // namespace Wintermute
