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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_SX3FSTATISTICS_H
#define WINTERMUTE_SX3FSTATISTICS_H

#include "common/str.h"
#include "engines/wintermute/base/base_scriptable.h"

namespace Wintermute {

class SX3fStatistics : public BaseScriptable {
public:
	DECLARE_PERSISTENT(SX3fStatistics, BaseScriptable)
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;
	SX3fStatistics(BaseGame *inGame, ScStack *stack);
	~SX3fStatistics() override;

private:
	Common::String _baseUrl;
	Common::String _chapter;
	Common::String _language;
	Common::String _buildNum;
	int32 _repeat{};
};

} // End of namespace Wintermute

#endif
