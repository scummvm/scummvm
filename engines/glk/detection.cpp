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
#include "common/md5.h"
#include "common/memstream.h"
#include "common/str-array.h"
#include "common/file.h"
#include "common/translation.h"
#include "common/config-manager.h"

#include "glk/detection.h"
#include "glk/game_description.h"

#include "glk/adrift/detection.h"
#include "glk/advsys/detection.h"
#include "glk/agt/detection.h"
#include "glk/alan2/detection.h"
#include "glk/alan3/detection.h"
#include "glk/archetype/detection.h"
#include "glk/glulx/detection.h"
#include "glk/hugo/detection.h"
#include "glk/jacl/detection.h"
#include "glk/level9/detection.h"
#include "glk/magnetic/detection.h"
#include "glk/quest/detection.h"
#include "glk/scott/detection.h"
#include "glk/zcode/detection.h"

#ifndef RELEASE_BUILD
#include "glk/comprehend/detection.h"
#include "glk/tads/detection.h"
#endif

#include "base/plugins.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/translation.h"

namespace Glk {

Common::String GlkDetectedGame::getGlkGUIOptions() {
#if defined (USE_TTS)
	return GUIO2(GUIO_NOMUSIC, GUIO_NOSUBTITLES);
#else
	return GUIO3(GUIO_NOSPEECH, GUIO_NOMUSIC, GUIO_NOSUBTITLES);
#endif
}

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const Common::String &filename,
		GameSupportLevel supportLevel) :
		DetectedGame("glk", id, desc, Common::EN_ANY, Common::kPlatformUnknown) {
	setGUIOptions(getGlkGUIOptions());
	gameSupportLevel = supportLevel;
	addExtraEntry("filename", filename);
}

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const Common::String &filename,
		Common::Language lang, GameSupportLevel supportLevel) : DetectedGame("glk", id, desc, lang, Common::kPlatformUnknown) {
	setGUIOptions(getGlkGUIOptions());
	gameSupportLevel = supportLevel;
	addExtraEntry("filename", filename);
}

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const char *xtra,
		const Common::String &filename, Common::Language lang,
		GameSupportLevel supportLevel) :
		DetectedGame("glk", id, desc, lang, Common::kPlatformUnknown, xtra) {
	setGUIOptions(getGlkGUIOptions());
	gameSupportLevel = supportLevel;
	addExtraEntry("filename", filename);
}

GlkDetectedGame::GlkDetectedGame(const char *id, const char *desc, const Common::String &filename,
		const Common::String &md5, size_t filesize, GameSupportLevel supportLevel) :
		DetectedGame("glk", id, desc, Common::UNK_LANG, Common::kPlatformUnknown) {
	setGUIOptions(getGlkGUIOptions());
	gameSupportLevel = supportLevel;
	addExtraEntry("filename", filename);

	canBeAdded = true;
	hasUnknownFiles = true;

	FileProperties fp;
	fp.md5 = md5;
	fp.size = filesize;
	matchedFiles[filename] = fp;
}

} // End of namespace Glk

PlainGameList GlkMetaEngineDetection::getSupportedGames() const {
	PlainGameList list;
	Glk::Adrift::AdriftMetaEngine::getSupportedGames(list);
	Glk::AdvSys::AdvSysMetaEngine::getSupportedGames(list);
	Glk::AGT::AGTMetaEngine::getSupportedGames(list);
	Glk::Alan2::Alan2MetaEngine::getSupportedGames(list);
	Glk::Alan3::Alan3MetaEngine::getSupportedGames(list);
	Glk::Archetype::ArchetypeMetaEngine::getSupportedGames(list);
	Glk::Glulx::GlulxMetaEngine::getSupportedGames(list);
	Glk::Hugo::HugoMetaEngine::getSupportedGames(list);
	Glk::JACL::JACLMetaEngine::getSupportedGames(list);
	Glk::Level9::Level9MetaEngine::getSupportedGames(list);
	Glk::Magnetic::MagneticMetaEngine::getSupportedGames(list);
	Glk::Quest::QuestMetaEngine::getSupportedGames(list);
	Glk::Scott::ScottMetaEngine::getSupportedGames(list);
	Glk::ZCode::ZCodeMetaEngine::getSupportedGames(list);
#ifndef RELEASE_BUILD
	Glk::Comprehend::ComprehendMetaEngine::getSupportedGames(list);
	Glk::TADS::TADSMetaEngine::getSupportedGames(list);
#endif

	return list;
}

