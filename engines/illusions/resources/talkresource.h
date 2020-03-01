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
	~TalkResourceLoader() override {}
	void load(Resource *resource) override;
	bool isFlag(int flag) override;
protected:
	IllusionsEngine *_vm;
};

struct TalkEntry {
	uint32 _talkId;
	//field_4 dd
	uint16 *_text;
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

class TalkInstance : public ResourceInstance {
public:
	TalkInstance(IllusionsEngine *vm);
	void load(Resource *resource) override;
	void unload() override;
	void pause() override;
	void unpause() override;
public:
	IllusionsEngine *_vm;
	uint32 _talkId;
	uint32 _sceneId;
	TalkResource *_talkRes;
	int _pauseCtr;
	void registerResources();
	void unregisterResources();
};

class TalkInstanceList {
public:
	TalkInstanceList(IllusionsEngine *vm);
	~TalkInstanceList();
	TalkInstance *createTalkInstance(Resource *resource);
	void removeTalkInstance(TalkInstance *talkInstance);
	TalkInstance *findTalkItem(uint32 talkId);
	TalkInstance *findTalkItemBySceneId(uint32 sceneId);
	void pauseBySceneId(uint32 sceneId);
	void unpauseBySceneId(uint32 sceneId);
//protected:
public:
	typedef Common::List<TalkInstance*> Items;
	typedef Items::iterator ItemsIterator;
	IllusionsEngine *_vm;
	Items _items;
};

} // End of namespace Illusions

#endif // ILLUSIONS_TALKRESOURCE_H
