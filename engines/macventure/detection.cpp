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

#define MACGAME(n, v, f, md5, s) {n, v, AD_ENTRY1s(f, md5, s), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_DEFAULT, GUIO1(GUIO_NOMIDI)}
#define MACDEMO(n, v, f, md5, s) {n, v, AD_ENTRY1s(f, md5, s), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_DEFAULT|ADGF_DEMO, GUIO1(GUIO_NOMIDI)}
#define IIGSGAME(n, v, f, md5, s) {n, v, AD_ENTRY1s(f, md5, s), Common::EN_ANY, Common::kPlatformApple2GS, ADGF_DEFAULT, GUIO1(GUIO_NOMIDI)}

static const ADGameDescription gameDescriptions[] = {
	// Original Mac releases (uses protection)
	MACGAME("shadowgate", "", "Shadowgate", "b9e8e5d68a81cdbd2cbb5cfe8ea7a47d", 58886), // protection cracked
	MACGAME("deja_vu", "", "xn--Dj Vu-sqa5d", "9e0436d1f24a1c8a3c9fd846f055201e", 58468), // original filename is "Déjà Vu", protection cracked
	MACGAME("uninvited", "", "Uninvited", "2cf518cddeda96bfc0cc9ba0bd91b42e", 58631), // v2.1D1, with protection
	MACGAME("uninvited", "", "Uninvited", "20291feb9bce70a32979031631c42da1", 58639), // protection cracked

	// 1993 Mac rereleases (identical to the Zojoi rereleases), no protection
	MACGAME("shadowgate", "1993 rerelease", "Shadowgate", "0f4eb65cf369c6c75e4b991b986c34a2", 68718),
	MACGAME("deja_vu", "1993 rerelease", "xn--Dj Vu-sqa5d", "0f4eb65cf369c6c75e4b991b986c34a2", 6877), // original filename is "Déjà Vu"
	MACGAME("deja_vu2", "1993 rerelease", "Lost in Las Vegas", "0f4eb65cf369c6c75e4b991b986c34a2", 66264),
	MACGAME("uninvited", "1993 rerelease", "Uninvited", "0f4eb65cf369c6c75e4b991b986c34a2", 68974),

	// IIGS releases
	IIGSGAME("shadowgate", "", "SHADOWGATE", "45924ea00ea4ef088e00db0ee774b8df", 101376),
	IIGSGAME("deja_vu", "", "DEJAVU", "45924ea00ea4ef088e00db0ee774b8df", 101376),
	IIGSGAME("deja_vu", "Zojoi rerelease", "DEJAVU", "81f1ee050d2afae2fa90a3306010dc56", 101376), // 6 bytes different, BB BE F0 -> BB BE 80
	IIGSGAME("deja_vu2", "", "DEJAVUII", "f6ef6ead871d4d1cc0f6cf840f48b63f", 69047),
	IIGSGAME("uninvited", "", "UNINVITED", "45924ea00ea4ef088e00db0ee774b8df", 101376),

	// Misc
	MACDEMO("uninvited", "Demo", "Uninvited Demo", "e53adca77d773dca926f61faac68df86", 53119),
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

	const char *getName() const override {
		return "macventure";
	}

	const char *getEngineName() const override {
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
