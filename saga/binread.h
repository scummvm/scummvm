/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#ifndef SAGA_BINREAD_H
#define SAGA_BINREAD_H

#include <stddef.h>
#include "common/scummsys.h"

namespace Saga {

class BinReader {

protected:

	const byte *_buf;
	const byte *_bufPtr;
	const byte *_bufEnd;
	size_t      _bufLen;

public:
	
	BinReader();
	BinReader( const byte *buf, size_t buflen );
	virtual ~BinReader();

	void   setBuf( const byte *buf, size_t buflen );

	size_t getOffset() const;
	bool   setOffset( size_t offset );
	bool   setROffset( ptrdiff_t offset ); 
	void   skip( size_t skip_ct );

	bool  setPtr( const byte *buf_pos );
	byte *getPtr() const;

	unsigned int readUint16LE();
	unsigned int readUint16BE();
	int          readSint16LE();
	int          readSint16BE();
	uint32       readUint32LE();
	uint32       readUint32BE();
	int32        readSint32LE();
	int32        readSint32BE();
};

} // End of namespace Saga

#endif






