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

#include "lastexpress/entities/entity.h"

#include "lastexpress/entities/entity_intern.h"

#include "lastexpress/data/sequence.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"
#include "lastexpress/game/savegame.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/sound.h"

#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"

namespace LastExpress {

//////////////////////////////////////////////////////////////////////////
// EntityData
//////////////////////////////////////////////////////////////////////////

EntityData::EntityCallData::~EntityCallData() {
	SAFE_DELETE(frame);
	SAFE_DELETE(frame1);

	SAFE_DELETE(sequence);
	SAFE_DELETE(sequence2);
	SAFE_DELETE(sequence3);
}

void EntityData::EntityCallData::saveLoadWithSerializer(Common::Serializer &s) {
	for (uint i = 0; i < ARRAYSIZE(callbacks); i++)
		s.syncAsByte(callbacks[i]);

	s.syncAsByte(currentCall);
	s.syncAsUint16LE(entityPosition);
	s.syncAsUint16LE(location);
	s.syncAsUint16LE(car);
	s.syncAsByte(field_497);
	s.syncAsByte(entity);
	s.syncAsByte(inventoryItem);
	s.syncAsByte(direction);
	s.syncAsUint16LE(field_49B);
	s.syncAsUint16LE(currentFrame);
	s.syncAsUint16LE(currentFrame2);
	s.syncAsUint16LE(field_4A1);
	s.syncAsUint16LE(field_4A3);
	s.syncAsByte(clothes);
	s.syncAsByte(position);
	s.syncAsByte(car2);
	s.syncAsByte(doProcessEntity);
	s.syncAsByte(field_4A9);
	s.syncAsByte(field_4AA);
	s.syncAsByte(directionSwitch);

	// Sync strings
#define SYNC_STRING(varName, count) { \
	char seqName[13]; \
	memset(&seqName, 0, count); \
	if (s.isSaving()) strcpy((char *)&seqName, varName.c_str()); \
	s.syncBytes((byte *)&seqName, count); \
	if (s.isLoading()) varName = seqName; \
}

	SYNC_STRING(sequenceName, 13);
	SYNC_STRING(sequenceName2, 13);
	SYNC_STRING(sequenceNamePrefix, 7);
	SYNC_STRING(sequenceNameCopy, 13);

#undef SYNC_STRING

	// Skip pointers to frame & sequences
	s.skip(5 * 4);
}

//////////////////////////////////////////////////////////////////////////
// EntityData
//////////////////////////////////////////////////////////////////////////
EntityData::EntityParameters *EntityData::getParameters(uint callback, byte index) const {
	if (callback >= 9)
		error("[EntityData::getParameters] Invalid callback value (was: %d, max: 9)", callback);

	if (index >= 4)
		error("[EntityData::getParameters] Invalid index value (was: %d, max: 4)", index);

	return _parameters[callback].parameters[index];
}

int EntityData::getCallback(uint callback) const {
	if (callback >= 16)
		error("[EntityData::getCallback] Invalid callback value (was: %d, max: 16)", callback);

	return _data.callbacks[callback];
}

void EntityData::setCallback(uint callback, byte index) {
	if (callback >= 16)
		error("[EntityData::setCallback] Invalid callback value (was: %d, max: 16)", callback);

	_data.callbacks[callback] = index;
}

void EntityData::updateParameters(uint32 index) const {
	if (index < 8)
		getParameters(8, 0)->update(index);
	else if (index < 16)
		getParameters(8, 1)->update(index - 8);
	else if (index < 24)
		getParameters(8, 2)->update(index - 16);
	else if (index < 32)
		getParameters(8, 3)->update(index - 24);
	else
		error("[EntityData::updateParameters] Invalid param index to update (was:%d, max:32)", index);
}

void EntityData::saveLoadWithSerializer(Common::Serializer &s) {
	for (uint i = 0; i < ARRAYSIZE(_parameters); i++)
		_parameters[i].saveLoadWithSerializer(s);

	_data.saveLoadWithSerializer(s);
}

