/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Actor data table
#include "saga/saga.h"
#include "saga/objectdata.h"

namespace Saga {


ObjectTableData ITE_ObjectTable[ITE_OBJECTCOUNT] = {
	{  8,  49, 1256,  760,  0,  9,  5, kObjNotFlat }, // Magic Hat
	{  9,  52, 1080, 1864,  0, 10,  4, kObjUseWith }, // Berries
	{ 10, 259,  744,  524,  0, 11, 42, kObjUseWith }, // Card Key
	{ 11,   0,  480,  480,  0, 12,  6, 0           }, // Foot Print
	{ 12,   0,  480,  480,  0, 13, 38, kObjUseWith }, // Power Cell
	{ 13,  28,  640,  412, 40, 14, 15, kObjUseWith }, // Digital Clock
	{ 14,   0,  480,  480,  0, 15, 41, kObjUseWith }, // Oil Lamp
	{ 15,  24,  868,  456, 35, 16, 13, kObjUseWith }, // Magnetic Key
	{ 16,   0,  480,  480,  0, 17,  7, kObjUseWith }, // Plaster
	{ 17, 249,  320,  476, 45, 18, 44, 0           }, // Trophy
	{ 18,   0,  480,  480,  0, 19, 20, 0           }, // Coins
	{ 19,  19,  600,  480,  0, 20,  8, 0           }, // Lens Fragments
	{ 20,   0, 1012,  568, 80, 21, 10, kObjUseWith }, // Key to jail cell
	{ 21,   0,  480,  480,  0, 22,  9, 0           }, // Remade lens
	{ 22,   0,  480,  480,  0, 23, 21, 0           }, // Tycho's Map
	{ 23,   0,  480,  480,  0, 24, 23, 0           }, // Silver Medallion
	{ 24,   0,  480,  480,  0, 25, 24, 0           }, // Mud in Fur
	{ 25,   0,  480,  480,  0, 26, 25, 0           }, // Gold Ring
	{ 27,  13, 1036,  572,  0, 47, 14, kObjUseWith }, // Screwdriver
	{ 28,   0,  480,  480,  0, 29, 26, 0           }, // Apple Token
	{ 29,   0,  480,  480,  0, 30, 22, kObjUseWith }, // Letter from Elara
	{ 30,   0,  164,  440,  0, 31, 16, kObjUseWith }, // Spoon
	{ 32,   0,  480,  480,  0, 33, 43, 0           }, // Catnip
	{ 33,  31,  580,  392,  0, 45, 11, 0           }, // Twigs
	{ 35,   0,  468,  480,  0, 36, 12, kObjUseWith }, // Empty Bowl (also bowl of honey)
	{ 37,   0,  480,  480,  0, 38, 45, kObjUseWith }, // Needle and Thread
	{ 38,  25,  332,  328,  0, 39, 19, 0           }, // Rock Crystal
	{ 39,   0,  480,  480,  0, 40,  0, kObjUseWith }, // Salve
	{ 40, 269,  644,  416,  0, 41, 39, kObjNotFlat }, // Electrical Cable
	{ 41,  12,  280,  516,  0, 42, 17, kObjUseWith }, // Piece of flint
	{ 42,   5,  876,  332, 32, 65, 18, 0           }, // Rat Cloak
	{ 43,  52,  556, 1612,  0, 49, 28, kObjUseWith | kObjNotFlat }, // Bucket
	{ 48,  52,  732,  948,  0, 50, 27, kObjUseWith }, // Cup
	{ 49,  52,  520, 1872,  0, 53, 29, 0           }, // Fertilizer
	{ 50,  52, 1012, 1268,  0, 52, 30, 0           }, // Feeder
	{ 51, 252,  -20,  -20,  0, 71, 32, kObjUseWith | kObjNotFlat }, // Bowl in jail cell
	{ 53, 252, 1148,  388,  0, 70, 33, 0           }, // Loose stone block in jail cell
	{ 26,  12,  496,  368,  0, 76, 31, 0           }, // Coil of Rope from Quarry
	{ 54, 281,  620,  352,  0, 80, 46, 0           }  // Orb of Storms in Dam Lab
};

} // End of namespace Saga