#define FIND_GAME(SUBENGINE) \
	Glk::GameDescriptor gd##SUBENGINE = Glk::SUBENGINE::SUBENGINE##MetaEngine::findGame(gameId); \
	if (gd##SUBENGINE._description) return gd##SUBENGINE

PlainGameDescriptor GlkMetaEngineDetection::findGame(const char *gameId) const {
	FIND_GAME(Adrift);
	FIND_GAME(AdvSys);
	FIND_GAME(Alan2);
	FIND_GAME(AGT);
	FIND_GAME(Alan3);
	FIND_GAME(Archetype);
	FIND_GAME(Glulx);
	FIND_GAME(Hugo);
	FIND_GAME(JACL);
	FIND_GAME(Level9);
	FIND_GAME(Magnetic);
	FIND_GAME(Quest);
	FIND_GAME(Scott);
	FIND_GAME(ZCode);
#ifndef RELEASE_BUILD
	FIND_GAME(Comprehend);
	FIND_GAME(TADS);
#endif

	return PlainGameDescriptor();
}

#undef FIND_GAME

DetectedGames GlkMetaEngineDetection::detectGames(const Common::FSList &fslist) const {
#ifndef RELEASE_BUILD
	// This is as good a place as any to detect multiple sub-engines using the same Ids
	detectClashes();
#endif

	DetectedGames detectedGames;
	Glk::Adrift::AdriftMetaEngine::detectGames(fslist, detectedGames);
	Glk::AdvSys::AdvSysMetaEngine::detectGames(fslist, detectedGames);
	Glk::AGT::AGTMetaEngine::detectGames(fslist, detectedGames);
	Glk::Alan2::Alan2MetaEngine::detectGames(fslist, detectedGames);
	Glk::Alan3::Alan3MetaEngine::detectGames(fslist, detectedGames);
	Glk::Archetype::ArchetypeMetaEngine::detectGames(fslist, detectedGames);
	Glk::Glulx::GlulxMetaEngine::detectGames(fslist, detectedGames);
	Glk::Hugo::HugoMetaEngine::detectGames(fslist, detectedGames);
	Glk::JACL::JACLMetaEngine::detectGames(fslist, detectedGames);
	Glk::Level9::Level9MetaEngine::detectGames(fslist, detectedGames);
	Glk::Magnetic::MagneticMetaEngine::detectGames(fslist, detectedGames);
	Glk::Quest::QuestMetaEngine::detectGames(fslist, detectedGames);
	Glk::Scott::ScottMetaEngine::detectGames(fslist, detectedGames);
	Glk::ZCode::ZCodeMetaEngine::detectGames(fslist, detectedGames);
#ifndef RELEASE_BUILD
	Glk::Comprehend::ComprehendMetaEngine::detectGames(fslist, detectedGames);
	Glk::TADS::TADSMetaEngine::detectGames(fslist, detectedGames);
#endif

	return detectedGames;
}

void GlkMetaEngineDetection::detectClashes() const {
	Common::StringMap map;
	Glk::Adrift::AdriftMetaEngine::detectClashes(map);
	Glk::AdvSys::AdvSysMetaEngine::detectClashes(map);
	Glk::AGT::AGTMetaEngine::detectClashes(map);
	Glk::Alan2::Alan2MetaEngine::detectClashes(map);
	Glk::Alan3::Alan3MetaEngine::detectClashes(map);
	Glk::Archetype::ArchetypeMetaEngine::detectClashes(map);
	Glk::Glulx::GlulxMetaEngine::detectClashes(map);
	Glk::Hugo::HugoMetaEngine::detectClashes(map);
	Glk::JACL::JACLMetaEngine::detectClashes(map);
	Glk::Level9::Level9MetaEngine::detectClashes(map);
	Glk::Magnetic::MagneticMetaEngine::detectClashes(map);
	Glk::Quest::QuestMetaEngine::detectClashes(map);
	Glk::Scott::ScottMetaEngine::detectClashes(map);
	Glk::ZCode::ZCodeMetaEngine::detectClashes(map);
#ifndef RELEASE_BUILD
	Glk::Comprehend::ComprehendMetaEngine::detectClashes(map);
	Glk::TADS::TADSMetaEngine::detectClashes(map);
#endif
}

const ExtraGuiOptions GlkMetaEngineDetection::getExtraGuiOptions(const Common::String &) const {
	ExtraGuiOptions  options;
#if defined(USE_TTS)
	static const ExtraGuiOption ttsSpeakOptions = {
		_s("Enable Text to Speech"),
		_s("Use TTS to read the text"),
		"speak",
		false
	};
	static const ExtraGuiOption ttsSpeakInputOptions = {
		_s("Also read input text"),
		_s("Use TTS to read the input text"),
		"speak_input",
		false
	};
	options.push_back(ttsSpeakOptions);
	options.push_back(ttsSpeakInputOptions);
#endif
	return options;
}

REGISTER_PLUGIN_STATIC(GLK_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, GlkMetaEngineDetection);
