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

#include "common/str.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_resource.h"
#include "qdengine/qdcore/qd_named_object.h"
#include "qdengine/parser/qdscr_parser.h"


namespace QDEngine {

qdResource::qdResource() : _is_loaded(false) {
}

qdResource::qdResource(const qdResource &res) : _is_loaded(res._is_loaded) {
}

qdResource &qdResource::operator = (const qdResource &res) {
	if (this == &res) return *this;

	_is_loaded = res._is_loaded;

	return *this;
}

qdResource::~qdResource() {
}

qdResource::file_format_t qdResource::file_format(const Common::Path path) {
	Common::String file_name(path.baseName());

	if (file_name.size() < 4)
		return RES_UNKNOWN;

	if (path.isRelativeTo("scummvm"))
		return RES_SPRITE;

	if (file_name.hasSuffixIgnoreCase(".qda")) return RES_ANIMATION;
	if (file_name.hasSuffixIgnoreCase(".tga")) return RES_SPRITE;
	if (file_name.hasSuffixIgnoreCase(".wav")) return RES_SOUND;

	return RES_UNKNOWN;
}

#ifdef __QD_DEBUG_ENABLE__
qdResourceInfo::qdResourceInfo(const qdResource *res, const qdNamedObject *owner) : _resource(res), _data_size(0), _resource_owner(owner) {
	if (_resource)
		_data_size = _resource->resource_data_size();
}

qdResourceInfo::~qdResourceInfo() {
}

qdResource::file_format_t qdResourceInfo::file_format() const {
	if (_resource)
		return qdResource::file_format(_resource->resource_file());

	return qdResource::RES_UNKNOWN;
}

#endif

} // namespace QDEngine
