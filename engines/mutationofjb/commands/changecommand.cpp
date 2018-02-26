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

#include "mutationofjb/commands/changecommand.h"
#include "common/translation.h"

namespace MutationOfJB {

// CHANGEe rr ss ii val
// <e>   1B  Entity to change register for.
//           D door
//           O object
//           S static
// <rr>  2B  Register name.
// <ss>  2B  Scene ID.
// <ii>  2B  Entity ID.
// <val> VL  Value.

bool ChangeCommandParser::parseValueString(const Common::String &valueString, uint8 &sceneId, uint8 &entityId, ChangeCommand::ChangeRegister &reg, ChangeCommand::ChangeOperation &op, ChangeCommandValue &ccv) {
	if (valueString.size() < 8) {
		return false;
	}

	sceneId = atoi(valueString.c_str() + 3);
	entityId = atoi(valueString.c_str() + 6);
	const char *val = nullptr;
	if (valueString.size() >= 9) {
		val = valueString.c_str() + 9;
	}

	if (valueString.hasPrefix("NM")) {
		reg = ChangeCommand::NM;
		op = ChangeCommand::SetValue;
		strncpy(ccv._strVal, val, MAX_STR_LENGTH);
	} else if (valueString.hasPrefix("LT")) {
		reg = ChangeCommand::LT;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("SX")) {
		reg = ChangeCommand::SX;
		ccv._wordVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("SY")) {
		reg = ChangeCommand::SY;
		ccv._wordVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("XX")) {
		reg = ChangeCommand::XX;
		ccv._wordVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("YY")) {
		reg = ChangeCommand::YY;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("XL")) {
		reg = ChangeCommand::XL;
		ccv._wordVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("YL")) {
		reg = ChangeCommand::YL;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("WX")) {
		reg = ChangeCommand::WX;
		ccv._wordVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("WY")) {
		reg = ChangeCommand::WY;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("AC")) {
		reg = ChangeCommand::AC;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("FA")) {
		reg = ChangeCommand::FA;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("FR")) {
		reg = ChangeCommand::FR;
		ccv._byteVal = parseInteger(val, op); 
	} else if (valueString.hasPrefix("NA")) {
		reg = ChangeCommand::NA;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("FS")) {
		reg = ChangeCommand::FS;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("CA")) {
		reg = ChangeCommand::CA;
		ccv._byteVal = parseInteger(val, op);
	}

	return true;
}


bool ChangeDoorCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (!line.hasPrefix("CHANGED ")) {
		return false;
	}
	uint8 sceneId = 0;
	uint8 objectId = 0;
	ChangeCommand::ChangeRegister reg;
	ChangeCommand::ChangeOperation op;
	ChangeCommandValue val;
	if (!parseValueString(line.c_str() + 8, sceneId, objectId, reg, op, val)) {
		return false;
	}

	command = new ChangeObjectCommand(sceneId, objectId, reg, op, val);
	return true;
}

bool ChangeObjectCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (!line.hasPrefix("CHANGEO ")) {
		return false;
	}
	uint8 sceneId = 0;
	uint8 objectId = 0;
	ChangeCommand::ChangeRegister reg;
	ChangeCommand::ChangeOperation op;
	ChangeCommandValue val;
	if (!parseValueString(line.c_str() + 8, sceneId, objectId, reg, op, val)) {
		return false;
	}

	command = new ChangeObjectCommand(sceneId, objectId, reg, op, val);
	return true;
}

bool ChangeStaticCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (!line.hasPrefix("CHANGES ")) {
		return false;
	}
	uint8 sceneId = 0;
	uint8 objectId = 0;
	ChangeCommand::ChangeRegister reg;
	ChangeCommand::ChangeOperation op;
	ChangeCommandValue val;
	if (!parseValueString(line.c_str() + 8, sceneId, objectId, reg, op, val)) {
		return false;
	}

	command = new ChangeObjectCommand(sceneId, objectId, reg, op, val);
	return true;
}

