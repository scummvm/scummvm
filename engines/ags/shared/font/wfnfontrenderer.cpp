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

#include "ac/common.h" // our_eip
#include "core/assetmanager.h"
#include "debug/out.h"
#include "font/wfnfont.h"
#include "font/wfnfontrenderer.h"
#include "gfx/bitmap.h"
#include "util/stream.h"

using namespace AGS::Common;

static unsigned char GetCharCode(unsigned char wanted_code, const WFNFont* font)
{
    return wanted_code < font->GetCharCount() ? wanted_code : '?';
}

static int RenderChar(Bitmap *ds, const int at_x, const int at_y, const WFNChar &wfn_char, const int scale, const color_t text_color);

void WFNFontRenderer::AdjustYCoordinateForFont(int *ycoord, int fontNumber)
{
  // Do nothing
}

void WFNFontRenderer::EnsureTextValidForFont(char *text, int fontNumber)
{
  const WFNFont* font = _fontData[fontNumber].Font;
  // replace any extended characters with question marks
  for (; *text; ++text)
  {
    if ((unsigned char)*text >= font->GetCharCount()) 
    {
      *text = '?';
    }
  }
}

int WFNFontRenderer::GetTextWidth(const char *text, int fontNumber)
{
  const WFNFont* font = _fontData[fontNumber].Font;
  const FontRenderParams &params = _fontData[fontNumber].Params;
  int text_width = 0;

  for (; *text; ++text)
  {
    const WFNChar &wfn_char = font->GetChar(GetCharCode(*text, font));
    text_width += wfn_char.Width;
  }
  return text_width * params.SizeMultiplier;
}

int WFNFontRenderer::GetTextHeight(const char *text, int fontNumber)
{
  const WFNFont* font = _fontData[fontNumber].Font;
  const FontRenderParams &params = _fontData[fontNumber].Params;
  int max_height = 0;

  for (; *text; ++text) 
  {
    const WFNChar &wfn_char = font->GetChar(GetCharCode(*text, font));
    const uint16_t height = wfn_char.Height;
    if (height > max_height)
      max_height = height;
  }
  return max_height * params.SizeMultiplier;
}

Bitmap render_wrapper;
void WFNFontRenderer::RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour)
{
  int oldeip = get_our_eip();
  set_our_eip(415);

  const WFNFont* font = _fontData[fontNumber].Font;
  const FontRenderParams &params = _fontData[fontNumber].Params;
  render_wrapper.WrapAllegroBitmap(destination, true);

  for (; *text; ++text)
    x += RenderChar(&render_wrapper, x, y, font->GetChar(GetCharCode(*text, font)), params.SizeMultiplier, colour);

  set_our_eip(oldeip);
}

int RenderChar(Bitmap *ds, const int at_x, const int at_y, const WFNChar &wfn_char, const int scale, const color_t text_color)
{
  const int width = wfn_char.Width;
  const int height = wfn_char.Height;
  const unsigned char *actdata = wfn_char.Data;
  const int bytewid = wfn_char.GetRowByteCount();

  int x = at_x;
  int y = at_y;
  for (int h = 0; h < height; ++h)
  {
    for (int w = 0; w < width; ++w)
    {
      if (((actdata[h * bytewid + (w / 8)] & (0x80 >> (w % 8))) != 0)) {
        if (scale > 1)
        {
          ds->FillRect(Rect(x + w, y + h, x + w + (scale - 1),
              y + h + (scale - 1)), text_color);
        } 
        else
        {
          ds->PutPixel(x + w, y + h, text_color);
        }
      }

      x += scale - 1;
    }
    y += scale - 1;
    x = at_x;
  }
  return width * scale;
}

bool WFNFontRenderer::LoadFromDisk(int fontNumber, int fontSize)
{
  return LoadFromDiskEx(fontNumber, fontSize, nullptr);
}

bool WFNFontRenderer::IsBitmapFont()
{
    return true;
}

bool WFNFontRenderer::LoadFromDiskEx(int fontNumber, int fontSize, const FontRenderParams *params)
{
  String file_name;
  Stream *ffi = nullptr;

  file_name.Format("agsfnt%d.wfn", fontNumber);
  ffi = AssetManager::OpenAsset(file_name);
  if (ffi == nullptr)
  {
    // actual font not found, try font 0 instead
    file_name = "agsfnt0.wfn";
    ffi = AssetManager::OpenAsset(file_name);
    if (ffi == nullptr)
      return false;
  }

  WFNFont *font = new WFNFont();
  WFNError err = font->ReadFromFile(ffi, AssetManager::GetLastAssetSize());
  delete ffi;
  if (err == kWFNErr_HasBadCharacters)
    Debug::Printf(kDbgMsg_Warn, "WARNING: font '%s' has mistakes in data format, some characters may be displayed incorrectly", file_name.GetCStr());
  else if (err != kWFNErr_NoError)
  {
    delete font;
    return false;
  }
  _fontData[fontNumber].Font = font;
  _fontData[fontNumber].Params = params ? *params : FontRenderParams();
  return true;
}

void WFNFontRenderer::FreeMemory(int fontNumber)
{
  delete _fontData[fontNumber].Font;
  _fontData.erase(fontNumber);
}

bool WFNFontRenderer::SupportsExtendedCharacters(int fontNumber)
{
  return _fontData[fontNumber].Font->GetCharCount() > 128;
}
