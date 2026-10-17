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

#include "base/plugins.h"
#include "engines/advancedDetector.h"

#include "harold/bcb/bcb.h"
#include "harold/detection.h"
#include "harold/detection_tables.h"

static const PlainGameDescriptor haroldGames[] = {
	{"bcb", "J.B. Harold: Blue Chicago Blues"},
	{nullptr, nullptr}};

static const DebugChannelDef debugFlagList[] = {
	{BCB::kBCBDebugGeneral, "general", "General"},
	DEBUG_CHANNEL_END};

class HaroldMetaEngineDetection : public AdvancedMetaEngineDetection<BCB::HaroldGameDescription> {
public:
	HaroldMetaEngineDetection() : AdvancedMetaEngineDetection(BCB::gameDescriptions, haroldGames) {
		_guiOptions = GUIO2(GUIO_NOMIDI, GUIO_NOSUBTITLES);
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	const char *getName() const override {
		return "harold";
	}

	const char *getEngineName() const override {
		return "J.B. Harold series";
	}

	const char *getOriginalCopyright() const override {
		return "J.B. Harold series (C) Riverhill Soft";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

private:
	static const char *const directoryGlobs[];
};

const char *const HaroldMetaEngineDetection::directoryGlobs[] = {
	"bcb",
	"program",
	nullptr};

REGISTER_PLUGIN_STATIC(HAROLD_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, HaroldMetaEngineDetection);
