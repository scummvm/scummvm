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

#include "base/version.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/str.h"
#include "common/system.h"
#include "common/translation.h"

#include "gui/about.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/options.h"
#include "gui/saveload.h"
#include "gui/ThemeEngine.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/tab.h"

#include "graphics/font.h"

#include "engines/dialogs.h"
#include "engines/engine.h"
#include "engines/metaengine.h"

#ifdef GUI_ENABLE_KEYSDIALOG
#include "gui/KeysDialog.h"
#endif

MainMenuDialog::MainMenuDialog(Engine *engine)
	: GUI::Dialog("GlobalMenu"), _engine(engine) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;

#ifndef DISABLE_FANCY_THEMES
	_logo = 0;
	if (g_gui.xmlEval()->getVar("Globals.ShowGlobalMenuLogo", 0) == 1 && g_gui.theme()->supportsImages()) {
		_logo = new GUI::GraphicsWidget(this, "GlobalMenu.Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfx(g_gui.theme()->getImageSurface(GUI::ThemeEngine::kImageLogoSmall));
	} else {
		GUI::StaticTextWidget *title = new GUI::StaticTextWidget(this, "GlobalMenu.Title", Common::U32String("ScummVM"));
		title->setAlign(Graphics::kTextAlignCenter);
	}
#else
	GUI::StaticTextWidget *title = new GUI::StaticTextWidget(this, "GlobalMenu.Title", Common::U32String("ScummVM"));
	title->setAlign(Graphics::kTextAlignCenter);
#endif

	GUI::StaticTextWidget *version = new GUI::StaticTextWidget(this, "GlobalMenu.Version", Common::U32String(gScummVMVersionDate));
	version->setAlign(Graphics::kTextAlignCenter);

	new GUI::ButtonWidget(this, "GlobalMenu.Resume", _("~R~esume"), Common::U32String(), kPlayCmd, 'P');

	_loadButton = new GUI::ButtonWidget(this, "GlobalMenu.Load", _("~L~oad"), Common::U32String(), kLoadCmd);
	_loadButton->setVisible(_engine->hasFeature(Engine::kSupportsLoadingDuringRuntime));
	_loadButton->setEnabled(_engine->hasFeature(Engine::kSupportsLoadingDuringRuntime));

	_saveButton = new GUI::ButtonWidget(this, "GlobalMenu.Save", _("~S~ave"), Common::U32String(), kSaveCmd);
	_saveButton->setVisible(_engine->hasFeature(Engine::kSupportsSavingDuringRuntime));
	_saveButton->setEnabled(_engine->hasFeature(Engine::kSupportsSavingDuringRuntime));

	new GUI::ButtonWidget(this, "GlobalMenu.Options", _("~O~ptions"), Common::U32String(), kOptionsCmd);

	// The help button is disabled by default.
	// To enable "Help", an engine needs to use a subclass of MainMenuDialog
	// (at least for now, we might change how this works in the future).
	_helpButton = new GUI::ButtonWidget(this, "GlobalMenu.Help", _("~H~elp"), Common::U32String(), kHelpCmd);

	new GUI::ButtonWidget(this, "GlobalMenu.About", _("~A~bout"), Common::U32String(), kAboutCmd);

	if (g_system->getOverlayWidth() > 320)
		_returnToLauncherButton = new GUI::ButtonWidget(this, "GlobalMenu.ReturnToLauncher", _("~R~eturn to Launcher"), Common::U32String(), kLauncherCmd);
	else
		_returnToLauncherButton = new GUI::ButtonWidget(this, "GlobalMenu.ReturnToLauncher", _c("~R~eturn to Launcher", "lowres"), Common::U32String(), kLauncherCmd);
	_returnToLauncherButton->setEnabled(_engine->hasFeature(Engine::kSupportsReturnToLauncher));

	if (!g_system->hasFeature(OSystem::kFeatureNoQuit) && !(ConfMan.getBool("gui_return_to_launcher_at_exit")))
		new GUI::ButtonWidget(this, "GlobalMenu.Quit", _("~Q~uit"), Common::U32String(), kQuitCmd);

	_aboutDialog = new GUI::AboutDialog();
	_loadDialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"), false);
	_saveDialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
}

