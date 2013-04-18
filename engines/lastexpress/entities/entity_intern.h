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

#ifndef LASTEXPRESS_ENTITY_INTERN_H
#define LASTEXPRESS_ENTITY_INTERN_H

namespace LastExpress {

#define LOW_BYTE(w)           ((unsigned char)(((unsigned long)(w)) & 0xff))

//////////////////////////////////////////////////////////////////////////
// Callbacks
#define ENTITY_CALLBACK(class, name, pointer) \
	Common::Functor1Mem<const SavePoint&, void, class>(pointer, &class::name)

#define ADD_CALLBACK_FUNCTION(class, name) \
	_callbacks.push_back(new ENTITY_CALLBACK(class, name, this));

#define ADD_NULL_FUNCTION() \
	_callbacks.push_back(new ENTITY_CALLBACK(Entity, nullfunction, this));

//////////////////////////////////////////////////////////////////////////
// Declaration
//////////////////////////////////////////////////////////////////////////

#define DECLARE_FUNCTION(name) \
	void setup_##name(); \
	void name(const SavePoint &savepoint);

#define DECLARE_FUNCTION_1(name, param1) \
	void setup_##name(param1); \
	void name(const SavePoint &savepoint);

#define DECLARE_FUNCTION_2(name, param1, param2) \
	void setup_##name(param1, param2); \
	void name(const SavePoint &savepoint);

#define DECLARE_FUNCTION_3(name, param1, param2, param3) \
	void setup_##name(param1, param2, param3); \
	void name(const SavePoint &savepoint);

#define DECLARE_FUNCTION_4(name, param1, param2, param3, param4) \
	void setup_##name(param1, param2, param3, param4); \
	void name(const SavePoint &savepoint);

#define DECLARE_FUNCTION_NOSETUP(name) \
	void name(const SavePoint &savepoint);

#define DECLARE_NULL_FUNCTION() \
	void setup_nullfunction();

//////////////////////////////////////////////////////////////////////////
// Setup
//////////////////////////////////////////////////////////////////////////

#define IMPLEMENT_SETUP(class, callback_class, name, index) \
void class::setup_##name() { \
	BEGIN_SETUP(callback_class, name, index, EntityData::EntityParametersIIII) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::setup_" #name "()"); \
	END_SETUP() \
}

#define BEGIN_SETUP(class, name, index, type) \
	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]); \
	_data->setCurrentCallback(index); \
	_data->resetCurrentParameters<type>();

#define END_SETUP() \
	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);


//////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////

// Expose parameters and check validity
#define EXPOSE_PARAMS(type) \
	type *params = (type*)_data->getCurrentParameters(); \
	if (!params) \
		error("[EXPOSE_PARAMS] Trying to call an entity function with invalid parameters"); \


// function signature without setup (we keep the index for consistency but never use it)
#define IMPLEMENT_FUNCTION_NOSETUP(index, class, name) \
	void class::name(const SavePoint &savepoint) { \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(index=" #index ")");

// simple setup with no parameters
#define IMPLEMENT_FUNCTION(index, class, name) \
	IMPLEMENT_SETUP(class, class, name, index) \
	void class::name(const SavePoint &savepoint) { \
		EXPOSE_PARAMS(EntityData::EntityParametersIIII) \
		debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "() - action: %s", ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_END }

// nullfunction call
#define IMPLEMENT_NULL_FUNCTION(index, class) \
	IMPLEMENT_SETUP(class, Entity, nullfunction, index)

// setup with one uint parameter
#define IMPLEMENT_FUNCTION_I(index, class, name, paramType) \
	void class::setup_##name(paramType param1) { \
	BEGIN_SETUP(class, name, index, EntityData::EntityParametersIIII) \
	EntityData::EntityParametersIIII *params = (EntityData::EntityParametersIIII*)_data->getCurrentParameters(); \
	params->param1 = (unsigned int)param1; \
	END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersIIII) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d) - action: %s", params->param1, ACTION_NAME(savepoint.action));

