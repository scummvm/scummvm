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

#ifndef ULTIMA8_USECODE_BITSET_H
#define ULTIMA8_USECODE_BITSET_H

#include "ultima/ultima8/usecode/global_storage.h"

namespace Ultima {
namespace Ultima8 {

class BitSet : public GlobalStorage {
public:
	BitSet();
	BitSet(unsigned int size);
	~BitSet();

	//! set the size. The old value is cleared
	//! \param size the new size (in bits)
	void setSize(unsigned int size) override;

	//! get a value
	//! \param pos zero-based position (in bits)
	//! \param n number of bits (no greater than 32)
	//! \return the value these bits represent
	uint32 getEntries(unsigned int pos, unsigned int n) const override;

	//! set a value
	//! \param pos zero-based position (in bits)
	//! \param n number of bits (no greater than 32)
	//! \param bits the value to set
	void setEntries(unsigned int pos, unsigned int n, uint32 bits) override;

	void save(Common::WriteStream *ws) override;
	bool load(Common::ReadStream *rs, uint32 version) override;

private:
	unsigned int _size;
	unsigned int _bytes;
	uint8 *_data;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
