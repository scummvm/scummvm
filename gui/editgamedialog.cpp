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

#include "gui/editgamedialog.h"

#include "backends/keymapper/keymapper.h"

#include "common/config-manager.h"
#include "common/gui_options.h"
#include "common/translation.h"
#include "common/system.h"

#include "gui/browser.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#ifdef ENABLE_EVENTRECORDER
#include "gui/onscreendialog.h"
#include "gui/recorderdialog.h"
#include "gui/EventRecorder.h"
#endif
#include "gui/widgets/edittext.h"
#include "gui/widgets/tab.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/scrollcontainer.h"

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
#include "backends/cloud/cloudmanager.h"
#endif

using Common::ConfigManager;

namespace GUI {

enum {
	kStartCmd = 'STRT',
	kAboutCmd = 'ABOU',
	kOptionsCmd = 'OPTN',
	kAddGameCmd = 'ADDG',
	kEditGameCmd = 'EDTG',
	kRemoveGameCmd = 'REMG',
	kLoadGameCmd = 'LOAD',
	kQuitCmd = 'QUIT',
	kSearchCmd = 'SRCH',
	kListSearchCmd = 'LSSR',
	kSearchClearCmd = 'SRCL',

	kCmdGlobalGraphicsOverride = 'OGFX',
	kCmdGlobalShaderOverride = 'OSHD',
	kCmdGlobalAudioOverride = 'OSFX',
	kCmdGlobalMIDIOverride = 'OMID',
	kCmdGlobalMT32Override = 'OM32',
	kCmdGlobalVolumeOverride = 'OVOL',

	kCmdChooseSoundFontCmd = 'chsf',

	kCmdExtraBrowser = 'PEXT',
	kCmdExtraPathClear = 'PEXC',
	kCmdGameBrowser = 'PGME',
	kCmdSaveBrowser = 'PSAV',
	kCmdSavePathClear = 'PSAC',

