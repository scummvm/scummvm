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

#include "ags/shared/game/custom_properties.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_utils.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

PropertyDesc::PropertyDesc() {
	Type = kPropertyBoolean;
}

PropertyDesc::PropertyDesc(const String &name, PropertyType type, const String &desc, const String &def_value) {
	Name = name;
	Type = type;
	Description = desc;
	DefaultValue = def_value;
}


namespace Properties {

PropertyError ReadSchema(PropertySchema &schema, Stream *in) {
	PropertyVersion version = (PropertyVersion)in->ReadInt32();
	if (version < kPropertyVersion_Initial ||
	        version > kPropertyVersion_Current) {
		return kPropertyErr_UnsupportedFormat;
	}

	PropertyDesc prop;
	int count = in->ReadInt32();
	if (version == kPropertyVersion_Initial) {
		for (int i = 0; i < count; ++i) {
			prop.Name.Read(in, LEGACY_MAX_CUSTOM_PROP_SCHEMA_NAME_LENGTH);
			prop.Description.Read(in, LEGACY_MAX_CUSTOM_PROP_DESC_LENGTH);
			prop.DefaultValue.Read(in, LEGACY_MAX_CUSTOM_PROP_VALUE_LENGTH);
			prop.Type = (PropertyType)in->ReadInt32();
			schema[prop.Name] = prop;
		}
	} else {
		for (int i = 0; i < count; ++i) {
			prop.Name = StrUtil::ReadString(in);
			prop.Type = (PropertyType)in->ReadInt32();
			prop.Description = StrUtil::ReadString(in);
			prop.DefaultValue = StrUtil::ReadString(in);
			schema[prop.Name] = prop;
		}
	}
	return kPropertyErr_NoError;
}

void WriteSchema(const PropertySchema &schema, Stream *out) {
	out->WriteInt32(kPropertyVersion_Current);
	out->WriteInt32(schema.size());
	for (PropertySchema::const_iterator it = schema.begin();
	        it != schema.end(); ++it) {
		const PropertyDesc &prop = it->_value;
		StrUtil::WriteString(prop.Name, out);
		out->WriteInt32(prop.Type);
		StrUtil::WriteString(prop.Description, out);
		StrUtil::WriteString(prop.DefaultValue, out);
	}
}

PropertyError ReadValues(StringIMap &map, Stream *in) {
	PropertyVersion version = (PropertyVersion)in->ReadInt32();
	if (version < kPropertyVersion_Initial ||
	        version > kPropertyVersion_Current) {
		return kPropertyErr_UnsupportedFormat;
	}

	int count = in->ReadInt32();
	if (version == kPropertyVersion_Initial) {
		for (int i = 0; i < count; ++i) {
			String name = String::FromStream(in, LEGACY_MAX_CUSTOM_PROP_NAME_LENGTH);
			map[name] = String::FromStream(in, LEGACY_MAX_CUSTOM_PROP_VALUE_LENGTH);
		}
	} else {
		for (int i = 0; i < count; ++i) {
			String name = StrUtil::ReadString(in);
			map[name] = StrUtil::ReadString(in);
		}
	}
	return kPropertyErr_NoError;
}

void WriteValues(const StringIMap &map, Stream *out) {
	out->WriteInt32(kPropertyVersion_Current);
	out->WriteInt32(map.size());
	for (StringIMap::const_iterator it = map.begin();
	        it != map.end(); ++it) {
		StrUtil::WriteString(it->_key, out);
		StrUtil::WriteString(it->_value, out);
	}
}

} // namespace Properties

} // namespace Shared
} // namespace AGS
} // namespace AGS3
