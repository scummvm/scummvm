/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "dune/detection.h"

namespace Dune {

static const DebugChannelDef debugFlagList[] = {
	{ kDebugResource, "resource", "Resource archive and decompression" },
	{ kDebugGraphics, "graphics", "Graphics and sprite drawing" },
	{ kDebugScript, "script", "Game script interpreter" },
	{ kDebugSound, "sound", "Music and sound effects" },
	{ kDebugVideo, "video", "HNM movie playback" },
	DEBUG_CHANNEL_END
};

static const PlainGameDescriptor duneGames[] = {
	{ "dune", "Dune" },
	{ nullptr, nullptr }
};

const ADGameDescription gameDescriptions[] = {
	{
		"dune",
		"CD",
		AD_ENTRY2s("DUNE.DAT", "f096565944ab48cf4cb6cbf389384e6f", 397794384,
				   "DNCDPRG.EXE", "90913501234b0171c5965edec15d18b9", 78700),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_CD,
		GUIO1(GUIO_NOMIDI)
	},
	AD_TABLE_END_MARKER
};

} // End of namespace Dune

class DuneMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	DuneMetaEngineDetection() : AdvancedMetaEngineDetection(Dune::gameDescriptions, Dune::duneGames) {
	}

	const char *getName() const override {
		return "dune";
	}

	const char *getEngineName() const override {
		return "Dune";
	}

	const char *getOriginalCopyright() const override {
		return "Dune (C) 1992 Cryo Interactive Entertainment / Virgin Games";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return Dune::debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(DUNE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, DuneMetaEngineDetection);
