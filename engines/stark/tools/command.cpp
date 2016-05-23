/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/tools/command.h"

#include "common/debug.h"

namespace Stark {
namespace Tools {

Command::Command(Command *command) {
	_index = command->_index;
	_subType = command->_subType;
	_subTypeDesc = command->_subTypeDesc;
	_arguments = command->_arguments;
}

Command::Command(Resources::Command *resource) {
	_index = resource->getIndex();
	_subType = (Resources::Command::SubType) resource->getSubType();
	_subTypeDesc = searchSubTypeDesc(_subType);
	_arguments = resource->getArguments();
}

const Command::SubTypeDesc *Command::searchSubTypeDesc(Resources::Command::SubType subType) {
	static const SubTypeDesc typeNames[] = {
			{ Resources::Command::kCommandBegin,               "begin",                      kFlowNormal },
			{ Resources::Command::kCommandEnd,                 "end",                        kFlowEnd    },
			{ Resources::Command::kScriptCall,                 "scriptCall",                 kFlowNormal },
			{ Resources::Command::kDialogCall,                 "dialogCall",                 kFlowNormal },
			{ Resources::Command::kSetInteractiveMode,         "setInteractiveMode",         kFlowNormal },
			{ Resources::Command::kLocationGoTo,               "locationGoTo",               kFlowEnd    },
			{ Resources::Command::kWalkTo,                     "walkTo",                     kFlowBranch },
			{ Resources::Command::kGameLoop,                   "gameLoop",                   kFlowNormal },
			{ Resources::Command::kScriptPause,                "scriptPause",                kFlowNormal },
			{ Resources::Command::kScriptPauseRandom,          "scriptPauseRandom",          kFlowNormal },
			{ Resources::Command::kScriptAbort,                "scriptAbort",                kFlowNormal },
			{ Resources::Command::kExit2DLocation,             "exit2DLocation",             kFlowEnd    },
			{ Resources::Command::kGoto2DLocation,             "goto2DLocation",             kFlowEnd    },
			{ Resources::Command::kRumbleScene,                "rumbleScene",                kFlowNormal },
			{ Resources::Command::kFadeScene,                  "fadeScene",                  kFlowNormal },
			{ Resources::Command::kLocationGoToNewCD,          "locationGoToNewCD",          kFlowEnd    },
			{ Resources::Command::kGameEnd,                    "gameEnd",                    kFlowNormal },
			{ Resources::Command::kInventoryOpen,              "inventoryOpen",              kFlowNormal },
			{ Resources::Command::kBookOfSecretsOpen,          "bookOfSecretsOpen",          kFlowNormal },
			{ Resources::Command::kItem3DPlaceOn,              "item3DPlaceOn",              kFlowNormal },
			{ Resources::Command::kItem3DWalkTo,               "item3DWalkTo",               kFlowNormal },
			{ Resources::Command::kItem3DFollowPath,           "item3DFollowPath",           kFlowNormal },
			{ Resources::Command::kItemLookAt,                 "itemLookAt",                 kFlowNormal },
			{ Resources::Command::kItem2DFollowPath,           "item2DFollowPath",           kFlowNormal },
			{ Resources::Command::kItemEnable,                 "itemEnable",                 kFlowNormal },
			{ Resources::Command::kItemSetActivity,            "itemSetActivity",            kFlowNormal },
			{ Resources::Command::kItemSelectInInventory,      "itemSelectInInventory",      kFlowNormal },
			{ Resources::Command::kUseAnimHierarchy,           "useAnimHierarchy",           kFlowNormal },
			{ Resources::Command::kPlayAnimation,              "playAnimation",              kFlowNormal },
			{ Resources::Command::kScriptEnable,               "scriptEnable",               kFlowNormal },
			{ Resources::Command::kShowPlay,                   "showPlay",                   kFlowNormal },
			{ Resources::Command::kKnowledgeSetBoolean,        "knowledgeSetBoolean",        kFlowNormal },
			{ Resources::Command::kKnowledgeSetInteger,        "knowledgeSetInteger",        kFlowNormal },
			{ Resources::Command::kKnowledgeAddInteger,        "knowledgeAddInteger",        kFlowNormal },
			{ Resources::Command::kEnableFloorField,           "enableFloorField",           kFlowNormal },
			{ Resources::Command::kPlayAnimScriptItem,         "playAnimScriptItem",         kFlowNormal },
			{ Resources::Command::kKnowledgeAssignBool,        "knowledgeAssignBool",        kFlowNormal },
			{ Resources::Command::kKnowledgeAssignInteger,     "knowledgeAssignInteger",     kFlowNormal },
			{ Resources::Command::kLocationScrollTo,           "locationScrollTo",           kFlowNormal },
			{ Resources::Command::kSoundPlay,                  "soundPlay",                  kFlowNormal },
			{ Resources::Command::kKnowledgeSetIntRandom,      "knowledgeSetIntRandom",      kFlowNormal },
			{ Resources::Command::kKnowledgeSubValue,          "knowledgeSubValue",          kFlowNormal },
			{ Resources::Command::kItemLookDirection,          "itemLookDirection",          kFlowNormal },
			{ Resources::Command::kStopPlayingSound,           "stopPlayingSound",           kFlowNormal },
			{ Resources::Command::kLayerGoTo,                  "layerGoTo",                  kFlowNormal },
			{ Resources::Command::kLayerEnable,                "layerEnable",                kFlowNormal },
			{ Resources::Command::kLocationScrollSet,          "locationScrollSet",          kFlowNormal },
			{ Resources::Command::kPlayFullMotionVideo,        "playFullMotionVideo",        kFlowNormal },
			{ Resources::Command::kAnimSetFrame,               "animSetFrame",               kFlowNormal },
			{ Resources::Command::kKnowledgeAssignNegatedBool, "knowledgeAssignNegatedBool", kFlowNormal },
			{ Resources::Command::kEnableDiaryEntry,           "enableDiaryEntry",           kFlowNormal },
			{ Resources::Command::kPATChangeTooltip,           "pATChangeTooltip",           kFlowNormal },
			{ Resources::Command::kSoundChange,                "soundChange",                kFlowNormal },
			{ Resources::Command::kLightSetColor,              "lightSetColor",              kFlowNormal },
			{ Resources::Command::kItem3DRunTo,                "item3DRunTo",                kFlowNormal },
			{ Resources::Command::kItemPlaceDirection,         "itemPlaceDirection",         kFlowNormal },
			{ Resources::Command::kActivateTexture,            "activateTexture",            kFlowNormal },
			{ Resources::Command::kActivateMesh,               "activateMesh",               kFlowNormal },
			{ Resources::Command::kItem3DSetWalkTarget,        "item3DSetWalkTarget",        kFlowNormal },
			{ Resources::Command::kSpeakWithoutTalking,        "speakWithoutTalking",        kFlowNormal },
			{ Resources::Command::kIsOnFloorField,             "isOnFloorField",             kFlowBranch },
			{ Resources::Command::kIsItemEnabled,              "isItemEnabled",              kFlowBranch },
			{ Resources::Command::kIsScriptEnabled,            "isScriptEnabled",            kFlowBranch },
			{ Resources::Command::kIsKnowledgeBooleanSet,      "isKnowledgeBooleanSet",      kFlowBranch },
			{ Resources::Command::kIsKnowledgeIntegerInRange,  "isKnowledgeIntegerInRange",  kFlowBranch },
			{ Resources::Command::kIsKnowledgeIntegerAbove,    "isKnowledgeIntegerAbove",    kFlowBranch },
			{ Resources::Command::kIsKnowledgeIntegerEqual,    "isKnowledgeIntegerEqual",    kFlowBranch },
			{ Resources::Command::kIsKnowledgeIntegerLower,    "isKnowledgeIntegerLower",    kFlowBranch },
			{ Resources::Command::kIsScriptActive,             "isScriptActive",             kFlowBranch },
			{ Resources::Command::kIsRandom,                   "isRandom",                   kFlowBranch },
			{ Resources::Command::kIsAnimScriptItemReached,    "isAnimScriptItemReached",    kFlowBranch },
			{ Resources::Command::kIsItemOnPlace,              "isItemOnPlace",              kFlowBranch },
			{ Resources::Command::kIsAnimPlaying,              "isAnimPlaying",              kFlowBranch },
			{ Resources::Command::kIsItemActivity,             "isItemActivity",             kFlowBranch },
			{ Resources::Command::kIsItemNearPlace,            "isItemNearPlace",            kFlowBranch },
			{ Resources::Command::kIsAnimAtTime,               "isAnimAtTime",               kFlowBranch },
			{ Resources::Command::kIsLocation2D,               "isLocation2D",               kFlowBranch },
			{ Resources::Command::kIsInventoryOpen,            "isInventoryOpen",            kFlowBranch }
	};

	for (uint i = 0; i < ARRAYSIZE(typeNames); i++) {
		if (typeNames[i].subType == subType) {
			return &typeNames[i];
		}
	}

	error("Command subtype %d is not described", subType);
}

Common::String Command::describeArguments() const {
	Common::String desc;

	for (uint i = 0; i < _arguments.size(); i++) {
		switch (_arguments[i].type) {
			case Resources::Command::Argument::kTypeInteger1:
			case Resources::Command::Argument::kTypeInteger2:
				desc += Common::String::format("%d", _arguments[i].intValue);
				break;

			case Resources::Command::Argument::kTypeResourceReference: {
				desc += Common::String::format("%s", _arguments[i].referenceValue.describe().c_str());
			}
				break;
			case Resources::Command::Argument::kTypeString:
				desc += Common::String::format("%s", _arguments[i].stringValue.c_str());
				break;
			default:
				error("Unknown argument type %d", _arguments[i].type);
		}

		if (i != _arguments.size() - 1) {
			desc += ", ";
		}
	}

	return desc;
}

void Command::printCall() const {
	debug("%d: %s(%s)", _index, _subTypeDesc->name, describeArguments().c_str());
}

uint16 Command::getIndex() const {
	return _index;
}

CFGCommand::CFGCommand(Resources::Command *resource) :
		Command(resource),
		_followerIndex(-1),
		_trueBranchIndex(-1),
		_falseBranchIndex(-1),
		_follower(nullptr),
		_trueBranch(nullptr),
		_falseBranch(nullptr),
		_block(nullptr) {
	initBranches();
}

void CFGCommand::initBranches() {
	switch (_subTypeDesc->controlFlowType) {
		case kFlowNormal:
			_followerIndex = _arguments[0].intValue;
			break;
		case kFlowBranch:
			if (_arguments[0].intValue == _arguments[1].intValue) {
				// Degenerate conditions are handled here so that blocks are not split after them
				_followerIndex = _arguments[0].intValue;
			} else {
				_falseBranchIndex = _arguments[0].intValue;
				_trueBranchIndex = _arguments[1].intValue;
			}
			break;
		case kFlowEnd:
			// No followers
			break;
	}
}

bool CFGCommand::isEntryPoint() const {
	return _subType == Resources::Command::kCommandBegin;
}

bool CFGCommand::isBranch() const {
	return _trueBranchIndex >= 0 && _falseBranchIndex >= 0;
}

bool CFGCommand::isBranchTarget() const {
	return _predecessors.size() > 1;
}

Block *CFGCommand::getBlock() const {
	return _block;
}

void CFGCommand::setBlock(Block *block) {
	_block = block;
}

CFGCommand *CFGCommand::getFollower() const {
	return _follower;
}

CFGCommand *CFGCommand::getTrueBranch() const {
	return _trueBranch;
}

CFGCommand *CFGCommand::getFalseBranch() const {
	return _falseBranch;
}

void CFGCommand::linkBranches(const Common::Array<CFGCommand *> &commands) {
	if (_followerIndex >= 0) {
		_follower = findCommandWithIndex(commands, _followerIndex);
		_follower->_predecessors.push_back(this);
	}

	if (_falseBranchIndex >= 0) {
		_falseBranch = findCommandWithIndex(commands, _falseBranchIndex);
		_falseBranch->_predecessors.push_back(this);
	}

	if (_trueBranchIndex >= 0) {
		_trueBranch = findCommandWithIndex(commands, _trueBranchIndex);
		_trueBranch->_predecessors.push_back(this);
	}
}

CFGCommand *CFGCommand::findCommandWithIndex(const Common::Array<CFGCommand *> &commands, int32 index) {
	for (uint i = 0; i < commands.size(); i++) {
		CFGCommand *command = commands[i];

		if (command->_index == index) {
			return command;
		}
	}

	error("Unable to find command with index %d", index);
}

} // End of namespace Tools
} // End of namespace Stark
