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

#ifndef DGDS_SCENE_CONDITION_H
#define DGDS_SCENE_CONDITION_H

#include "common/types.h"
#include "common/str.h"
#include "common/array.h"

namespace Dgds {

enum SceneCondition {
	kSceneCondNone = 0,
	kSceneCondLessThan = 1,
	kSceneCondEqual = 2,
	kSceneCondNegate = 4,
	kSceneCondAbsVal = 8,
	kSceneCondOr = 0x10,
	kSceneCondNeedItemSceneNum = 0x20,
	kSceneCondNeedItemQuality = 0x40,
	kSceneCondSceneState = 0x80
};

class SceneConditions {
public:
	SceneConditions(uint16 num, SceneCondition cond, int16 val) : _num(num), _flags(cond), _val(val) {}
	Common::String dump(const Common::String &indent) const;

	static bool check(const Common::Array<SceneConditions> &conds);

	uint16 getNum() const { return _num; }
	SceneCondition getCond() const { return _flags; }
	int16 getVal() const { return _val; }

private:
	uint16 _num;
	SceneCondition _flags; /* eg, see usage in FUN_1f1a_2106 */
	int16 _val;
};

} // end namespace Dgds

#endif // DGDS_SCENE_CONDITION_H
