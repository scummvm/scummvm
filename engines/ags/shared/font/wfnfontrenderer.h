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

#ifndef __AC_WFNFONTRENDERER_H
#define __AC_WFNFONTRENDERER_H

#include <map>
#include "font/agsfontrenderer.h"

class WFNFont;

class WFNFontRenderer : public IAGSFontRenderer, public IAGSFontRenderer2 {
public:
  bool LoadFromDisk(int fontNumber, int fontSize) override;
  void FreeMemory(int fontNumber) override;
  bool SupportsExtendedCharacters(int fontNumber) override;
  int GetTextWidth(const char *text, int fontNumber) override;
  int GetTextHeight(const char *text, int fontNumber) override;
  void RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) override;
  void AdjustYCoordinateForFont(int *ycoord, int fontNumber) override;
  void EnsureTextValidForFont(char *text, int fontNumber) override;

  bool IsBitmapFont() override;
  bool LoadFromDiskEx(int fontNumber, int fontSize, const FontRenderParams *params) override;

private:
  struct FontData
  {
    WFNFont         *Font;
    FontRenderParams Params;
  };
  std::map<int, FontData> _fontData;
};

#endif // __AC_WFNFONTRENDERER_H
