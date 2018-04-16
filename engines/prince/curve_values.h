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

namespace Prince {

const int curveValues[17][4] = {
	{ 32768, 0, 0, 0 },
	{ 25200, 7200, 480, -112 },
	{ 18816, 12544, 1792, -384 },
	{ 13520, 16224, 3744, -720 },
	{ 9216, 18432, 6144, -1024 },
	{ 5808, 19360, 8800, -1200 },
	{ 3200, 19200, 11520, -1152 },
	{ 1296, 18144, 14112, -784 },
	{ 0, 16384, 16384, 0 },
	{ -784, 14112, 18144, 1296 },
	{ -1152, 11520, 19200, 3200 },
	{ -1200, 8800, 19360, 5808 },
	{ -1024, 6144, 18432, 9216 },
	{ -720, 3744, 16224, 13520 },
	{ -384, 1792, 12544, 18816 },
	{ -112, 480, 7200, 25200 },
	{ 0, 0, 0, 32768 }
};

} // End of namespace Prince
