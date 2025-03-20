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
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HODJNPODJ_NOVACANCY_DEFINES_H
#define HODJNPODJ_NOVACANCY_DEFINES_H

namespace Bagel {
namespace HodjNPodj {
namespace NoVacancy {

#define BACKGROUND_BMP "novac/art/novacsy3.bmp"

#define LEFT 0
#define RIGHT 1                             

#define FIXED			2     
#define CLOSED	 	1
#define OPEN			 0

#define  toggle(V)  (V=!(V))
#define is_locked(door)		(m_iDoorStatus[door]==FIXED)

#define NUM_DOOR_CELS 5                   //originally 9.
#define NUM_LDIE_CELS 	12					//originally 22
#define NUM_RDIE_CELS	11//9					//originally 17
#define NUM_SINGLE_DIE_CELS			 NUM_LDIE_CELS	
#define BOTTLE_CELS			28
#define HAT4_CELS			10
#define SLEEP_OPENING_TIME			(810/NUM_DOOR_CELS)                            
#define SLEEP_CLOSING_TIME			(740/NUM_DOOR_CELS)                

#define _STRINGTABLE	1                 // use stringtable for retrieving string constants.

#define RULES					"novac/novac.txt"						//	rules text file.                

} // namespace NoVacancy
} // namespace HodjNPodj
} // namespace Bagel

#endif
