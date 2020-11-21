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

#ifndef __AGS_EE_PLATFORM__MUTEX_BASE_H
#define __AGS_EE_PLATFORM__MUTEX_BASE_H


namespace AGS
{
namespace Common
{


class BaseMutex
{
public:
  BaseMutex() = 0;
  virtual ~BaseMutex() = 0;
  virtual void Lock() = 0;
  virtual void Unlock() = 0;
};


} // namespace Common
} // namespace AGS

#endif // __AGS_EE_PLATFORM__MUTEX_BASE_H
