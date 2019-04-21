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

#ifndef CRYO_PLATDEFS_H
#define CRYO_PLATDEFS_H

namespace Cryo {

#if 1
const int _subtitlesXMargin = 16;          //PC
const int _subtitlesXScrMargin = 16;
const int _spaceWidth = 6;
#define FAKE_DOS_VERSION
#else
const int _subtitlesXMargin = 16;          //MAC
const int _subtitlesXScrMargin = 16;          //MAC
const int _spaceWidth = 4;
#endif
const int _subtitlesXWidth = (320 - _subtitlesXMargin * 2);
const int _subtitlesXCenter = _subtitlesXWidth / 2;

} // End of namespace Cryo

#endif
