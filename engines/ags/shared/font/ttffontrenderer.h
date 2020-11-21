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

#ifndef __AC_TTFFONTRENDERER_H
#define __AC_TTFFONTRENDERER_H

#include <map>
#include "font/agsfontrenderer.h"

struct ALFONT_FONT;

class TTFFontRenderer : public IAGSFontRenderer, public IAGSFontRenderer2 {
public:
  // IAGSFontRenderer implementation
  bool LoadFromDisk(int fontNumber, int fontSize) override;
  void FreeMemory(int fontNumber) override;
  bool SupportsExtendedCharacters(int fontNumber) override { return true; }
  int GetTextWidth(const char *text, int fontNumber) override;
  int GetTextHeight(const char *text, int fontNumber) override;
  void RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) override ;
  void AdjustYCoordinateForFont(int *ycoord, int fontNumber) override;
  void EnsureTextValidForFont(char *text, int fontNumber) override;

  // IAGSFontRenderer2 implementation
  bool IsBitmapFont() override;
  bool LoadFromDiskEx(int fontNumber, int fontSize, const FontRenderParams *params) override;

private:
    struct FontData
    {
        ALFONT_FONT     *AlFont;
        FontRenderParams Params;
    };
    std::map<int, FontData> _fontData;
};

#endif // __AC_TTFFONTRENDERER_H