//////////////////////////////////////////////////////////////////////////
// Entity
//////////////////////////////////////////////////////////////////////////
Entity::Entity(LastExpressEngine *engine, EntityIndex index) : _engine(engine), _entityIndex(index) {
	_data = new EntityData();

	// Add first empty entry to callbacks array
	_callbacks.push_back(NULL);
}

Entity::~Entity() {
	for (uint i = 0; i < _callbacks.size(); i++)
		SAFE_DELETE(_callbacks[i]);

	_callbacks.clear();

	SAFE_DELETE(_data);

	// Zero-out passed pointers
	_engine = NULL;
}

void Entity::setup(ChapterIndex index) {
	switch(index) {
	case kChapterAll:
		getSavePoints()->setCallback(_entityIndex, _callbacks[_data->getCurrentCallback()]);
		break;

	case kChapter1:
		setup_chapter1();
		break;

	case kChapter2:
		setup_chapter2();
		break;

	case kChapter3:
		setup_chapter3();
		break;

	case kChapter4:
		setup_chapter4();
		break;

	case kChapter5:
		setup_chapter5();
		break;

	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// Shared functions
//////////////////////////////////////////////////////////////////////////

void Entity::reset(const SavePoint &savepoint, bool resetClothes, bool resetItem) {
	EXPOSE_PARAMS(EntityData::EntityParametersIIII)

	switch (savepoint.action) {
	default:
		break;

	case kAction1:
		if (resetClothes) {
			// Select next available clothes
			getData()->clothes = (ClothesIndex)(getData()->clothes + 1);
			if (getData()->clothes > kClothes3)
				getData()->clothes = kClothesDefault;
		}
		break;

	case kActionNone:
		if (getEntities()->updateEntity(_entityIndex, kCarGreenSleeping, (EntityPosition)params->param1))
			params->param1 = (params->param1 == 10000) ? 0 : 10000;
		break;

	case kActionDefault:
		getData()->entityPosition = kPositionNone;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarGreenSleeping;

		if (resetItem)
			getData()->inventoryItem = kItemInvalid;

		params->param1 = 10000;
		break;
	}
}

void Entity::savegame(const SavePoint &savepoint) {
	EXPOSE_PARAMS(EntityData::EntityParametersIIII)

	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		CALLBACK_ACTION();
		break;

	case kActionDefault:
		getSaveLoad()->saveGame((SavegameType)params->param1, _entityIndex, (EventIndex)params->param2);
		CALLBACK_ACTION();
		break;
	}
}

void Entity::playSound(const SavePoint &savepoint, bool resetItem, SoundFlag flag) {
	EXPOSE_PARAMS(EntityData::EntityParametersSIIS)

	switch (savepoint.action) {
	default:
		break;

	case kActionEndSound:
		CALLBACK_ACTION();
		break;

	case kActionDefault:
		if (resetItem)
			getData()->inventoryItem = kItemNone;

		getSound()->playSound(_entityIndex, (char *)&params->seq1, flag);
		break;
	}
}

void Entity::draw(const SavePoint &savepoint, bool handleExcuseMe) {
	EXPOSE_PARAMS(EntityData::EntityParametersSIIS)

	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		CALLBACK_ACTION();
		break;

	case kActionExcuseMeCath:
		if (handleExcuseMe && !params->param4) {
			getSound()->excuseMe(_entityIndex);
			params->param4 = 1;
		}
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(_entityIndex, (char *)&params->seq1);
		break;
	}
}

void Entity::draw2(const SavePoint &savepoint) {
	EXPOSE_PARAMS(EntityData::EntityParametersSSII)

	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		CALLBACK_ACTION();
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(_entityIndex, (char *)&params->seq1);
		getEntities()->drawSequenceRight((EntityIndex)params->param7, (char *)&params->seq2);
		break;
	}
}

void Entity::updateFromTicks(const SavePoint &savepoint) {
	EXPOSE_PARAMS(EntityData::EntityParametersIIII)

	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		UPDATE_PARAM(params->param2, getState()->timeTicks, params->param1)
		CALLBACK_ACTION();
		break;
	}
}

void Entity::updateFromTime(const SavePoint &savepoint) {
	EXPOSE_PARAMS(EntityData::EntityParametersIIII)

	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		UPDATE_PARAM(params->param2, getState()->time, params->param1)
		CALLBACK_ACTION();
		break;
	}
}

