/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/************************************************************************/
/************************************************************************/
/*																		*/
/*						System independent routines						*/
/*																		*/
/*																		*/
/************************************************************************/
/************************************************************************/

#include "ultima/ultima0/akalabeth.h"						/* Our prototypes */
#include "ultima/ultima0/sdw.h"

namespace Ultima {
namespace Ultima0 {

using namespace SDLWrapper;

int CharHeight = 16;						/* Height of one character */
int CharWidth = 16;							/* Width of one character */
int TextLines = 22;							/* Number of chars in visible line */
int TextLeft, xc, yc;
int TextOrigin;
int Red, Green, Blue;
Surface *MainSurface;

/************************************************************************/
/*																		*/
/*				Initialise all internal graphics routines				*/
/*																		*/
/************************************************************************/

void HWInitialise(void) {
	MainSurface = new Surface(DEFAULT_SCX, DEFAULT_SCY);
	TextLeft = DEFAULT_SCX - CharWidth * TextLines;
	TextOrigin = 224;
	xc = TextLeft; yc = TextOrigin;
}

/************************************************************************/
/*																		*/
/*							Refresh Status Area							*/
/*																		*/
/************************************************************************/

void HWStatus(double Food, int HP, int Gold) {
	char Temp[32];
	MainSurface->SetColour(0, 0, 0);
	MainSurface->FillRect(TextLeft, 0, DEFAULT_SCX, TextOrigin - 1);
	MainSurface->SetColour(0, 255, 255);
	Common::sprintf_s(Temp, "%.1f", Food);
	MainSurface->String(TextLeft + 16, 32, TextLeft + 16 + 32 * strlen(Temp), 32 + 32, Temp);
	Common::sprintf_s(Temp, "%u", HP);
	MainSurface->String(TextLeft + 16, 96, TextLeft + 16 + 32 * strlen(Temp), 96 + 32, Temp);
	Common::sprintf_s(Temp, "%u", Gold);
	MainSurface->String(TextLeft + 16, 160, TextLeft + 16 + 32 * strlen(Temp), 160 + 32, Temp);

	MainSurface->SetColour(255, 0, 0);
	MainSurface->String(TextLeft, 8, TextLeft + 64, 24, "Food");
	MainSurface->String(TextLeft, 72, TextLeft + 16 * 9, 88, "Hit Points");
	MainSurface->String(TextLeft, 136, TextLeft + 64, 152, "Gold");

	// /*	char Temp[32];
	// 	setfillstyle(SOLID_FILL,BLACK);    		/* Set up colours */
	// 	setcolor(LIGHTCYAN);
	// 	settextstyle(SMALL_FONT,HORIZ_DIR,6);
	// 	bar(xText+110,22,xMax-4,yScroll-4);		/* Erase old status values */
	// 	Common::sprintf_s(Temp,"%.1f",Food);				/* Fill in the new ones */
	// 	outtextxy(xText+110,22,Temp);
	// 	Common::sprintf_s(Temp,"%u",HP);
	// 	outtextxy(xText+110,37,Temp);
	// 	Common::sprintf_s(Temp,"%u",Gold);
	// 	outtextxy(xText+110,52,Temp);
	// 	HWColour(CurrentColour);				/* Set the current colour */*/
}

/************************************************************************/
/*																		*/
/*				Terminate all internal graphics routines				*/
/*																		*/
/************************************************************************/

void HWTerminate(void) {
}

/************************************************************************/
/*																		*/
/*				Output text to the scrolling text area					*/
/*																		*/
/************************************************************************/

void HWChar(int ch) {
	Rect src(TextLeft, TextOrigin + CharHeight, DEFAULT_SCX, DEFAULT_SCY);
	if (ch >= ' ')
	{
		MainSurface->SetColour(0, 255, 0);
		MainSurface->Char(xc, yc, xc + CharWidth - 1, yc + CharHeight - 1, ch);
		xc = xc + CharWidth;
	}
	if (xc >= DEFAULT_SCX || ch == 13)
	{
		xc = TextLeft;
		yc = yc + CharHeight;
		if (yc + CharHeight >= DEFAULT_SCY)
		{
			MainSurface->Copy(*MainSurface, src, TextLeft, TextOrigin);
			MainSurface->SetColour(0, 0, 0);
			MainSurface->FillRect(TextLeft, yc, DEFAULT_SCX, DEFAULT_SCY);
			yc = yc - CharHeight;
		}
	}
}

/************************************************************************/
/*																		*/
/*			Select the current drawing colour, should use RGB()			*/
/*																		*/
/************************************************************************/

void HWColour(int c) {
	Red = (c & 4) ? 255 : 0;
	Green = (c & 2) ? 255 : 0;
	Blue = (c & 1) ? 255 : 0;
}

/************************************************************************/
/*																		*/
/*						Draw line in current colour						*/
/*																		*/
/************************************************************************/

void HWLine(int x1, int y1, int x2, int y2) {
	x1 = x1 * TextLeft / 1280; y1 = DEFAULT_SCY - y1 * DEFAULT_SCY / 1024;
	x2 = x2 * TextLeft / 1280; y2 = DEFAULT_SCY - y2 * DEFAULT_SCY / 1024;
	MainSurface->SetColour(Red, Green, Blue);
	MainSurface->Line(x1, y1, x2, y2);
}

/***********************************************************************/
/*																		*/
/*			Read an upper case ASCII key, or Carriage Return (13)		*/
/*																		*/
/************************************************************************/

int HWGetKey(void) {
	int n;
	MainSurface->Copy();
	MainSurface->Flip();
	do
	{
		n = GetKey();
		if (n == 273) n = 'N';
		if (n == 274) n = 'S';
		if (n == 275) n = 'E';
		if (n == 276) n = 'W';

	} while (n < ' ' || n > 127);
	return toupper(n);
}


/************************************************************************/
/*																		*/
/*						Clear the graphics area							*/
/*																		*/
/************************************************************************/

void HWClear(void) {
	MainSurface->SetColour(0, 0, 0);
	MainSurface->FillRect(0, 0, TextLeft - 1, DEFAULT_SCY);
}

} // namespace Ultima0
} // namespace Ultima
