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

#ifndef M4_GRAPHICS_GRAPHICS_H
#define M4_GRAPHICS_GRAPHICS_H

namespace M4 {

constexpr int SCREEN_WIDTH = 640;
constexpr int SCREEN_HEIGHT = 480;

#define FILL_INTERIOR 1	// a flag for use by DrawTile
#define BORDER 0

//SS FILE DATA DEFINITIONS...
#define HEAD_M4SS		0x4D345353	//'M4SS'
#define HEAD_SS4M		0x5353344D	//'SS4M'
#define SS_FORMAT		101			//if it ever has to be printed, divide by 100

#define CELS__PAL		0x2050414C	//' PAL'
#define CELS_LAP_		0x4C415020	//INTEL ' PAL'
#define CELS___SS		0x20205353	//'  SS'
#define CELS_SS__		0x53532020	//INTEL '  SS'

#define CELS_SRC_SIZE		1

#define CELS_FRAME_RATE		3
#define CELS_PIX_SPEED		4
#define CELS_SS_MAX_W		5
#define CELS_SS_MAX_H		6

#define CELS_COUNT			13
#define SS_HEAD_SIZE		14	//includes all the previous dwords
#define CELS_OFFSETS		14

#define CELS_STREAM			1
#define CELS_X				2
#define CELS_Y				3
#define CELS_W				4
#define CELS_H				5
#define CELS_COMP			6

#define SS_INDV_HEAD		15	//includes all the previous dwords
#define CELS_DATA			15

} // namespace M4

#endif
