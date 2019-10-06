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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL


// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include "create_neverhood.h"
#include <vector>
#include "md5.h"
#include "tables.h"

const int DAT_VERSION = 0;

// The MD5 hash of the nhc.exe used to extract the tables from
const uint8 kNhcExeMd5[16] = {
	0x37, 0xD6, 0x54, 0xA2, 0xA7, 0xBB, 0xB0, 0x1F,
	0x8C, 0x41, 0x9A, 0xB8, 0x49, 0xFF, 0x29, 0xD4};

uint32 dataSize;
byte *data;
uint32 dataStart = 0x004AE000;
uint32 fileStart = 0x000AC600;

class HitRectList;
class RectList;
class MessageList;
class NavigationList;

void addMessageList(uint32 messageListCount, uint32 messageListOffset);

bool loadExe(const char *filename) {
	FILE *exe = fopen(filename, "rb");
	if (!exe) {
		printf("Could not open nhc.exe for reading! Quitting...\n");
		return false;
	}
	dataSize = fileSize(exe);
	data = new byte[dataSize];
	fread(data, dataSize, 1, exe);
	fclose(exe);
	return true;
}

bool validateMd5() {
	uint8 digest[16];

	md5_buffer(data, dataSize, digest);

	printf("MD5 of nhc.exe is %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
		digest[0], digest[1], digest[2], digest[3], digest[4], digest[5], digest[6], digest[7],
		digest[8], digest[9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15]);

	if (memcmp(kNhcExeMd5, digest, 16)) {
		printf("MD5 hash of nhc.exe doesn't match the expected value! Quitting...\n");
		return false;
	}
	return true;
}

byte *getData(uint32 offset) {
	return data + offset - dataStart + fileStart;
}

const char *getStringP(uint32 offset) {
	return offset != 0 ? (const char*)getData(offset) : NULL;
}

uint32 calcHash(const char *value) {
	if (!value)
		return 0;
	uint32 hash = 0, shiftValue = 0;
	while (*value != 0) {
		char ch = *value++;
		if (ch >= 'a' && ch <= 'z')
			ch -= 32;
		else if (ch >= '0' && ch <= '9')
			ch += 22;
		shiftValue += ch - 64;
		if (shiftValue >= 32)
			shiftValue -= 32;
		hash ^= 1 << shiftValue;
	}
	return hash;
}

struct HitRect {
	int16 x1, y1, x2, y2;
	uint16 messageNum;

	void load(uint32 offset) {
		byte *item = getData(offset);
		x1 = READ_LE_UINT16(item + 0);
		y1 = READ_LE_UINT16(item + 2);
		x2 = READ_LE_UINT16(item + 4);
		y2 = READ_LE_UINT16(item + 6);
		messageNum = READ_LE_UINT16(item + 8);
	}

	void save(FILE *fd) {
		writeUint16LE(fd, x1);
		writeUint16LE(fd, y1);
		writeUint16LE(fd, x2);
		writeUint16LE(fd, y2);
		writeUint16LE(fd, messageNum);
	}

	int getItemSize() const {
		return 10;
	}

};

struct MessageItem {
	uint16 messageNum;
	uint32 messageParam;
	MessageItem() {}
	MessageItem(uint16 msgNum, uint32 msgParam) : messageNum(msgNum), messageParam(msgParam) {}

	void load(uint32 offset) {
		byte *item = getData(offset);
		messageNum = READ_LE_UINT16(item + 0);
		messageParam = READ_LE_UINT32(item + 4);
	}

	void save(FILE *fd) {
		writeUint16LE(fd, messageNum);
		writeUint32LE(fd, messageParam);
	}

	int getItemSize() const {
		return 8;
	}

};

struct SubRectItem {
	int16 x1, y1, x2, y2;
	uint32 messageListCount;
	uint32 messageListOffset;

	void load(uint32 offset) {
		byte *item = getData(offset);
		x1 = READ_LE_UINT16(item + 0);
		y1 = READ_LE_UINT16(item + 2);
		x2 = READ_LE_UINT16(item + 4);
		y2 = READ_LE_UINT16(item + 6);
		messageListCount = READ_LE_UINT32(item + 8);
		messageListOffset = READ_LE_UINT32(item + 12);
		// Add the message to the message list
		addMessageList(messageListCount, messageListOffset);
	}

