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

#include "base/plugins.h"

#include "engines/advancedDetector.h"

namespace MacVenture {

#define ADGF_DEFAULT (ADGF_DROPLANGUAGE|ADGF_DROPPLATFORM|ADGF_MACRESFORK|ADGF_UNSTABLE)

#define BASEGAME(n, v, f, md5, s) {n, v, AD_ENTRY1s(f, md5, s), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_DEFAULT, GUIO1(GUIO_NOMIDI)}

static const ADGameDescription gameDescriptions[] = {
	BASEGAME("shadowgate", "Zojoi Rerelease", "Shadowgate.bin", "ebbfbcbf93938bd2900cb0c0213b19ad", 68974), // Zojoi Rerelease
	BASEGAME("deja_vu", "Zojoi Rerelease", "Deja Vu.bin", "5e9f5a8e3c8eb29ed02b34ae5937354f", 69034), // Zojoi Rerelease
	BASEGAME("deja_vu2", "Zojoi Rerelease", "Lost in Las Vegas.bin", "8f8e1d8d41f577ee0fbc03847969af0d", 66520), // Zojoi Rerelease
	AD_TABLE_END_MARKER
};

} // End of namespace MacVenture

static const PlainGameDescriptor macventureGames[] = {
	{ "shadowgate", "Shadowgate" },
	{ "deja_vu", "Deja Vu"},
	{ "deja_vu2", "Deja Vu II"},
	{ 0, 0 }
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
};

} // End of namespace MacVenture

REGISTER_PLUGIN_STATIC(MACVENTURE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, MacVenture::MacVentureMetaEngineDetection);
