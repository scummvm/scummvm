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
#include "illusions/talkresource.h"
#include "illusions/dictionary.h"

namespace Illusions {

// TalkResourceLoader

void TalkResourceLoader::load(Resource *resource) {
	debug("TalkResourceLoader::load() Loading text %08X from %s...", resource->_resId, resource->_filename.c_str());

	TalkResource *talkResource = new TalkResource();
	talkResource->load(resource->_data, resource->_dataSize);
	resource->_refId = talkResource;

	TalkItem *talkItem = _vm->_talkItems->newTalkItem(resource->_resId, resource->_tag, talkResource);
	talkItem->registerResources();
}

void TalkResourceLoader::unload(Resource *resource) {
	TalkItem *talkItem = _vm->_talkItems->findTalkItem(resource->_resId);
	talkItem->unregisterResources();
	_vm->_talkItems->freeTalkItem(talkItem);
}

void TalkResourceLoader::buildFilename(Resource *resource) {
	resource->_filename = Common::String::format("%08X.tlk", resource->_resId);
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
	_text = dataStart + textOffs;
	_tblPtr = dataStart + tblOffs;
	_voiceName = dataStart + voiceNameOffs;
	
	debug(0, "TalkEntry::load() _talkId: %08X; textOffs: %08X; tblOffs: %08X; voiceNameOffs: %08X",
		_talkId, textOffs, tblOffs, voiceNameOffs);
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

// TalkItem

TalkItem::TalkItem(IllusionsEngine *vm, uint32 talkId, uint32 tag, TalkResource *talkResource)
	: _vm(vm), _talkId(talkId), _tag(tag), _talkRes(talkResource), _pauseCtr(0) {
}

TalkItem::~TalkItem() {
}

void TalkItem::registerResources() {
	for (uint i = 0; i < _talkRes->_talkEntriesCount; ++i) {
		TalkEntry *talkEntry = &_talkRes->_talkEntries[i];
		_vm->_dict->addTalkEntry(talkEntry->_talkId, talkEntry);
	}
}

void TalkItem::unregisterResources() {
	for (uint i = 0; i < _talkRes->_talkEntriesCount; ++i) {
		TalkEntry *talkEntry = &_talkRes->_talkEntries[i];
		_vm->_dict->removeTalkEntry(talkEntry->_talkId);
	}
}

void TalkItem::pause() {
	++_pauseCtr;
	if (_pauseCtr == 1)
		unregisterResources();
}

void TalkItem::unpause() {
	--_pauseCtr;
	if (_pauseCtr == 0)
		registerResources();
}

// TalkItems

TalkItems::TalkItems(IllusionsEngine *vm)
	: _vm(vm) {
}

TalkItems::~TalkItems() {
}

TalkItem *TalkItems::newTalkItem(uint32 talkId, uint32 tag, TalkResource *talkResource) {
	TalkItem *talkItem = new TalkItem(_vm, talkId, tag, talkResource);
	_items.push_back(talkItem);
	return talkItem;
}

void TalkItems::freeTalkItem(TalkItem *talkItem) {
	_items.remove(talkItem);
	delete talkItem;
}

TalkItem *TalkItems::findTalkItem(uint32 talkId) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it)
		if ((*it)->_talkId == talkId)
			return (*it);
	return 0;
}

TalkItem *TalkItems::findTalkItemByTag(uint32 tag) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it)
		if ((*it)->_tag == tag)
			return (*it);
	return 0;
}

void TalkItems::pauseByTag(uint32 tag) {
	TalkItem *talkItem = findTalkItemByTag(tag);
	if (talkItem)
		talkItem->pause();
}

void TalkItems::unpauseByTag(uint32 tag) {
	TalkItem *talkItem = findTalkItemByTag(tag);
	if (talkItem)
		talkItem->unpause();
}

} // End of namespace Illusions
