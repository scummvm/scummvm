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

/*
           XConsole (Windows 32 API version)
     By K-D Lab::KranK, Dr.Tronick, K-D Lab::Steeler (C) 1993-97 V3.0nt
*/

#ifndef __XCONSOLE_H
#define __XCONSOLE_H

#undef  NULL
#ifndef NULL
#define NULL    0L
#endif



namespace QDEngine {

struct XConsole {
	int radix;
	int digits;

	int tab;

	void UpdateBuffer(const char *);

	XConsole();
	~XConsole();

	XConsole& operator< (const char*);
	XConsole& operator< (char);
	XConsole& operator< (unsigned char);

	XConsole& operator<= (short);
	XConsole& operator<= (unsigned short);
	XConsole& operator<= (int);
	XConsole& operator<= (unsigned int);
	XConsole& operator<= (long);
	XConsole& operator<= (unsigned long);
	XConsole& operator<= (float);
	XConsole& operator<= (double);
	XConsole& operator<= (long double);

	void SetRadix(int _radix) {
		radix = _radix;
	}
	void SetDigits(int _digits) {
		digits = _digits;
	}
	void SetTab(int size) {
		tab = size;
	}
	void clear();

	void setpos(int _x, int _y);
	void getpos(int &x, int &y);
	void initialize(int mode);
};

extern XConsole XCon;

} // namespace QDEngine

#endif /* __XCONSOLE_H */
