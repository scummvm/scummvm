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
#ifndef __AGS_EE_UTIL__LIBRARY_WINDOWS_H
#define __AGS_EE_UTIL__LIBRARY_WINDOWS_H

#include "debug/out.h"
#include "platform/windows/winapi_exclusive.h"
#include "util/string.h"

// Because this class may be exposed to generic code in sake of inlining,
// we should avoid including <windows.h> full of macros with common names.
#ifdef __cplusplus
extern "C" {
#endif

    WINBASEAPI BOOL WINAPI FreeLibrary(HMODULE hLibModule);
    WINBASEAPI FARPROC WINAPI GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
    WINBASEAPI HMODULE WINAPI LoadLibraryA(LPCSTR lpLibFileName);

#ifdef __cplusplus
} // extern "C"
#endif


namespace AGS
{
namespace Engine
{


class WindowsLibrary : BaseLibrary
{
public:
  WindowsLibrary()
    : _library(NULL)
  {
  };

  virtual ~WindowsLibrary()
  {
    Unload();
  };

  AGS::Common::String BuildFilename(AGS::Common::String libraryName)
  {
    return String::FromFormat("%s.dll", libraryName.GetCStr());
  }

  AGS::Common::String BuildPath(AGS::Common::String libraryName)
  {
    AGS::Common::String platformLibraryName = BuildFilename(libraryName);

    AGS::Common::Debug::Printf("Built library path: %s", platformLibraryName.GetCStr());

    return platformLibraryName;
  }

  AGS::Common::String GetFilenameForLib(AGS::Common::String libraryName) override
  {
    return BuildFilename(libraryName);
  }

  bool Load(AGS::Common::String libraryName)
  {
    Unload();

    _library = LoadLibraryA(BuildPath(libraryName).GetCStr());

    return (_library != NULL);
  }

  bool Unload()
  {
    if (_library)
    {
      return (FreeLibrary(_library) != 0);
    }
    else
    {
      return true;
    }
  }

  void *GetFunctionAddress(AGS::Common::String functionName)
  {
    return GetProcAddress(_library, functionName.GetCStr());
  }

private:
  HANDLE _library;
};


typedef WindowsLibrary Library;



} // namespace Engine
} // namespace AGS



#endif // __AGS_EE_UTIL__LIBRARY_WINDOWS_H
