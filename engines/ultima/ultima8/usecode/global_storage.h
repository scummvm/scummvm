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

#ifndef ULTIMA8_USECODE_GLOBAL_STORAGE_H
#define ULTIMA8_USECODE_GLOBAL_STORAGE_H

namespace Ultima {
namespace Ultima8 {

class GlobalStorage {
public:
	virtual ~GlobalStorage() {};

	virtual void setSize(unsigned int size) = 0;

	//! get a value
	//! \param pos zero-based position
	//! \param n number of entries to read
	//! \return the value these entries represent
	virtual uint32 getEntries(unsigned int pos, unsigned int n) const = 0;

	//! set a value
	//! \param pos zero-based position
	//! \param n number of entries (no greater than one uint32-worth)
	//! \param val the value to set
	virtual void setEntries(unsigned int pos, unsigned int n, uint32 val) = 0;

	virtual void save(Common::WriteStream *ws) = 0;
	virtual bool load(Common::ReadStream *rs, uint32 version) = 0;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
