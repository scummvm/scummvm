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

#ifndef ILLUSIONS_TALKRESOURCE_H
#define ILLUSIONS_TALKRESOURCE_H

#include "illusions/graphics.h"
#include "illusions/resourcesystem.h"

namespace Illusions {

class IllusionsEngine;

class TalkResourceLoader : public BaseResourceLoader {
public:
	TalkResourceLoader(IllusionsEngine *vm) : _vm(vm) {}
	virtual ~TalkResourceLoader() {}
	virtual void load(Resource *resource);
	virtual void unload(Resource *resource);
	virtual void buildFilename(Resource *resource);
	virtual bool isFlag(int flag);
protected:
	IllusionsEngine *_vm;
};

struct TalkEntry {
	uint32 _talkId;
	//field_4 dd
	byte *_text;
	byte *_tblPtr;
	byte *_voiceName;
	void load(byte *dataStart, Common::SeekableReadStream &stream);
};

class TalkResource {
public:
	TalkResource();
	~TalkResource();
	void load(byte *data, uint32 dataSize);
public:
	uint _talkEntriesCount;
	TalkEntry *_talkEntries;
};

class TalkItem {
public:
	TalkItem(IllusionsEngine *vm, uint32 talkId, uint32 tag, TalkResource *talkResource);
	~TalkItem();
	void registerResources();
	void unregisterResources();
	void pause();
	void unpause();
public:
	IllusionsEngine *_vm;
	uint32 _talkId;
	uint32 _tag;
	TalkResource *_talkRes;
	int _pauseCtr;
};

class TalkItems {
public:
	TalkItems(IllusionsEngine *vm);
	~TalkItems();
	TalkItem *newTalkItem(uint32 talkId, uint32 tag, TalkResource *talkResource);
	void freeTalkItem(TalkItem *talkItem);
	TalkItem *findTalkItem(uint32 talkId);
	TalkItem *findTalkItemByTag(uint32 tag);
	void pauseByTag(uint32 tag);
	void unpauseByTag(uint32 tag);
//protected:
public:
	typedef Common::List<TalkItem*> Items;
	typedef Items::iterator ItemsIterator;
	IllusionsEngine *_vm;
	Items _items;
};

} // End of namespace Illusions

#endif // ILLUSIONS_TALKRESOURCE_H
