/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

namespace ICB {

extern uint32 fileGetZipLength(const char *inFn);
uint32 fileGetZipLength2(Common::SeekableReadStream *fileStream);

// There are now two memUncompress's
// One takes a FILE * pointer and a place to unzip to
// The second takes a url, opens it, and calls the first

uint32 memUncompress(uint8 *outMem, const char *inFn);
uint32 memUncompress(uint8 *outMem, const char *inFn, Common::SeekableReadStream *fileStream);

} // End of namespace ICB
