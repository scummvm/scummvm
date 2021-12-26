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

#include "saga2/saga2.h"
#include "saga2/detection.h"

static const DebugChannelDef debugFlagList[] = {
	{Saga2::kDebugResources, "resources", "Debug the resources"},
	{Saga2::kDebugActors,    "actors",    "Debug the actors"},
	{Saga2::kDebugScripts,   "scripts",   "Debug the scripts"},
	{Saga2::kDebugEventLoop, "eventloop", "Debug the event loop"},
	{Saga2::kDebugInit,      "init",      "Debug the initialization process"},
	{Saga2::kDebugTiles,     "tiles",     "Debug the tiles"},
	{Saga2::kDebugPalettes,  "palettes",  "Debug the palettes"},
	{Saga2::kDebugLoading,   "loading",   "Debug the loading"},
	{Saga2::kDebugTimers,    "timers",    "Debug the timers"},
	{Saga2::kDebugPath,      "path",      "Debug the pathfinding"},
	{Saga2::kDebugTasks,     "tasks",     "Debug the tasks"},
	{Saga2::kDebugSound,     "sound",     "Debug the sound"},
	{Saga2::kDebugSaveload,  "saveload",  "Debug the game saving/loading"},
	{Saga2::kDebugSensors,   "sensors",   "Debug the sensors"},
	DEBUG_CHANNEL_END
};

namespace Saga2 {
static const PlainGameDescriptor saga2Games[] = {
	{"dino", "Dinotopia"},
	{"fta2", "Faery Tale Adventure II: Halls of the Dead"},
	{ nullptr, nullptr }
};


static const SAGA2GameDescription gameDescriptions[] = {
	// Dinotopia Section //////////////////////////////////////////////////////////////////////////////////////
	{
		{
			"dino",
			"",
			{
				{"dinodata.hrs", GAME_RESOURCEFILE,				    "45aa7026d441dd69957385c25d2fd33e", 2698},
				{"dino.hrs",     GAME_OBJRESOURCEFILE,				"7cf3665887a4a18e2fff6938a954c050", 66018},
				{"scripts.hrs",	 GAME_SCRIPTFILE,					"d405841a249a365cf92fc65dd52fb953", 164181},
				{"dinosnd.hrs",  GAME_SOUNDFILE,					"dcf4ade416614b8a64f99dacfd3bd071", 199163997},
				{"dinoimag.hrs", GAME_IMAGEFILE,					"d24d80676f7afcfaca0b61c95056044f", 42342931},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GID_DINO
	},

	{
		{
			"dino",
			"",
			{
				{"dinodata.hrs", GAME_RESOURCEFILE,				    "45aa7026d441dd69957385c25d2fd33e", 2698},
				{"dino.hrs",     GAME_OBJRESOURCEFILE,				"7cf3665887a4a18e2fff6938a954c050", 66018},
				{"scripts.hrs",	 GAME_SCRIPTFILE,					"d405841a249a365cf92fc65dd52fb953", 164181},
				{"dinosnd.hrs",  GAME_SOUNDFILE,					"27a57517be881ad9e0f671901486356e", 228642994},
				{"dinoimag.hrs", GAME_IMAGEFILE,					"d24d80676f7afcfaca0b61c95056044f", 42448562},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GID_DINO
	},

	// Faery Tale Adventure II: Halls of the Dead Section /////////////////////////////////////////////////////

	// Faery Tale Adventure II: Halls of the Dead - English CD version
	{
		{
			"fta2",
			"",
			{
				{"ftadata.hrs",	 GAME_RESOURCEFILE,					"6dc3cbed8df2ddd9f060a7dba4d33ef8", 15042},
				{"fta.hrs",	     GAME_OBJRESOURCEFILE,				"c34a5ee5f1a14a87712a35c4a209a033", 213376},
				{"scripts.hrs",	 GAME_SCRIPTFILE,					"95f33928f6c4f02ee04d2ec5c3314c30", 1041948},
				{"ftasound.hrs", GAME_SOUNDFILE,					"ce930cb38922e6a03461f55d51b4e165", 12403350},
				{"ftaimage.hrs", GAME_IMAGEFILE,					"09bb003733b20f924e2e373d2ddcd394", 21127397},
				{"ftavoice.hrs", GAME_VOICEFILE,					nullptr, -1 },
				{"fta2win.exe",  GAME_EXECUTABLE,					"9a94854fef932483754a8f929caa0dba", 1093120},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GID_FTA2
	},
	{ AD_TABLE_END_MARKER, 0 }
};
} // End of namespace Saga2

static const char *directoryGlobs[] = {
	"res",
	"win",
	nullptr
};

class Saga2MetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	Saga2MetaEngineDetection() : AdvancedMetaEngineDetection(Saga2::gameDescriptions, sizeof(Saga2::SAGA2GameDescription), Saga2::saga2Games) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "saga2";
	}

	const char *getName() const override {
		return "SAGA2";
	}

	const char *getOriginalCopyright() const override {
		return "SAGA2 (C) Wyrmkeep Entertainment";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(SAGA2_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Saga2MetaEngineDetection);
