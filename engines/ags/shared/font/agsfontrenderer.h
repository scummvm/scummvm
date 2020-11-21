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

#ifndef __AC_AGSFONTRENDERER_H
#define __AC_AGSFONTRENDERER_H

struct BITMAP;

// WARNING: this interface is exposed for plugins and declared for the second time in agsplugin.h
class IAGSFontRenderer
{
public:
  virtual bool LoadFromDisk(int fontNumber, int fontSize) = 0;
  virtual void FreeMemory(int fontNumber) = 0;
  virtual bool SupportsExtendedCharacters(int fontNumber) = 0;
  virtual int GetTextWidth(const char *text, int fontNumber) = 0;
  // Get actual height of the given line of text
  virtual int GetTextHeight(const char *text, int fontNumber) = 0;
  virtual void RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) = 0;
  virtual void AdjustYCoordinateForFont(int *ycoord, int fontNumber) = 0;
  virtual void EnsureTextValidForFont(char *text, int fontNumber) = 0;
protected:
  IAGSFontRenderer() = default;
  ~IAGSFontRenderer() = default;
};

// Font render params, mainly for dealing with various compatibility issues and
// broken fonts. NOTE: currently left empty as a result of rewrite, but may be
// used again in the future.
struct FontRenderParams
{
    // Font's render multiplier
    int SizeMultiplier = 1;
};

// NOTE: this extending interface is not yet exposed to plugins
class IAGSFontRenderer2
{
public:
  virtual bool IsBitmapFont() = 0;
  // Load font, applying extended font rendering parameters
  virtual bool LoadFromDiskEx(int fontNumber, int fontSize, const FontRenderParams *params) = 0;
protected:
  IAGSFontRenderer2() = default;
  ~IAGSFontRenderer2() = default;
};

#endif // __AC_AGSFONTRENDERER_H
