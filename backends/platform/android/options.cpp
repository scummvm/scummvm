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

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "backends/platform/android/android.h"
#include "backends/platform/android/jni-android.h"

#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

#include "common/translation.h"

class AndroidOptionsWidget final : public GUI::OptionsContainerWidget {
public:
	explicit AndroidOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	~AndroidOptionsWidget() override;

	// OptionsContainerWidget API
	void load() override;
	bool save() override;
	bool hasKeys() override;
	void setEnabled(bool e) override;

private:
	// OptionsContainerWidget API
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;

	GUI::CheckboxWidget *_onscreenCheckbox;
	GUI::StaticTextWidget *_preferredTouchModeDesc;
	GUI::StaticTextWidget *_preferredTMMenusDesc;
	GUI::PopUpWidget *_preferredTMMenusPopUp;
	GUI::StaticTextWidget *_preferredTM2DGamesDesc;
	GUI::PopUpWidget *_preferredTM2DGamesPopUp;
	GUI::StaticTextWidget *_preferredTM3DGamesDesc;
	GUI::PopUpWidget *_preferredTM3DGamesPopUp;
	GUI::CheckboxWidget *_onscreenSAFRevokeCheckbox;

	bool _enabled;


	uint32 loadTouchMode(const Common::String &setting, bool acceptDefault, uint32 defaultValue);
	void saveTouchMode(const Common::String &setting, uint32 touchMode);
};

enum {
	kTouchModeDefault = -1,
	kTouchModeTouchpad = 0,
	kTouchModeMouse,
	kTouchModeGamepad,
};

AndroidOptionsWidget::AndroidOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "AndroidOptionsDialog", false, domain), _enabled(true) {

	const bool inAppDomain = domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain);;

	_onscreenCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "AndroidOptionsDialog.OnScreenControl", _("Show On-screen control"));
	_preferredTouchModeDesc = new GUI::StaticTextWidget(widgetsBoss(), "AndroidOptionsDialog.PreferredTouchModeText", _("Choose the preferred touch mode:"));
	if (inAppDomain) {
		_preferredTMMenusDesc = new GUI::StaticTextWidget(widgetsBoss(), "AndroidOptionsDialog.TMMenusText", _("In menus"));
		_preferredTMMenusPopUp = new GUI::PopUpWidget(widgetsBoss(), "AndroidOptionsDialog.TMMenus");
		_preferredTMMenusPopUp->appendEntry(_("Touchpad emulation"), kTouchModeTouchpad);
		_preferredTMMenusPopUp->appendEntry(_("Direct mouse"), kTouchModeMouse); // TODO: Find a better name
		_preferredTMMenusPopUp->appendEntry(_("Gamepad emulation"), kTouchModeGamepad);
	} else {
		_preferredTMMenusDesc = nullptr;
		_preferredTMMenusPopUp = nullptr;
	}

	_preferredTM2DGamesDesc = new GUI::StaticTextWidget(widgetsBoss(), "AndroidOptionsDialog.TM2DGamesText", _("In 2D games"));
	_preferredTM2DGamesPopUp = new GUI::PopUpWidget(widgetsBoss(), "AndroidOptionsDialog.TM2DGames");
	_preferredTM3DGamesDesc = new GUI::StaticTextWidget(widgetsBoss(), "AndroidOptionsDialog.TM3DGamesText", _("In 3D games"));
	_preferredTM3DGamesPopUp = new GUI::PopUpWidget(widgetsBoss(), "AndroidOptionsDialog.TM3DGames");

	if (!inAppDomain) {
		_preferredTM2DGamesPopUp->appendEntry(_("<default>"), kTouchModeDefault);
		_preferredTM3DGamesPopUp->appendEntry(_("<default>"), kTouchModeDefault);
	}

	_preferredTM2DGamesPopUp->appendEntry(_("Touchpad emulation"), kTouchModeTouchpad);
	_preferredTM3DGamesPopUp->appendEntry(_("Touchpad emulation"), kTouchModeTouchpad);

	_preferredTM2DGamesPopUp->appendEntry(_("Direct mouse"), kTouchModeMouse); // TODO: Find a better name
	_preferredTM3DGamesPopUp->appendEntry(_("Direct mouse"), kTouchModeMouse);

	_preferredTM2DGamesPopUp->appendEntry(_("Gamepad emulation"), kTouchModeGamepad);
	_preferredTM3DGamesPopUp->appendEntry(_("Gamepad emulation"), kTouchModeGamepad);

	if (inAppDomain) {
		// Only show this checkbox in Options (via Options... in the launcher), and not at game domain level (via Edit Game...)
		// I18N: Show a button to revoke Storage Access Framework permissions for Android
		_onscreenSAFRevokeCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "AndroidOptionsDialog.SAFRevokePermsControl", _("Show SAF revoke permissions overlay button"));
	}
}

