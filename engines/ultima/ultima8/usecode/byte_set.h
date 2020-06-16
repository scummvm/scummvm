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

#ifndef ULTIMA8_USECODE_BYTESET_H
#define ULTIMA8_USECODE_BYTESET_H

#include "ultima/ultima8/usecode/global_storage.h"

namespace Ultima {
namespace Ultima8 {

// This is a minimal implementation just to support what Crusader needs.

class ByteSet : public GlobalStorage {
public:
	ByteSet();
	ByteSet(unsigned int size);
	~ByteSet();

	//! set the size. The old value is cleared
	//! \param size the new size (in bytes)
	void setSize(unsigned int size) override;

	//! get a value
	//! \param pos zero-based position (in bytes:)
	//! \param n number of bytes (no greater than 2)
	//! \return the value these bytes represent
	uint32 getEntries(unsigned int pos, unsigned int n) const override;

	//! set a value
	//! \param pos zero-based position (in bytes)
	//! \param n number of bytes (no greater than 2)
	//! \param val the value to set
	void setEntries(unsigned int pos, unsigned int n, uint32 val) override;

	void save(Common::WriteStream *ws) override;
	bool load(Common::ReadStream *rs, uint32 version) override;

private:
	unsigned int _size;
	uint8 *_data;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
