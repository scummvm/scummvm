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

#ifndef __AGS_EE_UTIL__LIBRARY_POSIX_H
#define __AGS_EE_UTIL__LIBRARY_POSIX_H

#include <dlfcn.h>
#include "core/platform.h"
#include "util/string.h"
#include "debug/out.h"

// FIXME: Replace with a unified way to get the directory which contains the engine binary
#if AGS_PLATFORM_OS_ANDROID
extern char android_app_directory[256];
#else
extern AGS::Common::String appDirectory;
#endif


namespace AGS
{
namespace Engine
{


class PosixLibrary : BaseLibrary
{
public:
  PosixLibrary()
    : _library(nullptr)
  {
  };

  ~PosixLibrary() override
  {
    Unload();
  };

  AGS::Common::String BuildFilename(AGS::Common::String libraryName)
  {
    return String::FromFormat(
#if AGS_PLATFORM_OS_MACOS
        "lib%s.dylib"
#else
        "lib%s.so"
#endif
        , libraryName.GetCStr());
  }

  AGS::Common::String BuildPath(const char *path, AGS::Common::String libraryName)
  {
    AGS::Common::String platformLibraryName = "";
    if (path)
    {
      platformLibraryName = path;
      platformLibraryName.Append("/");
    }
    platformLibraryName.Append(BuildFilename(libraryName));

    AGS::Common::Debug::Printf("Built library path: %s", platformLibraryName.GetCStr());
    return platformLibraryName;
  }

  AGS::Common::String GetFilenameForLib(AGS::Common::String libraryName) override
  {
    return BuildFilename(libraryName);
  }

  bool Load(AGS::Common::String libraryName) override
  {
    Unload();

    // Try rpath first
    _library = dlopen(BuildPath(nullptr, libraryName).GetCStr(), RTLD_LAZY);
    AGS::Common::Debug::Printf("dlopen returned: %s", dlerror());
    if (_library != nullptr)
    {
      return true;
    }

    // Try current path
    _library = dlopen(BuildPath(".", libraryName).GetCStr(), RTLD_LAZY);

    AGS::Common::Debug::Printf("dlopen returned: %s", dlerror());

    if (_library == nullptr)
    {
      // Try the engine directory

#if AGS_PLATFORM_OS_ANDROID
      char buffer[200];
      sprintf(buffer, "%s%s", android_app_directory, "/lib");
      _library = dlopen(BuildPath(buffer, libraryName).GetCStr(), RTLD_LAZY);
#else
      _library = dlopen(BuildPath(appDirectory, libraryName).GetCStr(), RTLD_LAZY);
#endif

      AGS::Common::Debug::Printf("dlopen returned: %s", dlerror());
    }

    return (_library != nullptr);
  }

  bool Unload() override
  {
    if (_library)
    {
      return (dlclose(_library) == 0);
    }
    else
    {
      return true;
    }
  }

  void *GetFunctionAddress(AGS::Common::String functionName) override
  {
    if (_library)
    {
      return dlsym(_library, functionName.GetCStr());
    }
    else
    {
      return nullptr;
    }
  }

private:
  void *_library;
};


typedef PosixLibrary Library;



} // namespace Engine
} // namespace AGS



#endif // __AGS_EE_UTIL__LIBRARY_POSIX_H