AndroidOptionsWidget::~AndroidOptionsWidget() {
}

void AndroidOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	const bool inAppDomain = _domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain);;

	layouts.addDialog(layoutName, overlayedLayout)
	        .addLayout(GUI::ThemeLayout::kLayoutVertical)
	            .addPadding(0, 0, 0, 0)
	            .addWidget("OnScreenControl", "Checkbox")
				.addWidget("PreferredTouchModeText", "", -1, layouts.getVar("Globals.Line.Height"));

	if (inAppDomain) {
		layouts.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
			.addPadding(0, 0, 0, 0)
			.addWidget("TMMenusText", "OptionsLabel")
			.addWidget("TMMenus", "PopUp")
		.closeLayout();
	}
	layouts.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
			.addPadding(0, 0, 0, 0)
			.addWidget("TM2DGamesText", "OptionsLabel")
			.addWidget("TM2DGames", "PopUp")
		.closeLayout()
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
			.addPadding(0, 0, 0, 0)
			.addWidget("TM3DGamesText", "OptionsLabel")
			.addWidget("TM3DGames", "PopUp")
		.closeLayout();
	if (inAppDomain) {
		layouts.addWidget("SAFRevokePermsControl", "Checkbox");
	}
	layouts.closeLayout()
	    .closeDialog();
}

uint32 AndroidOptionsWidget::loadTouchMode(const Common::String &setting, bool acceptDefault, uint32 defaultValue) {
	if (!acceptDefault || ConfMan.hasKey(setting, _domain)) {
		Common::String preferredTouchMode = ConfMan.get(setting, _domain);
		if (preferredTouchMode == "mouse") {
			return kTouchModeMouse;
		} else if (preferredTouchMode == "gamepad") {
			return kTouchModeGamepad;
		} else if (preferredTouchMode == "touchpad") {
			return kTouchModeTouchpad;
		} else {
			return defaultValue;
		}
	} else {
		return kTouchModeDefault;
	}
}

void AndroidOptionsWidget::load() {
	const bool inAppDomain = _domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain);

	_onscreenCheckbox->setState(ConfMan.getBool("onscreen_control", _domain));

	// When in application domain, we don't have default entry so we must have a value
	if (inAppDomain) {
		_preferredTMMenusPopUp->setSelectedTag(loadTouchMode("touch_mode_menus", !inAppDomain, kTouchModeMouse));
	}
	_preferredTM2DGamesPopUp->setSelectedTag(loadTouchMode("touch_mode_2d_games", !inAppDomain, kTouchModeTouchpad));
	_preferredTM3DGamesPopUp->setSelectedTag(loadTouchMode("touch_mode_3d_games", !inAppDomain, kTouchModeGamepad));

	if (inAppDomain) {
		_onscreenSAFRevokeCheckbox->setState(ConfMan.getBool("onscreen_saf_revoke_btn", _domain));
	}
}

void AndroidOptionsWidget::saveTouchMode(const Common::String &setting, uint32 touchMode) {
	switch (touchMode) {
	case kTouchModeTouchpad:
		ConfMan.set(setting, "touchpad", _domain);
		break;
	case kTouchModeMouse:
		ConfMan.set(setting, "mouse", _domain);
		break;
	case kTouchModeGamepad:
		ConfMan.set(setting, "gamepad", _domain);
		break;
	default:
		// default
		ConfMan.removeKey(setting, _domain);
		break;
	}
}

