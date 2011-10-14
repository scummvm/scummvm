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

#include "neverhood/staticdata.h"

namespace Neverhood {

StaticData::StaticData() {
}

StaticData::~StaticData() {
}

void StaticData::load(const char *filename) {

	Common::File fd;
	
	if (!fd.open(filename))
		error("StaticData::load() Could not open %s", filename);
		
	fd.readUint32LE(); // magic		
	fd.readUint32LE(); // version
	
	// Load message lists
	uint32 messageListsCount = fd.readUint32LE();
	debug("messageListsCount: %d", messageListsCount);
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
		_messageLists[id] = messageList;
	}

	// Load rect lists
	uint32 rectListsCount = fd.readUint32LE();
	debug("rectListsCount: %d", rectListsCount);
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
		_rectLists[id] = rectList;
	}
	
	// Load hit rects
	uint32 hitRectListsCount = fd.readUint32LE();
	debug("hitRectListsCount: %d", hitRectListsCount);
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
		_hitRectLists[id] = hitRectList;
	}

	// Load navigation lists
	uint32 navigationListsCount = fd.readUint32LE();
	debug("navigationListsCount: %d", navigationListsCount);
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
		_navigationLists[id] = navigationList;
	}

	// Load SceneInfo140 items
	uint32 sceneInfo140ItemsCount = fd.readUint32LE();
	debug("sceneInfo140ItemsCount: %d", sceneInfo140ItemsCount);
	for (uint32 i = 0; i < sceneInfo140ItemsCount; i++) {
		SceneInfo140 *sceneInfo140 = new SceneInfo140();
		uint32 id = fd.readUint32LE();
		sceneInfo140->bgFilename1 = fd.readUint32LE();
		sceneInfo140->bgFilename2 = fd.readUint32LE();
		sceneInfo140->txFilename = fd.readUint32LE();
		sceneInfo140->bgFilename3 = fd.readUint32LE();
		sceneInfo140->xPosIndex = fd.readByte();
		sceneInfo140->count = fd.readByte();
		_sceneInfo140Items[id] = sceneInfo140;
	}

	// Load SceneInfo2700 items
	uint32 sceneInfo2700ItemsCount = fd.readUint32LE();
	debug("sceneInfo2700ItemsCount: %d", sceneInfo2700ItemsCount);
	for (uint32 i = 0; i < sceneInfo2700ItemsCount; i++) {
		SceneInfo2700 *sceneInfo2700 = new SceneInfo2700();
		uint32 id = fd.readUint32LE();
		sceneInfo2700->bgFilename = fd.readUint32LE();
		sceneInfo2700->class437Filename = fd.readUint32LE();
		sceneInfo2700->dataResourceFilename = fd.readUint32LE();
		sceneInfo2700->pointListName = fd.readUint32LE();
		sceneInfo2700->rectListName = fd.readUint32LE();
		sceneInfo2700->exPaletteFilename2 = fd.readUint32LE();
		sceneInfo2700->exPaletteFilename1 = fd.readUint32LE();
		sceneInfo2700->mouseCursorFilename = fd.readUint32LE();
		sceneInfo2700->which1 = fd.readUint16LE();
		sceneInfo2700->which2 = fd.readUint16LE();
		_sceneInfo2700Items[id] = sceneInfo2700;
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

SceneInfo140 *StaticData::getSceneInfo140Item(uint32 id) {
	if (!_sceneInfo140Items[id])
		error("StaticData::getSceneInfo140Item() SceneInfo140 with id %08X not found", id);
	return _sceneInfo140Items[id];
}

SceneInfo2700 *StaticData::getSceneInfo2700(uint32 id) {
	if (!_sceneInfo2700Items[id])
		error("StaticData::getSceneInfo2700() SceneInfo2700 with id %08X not found", id);
	return _sceneInfo2700Items[id];
}

} // End of namespace Neverhood
