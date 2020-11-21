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

#ifndef __AGS_EE_UTIL__LIBRARY_DUMMY_H
#define __AGS_EE_UTIL__LIBRARY_DUMMY_H


namespace AGS
{
namespace Engine
{


class DummyLibrary : BaseLibrary
{
public:
  DummyLibrary()
  {
  };

  ~DummyLibrary() override
  {
  };

  AGS::Common::String GetFilenameForLib(AGS::Common::String libraryName) override
  {
      return libraryName;
  }

  bool Load(AGS::Common::String libraryName) override
  {
    return false;
  }

  bool Unload() override
  {
    return true;
  }

  void *GetFunctionAddress(AGS::Common::String functionName) override
  {
    return NULL;
  }
};


typedef DummyLibrary Library;



} // namespace Engine
} // namespace AGS



#endif // __AGS_EE_UTIL__LIBRARY_DUMMY_H
