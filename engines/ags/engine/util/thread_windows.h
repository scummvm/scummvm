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
#ifndef __AGS_EE_PLATFORM__THREAD_WINDOWS_H
#define __AGS_EE_PLATFORM__THREAD_WINDOWS_H

namespace AGS
{
namespace Engine
{


class WindowsThread : public BaseThread
{
public:
  WindowsThread()
  {
    _thread = NULL;
    _entry = NULL;
    _running = false;
    _looping = false;
  }

  ~WindowsThread()
  {
    Stop();
  }

  inline bool Create(AGSThreadEntry entryPoint, bool looping)
  {
    _looping = looping;
    _entry = entryPoint;
    _thread = CreateThread(NULL, 0, _thread_start, this, CREATE_SUSPENDED, NULL);

    return (_thread != NULL);
  }

  inline bool Start()
  {
    if ((_thread != NULL) && (!_running))
    {
      DWORD result = ResumeThread(_thread);

      _running = (result != (DWORD) - 1);
      return _running;
    }
    else
    {
      return false;
    }
  }

  bool Stop()
  {
    if ((_thread != NULL) && (_running))
    {
      if (_looping)
      {
        _looping = false;
        WaitForSingleObject(_thread, INFINITE);
      }

      CloseHandle(_thread);

      _running = false;
      _thread = NULL;
      return true;
    }
    else
    {
      return false;
    }
  }

private:
  HANDLE _thread;
  bool   _running;
  bool   _looping;

  AGSThreadEntry _entry;

  static DWORD __stdcall _thread_start(LPVOID lpParam)
  {
    AGSThreadEntry entry = ((WindowsThread *)lpParam)->_entry;
    bool *looping = &((WindowsThread *)lpParam)->_looping;

    do
    {
      entry();
    }
    while (*looping);

    return 0;
  }
};


typedef WindowsThread Thread;


} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_PLATFORM__THREAD_WINDOWS_H