bool AndroidOptionsWidget::save() {
	const bool inAppDomain = _domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain);

	if (_enabled) {
		ConfMan.setBool("onscreen_control", _onscreenCheckbox->getState(), _domain);

		if (inAppDomain) {
			saveTouchMode("touch_mode_menus", _preferredTMMenusPopUp->getSelectedTag());
		}
		saveTouchMode("touch_mode_2d_games", _preferredTM2DGamesPopUp->getSelectedTag());
		saveTouchMode("touch_mode_3d_games", _preferredTM3DGamesPopUp->getSelectedTag());

		if (inAppDomain) {
			ConfMan.setBool("onscreen_saf_revoke_btn", _onscreenSAFRevokeCheckbox->getState(), _domain);
		}
	} else {
		ConfMan.removeKey("onscreen_control", _domain);

		if (inAppDomain) {
			ConfMan.removeKey("touch_mode_menus", _domain);
		}
		ConfMan.removeKey("touch_mode_2d_games", _domain);
		ConfMan.removeKey("touch_mode_3d_games", _domain);

		if (inAppDomain) {
			ConfMan.removeKey("onscreen_saf_revoke_btn", _domain);
		}
	}

	return true;
}

bool AndroidOptionsWidget::hasKeys() {
	return ConfMan.hasKey("onscreen_control", _domain) ||
	       (_domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain) && ConfMan.hasKey("touch_mode_menus", _domain)) ||
	       ConfMan.hasKey("touch_mode_2d_games", _domain) ||
	       ConfMan.hasKey("touch_mode_3d_games", _domain) ||
	       (_domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain) && ConfMan.hasKey("onscreen_saf_revoke_btn", _domain));
}

void AndroidOptionsWidget::setEnabled(bool e) {
	const bool inAppDomain = _domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain);

	_enabled = e;

	_onscreenCheckbox->setEnabled(e);

	if (inAppDomain) {
		_preferredTMMenusDesc->setEnabled(e);
		_preferredTMMenusPopUp->setEnabled(e);
	}
	_preferredTM2DGamesDesc->setEnabled(e);
	_preferredTM2DGamesPopUp->setEnabled(e);
	_preferredTM3DGamesDesc->setEnabled(e);
	_preferredTM3DGamesPopUp->setEnabled(e);

	if (inAppDomain) {
		_onscreenSAFRevokeCheckbox->setEnabled(e);
	}
}


GUI::OptionsContainerWidget *OSystem_Android::buildBackendOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new AndroidOptionsWidget(boss, name, target);
}

void OSystem_Android::registerDefaultSettings(const Common::String &target) const {
	ConfMan.registerDefault("onscreen_control", true);
	ConfMan.registerDefault("touch_mode_menus", "mouse");
	ConfMan.registerDefault("touch_mode_2d_games", "touchpad");
	ConfMan.registerDefault("touch_mode_3d_games", "gamepad");
	ConfMan.registerDefault("onscreen_saf_revoke_btn", false);
}

void OSystem_Android::applyTouchSettings(bool _3dMode, bool overlayShown) {
	Common::String setting;
	int defaultMode;

	if (overlayShown) {
		setting = "touch_mode_menus";
		defaultMode = TOUCH_MODE_MOUSE;
	} else if (_3dMode) {
		setting = "touch_mode_3d_games";
		defaultMode = TOUCH_MODE_GAMEPAD;
	} else {
		setting = "touch_mode_2d_games";
		defaultMode = TOUCH_MODE_TOUCHPAD;
	}

	Common::String preferredTouchMode = ConfMan.get(setting);
	if (preferredTouchMode == "mouse") {
		JNI::setTouchMode(TOUCH_MODE_MOUSE);
	} else if (preferredTouchMode == "gamepad") {
		JNI::setTouchMode(TOUCH_MODE_GAMEPAD);
	} else if (preferredTouchMode == "touchpad") {
		JNI::setTouchMode(TOUCH_MODE_TOUCHPAD);
	} else {
		JNI::setTouchMode(defaultMode);
	}
}

void OSystem_Android::applyBackendSettings() {
	JNI::showKeyboardControl(ConfMan.getBool("onscreen_control"));
	JNI::showSAFRevokePermsControl(ConfMan.getBool("onscreen_saf_revoke_btn"));
}
