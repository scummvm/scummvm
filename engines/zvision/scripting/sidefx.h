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
 */

#ifndef SIDEFX_H_INCLUDED
#define SIDEFX_H_INCLUDED

namespace Common {
class SeekableReadStream;
struct Point;
class WriteStream;
}

namespace ZVision {

class ZVision;

class SideFX {
public:

	enum SideFXType {
		SIDEFX_ANIM  = 1,
		SIDEFX_AUDIO = 2,
		SIDEFX_DISTORT = 4,
		SIDEFX_PANTRACK = 8,
		SIDEFX_REGION = 16,
		SIDEFX_TIMER = 32,
		SIDEFX_TTYTXT = 64,
		SIDEFX_UNK = 128,
		SIDEFX_ALL = 255
	};

	SideFX() : _engine(0), _key(0), _type(SIDEFX_UNK) {}
	SideFX(ZVision *engine, uint32 key, SideFXType type) : _engine(engine), _key(key), _type(type) {}
	virtual ~SideFX() {}

	uint32 getKey() {
		return _key;
	}
	SideFXType getType() {
		return _type;
	}

	virtual bool process(uint32 deltaTimeInMillis) {
		return false;
	}
	/**
	 * Serialize a SideFX for save game use. This should only be used if a SideFX needs
	 * to save values that would be different from initialization. AKA a TimerNode needs to
	 * store the amount of time left on the timer. Any Controls overriding this *MUST* write
	 * their key as the first data outputted. The default implementation is NOP.
	 *
	 * NOTE: If this method is overridden, you MUST also override deserialize()
	 * and needsSerialization()
	 *
	 * @param stream    Stream to write any needed data to
	 */
	virtual void serialize(Common::WriteStream *stream) {}
	/**
	 * De-serialize data from a save game stream. This should only be implemented if the
	 * SideFX also implements serialize(). The calling method assumes the size of the
	 * data read from the stream exactly equals that written in serialize(). The default
	 * implementation is NOP.
	 *
	 * NOTE: If this method is overridden, you MUST also override serialize()
	 * and needsSerialization()
	 *
	 * @param stream    Save game file stream
	 */
	virtual void deserialize(Common::SeekableReadStream *stream) {}
	/**
	 * If a SideFX overrides serialize() and deserialize(), this should return true
	 *
	 * @return    Does the SideFX need save game serialization?
	 */
	virtual inline bool needsSerialization() {
		return false;
	}

	virtual bool stop() {
		return true;
	}
	virtual void kill() {}

protected:
	ZVision *_engine;
	uint32 _key;
	SideFXType _type;

// Static member functions
public:

};
} // End of namespace ZVision

#endif // SIDEFX_H_INCLUDED
