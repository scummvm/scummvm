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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/*$M $800,0,0*/
/*#include "Dos.h"*/

namespace Avalanche {

struct infotype {
            matrix<128,255,1,8,byte> chars;
            string data;
};
infotype table;
pointer where;
string comspec;
int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 getintvec(0x1f,where); move(where,table,1280);
 comspec=getenv("comspec");
 table.data=table.data+"Avalot;";
 setintvec(0x1f,&table);
 output << NL;
 output << "The Astounding Avvy DOS Shell...       don't forget to register!" << NL;
 output << NL;
 output << "Use the command EXIT to return to your game." << NL;
 output << NL;
 output << "Remember: You *mustn't* load any TSRs while in this DOS shell." << NL;
 output << NL;
 output << "This includes: GRAPHICS, PRINT, DOSKEY, and pop-up programs (like Sidekick.)" << NL;
 output << NL;
 output << "About to execute \"" << comspec << "\"..." << NL;
 swapvectors;
 exec("c:\\command.com","");
 swapvectors;
 setintvec(0x1f,where);
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.