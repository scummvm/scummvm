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

#ifndef TWP_TASK_H
#define TWP_TASK_H

#include "common/str.h"
#include "twp/detection.h"
#include "twp/thread.h"

namespace Twp {

class Task {
public:
	virtual ~Task() {}

	virtual bool update(float elapsed) = 0;
};

typedef bool Predicate();

template<typename Predicate>
class BreakWhileCond final : public Task {
public:
	BreakWhileCond(int parentId, const Common::String &name, Predicate cond)
		: _parentId(parentId),
		  _name(name),
		  _cond(Common::move(cond)) {
	}
	virtual ~BreakWhileCond() override final {}

	virtual bool update(float elapsed) override final {
		if (_cond())
			return false;
		Common::SharedPtr<ThreadBase> pt = sqthread(_parentId);
		if (pt) {
			debugC(kDebugGame, "Resume task: %d, %s", _parentId, pt->getName().c_str());
			pt->resume();
		}
		return true;
	}

private:
	int _parentId = 0;
	Common::String _name;
	Predicate _cond;
};

} // namespace Twp

#endif
