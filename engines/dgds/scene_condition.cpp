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

#include "dgds/scene_condition.h"

namespace Dgds {


Common::String _sceneConditionStr(SceneCondition cflag) {
	Common::String ret;

	if (cflag & kSceneCondOr)
		return "or";

	if (cflag & kSceneCondSceneState)
		ret += "state|";
	if (cflag & kSceneCondNeedItemSceneNum)
		ret += "itemsnum|";
	if (cflag & kSceneCondNeedItemQuality)
		ret += "quality|";
	if ((cflag & (kSceneCondSceneState | kSceneCondNeedItemSceneNum | kSceneCondNeedItemQuality)) == 0)
		ret += "global|";

	cflag = static_cast<SceneCondition>(cflag & ~(kSceneCondSceneState | kSceneCondNeedItemSceneNum | kSceneCondNeedItemQuality));
	if (cflag == kSceneCondNone)
		ret += "nocond";
	if (cflag & kSceneCondLessThan)
		ret += "less";
	if (cflag & kSceneCondEqual)
		ret += "equal";
	if (cflag & kSceneCondNegate)
		ret += "-not";
	if (cflag & kSceneCondAbsVal)
		ret += "(abs)";

	return ret;
}

Common::String SceneConditions::dump(const Common::String &indent) const {
	return Common::String::format("%sSceneCondition<flg 0x%02x(%s) num %d val %d>", indent.c_str(),
			_flags, _sceneConditionStr(_flags).c_str(), _num, _val);
}

} // end namespace Dgds
