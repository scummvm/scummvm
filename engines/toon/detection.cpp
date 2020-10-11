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

#include "engines/advancedDetector.h"

#include "base/plugins.h"

static const PlainGameDescriptor toonGames[] = {
	{ "toon", "Toonstruck" },
	{ 0, 0 }
};

namespace Toon {

static const ADGameDescription gameDescriptions[] = {
	{
		"toon", "",
		{
			{"local.pak", 0, "3290209ef9bc92692108dd2f45df0736", 3237611},
			{"arcaddbl.svl", 0, "c418478cd2833c7c983799f948af41ac", 7844688},
			{"study.svl", 0, "281efa3f33f6712c0f641a605f4d40fd", 2511090},
			AD_LISTEND
		},
		Common::EN_ANY, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GUIO_NOMIDI)
	},
	{
		"toon", "",
		{
			{"local.pak", 0, "517132c3575b38806d1e7b6f59848072", 3224044},
			{"arcaddbl.svl", 0, "ff74008827b62fbef1f46f104c438e44", 9699256},
			{"study.svl", 0, "df056b94ea83f1ed92a539cf636053ab", 2542668},
			AD_LISTEND
		},
		Common::FR_FRA, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GUIO_NOMIDI)
	},
	{
		"toon", "",
		{
			{"local.pak", 0, "bf5da4c03f78ffbd643f12122319366e", 3250841},
			{"arcaddbl.svl", 0, "7a0d74f4d66d1c722b946abbeb0834ef", 9122249},
			{"study.svl", 0, "72fe96a9e10967d3138e918295babc42", 2910283},
			AD_LISTEND
		},
		Common::DE_DEU, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GUIO_NOMIDI)
	},
	{
		"toon", "",
		{
			{"local.pak", 0, "e8645168a247e2abdbfc2f9fa9d1c0fa", 3232222},
			{"arcaddbl.svl", 0, "7893ac4cc78d51356baa058bbee7aa28", 8275016},
			{"study.svl", 0, "b6b1ee2d9d94d53d305856039ab7bde7", 2634620},
			AD_LISTEND
		},
		Common::ES_ESP, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GUIO_NOMIDI)
	},
	{
		"toon", "",
		{
			{"local.pak", 0, "48ec60709bebbdeff791d55ee18ec910", 3417846},
			{"arcaddbl.svl", 0, "1d1b96e317e03ffd3874a8ebe59556f3", 6246232},
			{"study.svl", 0, "d4aff126ee27be3c3d25e2996369d7cb", 2324368},
		},
		Common::RU_RUS, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GUIO_NOMIDI)
	},
	{
		"toon", "",
		{
			{"local.pak", 0, "bf5da4c03f78ffbd643f12122319366e", 3250841},
			{"wacexdbl.emc", 0, "cfbc2156a31b294b038204888407ebc8", 6974},
			{"generic.svl", 0, "5eb99850ada22f0b8cf6392262d4dd07", 9404599},
			AD_LISTEND
		},
		Common::DE_DEU, Common::kPlatformDOS, ADGF_DEMO, GUIO1(GUIO_NOMIDI)
	},
	{
		"toon", "",
		{
			{"local.pak", 0, "8ef3368078b9ea70b305c04db826feea", 2680573},
			{"generic.svl", 0, "5c42724bb93b360dca7044d6b7ef26e5", 7739319},
			AD_LISTEND
		},
		Common::EN_ANY, Common::kPlatformDOS, ADGF_DEMO, GUIO1(GUIO_NOMIDI)
	},
	{
		// English 2-CD "Sold out" release
		"toon", "",
		{
			{"local.pak", 0, "3290209ef9bc92692108dd2f45df0736", 3237611},
			{"generic.svl", 0, "331eead1d20af7ee809a9e2f35b8362f", 6945180},
			AD_LISTEND
		},
		Common::EN_ANY, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

static const ADFileBasedFallback fileBasedFallback[] = {
	{ &gameDescriptions[0], { "local.pak", "arcaddbl.svl", "study.svl", 0 } }, // default to english version
	{ 0, { 0 } }
};

} // End of namespace Toon

static const char * const directoryGlobs[] = {
	"misc",
	"act1",
	"arcaddbl",
	"act2",
	"study",
	0
};

class ToonMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	ToonMetaEngineDetection() : AdvancedMetaEngineDetection(Toon::gameDescriptions, sizeof(ADGameDescription), toonGames) {
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override {
		return detectGameFilebased(allFiles, Toon::fileBasedFallback);
	}

	const char *getEngineId() const override {
		return "toon";
	}

	const char *getName() const override {
		return "Toonstruck";
	}

	const char *getOriginalCopyright() const override {
		return "Toonstruck (C) 1996 Virgin Interactive";
	}
};

REGISTER_PLUGIN_STATIC(TOON_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ToonMetaEngineDetection);
