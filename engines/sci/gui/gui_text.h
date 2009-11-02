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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_GUI_TEXT_H
#define SCI_GUI_TEXT_H

namespace Sci {

#define SCI_TEXT_ALIGNMENT_RIGHT -1
#define SCI_TEXT_ALIGNMENT_CENTER 1
#define SCI_TEXT_ALIGNMENT_LEFT	0

class SciGuiGfx;
class SciGuiScreen;
class SciGuiFont;
class SciGuiText {
public:
	SciGuiText(ResourceManager *_resMan, SciGuiGfx *gfx, SciGuiScreen *screen);
	~SciGuiText();

	GuiResourceId GetFontId();
	SciGuiFont *GetFont();
	void SetFont(GuiResourceId fontId);

	void CodeSetFonts(int argc, reg_t *argv);
	void CodeSetColors(int argc, reg_t *argv);
	int16 CodeProcessing(const char *&text, GuiResourceId orgFontId, int16 orgPenColor);

	void ClearChar(int16 chr);
	void DrawChar(int16 chr);
	void StdChar(int16 chr);

	int16 GetLongest(const char *text, int16 maxWidth, GuiResourceId orgFontId);
	void Width(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight);
	void StringWidth(const char *str, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight);
	void ShowString(const char *str, GuiResourceId orgFontId, int16 orgPenColor);
	void DrawString(const char *str, GuiResourceId orgFontId, int16 orgPenColor);
	int16 Size(Common::Rect &rect, const char *str, GuiResourceId fontId, int16 maxWidth);
	void Draw(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 orgPenColor);
	void Show(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 orgPenColor);
	void Box(const char *text, int16 bshow, const Common::Rect &rect, GuiTextAlignment alignment, GuiResourceId fontId);
	void Draw_String(const char *text);

	SciGuiFont *_font;

private:
	void init();

	ResourceManager *_resMan;
	SciGuiGfx *_gfx;
	SciGuiScreen *_screen;

	int _codeFontsCount;
	GuiResourceId *_codeFonts;
	int _codeColorsCount;
	uint16 *_codeColors;
};

} // End of namespace Sci

#endif
