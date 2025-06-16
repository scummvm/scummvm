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
#include "mediastation/assets/document.h"

namespace MediaStation {

ScriptValue Document::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kBranchToScreenMethod: {
		assert(args.size() >= 1);
		if (args.size() > 1) {
			// TODO: Figure out what the rest of the args can be.
			warning("branchToScreen got more than one arg");
		}
		uint32 contextId = args[0].asAssetId();
		g_engine->scheduleScreenBranch(contextId);
		return returnValue;
	}

	case kReleaseContextMethod: {
		assert(args.size() == 1);
		uint32 contextId = args[0].asAssetId();
		g_engine->scheduleContextRelease(contextId);
		return returnValue;
	}

	default:
		return Asset::callMethod(methodId, args);
	}
}

} // End of namespace MediaStation
