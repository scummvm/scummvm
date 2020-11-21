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

#ifndef __AGS_EE_UTIL__LIBRARY_DUMMY_H
#define __AGS_EE_UTIL__LIBRARY_DUMMY_H


namespace AGS
{
namespace Engine
{


class DummyLibrary : BaseLibrary
{
public:
  DummyLibrary()
  {
  };

  ~DummyLibrary() override
  {
  };

  AGS::Common::String GetFilenameForLib(AGS::Common::String libraryName) override
  {
      return libraryName;
  }

  bool Load(AGS::Common::String libraryName) override
  {
    return false;
  }

  bool Unload() override
  {
    return true;
  }

  void *GetFunctionAddress(AGS::Common::String functionName) override
  {
    return NULL;
  }
};


typedef DummyLibrary Library;



} // namespace Engine
} // namespace AGS



#endif // __AGS_EE_UTIL__LIBRARY_DUMMY_H
