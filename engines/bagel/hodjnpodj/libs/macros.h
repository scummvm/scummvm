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

#ifndef BAGEL_HODJNPODJ_LIBS_MACROS_H
#define BAGEL_HODJNPODJ_LIBS_MACROS_H

#define EM(X)   ErrorLog(".\\msg.log",(X));
#define EMCR()  ErrorLog(".\\msg.log","\n");
#define EMTime(t)   ErrorLog(".\\msg.log","%s",_strtime(t));
#define EMLoc(i)    ErrorLog(".\\msg.log","Locals Dump #%d",(i));

#define DMint(X)    ErrorLog(".\\msg.log",#X"=%d",(X));
#define DMstr(X)    ErrorLog(".\\msg.log",#X"=%s",(X));
#define  DMaddr(X)  {\
		(X)? ErrorLog(".\\msg.log",#X"=%p",(void _far*)(X)) : ErrorLog(".\\msg.log",#X"is nullptr");   }

#endif
