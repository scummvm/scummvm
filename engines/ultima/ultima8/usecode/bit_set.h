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

namespace Ultima {
namespace Ultima8 {

class IDataSource;
class ODataSource;

class BitSet {
public:
	BitSet();
	BitSet(unsigned int size);
	~BitSet();

	//! set the size. The old value is cleared
	//! \param size the new size (in bits)
	void setSize(unsigned int size);

	//! get a value
	//! \param pos zero-based position (in bits)
	//! \param n number of bits (no greater than 32)
	//! \return the value these bits represent
	uint32 getBits(unsigned int pos, unsigned int n) const;

	//! set a value
	//! \param pos zero-based position (in bits)
	//! \param n number of bits (no greater than 32)
	//! \param bits the value to set
	void setBits(unsigned int pos, unsigned int n, uint32 bits);

	void save(ODataSource *ods);
	bool load(IDataSource *ids, uint32 version);

private:
	unsigned int _size;
	unsigned int _bytes;
	uint8 *_data;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
