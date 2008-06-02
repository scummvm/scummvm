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

#ifndef GOB_VARIABLES_H
#define GOB_VARIABLES_H

namespace Gob {

class Variables {
public:
	Variables(uint32 size);
	virtual ~Variables();

	void writeVar8(uint32 var, uint8 value);
	void writeVar16(uint32 var, uint16 value);
	void writeVar32(uint32 var, uint32 value);

	void writeVarString(uint32 var, const char *value);

	void writeOff8(uint32 offset, uint8 value);
	void writeOff16(uint32 offset, uint16 value);
	void writeOff32(uint32 offset, uint32 value);

	void writeOffString(uint32 offset, const char *value);

	uint8 readVar8(uint32 var) const;
	uint16 readVar16(uint32 var) const;
	uint32 readVar32(uint32 var) const;

	void readVarString(uint32 var, char *value, uint32 length);

	uint8 readOff8(uint32 offset) const;
	uint16 readOff16(uint32 offset) const;
	uint32 readOff32(uint32 offset) const;

	void readOffString(uint32 offset, char *value, uint32 length);


	const uint8 *getAddressVar8(uint32 var) const;
	uint8 *getAddressVar8(uint32 var, uint32 n = 1);

	const uint16 *getAddressVar16(uint32 var) const;
	uint16 *getAddressVar16(uint32 var, uint32 n = 1);

	const uint32 *getAddressVar32(uint32 var) const;
	uint32 *getAddressVar32(uint32 var, uint32 n = 1);

	const char *getAddressVarString(uint32 var) const;
	char *getAddressVarString(uint32 var, uint32 n = 0xFFFFFFFF);

	const uint8 *getAddressOff8(uint32 offset) const;
	uint8 *getAddressOff8(uint32 offset, uint32 n = 1);

	const uint16 *getAddressOff16(uint32 offset) const;
	uint16 *getAddressOff16(uint32 offset, uint32 n = 1);

	const uint32 *getAddressOff32(uint32 offset) const;
	uint32 *getAddressOff32(uint32 offset, uint32 n = 1);

	const char *getAddressOffString(uint32 offset) const;
	char *getAddressOffString(uint32 offset, uint32 n = 0xFFFFFFFF);


	bool copyTo(uint32 offset, byte *variables, byte *sizes, uint32 n) const;
	bool copyFrom(uint32 offset, const byte *variables, const byte *sizes, uint32 n);

protected:
	virtual void write8(byte *buf, uint8 data) const = 0;
	virtual void write16(byte *buf, uint16 data) const = 0;
	virtual void write32(byte *buf, uint32 data) const = 0;

	virtual uint8 read8(const byte *buf) const = 0;
	virtual uint16 read16(const byte *buf) const = 0;
	virtual uint32 read32(const byte *buf) const = 0;

private:
	// Basically the number of additional bytes occupied
	static const byte kSize8 = 0;
	static const byte kSize16 = 1;
	static const byte kSize32 = 3;

	uint32 _size;

	byte *_vars;
	byte *_sizes;

	void clear();
	void clearSize(uint32 offset);
	void writeSize(uint32 offset, byte n);
	void writeSizeString(uint32 offset, uint32 length);
};

class VariablesLE : public Variables {
public:
	VariablesLE(uint32 size);
	~VariablesLE();

protected:
	void write8(byte *buf, uint8 data) const;
	void write16(byte *buf, uint16 data) const;
	void write32(byte *buf, uint32 data) const;

	uint8 read8(const byte *buf) const;
	uint16 read16(const byte *buf) const;
	uint32 read32(const byte *buf) const;
};

class VariablesBE : public Variables {
public:
	VariablesBE(uint32 size);
	~VariablesBE();

protected:
	void write8(byte *buf, uint8 data) const;
	void write16(byte *buf, uint16 data) const;
	void write32(byte *buf, uint32 data) const;

	uint8 read8(const byte *buf) const;
	uint16 read16(const byte *buf) const;
	uint32 read32(const byte *buf) const;
};

} // End of namespace Gob

#endif // GOB_VARIABLES_H
