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

//=============================================================================
//
// Custom property structs
//
//-----------------------------------------------------------------------------
//
// Custom property schema is kept by GameSetupStruct object as a single
// instance and defines property type and default value. Every game entity that
// has properties implemented keeps CustomProperties object, which stores
// actual property values only if ones are different from defaults.
//
//=============================================================================

#ifndef AGS_SHARED_GAME_CUSTOM_PROPERTIES_H
#define AGS_SHARED_GAME_CUSTOM_PROPERTIES_H

#include "ags/lib/std/map.h"
#include "ags/shared/util/string.h"
#include "ags/shared/util/string_types.h"

namespace AGS3 {

#define LEGACY_MAX_CUSTOM_PROPERTIES                30
// NOTE: for some reason the property name stored in schema object was limited
// to only 20 characters, while the custom properties map could hold up to 200.
// Whether this was an error or design choice is unknown.
#define LEGACY_MAX_CUSTOM_PROP_SCHEMA_NAME_LENGTH   20
#define LEGACY_MAX_CUSTOM_PROP_NAME_LENGTH          200
#define LEGACY_MAX_CUSTOM_PROP_DESC_LENGTH          100
#define LEGACY_MAX_CUSTOM_PROP_VALUE_LENGTH         500

namespace AGS {
namespace Shared {

enum PropertyVersion {
	kPropertyVersion_Initial = 1,
	kPropertyVersion_340,
	kPropertyVersion_Current = kPropertyVersion_340
};

enum PropertyType {
	kPropertyUndefined = 0,
	kPropertyBoolean,
	kPropertyInteger,
	kPropertyString
};

enum PropertyError {
	kPropertyErr_NoError,
	kPropertyErr_UnsupportedFormat
};

//
// PropertyDesc - a description of a single custom property
//
struct PropertyDesc {
	String       Name;
	PropertyType Type;
	String       Description;
	String       DefaultValue;

	PropertyDesc();
	PropertyDesc(const String &name, PropertyType type, const String &desc, const String &def_value);
};

// NOTE: AGS has case-insensitive property IDs
// Schema - a map of property descriptions
typedef std::unordered_map<String, PropertyDesc, IgnoreCase_Hash, IgnoreCase_EqualTo> PropertySchema;


namespace Properties {
PropertyError ReadSchema(PropertySchema &schema, Stream *in);
void WriteSchema(const PropertySchema &schema, Stream *out);

// Reads property values from the stream and assign them to map.
// The non-matching existing map items, if any, are NOT erased.
PropertyError ReadValues(StringIMap &map, Stream *in);
// Writes property values chunk to the stream
void WriteValues(const StringIMap &map, Stream *out);

} // namespace Properties

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
