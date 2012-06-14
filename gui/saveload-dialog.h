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
 */

#ifndef GUI_SAVELOAD_DIALOG_H
#define GUI_SAVELOAD_DIALOG_H

#include "gui/dialog.h"
#include "gui/widgets/list.h"

#include "engines/metaengine.h"

namespace GUI {

class SaveLoadChooserDialog : protected Dialog {
public:
	SaveLoadChooserDialog(const Common::String &dialogName);
	SaveLoadChooserDialog(int x, int y, int w, int h);

	virtual void open();

	int run(const Common::String &target, const MetaEngine *metaEngine);
	virtual const Common::String &getResultString() const = 0;

protected:
	virtual int runIntern() = 0;

	const MetaEngine		*_metaEngine;
	bool					_delSupport;
	bool					_metaInfoSupport;
	bool					_thumbnailSupport;
	bool					_saveDateSupport;
	bool					_playTimeSupport;
	Common::String			_target;
};

class SaveLoadChooserSimple : public SaveLoadChooserDialog {
	typedef Common::String String;
	typedef Common::Array<Common::String> StringArray;
public:
	SaveLoadChooserSimple(const String &title, const String &buttonLabel, bool saveMode);

	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

	virtual const Common::String &getResultString() const;

	virtual void reflowLayout();

	virtual void close();
private:
	virtual int runIntern();

	GUI::ListWidget		*_list;
	GUI::ButtonWidget	*_chooseButton;
	GUI::ButtonWidget	*_deleteButton;
	GUI::GraphicsWidget	*_gfxWidget;
	GUI::ContainerWidget	*_container;
	GUI::StaticTextWidget	*_date;
	GUI::StaticTextWidget	*_time;
	GUI::StaticTextWidget	*_playtime;

	SaveStateList			_saveList;
	String					_resultString;

	uint8 _fillR, _fillG, _fillB;

	void updateSaveList();
	void updateSelection(bool redraw);
};

} // End of namespace GUI

#endif
