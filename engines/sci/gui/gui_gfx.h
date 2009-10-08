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

#ifndef SCI_GUI_GFX_H
#define SCI_GUI_GFX_H

#include "sci/gui/gui.h"

namespace Sci {

#define SCI_PATTERN_CODE_RECTANGLE 0x10
#define SCI_PATTERN_CODE_USE_TEXTURE 0x20
#define SCI_PATTERN_CODE_PENSIZE 0x07

class SciGuiScreen;
class SciGuiPalette;
class SciGuiFont;
class SciGuiPicture;
class SciGuiView;
class SciGuiGfx {
public:
	SciGuiGfx(EngineState *state, SciGuiScreen *screen, SciGuiPalette *palette);
	~SciGuiGfx();

	void init(void);

	byte *GetSegment(byte seg);
	void ResetScreen();

	GuiPort *SetPort(GuiPort *port);
	GuiPort *GetPort();
	void SetOrigin(int16 left, int16 top);
	void MoveTo(int16 left, int16 top);
	void Move(int16 left, int16 top);
	void OpenPort(GuiPort *port);
	void PenColor(int16 color);
	void PenMode(int16 mode);
	void TextFace(int16 textFace);
	int16 GetPointSize(void);
	GuiResourceId GetFontId();
	SciGuiFont *GetFont();
	void SetFont(GuiResourceId fontId);

	void ClearScreen(byte color = 255);
	void InvertRect(const Common::Rect &rect);
	void EraseRect(const Common::Rect &rect);
	void PaintRect(const Common::Rect &rect);
	void FillRect(const Common::Rect &rect, int16 drawFlags, byte clrPen, byte clrBack = 0, byte bControl = 0);
	void FrameRect(const Common::Rect &rect);
	void OffsetRect(Common::Rect &r);

	byte CharHeight(int16 ch);
	byte CharWidth(int16 ch);
	void ClearChar(int16 chr);
	void DrawChar(int16 chr);
	void StdChar(int16 chr);

	void SetTextFonts(int argc, reg_t *argv);
	void SetTextColors(int argc, reg_t *argv);
	int16 TextSize(Common::Rect &rect, const char *str, GuiResourceId fontId, int16 maxwidth);
	void ShowString(const char *str, GuiResourceId orgFontId, int16 orgPenColor) {
		ShowText(str, 0, (int16)strlen(str), orgFontId, orgPenColor);
	}
	void DrawString(const char *str, GuiResourceId orgFontId, int16 orgPenColor) {
		DrawText(str, 0, (int16)strlen(str), orgFontId, orgPenColor);
	}
	void TextBox(const char *str, int16 bshow, const Common::Rect &rect, int16 align, GuiResourceId fontId);
	void ShowBits(const Common::Rect &r, uint16 flags);
	GuiMemoryHandle SaveBits(const Common::Rect &rect, byte screenFlags);
	void RestoreBits(GuiMemoryHandle memoryHandle);

	void drawLine(int16 left, int16 top, int16 right, int16 bottom, byte color, byte prio, byte control);
	void Draw_Box(Common::Rect box, byte color, byte prio, byte control);
	void Draw_TexturedBox(Common::Rect box, byte color, byte prio, byte control, byte texture);
	void Draw_Circle(Common::Rect box, byte size, byte color, byte prio, byte control);
	void Draw_TexturedCircle(Common::Rect box, byte size, byte color, byte prio, byte control, byte texture);
	void Draw_Pattern(int16 x, int16 y, byte pic_color, byte pic_priority, byte pic_control, byte code, byte texture);
	void Draw_String(const char *text);
	void Pic_Fill(int16 x, int16 y, byte color, byte prio, byte control);
	
	void drawPicture(GuiResourceId pictureId, int16 animationNr, bool mirroredFlag, bool addToFlag, GuiResourceId paletteId);
	void drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, uint16 leftPos, uint16 topPos, byte priority, uint16 paletteNo);

	int16 onControl(uint16 screenMask, Common::Rect rect);
	void AnimateDisposeLastCast();
	void AnimateInvoke(List *list, int argc, reg_t *argv);
	void AnimateFill(List *list, byte &oldPicNotValid);
	Common::List<GuiAnimateList> *AnimateMakeSortedList(List *list);
	void AnimateUpdate();
	void AnimateDrawCels(List *list);
	void AnimateRestoreAndDelete();
	void SetNowSeen(reg_t objectReference);

	GuiPort *_menuPort;
	Common::Rect _menuRect;

private:
	int16 TextCodeProcessing(const char *&text, GuiResourceId orgFontId, int16 orgPenColor);
	void TextWidth(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight);
	void StringWidth(const char *str, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight);
	int16 GetLongest(const char *str, int16 maxwidth, GuiResourceId orgFontId);
	void DrawText(const char *str, int16 from, int16 len, GuiResourceId orgFontId, int16 orgPenColor);
	void ShowText(const char *str, int16 from, int16 len, GuiResourceId orgFontId, int16 orgPenColor);

	EngineState *_s;
	SciGuiScreen *_screen;
	SciGuiPalette *_palette;

	Common::Rect _bounds;
	GuiPort *_mainPort;
	GuiPort *_curPort;
	uint16 _clrPowers[256];

	byte bMapColors;
	GuiPalette *pPicPal;
	Common::Array<GuiPalSchedule> _palSchedules;

	int _textFontsCount;
	GuiResourceId *_textFonts;
	int _textColorsCount;
	uint16 *_textColors;

	// Animate* related variables
	List *_lastCast;

	SciGuiFont *_font;
};

} // End of namespace Sci

#endif
