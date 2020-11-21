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

//=============================================================================
//
// Graphics driver exception class
//
//=============================================================================

#ifndef __AGS_EE_GFX__ALI3DEXCEPTION_H
#define __AGS_EE_GFX__ALI3DEXCEPTION_H

namespace AGS
{
namespace Engine
{

class Ali3DException
{
public:
    Ali3DException(const char *message)
    {
        _message = message;
    }

    const char *_message;
};

class Ali3DFullscreenLostException : public Ali3DException
{
public:
    Ali3DFullscreenLostException() : Ali3DException("User has switched away from application")
    {
    }
};

} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GFX__ALI3DEXCEPTION_H
