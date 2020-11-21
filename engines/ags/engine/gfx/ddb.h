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
// Driver-dependant bitmap interface
//
//=============================================================================
#ifndef __AGS_EE_GFX__DDB_H
#define __AGS_EE_GFX__DDB_H

namespace AGS
{
namespace Engine
{

class IDriverDependantBitmap
{
public:
  virtual ~IDriverDependantBitmap() = default;

  virtual void SetTransparency(int transparency) = 0;  // 0-255
  virtual void SetFlippedLeftRight(bool isFlipped) = 0;
  virtual void SetStretch(int width, int height, bool useResampler = true) = 0;
  virtual void SetLightLevel(int light_level) = 0;   // 0-255
  virtual void SetTint(int red, int green, int blue, int tintSaturation) = 0;  // 0-255

  virtual int GetWidth() = 0;
  virtual int GetHeight() = 0;
  virtual int GetColorDepth() = 0;
};

} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GFX__DDB_H
