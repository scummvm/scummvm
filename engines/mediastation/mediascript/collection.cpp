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

#include "mediastation/mediastation.h"
#include "mediastation/mediascript/collection.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/codechunk.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

ScriptValue Collection::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	// Debug print the collection contents.
	debugC(7, kDebugScript, "	COLLECTION: [");
	for (uint i = 0; i < size(); i++) {
		const ScriptValue &rhs = operator[](i);
		debugC(7, kDebugScript, "		%d of %d: %s", i, size(), rhs.getDebugString().c_str());
	}
	debugC(7, kDebugScript, "	]");

	ScriptValue returnValue;
	switch (methodId) {
	case kAppendMethod:
		for (ScriptValue value : args) {
			push_back(value);
		}
		break;

	case kApplyMethod:
		apply(args);
		break;

	case kCountMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(size());
		break;

	case kDeleteFirstMethod:
		ARGCOUNTCHECK(0);
		if (size() > 0) {
			returnValue = remove_at(0);
			debugC(7, kDebugScript, "%s: %s", __func__, returnValue.getDebugString().c_str());
		} else {
			warning("%s: Array is empty", __func__);
		}
		break;

	case kDeleteLastMethod:
		ARGCOUNTCHECK(0);
		if (size() > 0) {
			returnValue = remove_at(size() - 1);
			debugC(7, kDebugScript, "%s: %s", __func__, returnValue.getDebugString().c_str());
		} else {
			warning("%s: Array is empty", __func__);
		}
		break;

	case kEmptyMethod:
		ARGCOUNTCHECK(0);
		clear();
		break;

	case kGetAtMethod: {
		ARGCOUNTCHECK(1);
		uint index = static_cast<uint>(args[0].asFloat());
		if (index < size()) {
			returnValue = operator[](index);
		} else {
			warning("%s: Index %d out of bounds %d", __func__, index, size());
		}
		break;
	}

	case kIsEmptyMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToBool(empty());
		break;

	case kJumbleMethod:
		ARGCOUNTCHECK(0);
		jumble();
		break;

	case kSeekMethod: {
		ARGCOUNTCHECK(1);
		int index = seek(args[0]);
		returnValue.setToFloat(index);
		break;
	}

	case kSendMethod:
		ARGCOUNTMIN(1);
		send(args);
		break;

	case kDeleteAtMethod: {
		ARGCOUNTCHECK(1);
		uint index = static_cast<uint>(args[0].asFloat());
		if (index < size()) {
			returnValue = remove_at(index);
			debugC(7, kDebugScript, "%s: %s", __func__, returnValue.getDebugString().c_str());
		} else {
			warning("%s: Index %d out of bounds %d", __func__, index, size());
		}
		break;
	}

	case kInsertAtMethod: {
		ARGCOUNTCHECK(2);
		uint index = static_cast<uint>(args[1].asFloat());
		if (index <= size()) {
			insert_at(index, args[0]);
		} else {
			warning("%s: Index %d out of bounds %d", __func__, index, size());
		}
		break;
	}

	case kReplaceAtMethod: {
		ARGCOUNTCHECK(2);
		uint index = static_cast<uint>(args[1].asFloat());
		if (index < size()) {
			operator[](index) = args[0];
		} else {
			warning("%s: Index %d out of bounds %d", __func__, index, size());
		}
		break;
	}

	case kPrependListMethod:
		ARGCOUNTMIN(1);
		insert_at(0, args);
		break;

	case kSortMethod:
		ARGCOUNTCHECK(0);
		Common::sort(begin(), end());
		break;

	default:
		error("%s: Attempt to call unimplemented method %s (%d)", __func__, builtInMethodToStr(methodId), static_cast<uint>(methodId));
	}
	return returnValue;
}

void Collection::apply(const Common::Array<ScriptValue> &args) {
	// Calls a function with each element of the collection as the first arg.
	Common::Array<ScriptValue> argsToApply = args;
	uint functionId = args[0].asFunctionId();
	for (const ScriptValue &item : *this) {
		argsToApply[0] = item;
		debugC(7, kDebugScript, "%s: %s: %s", __func__, g_engine->formatFunctionName(functionId).c_str(), item.getDebugString().c_str());
		g_engine->getFunctionManager()->call(functionId, argsToApply);
	}
}

void Collection::send(const Common::Array<ScriptValue> &args) {
	Common::Array<ScriptValue> argsToSend(args.size() - 1);
	if (argsToSend.size() > 0) {
		for (uint i = 1; i < args.size(); i++) {
			argsToSend[i - 1] = args[i];
		}
	}

	BuiltInMethod methodToSend = static_cast<BuiltInMethod>(args[0].asMethodId());
	Common::Array<ScriptValue> sendArgs;
	for (const ScriptValue &item : *this) {
		uint actorId = item.asActorId();
		Actor *targetActor = g_engine->getImtGod()->getActorById(actorId);
		if (targetActor != nullptr) {
			debugC(7, kDebugScript, "%s: %s: %s", __func__, builtInMethodToStr(methodToSend), targetActor->debugName());
			targetActor->callMethod(methodToSend, argsToSend);
		}
	}
}

int Collection::seek(const ScriptValue &lhs) {
	// Search from back to front.
	for (int i = size() - 1; i >= 0; i--) {
		const ScriptValue &rhs = operator[](i);
		debugC(7, kDebugScript, "%s: %d of %d: Checking (%s) == (%s)",
			__func__, i, size(), lhs.getDebugString().c_str(), rhs.getDebugString().c_str());

		// Only compare values if types match.
		if (lhs.getType() == rhs.getType() && lhs == rhs) {
			return i;
		}
	}
	return -1;
}

void Collection::jumble() {
	if (!empty()) {
		for (uint i = size() - 1; i > 0; --i) {
			uint j = g_engine->_randomSource.getRandomNumber(size() - 1);
			SWAP(operator[](i), operator[](j));
		}
	}
}

} // End of namespace MediaStation
