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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/str.h"
#include "qdengine/qd_precomp.h"

#include "qdengine/qdcore/qd_resource.h"
#include "qdengine/qdcore/qd_named_object.h"
#include "qdengine/parser/qdscr_parser.h"


namespace QDEngine {

qdResource::qdResource() : is_loaded_(false) {
}

qdResource::qdResource(const qdResource &res) : is_loaded_(res.is_loaded_) {
}

qdResource &qdResource::operator = (const qdResource &res) {
	if (this == &res) return *this;

	is_loaded_ = res.is_loaded_;

	return *this;
}

qdResource::~qdResource() {
}

qdResource::file_format_t qdResource::file_format(const char *file_name) {
	char ext[_MAX_EXT];
	uint len = strlen(file_name);

	if (len < 4)
	return RES_UNKNOWN;

	Common::strlcpy(ext, &file_name[len - 4], _MAX_EXT);

	if (!scumm_stricmp(ext, ".qda")) return RES_ANIMATION;
	if (!scumm_stricmp(ext, ".tga")) return RES_SPRITE;
	if (!scumm_stricmp(ext, ".wav")) return RES_SOUND;

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
