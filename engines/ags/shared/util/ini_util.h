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
// Functions for exchanging configuration data between key-value tree and
// INI file.
//
//=============================================================================
#ifndef __AGS_CN_UTIL__INIUTIL_H
#define __AGS_CN_UTIL__INIUTIL_H

#include <map>
#include "util/string.h"

namespace AGS
{
namespace Common
{

typedef std::map<String, String>         StringOrderMap;
typedef StringOrderMap::const_iterator   StrStrOIter;

typedef std::map<String, StringOrderMap> ConfigTree;
typedef ConfigTree::const_iterator       ConfigNode;

namespace IniUtil
{
    // Parse the contents of given file as INI format and insert values
    // into the tree. The pre-existing tree items, if any, are NOT erased.
    // Returns FALSE if the file could not be opened.
    bool Read(const String &file, ConfigTree &tree);
    // Serialize given tree to the stream in INI text format.
    // The INI format suggests only one nested level (group - items).
    // The first level values are treated as a global section items.
    // The sub-nodes beyond 2nd level are ignored completely.
    void Write(const String &file, const ConfigTree &tree);
    // Serialize given tree to the string in INI text format.
    // TODO: implement proper memory/string stream compatible with base Stream
    // class and merge this with Write function.
    void WriteToString(String &s, const ConfigTree &tree);
    // Parse the contents of given source stream as INI format and merge
    // with values of the given tree while doing only minimal replaces;
    // write the result into destination stream.
    // If item already exists, only value is overwrited, if section exists,
    // new items are appended to the end of it; completely new sections are
    // appended to the end of text.
    // Source and destination streams may refer either to different objects,
    // or same stream opened for both reading and writing.
    // Returns FALSE if the file could not be opened for writing.
    bool Merge(const String &file, const ConfigTree &tree);
};

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_UTIL__INIUTIL_H
