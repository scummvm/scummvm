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
#include "common/config-manager.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/str-array.h"
#include "common/translation.h"
#include "common/util.h"
#include "ags/detection.h"
#include "ags/detection_tables.h"

#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

namespace AGS3 {

static const char *const HEAD_SIG = "CLIB\x1a";
static const char *const TAIL_SIG = "CLIB\x1\x2\x3\x4SIGE";
#define HEAD_SIG_SIZE 5
#define TAIL_SIG_SIZE 12

/**
 * Detect the presence of an AGS game
 * TODO: This is a compact version of MFLUtil::ReadSigsAndVersion. I didn't
 * use the full version due to the complexities of including it when
 * plugins are enabled. In the future, though, it would be nice to figure
 * out, since the full version can handle not detecting on files that are
 * AGS, but only contain sounds, etc. rather than a game
 */
static bool isAGSFile(Common::File &f) {
	// Check for signature at beginning of file
	char buffer[16];
	if (f.read(buffer, HEAD_SIG_SIZE) == HEAD_SIG_SIZE &&
			!memcmp(buffer, HEAD_SIG, HEAD_SIG_SIZE))
		return true;

	// Check for signature at end of EXE files
	f.seek(-TAIL_SIG_SIZE, SEEK_END);
	if (f.read(buffer, TAIL_SIG_SIZE) == TAIL_SIG_SIZE &&
		!memcmp(buffer, TAIL_SIG, TAIL_SIG_SIZE))
		return true;

	return false;
}

class AGSOptionsWidget : public GUI::OptionsContainerWidget {
public:
	explicit AGSOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);

	// OptionsContainerWidget API
	void load() override;
	bool save() override;

private:
	// OptionsContainerWidget API
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;

	GUI::PopUpWidget *_langPopUp;
	Common::StringArray _traFileNames;

	GUI::CheckboxWidget *_forceTextAACheckbox;
};

AGSOptionsWidget::AGSOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "AGSGameOptionsDialog", false, domain) {

	// Language
	GUI::StaticTextWidget *textWidget = new GUI::StaticTextWidget(widgetsBoss(), _dialogLayout + ".translation_desc", _("Game language:"), _("Language to use for multilingual games"));
	textWidget->setAlign(Graphics::kTextAlignRight);

	_langPopUp = new GUI::PopUpWidget(widgetsBoss(), _dialogLayout + ".translation");
	_langPopUp->appendEntry(_("<default>"), (uint32)-1);

	Common::String path = ConfMan.get("path", _domain);
	Common::FSDirectory dir(path);
	Common::ArchiveMemberList traFileList;
	dir.listMatchingMembers(traFileList, "*.tra");

	int i = 0;
	for (Common::ArchiveMemberList::iterator iter = traFileList.begin(); iter != traFileList.end(); ++iter) {
		Common::String traFileName = (*iter)->getName();
		traFileName.erase(traFileName.size() - 4); // remove .tra extension
		_traFileNames.push_back(traFileName);
		_langPopUp->appendEntry(traFileName, i++);
	}

	// Force font antialiasing
	_forceTextAACheckbox = new GUI::CheckboxWidget(widgetsBoss(), _dialogLayout + ".textAA", _("Force antialiased text"), _("Use antialiasing to draw text even if the game does not ask for it"));
}

void AGSOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout);
	layouts.addLayout(GUI::ThemeLayout::kLayoutVertical).addPadding(16, 16, 16, 16);

	layouts.addLayout(GUI::ThemeLayout::kLayoutHorizontal).addPadding(0, 0, 0, 0);
	layouts.addWidget("translation_desc", "OptionsLabel");
	layouts.addWidget("translation", "PopUp").closeLayout();

	layouts.addWidget("textAA", "Checkbox");

	layouts.closeLayout().closeDialog();
}

void AGSOptionsWidget::load() {
	Common::ConfigManager::Domain *gameConfig = ConfMan.getDomain(_domain);
	if (!gameConfig)
		return;

	uint32 curLangIndex = (uint32)-1;
	Common::String curLang;
	gameConfig->tryGetVal("translation", curLang);
	if (!curLang.empty()) {
		for (uint i = 0; i < _traFileNames.size(); ++i) {
			if (_traFileNames[i].equalsIgnoreCase(curLang)) {
				curLangIndex = i;
				break;
			}
		}
	}
	_langPopUp->setSelectedTag(curLangIndex);

	Common::String forceTextAA;
	gameConfig->tryGetVal("force_text_aa", forceTextAA);
	if (!forceTextAA.empty()) {
		bool val;
		if (parseBool(forceTextAA, val))
			_forceTextAACheckbox->setState(val);
	}
}

bool AGSOptionsWidget::save() {
	uint langIndex = _langPopUp->getSelectedTag();
	if (langIndex < _traFileNames.size())
		ConfMan.set("translation", _traFileNames[langIndex], _domain);
	else
		ConfMan.removeKey("translation", _domain);

	ConfMan.setBool("force_text_aa", _forceTextAACheckbox->getState(), _domain);

	return true;
}

} // namespace AGS3