int ChangeCommandParser::parseInteger(const char *val, ChangeCommand::ChangeOperation &op) {
	if (!val || !(*val)) {
		op = ChangeCommand::SetValue;
		return 0;
	}

	if (val[0] == '\\') {
		op = ChangeCommand::SetValue;
		val++;
	} else if (val[0] == '+') {
		op = ChangeCommand::AddValue;
		val++;
	} else if (val[0] == '-') {
		op = ChangeCommand::SubtractValue;
		val++;
	}

	return atoi(val);
}

Command::ExecuteResult ChangeDoorCommand::execute(GameData &gameData) {
	Scene *const scene = gameData.getScene(_sceneId);
	if (!scene) {
		return Finished;
	}

	Door *const door = scene->getDoor(_entityId);
	if (!door) {
		return Finished;
	}

	switch (_register) {
	case NM:
		strncpy(door->_name, _value._strVal, MAX_STR_LENGTH);
		break;
	case LT:
		door->_destSceneId = _value._byteVal;
		break;
	case SX:
		door->_destX = _value._wordVal;
		break;
	case SY:
		door->_destY = _value._wordVal;
		break;
	case XX:
		door->_x = _value._wordVal;
		break;
	case YY:
		door->_y = _value._byteVal;
		break;
	case XL:
		door->_width = _value._wordVal;
		break;
	case YL:
		door->_height = _value._byteVal;
		break;
	case WX:
		door->_walkToX = _value._wordVal;
		break;
	case WY:
		door->_walkToY = _value._byteVal;
		break;
	case SP:
		door->_SP = _value._byteVal;
		break;
	default:
		warning("Object does not support changing this register.");
		break;
	}

	return Finished;
}

Command::ExecuteResult ChangeObjectCommand::execute(GameData &gameData) {
	Scene *const scene = gameData.getScene(_sceneId);
	if (!scene) {
		return Finished;
	}

	Object *const object = scene->getObject(_entityId);
	if (!object) {
		return Finished;
	}

	switch (_register) {
	case AC:
		object->_AC = _value._byteVal;
		break;
	case FA:
		object->_FA = _value._byteVal;
		break;
	case FR:
		object->_FR = _value._byteVal;
		break;
	case NA:
		object->_NA = _value._byteVal;
		break;
	case FS:
		object->_FS = _value._byteVal;
		break;
	case CA:
		object->_CA = _value._byteVal;
		break;
	case XX:
		object->_x = _value._wordVal;
		break;
	case YY:
		object->_y = _value._byteVal;
		break;
	case XL:
		object->_XL = _value._wordVal;
		break;
	case YL:
		object->_YL = _value._byteVal;
		break;
	case WX:
		object->_WX = _value._wordVal;
		break;
	case WY:
		object->_WY = _value._byteVal;
		break;
	case SP:
		object->_SP = _value._byteVal;
		break;
	default:
		warning("Object does not support changing this register.");
		break;
	}

	return Finished;
}

Command::ExecuteResult ChangeStaticCommand::execute(GameData &gameData) {
	Scene *const scene = gameData.getScene(_sceneId);
	if (!scene) {
		return Finished;
	}

	Static *const stat = scene->getStatic(_entityId);
	if (!stat) {
		return Finished;
	}

	switch (_register) {
	case AC:
		stat->_active = _value._byteVal;
		break;
	case NM:
		strncpy(stat->_name, _value._strVal, MAX_STR_LENGTH);
		break;
	case XX:
		stat->_x = _value._wordVal;
		break;
	case YY:
		stat->_y = _value._byteVal;
		break;
	case XL:
		stat->_width = _value._wordVal;
		break;
	case YL:
		stat->_height = _value._byteVal;
		break;
	case WX:
		stat->_walkToX = _value._wordVal;
		break;
	case WY:
		stat->_walkToY = _value._byteVal;
		break;
	case SP:
		stat->_SP = _value._byteVal;
		break;
	default:
		warning("Object does not support changing this register.");
		break;
	}

	return Finished;
}
}
