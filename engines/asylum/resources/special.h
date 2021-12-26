/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_RESOURCES_SPECIAL_H
#define ASYLUM_RESOURCES_SPECIAL_H

#include "common/scummsys.h"
#include "common/rect.h"

#include "asylum/shared.h"
#include "asylum/asylum.h"

namespace Asylum {

class Object;

class Special {
public:

	Special(AsylumEngine *engine);
	~Special() {};

	void run(Object *object, ActorIndex index);

	void reset(bool resetPaletteTick2);

private:
	AsylumEngine *_vm;

	// Counters
	int32 _chapter5Counter;

	bool _paletteFlag;
	uint32 _paletteTick1;
	uint32 _paletteTick2;

	//////////////////////////////////////////////////////////////////////////
	// Spec functions
	//////////////////////////////////////////////////////////////////////////
	void chapter1(Object *object, ActorIndex actorIndex);
	void chapter2(Object *object, ActorIndex actorIndex);
	void chapter3(Object *object, ActorIndex actorIndex);
	void chapter4(Object *object, ActorIndex actorIndex);
	void chapter5(Object *object, ActorIndex actorIndex);
	void chapter6(Object *object, ActorIndex actorIndex);
	void chapter7(Object *object, ActorIndex actorIndex);
	void chapter8(Object *object, ActorIndex actorIndex);
	void chapter9(Object *object, ActorIndex actorIndex);
	void chapter11(Object *object, ActorIndex actorIndex);
	void chapter12(Object *object, ActorIndex actorIndex);
	void chapter13(Object *object, ActorIndex actorIndex);

	//////////////////////////////////////////////////////////////////////////
	// Sound logic
	//////////////////////////////////////////////////////////////////////////
	void playChapterSound(Object *object, ActorIndex actorIndex);
	void playSoundChapter1(Object *object, ActorIndex actorIndex);
	void playSoundChapter2(Object *object, ActorIndex actorIndex);
	void playSoundChapter3(Object *object, ActorIndex actorIndex);
	void playSoundChapter4(Object *object, ActorIndex actorIndex);
	void playSoundChapter6(Object *object, ActorIndex actorIndex);
	void playSoundChapter7(Object *object, ActorIndex actorIndex);
	void playSoundChapter8(Object *object, ActorIndex actorIndex);

	//////////////////////////////////////////////////////////////////////////
	// Misc
	//////////////////////////////////////////////////////////////////////////
	void setPaletteGamma(ResourceId palette1, ResourceId palette2);

	void playSoundPanning(ResourceId resourceId, int32 attenuation, Object *object);
	void playSoundPanning(ResourceId resourceId, int32 attenuation, ActorIndex actorIndex);

	void updateObjectFlag(ObjectId id);
	void checkFlags(ObjectId id, GameFlag flag1, GameFlag flag2, GameFlag flag3, GameFlag flag4, uint32 *val1, uint32 *val2, GameFlag flag5, uint32 *val3);

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	ResourceId getResourceId(Object *object, ActorIndex actorIndex);
	uint32 getCounter(ActorIndex index) const;
	bool chance() { return _vm->getRandom(10000) < 30; }

	void checkObject(Object *object, GameFlag flagToSet, GameFlag flagToClear, ObjectId objectId = kObjectNone);
	void checkOtherObject(Object *object, ObjectId otherObjectId, GameFlag flagToClear, GameFlag flagToSet);

	//////////////////////////////////////////////////////////////////////////
	// Chapter 11 helpers
	//////////////////////////////////////////////////////////////////////////

	void tentacle(ActorIndex actorIndex, GameFlag flag1, GameFlag flag2, GameFlag flag3, const int16 (*rectPtr)[4]);
	void rock(ActorIndex actorIndex, GameFlag flag1, GameFlag flag2, GameFlag flag3, GameFlag flag4, const int16 (*rectPtr)[4]);
};

} // End of namespace Asylum

#endif // ASYLUM_RESOURCES_SPECIAL_H
