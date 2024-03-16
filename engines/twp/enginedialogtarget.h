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

#ifndef TWP_ENGINEDIALOGTARGET_H
#define TWP_ENGINEDIALOGTARGET_H

#include "twp/dialog.h"

namespace Twp {

class EngineDialogTarget : public DialogTarget {
public:
	Color actorColor(const Common::String &actor) override;
	Color actorColorHover(const Common::String &actor) override;
	Common::SharedPtr<Motor> say(const Common::String &actor, const Common::String &text) override;
	Common::SharedPtr<Motor> waitWhile(const Common::String &cond) override;
	void shutup() override;
	Common::SharedPtr<Motor> pause(float time) override;
	bool execCond(const Common::String &cond) override;
};

} // namespace Twp

#endif