	void save(FILE *fd) {
		writeUint16LE(fd, x1);
		writeUint16LE(fd, y1);
		writeUint16LE(fd, x2);
		writeUint16LE(fd, y2);
		writeUint32LE(fd, messageListOffset);
	}

	int getItemSize() const {
		return 16;
	}

};

struct RectItem {
	int16 x1, y1, x2, y2;
	uint32 subRectListCount;
	uint32 subRectListOffset;
	std::vector<SubRectItem> subRectItems;

	void load(uint32 offset) {
		byte *item = getData(offset);
		uint32 subItemOffset;
		x1 = READ_LE_UINT16(item + 0);
		y1 = READ_LE_UINT16(item + 2);
		x2 = READ_LE_UINT16(item + 4);
		y2 = READ_LE_UINT16(item + 6);
		subRectListCount = READ_LE_UINT32(item + 8);
		subRectListOffset = READ_LE_UINT32(item + 12);
		subItemOffset = subRectListOffset;
		for (uint32 j = 0; j < subRectListCount; j++) {
			SubRectItem subRectItem;
			subRectItem.load(subItemOffset);
			subItemOffset += 16;
			subRectItems.push_back(subRectItem);
		}
	}

	void save(FILE *fd) {
		writeUint16LE(fd, x1);
		writeUint16LE(fd, y1);
		writeUint16LE(fd, x2);
		writeUint16LE(fd, y2);
		writeUint32LE(fd, subRectItems.size());
		for (uint32 j = 0; j < subRectItems.size(); j++)
			subRectItems[j].save(fd);
	}

	int getItemSize() const {
		return 16;
	}

};

struct NavigationItem {
	uint32 fileHash;
	uint32 leftSmackerFileHash;
	uint32 rightSmackerFileHash;
	uint32 middleSmackerFileHash;
	byte interactive;
	byte middleFlag;
	uint32 mouseCursorFileHash;

	void load(uint32 offset) {
		byte *item = getData(offset);
		fileHash = READ_LE_UINT32(item + 0);
		leftSmackerFileHash = READ_LE_UINT32(item + 4);
		rightSmackerFileHash = READ_LE_UINT32(item + 8);
		middleSmackerFileHash = READ_LE_UINT32(item + 12);
		interactive = item[16];
		middleFlag = item[17];
		mouseCursorFileHash = READ_LE_UINT32(item + 20);
	}

	void save(FILE *fd) {
		writeUint32LE(fd, fileHash);
		writeUint32LE(fd, leftSmackerFileHash);
		writeUint32LE(fd, rightSmackerFileHash);
		writeUint32LE(fd, middleSmackerFileHash);
		writeByte(fd, interactive);
		writeByte(fd, middleFlag);
		writeUint32LE(fd, mouseCursorFileHash);
	}

	int getItemSize() const {
		return 24;
	}

};

struct SceneInfo140Item {
	uint32 id;
	uint32 bgFilename1;
	uint32 bgFilename2;
	uint32 txFilename;
	uint32 bgFilename3;
	byte xPosIndex;
	byte count;

	void load(uint32 offset) {
		byte *item = getData(offset);
		id = offset;
		// Only save the hashes instead of the full names
		bgFilename1 = calcHash(getStringP(READ_LE_UINT32(item + 0)));
		bgFilename2 = calcHash(getStringP(READ_LE_UINT32(item + 4)));
		txFilename = calcHash(getStringP(READ_LE_UINT32(item + 8)));
		bgFilename3 = calcHash(getStringP(READ_LE_UINT32(item + 12)));
		xPosIndex = item[16];
		count = item[17];
	}

	void save(FILE *fd) {
		writeUint32LE(fd, id);
		writeUint32LE(fd, bgFilename1);
		writeUint32LE(fd, bgFilename2);
		writeUint32LE(fd, txFilename);
		writeUint32LE(fd, bgFilename3);
		writeByte(fd, xPosIndex);
		writeByte(fd, count);
	}

};

struct SceneInfo2700Item {
	uint32 id;
	uint32 bgFilename;
	uint32 class437Filename;
	uint32 dataResourceFilename;
	uint32 pointListName;
	uint32 rectListName;
	uint32 exPaletteFilename2;
	uint32 exPaletteFilename1;
	uint32 mouseCursorFilename;
	int16 which1;
	int16 which2;

