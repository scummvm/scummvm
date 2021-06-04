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

#include "common/scummsys.h"
#include "common/system.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/translation.h"
#include "common/substream.h"
#include "gui/saveload.h"

#include "trecision/trecision.h"
#include "trecision/anim.h"
#include "trecision/actor.h"
#include "trecision/3d.h"
#include "trecision/defines.h"
#include "trecision/dialog.h"
#include "trecision/graphics.h"
#include "trecision/video.h"
#include "trecision/sound.h"

namespace Trecision {

Common::SeekableReadStreamEndian *TrecisionEngine::readEndian(Common::SeekableReadStream *stream, DisposeAfterUse::Flag dispose) {
	return new Common::SeekableSubReadStreamEndian(stream, 0, stream->size(), isAmiga(), dispose);
}

void TrecisionEngine::loadAll() {
	Common::File dataNl;

	if (!dataNl.open("DATA.NL"))
		error("loadAll : Couldn't open DATA.NL");
	Common::SeekableReadStreamEndian *data = readEndian(&dataNl, DisposeAfterUse::NO);

	for (int i = 0; i < MAXROOMS; ++i)
		_room[i].loadRoom(data);

	for (int i = 0; i < MAXOBJ; ++i)
		_obj[i].loadObj(data);

	for (int i = 0; i < MAXINVENTORY; ++i)
		_inventoryObj[i].loadObj(data);

	_soundMgr->loadSamples(data);

	// TODO: Unused Amiga data?
	if (isAmiga()) {
		data->skip(12 * 12 + 1);
	}

	for (int i = 0; i < MAXSCRIPTFRAME; ++i) {
		_scriptFrame[i]._class = data->readByte();
		_scriptFrame[i]._event = data->readByte();
		_scriptFrame[i]._u8Param = data->readByte();
		data->readByte(); // Padding
		_scriptFrame[i]._u16Param1 = data->readUint16();
		_scriptFrame[i]._u16Param2 = data->readUint16();
		_scriptFrame[i]._u32Param = data->readUint16();
		_scriptFrame[i]._noWait = !(data->readSint16() == 0);
	}

	for (int i = 0; i < MAXSCRIPT; ++i) {
		_scriptFirstFrame[i] = data->readUint16();
		data->readByte(); // unused field
		data->readByte(); // Padding
	}

	_animMgr->loadAnimTab(data);
	_dialogMgr->loadData(data);

	data->skip(620);	// actions (unused)

	int numFileRef = data->readSint32();
	data->skip(numFileRef * (12 + 4));	// fileRef name + offset

	_textArea = new char[MAXTEXTAREA];
	data->read(_textArea, MAXTEXTAREA);

	_textPtr = _textArea;

	for (int a = 0; a < MAXOBJNAME; a++)
		_objName[a] = getNextSentence();

	for (int a = 0; a < MAXSENTENCE; a++)
		_sentence[a] = getNextSentence();

	for (int a = 0; a < MAXSYSTEXT; a++)
		_sysText[a] = getNextSentence();

	delete data;
	dataNl.close();
}

byte *TrecisionEngine::readData(const Common::String &fileName) {
	Common::SeekableReadStream *stream = _dataFile.createReadStreamForMember(fileName);
	if (stream == nullptr)
		error("readData(): File %s not found", fileName.c_str());

	byte *buf = new byte[stream->size()];
	stream->read(buf, stream->size());
	delete stream;

	return buf;
}

void TrecisionEngine::read3D(const Common::String &filename) {
	Common::SeekableReadStreamEndian *ff = readEndian(_dataFile.createReadStreamForMember(filename));
	if (ff == nullptr)
		error("read3D: Can't open 3D file %s", filename.c_str());

	_actor->read3D(ff);
	_pathFind->read3D(ff);

	delete ff;

	_cx = 320;
	_cy = 240;

	_pathFind->initSortPan();

	_renderer->init3DRoom();
	_renderer->setClipping(0, TOP, MAXX, AREA + TOP);
}

void TrecisionEngine::readObject(Common::SeekableReadStream *stream, uint16 objIndex, uint16 objectId) {
	SObject *obj = &_obj[objectId];

	if (obj->isModeFull()) {
		obj->readRect(stream);

		uint32 size = obj->_rect.width() * obj->_rect.height();
		delete[] _objPointers[objIndex];
		_objPointers[objIndex] = new uint16[size];
		for (uint32 i = 0; i < size; ++i)
			_objPointers[objIndex][i] = stream->readUint16LE();

		_graphicsMgr->updatePixelFormat(_objPointers[objIndex], size);
	}

	if (obj->isModeMask()) {
		obj->readRect(stream);

		uint32 size = stream->readUint32LE();
		delete[] _objPointers[objIndex];
		_objPointers[objIndex] = new uint16[size];
		for (uint32 i = 0; i < size; ++i)
			_objPointers[objIndex][i] = stream->readUint16LE();

		_graphicsMgr->updatePixelFormat(_objPointers[objIndex], size);

		size = stream->readUint32LE();
		delete[] _maskPointers[objIndex];
		_maskPointers[objIndex] = new uint8[size];
		for (uint32 i = 0; i < size; ++i)
			_maskPointers[objIndex][i] = (uint8)stream->readByte();
	}

	refreshObject(objectId);
}

void TrecisionEngine::readObj(Common::SeekableReadStream *stream) {
	if (!_room[_curRoom]._object[0])
		return;

	for (uint16 objIndex = 0; objIndex < MAXOBJINROOM; objIndex++) {
		const uint16 objectId = _room[_curRoom]._object[objIndex];
		if (!objectId)
			break;

		if (_curRoom == kRoom41D && objIndex == PATCHOBJ_ROOM41D)
			break;

		if (_curRoom == kRoom2C && objIndex == PATCHOBJ_ROOM2C)
			break;

		readObject(stream, objIndex, objectId);
	}
}

void TrecisionEngine::readExtraObj2C() {
	if (!_room[_curRoom]._object[32])
		return;

	Common::SeekableReadStream *ff = _dataFile.createReadStreamForMember("2c2.bm");

	for (uint16 objIndex = PATCHOBJ_ROOM2C; objIndex < MAXOBJINROOM; objIndex++) {
		const uint16 objectId = _room[_curRoom]._object[objIndex];
		if (!objectId)
			break;

		readObject(ff, objIndex, objectId);
	}

	delete ff;
}

void TrecisionEngine::readExtraObj41D() {
	if (!_room[_curRoom]._object[32])
		return;

	Common::SeekableReadStream *ff = _dataFile.createReadStreamForMember("41d2.bm");
	for (uint16 objIndex = PATCHOBJ_ROOM41D; objIndex < MAXOBJINROOM; objIndex++) {
		const uint16 objectId = _room[_curRoom]._object[objIndex];
		if (!objectId)
			break;

		readObject(ff, objIndex, objectId);
	}
	delete ff;
}

} // End of namespace Trecision
