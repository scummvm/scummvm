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
#ifndef __AC_SCRIPTCONTAINERS_H
#define __AC_SCRIPTCONTAINERS_H

class ScriptDictBase;
class ScriptSetBase;

// Create and register new dictionary
ScriptDictBase *Dict_Create(bool sorted, bool case_sensitive);
// Unserialize dictionary from the memory stream
ScriptDictBase *Dict_Unserialize(int index, const char *serializedData, int dataSize);
// Create and register new set
ScriptSetBase *Set_Create(bool sorted, bool case_sensitive);
// Unserialize set from the memory stream
ScriptSetBase *Set_Unserialize(int index, const char *serializedData, int dataSize);

#endif // __AC_SCRIPTCONTAINERS_H
