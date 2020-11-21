//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
//
//
//=============================================================================
#ifndef __AGS_EE_AC__PROPERTIES_H
#define __AGS_EE_AC__PROPERTIES_H

#include "game/customproperties.h"

using AGS::Common::StringIMap;

// Getting a property value requires static and runtime property maps.
// Key is first searched in runtime map, if not found - static map is taken,
// which contains original property values for particular game entity.
// Lastly, if the key is still not found, then the default schema value is
// returned for the given property.
int get_int_property(const StringIMap &st_prop, const StringIMap &rt_prop, const char *property);
void get_text_property(const StringIMap &st_prop, const StringIMap &rt_prop, const char *property, char *bufer);
const char* get_text_property_dynamic_string(const StringIMap &st_prop, const StringIMap &rt_prop, const char *property);

bool set_int_property(StringIMap &rt_prop, const char *property, int value);
bool set_text_property(StringIMap &rt_prop, const char *property, const char* value);

#endif // __AGS_EE_AC__PROPERTIES_H
