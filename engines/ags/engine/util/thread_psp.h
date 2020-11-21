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

#ifndef __AGS_EE_UTIL__PSP_THREAD_H
#define __AGS_EE_UTIL__PSP_THREAD_H

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman.h>

namespace AGS
{
namespace Engine
{


class PSPThread : public BaseThread
{
public:
  PSPThread()
  {
    _thread = -1;
    _entry = NULL;
    _running = false;
    _looping = false;
  }

  ~PSPThread()
  {
    Stop();
  }

  inline bool Create(AGSThreadEntry entryPoint, bool looping)
  {
    _looping = looping;
    _entry = entryPoint;
    _thread = sceKernelCreateThread("ags", _thread_start, 0x20, 0x10000, THREAD_ATTR_USER, 0);

    return (_thread > -1);
  }

  inline bool Start()
  {
    if ((_thread > -1) && (!_running))
    {
      PSPThread* thisPointer = this;
      SceUID result = sceKernelStartThread(_thread, sizeof(this), &thisPointer);

      _running = (result > -1);
      return _running;
    }
    else
    {
      return false;
    }
  }

  bool Stop()
  {
    if ((_thread > -1) && (_running))
    {
      if (_looping)
      {
        _looping = false;
        sceKernelWaitThreadEnd(_thread, 0);
      }

      _running = false;
      return (sceKernelTerminateDeleteThread(_thread) > -1);
    }
    else
    {
      return false;
    }
  }

private:
  SceUID _thread;
  bool   _running;
  bool   _looping;

  AGSThreadEntry _entry;

  static int _thread_start(SceSize args, void *argp)
  {
    AGSThreadEntry entry = (*(PSPThread **)argp)->_entry;
    bool *looping = &(*(PSPThread **)argp)->_looping;

    do
    {
      entry();
    }
    while (*looping);
  }
};


typedef PSPThread Thread;


} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_UTIL__PSP_THREAD_H
