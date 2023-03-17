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

#include "engines/advancedDetector.h"

#include "sherlock/detection.h"
#include "sherlock/sherlock.h"

static const PlainGameDescriptor sherlockGames[] = {
	{ "scalpel", "The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel" },
	{ "rosetattoo", "The Lost Files of Sherlock Holmes: Case of the Rose Tattoo" },
	{nullptr, nullptr}
};

static const DebugChannelDef debugFlagList[] = {
	{Sherlock::kDebugLevelScript, "scripts", "Script debug level"},
	{Sherlock::kDebugLevelAdLibDriver, "AdLib", "AdLib driver debugging"},
	{Sherlock::kDebugLevelMT32Driver, "MT32", "MT32 driver debugging"},
	{Sherlock::kDebugLevelMusic, "Music", "Music debugging"},
	DEBUG_CHANNEL_END
};

#include "sherlock/detection_tables.h"

class SherlockMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	SherlockMetaEngineDetection() : AdvancedMetaEngineDetection(Sherlock::gameDescriptions, sizeof(Sherlock::SherlockGameDescription),
		sherlockGames) {}

	const char *getName() const override {
		return "sherlock";
	}

	const char *getEngineName() const override {
		return "Sherlock";
	}

	const char *getOriginalCopyright() const override {
		return "Sherlock (C) 1992-1996 Mythos Software, (C) 1992-1996 Electronic Arts";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};


REGISTER_PLUGIN_STATIC(SHERLOCK_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, SherlockMetaEngineDetection);
