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

ScriptValue Collection::callMethod(BuiltInMethod method, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;
	switch (method) {
	case kAppendMethod:
		for (ScriptValue value : args) {
			push_back(value);
		}
		break;

	case kApplyMethod:
		apply(args);
		break;

	case kCountMethod:
		assert(args.empty());
		returnValue.setToFloat(size());
		break;

	case kDeleteFirstMethod:
		assert(args.empty());
		returnValue = remove_at(0);
		break;

	case kDeleteLastMethod:
		assert(args.empty());
		returnValue = remove_at(size() - 1);
		break;

	case kEmptyMethod:
		assert(args.empty());
		clear();
		break;

	case kGetAtMethod: {
		assert(args.size() == 1);
		uint index = static_cast<uint>(args[0].asFloat());
		returnValue = operator[](index);
		break;
	}

	case kIsEmptyMethod:
		assert(args.empty());
		returnValue.setToBool(empty());
		break;

	case kJumbleMethod:
		assert(args.empty());
		jumble();
		break;

	case kSeekMethod: {
		assert(args.size() == 1);
		int index = seek(args[0]);
		returnValue.setToFloat(index);
		break;
	}

	case kSendMethod:
		send(args);
		break;

	case kDeleteAtMethod: {
		assert(args.size() == 1);
		uint index = static_cast<uint>(args[0].asFloat());
		returnValue = remove_at(index);
		break;
	}

	case kInsertAtMethod: {
		assert(args.size() == 2);
		uint index = static_cast<uint>(args[1].asFloat());
		insert_at(index, args[0]);
		break;
	}

	case kReplaceAtMethod: {
		assert(args.size() == 2);
		uint index = static_cast<uint>(args[1].asFloat());
		operator[](index) = args[0];
		break;
	}

	case kPrependListMethod:
		insert_at(0, args);
		break;

	case kSortMethod:
		assert(args.empty());
		Common::sort(begin(), end());
		break;

	default:
		error("%s: Attempt to call unimplemented method %s (%d)", __func__, builtInMethodToStr(method), static_cast<uint>(method));
	}
	return returnValue;
}

void Collection::apply(const Common::Array<ScriptValue> &args) {
	// Calls a function with each element of the collection as the first arg.
	Common::Array<ScriptValue> argsToApply = args;
	uint functionId = args[0].asFunctionId();
	for (const ScriptValue &item : *this) {
		argsToApply[0] = item;
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
		Actor *targetActor = g_engine->getActorById(actorId);
		if (targetActor != nullptr) {
			targetActor->callMethod(methodToSend, argsToSend);
		}
	}
}

int Collection::seek(const ScriptValue &item) {
	// Search from back to front.
	for (int i = size() - 1; i >= 0; i--) {
		if (item == operator[](i)) {
			return i;
		}
	}
	return -1;
}

void Collection::jumble() {
	for (uint i = size() - 1; i > 0; --i) {
		uint j = g_engine->_randomSource.getRandomNumber(size() - 1);
		SWAP(operator[](i), operator[](j));
	}
}

} // End of namespace MediaStation
