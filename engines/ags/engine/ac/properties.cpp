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

#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/shared/util/string_utils.h"

namespace AGS3 {

using namespace AGS::Shared;




// begin custom property functions

bool get_property_desc(PropertyDesc &desc, const char *property, PropertyType want_type) {
	PropertySchema::const_iterator sch_it = _GP(game).propSchema.find(property);
	if (sch_it == _GP(game).propSchema.end())
		quitprintf("!Did not find property '%s' in the schema. Make sure you are using the property's name, and not its description, when calling this command.", property);

	desc = sch_it->_value;
	if (want_type == kPropertyString && desc.Type != kPropertyString)
		quitprintf("!Property '%s' isn't a text property.  Use GetProperty/SetProperty for non-text properties", property);
	else if (want_type != kPropertyString && desc.Type == kPropertyString)
		quitprintf("!Property '%s' is a text property.  Use GetTextProperty/SetTextProperty for text properties", property);
	return true;
}

String get_property_value(const StringIMap &st_prop, const StringIMap &rt_prop, const char *property, const String def_val) {
	// First check runtime properties, then static properties;
	// if no matching entry was found, use default schema value
	StringIMap::const_iterator it = rt_prop.find(property);
	if (it != rt_prop.end())
		return it->_value;
	it = st_prop.find(property);
	if (it != st_prop.end())
		return it->_value;
	return def_val;
}

// Get an integer property
int get_int_property(const StringIMap &st_prop, const StringIMap &rt_prop, const char *property) {
	PropertyDesc desc;
	if (!get_property_desc(desc, property, kPropertyInteger))
		return 0;
	return StrUtil::StringToInt(get_property_value(st_prop, rt_prop, property, desc.DefaultValue));
}

// Get a string property
void get_text_property(const StringIMap &st_prop, const StringIMap &rt_prop, const char *property, char *bufer) {
	PropertyDesc desc;
	if (!get_property_desc(desc, property, kPropertyString))
		return;

	String val = get_property_value(st_prop, rt_prop, property, desc.DefaultValue);
	snprintf(bufer, MAX_MAXSTRLEN, "%s", val.GetCStr());
}

const char *get_text_property_dynamic_string(const StringIMap &st_prop, const StringIMap &rt_prop, const char *property) {
	PropertyDesc desc;
	if (!get_property_desc(desc, property, kPropertyString))
		return nullptr;

	String val = get_property_value(st_prop, rt_prop, property, desc.DefaultValue);
	return CreateNewScriptString(val.GetCStr());
}

bool set_int_property(StringIMap &rt_prop, const char *property, int value) {
	PropertyDesc desc;
	if (get_property_desc(desc, property, kPropertyInteger)) {
		rt_prop[desc.Name] = StrUtil::IntToString(value);
		return true;
	}
	return false;
}

bool set_text_property(StringIMap &rt_prop, const char *property, const char *value) {
	PropertyDesc desc;
	if (get_property_desc(desc, property, kPropertyString)) {
		rt_prop[desc.Name] = value;
		return true;
	}
	return false;
}

} // namespace AGS3