MainMenuDialog::~MainMenuDialog() {
	delete _aboutDialog;
	delete _loadDialog;
	delete _saveDialog;
}

void MainMenuDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kPlayCmd:
		close();
		break;
	case kLoadCmd:
		load();
		break;
	case kSaveCmd:
		save();
		break;
	case kOptionsCmd: {
		GUI::ConfigDialog configDialog;
		configDialog.runModal();
		break;
	}
	case kAboutCmd:
		_aboutDialog->runModal();
		break;
	case kHelpCmd: {
		GUI::MessageDialog dialog(
					_("Sorry, this engine does not currently provide in-game help. "
					"Please consult the README for basic information, and for "
					"instructions on how to obtain further assistance."));
		dialog.runModal();
		}
		break;
	case kLauncherCmd: {
		Common::Event eventReturnToLauncher;
		eventReturnToLauncher.type = Common::EVENT_RETURN_TO_LAUNCHER;
		g_system->getEventManager()->pushEvent(eventReturnToLauncher);
		close();
		}
		break;
	case kQuitCmd: {
		Common::Event eventQ;
		eventQ.type = Common::EVENT_QUIT;
		g_system->getEventManager()->pushEvent(eventQ);
		close();
		}
		break;
	default:
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

void MainMenuDialog::reflowLayout() {
	if (_engine->hasFeature(Engine::kSupportsLoadingDuringRuntime))
		_loadButton->setEnabled(_engine->canLoadGameStateCurrently());
	if (_engine->hasFeature(Engine::kSupportsSavingDuringRuntime))
		_saveButton->setEnabled(_engine->canSaveGameStateCurrently());

	// Overlay size might have changed since the construction of the dialog.
	// Update labels when it might be needed
	// FIXME: it might be better to declare GUI::StaticTextWidget::setLabel() virtual
	// and to reimplement it in GUI::ButtonWidget to handle the hotkey.
	if (g_system->getOverlayWidth() > 320)
		_returnToLauncherButton->setLabel(_returnToLauncherButton->cleanupHotkey(_("~R~eturn to Launcher")));
	else
		_returnToLauncherButton->setLabel(_returnToLauncherButton->cleanupHotkey(_c("~R~eturn to Launcher", "lowres")));

#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowGlobalMenuLogo", 0) == 1 && g_gui.theme()->supportsImages()) {
		if (!_logo)
			_logo = new GUI::GraphicsWidget(this, "GlobalMenu.Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfx(g_gui.theme()->getImageSurface(GUI::ThemeEngine::kImageLogoSmall));

		GUI::StaticTextWidget *title = (GUI::StaticTextWidget *)findWidget("GlobalMenu.Title");
		if (title) {
			removeWidget(title);
			title->setNext(0);
			delete title;
		}
	} else {
		GUI::StaticTextWidget *title = (GUI::StaticTextWidget *)findWidget("GlobalMenu.Title");
		if (!title) {
			title = new GUI::StaticTextWidget(this, "GlobalMenu.Title", Common::U32String("ScummVM"));
			title->setAlign(Graphics::kTextAlignCenter);
		}

		if (_logo) {
			removeWidget(_logo);
			_logo->setNext(0);
			delete _logo;
			_logo = 0;
		}
	}
#endif

	Dialog::reflowLayout();
}

void MainMenuDialog::save() {
	int slot = _saveDialog->runModalWithCurrentTarget();

	if (slot >= 0) {
		Common::String result(_saveDialog->getResultString());
		if (result.empty()) {
			// If the user was lazy and entered no save name, come up with a default name.
			result = _saveDialog->createDefaultSaveDescription(slot);
		}

		Common::Error status = _engine->saveGameState(slot, result);
		if (status.getCode() != Common::kNoError) {
			Common::U32String failMessage = Common::U32String::format(_("Failed to save game (%s)! "
				  "Please consult the README for basic information, and for "
				  "instructions on how to obtain further assistance."), status.getDesc().c_str());
			GUI::MessageDialog dialog(failMessage);
			dialog.runModal();
		}

		close();
	}
}

