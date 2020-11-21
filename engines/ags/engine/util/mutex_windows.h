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
#ifndef __AGS_EE_UTIL__WINDOWS_MUTEX_H
#define __AGS_EE_UTIL__WINDOWS_MUTEX_H

namespace AGS
{
namespace Engine
{


class WindowsMutex : public BaseMutex
{
public:
  WindowsMutex()
  {
    _mutex = CreateMutex(NULL, FALSE, NULL);

    _ASSERT(_mutex != NULL);
  }

  ~WindowsMutex()
  {
    _ASSERT(_mutex != NULL);

    CloseHandle(_mutex);
  }

  inline void Lock()
  {
    _ASSERT(_mutex != NULL);

    WaitForSingleObject(_mutex, INFINITE);
  }

  inline void Unlock()
  {
    _ASSERT(_mutex != NULL);

    ReleaseMutex(_mutex);
  }

private:
  HANDLE _mutex;
};


typedef WindowsMutex Mutex;


} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_UTIL__WINDOWS_MUTEX_H
