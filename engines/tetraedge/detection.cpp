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

#include "common/config-manager.h"
#include "common/file.h"

#include "tetraedge/detection.h"
#include "tetraedge/metaengine.h"
#include "tetraedge/detection_tables.h"
#include "tetraedge/obb_archive.h"

const DebugChannelDef TetraedgeMetaEngineDetection::debugFlagList[] = {
	{ Tetraedge::kDebugGraphics, "Graphics", "Graphics debug level" },
	{ Tetraedge::kDebugPath, "Path", "Pathfinding debug level" },
	{ Tetraedge::kDebugFilePath, "FilePath", "File path debug level" },
	{ Tetraedge::kDebugScan, "Scan", "Scan for unrecognised games" },
	{ Tetraedge::kDebugScript, "Script", "Enable debug script dump" },
	DEBUG_CHANNEL_END
};

TetraedgeMetaEngineDetection::TetraedgeMetaEngineDetection() : AdvancedMetaEngineDetection(Tetraedge::GAME_DESCRIPTIONS,
	sizeof(ADGameDescription), Tetraedge::GAME_NAMES) {
	_flags = kADFlagMatchFullPaths;
}

Common::String TetraedgeMetaEngineDetection::customizeGuiOptionsLanguages(const Common::String &optionsString, const Common::String &domain) const {
	Common::String result;

	struct {
		Common::Language id;
		const char *code;
	} languages[] = {
		{ Common::EN_ANY, "en" },
		{ Common::FR_FRA, "fr" },
		{ Common::DE_DEU, "de" },
		{ Common::IT_ITA, "it" },
		{ Common::ES_ESP, "es" },
		{ Common::RU_RUS, "ru" },
		{ Common::HE_ISR, "he" }  // This is a Fan-translation, which requires additional patch
	};

	static const char *obbNames[] = {
		"main.5.com.microids.syberia.obb",
		"main.12.com.microids.syberia.obb",
		"main.2.ru.buka.syberia1.obb",
		"main.4.com.microids.syberia2.obb",
		"main.2.ru.buka.syberia2.obb",
	};

	static const char *subDirs[] = {
		nullptr,
		"PC-MacOSX-Android-iPhone-iPad"
	};

	bool hasLang[ARRAYSIZE(languages)];

	memset(hasLang, 0, sizeof(hasLang));

	const Common::Platform platform = Common::parsePlatform(ConfMan.get("platform", domain));

	Common::FSNode dir(ConfMan.get("path", domain));

	if (platform == Common::Platform::kPlatformMacintosh)
		dir = dir.getChild("Resources");

	for (uint i = 0; i < ARRAYSIZE(languages); i++) {
		Common::FSNode base = dir.getChild("texts");
		for (uint k = 0; k < ARRAYSIZE(subDirs); k++) {
			Common::FSNode base2 = subDirs[k] ? base.getChild(subDirs[k]) : base;
			if (base2.getChild(Common::String::format("%s.xml", languages[i].code)).exists())
				hasLang[i] = true;
		}
	}

	if (platform == Common::Platform::kPlatformAndroid)
		for (uint j = 0; j < ARRAYSIZE(obbNames); j++) {
			Common::FSNode obbPath = dir.getChild(obbNames[j]);
			Common::File obbFile;
			if (!obbPath.exists() || !obbFile.open(obbPath))
				continue;

			Tetraedge::ObbArchive::FileMap fileMap;
			if (!Tetraedge::ObbArchive::readFileMap(obbFile, fileMap))
				continue;

			for (uint i = 0; i < ARRAYSIZE(languages); i++)
				for (uint k = 0; k < ARRAYSIZE(subDirs); k++) {
					Common::String dname = "texts/";
					if (subDirs[k]) {
						dname += subDirs[k];
						dname += "/";
					}
					if (fileMap.contains(dname + languages[i].code))
						hasLang[i] = true;
				}
		}


	for (uint i = 0; i < ARRAYSIZE(languages); i++)
		if(hasLang[i])
			result += " " + Common::getGameGUIOptionsDescriptionLanguage(languages[i].id);

	return result;
}

REGISTER_PLUGIN_STATIC(TETRAEDGE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TetraedgeMetaEngineDetection);
