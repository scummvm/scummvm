/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef __AGS_EE_UTIL__LIBRARY_H
#define __AGS_EE_UTIL__LIBRARY_H

#include "core/platform.h"
#include "util/string.h"

namespace AGS
{
namespace Engine
{


class BaseLibrary
{
public:
  BaseLibrary() = default;

  virtual ~BaseLibrary() = default;

  virtual AGS::Common::String GetFilenameForLib(AGS::Common::String libraryName) = 0;

  virtual bool Load(AGS::Common::String libraryName) = 0;

  virtual bool Unload() = 0;

  virtual void *GetFunctionAddress(AGS::Common::String functionName) = 0;
};


} // namespace Engine
} // namespace AGS


#if AGS_PLATFORM_OS_WINDOWS
#include "library_windows.h"

#elif AGS_PLATFORM_OS_LINUX \
   || AGS_PLATFORM_OS_MACOS \
   || AGS_PLATFORM_OS_ANDROID
#include "library_posix.h"

#elif AGS_PLATFORM_OS_IOS
#include "library_dummy.h"

#endif


#endif // __AGS_EE_UTIL__MUTEX_H
