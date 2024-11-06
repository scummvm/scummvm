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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "common/crc.h"

#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/gfx/3deffect_params.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
Effect3DParams::Effect3DParam::Effect3DParam() {
	setDefaultValues();
}

//////////////////////////////////////////////////////////////////////////
Effect3DParams::Effect3DParam::Effect3DParam(const char *paramName) {
	setDefaultValues();
	_paramName = paramName;
}

//////////////////////////////////////////////////////////////////////////
Effect3DParams::Effect3DParam::~Effect3DParam() {
}

//////////////////////////////////////////////////////////////////////////
void Effect3DParams::Effect3DParam::setValue(char *val) {
	_type = EP_STRING;
	_valString = val;
}

//////////////////////////////////////////////////////////////////////////
void Effect3DParams::Effect3DParam::setValue(int val) {
	_type = EP_INT;
	_valInt = val;
}

//////////////////////////////////////////////////////////////////////////
void Effect3DParams::Effect3DParam::setValue(float val) {
	_type = EP_FLOAT;
	_valFloat = val;
}

//////////////////////////////////////////////////////////////////////////
void Effect3DParams::Effect3DParam::setValue(bool val) {
	_type = EP_BOOL;
	_valBool = val;
}

//////////////////////////////////////////////////////////////////////////
void Effect3DParams::Effect3DParam::setValue(DXVector4 val) {
	_type = EP_VECTOR;
	_valVector = val;
}

//////////////////////////////////////////////////////////////////////////
void Effect3DParams::Effect3DParam::setDefaultValues() {
	_paramName = "";
	_valString = "";
	_valInt = 0;
	_valFloat = 0;
	_valBool = 0;
	_valVector = DXVector4(0, 0, 0, 0);

	_type = EP_UNKNOWN;

	_initialized = false;
}

//////////////////////////////////////////////////////////////////////////
bool Effect3DParams::Effect3DParam::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferString(TMEMBER(_paramName));
	persistMgr->transferSint32(TMEMBER_INT(_type));
	persistMgr->transferString(TMEMBER(_valString));
	persistMgr->transferSint32(TMEMBER(_valInt));
	persistMgr->transferFloat(TMEMBER(_valFloat));
	persistMgr->transferVector4d(TMEMBER(_valVector));
	persistMgr->transferBool(TMEMBER(_valBool));

	if (!persistMgr->getIsSaving()) {
		_initialized = false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
Effect3DParams::Effect3DParams() {
}

//////////////////////////////////////////////////////////////////////////
Effect3DParams::~Effect3DParams() {
	clear();
}

//////////////////////////////////////////////////////////////////////////
void Effect3DParams::clear() {
	for (size_t i = 0; i < _params.size(); i++) {
		delete _params[i];
		_params[i] = nullptr;
	}

	_params.clear();
}

//////////////////////////////////////////////////////////////////////////
bool Effect3DParams::persist(BasePersistenceManager *persistMgr) {
	if (persistMgr->getIsSaving()) {
		uint32 numItems = _params.size();
		persistMgr->transferUint32(TMEMBER(numItems));

		for (uint32 i = 0; i < numItems; i++) {
			_params[i]->persist(persistMgr);
		}
	} else {
		uint32 numItems = 0;
		persistMgr->transferUint32(TMEMBER(numItems));

		for (uint32 i = 0; i < numItems; i++) {
			Effect3DParam *param = new Effect3DParam();
			param->persist(persistMgr);

			_params.add(param);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void Effect3DParams::setParam(const char *paramName, ScValue *val) {
	Effect3DParam *param = getParamByName(paramName);

	switch (val->getType()) {
	case VAL_INT:
		param->setValue(val->getInt());
	break;
	case VAL_FLOAT:
		param->setValue((float)val->getFloat());
	break;
	case VAL_BOOL:
		param->setValue(val->getBool());
	break;
	default:
		param->setValue(val->getString());
	}
}

//////////////////////////////////////////////////////////////////////////
void Effect3DParams::setParam(const char *paramName, DXVector4 val) {
	Effect3DParam *param = getParamByName(paramName);
	param->setValue(val);
}

//////////////////////////////////////////////////////////////////////////
Effect3DParams::Effect3DParam *Effect3DParams::getParamByName(const char *paramName) {
	Effect3DParam *param = nullptr;

	for (uint32 i = 0; i < _params.size(); i++) {
		if (_params[i]->getParamName() && strcmp(paramName, _params[i]->getParamName()) == 0) {
			param = _params[i];
			break;
		}
	}

	if (!param) {
		param = new Effect3DParam(paramName);
		_params.add(param);
	}

	return param;
}

} // namespace Wintermute
