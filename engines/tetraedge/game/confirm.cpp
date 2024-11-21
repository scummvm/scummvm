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

#include "tetraedge/game/confirm.h"
#include "tetraedge/game/application.h"
#include "tetraedge/tetraedge.h"

#include "tetraedge/te/te_text_layout.h"

namespace Tetraedge {

Confirm::Confirm() {
}

void Confirm::enter(const Common::Path &guiPath, const Common::String &y) {
	_gui.load(guiPath);
	TeLayout *backgroundLayout = _gui.layout("background");
	if (!backgroundLayout) {
		warning("confirm script not loaded, default to Yes.");
		onButtonYes();
		return;
	}
	backgroundLayout->setRatioMode(TeILayout::RATIO_MODE_NONE);

	Application *app = g_engine->getApplication();
	TeButtonLayout *confirmButtonLayout = _gui.buttonLayout("confirm");
	app->frontOrientationLayout().addChild(confirmButtonLayout);

	TeButtonLayout *yesButtonLayout = _gui.buttonLayout("yes");
	if (yesButtonLayout)
		yesButtonLayout->onMouseClickValidated().add(this, &Confirm::onButtonYes);

	TeButtonLayout *noButtonLayout = _gui.buttonLayout("no");
	if (noButtonLayout)
		noButtonLayout->onMouseClickValidated().add(this, &Confirm::onButtonNo);

	TeLayout *textLayout = _gui.layout("text");
	if (textLayout) {
		const Common::String textAttributs = _gui.value("textAttributs").toString();
		const Common::String textAttributsDown = _gui.value("textAttributsDown").toString();
		const Common::String *okButtonLoc = app->loc().value("okButton");
		const Common::String *cancelButtonLoc = app->loc().value("cancelButton");

		TeTextLayout *textTextLayout = dynamic_cast<TeTextLayout *>(textLayout->child(0));
		if (!textTextLayout)
			error("Expected text layout child.");
		const Common::String *textLayoutName = app->loc().value(textTextLayout->name());
		const char *fallbackText = "Do you really want to quit?";	// FIXME: Needed for Syberia II
		textTextLayout->setText(textAttributs + (textLayoutName ? *textLayoutName : fallbackText));

		if (!okButtonLoc || !cancelButtonLoc) {
			error("Missing translations for ok and cancel");
		}

		TeITextLayout *yesUpLayout = _gui.textLayout("yesUpLayout");
		if (yesUpLayout)
			yesUpLayout->setText(textAttributs + *okButtonLoc);

		TeITextLayout *yesDownLayout = _gui.textLayout("yesDownLayout");
		if (yesDownLayout)
			yesDownLayout->setText(textAttributsDown + *okButtonLoc);

		TeITextLayout *yesRollOverLayout = _gui.textLayout("yesRollOverLayout");
		if (yesRollOverLayout)
			yesRollOverLayout->setText(textAttributs + *okButtonLoc);

		TeITextLayout *noUpLayout = _gui.textLayout("noUpLayout");
		if (noUpLayout)
			noUpLayout->setText(textAttributs + *cancelButtonLoc);

		TeITextLayout *noDownLayout = _gui.textLayout("noDownLayout");
		if (noDownLayout)
			noDownLayout->setText(textAttributsDown + *cancelButtonLoc);

		TeITextLayout *noRollOverLayout = _gui.textLayout("noRollOverLayout");
		if (noRollOverLayout)
			noRollOverLayout->setText(textAttributs + *cancelButtonLoc);
	}

	// Make sure the mouse cursor is back on top.
	app->frontOrientationLayout().removeChild(&app->mouseCursorLayout());
	app->frontOrientationLayout().addChild(&app->mouseCursorLayout());

	if (ConfMan.getBool("skip_confirm")) {
		onButtonYes();
	}
}

void Confirm::leave() {
	Application *app = g_engine->getApplication();
	TeButtonLayout *confirmButtonLayout = _gui.buttonLayout("confirm");
	if (confirmButtonLayout) {
		app->frontLayout().removeChild(confirmButtonLayout);
	}
	_gui.unload();
}

bool Confirm::onButtonNo() {
	Application *app = g_engine->getApplication();
	app->captureFade();
	leave();
	_onButtonNoSignal.call();
	app->fade();
	return true;
}

bool Confirm::onButtonYes() {
	Application *app = g_engine->getApplication();
	app->captureFade();
	leave();
	_onButtonYesSignal.call();
	app->fade();
	return true;
}

} // end namespace Tetraedge
