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
#include "dgds/dgds.h"
#include "dgds/ads.h"
#include "dgds/scene.h"

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

/*static*/
bool SceneConditions::check(const Common::Array<SceneConditions> &conds) {
	DgdsEngine *engine = DgdsEngine::getInstance();

	uint cnum = 0;
	while (cnum < conds.size()) {
		const SceneConditions &c = conds[cnum];
		int16 refval = c.getVal();
		int16 checkval = -1;
		SceneCondition cflag = c.getCond();
		// Hit an "or" here means the last result was true.
		if (cflag & kSceneCondOr)
			return true;

		if (cflag & kSceneCondSceneState) {
			refval = 1;
			checkval = engine->adsInterpreter()->getStateForSceneOp(c.getNum());
			SceneCondition equalOrNegate = static_cast<SceneCondition>(cflag & (kSceneCondEqual | kSceneCondNegate));
			if (equalOrNegate != kSceneCondEqual && equalOrNegate != kSceneCondNegate)
				refval = 0;
			cflag = kSceneCondEqual;
		} else if (cflag & kSceneCondNeedItemQuality || cflag & kSceneCondNeedItemSceneNum) {
			const Common::Array<GameItem> &items = engine->getGDSScene()->getGameItems();
			for (const auto &item : items) {
				if (item._num == c.getNum()) {
					if (cflag & kSceneCondNeedItemSceneNum)
						checkval = item._inSceneNum;
					else // cflag & kSceneCondNeedItemQuality
						checkval = item._quality;
					break;
				}
			}
		} else {
			checkval = engine->getGDSScene()->getGlobal(c.getNum());
			if (!(cflag & kSceneCondAbsVal))
				refval = engine->getGDSScene()->getGlobal((uint16)refval);
		}

		bool result = false;
		cflag = static_cast<SceneCondition>(cflag & ~(kSceneCondSceneState | kSceneCondNeedItemSceneNum | kSceneCondNeedItemQuality));
		if (cflag == kSceneCondNone)
			cflag = static_cast<SceneCondition>(kSceneCondEqual | kSceneCondNegate);
		if ((cflag & kSceneCondLessThan) && checkval < refval)
			result = true;
		if ((cflag & kSceneCondEqual) && checkval == refval)
			result = true;
		if (cflag & kSceneCondNegate)
			result = !result;

		debug(11, "Cond: %s -> %s", c.dump("").c_str(), result ? "true": "false");

		if (!result) {
			// Skip just past the next or, or to the end.
			while (cnum < conds.size() && !(conds[cnum].getCond() & kSceneCondOr))
				cnum++;
			if (cnum >= conds.size())
				return false;
		}
		cnum++;
	}
	return true;
}


Common::String SceneConditions::dump(const Common::String &indent) const {
	return Common::String::format("%sSceneCondition<flg 0x%02x(%s) num %d val %d>", indent.c_str(),
			_flags, _sceneConditionStr(_flags).c_str(), _num, _val);
}

} // end namespace Dgds