	void load(uint32 offset) {
		byte *item = getData(offset);
		id = offset;
		// Only save the hashes instead of the full names
		bgFilename = calcHash(getStringP(READ_LE_UINT32(item + 0)));
		class437Filename = calcHash(getStringP(READ_LE_UINT32(item + 4)));
		dataResourceFilename = calcHash(getStringP(READ_LE_UINT32(item + 8)));
		pointListName = calcHash(getStringP(READ_LE_UINT32(item + 12)));
		rectListName = calcHash(getStringP(READ_LE_UINT32(item + 16)));
		exPaletteFilename2 = calcHash(getStringP(READ_LE_UINT32(item + 20)));
		exPaletteFilename1 = calcHash(getStringP(READ_LE_UINT32(item + 24)));
		mouseCursorFilename = calcHash(getStringP(READ_LE_UINT32(item + 28)));
		which1 = READ_LE_UINT16(item + 32);
		which2 = READ_LE_UINT16(item + 34);
	}

	void save(FILE *fd) {
		writeUint32LE(fd, id);
		writeUint32LE(fd, bgFilename);
		writeUint32LE(fd, class437Filename);
		writeUint32LE(fd, dataResourceFilename);
		writeUint32LE(fd, pointListName);
		writeUint32LE(fd, rectListName);
		writeUint32LE(fd, exPaletteFilename2);
		writeUint32LE(fd, exPaletteFilename1);
		writeUint32LE(fd, mouseCursorFilename);
		writeUint16LE(fd, which1);
		writeUint16LE(fd, which2);
	}

};

template<class ITEMCLASS>
class StaticDataList {
public:
	uint32 id;
	std::vector<ITEMCLASS> items;

	virtual ~StaticDataList() {
	}

	void add(ITEMCLASS item) {
		items.push_back(item);
	}

	int getCount() const {
		return items.size();
	}

	ITEMCLASS *getListItem(int index) {
		return &items[index];
	}

	virtual bool specialLoadList(uint32 count, uint32 offset) {
		return false;
	}

	void loadList(uint32 count, uint32 offset) {
		id = offset;
		if (!specialLoadList(count, offset)) {
			for (uint32 i = 0; i < count; i++) {
				ITEMCLASS listItem;
				listItem.load(offset);
				offset += listItem.getItemSize();
				add(listItem);
			}
		}
	}

	void saveList(FILE *fd) {
		writeUint32LE(fd, id);
		writeUint32LE(fd, getCount());
		for (int i = 0; i < getCount(); i++) {
			items[i].save(fd);
		}
	}

};

class HitRectList : public StaticDataList<HitRect> {
};

class RectList : public StaticDataList<RectItem> {
};

class MessageList : public StaticDataList<MessageItem> {
public:

	virtual bool specialLoadList(uint32 count, uint32 offset) {
		// Special code for message lists which are set at runtime (but otherwise constant)
		switch (offset) {
		// Scene 1002 rings
		case 0x004B4200:
			add(MessageItem(0x4800, 258));
			add(MessageItem(0x100D, 0x4A845A00));
			add(MessageItem(0x4805, 1));
			return true;
		case 0x004B4218:
			add(MessageItem(0x4800, 297));
			add(MessageItem(0x100D, 0x43807801));
			add(MessageItem(0x4805, 2));
			return true;
		case 0x004B4230:
			add(MessageItem(0x4800, 370));
			add(MessageItem(0x100D, 0x46C26A01));
			return true;
		case 0x004B4240:
			add(MessageItem(0x4800, 334));
			add(MessageItem(0x100D, 0x468C7B11));
			add(MessageItem(0x4805, 1));
			return true;
		case 0x004B4258:
			add(MessageItem(0x4800, 425));
			add(MessageItem(0x100D, 0x42845B19));
			add(MessageItem(0x4805, 1));
			return true;
		// Scene 1302 rings
		case 0x004B0888:
			add(MessageItem(0x4800, 218));
			add(MessageItem(0x100D, 0x4A845A00));
			add(MessageItem(0x4805, 1));
			return true;
		case 0x004B08A0:
			add(MessageItem(0x4800, 218 + 32));
			add(MessageItem(0x100D, 0x43807801));
			return true;
		case 0x004B08B0:
			add(MessageItem(0x4800, 218 + 32 + 32));
			add(MessageItem(0x100D, 0x46C26A01));
			add(MessageItem(0x4805, 1));
			return true;
		case 0x004B08C8:
			add(MessageItem(0x4800, 218 + 32 + 32 + 32));
			add(MessageItem(0x100D, 0x468C7B11));
			return true;
		case 0x004B08D8:
			add(MessageItem(0x4800, 218 + 32 + 32 + 32 + 32));
			add(MessageItem(0x100D, 0x42845B19));
			add(MessageItem(0x4805, 4));
			return true;
		default:
			break;
		}
		return false;
	}

};

