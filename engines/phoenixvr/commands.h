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

#ifndef PHOENIXVR_COMMANDS_H
#define PHOENIXVR_COMMANDS_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/str.h"

namespace PhoenixVR {

struct Scope;

struct ExecutionContext {
	bool running = true;
	bool subroutine = false;
	const Scope *scope = nullptr;
	const Scope *rootScope = nullptr;
};

struct Command {
	virtual ~Command() {}
	virtual void exec(ExecutionContext &ctx) const = 0;

	static int valueOf(const Common::String &value);
};
using CommandPtr = Common::SharedPtr<Command>;

struct Scope : public Command {
	Common::Array<CommandPtr> commands;

	struct Label {
		Common::String name;
		uint offset;
	};
	Common::Array<Label> labels;

	const Label *findLabel(const Common::String &name) const {
		auto it = Common::find_if(labels.begin(), labels.end(), [&](const Label &label) { return label.name.equalsIgnoreCase(name); });
		return it != labels.end() ? &*it : nullptr;
	}

	void exec(ExecutionContext &ctx) const override;
	void exec(ExecutionContext &ctx, uint offset) const;
};
using ScopePtr = Common::SharedPtr<Scope>;

} // namespace PhoenixVR

#endif
