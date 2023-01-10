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
		_scaleMakingOfVideosCheckbox(nullptr),
		_nhcPopUp(nullptr) {

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

	_repeatWillieHint = new GUI::CheckboxWidget(
		widgetsBoss(),
		"NeverhoodGameOptionsDialog.RepeatWillieHint", _("Repeat useful Willie's hint"),
		_("Repeat actual useful hint by Willie"));

	Common::FSDirectory dir(ConfMan.get("path", _domain));
	Common::String extraPath(ConfMan.get("extrapath", _domain));
	Common::FSDirectory extraDir(extraPath);
	Common::Array<Common::FSDirectory *> langdirs = { &dir, dir.getSubDirectory("language") };
	if (!extraPath.empty()) {
		langdirs.push_back(&extraDir);
		langdirs.push_back(extraDir.getSubDirectory("language"));
	}
	_nhcFiles.push_back("");
	for (Common::Array<Common::FSDirectory *>::const_iterator langdir = langdirs.begin(); langdir != langdirs.end(); langdir++) {
		Common::ArchiveMemberList nhcFileList;
		if (!(*langdir))
			continue;
		(*langdir)->listMatchingMembers(nhcFileList, "*.nhc");

		for (Common::ArchiveMemberList::iterator iter = nhcFileList.begin(); iter != nhcFileList.end(); ++iter) {
			Common::String nhcFileName = (*iter)->getName();
			nhcFileName.erase(nhcFileName.size() - 4); // remove .nhc extension
			_nhcFiles.push_back(nhcFileName);
		}
	}

	if (_nhcFiles.size() > 1) {
		// I18N: NHC is a file extension
		GUI::StaticTextWidget *nhcCaption = new GUI::StaticTextWidget(widgetsBoss(), "NeverhoodGameOptionsDialog.NhcDesc", _("NHC replacement:"));
		nhcCaption->setAlign(Graphics::kTextAlignRight);

		_nhcPopUp = new GUI::PopUpWidget(widgetsBoss(), "NeverhoodGameOptionsDialog.Nhc");

		for (uint i = 0; i < _nhcFiles.size(); i++)
			_nhcPopUp->appendEntry(_nhcFiles[i].empty() ? _("<original>") : Common::U32String(_nhcFiles[i]), i);
	}
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
			.addWidget("RepeatWillieHint", "Checkbox")
	                .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
	                    .addPadding(0, 0, 0, 0)
	                    .addWidget("NhcDesc", "OptionsLabel")
	                    .addWidget("Nhc", "PopUp")
	                .closeLayout()
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

	if (_repeatWillieHint) {
		_repeatWillieHint->setState(ConfMan.getBool("repeatwilliehint", _domain));
	}

	if (_nhcPopUp) {
		Common::String nhcFile(ConfMan.get("nhc_file", _domain));
		for (uint i = 0; i < _nhcFiles.size(); i++)
			if (_nhcFiles[i].equalsIgnoreCase(nhcFile))
				_nhcPopUp->setSelectedTag(i);
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

	if (_repeatWillieHint) {
		ConfMan.setBool("repeatwilliehint", _repeatWillieHint->getState(), _domain);
	}

	if (_nhcPopUp) {
		uint32 selectedNhcFile = _nhcPopUp->getSelectedTag();
		if (selectedNhcFile < _nhcFiles.size())
			ConfMan.set("nhc_file", _nhcFiles[selectedNhcFile], _domain);
	}

	return true;
}

} // End of namespace Neverhood
