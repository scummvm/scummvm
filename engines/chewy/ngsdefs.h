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

#ifndef CHEWY_NGSDEFS_H
#define CHEWY_NGSDEFS_H

#include "common/events.h"

namespace Chewy {

#define MAXSPRITE 620
#define NOSPEICHER 0
#define MAXFONTBREITE 32
#define MAXFONTHOEHE 32

#define CHECK 0x00
#define WRITE 0x04
#define READ 0x08

#define MCGA_SPEICHER 64000L
#define SETZEN 0
#define UND 1
#define ODER 2
#define XODER 3

#define COPY_PAGE _G(out)->screen2screen

#define END_POOL 32767

#define MAXDIRS 50
#define MAXFNAMEN 50
#define GRAFIK 1
#define SPEICHER 2
#define DATEI 3
#define MOD15 10
#define TYPE_ANZ 27
#define PCXDATEI 0
#define TBFDATEI 1
#define TAFDATEI 2
#define TFFDATEI 3
#define VOCDATEI 4
#define TPFDATEI 5
#define TMFDATEI 6
#define MODDATEI 7
#define RAWDATEI 8
#define LBMDATEI 9
#define RDIDATEI 10
#define TXTDATEI 11
#define IIBDATEI 12
#define SIBDATEI 13
#define EIBDATEI 14
#define ATSDATEI 15
#define SAADATEI 16
#define FLCDATEI 17
#define AADDATEI 18
#define ADSDATEI 19
#define ADHDATEI 20
#define TGPDATEI 21
#define TVPDATEI 22
#define TTPDATEI 23
#define TAPDATEI 24
#define CFODATEI 25
#define TCFDATEI 26
#define NODATEI 255
#define SCREENSAVE 0
#define SPRITESAVE 1

#define ALT 0x1000

#define MAUS_LINKS 255
#define MAUS_RECHTS 1
#define MAUS_MITTE 254

#define SOUND 5
#define BASE 0x220
#define JA 1
#define NEIN 0
#define ON 1
#define OFF 0

} // namespace Chewy

#endif
