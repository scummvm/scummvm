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

/* ---------------------------- INCLUDE SECTION ----------------------------- */

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/str.h"
#include "qdengine/core/qd_precomp.h"

#ifdef __QD_DEBUG_ENABLE__
#include <stdio.h>
#endif

#include "qdengine/core/qdcore/qd_resource.h"
#include "qdengine/core/qdcore/qd_named_object.h"
#include "qdengine/core/parser/qdscr_parser.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

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
qdResourceInfo::qdResourceInfo(const qdResource *res, const qdNamedObject *owner) : resource_(res), data_size_(0), resource_owner_(owner) {
	if (resource_)
		data_size_ = resource_->resource_data_size();
}

qdResourceInfo::~qdResourceInfo() {
}

qdResource::file_format_t qdResourceInfo::file_format() const {
	if (resource_)
		return qdResource::file_format(resource_->resource_file());

	return qdResource::RES_UNKNOWN;
}

bool qdResourceInfo::write(XStream &fh, int line_class_id) const {
	if (!resource_) return false;

	fh < "<tr";

	if (line_class_id != -1) {
		if (line_class_id) fh < " class=\"line1\"";
		else fh < " class=\"line0\"";
	}

	static XBuffer name;
	name.init();

	if (resource_owner_)
		name < resource_owner_->name();
	else
		name < "???";

	fh < "><td nowrap class=\"name\">" < qdscr_XML_string(name.c_str());
	fh < "</td><td nowrap>" < qdscr_XML_string(resource_->resource_file()) < "</td><td class=\"to_r\">";

	static char buf[1024];
	float sz = float(data_size_) / (1024.0f * 1024.0f);
	sprintf(buf, "%.2f", sz);
	fh < buf;

	fh < "</td></tr>\r\n";

	return true;
}
#endif
} // namespace QDEngine
