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

#ifndef __AGS_EE_PLATFORM__THREAD_WII_H
#define __AGS_EE_PLATFORM__THREAD_WII_H

#include <gccore.h>

namespace AGS
{
namespace Engine
{


class WiiThread : public BaseThread
{
public:
  WiiThread()
  {
    _thread = (lwp_t)NULL;
    _entry = NULL;
    _running = false;
    _looping = false;
  }

  ~WiiThread()
  {
    Stop();
  }

  inline bool Create(AGSThreadEntry entryPoint, bool looping)
  {
    _looping = looping;
    _entry = entryPoint;

    // Thread creation is delayed till the thread is started
    return true;
  }

  inline bool Start()
  {
    if (!_running)
    {
      _running = (LWP_CreateThread(&_thread, _thread_start, this, 0, 8 * 1024, 64) != 0);

      return _running;
    }
    else
    {
      return false;
    }
  }

  bool Stop()
  {
    if (_running)
    {
      if (_looping)
      {
        _looping = false;
      }

      LWP_JoinThread(_thread, NULL);

      _running = false;
      return true;
    }
    else
    {
      return false;
    }
  }

private:
  lwp_t     _thread;
  bool      _running;
  bool      _looping;

  AGSThreadEntry _entry;

  static void *_thread_start(void *arg)
  {
    AGSThreadEntry entry = ((WiiThread *)arg)->_entry;
    bool *looping = &((WiiThread *)arg)->_looping;

    do
    {
      entry();
    }
    while (*looping);

    return NULL;
  }
};


typedef WiiThread Thread;


} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_PLATFORM__THREAD_WII_H
