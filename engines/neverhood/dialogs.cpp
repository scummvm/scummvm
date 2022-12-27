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

#include "neverhood/neverhood.h"
#include "neverhood/dialogs.h"

#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/saveload.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

#include "common/gui_options.h"
#include "common/system.h"
#include "common/translation.h"

namespace Neverhood {

NeverhoodOptionsWidget::NeverhoodOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "NeverhoodGameOptionsDialog", false, domain),
		_originalSaveLoadCheckbox(nullptr),
		_skipHallOfRecordsCheckbox(nullptr),
		_scaleMakingOfVideosCheckbox(nullptr) {

	_originalSaveLoadCheckbox = new GUI::CheckboxWidget(
		widgetsBoss(),
		"NeverhoodGameOptionsDialog.OriginalSaveLoad",
		_("Use original save/load screens"),
		_("Use the original save/load screens instead of the ScummVM ones"));

	_skipHallOfRecordsCheckbox = new GUI::CheckboxWidget(
		widgetsBoss(),
		"NeverhoodGameOptionsDialog.SkipHallOfRecords", _("Skip the Hall of Records storyboard scenes"),
		_("Allows the player to skip past the Hall of Records storyboard scenes"));

	_scaleMakingOfVideosCheckbox = new GUI::CheckboxWidget(
		widgetsBoss(),
		"NeverhoodGameOptionsDialog.ScaleMakingOfVideos", _("Scale the making of videos to full screen"),
		_("Scale the making of videos, so that they use the whole screen"));
}

NeverhoodOptionsWidget::~NeverhoodOptionsWidget() {
}

void NeverhoodOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
	            .addLayout(GUI::ThemeLayout::kLayoutVertical)
	                .addPadding(16, 16, 16, 16)
	                .addWidget("OriginalSaveLoad", "Checkbox")
	                .addWidget("SkipHallOfRecords", "Checkbox")
		        .addWidget("ScaleMakingOfVideos", "Checkbox")
	            .closeLayout()
	        .closeDialog();
}

void NeverhoodOptionsWidget::load() {
	if (_originalSaveLoadCheckbox) {
		_originalSaveLoadCheckbox->setState(ConfMan.getBool("originalsaveload", _domain));
	}

	if (_skipHallOfRecordsCheckbox) {
		_skipHallOfRecordsCheckbox->setState(ConfMan.getBool("skiphallofrecordsscenes", _domain));
	}

	if (_scaleMakingOfVideosCheckbox) {
		_scaleMakingOfVideosCheckbox->setState(ConfMan.getBool("scalemakingofvideos", _domain));
	}
}

bool NeverhoodOptionsWidget::save() {
	if (_originalSaveLoadCheckbox) {
		ConfMan.setBool("originalsaveload", _originalSaveLoadCheckbox->getState(), _domain);
	}

	if (_skipHallOfRecordsCheckbox) {
		ConfMan.setBool("skiphallofrecordsscenes", _skipHallOfRecordsCheckbox->getState(), _domain);
	}

	if (_scaleMakingOfVideosCheckbox) {
		ConfMan.setBool("scalemakingofvideos", _scaleMakingOfVideosCheckbox->getState(), _domain);
	}

	return true;
}

} // End of namespace Neverhood
