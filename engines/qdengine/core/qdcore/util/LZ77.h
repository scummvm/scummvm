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

#ifndef __LZ77_H
#define __LZ77_H


namespace QDEngine {

class CLZ77 {
private:
	long LZComp(unsigned char *s1, unsigned char *s2, long maxlen);
	unsigned char *FindLZ(unsigned char *source, unsigned char *s, long slen, long border, long mlen, long &len);
public:
	CLZ77();
	virtual ~CLZ77();

	void Encode(unsigned char *target, long &tlen, const unsigned char *source, long slen);
	long Decode(unsigned char *target, long &tlen, const unsigned char *source, long slen);
	long GetMaxEncoded(long len);
	long GetMaxDecoded(unsigned char *source);

//	virtual void OnStep() = 0;
};

} // namespace QDEngine

#endif
