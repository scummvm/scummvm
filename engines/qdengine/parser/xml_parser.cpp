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

#ifndef _XML_ONLY_BINARY_SCRIPT_
#endif
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/file.h"
#include "common/textconsole.h"
#include "qdengine/qd_precomp.h"
#include "qdengine/parser/xml_parser.h"

namespace QDEngine {

namespace xml {

#ifndef _XML_ONLY_BINARY_SCRIPT_
static void start_element_handler(void *userData, const XML_Char *name, const XML_Char **atts);
static void end_element_handler(void *userData, const XML_Char *name);
static void character_data_handler(void *userData, const XML_Char *s, int len);
static int unknown_encoding_handler(void *encodingHandlerData, const XML_Char *name, XML_Encoding *info);

static const char *UTF8_convert(const char *input_string, int input_string_length = -1);
#endif

}; /* namespace xml */

/* --------------------------- DEFINITION SECTION --------------------------- */

namespace xml {

#ifndef _XML_ONLY_BINARY_SCRIPT_
static const char *UTF8_convert(const char *input_string, int input_string_length) {
	static std::wstring wstr(1024, 0);
	static std::string str(1024, 0);

	unsigned int length = MultiByteToWideChar(CP_UTF8, 0, input_string, input_string_length, NULL, 0);
	if (wstr.length() < length)
		wstr.resize(length, 0);

	MultiByteToWideChar(CP_UTF8, 0, input_string, input_string_length, &*wstr.begin(), length);

	if (str.length() < length + 1)
		str.resize(length + 1, 0);
	str[length] = 0;

	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), length, &*str.begin(), length, NULL, NULL);

	return str.c_str();
}

static int unknown_encoding_handler(void *encodingHandlerData, const XML_Char *name, XML_Encoding *info) {
	if (!scumm_stricmp(name, "WINDOWS-1251")) {
		info->data = NULL;
		info->convert = NULL;
		info->release = NULL;

		for (int i = 0; i < 256; i++) {
			char c = i;
			wchar_t cc = 0;
			MultiByteToWideChar(1251, 0, &c, 1, &cc, 1);
			info->map[i] = cc;
		}

		return 1;
	}

	return 0;
}

static void start_element_handler(void *userData, const XML_Char *name, const XML_Char **atts) {
	parser *p = static_cast<parser *>(userData);
	p->start_element_handler(name, atts);
}

static void end_element_handler(void *userData, const XML_Char *name) {
	parser *p = static_cast<parser *>(userData);
	p->end_element_handler(name);
}

static void character_data_handler(void *userData, const XML_Char *s, int len) {
	parser *p = static_cast<parser *>(userData);
	p->character_data_handler(s, len);
}
#endif

parser::parser() : _data_pool_position(0), _data_buffer(1024, 0), _cur_level(0), _skip_mode(false), _binary_script(false) {
	_root_tag.set_data(&_data_pool);
}

parser::~parser() {
}

void parser::clear() {
	_root_tag.clear();
	while (!_tag_stack.empty()) _tag_stack.pop();

	_cur_level = 0;
	_skip_mode = false;

	_data_pool.clear();
	std::vector<char>(_data_pool).swap(_data_pool);
}

#ifndef _XML_ONLY_BINARY_SCRIPT_
void parser::start_element_handler(const char *tag_name, const char **tag_attributes) {
	if (!_skip_mode) {
		const tag *fmt = get_tag_format(tag_name);
		if (fmt) {
			tag tg(*fmt);
			tg.set_data(&_data_pool);

			int sz = 0;
			while (tag_attributes[sz]) sz++;

			if (sz >= 2) {
				for (int i = 0; i < sz; i += 2) {
					const tag *afmt = get_tag_format(tag_attributes[i]);
					if (afmt) {
						tag att(*afmt);
						att.set_data(&_data_pool);

						read_tag_data(att, tag_attributes[i + 1], strlen(tag_attributes[i + 1]));
						tg.add_subtag(att);
					}
				}
			}

			if (!_tag_stack.empty())
				_tag_stack.push(&_tag_stack.top()->add_subtag(tg));
		} else {
			_skip_mode = true;
			_cur_level = 0;
		}

		_data_buffer.clear();
	} else
		_cur_level++;
}

void parser::end_element_handler(const char *tag_name) {
	if (!_skip_mode) {
		if (!_tag_stack.empty())
			read_tag_data(*_tag_stack.top(), _data_buffer.c_str(), strlen(_data_buffer.c_str()));

		_tag_stack.pop();
	} else {
		if (!_cur_level--) _skip_mode = false;
	}
}

void parser::character_data_handler(const char *data, int data_length) {
	_data_buffer.append(data, data_length);
}
#endif

