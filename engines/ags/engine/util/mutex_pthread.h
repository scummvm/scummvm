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

#ifndef __AGS_EE_UTIL__MUTEX_PTHREAD_H
#define __AGS_EE_UTIL__MUTEX_PTHREAD_H

#include <pthread.h>

namespace AGS
{
namespace Engine
{


class PThreadMutex : public BaseMutex
{
public:
  inline PThreadMutex()
  {
    pthread_mutex_init(&_mutex, NULL);
  }

  inline ~PThreadMutex()
  {
    pthread_mutex_destroy(&_mutex);
  }

  inline void Lock()
  {
    pthread_mutex_lock(&_mutex);
  }

  inline void Unlock()
  {
    pthread_mutex_unlock(&_mutex);
  }

private:
  pthread_mutex_t _mutex;
};

typedef PThreadMutex Mutex;


} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_UTIL__MUTEX_PTHREAD_H
