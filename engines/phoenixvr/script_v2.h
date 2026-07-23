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

#ifndef PHOENIXVR_SCRIPT_V2_H
#define PHOENIXVR_SCRIPT_V2_H

#include "phoenixvr/script.h"

namespace PhoenixVR {
class ScriptV2 : public Script {
public:
	struct Condition {
		Common::String op;
		Common::String arg1;
		Common::String arg2;

		Condition(const Common::String &text);
		bool parse(const Common::String &text, const Common::String &maybeOp);
		int value() const;
	};
	using ConditionPtr = Common::SharedPtr<Condition>;

	struct Conditional : public Command {
		Common::Array<ConditionPtr> conditions;
		ScopePtr trueScope;
		ScopePtr falseScope;

		Conditional(const Common::Array<Common::String> &args);
		void branch(ExecutionContext &ctx, bool branch) const;
	};
	using ConditionalPtr = Common::SharedPtr<Conditional>;

private:
	WarpPtr _currentWarp;
	TestPtr _currentTest;
	struct ConditionalScope {
		ConditionalPtr conditional;
		ScopePtr scope;
	};
	Common::Array<ConditionalScope> _conditionals;

	Scope &topScope() const {
		assert(_currentTest);
		return !_conditionals.empty() ? *_conditionals.back().scope : _currentTest->scope;
	}
	void closeScope();
	void closeAllScopes();

private:
	void parseLine(const Common::String &line, uint lineno) override;
};
} // namespace PhoenixVR

#endif
