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

#define kSwitchToList -2
#define kSwitchToGrid -3

class SaveLoadChooserDialog : protected Dialog {
public:
	SaveLoadChooserDialog(const Common::String &dialogName, const bool saveMode);
	SaveLoadChooserDialog(int x, int y, int w, int h, const bool saveMode);

	virtual void open();

	virtual void reflowLayout();

	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

	int run(const Common::String &target, const MetaEngine *metaEngine);
	virtual const Common::String &getResultString() const = 0;

protected:
	virtual int runIntern() = 0;

	const bool				_saveMode;
	const MetaEngine		*_metaEngine;
	bool					_delSupport;
	bool					_metaInfoSupport;
	bool					_thumbnailSupport;
	bool					_saveDateSupport;
	bool					_playTimeSupport;
	Common::String			_target;

	GUI::ButtonWidget *_listButton;
	GUI::ButtonWidget *_gridButton;

	void addChooserButtons();
	GUI::ButtonWidget *createSwitchButton(const Common::String &name, const char *desc, const char *tooltip, const char *image, uint32 cmd = 0);
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

class LoadChooserThumbnailed : public SaveLoadChooserDialog {
public:
	LoadChooserThumbnailed(const Common::String &title);

	virtual const Common::String &getResultString() const;

	virtual void open();

	virtual void reflowLayout();

	virtual void close();
protected:
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleMouseWheel(int x, int y, int direction);
private:
	virtual int runIntern();

	uint _columns, _lines;
	uint _entriesPerPage;
	uint _curPage;
	SaveStateList _saveList;

	GUI::ButtonWidget *_nextButton;
	GUI::ButtonWidget *_prevButton;

	struct SlotButton {
		SlotButton() : container(0), button(0), description(0) {}
		SlotButton(ContainerWidget *c, PicButtonWidget *b, StaticTextWidget *d) : container(c), button(b), description(d) {}

		ContainerWidget  *container;
		PicButtonWidget  *button;
		StaticTextWidget *description;

		void setVisible(bool state) {
			container->setVisible(state);
			button->setVisible(state);
			description->setVisible(state);
		}
	};
	typedef Common::Array<SlotButton> ButtonArray;
	ButtonArray _buttons;
	void destroyButtons();
	void hideButtons();
	void updateSaves();
};

} // End of namespace GUI

#endif
