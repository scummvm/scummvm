/* ResidualVMVM - A 3D game interpreter
 *
 * ResidualVMVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/array.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "common/tokenizer.h"
#include "common/md5.h"
#include "common/file.h"

#include "engines/grim/patchr.h"
#include "engines/grim/debug.h"

namespace Grim {

const char *Patchr::InstructionS[8]  = {"BEGIN", "END", "REPLACE", "INSERT", "DELETE", "FILL", "COPY", NULL};

void Patchr::loadPatch(Common::SeekableReadStream *patchStream) {
	Common::String line, token;
	struct Op op;
	uint line_n = 0;

	while (!patchStream->eos()) {
		line = patchStream->readLine();
		op.line_n = ++line_n;
		line.trim();
		Common::StringTokenizer patchTokens(line);

		if (line.empty() || line[0] == '#')
			continue;

		//Extract the instruction
		token = patchTokens.nextToken();
		op.ist = INVALID;
		for (uint i = 0; InstructionS[i] != NULL; ++i)
			if (token.equalsIgnoreCase(InstructionS[i]))
				op.ist = Instruction(i);

		//Extract the arguments
		op.args.clear();
		while (!patchTokens.empty()) {
			token = patchTokens.nextToken();
			if (token.empty())
				continue;
			if (token[0] == '#')
				break;
			op.args.push_back(token);
		}
		_patch.push_back(op);
	}
	delete patchStream;
}

uint32 Patchr::calcIncSize(Common::Array<Op>::const_iterator start) {
	uint32 incSize = 0;
	for (Common::Array<Op>::const_iterator i = start; i->ist != END && i != _patch.end(); ++i)
		if (i->ist == INSERT)
			incSize += str2num(i->args[1]);

	return incSize;
}

bool Patchr::patchFile(Common::SeekableReadStream *&file, const Common::String &name) {
	Common::Array<Op>::const_iterator line;
	Common::String md5;
	uint32 maxSize, fileSize;
	uint32 offset, offset2, size;
	byte fill;

	//Compute the MD5 of the original file
	md5 = computeStreamMD5AsString(*file, _kMd5size);
	file->seek(0, SEEK_SET);

	//Search a BEGIN statement with the right md5
	for (line = _patch.begin(); line != _patch.end(); ++line)
		if (line->ist == BEGIN)
			if (md5.equalsIgnoreCase(line->args[0])) {
				++line;
				break;
			}

	if (line == _patch.end()) {
		Debug::warning(Debug::Patchr, "No suitable patch for %s", name.c_str());
		return false;
	}

	//Calc the maximum size of resulting file and read it
	fileSize = file->size();
	maxSize = fileSize + calcIncSize(line);
	if (_err)
		return false;
	if (maxSize > _kMaxFileSize) {
		Debug::warning(Debug::Patchr, "Requested patch makes the resulting file too big (> %u bytes)", _kMaxFileSize);
		return false;
	}

	_data = (byte*)malloc(maxSize);
	file->read(_data, fileSize);

	//Patch it!!
	while (line->ist != END && line != _patch.end()) {
		_curLine = *line;
		switch (line->ist) {
			case REPLACE:
				if (line->args.size() < 2) {
					err("Too few arguments");
					return false;
				}
				offset = str2num(line->args[0]);
				size = line->args.size() - 1;
				if (offset + size > fileSize) {
					err("out of bounds");
					return false;
				}
				for (uint32 i = 0; i < size; ++i)
					_data[offset + i] = byte(str2num(line->args[i + 1]));
				break;

			case INSERT:
				if (line->args.size() < 2) {
					err("Too few arguments");
					return false;
				}
				offset = str2num(line->args[0]);
				size = str2num(line->args[1]);
				assert(fileSize + size <= maxSize);
				if (offset > fileSize) {
					err("out of bounds");
					return false;
				}
				memmove(_data + offset + size, _data + offset, fileSize - offset);
				fileSize += size;
				break;

			case DELETE:
				if (line->args.size() < 2) {
					err("Too few arguments");
					return false;
				}
				offset = str2num(line->args[0]);
				size = str2num(line->args[1]);
				if (offset + size > fileSize) {
					err("out of bounds");
					return false;
				}
				memmove(_data + offset, _data + offset + size, fileSize - (offset + size));
				fileSize -= size;
				break;

			case FILL:
				if (line->args.size() < 3) {
					err("Too few arguments");
					return false;
				}
				offset = str2num(line->args[0]);
				size = str2num(line->args[1]);
				fill = byte(str2num(line->args[2]));
				if (offset + size > fileSize) {
					err("out of bounds");
					return false;
				}
				memset(_data + offset, fill, size);
				break;

			case COPY:
				if (line->args.size() < 3) {
					err("Too few arguments");
					return false;
				}
				offset = str2num(line->args[0]);
				size = str2num(line->args[1]);
				offset2 = str2num(line->args[2]);
				if (offset + size > fileSize || offset2 + size > fileSize) {
					err("out of bounds");
					return false;
				}
				memmove(_data + offset, _data + offset2, size);
				break;

			case BEGIN:
				err("misplaced instruction. Instructions block not closed by an END.");
				break;

			case INVALID:
				Debug::warning(Debug::Patchr, "Invalid instruction at line %u", line->line_n);
				_err = true;
				break;

			default:
				Debug::error(Debug::Patchr, "Patchr: Internal error!");
				break;
		}
		if (_err)
			return false;
		++line;
	}

	//During debug, dump the patched file
	if (Debug::isChannelEnabled(Debug::Patchr)) {
		Common::DumpFile dump;
		if (dump.open(name)) {
			dump.write(_data, fileSize);
			dump.close();
		} else
			Debug::warning(Debug::Patchr,"Couldn't open file '%s' for writing", name.c_str());
	}

	//If the patch has correctly applied, return the updated file
	delete file;
	file = new Common::MemoryReadStream(_data, fileSize, DisposeAfterUse::YES);
	_data = NULL;
	return true;
}

uint32 Patchr::str2num(Common::String num) {
	char *errpos;
	uint32 val;
	val = strtoul(num.c_str(), &errpos, 16);
	if (num.c_str() == errpos)
		err("Invalid number");
	return val;
}

void Patchr::err(const char *s) {
	Debug::warning(Debug::Patchr, "%s at line %u: %s", InstructionS[_curLine.ist], _curLine.line_n, s);
	_err = true;
}

} // end of namespace Grim
