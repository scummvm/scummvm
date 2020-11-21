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
