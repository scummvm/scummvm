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

#include "common/util.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/script.h"
#include "gob/dataio.h"
#include "gob/parse.h"
#include "gob/videoplayer.h"

namespace Gob {

Script::Script(GobEngine *vm) : _vm(vm) {
	_parser = new Parse(vm);

	_finished = true;

	_totData = 0;

	_totSize = 0;

	_totPtr = 0;

	_lomHandle = -1;
}

Script::~Script() {
	unload();

	delete _parser;
}

uint32 Script::read(byte *data, uint32 size) {
	uint32 toRead = MIN<uint32>(size, _totSize - (_totPtr - _totData));

	memcpy(data, _totPtr, toRead);
	_totPtr += toRead;

	return toRead;
}

uint32 Script::peek(byte *data, uint32 size, int32 offset) {
	int32 totOffset = ((_totPtr + offset) - _totData);

	if (totOffset < 0)
		return 0;
	if (((uint32) totOffset) >= _totSize)
		return 0;

	uint32 toPeek = MIN<uint32>(size, _totSize - totOffset);

	memcpy(data, _totPtr + offset, toPeek);

	return toPeek;
}

int32 Script::pos() const {
	if (!_totData)
		return -1;

	return _totPtr - _totData;
}

int32 Script::getSize() const {
	if (!_totData)
		return -1;

	return _totSize;
}

bool Script::seek(int32 offset, int whence) {
	if (!_totData)
		return false;

	if (whence == SEEK_CUR)
		offset += pos();
	else if (whence == SEEK_END)
		offset += _totSize;

	if ((offset < 0) || (((uint32) offset) >= _totSize))
		return false;

	_finished = false;

	_totPtr = _totData + offset;

	return true;
}

bool Script::skip(uint32 offset) {
	return seek(offset, SEEK_CUR);
}

uint32 Script::getOffset(byte *ptr) {
	if (!_totData)
		return 0;

	return ptr - _totData;
}

byte *Script::getData() {
	return _totData;
}

byte Script::readByte() {
	byte v;

	read(&v, 1);

	return v;
}

char Script::readChar() {
	return (char) readByte();
}

uint8 Script::readUint8() {
	return (uint8) readByte();
}

uint16 Script::readUint16() {
	byte v[2];

	read(v, 2);

	return READ_LE_UINT16(v);
}

uint32 Script::readUint32() {
	byte v[4];

	read(v, 4);

	return READ_LE_UINT32(v);
}

int8 Script::readInt8() {
	return (int8) readByte();
}

int16 Script::readInt16() {
	return (int16) readUint16();
}

int32 Script::readInt32() {
	return (int32) readUint32();
}

char *Script::readString(int32 length) {
	if (length < 0) {
		length = 0;
		while (_totPtr[length++] != '\0');
	}

	char *string = (char *) _totPtr;

	_totPtr += length;

	return string;
}

byte Script::peekByte(int32 offset) {
	byte v;

	peek(&v, 1, offset);

	return v;
}

char Script::peekChar(int32 offset) {
	return (char) peekByte(offset);
}

uint8 Script::peekUint8(int32 offset) {
	return (uint8) peekByte(offset);
}

uint16 Script::peekUint16(int32 offset) {
	byte v[2];

	peek(v, 2, offset);

	return READ_LE_UINT16(v);
}

uint32 Script::peekUint32(int32 offset) {
	byte v[4];

	peek(v, 4, offset);

	return READ_LE_UINT32(v);
}

int8 Script::peekInt8(int32 offset) {
	return (int8) peekByte(offset);
}

int16 Script::peekInt16(int32 offset) {
	return (int16) peekUint16(offset);
}

int32 Script::peekInt32(int32 offset) {
	return (int32) peekUint32(offset);
}

char *Script::peekString(int32 offset) {
	return (char *) (_totPtr + offset);
}

int16 Script::readVarIndex(uint16 *size, uint16 *type) {
	return _parser->parseVarIndex(size, type);
}

int16 Script::readValExpr(byte stopToken) {
	return _parser->parseValExpr(stopToken);
}

int16 Script::readExpr(byte stopToken, byte *type) {
	return _parser->parseExpr(stopToken, type);
}

void Script::skipExpr(char stopToken) {
	_parser->skipExpr(stopToken);
}

int32 Script::getResultInt() {
	return _parser->getResultInt();
}

char *Script::getResultStr() {
	return _parser->getResultStr();
}

bool Script::load(const char *fileName) {
	unload();

	_finished = false;

	bool lom = false;

	Common::String *fileBase;

	char *dot;
	if ((dot = strrchr(fileName, '.'))) {
		fileBase = new Common::String(fileName, dot);

		if (!scumm_stricmp(dot + 1, "LOM"))
			lom = true;
	} else
		fileBase = new Common::String(fileName);

	_totFile = *fileBase + (lom ? ".lom" : ".tot");

	delete fileBase;

	if (lom) {
		if (!loadLOM(_totFile)) {
			unload();
			return false;
		}
	} else {
		if (!loadTOT(_totFile)) {
			unload();
			return false;
		}
	}

	return true;
}

bool Script::loadTOT(const Common::String &fileName) {
	if (_vm->_dataIO->existData(fileName.c_str())) {
		_totSize = _vm->_dataIO->getDataSize(_totFile.c_str());
		_totData = _vm->_dataIO->getData(_totFile.c_str());
	} else  {
		Common::MemoryReadStream *videoExtraData = _vm->_vidPlayer->getExtraData(fileName.c_str());

		if (videoExtraData) {
			warning("Loading TOT \"%s\" from video file", fileName.c_str());

			_totSize = videoExtraData->size();
			_totData = new byte[_totSize];

			videoExtraData->read(_totData, _totSize);

			delete videoExtraData;
		}
	}

	return (_totData != 0);
}

bool Script::loadLOM(const Common::String &fileName) {
	warning("Urban Stub: loadLOM %s", _totFile.c_str());

	_lomHandle = _vm->_dataIO->openData(_totFile.c_str());
	if (_lomHandle < 0)
		return false;

	DataStream *stream = _vm->_dataIO->openAsStream(_lomHandle);

	stream->seek(48);
	_totSize = stream->readUint32LE();
	stream->seek(0);

	_totData = new byte[_totSize];
	stream->read(_totData, _totSize);

	delete stream;

	return true;
}

void Script::unload() {
	unloadTOT();
}

void Script::unloadTOT() {
	if (_lomHandle >= 0)
		_vm->_dataIO->closeData(_lomHandle);

	while (!_callStack.empty())
		pop();

	delete[] _totData;

	_totData = 0;
	_totSize = 0;
	_totPtr = 0;
	_lomHandle = -1;
	_totFile.clear();

	_finished = true;
}

bool Script::isLoaded() const {
	return _totData != 0;
}

void Script::setFinished(bool finished) {
	_finished = finished;
}

bool Script::isFinished() const {
	return _finished;
}

void Script::push() {
	CallEntry currentCall;

	currentCall.totPtr = _totPtr;
	currentCall.finished = _finished;

	_callStack.push(currentCall);
}

void Script::pop(bool ret) {
	assert(!_callStack.empty());

	CallEntry lastCall = _callStack.pop();

	if (ret) {
		_totPtr = lastCall.totPtr;
		_finished = lastCall.finished;
	}
}

void Script::call(uint32 offset) {
	push();
	seek(offset);
}

} // End of namespace Gob
