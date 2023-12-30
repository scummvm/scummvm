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

#ifndef TWP_CALLBACK_H
#define TWP_CALLBACK_H

#include "common/array.h"
#include "common/str.h"
#include "twp/squirrel/squirrel.h"

namespace Twp {

class Callback {
public:
	Callback(int id, float duration, const Common::String& name, const Common::Array<HSQOBJECT>& args);
	bool update(float elapsed);
	int getId() const { return _id; }

private:
	int _id = 0;
	Common::String _name;
	Common::Array<HSQOBJECT> _args;
	float _duration = 0.f;
	float _elapsed = 0.f;

public:
	void call();
};

} // namespace Twp

#endif