// setup with two uint parameters
#define IMPLEMENT_FUNCTION_II(index, class, name, paramType1, paramType2) \
	void class::setup_##name(paramType1 param1, paramType2 param2) { \
		BEGIN_SETUP(class, name, index, EntityData::EntityParametersIIII) \
		EntityData::EntityParametersIIII *params = (EntityData::EntityParametersIIII*)_data->getCurrentParameters(); \
		params->param1 = param1; \
		params->param2 = param2; \
		END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersIIII) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d, %d) - action: %s", params->param1, params->param2, ACTION_NAME(savepoint.action));

// setup with three uint parameters
#define IMPLEMENT_FUNCTION_III(index, class, name, paramType1, paramType2, paramType3) \
	void class::setup_##name(paramType1 param1, paramType2 param2, paramType3 param3) { \
		BEGIN_SETUP(class, name, index, EntityData::EntityParametersIIII) \
		EntityData::EntityParametersIIII *params = (EntityData::EntityParametersIIII*)_data->getCurrentParameters(); \
		params->param1 = param1; \
		params->param2 = param2; \
		params->param3 = param3; \
		END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersIIII) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d, %d, %d) - action: %s", params->param1, params->param2, params->param3, ACTION_NAME(savepoint.action));

// setup with one char *parameter
#define IMPLEMENT_FUNCTION_S(index, class, name) \
	void class::setup_##name(const char *seq1) { \
		BEGIN_SETUP(class, name, index, EntityData::EntityParametersSIIS) \
		EntityData::EntityParametersSIIS *params = (EntityData::EntityParametersSIIS*)_data->getCurrentParameters(); \
		strncpy((char *)&params->seq1, seq1, 12); \
		END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersSIIS) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s) - action: %s", (char *)&params->seq1, ACTION_NAME(savepoint.action));

// setup with one char *parameter and one uint
#define IMPLEMENT_FUNCTION_SI(index, class, name, paramType2) \
	void class::setup_##name(const char *seq1, paramType2 param4) { \
		BEGIN_SETUP(class, name, index, EntityData::EntityParametersSIIS) \
		EntityData::EntityParametersSIIS *params = (EntityData::EntityParametersSIIS*)_data->getCurrentParameters(); \
		strncpy((char *)&params->seq1, seq1, 12); \
		params->param4 = param4; \
		END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersSIIS) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s, %d) - action: %s", (char *)&params->seq1, params->param4, ACTION_NAME(savepoint.action));

// setup with one char *parameter and two uints
#define IMPLEMENT_FUNCTION_SII(index, class, name, paramType2, paramType3) \
	void class::setup_##name(const char *seq1, paramType2 param4, paramType3 param5) { \
		BEGIN_SETUP(class, name, index, EntityData::EntityParametersSIIS) \
		EntityData::EntityParametersSIIS *params = (EntityData::EntityParametersSIIS*)_data->getCurrentParameters(); \
		strncpy((char *)&params->seq1, seq1, 12); \
		params->param4 = param4; \
		params->param5 = param5; \
		END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersSIIS) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s, %d, %d) - action: %s", (char *)&params->seq1, params->param4, params->param5, ACTION_NAME(savepoint.action));

