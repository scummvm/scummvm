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

#ifndef AGS_ENGINE_AC_PROPERTIES_H
#define AGS_ENGINE_AC_PROPERTIES_H

#include "ags/shared/game/custom_properties.h"

namespace AGS3 {

using AGS::Shared::StringIMap;

// Getting a property value requires static and runtime property maps.
// Key is first searched in runtime map, if not found - static map is taken,
// which contains original property values for particular game entity.
// Lastly, if the key is still not found, then the default schema value is
// returned for the given property.
int get_int_property(const StringIMap &st_prop, const StringIMap &rt_prop, const char *property);
void get_text_property(const StringIMap &st_prop, const StringIMap &rt_prop, const char *property, char *bufer);
const char *get_text_property_dynamic_string(const StringIMap &st_prop, const StringIMap &rt_prop, const char *property);

bool set_int_property(StringIMap &rt_prop, const char *property, int value);
bool set_text_property(StringIMap &rt_prop, const char *property, const char *value);

} // namespace AGS3

#endif
