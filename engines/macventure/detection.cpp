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
#include "macventure/macventure.h"

namespace MacVenture {

#define ADGF_DEFAULT (ADGF_DROPLANGUAGE|ADGF_DROPPLATFORM|ADGF_MACRESFORK|ADGF_UNSTABLE)

#define BASEGAME(n, v, f, md5, s) {n, v, AD_ENTRY1s(f, md5, s), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_DEFAULT, GUIO1(GUIO_NOMIDI)}
#define BASEDEMO(n, v, f, md5, s) {n, v, AD_ENTRY1s(f, md5, s), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_DEFAULT|ADGF_DEMO, GUIO1(GUIO_NOMIDI)}

static const ADGameDescription gameDescriptions[] = {
	BASEGAME("shadowgate", "Zojoi Rerelease", "Shadowgate", "ebbfbcbf93938bd2900cb0c0213b19ad", 68718), // Zojoi Rerelease
	BASEGAME("deja_vu", "Zojoi Rerelease", "Deja Vu", "5e9f5a8e3c8eb29ed02b34ae5937354f", 68778), // Zojoi Rerelease
	BASEGAME("deja_vu2", "Zojoi Rerelease", "Lost in Las Vegas", "8f8e1d8d41f577ee0fbc03847969af0d", 66264), // Zojoi Rerelease
	BASEDEMO("uninvited", "Demo", "Uninvited Demo", "e53adca77d773dca926f61faac68df86", 53119),
	AD_TABLE_END_MARKER
};

} // End of namespace MacVenture

static const DebugChannelDef debugFlagList[] = {
	{MacVenture::kMVDebugMain, "main", "Engine state"},
	{MacVenture::kMVDebugGUI, "gui", "Gui"},
	{MacVenture::kMVDebugText, "text", "Text decoders and printers"},
	{MacVenture::kMVDebugImage, "image", "Image decoders and renderers"},
	{MacVenture::kMVDebugScript, "script", "Script engine"},
	{MacVenture::kMVDebugSound, "sound", "Sound decoders"},
	{MacVenture::kMVDebugContainer, "container", "Containers"},
	DEBUG_CHANNEL_END
};

static const PlainGameDescriptor macventureGames[] = {
	{ "shadowgate", "Shadowgate" },
	{ "deja_vu", "Deja Vu"},
	{ "deja_vu2", "Deja Vu II"},
	{ "uninvited", "Uninvited."},
	{ nullptr, nullptr }
};

namespace MacVenture {

SaveStateDescriptor loadMetaData(Common::SeekableReadStream *s, int slot, bool skipThumbnail = true);

class MacVentureMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	MacVentureMetaEngineDetection() : AdvancedMetaEngineDetection(MacVenture::gameDescriptions, sizeof(ADGameDescription), macventureGames) {
		_guiOptions = GUIO1(GUIO_NOMIDI);
		_md5Bytes = 5000000; // TODO: Upper limit, adjust it once all games are added
	}

	const char *getEngineId() const override {
		return "macventure";
	}

	const char *getName() const override {
		return "MacVenture";
	}

	const char *getOriginalCopyright() const override {
		return "(C) ICOM Simulations";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

} // End of namespace MacVenture

REGISTER_PLUGIN_STATIC(MACVENTURE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, MacVenture::MacVentureMetaEngineDetection);
