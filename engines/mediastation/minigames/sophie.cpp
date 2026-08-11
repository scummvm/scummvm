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

#include "mediastation/mediascript/function.h"
#include "mediastation/actors/sprite.h"
#include "mediastation/mediastation.h"
#include "math/angle.h"

namespace MediaStation {

// This is a hardcoded function in Puzzle Castle.
void FunctionManager::script_MoveSophie(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	// This seems to be hardcoded rather than implemented as a script function due to the need for atan2l.
	uint sophieActorId = args[0].asActorId();
	SpriteMovieActor *spriteActor = static_cast<SpriteMovieActor *>(g_engine->getImtGod()->getActorByIdAndType(sophieActorId, kActorTypeSprite));

	// This seems to be some sort of angle perhaps, but it is not used.
	// int unk1 = static_cast<int>(args[1].asFloat());
	int dx = static_cast<int>(args[2].asFloat());
	int dy = static_cast<int>(args[3].asFloat());
	// args[4] through args[11] are the sprite clip ids. They typically should be:
	// args[4] = $sophieWalk_North (10018),
	// args[5] = $sophieWalk_NorthEast (10021),
	// args[6] = $sophieWalk_East (10020),
	// args[7] = $sophieWalk_SouthEast (10022),
	// args[8] = $sophieWalk_South (10019),
	// args[9] = $sophieWalk_SouthWest (10025),
	// args[10] = $sophieWalk_West (10023),
	// args[11] = $sophieWalk_NorthWest (10024)
	double sophieSpatialCenterX = args[12].asFloat();
	double sophieSpatialCenterY = args[13].asFloat();

	// Convert the direction we're moving into an octant to select the proper sprite to show.
	const float angleDegrees = Math::Angle::arcTangent2(dy, dx).getDegrees(0);
	int octant = int((angleDegrees + 22.5f) / 45.0f) & 7;

	// Get that sprite clip.
	Common::Array<ScriptValue> sophieArgs;
	ScriptValue temp;
	static const int octantToScriptArgIndex[] = {
		6,  // East
		5,  // NE
		4,  // North
		11, // NW
		10, // West
		9,  // SW
		8,  // South
		7   // SE
	};
	int directionIndex = octantToScriptArgIndex[octant];
	sophieArgs.push_back(args[directionIndex]);
	spriteActor->callMethod(kSetCurrentClipMethod, sophieArgs);

	// Move Sophie to the right location.
	sophieArgs.clear();
	temp.setToFloat(sophieSpatialCenterX);
	sophieArgs.push_back(temp);
	temp.setToFloat(sophieSpatialCenterY);
	sophieArgs.push_back(temp);
	spriteActor->callMethod(kSpatialCenterMoveToMethod, sophieArgs);

	// Increment the frame to show Sophie walking.
	sophieArgs.clear();
	temp.setToBool(true);
	sophieArgs.push_back(temp);
	spriteActor->callMethod(kIncrementFrameMethod, sophieArgs);
}

} // End of namespace Mediastation