class NavigationList : public StaticDataList<NavigationItem> {
};

template<class LISTCLASS>
class StaticDataListVector {
public:
	std::vector<LISTCLASS*> lists;

	void add(LISTCLASS *list) {
		bool doAppend = true;
		for (typename std::vector<LISTCLASS*>::iterator it = lists.begin(); it != lists.end(); it++) {
			if ((*it)->id == list->id) {
				doAppend = false;
				break;
			}
		}
		if (doAppend)
			lists.push_back(list);
	}

	void loadListVector(const uint32 *offsets) {
		for (int i = 0; offsets[i] != 0; i += 2) {
			LISTCLASS *list = new LISTCLASS();
			list->loadList(offsets[i], offsets[i + 1]);
			bool doAppend = true;
			for (typename std::vector<LISTCLASS*>::iterator it = lists.begin(); it != lists.end(); it++) {
				if ((*it)->id == list->id) {
					doAppend = false;
					break;
				}
			}
			if (doAppend)
				lists.push_back(list);
		}
	}

	void saveListVector(FILE *fd) {
		writeUint32LE(fd, lists.size());
		for (typename std::vector<LISTCLASS*>::iterator it = lists.begin(); it != lists.end(); it++) {
			(*it)->saveList(fd);
		}
	}

};

template<class ITEMCLASS>
class StaticDataVector {
public:
	std::vector<ITEMCLASS> items;

	void loadVector(const uint32 *offsets) {
		for (int i = 0; offsets[i] != 0; i++) {
			ITEMCLASS item;
			item.load(offsets[i]);
			items.push_back(item);
		}
	}

	void saveVector(FILE *fd) {
		writeUint32LE(fd, items.size());
		for (typename std::vector<ITEMCLASS>::iterator it = items.begin(); it != items.end(); it++) {
			(*it).save(fd);
		}
	}

};

StaticDataListVector<HitRectList> hitRectLists;
StaticDataListVector<RectList> rectLists;
StaticDataListVector<MessageList> messageLists;
StaticDataListVector<NavigationList> navigationLists;
StaticDataVector<SceneInfo140Item> sceneInfo140Items;
StaticDataVector<SceneInfo2700Item> sceneInfo2700Items;

void addMessageList(uint32 messageListCount, uint32 messageListOffset) {
	MessageList *messageList = new MessageList();
	messageList->loadList(messageListCount, messageListOffset);
	messageLists.add(messageList);
}

int main(int argc, char *argv[]) {

	if (!loadExe("nhc.exe") ||
		!validateMd5())
		return 1;

	FILE *datFile;

	hitRectLists.loadListVector(hitRectListOffsets);
	rectLists.loadListVector(rectListOffsets);
	messageLists.loadListVector(messageListOffsets);
	navigationLists.loadListVector(navigationListOffsets);
	sceneInfo140Items.loadVector(sceneInfo140Offsets);
	sceneInfo2700Items.loadVector(sceneInfo2700Offsets);

	datFile = fopen("neverhood.dat", "wb");

	writeUint32LE(datFile, 0x11223344); // Some magic
	writeUint32LE(datFile, DAT_VERSION);

	messageLists.saveListVector(datFile);
	rectLists.saveListVector(datFile);
	hitRectLists.saveListVector(datFile);
	navigationLists.saveListVector(datFile);
	sceneInfo140Items.saveVector(datFile);
	sceneInfo2700Items.saveVector(datFile);

	fclose(datFile);

	printf("Done.\n");

	return 0;
}
