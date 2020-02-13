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

#ifndef ADRIFT_SERIALIZATION_H
#define ADRIFT_SERIALIZATION_H

#include "common/memstream.h"
#include "common/str.h"
#include "glk/adrift/scprotos.h"
#include "glk/jumps.h"

namespace Glk {
namespace Adrift {

/**
 * Saving serializer class
 */
class SaveSerializer {
private:
	sc_gameref_t _game;
	sc_write_callbackref_t _callback;
	void *_opaque;
	Common::MemoryWriteStreamDynamic _buffer;
private:
	/**
	 * Flush pending buffer contents
	 */
	void flush(sc_bool is_final);

	/**
	 * add a character to the buffer.
	 */
	void writeChar(sc_char character);

	/**
	 * Write a buffer
	 */
	void write(const sc_char *buffer, sc_int length);

	/**
	 * Write a string
	 */
	void writeString(const sc_char *string);

	/**
	 * Write an integer
	 */
	void writeInt(sc_int value);

	/**
	 * Write a special/long integer
	 */
	void writeIntSpecial(sc_int value);

	/**
	 * Write an unsigned integer
	 */
	void writeUint(sc_uint value);

	/**
	 * Write a boolean
	 */
	void writeBool(sc_bool boolean);
public:
	/**
	 * Constructor
	 */
	SaveSerializer(sc_gameref_t game, sc_write_callbackref_t callback, void *opaque) :
		_game(game), _callback(callback), _opaque(opaque), _buffer(DisposeAfterUse::YES) {
		assert(callback);
	}

	/**
	 * Save method
	 */
	void save();
};

/**
 * Loading serializer class
 */
class LoadSerializer {
private:
	sc_gameref_t _game;
	sc_read_callbackref_t _callback;
	void *_opaque;
	sc_tafref_t ser_tas;
	sc_int ser_tasline;
private:
	/**
	 * Reads a string
	 */
	const sc_char *readString(CONTEXT);

	/**
	 * Read a signed integer
	 */
	sc_int readInt(CONTEXT);

	/**
	 * Read an unsigned integer
	 */
	sc_uint readUint(CONTEXT);

	/**
	 * Read a boolean
	 */
	sc_bool readBool(CONTEXT);
public:
	/**
	 * Constructor
	 */
	LoadSerializer(sc_gameref_t game, sc_read_callbackref_t callback, void *opaque) :
			_game(game), _callback(callback), _opaque(opaque), ser_tas(nullptr), ser_tasline(0) {
		assert(callback);
	}

	/**
	 * Does the loading
	 */
	bool load();
};

} // End of namespace Adrift
} // End of namespace Glk

#endif
