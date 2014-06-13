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

#include "neverhood/staticdata.h"

namespace Neverhood {

StaticData::StaticData() {
}

StaticData::~StaticData() {
	for (Common::HashMap<uint32, HitRectList*>::iterator i = _hitRectLists.begin(); i != _hitRectLists.end(); ++i)
		delete i->_value;
	for (Common::HashMap<uint32, RectList*>::iterator i = _rectLists.begin(); i != _rectLists.end(); ++i)
		delete i->_value;
	for (Common::HashMap<uint32, MessageList*>::iterator i = _messageLists.begin(); i != _messageLists.end(); ++i)
		delete i->_value;
	for (Common::HashMap<uint32, NavigationList*>::iterator i = _navigationLists.begin(); i != _navigationLists.end(); ++i)
		delete i->_value;
	for (Common::HashMap<uint32, HallOfRecordsInfo*>::iterator i = _hallOfRecordsInfoItems.begin(); i != _hallOfRecordsInfoItems.end(); ++i)
		delete i->_value;
	for (Common::HashMap<uint32, TrackInfo*>::iterator i = _trackInfoItems.begin(); i != _trackInfoItems.end(); ++i)
		delete i->_value;
}

void StaticData::load(const char *filename) {

	Common::File fd;

	if (!fd.open(filename))
		error("StaticData::load() Could not open %s", filename);

	fd.readUint32LE(); // magic
	fd.readUint32LE(); // version

	// Load message lists
	uint32 messageListsCount = fd.readUint32LE();
	debug(3, "messageListsCount: %d", messageListsCount);
	for (uint32 i = 0; i < messageListsCount; i++) {
		MessageList *messageList = new MessageList();
		uint32 id = fd.readUint32LE();
		uint32 itemCount = fd.readUint32LE();
		for (uint32 itemIndex = 0; itemIndex < itemCount; itemIndex++) {
			MessageItem messageItem;
			messageItem.messageNum = fd.readUint16LE();
			messageItem.messageValue = fd.readUint32LE();
			messageList->push_back(messageItem);
		}

		// WORKAROUND for a problem in two of the game's message lists:
		// the message lists used when Klaymen is drinking the wrong potion
		// have as a last element the animation itself (message 0x4832).
		// However, when processMessageList() reaches the last element in a
		// message list, it allows player input, which means that the player
		// can erroneously skip these potion drinking animations. We insert
		// another message at the end of these lists to prevent player input
		// till the animations are finished
		if (id == 0x004AF0C8 || id == 0x004B5BD0) {	// wrong potion message lists
			MessageItem messageItem;
			messageItem.messageNum = 0x4004;	// set Klaymen's state to idle
			messageItem.messageValue = 0;
			messageList->push_back(messageItem);
		}

		if(_messageLists.contains(id)) {
			warning("Duplicate id %d in _messageLists - freeing older entry", id);
			delete _messageLists[id];
		}

		_messageLists[id] = messageList;
	}

	// Load rect lists
	uint32 rectListsCount = fd.readUint32LE();
	debug(3, "rectListsCount: %d", rectListsCount);
	for (uint32 i = 0; i < rectListsCount; i++) {
		RectList *rectList = new RectList();
		uint32 id = fd.readUint32LE();
		uint32 itemCount = fd.readUint32LE();
		for (uint32 itemIndex = 0; itemIndex < itemCount; itemIndex++) {
			RectItem rectItem;
			rectItem.rect.x1 = fd.readUint16LE();
			rectItem.rect.y1 = fd.readUint16LE();
			rectItem.rect.x2 = fd.readUint16LE();
			rectItem.rect.y2 = fd.readUint16LE();
			uint32 subItemCount = fd.readUint32LE();
			rectItem.subRects.reserve(subItemCount);
			for (uint32 subItemIndex = 0; subItemIndex < subItemCount; subItemIndex++) {
				SubRectItem subRectItem;
				subRectItem.rect.x1 = fd.readUint16LE();
				subRectItem.rect.y1 = fd.readUint16LE();
				subRectItem.rect.x2 = fd.readUint16LE();
				subRectItem.rect.y2 = fd.readUint16LE();
				subRectItem.messageListId = fd.readUint32LE();
				rectItem.subRects.push_back(subRectItem);
			}
			rectList->push_back(rectItem);
		}

		if(_rectLists.contains(id)) {
			warning("Duplicate id %d in _rectLists - freeing older entry", id);
			delete _rectLists[id];
		}

		_rectLists[id] = rectList;
	}

	// Load hit rects
	uint32 hitRectListsCount = fd.readUint32LE();
	debug(3, "hitRectListsCount: %d", hitRectListsCount);
	for (uint32 i = 0; i < hitRectListsCount; i++) {
		HitRectList *hitRectList = new HitRectList();
		uint32 id = fd.readUint32LE();
		uint32 itemCount = fd.readUint32LE();
		for (uint32 itemIndex = 0; itemIndex < itemCount; itemIndex++) {
			HitRect hitRect;
			hitRect.rect.x1 = fd.readUint16LE();
			hitRect.rect.y1 = fd.readUint16LE();
			hitRect.rect.x2 = fd.readUint16LE();
			hitRect.rect.y2 = fd.readUint16LE();
			hitRect.type = fd.readUint16LE();
			hitRectList->push_back(hitRect);
		}

		if(_hitRectLists.contains(id)) {
			warning("Duplicate id %d in _hitRectLists - freeing older entry", id);
			delete _hitRectLists[id];
		}

		_hitRectLists[id] = hitRectList;
	}

	// Load navigation lists
	uint32 navigationListsCount = fd.readUint32LE();
	debug(3, "navigationListsCount: %d", navigationListsCount);
	for (uint32 i = 0; i < navigationListsCount; i++) {
		NavigationList *navigationList = new NavigationList();
		uint32 id = fd.readUint32LE();
		uint32 itemCount = fd.readUint32LE();
		for (uint32 itemIndex = 0; itemIndex < itemCount; itemIndex++) {
			NavigationItem navigationItem;
			navigationItem.fileHash = fd.readUint32LE();
			navigationItem.leftSmackerFileHash = fd.readUint32LE();
			navigationItem.rightSmackerFileHash = fd.readUint32LE();
			navigationItem.middleSmackerFileHash = fd.readUint32LE();
			navigationItem.interactive = fd.readByte();
			navigationItem.middleFlag = fd.readByte();
			navigationItem.mouseCursorFileHash = fd.readUint32LE();
			navigationList->push_back(navigationItem);
		}

		if(_navigationLists.contains(id)) {
			warning("Duplicate id %d in _navigationLists - freeing older entry", id);
			delete _navigationLists[id];
		}

		_navigationLists[id] = navigationList;
	}

	// Load HallOfRecordsInfo items
	uint32 hallOfRecordsInfoItemsCount = fd.readUint32LE();
	debug(3, "hallOfRecordsInfoItemsCount: %d", hallOfRecordsInfoItemsCount);
	for (uint32 i = 0; i < hallOfRecordsInfoItemsCount; i++) {
		HallOfRecordsInfo *hallOfRecordsInfo = new HallOfRecordsInfo();
		uint32 id = fd.readUint32LE();
		hallOfRecordsInfo->bgFilename1 = fd.readUint32LE();
		hallOfRecordsInfo->bgFilename2 = fd.readUint32LE();
		hallOfRecordsInfo->txFilename = fd.readUint32LE();
		hallOfRecordsInfo->bgFilename3 = fd.readUint32LE();
		hallOfRecordsInfo->xPosIndex = fd.readByte();
		hallOfRecordsInfo->count = fd.readByte();

		if(_hallOfRecordsInfoItems.contains(id)) {
			warning("Duplicate id %d in _hallOfRecordsInfoItems - freeing older entry", id);
			delete _hallOfRecordsInfoItems[id];
		}

		_hallOfRecordsInfoItems[id] = hallOfRecordsInfo;
	}

	// Load TrackInfo items
	uint32 trackInfoItemsCount = fd.readUint32LE();
	debug(3, "trackInfoItemsCount: %d", trackInfoItemsCount);
	for (uint32 i = 0; i < trackInfoItemsCount; i++) {
		TrackInfo *trackInfo = new TrackInfo();
		uint32 id = fd.readUint32LE();
		trackInfo->bgFilename = fd.readUint32LE();
		trackInfo->bgShadowFilename = fd.readUint32LE();
		trackInfo->dataResourceFilename = fd.readUint32LE();
		trackInfo->trackPointsName = fd.readUint32LE();
		trackInfo->rectListName = fd.readUint32LE();
		trackInfo->exPaletteFilename2 = fd.readUint32LE();
		trackInfo->exPaletteFilename1 = fd.readUint32LE();
		trackInfo->mouseCursorFilename = fd.readUint32LE();
		trackInfo->which1 = fd.readUint16LE();
		trackInfo->which2 = fd.readUint16LE();

		if(_trackInfoItems.contains(id)) {
			warning("Duplicate id %d in _trackInfoItems - freeing older entry", id);
			delete _trackInfoItems[id];
		}

		_trackInfoItems[id] = trackInfo;
	}

}

HitRectList *StaticData::getHitRectList(uint32 id) {
	if (!_hitRectLists[id])
		error("StaticData::getHitRectList() HitRectList with id %08X not found", id);
	return _hitRectLists[id];
}

RectList *StaticData::getRectList(uint32 id) {
	if (!_rectLists[id])
		error("StaticData::getRectList() RectList with id %08X not found", id);
	return _rectLists[id];
}

MessageList *StaticData::getMessageList(uint32 id) {
	if (!_messageLists[id])
		error("StaticData::getMessageList() MessageList with id %08X not found", id);
	return _messageLists[id];
}

NavigationList *StaticData::getNavigationList(uint32 id) {
	if (!_navigationLists[id])
		error("StaticData::getNavigationList() NavigationList with id %08X not found", id);
	return _navigationLists[id];
}

HallOfRecordsInfo *StaticData::getHallOfRecordsInfoItem(uint32 id) {
	if (!_hallOfRecordsInfoItems[id])
		error("StaticData::getHallOfRecordsInfoItem() HallOfRecordsInfo with id %08X not found", id);
	return _hallOfRecordsInfoItems[id];
}

TrackInfo *StaticData::getTrackInfo(uint32 id) {
	if (!_trackInfoItems[id])
		error("StaticData::getTrackInfo() TrackInfo with id %08X not found", id);
	return _trackInfoItems[id];
}

} // End of namespace Neverhood