void MainMenuDialog::load() {
	int slot = _loadDialog->runModalWithCurrentTarget();

	_engine->setGameToLoadSlot(slot);

	if (slot >= 0)
		close();
}

#ifdef GUI_ENABLE_KEYSDIALOG
enum {
	kKeysCmd = 'KEYS'
};
#endif

namespace GUI {

// FIXME: We use the empty string as domain name here. This tells the
// ConfigManager to use the 'default' domain for all its actions. We do that
// to get as close as possible to editing the 'active' settings.
//
// However, that requires bad & evil hacks in the ConfigManager code,
// and even then still doesn't work quite correctly.
// For example, if the transient domain contains 'false' for the 'fullscreen'
// flag, but the user used a hotkey to switch to windowed mode, then the dialog
// will display the wrong value anyway.
//
// Proposed solution consisting of multiple steps:
// 1) Add special code to the open() code that reads out everything stored
//    in the transient domain that is controlled by this dialog, and updates
//    the dialog accordingly.
// 2) Even more code is added to query the backend for current settings, like
//    the fullscreen mode flag etc., and also updates the dialog accordingly.
// 3) The domain being edited is set to the active game domain.
// 4) If the dialog is closed with the "OK" button, then we remove everything
//    stored in the transient domain (or at least everything corresponding to
//    switches in this dialog.
//    If OTOH the dialog is closed with "Cancel" we do no such thing.
//
// These changes will achieve two things at once: Allow us to get rid of using
//  "" as value for the domain, and in fact provide a somewhat better user
// experience at the same time.
ConfigDialog::ConfigDialog() :
		GUI::OptionsDialog("", "GlobalConfig"),
		_engineOptions(nullptr) {
	assert(g_engine);

	const Common::String &gameDomain = ConfMan.getActiveDomainName();
	const MetaEngine &metaEngine = g_engine->getMetaEngine();

	// GUI:  Add tab widget
	GUI::TabWidget *tab = new GUI::TabWidget(this, "GlobalConfig.TabWidget");

	//
	// The game specific options tab
	//

	int tabId = tab->addTab(_("Game"), "GlobalConfig_Engine");

	if (g_engine->hasFeature(Engine::kSupportsChangingOptionsDuringRuntime)) {
		_engineOptions = metaEngine.buildEngineOptionsWidgetDynamic(tab, "GlobalConfig_Engine.Container", gameDomain);
	}

	if (_engineOptions) {
		_engineOptions->setParentDialog(this);
	} else {
		tab->removeTab(tabId);
	}

	//
	// The Audio / Subtitles tab
	//

	tab->addTab(_("Audio"), "GlobalConfig_Audio");

	//
	// Sound controllers
	//

	addVolumeControls(tab, "GlobalConfig_Audio.");
	setVolumeSettingsState(true); // could disable controls by GUI options

	//
	// Subtitle speed and toggle controllers
	//

	if (g_engine->hasFeature(Engine::kSupportsSubtitleOptions)) {
		// Global talkspeed range of 0-255
		addSubtitleControls(tab, "GlobalConfig_Audio.", 255);
		setSubtitleSettingsState(true); // could disable controls by GUI options
	}

	//
	// The Keymap tab
	//

	Common::KeymapArray keymaps = metaEngine.initKeymaps(gameDomain.c_str());
	if (!keymaps.empty()) {
		tab->addTab(_("Keymaps"), "GlobalConfig_KeyMapper");
		addKeyMapperControls(tab, "GlobalConfig_KeyMapper.", keymaps, gameDomain);
	}

	//
	// The backend tab (shown only if the backend implements one)
	//
	int backendTabId = tab->addTab(_("Backend"), "GlobalConfig_Backend");

	_backendOptions = g_system->buildBackendOptionsWidget(tab, "GlobalConfig_Backend.Container", _domain);

	if (_backendOptions) {
		_backendOptions->setParentDialog(this);
	} else {
		tab->removeTab(backendTabId);
	}

	//
	// The Achievements tab
	//
	Common::AchievementsInfo achievementsInfo = metaEngine.getAchievementsInfo(gameDomain);
	if (achievementsInfo.descriptions.size() > 0) {
		tab->addTab(_("Achievements"), "GlobalConfig_Achievements");
		addAchievementsControls(tab, "GlobalConfig_Achievements.", achievementsInfo);
	}

	// Activate the first tab
	tab->setActiveTab(0);

	//
	// Add the buttons
	//

	new GUI::ButtonWidget(this, "GlobalConfig.Ok", _("~O~K"), Common::U32String(), GUI::kOKCmd);
	new GUI::ButtonWidget(this, "GlobalConfig.Cancel", _("~C~ancel"), Common::U32String(), GUI::kCloseCmd);

#ifdef GUI_ENABLE_KEYSDIALOG
	new GUI::ButtonWidget(this, "GlobalConfig.Keys", _("~K~eys"), Common::U32String(), kKeysCmd);
	_keysDialog = NULL;
#endif
}

ConfigDialog::~ConfigDialog() {
#ifdef GUI_ENABLE_KEYSDIALOG
	delete _keysDialog;
#endif
}

void ConfigDialog::build() {
	OptionsDialog::build();

	// Engine options
	if (_engineOptions) {
		_engineOptions->load();
	}
}

void ConfigDialog::apply() {
	if (_engineOptions) {
		_engineOptions->save();
	}

	OptionsDialog::apply();
}

#ifdef GUI_ENABLE_KEYSDIALOG
void ConfigDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kKeysCmd:
		//
		// Create the sub dialog(s)
		//
		_keysDialog = new GUI::KeysDialog();
		_keysDialog->runModal();
		delete _keysDialog;
		_keysDialog = NULL;
		break;
	default:
		GUI::OptionsDialog::handleCommand (sender, cmd, data);
	}
}
#endif