const DebugChannelDef AGSMetaEngineDetection::debugFlagList[] = {
	{AGS::kDebugGraphics, "Graphics", "Graphics debug level"},
	{AGS::kDebugPath, "Path", "Pathfinding debug level"},
	{AGS::kDebugFilePath, "FilePath", "File path debug level"},
	{AGS::kDebugScan, "Scan", "Scan for unrecognised games"},
	{AGS::kDebugScript, "Script", "Enable debug script dump"},
	DEBUG_CHANNEL_END
};

AGSMetaEngineDetection::AGSMetaEngineDetection() : AdvancedMetaEngineDetection(AGS::GAME_DESCRIPTIONS,
	        sizeof(AGS::AGSGameDescription), AGS::GAME_NAMES) {
}

DetectedGames AGSMetaEngineDetection::detectGames(const Common::FSList &fslist) const {
	FileMap allFiles;

	if (fslist.empty())
		return DetectedGames();

	// Compose a hashmap of all files in fslist.
	composeFileHashMap(allFiles, fslist, (_maxScanDepth == 0 ? 1 : _maxScanDepth));

	// Run the detector on this
	ADDetectedGames matches = detectGame(fslist.begin()->getParent(), allFiles, Common::UNK_LANG, Common::kPlatformUnknown, "");

	cleanupPirated(matches);

	bool foundKnownGames = false;
	DetectedGames detectedGames;
	for (uint i = 0; i < matches.size(); i++) {
		DetectedGame game = toDetectedGame(matches[i]);
		if (game.hasUnknownFiles) {
			// Check the game is an AGS game
			for (FilePropertiesMap::const_iterator it = game.matchedFiles.begin(); it != game.matchedFiles.end(); it++) {
				Common::File f;
				if (f.open(allFiles[it->_key]) && AGS3::isAGSFile(f)) {
					detectedGames.push_back(game);
					break;
				}
			}
		} else {
			detectedGames.push_back(game);
			foundKnownGames = true;
		}
	}

	// If we didn't find a known game, also add a fallback detection
	if (!foundKnownGames) {
		// Use fallback detector if there were no matches by other means
		ADDetectedGame fallbackDetectionResult = fallbackDetect(allFiles, fslist);
		if (fallbackDetectionResult.desc) {
			DetectedGame fallbackDetectedGame = toDetectedGame(fallbackDetectionResult);
			fallbackDetectedGame.preferredTarget += "-fallback";

			detectedGames.push_back(fallbackDetectedGame);
		}
	}
	return detectedGames;
}

ADDetectedGame AGSMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	// Set the default values for the fallback descriptor's ADGameDescription part.
	AGS::g_fallbackDesc.desc.language = Common::UNK_LANG;
	AGS::g_fallbackDesc.desc.platform = Common::kPlatformUnknown;
	AGS::g_fallbackDesc.desc.flags = ADGF_NO_FLAGS;

	// FIXME: Hack to return match without checking for game data,
	// so that the command line game scanner will work
	if (ConfMan.get("gameid") == "ags-scan") {
		_gameid = "ags-scan";
		AGS::g_fallbackDesc.desc.gameId = "ags-scan";
		return ADDetectedGame(&AGS::g_fallbackDesc.desc);
	}

	// Set the defaults for gameid and extra
	_gameid = "ags";
	_extra.clear();
	bool hasUnknownFiles = true;

	// Scan for AGS games
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String filename = file->getName();
		if (!filename.hasSuffixIgnoreCase(".exe") &&
				!filename.hasSuffixIgnoreCase(".ags") &&
				!filename.equalsIgnoreCase("ac2game.dat"))
			// Neither, so move on
			continue;

		Common::File f;
		if (!f.open(allFiles[filename]))
			continue;

		if (AGS3::isAGSFile(f)) {
			_filename = filename;
			f.seek(0);
			_md5 = Common::computeStreamMD5AsString(f, 5000);

			// Check whether the game is in the detection list with a different filename
			for (const ::AGS::AGSGameDescription *gameP = ::AGS::GAME_DESCRIPTIONS;
				gameP->desc.gameId; ++gameP) {
				if (_md5 == gameP->desc.filesDescriptions[0].md5 &&
					f.size() == gameP->desc.filesDescriptions[0].fileSize) {
					hasUnknownFiles = false;
					_gameid = gameP->desc.gameId;
					break;
				}
			}

			AGS::g_fallbackDesc.desc.gameId = _gameid.c_str();
			AGS::g_fallbackDesc.desc.extra = _extra.c_str();
			AGS::g_fallbackDesc.desc.filesDescriptions[0].fileName = _filename.c_str();
			AGS::g_fallbackDesc.desc.filesDescriptions[0].fileSize = f.size();
			AGS::g_fallbackDesc.desc.filesDescriptions[0].md5 = _md5.c_str();

			ADDetectedGame game(&AGS::g_fallbackDesc.desc);
			game.matchedFiles[_filename].md5 = _md5;
			game.matchedFiles[_filename].size = f.size();

			game.hasUnknownFiles = hasUnknownFiles;
			return game;
		}
	}

	return ADDetectedGame();
}

GUI::OptionsContainerWidget *AGSMetaEngineDetection::buildEngineOptionsWidgetStatic(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new AGS3::AGSOptionsWidget(boss, name, target);
}

REGISTER_PLUGIN_STATIC(AGS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AGSMetaEngineDetection);