void Entity::callbackActionOnDirection(const SavePoint &savepoint) {
	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		CALLBACK_ACTION();
		break;

	case kActionDefault:
		if (getData()->direction != kDirectionRight)
			CALLBACK_ACTION();
		break;
	}
}

void Entity::callbackActionRestaurantOrSalon(const SavePoint &savepoint) {
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
	case kActionDefault:
		if (getEntities()->isSomebodyInsideRestaurantOrSalon())
			CALLBACK_ACTION();
		break;
	}
}

void Entity::updateEntity(const SavePoint &savepoint, bool handleExcuseMe) {
	EXPOSE_PARAMS(EntityData::EntityParametersIIII)

	switch (savepoint.action) {
	default:
		break;

	case kActionExcuseMeCath:
		if (handleExcuseMe)
			getSound()->excuseMeCath();
		break;

	case kActionExcuseMe:
		if (handleExcuseMe)
			getSound()->excuseMe(_entityIndex);
		break;

	case kActionNone:
	case kActionDefault:
		if (getEntities()->updateEntity(_entityIndex, (CarIndex)params->param1, (EntityPosition)params->param2))
			CALLBACK_ACTION();
		break;
	}
}

void Entity::callSavepoint(const SavePoint &savepoint, bool handleExcuseMe) {
	EXPOSE_PARAMS(EntityData::EntityParametersSIIS)

	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		if (!CURRENT_PARAM(1, 1))
			getSavePoints()->call(_entityIndex, (EntityIndex)params->param4, (ActionIndex)params->param5, (char *)&params->seq2);
		CALLBACK_ACTION();
		break;

	case kActionExcuseMeCath:
		if (handleExcuseMe && !CURRENT_PARAM(1, 2)) {
			getSound()->excuseMe(_entityIndex);
			CURRENT_PARAM(1, 2) = 1;
		}
		break;

	case kAction10:
		if (!CURRENT_PARAM(1, 1)) {
			getSavePoints()->call(_entityIndex, (EntityIndex)params->param4, (ActionIndex)params->param5, (char *)&params->seq2);
			CURRENT_PARAM(1, 1) = 1;
		}
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(_entityIndex, (char *)&params->seq1);
		break;
	}
}

void Entity::enterExitCompartment(const SavePoint &savepoint, EntityPosition position1, EntityPosition position2, CarIndex car, ObjectIndex compartment, bool alternate, bool updateLocation) {
	EXPOSE_PARAMS(EntityData::EntityParametersSIIS)

	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		getEntities()->exitCompartment(_entityIndex, (ObjectIndex)params->param4);
		if (position1)
			getData()->entityPosition = position1;

		if (updateLocation)
			getData()->location = kLocationInsideCompartment;

		CALLBACK_ACTION();
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(_entityIndex, (char *)&params->seq1);
		getEntities()->enterCompartment(_entityIndex, (ObjectIndex)params->param4);

		if (position1) {
			getData()->location = kLocationInsideCompartment;

			if (getEntities()->isInsideCompartment(kEntityPlayer, car, position1) || getEntities()->isInsideCompartment(kEntityPlayer, car, position2)) {
				getAction()->playAnimation(isNight() ? kEventCathTurningNight : kEventCathTurningDay);
				getSound()->playSound(kEntityPlayer, "BUMP");
				getScenes()->loadSceneFromObject(compartment, alternate);
			}
		}
		break;
	}
}

void Entity::updatePosition(const SavePoint &savepoint, bool handleExcuseMe) {
	EXPOSE_PARAMS(EntityData::EntityParametersSIII)

	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		getEntities()->updatePositionExit(_entityIndex, (CarIndex)params->param4, (Position)params->param5);
		CALLBACK_ACTION();
		break;

	case kActionExcuseMeCath:
		if (handleExcuseMe && !params->param6) {
			getSound()->excuseMe(_entityIndex);
			params->param6 = 1;
		}
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(_entityIndex, (char *)&params->seq);
		getEntities()->updatePositionEnter(_entityIndex, (CarIndex)params->param4, (Position)params->param5);
		break;
	}
}

} // End of namespace LastExpress
