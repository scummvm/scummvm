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

#ifndef GOB_SCRIPT_H
#define GOB_SCRIPT_H

#include "common/str.h"
#include "common/stack.h"

namespace Gob {

class GobEngine;
class Parse;

class Script {
public:
	Script(GobEngine *vm);
	~Script();

	uint32 read(byte *data, uint32 size);
	uint32 peek(byte *data, uint32 size, int32 offset = 0);

	byte   readByte();
	char   readChar();
	uint8  readUint8();
	uint16 readUint16();
	uint32 readUint32();
	int8   readInt8();
	int16  readInt16();
	int32  readInt32();

	char *readString(int32 length = -1);

	byte   peekByte(int32 offset = 0);
	char   peekChar(int32 offset = 0);
	uint8  peekUint8(int32 offset = 0);
	uint16 peekUint16(int32 offset = 0);
	uint32 peekUint32(int32 offset = 0);
	int8   peekInt8(int32 offset = 0);
	int16  peekInt16(int32 offset = 0);
	int32  peekInt32(int32 offset = 0);

	char *peekString(int32 offset = 0);

	int16 readVarIndex(uint16 *size = 0, uint16 *type = 0);
	int16 readValExpr(byte stopToken = 99);
	int16 readExpr(byte stopToken, byte *type);
	void skipExpr(char stopToken);

	char evalExpr(int16 *pRes);
	bool evalBoolResult();

	int32 getResultInt();
	char *getResultStr();

	int32 pos() const;
	int32 getSize() const;
	bool seek(int32 offset, int whence = SEEK_SET);
	bool skip(uint32 offset);

	uint32 getOffset(byte *ptr);
	byte *getData();

	bool load(const char *fileName);

	void unload();

	bool isLoaded() const;

	void setFinished(bool finished);
	bool isFinished() const;

	void push();
	void pop(bool ret = true);
	void call(uint32 offset);

/*	byte *loadExtData(int16 dataId, int16 *pResWidth, int16 *pResHeight, uint32 *dataSize = 0);
	byte *loadTotResource(int16 id, int16 *dataSize = 0, int16 *width = 0, int16 *height = 0);

	byte *loadLocTexts(int32 *dataSize = 0);
	int32 loadTotFile(const char *path);
	void loadExtTable(void);
	void loadImFile(void);

	int16 openLocTextFile(char *locTextFile, int language);*/

private:
	struct CallEntry {
		byte *totPtr;
		bool finished;
	};

	GobEngine *_vm;
	Parse *_parser;

	bool _finished;

	Common::String _totFile;

	byte *_totData;

	uint32 _totSize;

	byte *_totPtr;

	int16 _lomHandle;

	Common::Stack<CallEntry> _callStack;

	bool loadTOT(const Common::String &fileName);
	bool loadLOM(const Common::String &fileName);

	void unloadTOT();
};

} // End of namespace Gob

#endif // GOB_SCRIPT_H
