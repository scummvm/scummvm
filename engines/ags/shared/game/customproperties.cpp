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

#include "game/customproperties.h"
#include "util/stream.h"
#include "util/string_utils.h"

namespace AGS
{
namespace Common
{

PropertyDesc::PropertyDesc()
{
    Type = kPropertyBoolean;
}

PropertyDesc::PropertyDesc(const String &name, PropertyType type, const String &desc, const String &def_value)
{
    Name = name;
    Type = type;
    Description = desc;
    DefaultValue = def_value;
}


namespace Properties
{

PropertyError ReadSchema(PropertySchema &schema, Stream *in)
{
    PropertyVersion version = (PropertyVersion)in->ReadInt32();
    if (version < kPropertyVersion_Initial ||
        version > kPropertyVersion_Current)
    {
        return kPropertyErr_UnsupportedFormat;
    }

    PropertyDesc prop;
    int count = in->ReadInt32();
    if (version == kPropertyVersion_Initial)
    {
        for (int i = 0; i < count; ++i)
        {
            prop.Name.Read(in, LEGACY_MAX_CUSTOM_PROP_SCHEMA_NAME_LENGTH);
            prop.Description.Read(in, LEGACY_MAX_CUSTOM_PROP_DESC_LENGTH);
            prop.DefaultValue.Read(in, LEGACY_MAX_CUSTOM_PROP_VALUE_LENGTH);
            prop.Type = (PropertyType)in->ReadInt32();
            schema[prop.Name] = prop;
        }
    }
    else
    {
        for (int i = 0; i < count; ++i)
        {
            prop.Name = StrUtil::ReadString(in);
            prop.Type = (PropertyType)in->ReadInt32();
            prop.Description = StrUtil::ReadString(in);
            prop.DefaultValue = StrUtil::ReadString(in);
            schema[prop.Name] = prop;
        }
    }
    return kPropertyErr_NoError;
}

void WriteSchema(const PropertySchema &schema, Stream *out)
{
    out->WriteInt32(kPropertyVersion_Current);
    out->WriteInt32(schema.size());
    for (PropertySchema::const_iterator it = schema.begin();
         it != schema.end(); ++it)
    {
        const PropertyDesc &prop = it->second;
        StrUtil::WriteString(prop.Name, out);
        out->WriteInt32(prop.Type);
        StrUtil::WriteString(prop.Description, out);
        StrUtil::WriteString(prop.DefaultValue, out);
    }
}

PropertyError ReadValues(StringIMap &map, Stream *in)
{
    PropertyVersion version = (PropertyVersion)in->ReadInt32();
    if (version < kPropertyVersion_Initial ||
        version > kPropertyVersion_Current)
    {
        return kPropertyErr_UnsupportedFormat;
    }

    int count = in->ReadInt32();
    if (version == kPropertyVersion_Initial)
    {
        for (int i = 0; i < count; ++i)
        {
            String name  = String::FromStream(in, LEGACY_MAX_CUSTOM_PROP_NAME_LENGTH);
            map[name] = String::FromStream(in, LEGACY_MAX_CUSTOM_PROP_VALUE_LENGTH);
        }
    }
    else
    {
        for (int i = 0; i < count; ++i)
        {
            String name  = StrUtil::ReadString(in);
            map[name] = StrUtil::ReadString(in);
        }
    }
    return kPropertyErr_NoError;
}

void WriteValues(const StringIMap &map, Stream *out)
{
    out->WriteInt32(kPropertyVersion_Current);
    out->WriteInt32(map.size());
    for (StringIMap::const_iterator it = map.begin();
         it != map.end(); ++it)
    {
        StrUtil::WriteString(it->first, out);
        StrUtil::WriteString(it->second, out);
    }
}

} // namespace Properties

} // namespace Common
} // namespace AGS
