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

#ifndef DGDS_INCLUDES_H
#define DGDS_INCLUDES_H

namespace Dgds {

#define MKTAG24(a0, a1, a2) ((uint32)((a2) | (a1) << 8 | ((a0) << 16)))

#define ID_BIN MKTAG24('B', 'I', 'N')
#define ID_CGA MKTAG24('C', 'G', 'A')
#define ID_DAT MKTAG24('D', 'A', 'T')
#define ID_EGA MKTAG24('E', 'G', 'A')
#define ID_FNM MKTAG24('F', 'N', 'M')
#define ID_FNT MKTAG24('F', 'N', 'T')
#define ID_GAD MKTAG24('G', 'A', 'D')
#define ID_GDS MKTAG24('G', 'D', 'S')
#define ID_INF MKTAG24('I', 'N', 'F')
#define ID_MTX MKTAG24('M', 'T', 'X')
#define ID_OFF MKTAG24('O', 'F', 'F')
#define ID_PAG MKTAG24('P', 'A', 'G')
#define ID_PAL MKTAG24('P', 'A', 'L')
#define ID_RAW MKTAG24('R', 'A', 'W')
#define ID_REQ MKTAG24('R', 'E', 'Q')
#define ID_RES MKTAG24('R', 'E', 'S')
#define ID_SCR MKTAG24('S', 'C', 'R')
#define ID_SCN MKTAG24('S', 'C', 'N')
#define ID_SDS MKTAG24('S', 'D', 'S')
#define ID_SND MKTAG24('S', 'N', 'D')
#define ID_SNG MKTAG24('S', 'N', 'G')
#define ID_TAG MKTAG24('T', 'A', 'G')
#define ID_TT3 MKTAG24('T', 'T', '3')
#define ID_TTI MKTAG24('T', 'T', 'I')
#define ID_VER MKTAG24('V', 'E', 'R')
#define ID_VGA MKTAG24('V', 'G', 'A')
#define ID_VQT MKTAG24('V', 'Q', 'T')

/* Heart of China */
#define ID_MA8 MKTAG24('M', 'A', '8')
#define ID_DDS MKTAG24('D', 'D', 'S')
#define ID_THD MKTAG24('T', 'H', 'D')

/* EX_ are File extensions types */
#define EX_ADH MKTAG24('A', 'D', 'H')
#define EX_ADL MKTAG24('A', 'D', 'L')
#define EX_ADS MKTAG24('A', 'D', 'S')
#define EX_AMG MKTAG24('A', 'M', 'G')
#define EX_BMP MKTAG24('B', 'M', 'P')
#define EX_FNT MKTAG24('F', 'N', 'T')
#define EX_GDS MKTAG24('G', 'D', 'S')
#define EX_INS MKTAG24('I', 'N', 'S')
#define EX_PAL MKTAG24('P', 'A', 'L')
#define EX_RAW MKTAG24('R', 'A', 'W')
#define EX_REQ MKTAG24('R', 'E', 'Q')
#define EX_RST MKTAG24('R', 'S', 'T')
#define EX_SCR MKTAG24('S', 'C', 'R')
#define EX_SDS MKTAG24('S', 'D', 'S')
#define EX_SNG MKTAG24('S', 'N', 'G')
#define EX_SX MKTAG24('S', 'X', 0)
#define EX_TTM MKTAG24('T', 'T', 'M')
#define EX_VIN MKTAG24('V', 'I', 'N')

/* Heart of China */
#define EX_DAT MKTAG24('D', 'A', 'T')
#define EX_DDS MKTAG24('D', 'D', 'S')
#define EX_TDS MKTAG24('T', 'D', 'S')

#define EX_OVL MKTAG24('O', 'V', 'L')

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

} // End of namespace Dgds

#endif // DGDS_INCLUDES_H
