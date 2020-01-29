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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA6_CORE_CONSOLE_H
#define ULTIMA6_CORE_CONSOLE_H

//#include <stdarg.h>



#include "ultima/ultima6/gui/gui_console.h"
#include "ultima/ultima6/screen/screen.h"
#include "ultima/ultima6/conf/configuration.h"

namespace Ultima {
namespace Ultima6 {

class Console : public GUI_Console {

protected:
	GUI *gui;
	Screen *screen;
	Configuration *config;

	bool displayConsole;

public:
	Console(Configuration *c, Screen *s, GUI *g, uint16 x, uint16 y, uint16 w, uint16 h);
	~Console();

	void AddLine(Std::string line);

protected:

};

void ConsoleInit(Configuration *c, Screen *s, GUI *gui, uint16 w, uint16 h);
void ConsoleDelete();
void ConsoleAddInfo(const char *s, ...);
void ConsoleAddError(Std::string s);
void ConsoleAddWarning(Std::string s);
void ConsolePause();
void ConsoleShow();
void ConsoleHide();

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
