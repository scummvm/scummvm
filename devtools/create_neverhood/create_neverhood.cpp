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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL


// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include <vector>
#include "create_neverhood.h"
#include "tables.h"

const int DAT_VERSION = 0;

uint32 dataSize;
byte *data;
uint32 dataStart = 0x004AE000;
uint32 fileStart = 0x000AC600;

void loadExe(const char *filename) {
	FILE *exe = fopen(filename, "rb");
	dataSize = fileSize(exe);
	data = new byte[dataSize];
	fread(data, dataSize, 1, exe);
	fclose(exe);
}

struct HitRect {
	int16 x1, y1, x2, y2;
	uint16 messageNum;
};

typedef std::vector<HitRect> HitRects;

struct SubRectItem {
	int16 x1, y1, x2, y2;
	uint32 messageListCount;
	uint32 messageListOffset;
};

typedef std::vector<SubRectItem> SubRectItems;

struct RectItem {
	int16 x1, y1, x2, y2;
	uint32 subRectListCount;
	uint32 subRectListOffset;
	SubRectItems subRectItems;
};

typedef std::vector<RectItem> RectItems;

struct MessageItem {
	uint16 messageNum;
	uint32 messageParam;
};

typedef std::vector<MessageItem> MessageItems;

struct HitRectList {
	uint32 id;	
	HitRects hitRects;
};

struct RectList {
	uint32 id;	
	RectItems rectItems;
};

struct MessageList {
	uint32 id;
	MessageItems messageItems;	
};

std::vector<HitRectList*> hitRectLists;
std::vector<RectList*> rectLists;
std::vector<MessageList*> messageLists;

byte *getData(uint32 offset) {
	return data + offset - dataStart + fileStart;
}

void addHitRect(uint32 offset) {
	HitRectList *hitRectList = new HitRectList();
	hitRectList->id = offset;
	byte *item = getData(offset);
	uint32 count = READ_LE_UINT32(item);
	//printf("count = %d\n", count);
	// Align to 16 bytes
	item = data + ((offset + 4 + 16) & 0xFFFFFFF0) - dataStart + fileStart;
	for (uint32 i = 0; i < count; i++) {
		HitRect hitRect;
		hitRect.x1 = READ_LE_UINT16(item + 0);
		hitRect.y1 = READ_LE_UINT16(item + 2);
		hitRect.x2 = READ_LE_UINT16(item + 4);
		hitRect.y2 = READ_LE_UINT16(item + 6);
		hitRect.messageNum = READ_LE_UINT16(item + 8);
		item += 10;
		//printf("(%d, %d, %d, %d) -> %04X\n", hitRect.x1, hitRect.y1, hitRect.x2, hitRect.y2, hitRect.messageNum);
		hitRectList->hitRects.push_back(hitRect);
	}
	hitRectLists.push_back(hitRectList);
}

void addMessage(uint32 count, uint32 offset) {
	MessageList *messageList = new MessageList();
	messageList->id = offset;
	byte *item = getData(offset);
	for (uint32 i = 0; i < count; i++) {
		MessageItem messageItem;
		messageItem.messageNum = READ_LE_UINT16(item + 0);
		messageItem.messageParam = READ_LE_UINT32(item + 4);
		//printf("%04X, %08X\n", messageItem.messageNum, messageItem.messageParam);
		item += 8;
		messageList->messageItems.push_back(messageItem);
	}
	messageLists.push_back(messageList);	   
}

void addRect(uint32 count, uint32 offset) {
	RectList *rectList = new RectList();
	rectList->id = offset;
	byte *item = getData(offset);
	for (uint32 i = 0; i < count; i++) {
		RectItem rectItem;
		byte *subItem;
		rectItem.x1 = READ_LE_UINT16(item + 0);
		rectItem.y1 = READ_LE_UINT16(item + 2);
		rectItem.x2 = READ_LE_UINT16(item + 4);
		rectItem.y2 = READ_LE_UINT16(item + 6);
		rectItem.subRectListCount = READ_LE_UINT32(item + 8);
		rectItem.subRectListOffset = READ_LE_UINT32(item + 12);
		//printf("(%d, %d, %d, %d), %d, %08X\n", rectItem.x1, rectItem.y1, rectItem.x2, rectItem.y2, rectItem.subRectListCount, rectItem.subRectListOffset);
		subItem = getData(rectItem.subRectListOffset);
		for (uint32 j = 0; j < rectItem.subRectListCount; j++) {
			SubRectItem subRectItem;
			subRectItem.x1 = READ_LE_UINT16(subItem + 0);
			subRectItem.y1 = READ_LE_UINT16(subItem + 2);
			subRectItem.x2 = READ_LE_UINT16(subItem + 4);
			subRectItem.y2 = READ_LE_UINT16(subItem + 6);
			subRectItem.messageListCount = READ_LE_UINT32(subItem + 8);
			subRectItem.messageListOffset = READ_LE_UINT32(subItem + 12);
			subItem += 16;
			//printf("(%d, %d, %d, %d), %d, %08X\n", subRectItem.x1, subRectItem.y1, subRectItem.x2, subRectItem.y2, subRectItem.messageListCount, subRectItem.messageListOffset);
			addMessage(subRectItem.messageListCount, subRectItem.messageListOffset);
			rectItem.subRectItems.push_back(subRectItem);
		}
		item += 16;
		rectList->rectItems.push_back(rectItem);
	}
	rectLists.push_back(rectList);
}

