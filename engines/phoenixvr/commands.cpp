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

#include "phoenixvr/commands.h"
#include "phoenixvr/phoenixvr.h"

namespace PhoenixVR {

int Command::valueOf(const Common::String &value) {
	if (!value.empty() && (Common::isDigit(value[0]) || value[0] == '-' || value[0] == '+'))
		return atoi(value.c_str());
	return g_engine->getVariable(value);
}

void Scope::exec(ExecutionContext &ctx) const {
	exec(ctx, 0);
}

void Scope::exec(ExecutionContext &ctx, uint offset) const {
	auto oldScope = ctx.scope;
	if (!ctx.rootScope)
		ctx.rootScope = this;
	ctx.scope = this;
	for (uint i = offset, n = commands.size(); i < n; ++i) {
		if (!ctx.running)
			break;
		commands[i]->exec(ctx);
	}
	ctx.scope = oldScope;
}

} // namespace PhoenixVR
