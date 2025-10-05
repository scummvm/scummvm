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

#ifndef GOB_HTML_PARSER_H
#define GOB_HTML_PARSER_H

#include "common/hashmap.h"
#include "common/str.h"

#include "gob/gob.h"

namespace Common {
class SeekableReadStream;
}

namespace Gob {

struct HtmlMark {
	uint32 _field_0;
	uint32 _field_4;
	uint32 _field_8;
	uint32 _field_C;
	uint32 _pos;
	uint32 _field_14[5];
	uint32 _field_28;
};


class HtmlContext {
public:
	HtmlContext(Common::SeekableReadStream *stream, GobEngine *vm);
	~HtmlContext();

	void seekCommand(Common::String command, Common::String commandArg, uint16 destVar);
	void nextKeyword(uint16 destVar, uint16 destVarTagType);

private:
	enum HtmlTagType {
		kHtmlTagType_None = -1,
		kHtmlTagType_Error,
		kHtmlTagType_OutsideTag,
		kHtmlTagType_Body,
		kHtmlTagType_Font,
		kHtmlTagType_Font_Close,
		kHtmlTagType_Img,
		kHtmlTagType_A,
		kHtmlTagType_A_Close,
		kHtmlTagType_Title,
		kHtmlTagType_Title_Close,
		kHtmlTagType_HTML_Close,
		kHtmlTagType_BR,
		kHtmlTagType_P,
		kHtmlTagType_P_Close,
		kHtmlTagType_U,
		kHtmlTagType_U_Close,
		kHtmlTagType_B,
		kHtmlTagType_B_Close,
		kHtmlTagType_EM,
		kHtmlTagType_EM_Close,
		kHtmlTagType_I,
		kHtmlTagType_I_Close,
		kHtmlTagType_SUB,
		kHtmlTagType_SUB_Close,
		kHtmlTagType_SUP,
		kHtmlTagType_SUP_Close,
	};

	static Common::HashMap<Common::String, HtmlTagType> *_htmlTagsTypesMap;
	static Common::HashMap<Common::String, char> *_htmlEntitiesMap;
	static Common::String popStringPrefix(const char **charPtr, char sep);
	static HtmlTagType getHtmlTagType(const char *tag);
	static char getHtmlEntityLatin1Char(const char *entity);
	static Common::String substituteHtmlEntities(const char *text);

	void parseTagAttributes(const char* tagAttributes);
	void cleanTextNode(int animDataSize);

	static const uint32 kMaxNbrOfHtmlMarks = 20;

	HtmlTagType _currentTagType;
	Common::SeekableReadStream *_stream;
	int _pos;
	int _posBak;
	bool _field_10;
	char _buffer[256];
	HtmlMark _marks[kMaxNbrOfHtmlMarks];
	uint32 _currentMarkIndex;
	Common::String _htmlVariables[10];

	GobEngine *_vm;
};


} // End of namespace Gob

#endif // GOB_HTML_PARSER_H