	kGraphicsTabContainerReflowCmd = 'gtcr'
};

/*
* TODO: Clean up this ugly design: we subclass EditTextWidget to perform
* input validation. It would be much more elegant to use a decorator pattern,
* or a validation callback, or something like that.
*/
class DomainEditTextWidget : public EditTextWidget {
public:
	DomainEditTextWidget(GuiObject *boss, const String &name, const U32String &text, const U32String &tooltip = U32String())
		: EditTextWidget(boss, name, text, tooltip) {}

protected:
	bool tryInsertChar(byte c, int pos) override {
		if (Common::isAlnum(c) || c == '-' || c == '_') {
			_editString.insertChar(c, pos);
			return true;
		}
		return false;
	}
};

EditGameDialog::EditGameDialog(const String &domain)
	: OptionsDialog(domain, "GameOptions") {
	EngineMan.upgradeTargetIfNecessary(domain);

	_engineOptions = nullptr;

	// Retrieve the plugin, since we need to access the engine's MetaEngine
	// implementation.
	const Plugin *metaEnginePlugin = nullptr;
	const Plugin *enginePlugin = nullptr;
	QualifiedGameDescriptor qgd = EngineMan.findTarget(domain, &metaEnginePlugin);
	if (!metaEnginePlugin) {
		warning("MetaEnginePlugin for target \"%s\" not found!", domain.c_str());
	} else {
		enginePlugin = PluginMan.getEngineFromMetaEngine(metaEnginePlugin);
		if (!enginePlugin) {
			warning("Engine Plugin for target \"%s\" not found! Game specific settings might be missing.", domain.c_str());
		}
	}

	// GAME: Path to game data (r/o), extra data (r/o), and save data (r/w)
	String gamePath(ConfMan.get("path", _domain));
	String extraPath(ConfMan.get("extrapath", _domain));
	String savePath(ConfMan.get("savepath", _domain));

	// GAME: Determine the description string
	String description(ConfMan.get("description", domain));
	if (description.empty() && !qgd.description.empty()) {
		description = qgd.description;
	}

	// GUI:  Add tab widget
	TabWidget *tab = new TabWidget(this, "GameOptions.TabWidget");

	//
	// 1) The game tab
	//
	tab->addTab(_("Game"), "GameOptions_Game");

	// GUI:  Label & edit widget for the game ID
	if (g_system->getOverlayWidth() > 320)
		new StaticTextWidget(tab, "GameOptions_Game.Id", _("ID:"), _("Short game identifier used for referring to saved games and running the game from the command line"));
	else
		new StaticTextWidget(tab, "GameOptions_Game.Id", _c("ID:", "lowres"), _("Short game identifier used for referring to saved games and running the game from the command line"));
	_domainWidget = new DomainEditTextWidget(tab, "GameOptions_Game.Domain", _domain, _("Short game identifier used for referring to saved games and running the game from the command line"));

	// GUI:  Label & edit widget for the description
	if (g_system->getOverlayWidth() > 320)
		new StaticTextWidget(tab, "GameOptions_Game.Name", _("Name:"), _("Full title of the game"));
	else
		new StaticTextWidget(tab, "GameOptions_Game.Name", _c("Name:", "lowres"), _("Full title of the game"));
	_descriptionWidget = new EditTextWidget(tab, "GameOptions_Game.Desc", description, _("Full title of the game"));

	// Language popup
	_langPopUpDesc = new StaticTextWidget(tab, "GameOptions_Game.LangPopupDesc", _("Language:"), _("Language of the game. This will not turn your Spanish game version into English"));
	_langPopUp = new PopUpWidget(tab, "GameOptions_Game.LangPopup", _("Language of the game. This will not turn your Spanish game version into English"));
	_langPopUp->appendEntry(_("<default>"), (uint32)Common::UNK_LANG);
	_langPopUp->appendEntry("", (uint32)Common::UNK_LANG);
	const Common::LanguageDescription *l = Common::g_languages;
	for (; l->code; ++l) {
		if (checkGameGUIOptionLanguage(l->id, _guioptionsString))
			_langPopUp->appendEntry(l->description, l->id);
	}

	// Platform popup
	if (g_system->getOverlayWidth() > 320)
		_platformPopUpDesc = new StaticTextWidget(tab, "GameOptions_Game.PlatformPopupDesc", _("Platform:"), _("Platform the game was originally designed for"));
	else
		_platformPopUpDesc = new StaticTextWidget(tab, "GameOptions_Game.PlatformPopupDesc", _c("Platform:", "lowres"), _("Platform the game was originally designed for"));
	_platformPopUp = new PopUpWidget(tab, "GameOptions_Game.PlatformPopup", _("Platform the game was originally designed for"));
	_platformPopUp->appendEntry(_("<default>"));
	_platformPopUp->appendEntry("");
	const Common::PlatformDescription *p = Common::g_platforms;
	for (; p->code; ++p) {
		_platformPopUp->appendEntry(p->description, p->id);
	}

	//
	// 2) The engine tab (shown only if the engine implements one or there are custom engine options)
	//

	if (metaEnginePlugin) {
		int tabId = tab->addTab(_("Engine"), "GameOptions_Engine");

		const MetaEngineDetection &metaEngine = metaEnginePlugin->get<MetaEngineDetection>();
		metaEngine.registerDefaultSettings(_domain);
		_engineOptions = metaEngine.buildEngineOptionsWidgetStatic(tab, "GameOptions_Engine.Container", _domain);

		if (_engineOptions) {
			_engineOptions->setParentDialog(this);
		} else {
			tab->removeTab(tabId);
		}
	}

	//
	// 3) The graphics tab
	//
	_graphicsTabId = tab->addTab(g_system->getOverlayWidth() > 320 ? _("Graphics") : _("GFX"), "GameOptions_Graphics");
	ScrollContainerWidget *graphicsContainer = new ScrollContainerWidget(tab, "GameOptions_Graphics.Container", "GameOptions_Graphics_Container", kGraphicsTabContainerReflowCmd);
	graphicsContainer->setBackgroundType(ThemeEngine::kWidgetBackgroundNo);
	graphicsContainer->setTarget(this);

	if (g_system->getOverlayWidth() > 320)
		_globalGraphicsOverride = new CheckboxWidget(graphicsContainer, "GameOptions_Graphics_Container.EnableTabCheckbox", _("Override global graphic settings"), Common::U32String(), kCmdGlobalGraphicsOverride);
	else
		_globalGraphicsOverride = new CheckboxWidget(graphicsContainer, "GameOptions_Graphics_Container.EnableTabCheckbox", _c("Override global graphic settings", "lowres"), Common::U32String(), kCmdGlobalGraphicsOverride);

	addGraphicControls(graphicsContainer, "GameOptions_Graphics_Container.");

	//
	// The shader tab (currently visible only for Vita platform), visibility checking by features
	//

	_globalShaderOverride = nullptr;
	if (g_system->hasFeature(OSystem::kFeatureShader)) {
		tab->addTab(_("Shader"), "GameOptions_Shader");

		if (g_system->getOverlayWidth() > 320)
			_globalShaderOverride = new CheckboxWidget(tab, "GameOptions_Shader.EnableTabCheckbox", _("Override global shader settings"), Common::U32String(), kCmdGlobalShaderOverride);
		else
			_globalShaderOverride = new CheckboxWidget(tab, "GameOptions_Shader.EnableTabCheckbox", _c("Override global shader settings", "lowres"), Common::U32String(), kCmdGlobalShaderOverride);

		addShaderControls(tab, "GameOptions_Shader.");
	}

	//
	// The Keymap tab
	//
	Common::KeymapArray keymaps;
	if (enginePlugin) {
		keymaps = enginePlugin->get<MetaEngine>().initKeymaps(domain.c_str());
	}

	if (!keymaps.empty()) {
		tab->addTab(_("Keymaps"), "GameOptions_KeyMapper");
		addKeyMapperControls(tab, "GameOptions_KeyMapper.", keymaps, domain);
	}

	//
	// The backend tab (shown only if the backend implements one)
	//
	int backendTabId = tab->addTab(_("Backend"), "GameOptions_Backend");

	g_system->registerDefaultSettings(_domain);
	_backendOptions = g_system->buildBackendOptionsWidget(tab, "GameOptions_Backend.Container", _domain);

	if (_backendOptions) {
		_backendOptions->setParentDialog(this);
	} else {
		tab->removeTab(backendTabId);
	}

	//
	// 4) The audio tab
	//
	tab->addTab(_("Audio"), "GameOptions_Audio");

	if (g_system->getOverlayWidth() > 320)
		_globalAudioOverride = new CheckboxWidget(tab, "GameOptions_Audio.EnableTabCheckbox", _("Override global audio settings"), Common::U32String(), kCmdGlobalAudioOverride);
	else
		_globalAudioOverride = new CheckboxWidget(tab, "GameOptions_Audio.EnableTabCheckbox", _c("Override global audio settings", "lowres"), Common::U32String(), kCmdGlobalAudioOverride);

	addAudioControls(tab, "GameOptions_Audio.");
	addSubtitleControls(tab, "GameOptions_Audio.");

	//
	// 5) The volume tab
	//
	if (g_system->getOverlayWidth() > 320)
		tab->addTab(_("Volume"), "GameOptions_Volume");
	else
		tab->addTab(_c("Volume", "lowres"), "GameOptions_Volume");

	if (g_system->getOverlayWidth() > 320)
		_globalVolumeOverride = new CheckboxWidget(tab, "GameOptions_Volume.EnableTabCheckbox", _("Override global volume settings"), Common::U32String(), kCmdGlobalVolumeOverride);
	else
		_globalVolumeOverride = new CheckboxWidget(tab, "GameOptions_Volume.EnableTabCheckbox", _c("Override global volume settings", "lowres"), Common::U32String(), kCmdGlobalVolumeOverride);

	addVolumeControls(tab, "GameOptions_Volume.");

	bool showMidi = !_guioptions.contains(GUIO_NOMIDI) && !_guioptions.contains(GUIO_NOMUSIC);

	//
	// 6) The MIDI tab
	//
	_globalMIDIOverride = nullptr;
	if (showMidi) {
		tab->addTab(_("MIDI"), "GameOptions_MIDI");

		if (g_system->getOverlayWidth() > 320)
			_globalMIDIOverride = new CheckboxWidget(tab, "GameOptions_MIDI.EnableTabCheckbox", _("Override global MIDI settings"), Common::U32String(), kCmdGlobalMIDIOverride);
		else
			_globalMIDIOverride = new CheckboxWidget(tab, "GameOptions_MIDI.EnableTabCheckbox", _c("Override global MIDI settings", "lowres"), Common::U32String(), kCmdGlobalMIDIOverride);

		addMIDIControls(tab, "GameOptions_MIDI.");
	}

	//
	// 7) The MT-32 tab
	//
	_globalMT32Override = nullptr;
	if (showMidi) {
		tab->addTab(_("MT-32"), "GameOptions_MT32");

		if (g_system->getOverlayWidth() > 320)
			_globalMT32Override = new CheckboxWidget(tab, "GameOptions_MT32.EnableTabCheckbox", _("Override global MT-32 settings"), Common::U32String(), kCmdGlobalMT32Override);
		else
			_globalMT32Override = new CheckboxWidget(tab, "GameOptions_MT32.EnableTabCheckbox", _c("Override global MT-32 settings", "lowres"), Common::U32String(), kCmdGlobalMT32Override);

		addMT32Controls(tab, "GameOptions_MT32.");
	}

	//
	// 8) The Paths tab
	//
	if (g_system->getOverlayWidth() > 320)
		tab->addTab(_("Paths"), "GameOptions_Paths");
	else
		tab->addTab(_c("Paths", "lowres"), "GameOptions_Paths");

	// These buttons have to be extra wide, or the text will be truncated
	// in the small version of the GUI.

	// GUI:  Button + Label for the game path
	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GameOptions_Paths.Gamepath", _("Game Path:"), Common::U32String(), kCmdGameBrowser);
	else
		new ButtonWidget(tab, "GameOptions_Paths.Gamepath", _c("Game Path:", "lowres"), Common::U32String(), kCmdGameBrowser);
	_gamePathWidget = new StaticTextWidget(tab, "GameOptions_Paths.GamepathText", gamePath);

	// GUI:  Button + Label for the additional path
	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GameOptions_Paths.Extrapath", _("Extra Path:"), _("Specifies path to additional data used by the game"), kCmdExtraBrowser);
	else
		new ButtonWidget(tab, "GameOptions_Paths.Extrapath", _c("Extra Path:", "lowres"), _("Specifies path to additional data used by the game"), kCmdExtraBrowser);
	_extraPathWidget = new StaticTextWidget(tab, "GameOptions_Paths.ExtrapathText", extraPath, _("Specifies path to additional data used by the game"));

	_extraPathClearButton = addClearButton(tab, "GameOptions_Paths.ExtraPathClearButton", kCmdExtraPathClear);

	// GUI:  Button + Label for the save path
	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GameOptions_Paths.Savepath", _("Save Path:"), _("Specifies where your saved games are put"), kCmdSaveBrowser);
	else
		new ButtonWidget(tab, "GameOptions_Paths.Savepath", _c("Save Path:", "lowres"), _("Specifies where your saved games are put"), kCmdSaveBrowser);
	_savePathWidget = new StaticTextWidget(tab, "GameOptions_Paths.SavepathText", savePath, _("Specifies where your saved games are put"));

	_savePathClearButton = addClearButton(tab, "GameOptions_Paths.SavePathClearButton", kCmdSavePathClear);

	//
	// 9) The Achievements tab
	//
	if (enginePlugin) {
		const MetaEngine &metaEngineConnect = enginePlugin->get<MetaEngine>();
		Common::AchievementsInfo achievementsInfo = metaEngineConnect.getAchievementsInfo(domain);
		if (achievementsInfo.descriptions.size() > 0) {
			tab->addTab(_("Achievements"), "GameOptions_Achievements");
			addAchievementsControls(tab, "GameOptions_Achievements.", achievementsInfo);
		}
	}

	// Activate the first tab
	tab->setActiveTab(0);
	_tabWidget = tab;

	// Add OK & Cancel buttons
	new ButtonWidget(this, "GameOptions.Cancel", _("Cancel"), Common::U32String(), kCloseCmd);
	new ButtonWidget(this, "GameOptions.Ok", _("OK"), Common::U32String(), kOKCmd);
}

void EditGameDialog::setupGraphicsTab() {
	OptionsDialog::setupGraphicsTab();
	_globalGraphicsOverride->setVisible(true);
}

void EditGameDialog::open() {
	OptionsDialog::open();

	String extraPath(ConfMan.get("extrapath", _domain));
	if (extraPath.empty() || !ConfMan.hasKey("extrapath", _domain)) {
		_extraPathWidget->setLabel(_c("None", "path"));
	}

	String savePath(ConfMan.get("savepath", _domain));
	if (savePath.empty() || !ConfMan.hasKey("savepath", _domain)) {
		_savePathWidget->setLabel(_("Default"));
	}

	int sel, i;
	bool e;

	// En-/disable dialog items depending on whether overrides are active or not.

	e = ConfMan.hasKey("gfx_mode", _domain) ||
		ConfMan.hasKey("render_mode", _domain) ||
		ConfMan.hasKey("stretch_mode", _domain) ||
		ConfMan.hasKey("aspect_ratio", _domain) ||
		ConfMan.hasKey("fullscreen", _domain) ||
		ConfMan.hasKey("vsync", _domain) ||
		ConfMan.hasKey("filtering", _domain) ||
		ConfMan.hasKey("renderer", _domain) ||
		ConfMan.hasKey("antialiasing", _domain);
	_globalGraphicsOverride->setState(e);

	if (g_system->hasFeature(OSystem::kFeatureShader)) {
		e = ConfMan.hasKey("shader", _domain);
		_globalShaderOverride->setState(e);
	}

	e = ConfMan.hasKey("music_driver", _domain) ||
		ConfMan.hasKey("output_rate", _domain) ||
		ConfMan.hasKey("opl_driver", _domain) ||
		ConfMan.hasKey("subtitles", _domain) ||
		ConfMan.hasKey("talkspeed", _domain);
	_globalAudioOverride->setState(e);

	e = ConfMan.hasKey("music_volume", _domain) ||
		ConfMan.hasKey("sfx_volume", _domain) ||
		ConfMan.hasKey("speech_volume", _domain);
	_globalVolumeOverride->setState(e);

	if (!_guioptions.contains(GUIO_NOMIDI) && !_guioptions.contains(GUIO_NOMUSIC)) {
		e = ConfMan.hasKey("soundfont", _domain) ||
			ConfMan.hasKey("multi_midi", _domain) ||
			ConfMan.hasKey("midi_gain", _domain);
		_globalMIDIOverride->setState(e);

		e = ConfMan.hasKey("native_mt32", _domain) ||
			ConfMan.hasKey("enable_gs", _domain);
		_globalMT32Override->setState(e);
	}

	// TODO: game path

	const Common::Language lang = Common::parseLanguage(ConfMan.get("language", _domain));

	if (ConfMan.hasKey("language", _domain)) {
		_langPopUp->setSelectedTag(lang);
	} else {
		_langPopUp->setSelectedTag((uint32)Common::UNK_LANG);
	}

	if (_langPopUp->numEntries() <= 3) { // If only one language is avaliable
		_langPopUpDesc->setEnabled(false);
		_langPopUp->setEnabled(false);
	}

	if (_engineOptions) {
		_engineOptions->load();
	}

	const Common::PlatformDescription *p = Common::g_platforms;
	const Common::Platform platform = Common::parsePlatform(ConfMan.get("platform", _domain));
	sel = 0;
	for (i = 0; p->code; ++p, ++i) {
		if (platform == p->id)
			sel = i + 2;
	}
	_platformPopUp->setSelected(sel);
}

void EditGameDialog::apply() {
	ConfMan.set("description", _descriptionWidget->getEditString(), _domain);

	Common::Language lang = (Common::Language)_langPopUp->getSelectedTag();
	if (lang < 0)
		ConfMan.removeKey("language", _domain);
	else
		ConfMan.set("language", Common::getLanguageCode(lang), _domain);

	U32String gamePath(_gamePathWidget->getLabel());
	if (!gamePath.empty())
		ConfMan.set("path", gamePath, _domain);

	U32String extraPath(_extraPathWidget->getLabel());
	if (!extraPath.empty() && (extraPath != _c("None", "path")))
		ConfMan.set("extrapath", extraPath, _domain);
	else
		ConfMan.removeKey("extrapath", _domain);

	U32String savePath(_savePathWidget->getLabel());
	if (!savePath.empty() && (savePath != _("Default")))
		ConfMan.set("savepath", savePath, _domain);
	else
		ConfMan.removeKey("savepath", _domain);

	Common::Platform platform = (Common::Platform)_platformPopUp->getSelectedTag();
	if (platform < 0)
		ConfMan.removeKey("platform", _domain);
	else
		ConfMan.set("platform", Common::getPlatformCode(platform), _domain);

	if (_engineOptions) {
		_engineOptions->save();
	}

	OptionsDialog::apply();
}

void EditGameDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCmdGlobalGraphicsOverride:
		setGraphicSettingsState(data != 0);
		g_gui.scheduleTopDialogRedraw();
		break;
	case kCmdGlobalShaderOverride:
		setShaderSettingsState(data != 0);
		g_gui.scheduleTopDialogRedraw();
		break;
	case kCmdGlobalAudioOverride:
		setAudioSettingsState(data != 0);
		setSubtitleSettingsState(data != 0);
		if (_globalVolumeOverride == nullptr)
			setVolumeSettingsState(data != 0);
		g_gui.scheduleTopDialogRedraw();
		break;
	case kCmdGlobalMIDIOverride:
		setMIDISettingsState(data != 0);
		g_gui.scheduleTopDialogRedraw();
		break;
	case kCmdGlobalMT32Override:
		setMT32SettingsState(data != 0);
		g_gui.scheduleTopDialogRedraw();
		break;
	case kCmdGlobalVolumeOverride:
		setVolumeSettingsState(data != 0);
		g_gui.scheduleTopDialogRedraw();
		break;
	case kCmdChooseSoundFontCmd:
	{
		BrowserDialog browser(_("Select SoundFont"), false);

		if (browser.runModal() > 0) {
			// User made this choice...
			Common::FSNode file(browser.getResult());
			_soundFont->setLabel(file.getPath());

			if (!file.getPath().empty() && (file.getPath() != Common::convertFromU32String(_c("None", "path"))))
				_soundFontClearButton->setEnabled(true);
			else
				_soundFontClearButton->setEnabled(false);

			g_gui.scheduleTopDialogRedraw();
		}
		break;
	}