bool parser::parse_file(const char *fname) {
	if (is_script_binary(fname))
		return read_binary_script(fname);

#ifndef _XML_ONLY_BINARY_SCRIPT_
	binary_script_ = false;
	warning("STUB: parser::parse_file()");
#if 0
	XML_Parser p = XML_ParserCreate(NULL);

	XML_SetUserData(p, this);
	XML_SetElementHandler(p, xml::start_element_handler, xml::end_element_handler);
	XML_SetCharacterDataHandler(p, xml::character_data_handler);

	XML_SetUnknownEncodingHandler(p, unknown_encoding_handler, NULL);
	if (!p) return false;
#endif

	unsigned int fsize = ff.size();

#if 0
	void *buf = XML_GetBuffer(p, fsize);
	if (!buf) return false;

	ff.read(static_cast<char *>(buf), fsize);
	ff.close();
#endif
	if (_data_pool.size() < fsize / 2)
		_data_pool.resize(fsize / 2);

	_tag_stack.push(&_root_tag);

#if 0
	if (XML_ParseBuffer(p, fsize, 1) == XML_STATUS_OK) {
		XML_ParserFree(p);
		return true;
	}

	XML_Error err_code = XML_GetErrorCode(p);

	XBuffer err_buf;
	err_buf < XML_ErrorString(err_code) < "\nLine: " <= XML_GetCurrentLineNumber(p);

	MessageBox(NULL, err_buf.c_str(), "XML Parser error", MB_OK);
	XML_ParserFree(p);
#endif
#endif
	return false;
}

#ifndef _XML_ONLY_BINARY_SCRIPT_
bool parser::read_tag_data(tag &tg, const char *data_ptr, int data_length) {
	if (tg.data_size() && tg.data_format() != tag::TAG_DATA_VOID) {
		if (tg.data_format() == tag::TAG_DATA_STRING) {
			const char *str = UTF8_convert(data_ptr, data_length);
			tg.set_data_size(strlen(str) + 1);

			tg.set_data_offset(_data_pool_position);
			unsigned int sz = tg.data_size() * tg.data_element_size();
			if (_data_pool.size() < _data_pool_position + sz)
				_data_pool.resize(_data_pool_position + sz);

			char *p = &*(_data_pool.begin() + _data_pool_position);
			size_t len = _data_pool.size() - _data_pool_position;
			Common::strlcpy(p, str, len);
			_data_pool_position += sz;

			return true;
		}

		tag_buffer buf(data_ptr, data_length);
		if (tg.data_size() == -1) {
			int sz;
			buf >= sz;
			tg.set_data_size(sz);
		}

		tg.set_data_offset(_data_pool_position);

		unsigned int sz = tg.data_size() * tg.data_element_size();
		if (_data_pool.size() < _data_pool_position + sz)
			_data_pool.resize(_data_pool_position + sz);

		switch (tg.data_format()) {
		case tag::TAG_DATA_SHORT: {
			short *p = reinterpret_cast<short *>(&*(_data_pool.begin() + _data_pool_position));
			for (int j = 0; j < tg.data_size(); j++) buf >= p[j];
		}
		break;
		case tag::TAG_DATA_UNSIGNED_SHORT: {
			unsigned short *p = reinterpret_cast<unsigned short *>(&*(_data_pool.begin() + _data_pool_position));
			for (int j = 0; j < tg.data_size(); j++) buf >= p[j];
		}
		break;
		case tag::TAG_DATA_INT: {
			int *p = reinterpret_cast<int *>(&*(_data_pool.begin() + _data_pool_position));
			for (int j = 0; j < tg.data_size(); j++) buf >= p[j];
		}
		break;
		case tag::TAG_DATA_UNSIGNED_INT: {
			unsigned int *p = reinterpret_cast<unsigned int *>(&*(_data_pool.begin() + _data_pool_position));
			for (int j = 0; j < tg.data_size(); j++) buf >= p[j];
		}
		break;
		case tag::TAG_DATA_FLOAT: {
			float *p = reinterpret_cast<float *>(&*(_data_pool.begin() + _data_pool_position));
			for (int j = 0; j < tg.data_size(); j++) buf >= p[j];
		}
		break;
		}

		_data_pool_position += sz;
	}
	return true;
}
#endif


bool tag::readTag(Common::SeekableReadStream *ff, tag &tg) {
	uint32 id = ff->readUint32LE();
	uint32 data_format = ff->readUint32LE();
	uint32 data_size = ff->readUint32LE();
	uint32 data_offset = ff->readUint32LE();

	tg = tag(tag(id, tag::tag_data_format(data_format), data_size, data_offset));

	uint32 num_subtags = ff->readUint32LE();

	for (int i = 0; i < num_subtags; i++) {
		tag stg;
		readTag(ff, stg);

		tg.add_subtag(stg);
	}

	return true;
}

bool parser::read_binary_script(const char *fname) {
	Common::File ff;
	ff.open(fname);

	_binary_script = true;

	/* uint32 v = */ff.readUint32LE();
	uint32 size = ff.readUint32LE();

	if (_data_pool.size() < size)
		_data_pool.resize(size);

	ff.read(&*_data_pool.begin(), size);

	_root_tag.clear();
	_root_tag.readTag(&ff, _root_tag);

	_root_tag.set_data(&_data_pool);

	ff.close();

	return true;
}

bool parser::is_script_binary(const char *fname) const {
	Common::File ff;
	ff.open(fname);

	uint32 v = ff.readUint32LE();

	if (v == 8383) return true;

	return false;
}

}; /* namespace xml */

} // namespace QDEngine
