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

#include "gui/editrecorddialog.h"
#include "gui/widgets/edittext.h"
#include "common/translation.h"


namespace GUI {

const Common::U32String EditRecordDialog::getAuthor() {
	return _authorEdit->getEditString();
}

void EditRecordDialog::setAuthor(const Common::U32String &author) {
	_authorEdit->setEditString(author);
}

const Common::String EditRecordDialog::getNotes() {
	return _notesEdit->getEditString();
}

void EditRecordDialog::setNotes(const Common::String &desc) {
	_notesEdit->setEditString(desc);
}

const Common::String EditRecordDialog::getName() {
	return _nameEdit->getEditString();
}

void EditRecordDialog::setName(const Common::String &name) {
	_nameEdit->setEditString(name);
}

EditRecordDialog::~EditRecordDialog() {
}

EditRecordDialog::EditRecordDialog(const Common::U32String author, const Common::String name, const Common::String notes) : Dialog("EditRecordDialog") {
	new StaticTextWidget(this, "EditRecordDialog.AuthorLabel", _("Author:"));
	new StaticTextWidget(this, "EditRecordDialog.NameLabel", _("Name:"));
	new StaticTextWidget(this, "EditRecordDialog.NotesLabel", _("Notes:"));
	_authorEdit = new EditTextWidget(this, "EditRecordDialog.AuthorEdit", Common::U32String());
	_notesEdit = new EditTextWidget(this, "EditRecordDialog.NotesEdit", Common::U32String());
	_nameEdit = new EditTextWidget(this, "EditRecordDialog.NameEdit", Common::U32String());
	_authorEdit->setEditString(author);
	_notesEdit->setEditString(notes);
	_nameEdit->setEditString(name);
	new GUI::ButtonWidget(this, "EditRecordDialog.Cancel", _("Cancel"), Common::U32String(), kCloseCmd);
	new GUI::ButtonWidget(this, "EditRecordDialog.OK", _("OK"), Common::U32String(), kOKCmd);
}

void EditRecordDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch(cmd) {
	case kCloseCmd:
		setResult(kCloseCmd);
		close();
		break;
	case kOKCmd:
		setResult(kOKCmd);
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
		break;
	}
}

}
