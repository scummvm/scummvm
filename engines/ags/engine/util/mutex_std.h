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

#ifndef __AGS_EE_UTIL__MUTEX_STD_H
#define __AGS_EE_UTIL__MUTEX_STD_H

#include <mutex>

namespace AGS
{
namespace Engine
{

class StdMutex : public BaseMutex
{
  public:
    inline StdMutex() : mutex_() {}
    inline ~StdMutex() override = default;

    StdMutex &operator=(const StdMutex &) = delete;
    StdMutex(const StdMutex &) = delete;

    inline void Lock() override { mutex_.lock(); }
    inline void Unlock() override { mutex_.unlock(); }

  private:
    std::recursive_mutex mutex_;
};

typedef StdMutex Mutex;

} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_UTIL__MUTEX_STD_H