ExtraGuiOptionsWidget::ExtraGuiOptionsWidget(GuiObject *containerBoss, const Common::String &name, const Common::String &domain, const ExtraGuiOptions &options) :
		OptionsContainerWidget(containerBoss, name, dialogLayout(domain), false, domain),
		_options(options) {

	// Note: up to 7 engine options can currently fit on screen (the most that
	// can fit in a 320x200 screen with the classic theme).
	// TODO: Increase this number by including the checkboxes inside a scroll
	// widget. The appropriate number of checkboxes will need to be added to
	// the theme files.

	uint i = 1;
	ExtraGuiOptions::const_iterator iter;
	for (iter = _options.begin(); iter != _options.end(); ++iter, ++i) {
		Common::String id = Common::String::format("%d", i);
		_checkboxes.push_back(new CheckboxWidget(widgetsBoss(),
			_dialogLayout + ".customOption" + id + "Checkbox", _(iter->label), _(iter->tooltip)));
	}
}

ExtraGuiOptionsWidget::~ExtraGuiOptionsWidget() {
}

Common::String ExtraGuiOptionsWidget::dialogLayout(const Common::String &domain) {
	if (ConfMan.getActiveDomainName().equals(domain)) {
		return "GlobalConfig_Engine_Container";
	} else {
		return "GameOptions_Engine_Container";
	}
}

void ExtraGuiOptionsWidget::load() {
	// Set the state of engine-specific checkboxes
	for (uint j = 0; j < _options.size(); ++j) {
		// The default values for engine-specific checkboxes are not set when
		// ScummVM starts, as this would require us to load and poll all of the
		// engine plugins on startup. Thus, we set the state of each custom
		// option checkbox to what is specified by the engine plugin, and
		// update it only if a value has been set in the configuration of the
		// currently selected game.
		bool isChecked = _options[j].defaultState;
		if (ConfMan.hasKey(_options[j].configOption, _domain))
			isChecked = ConfMan.getBool(_options[j].configOption, _domain);
		_checkboxes[j]->setState(isChecked);
	}
}

bool ExtraGuiOptionsWidget::save() {
	// Set the state of engine-specific checkboxes
	for (uint i = 0; i < _options.size(); i++) {
		ConfMan.setBool(_options[i].configOption, _checkboxes[i]->getState(), _domain);
	}

	return true;
}

} // End of namespace GUI
