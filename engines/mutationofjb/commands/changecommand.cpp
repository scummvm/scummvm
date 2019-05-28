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
#include "mutationofjb/script.h"
#include "mutationofjb/gamedata.h"

/** @file
 * "CHANGE" <entity> " " <register> " " <sceneId> " " <entityId> " " <value>
 *
 * Changes entity register value for specified scene.
 * <entity>   1B  Entity to change register for.
 *                Possible values:
 *                  'D' - door
 *                  'O' - object
 *                  'S' - static
 *                  ''  - scene
 * <register> 2B  Register name.
 * <sceneId>  2B  Scene ID.
 * <entityid> 2B  Entity ID.
 * <value>    *B  Value (variable length).
 */

namespace MutationOfJB {

bool ChangeCommandParser::parseValueString(const Common::String &valueString, bool changeEntity, uint8 &sceneId, uint8 &entityId, ChangeCommand::ChangeRegister &reg, ChangeCommand::ChangeOperation &op, ChangeCommandValue &ccv) {
	if (changeEntity) {
		if (valueString.size() < 8) {
			return false;
		}
	} else {
		if (valueString.size() < 7) {
			return false;
		}
	}

	sceneId = atoi(valueString.c_str() + 3);
	if (changeEntity) {
		entityId = atoi(valueString.c_str() + 6);
	}
	const char *val = "";
	if (changeEntity) {
		if (valueString.size() >= 9) {
			val = valueString.c_str() + 9;
		}
	} else {
		if (valueString.size() >= 6) {
			val = valueString.c_str() + 6;
		}
	}

	if (valueString.hasPrefix("NM")) {
		reg = ChangeCommand::NM;
		op = ChangeCommand::SetValue;
		strncpy(ccv._strVal, val, MAX_ENTITY_NAME_LENGTH);
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
	} else if (valueString.hasPrefix("DS")) {
		reg = ChangeCommand::DS;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("DL")) {
		reg = ChangeCommand::DL;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("ND")) {
		reg = ChangeCommand::ND;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("NO")) {
		reg = ChangeCommand::NO;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("NS")) {
		reg = ChangeCommand::NS;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("PF")) {
		reg = ChangeCommand::PF;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("PL")) {
		reg = ChangeCommand::PL;
		ccv._byteVal = parseInteger(val, op);
	} else if (valueString.hasPrefix("PD")) {
		reg = ChangeCommand::PD;
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
	if (!parseValueString(line.c_str() + 8, true, sceneId, objectId, reg, op, val)) {
		return false;
	}

	command = new ChangeDoorCommand(sceneId, objectId, reg, op, val);
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
	if (!parseValueString(line.c_str() + 8, true, sceneId, objectId, reg, op, val)) {
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
	if (!parseValueString(line.c_str() + 8, true, sceneId, objectId, reg, op, val)) {
		return false;
	}

	command = new ChangeStaticCommand(sceneId, objectId, reg, op, val);
	return true;
}

bool ChangeSceneCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	if (!line.hasPrefix("CHANGE ")) {
		return false;
	}
	uint8 sceneId = 0;
	uint8 objectId = 0;
	ChangeCommand::ChangeRegister reg;
	ChangeCommand::ChangeOperation op;
	ChangeCommandValue val;
	if (!parseValueString(line.c_str() + 7, false, sceneId, objectId, reg, op, val)) {
		return false;
	}

	command = new ChangeSceneCommand(sceneId, objectId, reg, op, val);
	return true;
}

int ChangeCommandParser::parseInteger(const char *val, ChangeCommand::ChangeOperation &op) {
	op = ChangeCommand::SetValue;

	if (!val || !(*val)) {
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


const char *ChangeCommand::getRegisterAsString() const {
	switch (_register) {
	case NM:
		return "NM";
	case LT:
		return "LT";
	case SX:
		return "SX";
	case SY:
		return "SY";
	case XX:
		return "XX";
	case YY:
		return "YY";
	case XL:
		return "XL";
	case YL:
		return "YL";
	case WX:
		return "WX";
	case WY:
		return "WY";
	case SP:
		return "SP";
	case AC:
		return "AC";
	case FA:
		return "FA";
	case FR:
		return "FR";
	case NA:
		return "NA";
	case FS:
		return "FS";
	case CA:
		return "CA";
	case DS:
		return "DS";
	case DL:
		return "DL";
	case ND:
		return "ND";
	case NO:
		return "NO";
	case NS:
		return "NS";
	case PF:
		return "PF";
	case PL:
		return "PL";
	case PD:
		return "PD";
	default:
		return "(unknown)";
	}
}

Common::String ChangeCommand::getValueAsString() const {
	switch (_register) {
	case NM:
		return Common::String::format("\"%s\"", _value._strVal);
	case LT:
	case YY:
	case YL:
	case WY:
	case SP:
	case AC:
	case FA:
	case FR:
	case NA:
	case FS:
	case CA:
	case DS:
	case DL:
	case ND:
	case NO:
	case NS:
	case PF:
	case PL:
	case PD:
		return Common::String::format("%d", static_cast<int>(_value._byteVal));
	case SX:
	case SY:
	case XX:
	case XL:
	case WX:
		return Common::String::format("%d", static_cast<int>(_value._wordVal));
	default:
		return "(unknown)";
	}
}

const char *ChangeCommand::getOperationAsString() const {
	switch (_operation) {
	case SetValue:
		return "=";
	case AddValue:
		return "+=";
	case SubtractValue:
		return "-=";
	default:
		return "(unknown)";
	}
}

Command::ExecuteResult ChangeDoorCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	Scene *const scene = scriptExecCtx.getGameData().getScene(_sceneId);
	if (!scene) {
		return Finished;
	}

	Door *const door = scene->getDoor(_entityId);
	if (!door) {
		return Finished;
	}

	switch (_register) {
	case NM:
		strncpy(door->_name, _value._strVal, MAX_ENTITY_NAME_LENGTH);
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

Common::String ChangeDoorCommand::debugString() const {
	return Common::String::format("SCENE%d.DOOR%d.%s %s %s", _sceneId, _entityId, getRegisterAsString(), getOperationAsString(), getValueAsString().c_str());
}

Command::ExecuteResult ChangeObjectCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	Scene *const scene = scriptExecCtx.getGameData().getScene(_sceneId);
	if (!scene) {
		return Finished;
	}

	Object *const object = scene->getObject(_entityId, true);
	if (!object) {
		return Finished;
	}

	switch (_register) {
	case AC:
		object->_active = _value._byteVal;
		break;
	case FA:
		object->_firstFrame = _value._byteVal;
		break;
	case FR:
		object->_randomFrame = _value._byteVal;
		break;
	case NA:
		object->_numFrames = _value._byteVal;
		break;
	case FS:
		object->_roomFrameLSB = _value._byteVal;
		break;
	case CA:
		object->_currentFrame = _value._byteVal;
		break;
	case XX:
		object->_x = _value._wordVal;
		break;
	case YY:
		object->_y = _value._byteVal;
		break;
	case XL:
		object->_width = _value._wordVal;
		break;
	case YL:
		object->_height = _value._byteVal;
		break;
	case WX:
		object->_WX = _value._wordVal;
		break;
	case WY:
		object->_roomFrameMSB = _value._byteVal;
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

Common::String ChangeObjectCommand::debugString() const {
	return Common::String::format("SCENE%d.OBJECT%d.%s %s %s", _sceneId, _entityId, getRegisterAsString(), getOperationAsString(), getValueAsString().c_str());
}

Command::ExecuteResult ChangeStaticCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	Scene *const scene = scriptExecCtx.getGameData().getScene(_sceneId);
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
		strncpy(stat->_name, _value._strVal, MAX_ENTITY_NAME_LENGTH);
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
		stat->_walkToFrame = _value._byteVal;
		break;
	default:
		warning("Object does not support changing this register.");
		break;
	}

	return Finished;
}

Common::String ChangeStaticCommand::debugString() const {
	return Common::String::format("SCENE%d.STATIC%d.%s %s %s", _sceneId, _entityId, getRegisterAsString(), getOperationAsString(), getValueAsString().c_str());
}

Command::ExecuteResult ChangeSceneCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	Scene *const scene = scriptExecCtx.getGameData().getScene(_sceneId);
	if (!scene) {
		return Finished;
	}

	switch (_register) {
	case DS:
		scene->_startup = _value._byteVal;
		break;
	case DL:
		scene->_delay = _value._byteVal;
		break;
	case ND:
		scene->_noDoors = _value._byteVal;
		break;
	case NO:
		scene->_noObjects = _value._byteVal;
		break;
	case NS:
		scene->_noStatics = _value._byteVal;
		break;
	case PF:
		scene->_palRotFirst = _value._byteVal;
		break;
	case PL:
		scene->_palRotLast = _value._byteVal;
		break;
	case PD:
		scene->_palRotDelay = _value._byteVal;
		break;
	default:
		warning("Scene does not support changing this register.");
		break;
	}

	return Finished;
}

Common::String ChangeSceneCommand::debugString() const {
	return Common::String::format("SCENE%d.%s %s %s", _sceneId, getRegisterAsString(), getOperationAsString(), getValueAsString().c_str());
}

}
