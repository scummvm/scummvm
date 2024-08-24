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

#ifndef QDENGINE_PARSER_XML_PARSER_H
#define QDENGINE_PARSER_XML_PARSER_H

#include "common/system.h"
#include "common/hashmap.h"
#include "common/stack.h"

#include "qdengine/parser/xml_tag_buffer.h"

namespace QDEngine {

namespace xml {

class parser {
public:
	typedef Common::HashMap<Common::String, tag> tag_format_t;
	typedef Common::Stack<tag *> tag_stack_t;

	parser();
	virtual ~parser();

	bool parse_file(const char *fname);

	bool read_binary_script(const char *fname);
	bool is_script_binary(const char *fname) const;
	bool is_script_binary() const {
		return _binary_script;
	}

	const tag &root_tag() const {
		return _root_tag;
	}

	void clear();

	void resize_data_pool(uint32 pool_sz) {
		_data_pool.resize(pool_sz);
	}

	bool register_tag_format(const char *tag_name, const tag &tg) {
		tag_format_t::iterator it = _tag_format.find(tag_name);
		if (it != _tag_format.end())
			return false;

		_tag_format[tag_name] = tg;
		return true;
	}
	const tag *get_tag_format(const char *tag_name) const {
		tag_format_t::const_iterator it = _tag_format.find(tag_name);
		if (it != _tag_format.end())
			return &it->_value;

		return NULL;
	}

	int num_tag_formats() const {
		return _tag_format.size();
	}

private:

	tag _root_tag;

	Std::vector<char> _data_pool;
	Common::String _data_buffer;

	bool _binary_script;

	tag_stack_t _tag_stack;
	tag_format_t _tag_format;
	int _cur_level;
	bool _skip_mode;
};

} /* namespace xml */

} // namespace QDEngine

#endif // QDENGINE_PARSER_XML_PARSER_H
