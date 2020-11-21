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


#ifndef __AGS_EE_UTIL__LIBRARY_PSP_H
#define __AGS_EE_UTIL__LIBRARY_PSP_H

#include <pspsdk.h>
#include "util/string.h"
#include "debug/out.h"

namespace AGS
{
namespace Engine
{

class PSPLibrary : BaseLibrary
{
public:
  PSPLibrary()
    : _library(-1)
  {
  };

  virtual ~PSPLibrary()
  {
    Unload();
  };

  AGS::Common::String BuildPath(char *path, AGS::Common::String libraryName)
  {
    AGS::Common::String platformLibraryName = path;
    platformLibraryName.Append(libraryName);
    platformLibraryName.Append(".prx");

    AGS::Common::Debug::Printf("Built library path: %s", platformLibraryName.GetCStr());

    return platformLibraryName;
  }

  bool Load(AGS::Common::String libraryName)
  {
    Unload();

    // Try current path
    _library = pspSdkLoadStartModule(BuildPath("./", libraryName).GetCStr(), PSP_MEMORY_PARTITION_USER);

    if (_library < 0)
    {
      // Try one directory higher, usually the AGS base directory
      _library = pspSdkLoadStartModule(BuildPath("../", libraryName).GetCStr(), PSP_MEMORY_PARTITION_USER);
    }

    // The PSP module and PSP library name are assumed to be the same as the file name
    _moduleName = libraryName;
    _moduleName.MakeLower();

    AGS::Common::Debug::Printf("Result is %s %d", _moduleName.GetCStr(), _library);

    return (_library > -1);
  }

  bool Unload()
  {
    if (_library > -1)
    {
      return (sceKernelUnloadModule(_library) > -1);
    }
    else
    {
      return true;
    }
  }

  void *GetFunctionAddress(AGS::Common::String functionName)
  {
    if (_library > -1)
    {
      // On the PSP functions are identified by an ID that is the first 4 byte of the SHA1 of the name.
      int functionId;

#if 1
      // Hardcoded values for plugin loading.
      if (functionName == "AGS_PluginV2")
      {
        functionId = 0x960C49BD;
      }
      else if (functionName == "AGS_EngineStartup")
      {
        functionId = 0x0F13D9E8;
      }
      else if (functionName == "AGS_EngineShutdown")
      {
        functionId = 0x2F131C76;
      }
      else if (functionName == "AGS_EngineOnEvent")
      {
        functionId = 0xE3DFFC5A;
      }
      else if (functionName == "AGS_EngineDebugHook")
      {
        functionId = 0xC37D6879;
      }
      else if (functionName == "AGS_EngineInitGfx")
      {
        functionId = 0xA428D254;
      }
      else
      {
        AGS::Common::Debug::Printf("Function ID not found: %s", functionName.GetCStr());
        functionId = -1;
      }
#else
      // This is a possible SHA1 implementation.
      SceKernelUtilsSha1Context ctx;
      uint8_t digest[20];
      sceKernelUtilsSha1BlockInit(&ctx);
      sceKernelUtilsSha1BlockUpdate(&ctx, (uint8_t *)functionName.GetCStr(), functionName.GetLength());
      sceKernelUtilsSha1BlockResult(&ctx, digest);

      functionId = strtol(digest, NULL, 8);
#endif

      return (void *)kernel_sctrlHENFindFunction((char *)_moduleName.GetCStr(), (char *)_moduleName.GetCStr(), functionId);
    }
    else
    {
      return NULL;
    }
  }

private:
  SceUID _library;
  AGS::Common::String _moduleName;
};


typedef PSPLibrary Library;



} // namespace Engine
} // namespace AGS



#endif // __AGS_EE_UTIL__LIBRARY_PSP_H
