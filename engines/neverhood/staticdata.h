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

#ifndef NEVERHOOD_STATICDATA_H
#define NEVERHOOD_STATICDATA_H

#include "common/array.h"
#include "common/hashmap.h"
#include "neverhood/neverhood.h"
#include "neverhood/graphics.h"

namespace Neverhood {

struct HitRect {
	NRect rect;
	uint16 type;
};

typedef Common::Array<HitRect> HitRectList;

struct SubRectItem {
	NRect rect;
	uint32 messageListId;
};

struct RectItem {
	NRect rect;
	Common::Array<SubRectItem> subRects;
};

typedef Common::Array<RectItem> RectList;

struct MessageItem {
	uint16 messageNum;
	uint32 messageValue;
};

typedef Common::Array<MessageItem> MessageList;

class StaticData {
public:
	StaticData();
	~StaticData();
	void load(const char *filename);
	HitRectList *getHitRectList(uint32 id);
	RectList *getRectList(uint32 id);
	MessageList *getMessageList(uint32 id);
protected:
	Common::HashMap<uint32, HitRectList*> _hitRectLists;
	Common::HashMap<uint32, RectList*> _rectLists;
	Common::HashMap<uint32, MessageList*> _messageLists;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_STATICDATA_H */
