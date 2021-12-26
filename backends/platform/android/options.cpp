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

#include "gui/ThemeEval.h"
#include "gui/widget.h"

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
	GUI::CheckboxWidget *_touchpadCheckbox;
	GUI::CheckboxWidget *_onscreenSAFRevokeCheckbox;

	bool _enabled;
};

AndroidOptionsWidget::AndroidOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "AndroidOptionsDialog", false, domain), _enabled(true) {
	_onscreenCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "AndroidOptionsDialog.OnScreenControl", _("Show On-screen control"));
	_touchpadCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "AndroidOptionsDialog.TouchpadMode", _("Touchpad mouse mode"));
	if (domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain)) {
		// Only show this checkbox in Options (via Options... in the launcher), and not at game domain level (via Edit Game...)
		// I18N: Show a button to revoke Storage Access Framework permissions for Android
		_onscreenSAFRevokeCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "AndroidOptionsDialog.SAFRevokePermsControl", _("Show SAF revoke permissions overlay button"));
	}
}

AndroidOptionsWidget::~AndroidOptionsWidget() {
}

void AndroidOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
	        .addLayout(GUI::ThemeLayout::kLayoutVertical)
	            .addPadding(0, 0, 0, 0)
	            .addWidget("OnScreenControl", "Checkbox")
	            .addWidget("TouchpadMode", "Checkbox");
	if (_domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain)) {
		layouts.addWidget("SAFRevokePermsControl", "Checkbox");
	}
	layouts.closeLayout()
	    .closeDialog();
}

void AndroidOptionsWidget::load() {
	_onscreenCheckbox->setState(ConfMan.getBool("onscreen_control", _domain));
	_touchpadCheckbox->setState(ConfMan.getBool("touchpad_mouse_mode", _domain));
	if (_domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain)) {
		_onscreenSAFRevokeCheckbox->setState(ConfMan.getBool("onscreen_saf_revoke_btn", _domain));
	}
}

bool AndroidOptionsWidget::save() {
	if (_enabled) {
		ConfMan.setBool("onscreen_control", _onscreenCheckbox->getState(), _domain);
		ConfMan.setBool("touchpad_mouse_mode", _touchpadCheckbox->getState(), _domain);
		if (_domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain)) {
			ConfMan.setBool("onscreen_saf_revoke_btn", _onscreenSAFRevokeCheckbox->getState(), _domain);
		}
	} else {
		ConfMan.removeKey("onscreen_control", _domain);
		ConfMan.removeKey("touchpad_mouse_mode", _domain);
		if (_domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain)) {
			ConfMan.removeKey("onscreen_saf_revoke_btn", _domain);
		}
	}

	return true;
}

bool AndroidOptionsWidget::hasKeys() {
	return ConfMan.hasKey("onscreen_control", _domain) ||
	       ConfMan.hasKey("touchpad_mouse_mode", _domain) ||
	       (_domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain) && ConfMan.hasKey("onscreen_saf_revoke_btn", _domain));
}

void AndroidOptionsWidget::setEnabled(bool e) {
	_enabled = e;

	_onscreenCheckbox->setEnabled(e);
	_touchpadCheckbox->setEnabled(e);
	if (_domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain)) {
		_onscreenSAFRevokeCheckbox->setEnabled(e);
	}
}


GUI::OptionsContainerWidget *OSystem_Android::buildBackendOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new AndroidOptionsWidget(boss, name, target);
}

void OSystem_Android::registerDefaultSettings(const Common::String &target) const {
	ConfMan.registerDefault("onscreen_control", true);
	ConfMan.registerDefault("touchpad_mouse_mode", true);
	ConfMan.registerDefault("onscreen_saf_revoke_btn", false);
}

void OSystem_Android::applyBackendSettings() {
	JNI::showKeyboardControl(ConfMan.getBool("onscreen_control"));
	JNI::showSAFRevokePermsControl(ConfMan.getBool("onscreen_saf_revoke_btn"));
	_touchpad_mode = ConfMan.getBool("touchpad_mouse_mode");
}
