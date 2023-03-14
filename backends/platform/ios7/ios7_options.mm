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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"

#include "common/translation.h"
#include "backends/platform/ios7/ios7_osys_main.h"

#include <TargetConditionals.h>

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
};

IOS7OptionsWidget::IOS7OptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "IOS7OptionsDialog", false, domain), _enabled(true) {

	const bool inAppDomain = domain.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain);

	_onscreenCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "IOS7OptionsDialog.OnScreenControl", _("Show On-screen control (iOS 15 and later)"));
	_touchpadCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "IOS7OptionsDialog.TouchpadMouseMode", _("Touchpad mouse mode"));
	_clickAndDragdCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "IOS7OptionsDialog.ClickAndDragMode", _("Mouse-click-and-drag mode"));

	new GUI::ButtonWidget(widgetsBoss(), "IOS7OptionsDialog.ControlsHelp", _("Controls Help"), Common::U32String(), kHelpCmd);

	// setEnabled is normally only called from the EditGameDialog, but some options (OnScreenControl)
	// should be disabled in all domains if system is running a lower version of iOS than 15.0.
	setEnabled(_enabled);
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
		GUI::MessageDialog help(
#if TARGET_OS_IOS
			_("Gestures and controls:\n"
			  "\n"
			  "One finger tap: Left mouse click\n"
			  "Two finger tap: Right mouse click\n"
			  "Two finger double tap: ESC\n"
			  "Two finger swipe (bottom to top): Toggles Click and drag mode\n"
			  "Two finger swipe (left to right): Toggles between touch direct mode and touchpad mode\n"
			  "Two finger swipe (right to left): Shows/hides on-screen controls\n"
			  "Two finger swipe (top to bottom): Global Main Menu\n"
			  "Three finger swipe: Arrow keys\n"
			  "Pinch gesture: Enables/disables keyboard\n"
			  "Keyboard spacebar: Pause"),
#else // TVOS
			_("Using the Apple TV remote control:\n"
			  "\n"
			  "Press Touch area: Left mouse click\n"
			  "Press Play/Pause button: Right mouse click\n"
			  "Press Back/Menu button in game: Global Main menu\n"
			  "Press Back/Menu button in launcher: Apple TV Home\n"
			  "Press and hold Play/Pause button: Show keyboard with extra keys\n"
			  "Touch (not press) on top of Touch area: Up arrow key\n"
			  "Touch (not press) on left of Touch area: Left arrow key\n"
			  "Touch (not press) on right of Touch area: Right arrow key\n"
			  "Touch (not press) on bottom of Touch area: Down arrow key\n"
			  "Keyboard spacebar: Pause"),
#endif
			Common::U32String(_("Close")), Common::U32String(), Graphics::kTextAlignLeft);

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
	       ConfMan.hasKey("clickanddrag_mode", _domain);
}

void IOS7OptionsWidget::setEnabled(bool e) {
	_enabled = e;

#if TARGET_OS_IOS && defined (__IPHONE_15_0)
	// On-screen controls (virtual controller is supported in iOS 15 and later)
	if (@available(iOS 15.0, *)) {
		_onscreenCheckbox->setEnabled(e);
	} else {
		_onscreenCheckbox->setEnabled(false);
	}
#else
	_onscreenCheckbox->setEnabled(false);
#endif
	_touchpadCheckbox->setEnabled(e);
	_clickAndDragdCheckbox->setEnabled(e);
}

GUI::OptionsContainerWidget *OSystem_iOS7::buildBackendOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new IOS7OptionsWidget(boss, name, target);
}

void OSystem_iOS7::registerDefaultSettings(const Common::String &target) const {
	ConfMan.registerDefault("onscreen_control", false);
	ConfMan.registerDefault("touchpad_mode", !iOS7_isBigDevice());
	ConfMan.registerDefault("clickanddrag_mode", false);
}

void OSystem_iOS7::applyBackendSettings() {
	virtualController(ConfMan.getBool("onscreen_control"));
	_touchpadModeEnabled = ConfMan.getBool("touchpad_mode");
	_mouseClickAndDragEnabled = ConfMan.getBool("clickanddrag_mode");
}
