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
#include "mediastation/actors/document.h"

namespace MediaStation {

ScriptValue DocumentActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kBranchToScreenMethod:
		processBranch(args);
		return returnValue;

	case kReleaseContextMethod: {
		assert(args.size() == 1);
		uint32 contextId = args[0].asActorId();
		g_engine->scheduleContextRelease(contextId);
		return returnValue;
	}

	default:
		return Actor::callMethod(methodId, args);
	}
}

void DocumentActor::processBranch(Common::Array<ScriptValue> &args) {
	assert(args.size() >= 1);
	uint contextId = args[0].asActorId();
	if (args.size() > 1) {
		bool disableUpdates = static_cast<bool>(args[1].asParamToken());
		if (disableUpdates)
			warning("%s: disableUpdates parameter not handled yet", __func__);
	}

	g_engine->scheduleScreenBranch(contextId);
}

} // End of namespace MediaStation
