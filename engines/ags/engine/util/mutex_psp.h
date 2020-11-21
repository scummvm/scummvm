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

#ifndef __AGS_EE_UTIL__PSP_MUTEX_H
#define __AGS_EE_UTIL__PSP_MUTEX_H

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman.h>

namespace AGS
{
namespace Engine
{


class PSPMutex : public BaseMutex
{
public:
  PSPMutex()
  {
    _mutex = sceKernelCreateSema("", 0, 1, 1, 0);
  }

  ~PSPMutex()
  {
    sceKernelDeleteSema(_mutex);
  }

  inline void Lock()
  {
    sceKernelWaitSema(_mutex, 1, 0);
  }

  inline void Unlock()
  {
    sceKernelSignalSema(_mutex, 1);
  }

private:
  SceUID _mutex;
};


typedef PSPMutex Mutex;


} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_UTIL__PSP_MUTEX_H
