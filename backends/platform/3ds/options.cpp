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

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "backends/platform/3ds/osystem.h"

#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/list.h"
#include "gui/widgets/popup.h"

#include "common/translation.h"

namespace N3DS {

class N3DSOptionsWidget : public GUI::OptionsContainerWidget {
public:
	explicit N3DSOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	~N3DSOptionsWidget() override;

	// OptionsContainerWidget API
	void load() override;
	bool save() override;
	bool hasKeys() override;
	void setEnabled(bool e) override;

private:
	// OptionsContainerWidget API
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;

	GUI::CheckboxWidget *_showCursorCheckbox;
	GUI::CheckboxWidget *_snapToBorderCheckbox;
	GUI::CheckboxWidget *_stretchToFitCheckbox;

	GUI::StaticTextWidget *_screenDesc;
	GUI::PopUpWidget *_screenPopUp;

	bool _enabled;
};

N3DSOptionsWidget::N3DSOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "N3DSOptionsDialog", domain), _enabled(true) {

	_showCursorCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "N3DSOptionsDialog.ShowCursor", _("Show mouse cursor"), Common::U32String(), 0, 'T');
	_snapToBorderCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "N3DSOptionsDialog.SnapToBorder", _("Snap to edges"), Common::U32String(), 0, 'T');
	_stretchToFitCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "N3DSOptionsDialog.StretchToFit", _("Stretch to fit"), Common::U32String(), 0, 'T');

	_screenDesc = new GUI::StaticTextWidget(widgetsBoss(), "N3DSOptionsDialog.ScreenText", _("Use Screen:"));
	_screenPopUp = new GUI::PopUpWidget(widgetsBoss(), "N3DSOptionsDialog.Screen");
	_screenPopUp->appendEntry(_c("Top", "3ds-screen"), kScreenTop);
	_screenPopUp->appendEntry(_c("Bottom", "3ds-screen"), kScreenBottom);
	_screenPopUp->appendEntry(_c("Both", "3ds-screen"), kScreenBoth);
}

N3DSOptionsWidget::~N3DSOptionsWidget() {
}

void N3DSOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
	    .addLayout(GUI::ThemeLayout::kLayoutVertical)
	        .addPadding(0, 0, 0, 0)
	        .addWidget("ShowCursor", "Checkbox")
	        .addWidget("SnapToBorder", "Checkbox")
	        .addWidget("StretchToFit", "Checkbox")
                .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
	            .addPadding(16, 16, 0, 0)
	            .addWidget("ScreenText", "OptionsLabel")
	            .addWidget("Screen", "PopUp")
	        .closeLayout()
	    .closeLayout()
	.closeDialog();
}

void N3DSOptionsWidget::load() {
	_showCursorCheckbox->setState(ConfMan.getBool("3ds_showcursor", _domain));
	_snapToBorderCheckbox->setState(ConfMan.getBool("3ds_snaptoborder", _domain));
	_stretchToFitCheckbox->setState(ConfMan.getBool("3ds_stretchtofit", _domain));
	_screenPopUp->setSelectedTag(ConfMan.getInt("3ds_screen", _domain));
}

bool N3DSOptionsWidget::save() {
	if (_enabled) {
		ConfMan.setBool("3ds_showcursor", _showCursorCheckbox->getState(), _domain);
		ConfMan.setBool("3ds_snaptoborder", _snapToBorderCheckbox->getState(), _domain);
		ConfMan.setBool("3ds_stretchtofit", _stretchToFitCheckbox->getState(), _domain);
		ConfMan.setInt("3ds_screen", _screenPopUp->getSelectedTag(), _domain);
	} else {
		ConfMan.removeKey("3ds_showcursor", _domain);
		ConfMan.removeKey("3ds_snaptoborder", _domain);
		ConfMan.removeKey("3ds_stretchtofit", _domain);
		ConfMan.removeKey("3ds_screen", _domain);
	}

	return true;
}

bool N3DSOptionsWidget::hasKeys() {
	return ConfMan.hasKey("3ds_showcursor", _domain) ||
	       ConfMan.hasKey("3ds_snaptoborder", _domain) ||
	       ConfMan.hasKey("3ds_stretchtofit", _domain) ||
	       ConfMan.hasKey("3ds_screen", _domain);
}

void N3DSOptionsWidget::setEnabled(bool e) {
	_enabled = e;

	_showCursorCheckbox->setEnabled(e);
	_snapToBorderCheckbox->setEnabled(e);
	_stretchToFitCheckbox->setEnabled(e);
	_screenDesc->setEnabled(e);
	_screenPopUp->setEnabled(e);
}


GUI::OptionsContainerWidget *OSystem_3DS::buildBackendOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new N3DSOptionsWidget(boss, name, target);
}

void OSystem_3DS::registerDefaultSettings(const Common::String &target) const {
	ConfMan.registerDefault("3ds_showcursor", true);
	ConfMan.registerDefault("3ds_snaptoborder", true);
	ConfMan.registerDefault("3ds_stretchtofit", false);
	ConfMan.registerDefault("3ds_screen", kScreenBoth);
}

void OSystem_3DS::applyBackendSettings() {
	int oldScreen = _screen;

	_showCursor = ConfMan.getBool("3ds_showcursor");
	_snapToBorder = ConfMan.getBool("3ds_snaptoborder");
	_stretchToFit = ConfMan.getBool("3ds_stretchtofit");
	_screen = (Screen)ConfMan.getInt("3ds_screen");

	updateBacklight();
	updateConfig();

	if (_screen != oldScreen) {
		_screenChangeId++;
		g_gui.checkScreenChange();
	}
}

} // namespace N3DS
