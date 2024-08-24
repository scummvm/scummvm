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

#include "common/file.h"
#include "common/textconsole.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_parser.h"

namespace QDEngine {

namespace xml {

parser::parser() : _cur_level(0), _skip_mode(false), _binary_script(false) {
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
	Std::vector<char>(_data_pool).swap(_data_pool);
}

bool parser::parse_file(const char *fname) {
	if (is_script_binary(fname))
		return read_binary_script(fname);

	return false;
}


bool tag::readTag(Common::SeekableReadStream *ff, tag &tg) {
	uint32 id = ff->readUint32LE();
	uint32 data_format = ff->readUint32LE();
	uint32 data_size = ff->readUint32LE();
	uint32 data_offset = ff->readUint32LE();

	tg = tag(tag(id, tag::tag_data_format(data_format), data_size, data_offset));

	uint32 num_subtags = ff->readUint32LE();

	for (uint32 i = 0; i < num_subtags; i++) {
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

} /* namespace xml */

} // namespace QDEngine
