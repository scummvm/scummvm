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

namespace Sci {

#define SCI_PATTERN_CODE_RECTANGLE 0x10
#define SCI_PATTERN_CODE_USE_TEXTURE 0x20
#define SCI_PATTERN_CODE_PENSIZE 0x07

class SciGUIscreen;
class SciGUIfont;
class SciGUIpicture;
class SciGUIview;
class SciGUIgfx {
public:
	SciGUIgfx(OSystem *system, EngineState *state, SciGUIscreen *screen);
	~SciGUIgfx();

	void init(void);
	void initPalette();
	void initTimer();
	static void timerHandler(void*ref);

	sciPort *mallocPort ();
	byte *GetSegment(byte seg);
	void ResetScreen();
	void SetEGApalette();
	void CreatePaletteFromData(byte *paletteData, sciPalette *paletteOut);
	bool SetResPalette(int16 resourceNo, int16 flag);
	void SetPalette(sciPalette *sciPal, int16 flag);
	void MergePalettes(sciPalette*pFrom, sciPalette*pTo, uint16 flag);
	uint16 MatchColor(sciPalette*pPal, byte r, byte g, byte b);
	void SetCLUT(sciPalette*pal);
	void GetCLUT(sciPalette*pal);

	sciPort *SetPort(sciPort *port);
	sciPort *GetPort();
	void SetOrigin(int16 left, int16 top);
	void MoveTo(int16 left, int16 top);
	void Move(int16 left, int16 top);
	void SetFont(int16 fontId);
	void OpenPort(sciPort *port);
	void PenColor(int16 color);
	void PenMode(int16 mode);
	void TextFace(int16 textFace);
	int16 GetPointSize(void);
	int16 GetFontId();
	SciGUIfont *GetFont();

	void ClearScreen(byte color = 255);
	void InvertRect(const Common::Rect &rect);
	void EraseRect(const Common::Rect &rect);
	void PaintRect(const Common::Rect &rect);
	void FillRect(const Common::Rect &rect, int16 arg2, byte clrPen, byte clrBack = 0, byte bControl = 0);
	void FrameRect(const Common::Rect &rect);
	void OffsetRect(Common::Rect &r);

	byte CharHeight(int16 ch);
	byte CharWidth(int16 ch);
	void ClearChar(int16 chr);
	void DrawChar(int16 chr);
	void StdChar(int16 chr);

	void SetTextFonts(int argc, reg_t *argv);
	void SetTextColors(int argc, reg_t *argv);
	int16 TextWidth(const char*text, int16 from, int16 len);
	int16 StringWidth(const char*str) {
		return TextWidth(str, 0, (int16)strlen(str));
	}
	int16 TextSize(Common::Rect &rect, const char *str, int16 fontId, int16 maxwidth);
	int16 GetLongest(const char *str, int16 maxwidth);
	void DrawText(const char *str, int16 from, int16 len);
	void ShowText(const char *str, int16 from, int16 len);
	void ShowString(const char *str) {
		ShowText(str, 0, (int16)strlen(str));
	}
	void DrawString(const char *str) {
		DrawText(str, 0, (int16)strlen(str));
	}
	void TextBox(const char *str, int16 bshow, const Common::Rect &rect, int16 align, int16 fontId);
	void ShowBits(const Common::Rect &r, uint16 flags);
	sciMemoryHandle SaveBits(const Common::Rect &rect, byte screenFlags);
	void RestoreBits(sciMemoryHandle memoryHandle);

	void Draw_Line(int16 left, int16 top, int16 right, int16 bottom, byte color, byte prio, byte control);
	void Draw_Horiz(int16 left, int16 right, int16 top, byte flag, byte color, byte prio, byte control);
	void Draw_Vert(int16 top, int16 bottom, int16 left, byte flag, byte color, byte prio, byte control);
	void Draw_Box(Common::Rect box, byte color, byte prio, byte control);
	void Draw_TexturedBox(Common::Rect box, byte color, byte prio, byte control, byte texture);
	void Draw_Circle(Common::Rect box, byte size, byte color, byte prio, byte control);
	void Draw_TexturedCircle(Common::Rect box, byte size, byte color, byte prio, byte control, byte texture);
	void Draw_Pattern(int16 x, int16 y, byte pic_color, byte pic_priority, byte pic_control, byte code, byte texture);
	void Pic_Fill(int16 x, int16 y, byte color, byte prio, byte control);
	
	void drawPicture(sciResourceId pictureId, uint16 style, bool addToFlag, sciResourceId paletteId);
	void drawCell(sciResourceId viewId, uint16 loopNo, uint16 cellNo, uint16 leftPos, uint16 topPos, byte priority, uint16 paletteNo);

	void animatePalette(byte fromColor, byte toColor, int speed);

	sciPort *_menuPort;
	uint32 _sysTicks;
	int32 _sysSpeed; // ticker timer in ms 
	sciPalette _sysPalette;

	uint16 _resolutionWidth;
	uint16 _resolutionHeight;
	uint _resolutionPixels;

private:
	OSystem *_system;
	EngineState *_s;
	SciGUIscreen *_screen;

	Common::Rect _bounds;
	sciPort *_mainPort;
	sciPort *_curPort;
	uint16 _clrPowers[256];

	byte bMapColors;
	sciPalette *pPicPal;
	Common::Array<sciPalSched> _palSchedules;

	int _textFontsCount;
	sciResourceId *_textFonts;
	int _textColorsCount;
	uint16 *_textColors;

	SciGUIfont *_font;
};

} // end of namespace Sci
