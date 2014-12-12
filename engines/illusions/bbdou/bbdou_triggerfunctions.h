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

#ifndef ILLUSIONS_BBDOU_BBDOU_TRIGGERFUNCTIONS_H
#define ILLUSIONS_BBDOU_BBDOU_TRIGGERFUNCTIONS_H

#include "common/algorithm.h"
#include "common/stack.h"

namespace Illusions {

struct TriggerFunction;

typedef Common::Functor2<TriggerFunction*, uint32, void> TriggerFunctionCallback;

struct TriggerFunction {
	uint32 _sceneId;
	uint32 _verbId;
	uint32 _objectId2;
	uint32 _objectId;
	TriggerFunctionCallback *_callback;
	TriggerFunction(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, TriggerFunctionCallback *callback);
	~TriggerFunction();
	void run(uint32 callingThreadId);
};

class TriggerFunctions {
public:
	void add(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, TriggerFunctionCallback *callback);
	TriggerFunction *find(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId);
	void removeBySceneId(uint32 sceneId);
public:
	typedef Common::List<TriggerFunction*> Items;
	typedef Items::iterator ItemsIterator;
	Items _triggerFunctions;
	ItemsIterator findInternal(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId);
};

} // End of namespace Illusions

#endif // ILLUSIONS_BBDOU_BBDOU_TRIGGERFUNCTIONS_H
