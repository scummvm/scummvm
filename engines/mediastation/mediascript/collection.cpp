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
#include "mediastation/datum.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

ScriptValue Collection::callMethod(BuiltInMethod method, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (method) {
	case kIsEmptyMethod: {
		returnValue.setToBool(empty());
		return returnValue;
	}

	case kAppendMethod: {
		for (ScriptValue arg : args) {
			push_back(arg);
		}
		return returnValue;
	}

	case kDeleteFirstMethod: {
		returnValue = remove_at(0);
		return returnValue;
	}

	case kDeleteAtMethod: {
		// Find the item in the collection, then remove and return it.
		assert(args.size() == 1);
		for (uint i = 0; i < size(); i++) {
			if (args[0] == operator[](i)) {
				returnValue = remove_at(i);
				return returnValue;
			}
		}

		// The item wasn't found.
		return returnValue;
	}

	case kCountMethod: {
		double size = static_cast<double>(this->size());
		returnValue.setToFloat(size);
		return returnValue;
	}

	case kGetAtMethod: {
		assert(args.size() == 1);
		uint index = static_cast<uint>(args[0].asFloat());
		returnValue = operator[](index);
		return returnValue;
	}

	case kSendMethod: {
		// Call a method on each item in the collection.
		BuiltInMethod methodToSend = static_cast<BuiltInMethod>(args[0].asMethodId());
		Common::Array<ScriptValue> sendArgs;
		for (uint i = 0; i < size(); i++) {
			ScriptValue self = operator[](i);

			uint assetId = self.asAssetId();
			Asset *selfAsset = g_engine->getAssetById(assetId);
			if (selfAsset != nullptr) {
				Common::Array<ScriptValue> emptyArgs;
				returnValue = selfAsset->callMethod(methodToSend, emptyArgs);
			}
		}
		return returnValue;
	}

	case kSeekMethod: {
		// Find the item in the collection if it exists.
		assert(args.size() == 1);
		for (uint i = 0; i < size(); i++) {
			if (args[0] == operator[](i)) {
				return operator[](i);
			}
		}

		// The item wasn't found.
		returnValue.setToFloat(-1.0);
		return returnValue;
	}

	case kJumbleMethod: {
		// Scramble the items in the collection.
		for (uint i = size() - 1; i > 0; --i) {
			uint j = g_engine->_randomSource.getRandomNumber(size() - 1);
			SWAP(operator[](i), operator[](j));
		}
		return returnValue;
	}

	case kSortMethod: {
		assert(args.empty());
		Common::sort(begin(), end());
		return returnValue;
	}

	case kEmptyMethod: {
		clear();
		return returnValue;
	}

	default:
		error("Collection::callMethod(): Attempt to call unimplemented method %s (%d)", builtInMethodToStr(method), static_cast<uint>(method));
	}
}

} // End of namespace MediaStation
