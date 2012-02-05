/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_PATCHR_H
#define GRIM_PATCHR_H

namespace Common {
template<class T> class Array;
class String;
class SeekableReadStream;
}

namespace Grim {

class Patchr {
public:
	Patchr(): _data(NULL), _err(false), _kMd5size(5000), _kMaxFileSize(0x100000) {}
	~Patchr() { if (_data) delete[] _data; }
	void loadPatch(Common::SeekableReadStream *patchStream);
	bool patchFile(Common::SeekableReadStream *&file, const Common::String &name);
private:
	uint32 _kMaxFileSize;
	uint32 _kMd5size;

	enum Instruction { BEGIN, END, REPLACE, INSERT, DELETE, FILL, COPY, INVALID };
	static const char *InstructionS[8];
	struct Op {
		Instruction ist;
		Common::Array<Common::String> args;
		uint line_n;
	};

	Common::Array<Op> _patch;
	byte *_data;
	Op _curLine;
	bool _err;

	uint32 calcIncSize(Common::Array<Op>::const_iterator start);
	uint32 str2num(Common::String num);
	void err(const char *s);
};

} // end of namespace Grim

#endif
