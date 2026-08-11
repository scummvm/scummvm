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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

static const byte kBlankScoreD2[] = {
	0x00, 0x00, 0x00, 0x06, // _framesStreamSize
	0x00, 0x02              // frame with empty channel information
};

static const byte kBlankScoreD4[] = {
	0x00, 0x00, 0x00, 0x12, // _framesStreamSize
	0x00, 0x00, 0x00, 0x10, // frame1Offset
	0x00, 0x00, 0x00, 0x01, // numOfFrames
	0x00, 0x07,             // _framesVersion
	0x00, 0x00,				// _numChannels
	0x00, 0x00,             // skipped
	0x00, 0x02              // frame with empty channel information
};
