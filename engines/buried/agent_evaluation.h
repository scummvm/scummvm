/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_AGENT_EVALUATION_H
#define BURIED_AGENT_EVALUATION_H

#include "buried/global_flags.h"
#include "common/str.h"

namespace Buried {

class BuriedEngine;

class AgentEvaluation {
public:
	AgentEvaluation(BuriedEngine *vm, GlobalFlags &globalFlags, int deathSceneIndex);

	Common::String _scoringTextDescriptions;
	Common::String _scoringTextScores;
	Common::String _scoringTextFinalScore;
	Common::String _scoringTextDescriptionsWithScores;

private:
	GlobalFlags _globalFlags;
};

} // End of namespace Buried

#endif
