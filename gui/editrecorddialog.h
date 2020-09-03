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

#ifndef GUI_EDITRECORDDIALOG_H
#define GUI_EDITRECORDDIALOG_H

#include "gui/dialog.h"

namespace GUI {

class EditTextWidget;
class StaticTextWidget;

class EditRecordDialog : public Dialog {
private:
	EditTextWidget *_notesEdit;
	EditTextWidget *_nameEdit;
	EditTextWidget *_authorEdit;
	EditRecordDialog() : Dialog("EditRecordDialog") {};
public:
	EditRecordDialog(const Common::U32String author, const Common::String name, const Common::String notes);
	~EditRecordDialog() override;

	const Common::U32String getAuthor();
	const Common::String getNotes();
	const Common::String getName();

	void setAuthor(const Common::U32String &author);
	void setNotes(const Common::String &desc);
	void setName(const Common::String &name);

	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
};

}// End of namespace GUI

#endif
