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


#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"

#include "common/translation.h"

enum {
	kHelpCmd = 'Help',
};

class IOS7OptionsWidget final : public GUI::OptionsContainerWidget {
public:
	explicit IOS7OptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	~IOS7OptionsWidget() override;

	// OptionsContainerWidget API
	void load() override;
	bool save() override;
	bool hasKeys() override;
	void setEnabled(bool e) override;

private:
	// OptionsContainerWidget API
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

	GUI::CheckboxWidget *_onscreenCheckbox;
	GUI::CheckboxWidget *_touchpadCheckbox;
	GUI::CheckboxWidget *_clickAndDragdCheckbox;

	bool _enabled;

	uint32 loadTouchMode(const Common::String &setting, bool acceptDefault, uint32 defaultValue);
	void saveTouchMode(const Common::String &setting, uint32 touchMode);
};

IOS7OptionsWidget::IOS7OptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "IOS7OptionsDialog", false, domain), _enabled(true) {

	const bool inAppDomain = domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain);

	_onscreenCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "IOS7OptionsDialog.OnScreenControl", _("Show On-screen control"));
	_touchpadCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "IOS7OptionsDialog.TouchpadMouseMode", _("Touchpad mouse mode"));
	_clickAndDragdCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "IOS7OptionsDialog.ClickAndDragMode", _("Mouse-click-and-drag mode"));

	new GUI::ButtonWidget(widgetsBoss(), "IOS7OptionsDialog.ControlsHelp", _("Controls Help"), Common::U32String(), kHelpCmd);
}

IOS7OptionsWidget::~IOS7OptionsWidget() {
}

void IOS7OptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	const bool inAppDomain = _domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain);;

	layouts.addDialog(layoutName, overlayedLayout)
	        .addLayout(GUI::ThemeLayout::kLayoutVertical)
	            .addPadding(0, 0, 0, 0)
	            .addWidget("OnScreenControl", "Checkbox")
	            .addWidget("TouchpadMouseMode", "Checkbox")
	            .addWidget("ClickAndDragMode", "Checkbox")
	            .addPadding(0, 0, 0, 0)
	            .addWidget("ControlsHelp", "WideButton");

	layouts.closeLayout()
	    .closeDialog();
}

void IOS7OptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kHelpCmd: {
		GUI::MessageDialog help(_(
"Gestures and controls:

One finger tap: Left mouse click
Two finger tap: Right mouse click
Two finger double tap: ESC
Two finger swipe (bottom to top): Toggles Click and drag mode
Two finger swipe (left to right): Toggles between touch direct mode and touchpad mode
Two finger swipe (top to bottom): Global Main Menu
Three finger swipe: Arrow keys
Pinch gesture: Enables/disables keyboard
Keyboard spacebar: Pause"));

		help.runModal();
		break;
	}
	default:
		GUI::OptionsContainerWidget::handleCommand(sender, cmd, data);
	}
}

void IOS7OptionsWidget::load() {
	_onscreenCheckbox->setState(ConfMan.getBool("onscreen_control", _domain));
	_touchpadCheckbox->setState(ConfMan.getBool("touchpad_mode", _domain));
	_clickAndDragdCheckbox->setState(ConfMan.getBool("clickanddrag_mode", _domain));
}

bool IOS7OptionsWidget::save() {
	if (_enabled) {
		ConfMan.setBool("onscreen_control", _onscreenCheckbox->getState(), _domain);
		ConfMan.setBool("touchpad_mode", _touchpadCheckbox->getState(), _domain);
		ConfMan.setBool("clickanddrag_mode", _clickAndDragdCheckbox->getState(), _domain);
	} else {
		ConfMan.removeKey("onscreen_control", _domain);
		ConfMan.removeKey("touchpad_mode", _domain);
		ConfMan.removeKey("clickanddrag_mode", _domain);
	}

	return true;
}

bool IOS7OptionsWidget::hasKeys() {
	return ConfMan.hasKey("onscreen_control", _domain) ||
	       ConfMan.hasKey("touchpad_mode", _domain) ||
	       ConfMan.hasKey("clickanddrag_mode", _domain));
}

void IOS7OptionsWidget::setEnabled(bool e) {
	_enabled = e;

	_onscreenCheckbox->setEnabled(e);
	_touchpadCheckbox->setEnabled(e);
	_clickAndDragdCheckbox->setEnabled(e);
}


GUI::OptionsContainerWidget *OSystem_iOS7::buildBackendOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new IOS7OptionsWidget(boss, name, target);
}

void OSystem_iOS7::registerDefaultSettings(const Common::String &target) const {
	ConfMan.registerDefault("onscreen_control", true);
	ConfMan.registerDefault("touchpad_mode", true);
	ConfMan.registerDefault("clickanddrag_mode", false);
}
