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

#ifndef __AGS_EE_PLATFORM__THREAD_STD_H
#define __AGS_EE_PLATFORM__THREAD_STD_H

#include <system_error>
#include <thread>

namespace AGS
{
namespace Engine
{

class StdThread : public BaseThread
{
public:
  StdThread() : thread_(), entry_(nullptr), looping_(false)
  {
  }

  ~StdThread() override
  {
    Stop();
  }

  StdThread &operator=(const StdThread &) = delete;
  StdThread(const StdThread &) = delete;

  bool Create(AGSThreadEntry entryPoint, bool looping) override
  {
    if (!entryPoint) { return false; }

    entry_ = entryPoint;
    looping_ = looping;
    return true;
  }

  bool Start() override
  {
    if (thread_.joinable()) { return true; }
    if (!entry_) { return false; }

    try {
      thread_ = std::thread(thread_start_, this);
    } catch (std::system_error) {
      return false;
    }
    return thread_.joinable();
  }

  bool Stop() override
  {
    if (!thread_.joinable()) { return true; }

    looping_ = false; // signal thread to stop
    thread_.join();
    return true;
  }

private:
  std::thread thread_;
  AGSThreadEntry entry_;
  bool looping_;

  static void thread_start_(StdThread *self)
  {
    auto entry = self->entry_;
    for (;;)
    {
      entry();
      if (!self->looping_)
      {
        break;
      }
      std::this_thread::yield();
    }
  }
};

typedef StdThread Thread;

} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_PLATFORM__THREAD_STD_H