	// Change path for the game
	case kCmdGameBrowser:
	{
		BrowserDialog browser(_("Select directory with game data"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());

			// TODO: Verify the game can be found in the new directory... Best
			// done with optional specific gameid to pluginmgr detectgames?
			// FSList files = dir.listDir(FSNode::kListFilesOnly);

			_gamePathWidget->setLabel(dir.getPath());
			g_gui.scheduleTopDialogRedraw();
		}
		g_gui.scheduleTopDialogRedraw();
		break;
	}

	// Change path for extra game data (eg, using sword cutscenes when playing via CD)
	case kCmdExtraBrowser:
	{
		BrowserDialog browser(_("Select additional game directory"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_extraPathWidget->setLabel(dir.getPath());
			g_gui.scheduleTopDialogRedraw();
		}
		g_gui.scheduleTopDialogRedraw();
		break;
	}
	// Change path for stored save game (perm and temp) data
	case kCmdSaveBrowser:
	{
		BrowserDialog browser(_("Select directory for saved games"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_savePathWidget->setLabel(dir.getPath());
#if defined(USE_CLOUD) && defined(USE_LIBCURL)
			MessageDialog warningMessage(_("Saved games sync feature doesn't work with non-default directories. If you want your saved games to sync, use default directory."));
			warningMessage.runModal();
#endif
			g_gui.scheduleTopDialogRedraw();
		}
		g_gui.scheduleTopDialogRedraw();
		break;
	}

	case kCmdExtraPathClear:
		_extraPathWidget->setLabel(_c("None", "path"));
		break;

	case kCmdSavePathClear:
		_savePathWidget->setLabel(_("Default"));
		break;

	case kOKCmd:
	{
		// Write back changes made to config object
		String newDomain(Common::convertFromU32String(_domainWidget->getEditString()));
		if (newDomain != _domain) {
			if (newDomain.empty()
				|| newDomain.hasPrefix("_")
				|| newDomain == ConfigManager::kApplicationDomain
				|| ConfMan.hasGameDomain(newDomain)) {
				MessageDialog alert(_("This game ID is already taken. Please choose another one."));
				alert.runModal();
				return;
			}
			ConfMan.renameGameDomain(_domain, newDomain);
			_domain = newDomain;
			if (_engineOptions) {
				_engineOptions->setDomain(newDomain);
			}
		}
	}
	// fall through
	default:
		OptionsDialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
