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

#ifndef __AGS_EE_UTIL__THREAD_H
#define __AGS_EE_UTIL__THREAD_H

namespace AGS
{
namespace Engine
{


class BaseThread
{
public:
  typedef void(* AGSThreadEntry)();

  BaseThread() = default;
  virtual ~BaseThread() = default;

  BaseThread &operator=(const BaseThread &) = delete;
  BaseThread(const BaseThread &) = delete;

  virtual bool Create(AGSThreadEntry entryPoint, bool looping) = 0;
  virtual bool Start() = 0;
  virtual bool Stop() = 0;

  inline bool CreateAndStart(AGSThreadEntry entryPoint, bool looping)
  {
    if (!Create(entryPoint, looping)) { return false; }
    return Start();
  }
};

} // namespace Engine
} // namespace AGS

#if 0
  // insert platforms here
#else
#include "thread_std.h"
#endif


#endif // __AGS_EE_UTIL__THREAD_H
