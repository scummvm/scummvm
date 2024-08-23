/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef QDENGINE_QDCORE_UTIL_LZ77_H
#define QDENGINE_QDCORE_UTIL_LZ77_H


namespace QDEngine {

class CLZ77 {
private:
	int32 lzComp(const byte *s1, const byte *s2, int32 maxlen);
	const byte *findLZ(const byte *source, const byte *s, int32 slen, int32 border, int32 mlen, int32 &len);
public:
	CLZ77();
	virtual ~CLZ77();

	void encode(byte *target, int32 &tlen, const byte *source, int32 slen);
	int32 decode(byte *target, int32 &tlen, const byte *source, int32 slen);
	int32 getMaxEncoded(int32 len);
	int32 getMaxDecoded(byte *source);

//	virtual void OnStep() = 0;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_UTIL_LZ77_H
