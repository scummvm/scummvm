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
class Expression;

class Script {
public:
	Script(GobEngine *vm);
	~Script();

	/** Read data and move the pointer accordingly. */
	uint32 read(byte *data, int32 size);
	/** Read data (from an optional offset) without moving the pointer. */
	uint32 peek(byte *data, int32 size, int32 offset = 0) const;

	// Stream properties
	int32 pos() const;
	int32 getSize() const;

	// Stream seeking
	bool seek(int32 offset, int whence = SEEK_SET);
	bool skip(int32 offset);

	// Reading data
	byte   readByte  ();
	char   readChar  ();
	uint8  readUint8 ();
	uint16 readUint16();
	uint32 readUint32();
	int8   readInt8  ();
	int16  readInt16 ();
	int32  readInt32 ();
	char  *readString(int32 length = -1);

	// Peeking data
	byte   peekByte  (int32 offset = 0);
	char   peekChar  (int32 offset = 0);
	uint8  peekUint8 (int32 offset = 0);
	uint16 peekUint16(int32 offset = 0);
	uint32 peekUint32(int32 offset = 0);
	int8   peekInt8  (int32 offset = 0);
	int16  peekInt16 (int32 offset = 0);
	int32  peekInt32 (int32 offset = 0);
	char  *peekString(int32 offset = 0);

	// Expression parsing functions
	int16 readVarIndex(uint16 *size = 0, uint16 *type = 0);
	int16 readValExpr(byte stopToken = 99);
	int16 readExpr(byte stopToken, byte *type);
	void  skipExpr(char stopToken);

	// Higher-level expression parsing functions
	char evalExpr(int16 *pRes);
	bool evalBoolResult();

	// Accessing the result of expressions
	int32 getResultInt() const;
	char *getResultStr() const;

	/** Returns the offset the specified pointer is within the script data. */
	int32 getOffset(byte *ptr) const;
	/** Returns the data pointer to the offset. */
	byte *getData(int32 offset) const;

	/** Returns the raw data pointer. */
	byte *getData();

	/** Load a script file. */
	bool load(const char *fileName);
	/** Unload the script. */
	void unload();
	/** Was a script loaded? */
	bool isLoaded() const;

	/** Setting the 'finished' property. */
	void setFinished(bool finished);
	/** Querying the 'finished' property. */
	bool isFinished() const;

	// Call stack operations
	/** Push the current script position onto the call stack. */
	void push();
	/** Pop a script position from the call stack (and return there). */
	void pop(bool ret = true);
	/** Push the current script position and branch to the specified offset. */
	void call(uint32 offset);

	// Fixed properties
	uint8  getVersionMajor   () const;
	uint8  getVersionMinor   () const;
	uint32 getVariablesCount () const;
	uint32 getTextsOffset    () const;
	uint32 getResourcesOffset() const;
	uint16 getAnimDataSize   () const;
	uint8  getImFileNumber   () const;
	uint8  getExFileNumber   () const;
	uint8  getCommunHandling () const;
	uint32 getStartOffset    () const;
	uint32 getCenterOffset   () const;

	static uint32 getVariablesCount(const char *fileName, GobEngine *vm);

private:
	struct CallEntry {
		byte *totPtr;
		bool finished;
	};

	GobEngine *_vm;
	Expression *_expression;

	bool _finished;

	Common::String _totFile;
	byte *_totData;
	byte *_totPtr;
	uint32 _totSize;

	int16 _lomHandle;

	uint8  _versionMajor;
	uint8  _versionMinor;
	uint32 _variablesCount;
	uint32 _textsOffset;
	uint32 _resourcesOffset;
	uint16 _animDataSize;
	uint8  _imFileNumber;
	uint8  _exFileNumber;
	uint8  _communHandling;
	uint32 _startOffset;
	uint16 _centerOffset;

	Common::Stack<CallEntry> _callStack;

	/** Loading a TOT file. */
	bool loadTOT(const Common::String &fileName);
	/** Loading a LOM file. */
	bool loadLOM(const Common::String &fileName);

	bool getTOTProperties();

	/** Unloading a TOT file. */
	void unloadTOT();
};

} // End of namespace Gob

#endif // GOB_SCRIPT_H
