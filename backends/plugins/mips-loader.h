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

#ifndef MIPS_LOADER_H
#define MIPS_LOADER_H

#include "loader.h"
#include "shorts-segment-manager.h"

class MIPS_DLObject : public DLObject {
protected:
	ShortSegmentManager::Segment *_shortsSegment;			// For assigning shorts ranges
	unsigned int _gpVal;									// Value of Global Pointer

public:
	MIPS_DLObject(char *errbuf = NULL) : _errbuf(_errbuf), _shortsSegment(NULL), _segment(NULL), _symtab(NULL),
			_strtab(NULL), _symbol_cnt(0), _symtab_sect(-1), _dtors_start(NULL), _dtors_end(NULL), _gpVal(0) ,
			_segmentSize(0) {}
};

#endif /* MIPS_LOADER_H */
