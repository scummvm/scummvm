
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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "elf-loader.h"
#include "shorts-segment-manager.h"

class MIPSDLObject : public DLObject {
protected:
	ShortSegmentManager::Segment *_shortsSegment;			// For assigning shorts ranges
	unsigned int _gpVal;									// Value of Global Pointer

	bool relocate(Common::SeekableReadStream* DLFile, unsigned long offset, unsigned long size, void *relSegment);
    bool relocateRels(Common::SeekableReadStream* DLFile, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr);
    void relocateSymbols(Elf32_Addr offset);
    bool loadSegment(Common::SeekableReadStream* DLFile, Elf32_Phdr *phdr);
    void unload();

public:
    MIPSDLObject() : DLObject() {
		_shortsSegment = NULL;
		_gpVal = 0;
    }
};
