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
/*							Draw Monster Graphics						*/
/*																		*/
/************************************************************************/
/************************************************************************/

#include "ultima/ultima0/akalabeth.h"

namespace Ultima {
namespace Ultima0 {

#define PARAMS  double x,double y,double d	/* Parameters for Draw funcs */
#define	END		(-9999.99)					/* End marker */

static void _DRAWSkeleton(PARAMS);			/* Local Prototypes */
static void _DRAWThief(PARAMS);
static void _DRAWRat(PARAMS);
static void _DRAWOrc(PARAMS);
static void _DRAWViper(PARAMS);
static void _DRAWCarrion(PARAMS);
static void _DRAWGremlin(PARAMS);
static void _DRAWMimic(PARAMS);
static void _DRAWDaemon(PARAMS);
static void _DRAWBalrog(PARAMS);

int		xPos = 640;							/* Drawing position */
int 	yPos = 512;

/************************************************************************/
/*																		*/
/*							Draw Monster Graphics						*/
/*																		*/
/************************************************************************/

void DRAWMonster(int x, int y, int Monster, double Scale) {
	xPos = x; yPos = y;						/* Save drawing pos */
	if (Monster == MN_MIMIC)				/* Fix for Mimic/Chest */
		xPos = xPos - 90;
	switch (Monster)							/* Call appropriate function */
	{
	case MN_SKELETON:   _DRAWSkeleton(0, 0, Scale); break;
	case MN_THIEF:      _DRAWThief(0, 0, Scale); break;
	case MN_RAT:        _DRAWRat(0, 0, Scale); break;
	case MN_ORC:        _DRAWOrc(0, 0, Scale); break;
	case MN_VIPER:      _DRAWViper(0, 0, Scale); break;
	case MN_CARRION:    _DRAWCarrion(0, 0, Scale); break;
	case MN_GREMLIN:    _DRAWGremlin(0, 0, Scale); break;
	case MN_MIMIC:      _DRAWMimic(0, 0, Scale); break;
	case MN_DAEMON:     _DRAWDaemon(0, 0, Scale); break;
	case MN_BALROG:     _DRAWBalrog(0, 0, Scale); break;
	}
}

/************************************************************************/
/*																		*/
/*		   Draw a text string. Here because use of ... like HPLOT		*/
/*																		*/
/************************************************************************/

void DRAWText(const char *Format, ...) {
	va_list alist;
	char Buffer[512], *p;
	va_start(alist, Format);
	Common::vsprintf_s(Buffer, Format, alist);
	p = Buffer;
	while (*p != '\0')
	{
		if (*p == '\n') HWChar(13), p++;
		else HWChar(*p++);
	}
	va_end(alist);
}

/************************************************************************/
/*																		*/
/*				Emulate the Apple ][ HPLOT function						*/
/*																		*/
/************************************************************************/

static void _HPlot(double x, double y, ...) {
	va_list alist;
	double y1, x1;
	va_start(alist, y);						/* Start reading values */
	do
	{
		x1 = va_arg(alist, double);			/* Get the next two */
		y1 = va_arg(alist, double);
		if (x1 != END && y1 != END)			/* If legit, draw the line */
			HWLine(xPos + x, yPos - y, xPos + x1, yPos - y1);
		x = x1; y = y1;
	} while (x1 != END && y1 != END);
	va_end(alist);
}

/************************************************************************/
/*																		*/
/*			Drawing functions, grabbed from the Apple II original		*/
/*																		*/
/************************************************************************/

static void _DRAWSkeleton(PARAMS) {
	_HPlot(y - 23 / d, x, y - 15 / d, x, y - 15 / d, x - 15 / d, y - 8 / d, x - 30 / d, y + 8 / d, x - 30 / d, y + 15 / d, x - 15 / d, y + 15 / d, x, y + 23 / d, x, END, END);
	_HPlot(y, x - 26 / d, y, x - 65 / d, END, END);
	_HPlot(y - 2 / d + .5, x - 38 / d, y + 2 / d + .5, x - 38 / d, END, END);
	_HPlot(y - 3 / d + .5, x - 45 / d, y + 3 / d + .5, x - 45 / d, END, END);
	_HPlot(y - 5 / d + .5, x - 53 / d, y + 5 / d + .5, x - 53 / d, END, END);
	_HPlot(y - 23 / d, x - 56 / d, y - 30 / d, x - 53 / d, y - 23 / d, x - 45 / d, y - 23 / d, x - 53 / d, y - 8 / d, x - 38 / d, END, END);
	_HPlot(y - 15 / d, x - 45 / d, y - 8 / d, x - 60 / d, y + 8 / d, x - 60 / d, y + 15 / d, x - 45 / d, END, END);
	_HPlot(y + 15 / d, x - 42 / d, y + 15 / d, x - 57 / d, END, END);
	_HPlot(y + 12 / d, x - 45 / d, y + 20 / d, x - 45 / d, END, END);
	_HPlot(y, x - 75 / d, y - 5 / d + .5, x - 80 / d, y - 8 / d, x - 75 / d, y - 5 / d + .5, x - 65 / d, y + 5 / d + .5, x - 65 / d, y + 5 / d + .5, x - 68 / d, y - 5 / d + .5, x - 68 / d, y - 5 / d + .5, x - 65 / d, END, END);
	_HPlot(y + 5 / d + .5, x - 65 / d, y + 8 / d, x - 75 / d, y + 5 / d + .5, x - 80 / d, y - 5 / d + .5, x - 80 / d, END, END);
	_HPlot(y - 5 / d + .5, x - 72 / d, END, END);
	_HPlot(y + 5 / d + .5, x - 72 / d, END, END);
}

static void _DRAWThief(PARAMS) {
	_HPlot(y, x - 56 / d, y, x - 8 / d, y + 10 / d, x, y + 30 / d, x, y + 30 / d, x - 45 / d, y + 10 / d, x - 64 / d, y, x - 56 / d, END, END);
	_HPlot(y - 10 / d, x - 64 / d, y - 30 / d, x - 45 / d, y - 30 / d, x, y - 10 / d, x, y, x - 8 / d, END, END);
	_HPlot(y - 10 / d, x - 64 / d, y - 10 / d, x - 75 / d, y, x - 83 / d, y + 10 / d, x - 75 / d, y, x - 79 / d, y - 10 / d, x - 75 / d, y, x - 60 / d, y + 10 / d, x - 75 / d, y + 10 / d, x - 64 / d, END, END);
}

static void _DRAWRat(PARAMS) {
	_HPlot(y + 5 / d, x - 30 / d, y, x - 25 / d, y - 5 / d, x - 30 / d, y - 15 / d, x - 5 / d, y - 10 / d, x, y + 10 / d, x, y + 15 / d, x - 5 / d, END, END);
	_HPlot(y + 20 / d, x - 5 / d, y + 10 / d, x, y + 15 / d, x - 5 / d, y + 5 / d, x - 30 / d, y + 10 / d, x - 40 / d, y + 3 / d + .5, x - 35 / d, y - 3 / d + .5, x - 35 / d, y - 10 / d, x - 40 / d, y - 5 / d, x - 30 / d, END, END);
	_HPlot(y - 5 / d, x - 33 / d, y - 3 / d + .5, x - 30 / d, END, END);
	_HPlot(y + 5 / d, x - 33 / d, y + 3 / d + .5, x - 30 / d, END, END);
	_HPlot(y - 5 / d, x - 20 / d, y - 5 / d, x - 15 / d, END, END);
	_HPlot(y + 5 / d, x - 20 / d, y + 5 / d, x - 15 / d, END, END);
	_HPlot(y - 7 / d, x - 20 / d, y - 7 / d, x - 15 / d, END, END);
	_HPlot(y + 7 / d, x - 20 / d, y + 7 / d, x - 15 / d, END, END);
}

static void _DRAWOrc(PARAMS) {
	_HPlot(y, x, y - 15 / d, x, y - 8 / d, x - 8 / d, y - 8 / d, x - 15 / d, y - 15 / d, x - 23 / d, y - 15 / d, x - 15 / d, y - 23 / d, x - 23 / d, END, END);
	_HPlot(y - 23 / d, x - 45 / d, y - 15 / d, x - 53 / d, y - 8 / d, x - 53 / d, y - 15 / d, x - 68 / d, y - 8 / d, x - 75 / d, y, x - 75 / d, END, END);
	_HPlot(y, x, y + 15 / d, x, y + 8 / d, x - 8 / d, y + 8 / d, x - 15 / d, y + 15 / d, x - 23 / d, y + 15 / d, x - 15 / d, y + 23 / d, x - 23 / d, END, END);
	_HPlot(y + 23 / d, x - 45 / d, y + 15 / d, x - 53 / d, y + 8 / d, x - 53 / d, y + 15 / d, x - 68 / d, y + 8 / d, x - 75 / d, y, x - 75 / d, END, END);
	_HPlot(y - 15 / d, x - 68 / d, y + 15 / d, x - 68 / d, END, END);
	_HPlot(y - 8 / d, x - 53 / d, y + 8 / d, x - 53 / d, END, END);
	_HPlot(y - 23 / d, x - 15 / d, y + 8 / d, x - 45 / d, END, END);
	_HPlot(y - 8 / d, x - 68 / d, y, x - 60 / d, y + 8 / d, x - 68 / d, y + 8 / d, x - 60 / d, y - 8 / d, x - 60 / d, y - 8 / d, x - 68 / d, END, END);
	_HPlot(y, x - 38 / d, y - 8 / d, x - 38 / d, y + 8 / d, x - 53 / d, y + 8 / d, x - 45 / d, y + 15 / d, x - 45 / d, y, x - 30 / d, y, x - 38 / d, END, END);
}

static void _DRAWViper(PARAMS) {
	_HPlot(y - 10 / d, x - 15 / d, y - 10 / d, x - 30 / d, y - 15 / d, x - 20 / d, y - 15 / d, x - 15 / d, y - 15 / d, x, y + 15 / d, x, y + 15 / d, x - 15 / d, y - 15 / d, x - 15 / d, END, END);
	_HPlot(y - 15 / d, x - 10 / d, y + 15 / d, x - 10 / d, END, END);
	_HPlot(y - 15 / d, x - 5 / d, y + 15 / d, x - 5 / d, END, END);
	_HPlot(y, x - 15 / d, y - 5 / d, x - 20 / d, y - 5 / d, x - 35 / d, y + 5 / d, x - 35 / d, y + 5 / d, x - 20 / d, y + 10 / d, x - 15 / d, END, END);
	_HPlot(y - 5 / d, x - 20 / d, y + 5 / d, x - 20 / d, END, END);
	_HPlot(y - 5 / d, x - 25 / d, y + 5 / d, x - 25 / d, END, END);
	_HPlot(y - 5 / d, x - 30 / d, y + 5 / d, x - 30 / d, END, END);
	_HPlot(y - 10 / d, x - 35 / d, y - 10 / d, x - 40 / d, y - 5 / d, x - 45 / d, y + 5 / d, x - 45 / d, y + 10 / d, x - 40 / d, y + 10 / d, x - 35 / d, END, END);
	_HPlot(y - 10 / d, x - 40 / d, y, x - 45 / d, y + 10 / d, x - 40 / d, END, END);
	_HPlot(y - 5 / d, x - 40 / d, y + 5 / d, x - 40 / d, y + 15 / d, x - 30 / d, y, x - 40 / d, y - 15 / d, x - 30 / d, y - 5 / d + .5, x - 40 / d, END, END);
}

static void _DRAWCarrion(PARAMS) {
	/* 79-dst.recty(d) line here */
	_HPlot(y - 20 / d, x - 79 / d, y - 20 / d, x - 88 / d, y - 10 / d, x - 83 / d, y + 10 / d, x - 83 / d, y + 20 / d, x - 88 / d, y + 20 / d, x - 79 / d, y - 20 / d, x - 79 / d, END, END);
	_HPlot(y - 20 / d, x - 88 / d, y - 30 / d, x - 83 / d, y - 30 / d, x - 78 / d, END, END);
	_HPlot(y + 20 / d, x - 88 / d, y + 30 / d, x - 83 / d, y + 40 / d, x - 83 / d, END, END);
	_HPlot(y - 15 / d, x - 86 / d, y - 20 / d, x - 83 / d, y - 20 / d, x - 78 / d, y - 30 / d, x - 73 / d, y - 30 / d, x - 68 / d, y - 20 / d, x - 63 / d, END, END);
	_HPlot(y - 10 / d, x - 83 / d, y - 10 / d, x - 58 / d, y, x - 50 / d, END, END);
	_HPlot(y + 10 / d, x - 83 / d, y + 10 / d, x - 78 / d, y + 20 / d, x - 73 / d, y + 20 / d, x - 40 / d, END, END);
	_HPlot(y + 15 / d, x - 85 / d, y + 20 / d, x - 78 / d, y + 30 / d, x - 76 / d, y + 30 / d, x - 60 / d, END, END);
	_HPlot(y, x - 83 / d, y, x - 73 / d, y + 10 / d, x - 68 / d, y + 10 / d, x - 63 / d, y, x - 58 / d, END, END);
}

static void _DRAWGremlin(PARAMS) {
	_HPlot(y + 5 / d + .5, x - 10 / d, y - 5 / d + .5, x - 10 / d, y, x - 15 / d, y + 10 / d, x - 20 / d, y + 5 / d + .5, x - 15 / d, y + 5 / d + .5, x - 10 / d, END, END);
	_HPlot(y + 7 / d + .5, x - 6 / d, y + 5 / d + .5, x - 3 / d, y - 5 / d + .5, x - 3 / d, y - 7 / d + .5, x - 6 / d, y - 5 / d + .5, x - 10 / d, END, END);
	_HPlot(y + 2 / d + .5, x - 3 / d, y + 5 / d + .5, x, y + 8 / d, x, END, END);
	_HPlot(y - 2 / d + .5, x - 3 / d, y - 5 / d + .5, x, y - 8 / d, x, END, END);
	_HPlot(y + 3 / d + .5, x - 8 / d, END, END);
	_HPlot(y - 3 / d + .5, x - 8 / d, END, END);
	_HPlot(y + 3 / d + .5, x - 5 / d, y - 3 / d + .5, x - 5 / d, END, END);
}

static void _DRAWMimic(PARAMS) {
	double xx = x;
	_HPlot(139 - 10 / d, xx, 139 - 10 / d, xx - 10 / d, 139 + 10 / d, xx - 10 / d, 139 + 10 / d, xx, 139 - 10 / d, xx, END, END);
	_HPlot(139 - 10 / d, xx - 10 / d, 139 - 5 / d, xx - 15 / d, 139 + 15 / d, xx - 15 / d, 139 + 15 / d, xx - 5 / d, 139 + 10 / d, xx, END, END);
	_HPlot(139 + 10 / d, xx - 10 / d, 139 + 15 / d, xx - 15 / d, END, END);
}

static void _DRAWDaemon(PARAMS) {
	_HPlot(y - 14 / d, x - 46 / d, y - 12 / d, x - 37 / d, y - 20 / d, x - 32 / d, y - 30 / d, x - 32 / d, y - 22 / d, x - 24 / d, y - 40 / d, x - 17 / d, y - 40 / d, x - 7 / d, y - 38 / d, x - 5 / d, y - 40 / d, x - 3 / d, y - 40 / d, x, END, END);
	_HPlot(y - 36 / d, x, y - 34 / d, x - 2 / d, y - 32 / d, x, y - 28 / d, x, y - 28 / d, x - 3 / d, y - 30 / d, x - 5 / d, y - 28 / d, x - 7 / d, y - 28 / d, x - 15 / d, y, x - 27 / d, END, END);
	_HPlot(y + 14 / d, x - 46 / d, y + 12 / d, x - 37 / d, y + 20 / d, x - 32 / d, y + 30 / d, x - 32 / d, y + 22 / d, x - 24 / d, y + 40 / d, x - 17 / d, y + 40 / d, x - 7 / d, y + 38 / d, x - 5 / d, y + 40 / d, x - 3 / d, y + 40 / d, x, END, END);
	_HPlot(y + 36 / d, x, y + 34 / d, x - 2 / d, y + 32 / d, x, y + 28 / d, x, y + 28 / d, x - 3 / d, y + 30 / d, x - 5 / d, y + 28 / d, x - 7 / d, y + 28 / d, x - 15 / d, y, x - 27 / d, END, END);
	_HPlot(y + 6 / d, x - 48 / d, y + 38 / d, x - 41 / d, y + 40 / d, x - 42 / d, y + 18 / d, x - 56 / d, y + 12 / d, x - 56 / d, y + 10 / d, x - 57 / d, y + 8 / d, x - 56 / d, y - 8 / d, x - 56 / d, y - 10 / d, x - 58 / d, y + 14 / d, x - 58 / d, y + 16 / d, x - 59 / d, END, END);
	_HPlot(y + 8 / d, x - 63 / d, y + 6 / d, x - 63 / d, y + 2 / d + .5, x - 70 / d, y + 2 / d + .5, x - 63 / d, y - 2 / d + .5, x - 63 / d, y - 2 / d + .5, x - 70 / d, y - 6 / d, x - 63 / d, y - 8 / d, x - 63 / d, y - 16 / d, x - 59 / d, y - 14 / d, x - 58 / d, END, END);
	_HPlot(y - 10 / d, x - 57 / d, y - 12 / d, x - 56 / d, y - 18 / d, x - 56 / d, y - 36 / d, x - 47 / d, y - 36 / d, x - 39 / d, y - 28 / d, x - 41 / d, y - 28 / d, x - 46 / d, y - 20 / d, x - 50 / d, y - 18 / d, x - 50 / d, y - 14 / d, x - 46 / d, END, END);
	_HPlot(y - 28 / d, x - 41 / d, y + 30 / d, x - 55 / d, END, END);
	_HPlot(y + 28 / d, x - 58 / d, y + 22 / d, x - 56 / d, y + 22 / d, x - 53 / d, y + 28 / d, x - 52 / d, y + 34 / d, x - 54 / d, END, END);
	_HPlot(y + 20 / d, x - 50 / d, y + 26 / d, x - 47 / d, END, END);
	_HPlot(y + 10 / d, x - 58 / d, y + 10 / d, x - 61 / d, y + 4 / d, x - 58 / d, END, END);
	_HPlot(y - 10 / d, x - 58 / d, y - 10 / d, x - 61 / d, y - 4 / d, x - 58 / d, END, END);
	_HPlot(y + 40 / d, x - 9 / d, y + 50 / d, x - 12 / d, y + 40 / d, x - 7 / d, END, END);
	_HPlot(y - 8 / d, x - 25 / d, y + 6 / d, x - 7 / d, y + 28 / d, x - 7 / d, y + 28 / d, x - 9 / d, y + 20 / d, x - 9 / d, y + 6 / d, x - 25 / d, END, END);
}

static void _DRAWBalrog(PARAMS) {
	_HPlot(y + 6 / d, x - 60 / d, y + 30 / d, x - 90 / d, y + 60 / d, x - 30 / d, y + 60 / d, x - 10 / d, y + 30 / d, x - 40 / d, y + 15 / d, x - 40 / d, END, END);
	_HPlot(y - 6 / d, x - 60 / d, y - 30 / d, x - 90 / d, y - 60 / d, x - 30 / d, y - 60 / d, x - 10 / d, y - 30 / d, x - 40 / d, y - 15 / d, x - 40 / d, END, END);
	_HPlot(y, x - 25 / d, y + 6 / d, x - 25 / d, y + 10 / d, x - 20 / d, y + 12 / d, x - 10 / d, y + 10 / d, x - 6 / d, y + 10 / d, x, y + 14 / d, x, y + 15 / d, x - 5 / d, y + 16 / d, x, y + 20 / d, x, END, END);
	_HPlot(y + 20 / d, x - 6 / d, y + 18 / d, x - 10 / d, y + 18 / d, x - 20 / d, y + 15 / d, x - 30 / d, y + 15 / d, x - 45 / d, y + 40 / d, x - 60 / d, y + 40 / d, x - 70 / d, END, END);
	_HPlot(y + 10 / d, x - 55 / d, y + 6 / d, x - 60 / d, y + 10 / d, x - 74 / d, y + 6 / d, x - 80 / d, y + 4 / d + .5, x - 80 / d, y + 3 / d + .5, x - 82 / d, y + 2 / d + .5, x - 80 / d, y, x - 80 / d, END, END);
	_HPlot(y, x - 25 / d, y - 6 / d, x - 25 / d, y - 10 / d, x - 20 / d, y - 12 / d, x - 10 / d, y - 10 / d, x - 6 / d, y - 10 / d, x, y - 14 / d, x, y - 15 / d, x - 5 / d, y - 16 / d, x, y - 20 / d, x, END, END);
	_HPlot(y - 20 / d, x - 6 / d, y - 18 / d, x - 10 / d, y - 18 / d, x - 20 / d, y - 15 / d, x - 30 / d, y - 15 / d, x - 45 / d, y - 40 / d, x - 60 / d, y - 40 / d, x - 70 / d, END, END);
	_HPlot(y - 10 / d, x - 55 / d, y - 6 / d, x - 60 / d, y - 10 / d, x - 74 / d, y - 6 / d, x - 80 / d, y - 4 / d + .5, x - 80 / d, y - 3 / d + .5, x - 82 / d, y - 2 / d + .5, x - 80 / d, y, x - 80 / d, END, END);
	_HPlot(y - 6 / d, x - 25 / d, y, x - 6 / d, y + 10 / d, x, y + 4 / d + .5, x - 8 / d, y + 6 / d, x - 25 / d, END, END);
	_HPlot(y - 40 / d, x - 64 / d, y - 40 / d, x - 90 / d, y - 52 / d, x - 80 / d, y - 52 / d, x - 40 / d, END, END);
	_HPlot(y + 40 / d, x - 86 / d, y + 38 / d, x - 92 / d, y + 42 / d, x - 92 / d, y + 40 / d, x - 86 / d, y + 40 / d, x - 50 / d, END, END);
	_HPlot(y + 4 / d + .5, x - 70 / d, y + 6 / d, x - 74 / d, END, END);
	_HPlot(y - 4 / d + .5, x - 70 / d, y - 6 / d, x - 74 / d, END, END);
	_HPlot(y, x - 64 / d, y, x - 60 / d, END, END);
}

} // namespace Ultima0
} // namespace Ultima