// setup with one char *parameter and three uints
#define IMPLEMENT_FUNCTION_SIII(index, class, name, paramType2, paramType3, paramType4) \
	void class::setup_##name(const char *seq, paramType2 param4, paramType3 param5, paramType4 param6) { \
		BEGIN_SETUP(class, name, index, EntityData::EntityParametersSIII) \
		EntityData::EntityParametersSIII *params = (EntityData::EntityParametersSIII*)_data->getCurrentParameters(); \
		strncpy((char *)&params->seq, seq, 12); \
		params->param4 = param4; \
		params->param5 = param5; \
		params->param6 = param6; \
		END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersSIII) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s, %d, %d, %d) - action: %s", (char *)&params->seq, params->param4, params->param5, params->param6, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_SIIS(index, class, name, paramType2, paramType3) \
	void class::setup_##name(const char *seq1, paramType2 param4, paramType3 param5, const char *seq2) { \
		BEGIN_SETUP(class, name, index, EntityData::EntityParametersSIIS) \
		EntityData::EntityParametersSIIS *params = (EntityData::EntityParametersSIIS*)_data->getCurrentParameters(); \
		strncpy((char *)&params->seq1, seq1, 12); \
		params->param4 = param4; \
		params->param5 = param5; \
		strncpy((char *)&params->seq2, seq2, 12); \
		END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersSIIS) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s, %d, %d, %s) - action: %s", (char *)&params->seq1, params->param4, params->param5, (char *)&params->seq2, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_SS(index, class, name) \
	void class::setup_##name(const char *seq1, const char *seq2) { \
		BEGIN_SETUP(class, name, index, EntityData::EntityParametersSSII) \
		EntityData::EntityParametersSSII *params = (EntityData::EntityParametersSSII*)_data->getCurrentParameters(); \
		strncpy((char *)&params->seq1, seq1, 12); \
		strncpy((char *)&params->seq2, seq2, 12); \
		END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersSSII) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s, %s) - action: %s", (char *)&params->seq1, (char *)&params->seq2, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_SSI(index, class, name, paramType3) \
	void class::setup_##name(const char *seq1, const char *seq2, paramType3 param7) { \
		BEGIN_SETUP(class, name, index, EntityData::EntityParametersSSII) \
		EntityData::EntityParametersSSII *params = (EntityData::EntityParametersSSII*)_data->getCurrentParameters(); \
		strncpy((char *)&params->seq1, seq1, 12); \
		strncpy((char *)&params->seq2, seq2, 12); \
		params->param7 = param7; \
		END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersSSII) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%s, %s, %d) - action: %s", (char *)&params->seq1, (char *)&params->seq2, params->param7, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_IS(index, class, name, paramType) \
	void class::setup_##name(paramType param1, const char *seq) { \
		BEGIN_SETUP(class, name, index, EntityData::EntityParametersISII) \
		EntityData::EntityParametersISII *params = (EntityData::EntityParametersISII*)_data->getCurrentParameters(); \
		params->param1 = (unsigned int)param1; \
		strncpy((char *)&params->seq, seq, 12); \
		END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersISII) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d, %s) - action: %s", params->param1, (char *)&params->seq, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_ISS(index, class, name, paramType) \
	void class::setup_##name(paramType param1, const char *seq1, const char *seq2) { \
		BEGIN_SETUP(class, name, index, EntityData::EntityParametersISSI) \
		EntityData::EntityParametersISSI *params = (EntityData::EntityParametersISSI*)_data->getCurrentParameters(); \
		params->param1 = param1; \
		strncpy((char *)&params->seq1, seq1, 12); \
		strncpy((char *)&params->seq2, seq2, 12); \
		END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersISSI) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d, %s, %s) - action: %s", params->param1, (char *)&params->seq1, (char *)&params->seq2, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_IIS(index, class, name, paramType1, paramType2) \
	void class::setup_##name(paramType1 param1, paramType2 param2, const char *seq) { \
		BEGIN_SETUP(class, name, index, EntityData::EntityParametersIISI) \
		EntityData::EntityParametersIISI *params = (EntityData::EntityParametersIISI*)_data->getCurrentParameters(); \
		params->param1 = param1; \
		params->param2 = param2; \
		strncpy((char *)&params->seq, seq, 12); \
		END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersIISI) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d, %d, %s) - action: %s", params->param1, params->param2, (char *)&params->seq, ACTION_NAME(savepoint.action));

