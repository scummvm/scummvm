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

#include "twp/callback.h"
#include "twp/squtil.h"

namespace Twp {

Callback::Callback(int id, float duration, const Common::String &name, const Common::Array<HSQOBJECT> &args)
	: _id(id), _duration(duration), _name(name), _args(args) {
}

void Callback::call() {
	sqcall(_name.c_str(), _args);
}

bool Callback::update(float elapsed) {
	_elapsed += elapsed;
	bool result = _elapsed > _duration;
	if (result)
		call();
	return result;
}

} // namespace Twp
