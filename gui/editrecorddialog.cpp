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

#include "editrecorddialog.h"
#include "gui/widgets/edittext.h"
#include "common/translation.h"


namespace GUI {

const Common::String EditRecordDialog::getAuthor() {
	return _authorEdit->getEditString();
}

void EditRecordDialog::setAuthor(const Common::String &author) {
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

EditRecordDialog::EditRecordDialog(const Common::String author, const Common::String name, const Common::String notes) : Dialog("EditRecordDialog") {
	new StaticTextWidget(this,"EditRecordDialog.AuthorLabel",_("Author:"));
	new StaticTextWidget(this,"EditRecordDialog.NameLabel",_("Name:"));
	new StaticTextWidget(this,"EditRecordDialog.NotesLabel",_("Notes:"));
	_authorEdit = new EditTextWidget(this, "EditRecordDialog.AuthorEdit","");
	_notesEdit = new EditTextWidget(this, "EditRecordDialog.NotesEdit","");
	_nameEdit = new EditTextWidget(this, "EditRecordDialog.NameEdit","");
	_authorEdit->setEditString(author);
	_notesEdit->setEditString(notes);
	_nameEdit->setEditString(name);
	new GUI::ButtonWidget(this, "EditRecordDialog.Cancel", _("Cancel"), 0, kCloseCmd);
	new GUI::ButtonWidget(this, "EditRecordDialog.OK", _("Ok"), 0, kOKCmd);
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