#define IMPLEMENT_FUNCTION_IISS(index, class, name, paramType1, paramType2) \
	void class::setup_##name(paramType1 param1, paramType2 param2, const char *seq1, const char *seq2) { \
		BEGIN_SETUP(class, name, index, EntityData::EntityParametersIISS) \
		EntityData::EntityParametersIISS *params = (EntityData::EntityParametersIISS*)_data->getCurrentParameters(); \
		params->param1 = param1; \
		params->param2 = param2; \
		strncpy((char *)&params->seq1, seq1, 12); \
		strncpy((char *)&params->seq2, seq2, 12); \
		END_SETUP() \
	} \
	void class::name(const SavePoint &savepoint) { \
	EXPOSE_PARAMS(EntityData::EntityParametersIISS) \
	debugC(6, kLastExpressDebugLogic, "Entity: " #class "::" #name "(%d, %d, %s, %s) - action: %s", params->param1, params->param2, (char *)&params->seq1, (char *)&params->seq2, ACTION_NAME(savepoint.action));


//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
#define RESET_ENTITY_STATE(entity, class, function) \
	getEntities()->resetState(entity); \
	((class*)getEntities()->get(entity))->function();

//////////////////////////////////////////////////////////////////////////
// Parameters macros (for default IIII parameters)
//////////////////////////////////////////////////////////////////////////
#define CURRENT_PARAM(index, id) \
	((EntityData::EntityParametersIIII*)_data->getCurrentParameters(index))->param##id

#define ENTITY_PARAM(index, id) \
	((EntityData::EntityParametersIIII*)_data->getParameters(8, index))->param##id

//////////////////////////////////////////////////////////////////////////
// Time check macros
//////////////////////////////////////////////////////////////////////////
#define TIME_CHECK(timeValue, parameter, function) \
	if (getState()->time > timeValue && !parameter) { \
		parameter = 1; \
		function(); \
		break; \
	}

#define TIME_CHECK_SAVEPOINT(timeValue, parameter, entity1, entity2, action) \
	if (getState()->time > timeValue && !parameter) { \
		parameter = 1; \
		getSavePoints()->push(entity1, entity2, action); \
	}

#define TIME_CHECK_CALLBACK(timeValue, parameter, callback, function) \
	if (getState()->time > timeValue && !parameter) { \
		parameter = 1; \
		setCallback(callback); \
		function(); \
		break; \
	}

#define TIME_CHECK_CALLBACK_1(timeValue, parameter, callback, function, param1) \
	if (getState()->time > timeValue && !parameter) { \
		parameter = 1; \
		setCallback(callback); \
		function(param1); \
		break; \
	}

#define TIME_CHECK_CALLBACK_2(timeValue, parameter, callback, function, param1, param2) \
	if (getState()->time > timeValue && !parameter) { \
		parameter = 1; \
		setCallback(callback); \
		function(param1, param2); \
		break; \
	}

#define TIME_CHECK_CALLBACK_3(timeValue, parameter, callback, function, param1, param2, param3) \
	if (getState()->time > timeValue && !parameter) { \
		parameter = 1; \
		setCallback(callback); \
		function(param1, param2, param3); \
		break; \
	}

#define TIME_CHECK_CALLBACK_INVENTORY(timeValue, parameter, callback, function) \
	if (getState()->time > timeValue && !parameter) { \
	parameter = 1; \
	getData()->inventoryItem = kItemNone; \
	setCallback(callback); \
	function(); \
	break; \
	}

#define TIME_CHECK_CALLBACK_ACTION(timeValue, parameter) \
	if (getState()->time > timeValue && !parameter) { \
		parameter = 1; \
		CALLBACK_ACTION(); \
		break; \
	}

#define TIME_CHECK_PLAYSOUND_UPDATEPOSITION(timeValue, parameter, callback, sound, position) \
	if (getState()->time > timeValue && !parameter) { \
		parameter = 1; \
		getData()->entityPosition = position; \
		setCallback(callback); \
		setup_playSound(sound); \
		break; \
	}

#define TIME_CHECK_OBJECT(timeValue, parameter, object, location) \
	if (getState()->time > timeValue && !parameter) { \
		parameter = 1; \
		getObjects()->updateLocation2(object, location); \
	}

#define TIME_CHECK_CAR(timeValue, parameter, callback, function) {\
	if ((getState()->time <= timeValue && !getEntities()->isPlayerInCar(kCarGreenSleeping)) || !parameter) \
		parameter = (uint)getState()->time + 75; \
	if (getState()->time > timeValue || parameter < getState()->time) { \
		parameter = kTimeInvalid; \
		setCallback(callback); \
		function(); \
		break; \
	} \
}

//////////////////////////////////////////////////////////////////////////
// Callback action
//////////////////////////////////////////////////////////////////////////
#define CALLBACK_ACTION() { \
	if (getData()->currentCall == 0) \
		error("[CALLBACK_ACTION] currentCall is already 0, cannot proceed"); \
	getData()->currentCall--; \
	getSavePoints()->setCallback(_entityIndex, _callbacks[_data->getCurrentCallback()]); \
	getSavePoints()->call(_entityIndex, _entityIndex, kActionCallback); \
	}

