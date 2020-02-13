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

#include "mutationofjb/commands/seqcommand.h"
#include "mutationofjb/gamedata.h"

namespace MutationOfJB {

union ChangeCommandValue {
	uint8 _byteVal;
	uint16 _wordVal;
	char _strVal[MAX_ENTITY_NAME_LENGTH + 1];
};

class ChangeCommand : public SeqCommand {
public:
	enum ChangeRegister {
		NM, // Name
		LT, // Destination scene ID
		SX, // Destination X
		SY, // Destination Y
		XX, // X
		YY, // Y
		XL, // Width
		YL, // Height
		WX, // Walk to X
		WY, // Walk to Y
		SP, //
		AC, // Active
		FA, // First animation
		FR,
		NA,
		FS,
		CA,
		DS, // Startup
		DL,
		ND, // Number of doors
		NO, // Number of objects
		NS, // Number of statics
		PF, // Palette rotation first
		PL, // Palette rotation last
		PD  // Palette rotation delay
	};

	enum ChangeOperation {
		SetValue,
		AddValue,
		SubtractValue
	};

	ChangeCommand(uint8 sceneId, uint8 entityId, ChangeRegister reg, ChangeOperation op, const ChangeCommandValue &val) :
		_sceneId(sceneId), _entityId(entityId), _register(reg), _operation(op), _value(val)
	{}
protected:
	const char *getRegisterAsString() const;
	Common::String getValueAsString() const;
	const char *getOperationAsString() const;

	uint8 _sceneId;
	uint8 _entityId;
	ChangeRegister _register;
	ChangeOperation _operation;
	ChangeCommandValue _value;
};

class ChangeCommandParser : public SeqCommandParser {
protected:
	bool parseValueString(const Common::String &valueString, bool changeEntity, uint8 &sceneId, uint8 &entityId, ChangeCommand::ChangeRegister &reg, ChangeCommand::ChangeOperation &op, ChangeCommandValue &ccv);
	int parseInteger(const char *val, ChangeCommand::ChangeOperation &op);
};

class ChangeObjectCommandParser : public ChangeCommandParser {
public:
	bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) override;
};

class ChangeDoorCommandParser : public ChangeCommandParser {
public:
	bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) override;
};

class ChangeStaticCommandParser : public ChangeCommandParser {
public:
	bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) override;
};

class ChangeSceneCommandParser : public ChangeCommandParser {
public:
	bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) override;
};

class ChangeDoorCommand : public ChangeCommand {
public:
	ChangeDoorCommand(uint8 sceneId, uint8 doorId, ChangeRegister reg, ChangeOperation op, const ChangeCommandValue &val)
		: ChangeCommand(sceneId, doorId, reg, op, val)
	{}
	ExecuteResult execute(ScriptExecutionContext &scriptExecCtx) override;
	Common::String debugString() const override;
};

class ChangeObjectCommand : public ChangeCommand {
public:
	ChangeObjectCommand(uint8 sceneId, uint8 objectId, ChangeRegister reg, ChangeOperation op, const ChangeCommandValue &val)
		: ChangeCommand(sceneId, objectId, reg, op, val)
	{}
	ExecuteResult execute(ScriptExecutionContext &scriptExecCtx) override;
	Common::String debugString() const override;
};

class ChangeStaticCommand : public ChangeCommand {
public:
	ChangeStaticCommand(uint8 sceneId, uint8 staticId, ChangeRegister reg, ChangeOperation op, const ChangeCommandValue &val)
		: ChangeCommand(sceneId, staticId, reg, op, val)
	{}
	ExecuteResult execute(ScriptExecutionContext &scriptExecCtx) override;
	Common::String debugString() const override;
};

class ChangeSceneCommand : public ChangeCommand {
public:
	ChangeSceneCommand(uint8 sceneId, uint8 staticId, ChangeRegister reg, ChangeOperation op, const ChangeCommandValue &val)
		: ChangeCommand(sceneId, staticId, reg, op, val)
	{}
	ExecuteResult execute(ScriptExecutionContext &scriptExecCtx) override;
	Common::String debugString() const override;
};

}
