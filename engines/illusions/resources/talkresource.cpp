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

#include "illusions/illusions.h"
#include "illusions/resources/talkresource.h"
#include "illusions/dictionary.h"

namespace Illusions {

// TalkResourceLoader

void TalkResourceLoader::load(Resource *resource) {
	resource->_instance = _vm->_talkItems->createTalkInstance(resource);
}

bool TalkResourceLoader::isFlag(int flag) {
	return
		flag == kRlfLoadFile;
}

// TalkEntry

void TalkEntry::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_talkId = stream.readUint32LE();
	stream.readUint32LE(); // Skip unknown
	uint32 textOffs = stream.readUint32LE();
	uint32 tblOffs = stream.readUint32LE();
	uint32 voiceNameOffs = stream.readUint32LE();
	_text = (uint16 *)(dataStart + textOffs);
	_tblPtr = dataStart + tblOffs;
	_voiceName = dataStart + voiceNameOffs;
	debug(0, "TalkEntry::load() _talkId: %08X; textOffs: %08X; tblOffs: %08X; voiceNameOffs: %08X",
		_talkId, textOffs, tblOffs, voiceNameOffs);

#if defined(SCUMM_BIG_ENDIAN)
	for (byte *ptr = (byte *)_text; ptr != _tblPtr; ptr += 2) {
		WRITE_UINT16(ptr, SWAP_BYTES_16(READ_UINT16(ptr)));
	}
#endif

}

// TalkResource

TalkResource::TalkResource()
	: _talkEntriesCount(0), _talkEntries(0) {
}

TalkResource::~TalkResource() {
	delete[] _talkEntries;
}

void TalkResource::load(byte *data, uint32 dataSize) {
	Common::MemoryReadStream stream(data, dataSize, DisposeAfterUse::NO);
	stream.skip(4); // Skip size
	_talkEntriesCount = stream.readUint16LE();
	stream.skip(2); // Skip padding
	_talkEntries = new TalkEntry[_talkEntriesCount];
	for (uint i = 0; i < _talkEntriesCount; ++i) {
		stream.seek(8 + i * 0x14);
		_talkEntries[i].load(data, stream);
	}
}

// TalkInstance

TalkInstance::TalkInstance(IllusionsEngine *vm)
	: _vm(vm), _pauseCtr(0) {
}

void TalkInstance::load(Resource *resource) {
	TalkResource *talkResource = new TalkResource();
	talkResource->load(resource->_data, resource->_dataSize);
	_talkRes = talkResource;
	_talkId = resource->_resId;
	_sceneId = resource->_sceneId;
	registerResources();
}

void TalkInstance::unload() {
	unregisterResources();
	_vm->_talkItems->removeTalkInstance(this);
	delete _talkRes;
}

void TalkInstance::pause() {
	++_pauseCtr;
	if (_pauseCtr == 1)
		unregisterResources();
}

void TalkInstance::unpause() {
	--_pauseCtr;
	if (_pauseCtr == 0)
		registerResources();
}

void TalkInstance::registerResources() {
	for (uint i = 0; i < _talkRes->_talkEntriesCount; ++i) {
		TalkEntry *talkEntry = &_talkRes->_talkEntries[i];
		_vm->_dict->addTalkEntry(talkEntry->_talkId, talkEntry);
	}
}

void TalkInstance::unregisterResources() {
	for (uint i = 0; i < _talkRes->_talkEntriesCount; ++i) {
		TalkEntry *talkEntry = &_talkRes->_talkEntries[i];
		_vm->_dict->removeTalkEntry(talkEntry->_talkId);
	}
}

// TalkInstanceList

TalkInstanceList::TalkInstanceList(IllusionsEngine *vm)
	: _vm(vm) {
}

TalkInstanceList::~TalkInstanceList() {
}

TalkInstance *TalkInstanceList::createTalkInstance(Resource *resource) {
	TalkInstance *talkInstance = new TalkInstance(_vm);
	talkInstance->load(resource);
	_items.push_back(talkInstance);
	return talkInstance;
}

void TalkInstanceList::removeTalkInstance(TalkInstance *talkInstance) {
	_items.remove(talkInstance);
}

TalkInstance *TalkInstanceList::findTalkItem(uint32 talkId) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it) {
		if ((*it)->_talkId == talkId)
			return (*it);
	}
	return 0;
}

TalkInstance *TalkInstanceList::findTalkItemBySceneId(uint32 sceneId) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it) {
		if ((*it)->_sceneId == sceneId)
			return (*it);
	}
	return 0;
}

void TalkInstanceList::pauseBySceneId(uint32 sceneId) {
	TalkInstance *talkInstance = findTalkItemBySceneId(sceneId);
	if (talkInstance)
		talkInstance->pause();
}

void TalkInstanceList::unpauseBySceneId(uint32 sceneId) {
	TalkInstance *talkInstance = findTalkItemBySceneId(sceneId);
	if (talkInstance)
		talkInstance->unpause();
}

} // End of namespace Illusions