//////////////////////////////////////////////////////////////////////////
// Param update
//////////////////////////////////////////////////////////////////////////
#define UPDATE_PARAM(parameter, type, value) { \
	if (!parameter) \
		parameter = (uint)(type + value); \
	if (parameter >= type) \
		break; \
	parameter = kTimeInvalid; \
}

// Todo: replace with UPDATE_PARAM_PROC as appropriate
#define UPDATE_PARAM_GOTO(parameter, type, value, label) { \
	if (!parameter) \
		parameter = (uint)(type + value); \
	if (parameter >= type) \
		goto label; \
	parameter = kTimeInvalid; \
}

// Updating parameter with code inside the check
#define UPDATE_PARAM_PROC(parameter, type, value) \
	if (!parameter) \
		parameter = (uint)(type + value); \
	if (parameter < type) { \
		parameter = kTimeInvalid;

#define UPDATE_PARAM_PROC_TIME(timeValue, test, parameter, value) \
	if (getState()->time <= timeValue) { \
		if (test || !parameter) \
			parameter = (uint)(getState()->time + value); \
	} \
	if (parameter < getState()->time || getState()->time > timeValue) { \
		parameter = kTimeInvalid;

#define UPDATE_PARAM_PROC_END }

// Updating parameter with an added check (and code inside the check)
#define UPDATE_PARAM_CHECK(parameter, type, value) \
	if (!parameter || parameter < type) { \
		if (!parameter) \
			parameter = (uint)(type + value);

//////////////////////////////////////////////////////////////////////////
// Compartments
//////////////////////////////////////////////////////////////////////////
// Go from one compartment to another (or the same one if no optional args are passed
#define COMPARTMENT_TO(class, compartmentFrom, positionFrom, sequenceFrom, sequenceTo) \
	switch (savepoint.action) { \
	default: \
		break; \
	case kActionDefault: \
		getData()->entityPosition = positionFrom; \
		setCallback(1); \
		setup_enterExitCompartment(sequenceFrom, compartmentFrom); \
		break; \
	case kActionCallback: \
		switch (getCallback()) { \
		default: \
			break; \
		case 1: \
			setCallback(2); \
			setup_enterExitCompartment(sequenceTo, compartmentFrom); \
			break; \
		case 2: \
			getData()->entityPosition = positionFrom; \
			getEntities()->clearSequences(_entityIndex); \
			CALLBACK_ACTION(); \
		} \
		break; \
	}

#define COMPARTMENT_FROM_TO(class, compartmentFrom, positionFrom, sequenceFrom, compartmentTo, positionTo, sequenceTo) \
	switch (savepoint.action) { \
	default: \
		break; \
	case kActionDefault: \
		getData()->entityPosition = positionFrom; \
		getData()->location = kLocationOutsideCompartment; \
		setCallback(1); \
		setup_enterExitCompartment(sequenceFrom, compartmentFrom); \
		break; \
	case kActionCallback: \
		switch (getCallback()) { \
		default: \
			break; \
		case 1: \
			setCallback(2); \
			setup_updateEntity(kCarGreenSleeping, positionTo); \
			break; \
		case 2: \
			setCallback(3); \
			setup_enterExitCompartment(sequenceTo, compartmentTo); \
			break; \
		case 3: \
			getData()->location = kLocationInsideCompartment; \
			getEntities()->clearSequences(_entityIndex); \
			CALLBACK_ACTION(); \
			break; \
		} \
		break; \
	}

} // End of namespace LastExpress

#endif // LASTEXPRESS_ENTITY_INTERN_H
