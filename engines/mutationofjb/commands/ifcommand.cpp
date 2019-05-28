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

#include "mutationofjb/commands/ifcommand.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/script.h"
#include "common/str.h"

/** @file
 * "IF" <tag> <sceneId> <objectId> <value> ["!"]
 *
 * IF command compares the value of the WX pseudo-register of the object in the specified scene.
 * If the values match, execution continues to the next line.
 * Otherwise execution continues after first "#ELSE" or "=ELSE" with the same <tag>.
 * The logic can be reversed with exclamation mark at the end.
 *
 * <tag> is always 1 character long, <sceneId> and <objectId> 2 characters long.
 *
 * Please note that this does not work like you are used to from saner languages.
 * IF does not have any blocks. It only searches for first #ELSE, so you can have stuff like:
 *   IF something
 *   IF something else
 *   #ELSE
 *   ...
 * This is effectively logical AND.
 */

namespace MutationOfJB {

bool IfCommandParser::parse(const Common::String &line, ScriptParseContext &, Command *&command) {
	// IFtss oo val!
	// <t>   1B Tag.
	// <ss>  2B Scene.
	// <oo>  2B Object ID.
	// <val> VL Value.
	// !     1B Negation (optional).

	if (line.size() < 10) {
		return false;
	}

	if (!line.hasPrefix("IF")) {
		return false;
	}

	const char *const cstr = line.c_str();
	const char tag = cstr[2] == ' ' ? 0 : cstr[2];
	const uint8 sceneId = atoi(cstr + 3);
	const uint8 objectId = atoi(cstr + 6);
	const uint8 value = atoi(cstr + 9);
	const bool negative = (line.lastChar() == '!');

	_tags.push(tag);

	command = new IfCommand(sceneId, objectId, value, negative);

	return true;
}


IfCommand::IfCommand(uint8 sceneId, uint8 objectId, uint16 value, bool negative) :
	_sceneId(sceneId),
	_objectId(objectId),
	_value(value),
	_negative(negative) {}

Command::ExecuteResult IfCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	Scene *const scene = scriptExecCtx.getGameData().getScene(_sceneId);
	if (!scene) {
		return Finished;
	}

	Object *const object = scene->getObject(_objectId, true);
	if (!object) {
		return Finished;
	}

	_cachedResult = (object->_WX == _value);
	if (_negative) {
		_cachedResult = !_cachedResult;
	}

	return Finished;
}

Common::String IfCommand::debugString() const {
	return Common::String::format("IF scene%d.object%d.WX %s %d", _sceneId, _objectId, _negative ? "!=" : "==", _value);
}

}
