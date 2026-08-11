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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#include "common/str.h"

#include "gob/html_parser.h"
#include "gob/global.h"
#include "gob/draw.h"
#include "gob/gob.h"
#include "gob/script.h"
#include "gob/inter.h"

namespace Gob {

HtmlContext::HtmlContext(Common::SeekableReadStream *stream, GobEngine *vm) : _stream(stream), _vm(vm) {
	_pos = 0;
	_posBak = -1;
	_field_10 = false;
	_currentMarkIndex = 1;
	for (uint32 i = 0; i < kMaxNbrOfHtmlMarks; ++i) {
		_marks[i]._field_0 = 3;
		_marks[i]._field_4 = 0;
		_marks[i]._field_8 = 0;
		_marks[i]._pos = 0;
		for (uint32 j = 0; j < 5; ++j) {
			_marks[i]._field_14[j] = 0;
		}

		_marks[i]._field_28 = 0;
	}
}

HtmlContext::~HtmlContext() {
	delete _stream;
}

Common::HashMap<Common::String, HtmlContext::HtmlTagType> *HtmlContext::_htmlTagsTypesMap = nullptr;
Common::HashMap<Common::String, char> *HtmlContext::_htmlEntitiesMap = nullptr;

HtmlContext::HtmlTagType HtmlContext::getHtmlTagType(const char *tag) {
	if (_htmlTagsTypesMap == nullptr) {
		_htmlTagsTypesMap = new Common::HashMap<Common::String, HtmlTagType>();
		// Initialize the map on first use
		(*_htmlTagsTypesMap)["BODY"] = kHtmlTagType_Body;
		(*_htmlTagsTypesMap)["FONT"] = kHtmlTagType_Font;
		(*_htmlTagsTypesMap)["/FONT"] = kHtmlTagType_Font_Close;
		(*_htmlTagsTypesMap)["IMG"] = kHtmlTagType_Img;
		(*_htmlTagsTypesMap)["A"] = kHtmlTagType_A;
		(*_htmlTagsTypesMap)["/A"] = kHtmlTagType_A_Close;
		(*_htmlTagsTypesMap)["TITLE"] = kHtmlTagType_Title;
		(*_htmlTagsTypesMap)["/TITLE"] = kHtmlTagType_Title_Close;
		(*_htmlTagsTypesMap)["/HTML"] = kHtmlTagType_HTML_Close;
		(*_htmlTagsTypesMap)["BR"] = kHtmlTagType_BR;
		(*_htmlTagsTypesMap)["P"] = kHtmlTagType_P;
		(*_htmlTagsTypesMap)["/P"] = kHtmlTagType_P_Close;
		(*_htmlTagsTypesMap)["U"] = kHtmlTagType_U;
		(*_htmlTagsTypesMap)["/U"] = kHtmlTagType_U_Close;
		(*_htmlTagsTypesMap)["B"] = kHtmlTagType_B;
		(*_htmlTagsTypesMap)["/B"] = kHtmlTagType_B_Close;
		(*_htmlTagsTypesMap)["EM"] = kHtmlTagType_EM;
		(*_htmlTagsTypesMap)["/EM"] = kHtmlTagType_EM_Close;
		(*_htmlTagsTypesMap)["I"] = kHtmlTagType_I;
		(*_htmlTagsTypesMap)["/I"] = kHtmlTagType_I_Close;
		(*_htmlTagsTypesMap)["SUB"] = kHtmlTagType_SUB;
		(*_htmlTagsTypesMap)["/SUB"] = kHtmlTagType_SUB_Close;
		(*_htmlTagsTypesMap)["SUP"] = kHtmlTagType_SUP;
		(*_htmlTagsTypesMap)["/SUP"] = kHtmlTagType_SUP_Close;
	}

	if (_htmlTagsTypesMap->contains(tag))
		return (*_htmlTagsTypesMap)[tag];
	else
		return kHtmlTagType_None;
}

char HtmlContext::getHtmlEntityLatin1Char(const char *entity) {
	if (_htmlEntitiesMap == nullptr) {
		_htmlEntitiesMap = new Common::HashMap<Common::String, char>();
		// Initialize the map on first use
		(*_htmlEntitiesMap)["quot"] = '\x22';
		(*_htmlEntitiesMap)["nbsp"] = '\x20';
		(*_htmlEntitiesMap)["iexcl"] = '\xA1';
		(*_htmlEntitiesMap)["cent"] = '\xA2';
		(*_htmlEntitiesMap)["pound"] = '\xA3';
		(*_htmlEntitiesMap)["curren"] = '\xA4';
		(*_htmlEntitiesMap)["yen"] = '\xA5';
		(*_htmlEntitiesMap)["brvbar"] = '\xA6';
		(*_htmlEntitiesMap)["sect"] = '\xA7';
		(*_htmlEntitiesMap)["uml"] = '\xA8';
		(*_htmlEntitiesMap)["copy"] = '\xA9';
		(*_htmlEntitiesMap)["ordf"] = '\xAA';
		(*_htmlEntitiesMap)["laquo"] = '\xAB';
		(*_htmlEntitiesMap)["not"] = '\xAC';
		(*_htmlEntitiesMap)["shy"] = '\xAD';
		(*_htmlEntitiesMap)["reg"] = '\xAE';
		(*_htmlEntitiesMap)["macr"] = '\xAF';
		(*_htmlEntitiesMap)["deg"] = '\xB0';
		(*_htmlEntitiesMap)["plusmn"] = '\xB1';
		(*_htmlEntitiesMap)["sup2"] = '\xB2';
		(*_htmlEntitiesMap)["sup3"] = '\xB3';
		(*_htmlEntitiesMap)["acute"] = '\xB4';
		(*_htmlEntitiesMap)["micro"] = '\xB5';
		(*_htmlEntitiesMap)["para"] = '\xB6';
		(*_htmlEntitiesMap)["middot"] = '\xB7';
		(*_htmlEntitiesMap)["cedil"] = '\xB8';
		(*_htmlEntitiesMap)["sup1"] = '\xB9';
		(*_htmlEntitiesMap)["ordm"] = '\xBA';
		(*_htmlEntitiesMap)["raquo"] = '\xBB';
		(*_htmlEntitiesMap)["frac14"] = '\xBC';
		(*_htmlEntitiesMap)["frac12"] = '\xBD';
		(*_htmlEntitiesMap)["frac34"] = '\xBE';
		(*_htmlEntitiesMap)["iquest"] = '\xBF';
		(*_htmlEntitiesMap)["Agrave"] = '\xC0';
		(*_htmlEntitiesMap)["Aacute"] = '\xC1';
		(*_htmlEntitiesMap)["Acirc"] = '\xC2';
		(*_htmlEntitiesMap)["Atilde"] = '\xC3';
		(*_htmlEntitiesMap)["Auml"] = '\xC4';
		(*_htmlEntitiesMap)["Aring"] = '\xC5';
		(*_htmlEntitiesMap)["AElig"] = '\xC6';
		(*_htmlEntitiesMap)["Ccedil"] = '\xC7';
		(*_htmlEntitiesMap)["Egrave"] = '\xC8';
		(*_htmlEntitiesMap)["Eacute"] = '\xC9';
		(*_htmlEntitiesMap)["Ecirc"] = '\xCA';
		(*_htmlEntitiesMap)["Euml"] = '\xCB';
		(*_htmlEntitiesMap)["Igrave"] = '\xCC';
		(*_htmlEntitiesMap)["Iacute"] = '\xCD';
		(*_htmlEntitiesMap)["Icirc"] = '\xCE';
		(*_htmlEntitiesMap)["Iuml"] = '\xCF';
		(*_htmlEntitiesMap)["ETH"] = '\xD0';
		(*_htmlEntitiesMap)["Ntilde"] = '\xD1';
		(*_htmlEntitiesMap)["Ograve"] = '\xD2';
		(*_htmlEntitiesMap)["Oacute"] = '\xD3';
		(*_htmlEntitiesMap)["Ocirc"] = '\xD4';
		(*_htmlEntitiesMap)["Otilde"] = '\xD5';
		(*_htmlEntitiesMap)["Ouml"] = '\xD6';
		(*_htmlEntitiesMap)["times"] = '\xD7';
		(*_htmlEntitiesMap)["Oslash"] = '\xD8';
		(*_htmlEntitiesMap)["Ugrave"] = '\xD9';
		(*_htmlEntitiesMap)["Uacute"] = '\xDA';
		(*_htmlEntitiesMap)["Ucirc"] = '\xDB';
		(*_htmlEntitiesMap)["Uuml"] = '\xDC';
		(*_htmlEntitiesMap)["Yacute"] = '\xDD';
		(*_htmlEntitiesMap)["THORN"] = '\xDE';
		(*_htmlEntitiesMap)["szlig"] = '\xDF';
		(*_htmlEntitiesMap)["agrave"] = '\xE0';
		(*_htmlEntitiesMap)["aacute"] = '\xE1';
		(*_htmlEntitiesMap)["acirc"] = '\xE2';
		(*_htmlEntitiesMap)["atilde"] = '\xE3';
		(*_htmlEntitiesMap)["auml"] = '\xE4';
		(*_htmlEntitiesMap)["aring"] = '\xE5';
		(*_htmlEntitiesMap)["aelig"] = '\xE6';
		(*_htmlEntitiesMap)["ccedil"] = '\xE7';
		(*_htmlEntitiesMap)["egrave"] = '\xE8';
		(*_htmlEntitiesMap)["eacute"] = '\xE9';
		(*_htmlEntitiesMap)["ecirc"] = '\xEA';
		(*_htmlEntitiesMap)["euml"] = '\xEB';
		(*_htmlEntitiesMap)["igrave"] = '\xEC';
		(*_htmlEntitiesMap)["iacute"] = '\xED';
		(*_htmlEntitiesMap)["icirc"] = '\xEE';
		(*_htmlEntitiesMap)["iuml"] = '\xEF';
		(*_htmlEntitiesMap)["eth"] = '\xF0';
		(*_htmlEntitiesMap)["ntilde"] = '\xF1';
		(*_htmlEntitiesMap)["ograve"] = '\xF2';
		(*_htmlEntitiesMap)["oacute"] = '\xF3';
		(*_htmlEntitiesMap)["ocirc"] = '\xF4';
		(*_htmlEntitiesMap)["otilde"] = '\xF5';
		(*_htmlEntitiesMap)["ouml"] = '\xF6';
		(*_htmlEntitiesMap)["divide"] = '\xF7';
		(*_htmlEntitiesMap)["oslash"] = '\xF8';
		(*_htmlEntitiesMap)["ugrave"] = '\xF9';
		(*_htmlEntitiesMap)["uacute"] = '\xFA';
		(*_htmlEntitiesMap)["ucirc"] = '\xFB';
		(*_htmlEntitiesMap)["uuml"] = '\xFC';
		(*_htmlEntitiesMap)["yacute"] = '\xFD';
		(*_htmlEntitiesMap)["thorn"] = '\xFE';
		(*_htmlEntitiesMap)["yuml"] = '\xFF';
		(*_htmlEntitiesMap)["gt"] = '\x3E';
		(*_htmlEntitiesMap)["lt"] = '\x3C';
	}

	if (*entity == '&') {
		++entity;
	}

	if (*entity == '#') {
		++entity;
		int code = atoi(entity);
		if (code >= 32 && code <= 255) {
			return (char) code;
		} else {
			warning("HtmlContext::getHtmlEntityLatin1Char(): Invalid char value %s", entity);
			return '&';
		}
	}


	if (_htmlEntitiesMap->contains(entity))
		return (*_htmlEntitiesMap)[entity];
	else {
		warning("HtmlContext::getHtmlEntityLatin1Char(): Unknown entity \"%s\"", entity);
		return '&';
	}
}

Common::String HtmlContext::substituteHtmlEntities(const char *text) {
	Common::String result;
	const char *charPtr = text;
	while (*charPtr != '\0') {
		if (*charPtr == '&') {
			Common::String entityName = popStringPrefix(&charPtr, ';');
			char entityChar = getHtmlEntityLatin1Char(entityName.c_str());
			result += entityChar;
		} else if (*charPtr == '\r') {
			result += ' ';
			++charPtr;
		} else {
			result += *charPtr;
		}

		if (*charPtr != '\0')
			++charPtr;
	}

	return result;
}

Common::String HtmlContext::popStringPrefix(const char **charPtr, char sep) {
	const char *startPtr = *charPtr;
	if (sep == ' ') {
		while (!Common::isSpace(**charPtr) && **charPtr != '\0')
			++*charPtr;
		return Common::String(startPtr, *charPtr - startPtr);
	} else {
		while (**charPtr != sep && **charPtr != '\0')
			++*charPtr;
		return Common::String(startPtr, *charPtr - startPtr);
	}
}

void HtmlContext::seekCommand(Common::String command, Common::String commandArg, uint16 destVar) {
	if (!_stream) {
		warning("HtmlContext::seekCommand(): No open stream");
		return;
	}

	if (command.hasPrefix("SAVEMARK")) {
		if (_buffer[0] == '\0') {
			_currentTagType = HtmlContext::kHtmlTagType_HTML_Close;
			return;
		}

		if (command == "SAVEMARK_BACK") {
			if (_posBak == -1) {
				warning("o7_seekHtmlFile(): No saved position");
			} else {
				debugC(5, kDebugGameFlow, "o7_seekHtmlFile: SAVEMARK_BACK pos %d -> %d", _pos, _posBak);
				_pos = _posBak;
			}
		}

		const char*htmlBufferPtr = _buffer;
		if (!commandArg.empty()) {
			while (*htmlBufferPtr != '\0') {
				Common::String string(htmlBufferPtr, 4 * _vm->_global->_inter_animDataSize);
				string = HtmlContext::substituteHtmlEntities(string.c_str());
				if (strcmp(commandArg.c_str(), string.c_str()) == 0) {
					break;
				}

				++htmlBufferPtr;
			}
		}

		if (strcmp(htmlBufferPtr, commandArg.c_str()) == 0) {
			_pos = _pos - strlen(_buffer) + htmlBufferPtr - _buffer;
		} else if (!commandArg.empty()) {
			warning("o7_seekHtmlFile(): Cannot find string \"%s\" while using SAVEMARK", commandArg.c_str());
		}

		if (_currentMarkIndex < 20) {
			HtmlMark &mark = _marks[_currentMarkIndex];
			mark._field_0 = READ_VARO_UINT32(destVar);
			mark._field_4 = READ_VARO_UINT32(destVar + 4);
			mark._field_8 = READ_VARO_UINT32(destVar + 8);
			mark._field_C = READ_VARO_UINT32(destVar + 0xC);
			mark._pos = _pos;
			for (uint32 i = 0; i < 5; ++i) {
				mark._field_14[i] = READ_VARO_UINT32(destVar + 0x14 + i * 4);
			}

			mark._field_28 = READ_VARO_UINT32(destVar + 0x28);

			++_currentMarkIndex;
		} else {
			warning("o7_seekHtmlFile(): Mark index %d out of range while using SAVEMARK (max number of marks = %d)",
					_currentMarkIndex,
					20);
		}

	} else if (command.hasPrefix("GETMARK")) {
		int markIndex = atoi(commandArg.c_str());
		if (markIndex < 0 || markIndex >= (int) kMaxNbrOfHtmlMarks) {
			warning("o7_seekHtmlFile(): mark index %d out of range", markIndex);
			return;
		}

		_currentMarkIndex = markIndex;
		HtmlMark &mark = _marks[markIndex];

		_pos = mark._pos;

		WRITE_VAR_OFFSET(destVar, mark._field_0);
		WRITE_VAR_OFFSET(destVar + 4, mark._field_4);
		WRITE_VAR_OFFSET(destVar + 8, mark._field_8);
		WRITE_VAR_OFFSET(destVar + 0xC, mark._field_C);
		for (uint32 i = 0; i < 5; ++i) {
			WRITE_VAR_OFFSET(destVar + 0x14 + i * 4, mark._field_14[i]);
		}

		WRITE_VAR_OFFSET(destVar + 0x28, mark._field_28);

	} else if (command == "SEEK") {
		if (commandArg == "0")
			_pos = 0;

		if (_pos >= 0)
			_stream->seek(_pos, SEEK_SET);
		else
			warning("o7_seekHtmlFile(): Invalid seek position %d", _pos);

		// NB: Seek to other offsets than 0 does not seem to be implemented in the original engine

	} else {
		warning("o7_seekHtmlFile(): Unknown command \"%s\"", command.c_str());
	}
}

void HtmlContext::parseTagAttributes(const char *tagAttributes) {
	const char *tagAttributesPtr = tagAttributes;
	while (*tagAttributesPtr != '\0') {
		// Skip leading spaces
		while (Common::isSpace(*tagAttributesPtr))
			++tagAttributesPtr;

		if (*tagAttributesPtr == '\0')
			return;

		Common::String attrName = popStringPrefix(&tagAttributesPtr, '=');
		if (attrName.empty()) {
			warning("HtmlContext::parseTagAttributes(): Missing '=' in tag attributes");
			return;
		}

		Common::String attrValue;
		if (tagAttributesPtr[1] == '\"') {
			tagAttributesPtr += 2;
			attrValue = popStringPrefix(&tagAttributesPtr, '\"');
		} else {
			++tagAttributesPtr;
			attrValue = popStringPrefix(&tagAttributesPtr, ' ');
		}

		attrValue.toUppercase();

		// Handle attribute and value
		switch (_currentTagType) {
		case kHtmlTagType_Font:
			if (strcmp(attrName.c_str(), "FACE") == 0)
				_htmlVariables[0] = attrValue;
			else if (strcmp(attrName.c_str(), "SIZE") == 0)
				_htmlVariables[1] = attrValue;
			else if (strcmp(attrName.c_str(), "COLOR") == 0) {
				_htmlVariables[2] = attrValue;
			}
			break;

		case kHtmlTagType_Img:
			if (strcmp(attrName.c_str(), "SRC") == 0)
				_htmlVariables[0] = attrValue;
			else if (strcmp(attrName.c_str(), "HEIGHT") == 0)
				_htmlVariables[1] = attrValue;
			else if (strcmp(attrName.c_str(), "WIDTH") == 0)
				_htmlVariables[2] = attrValue;
			else if (strcmp(attrName.c_str(), "BORDER") == 0)
				_htmlVariables[3] = attrValue;
			_htmlVariables[0] = attrValue;

			if (!_field_10) {
				_posBak = _pos;
			}
			break;

		case kHtmlTagType_A:
			if (strcmp(attrName.c_str(), "NAME") == 0)
				_htmlVariables[0] = attrValue;
			else if (strcmp(attrName.c_str(), "HREF") == 0) {
				_htmlVariables[1] = attrValue;
				_posBak = _pos;
				_field_10 = true;
			}
			break;

		default:
			break;
		}

		if (*tagAttributesPtr == '\"')
			++tagAttributesPtr;
	}
}

void HtmlContext::cleanTextNode(int animDataSize) {
	_buffer[animDataSize * 4 - 1] = '\0';
	size_t len = strlen(_buffer);
	char *htmlBufferPtr = strchr(_buffer, '<');
	if (!htmlBufferPtr) {
		char* charPtr2 = _buffer + len - 1;
		while (charPtr2 > _buffer && *charPtr2 != ' ') {
			--charPtr2;
		}

		if (charPtr2 > _buffer) {
			if (*charPtr2 == ' ')
				charPtr2[1] = '\0';
			else
				*charPtr2 = '\0';
		}
	} else {
		*htmlBufferPtr = '\0';
	}

	htmlBufferPtr = strchr(_buffer, '&');
	if (htmlBufferPtr) {
		char *charPtr2 = strchr(_buffer, ';');
		if (!charPtr2 || charPtr2 < htmlBufferPtr) {
			*htmlBufferPtr = '\0';
		}
	}
}

void HtmlContext::nextKeyword(uint16 destVar, uint16 destVarTagType) {
	_currentTagType = kHtmlTagType_None;

	if (!_stream) {
		warning("HtmlContext::nextKeyword(): No open stream");
		return;
	}

	if (!_stream->seek(_pos, SEEK_SET)) {
		_currentTagType = kHtmlTagType_HTML_Close;
		return;
	}

	memset(_buffer, 0, sizeof(_buffer));
	_stream->read(_buffer, sizeof(_buffer) - 1);

	const char *htmlBufferPtr = _buffer;
	if (*htmlBufferPtr == '<') {
		++htmlBufferPtr;
		Common::String tagAndAttributes = popStringPrefix(&htmlBufferPtr, '>');
		const char *tagAndAttributesPtr = tagAndAttributes.c_str();
		if (!tagAndAttributes.empty()) {
			Common::String tagName = popStringPrefix(&tagAndAttributesPtr, ' ');
			_currentTagType = getHtmlTagType(tagName.c_str());
		}

		if (_currentTagType == kHtmlTagType_None) {
			while (*htmlBufferPtr != '>' && *htmlBufferPtr != '\0') {
				++htmlBufferPtr;
			}
		} else {
			// Handle tag attributes
			parseTagAttributes(tagAndAttributesPtr);
		}

		if (*htmlBufferPtr == '>') {
			++htmlBufferPtr;
			// Skip CRLF
			while (*htmlBufferPtr == '\r' || *htmlBufferPtr == '\n') {
				++htmlBufferPtr;
			}
		}
	} else {
		_currentTagType = kHtmlTagType_OutsideTag;
		cleanTextNode(_vm->_global->_inter_animDataSize);

		Common::String text = popStringPrefix(&htmlBufferPtr, '<');
		if (text.empty()) {
			_currentTagType = kHtmlTagType_Error;
		} else {
			_htmlVariables[0] = substituteHtmlEntities(text.c_str());;
		}
	}

	_pos = _pos + (htmlBufferPtr - _buffer);

	// Write Html info to game variables
	WRITE_VAR_OFFSET(destVarTagType, _currentTagType);
	for (int i = 0; i < 10; ++i) {
		WRITE_VARO_STR(destVar + i * 4 * _vm->_global->_inter_animDataSize,
					   _htmlVariables[i].substr(0, 4 * _vm->_global->_inter_animDataSize).c_str());
	}
}


} // End of namespace Gob