int main(int argc, char *argv[]) {

	FILE *datFile;

	loadExe("nhc.exe");

	for (int i = 0; hitRectListOffsets[i] != 0; i++) {
		addHitRect(hitRectListOffsets[i]);
	}

	for (int i = 0; rectListOffsets[i] != 0; i += 2) {
		addRect(rectListOffsets[i], rectListOffsets[i + 1]);
	}
	
	for (int i = 0; messageListOffsets[i] != 0; i += 2) {
		addMessage(messageListOffsets[i], messageListOffsets[i + 1]);
	}
	
	datFile = fopen("neverhood.dat", "wb");

	writeUint32LE(datFile, 0x11223344); // Some magic
	writeUint32LE(datFile, DAT_VERSION);
		
	// Write all message lists
	writeUint32LE(datFile, messageLists.size());
	for (std::vector<MessageList*>::iterator it = messageLists.begin(); it != messageLists.end(); it++) {
		MessageList *messageList = *it;
		writeUint32LE(datFile, messageList->id);
		writeUint32LE(datFile, messageList->messageItems.size());
		for (uint32 i = 0; i < messageList->messageItems.size(); i++) {
			writeUint16LE(datFile, messageList->messageItems[i].messageNum);
			writeUint32LE(datFile, messageList->messageItems[i].messageParam);
		}
	}

	// Write all rect lists
	writeUint32LE(datFile, rectLists.size());
	for (std::vector<RectList*>::iterator it = rectLists.begin(); it != rectLists.end(); it++) {
		RectList *rectList = *it;
		writeUint32LE(datFile, rectList->id);
		writeUint32LE(datFile, rectList->rectItems.size());
		for (uint32 i = 0; i < rectList->rectItems.size(); i++) {
			const RectItem &rectItem = rectList->rectItems[i]; 
			writeUint16LE(datFile, rectItem.x1);
			writeUint16LE(datFile, rectItem.y1);
			writeUint16LE(datFile, rectItem.x2);
			writeUint16LE(datFile, rectItem.y2);
			writeUint32LE(datFile, rectItem.subRectItems.size());
			for (uint32 j = 0; j < rectItem.subRectItems.size(); j++) {
				const SubRectItem &subRectItem = rectItem.subRectItems[j]; 
				writeUint16LE(datFile, subRectItem.x1);
				writeUint16LE(datFile, subRectItem.y1);
				writeUint16LE(datFile, subRectItem.x2);
				writeUint16LE(datFile, subRectItem.y2);
				writeUint32LE(datFile, subRectItem.messageListOffset);
			}
		}
	}
		
	// Write all hit rect lists
	writeUint32LE(datFile, hitRectLists.size());
	for (std::vector<HitRectList*>::iterator it = hitRectLists.begin(); it != hitRectLists.end(); it++) {
		HitRectList *hitRectList = *it;
		writeUint32LE(datFile, hitRectList->id);
		writeUint32LE(datFile, hitRectList->hitRects.size());
		for (uint32 i = 0; i < hitRectList->hitRects.size(); i++) {
			const HitRect &hitRect  = hitRectList->hitRects[i]; 
			writeUint16LE(datFile, hitRect.x1);
			writeUint16LE(datFile, hitRect.y1);
			writeUint16LE(datFile, hitRect.x2);
			writeUint16LE(datFile, hitRect.y2);
			writeUint16LE(datFile, hitRect.messageNum);
		}
	}
		
	fclose(datFile);

	printf("Done.\n");

	return 0;
}
