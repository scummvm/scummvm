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

#ifndef GUI_SAVELOAD_DIALOG_H
#define GUI_SAVELOAD_DIALOG_H

#include "gui/dialog.h"
#include "gui/widgets/list.h"

#include "engines/metaengine.h"

namespace GUI {

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
class SaveLoadChooserDialog;

class SaveLoadCloudSyncProgressDialog : public Dialog { //protected?
	StaticTextWidget *_label, *_percentLabel;
	SliderWidget *_progressBar;
	SaveLoadChooserDialog *_parent;
	bool _close;
	int _pollFrame;

public:
	SaveLoadCloudSyncProgressDialog(bool canRunInBackground, SaveLoadChooserDialog *parent);
	~SaveLoadCloudSyncProgressDialog() override;

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleTickle() override;

private:
	void pollCloudMan();
};
#endif

#define kSwitchSaveLoadDialog -2

// TODO: We might want to disable the grid based save/load chooser for more
// platforms, than those which define DISABLE_FANCY_THEMES. But those are
// probably not able to handle the grid chooser anyway, so disabling it
// for them is a good start.
#ifdef DISABLE_FANCY_THEMES
#define DISABLE_SAVELOADCHOOSER_GRID
#endif // DISABLE_FANCY_THEMES

#ifndef DISABLE_SAVELOADCHOOSER_GRID
enum SaveLoadChooserType {
	kSaveLoadDialogList = 0,
	kSaveLoadDialogGrid = 1
};

SaveLoadChooserType getRequestedSaveLoadDialog(const MetaEngine *metaEngine);
#endif // !DISABLE_SAVELOADCHOOSER_GRID

class SaveLoadChooserDialog : protected Dialog {
public:
	SaveLoadChooserDialog(const Common::String &dialogName, const bool saveMode);
	SaveLoadChooserDialog(int x, int y, int w, int h, const bool saveMode);
	~SaveLoadChooserDialog() override;

	void open() override;
	void close() override;

	void reflowLayout() override;

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	virtual void runSaveSync(bool hasSavepathOverride);
#endif

	void handleTickle() override;

#ifndef DISABLE_SAVELOADCHOOSER_GRID
	virtual SaveLoadChooserType getType() const = 0;
#endif // !DISABLE_SAVELOADCHOOSER_GRID

	int run(const Common::String &target, const MetaEngine *metaEngine);
	virtual const Common::U32String getResultString() const = 0;

protected:
	virtual int runIntern() = 0;

	/** Common function to refresh the list on the screen. */
	virtual void updateSaveList(bool external);

	/**
	* Common function to get saves list from MetaEngine.
	*
	* It also checks whether there are some locked saves
	* because of saves sync and adds such saves as locked
	* slots. User sees these slots, but is unable to save
	* or load from these.
	*/
	virtual void listSaves();

	void activate(int slot, const Common::U32String &description);

	const bool					_saveMode;
	const MetaEngine		    *_metaEngine;
	bool						_delSupport;
	bool						_metaInfoSupport;
	bool						_thumbnailSupport;
	bool						_saveDateSupport;
	bool						_playTimeSupport;
	Common::String				_target;
	bool _dialogWasShown;
	SaveStateList				_saveList;
	Common::U32String			_resultString;

#ifndef DISABLE_SAVELOADCHOOSER_GRID
	ButtonWidget *_listButton;
	ButtonWidget *_gridButton;

	void addChooserButtons();
	ButtonWidget *createSwitchButton(const Common::String &name, const Common::U32String &desc, const Common::U32String &tooltip, const char *image, uint32 cmd = 0);
#endif // !DISABLE_SAVELOADCHOOSER_GRID

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	int _pollFrame;
	bool _didUpdateAfterSync;

	/** If CloudMan is syncing, this will refresh the list of saves. */
	void pollCloudMan();

	friend class SaveLoadCloudSyncProgressDialog;
#endif
};

class SaveLoadChooserSimple : public SaveLoadChooserDialog {
public:
	SaveLoadChooserSimple(const Common::U32String &title, const Common::U32String &buttonLabel, bool saveMode);

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

	const Common::U32String getResultString() const override;

	void reflowLayout() override;

#ifndef DISABLE_SAVELOADCHOOSER_GRID
	SaveLoadChooserType getType() const override { return kSaveLoadDialogList; }
#endif // !DISABLE_SAVELOADCHOOSER_GRID

	void open() override;
	void close() override;
protected:
	void updateSaveList(bool external) override;
private:
	int runIntern() override;

	ListWidget		*_list;
	ButtonWidget	*_chooseButton;
	ButtonWidget	*_deleteButton;
	GraphicsWidget	*_gfxWidget;
	ContainerWidget	*_container;
	StaticTextWidget	*_date;
	StaticTextWidget	*_time;
	StaticTextWidget	*_playtime;
	StaticTextWidget	*_pageTitle;

	void addThumbnailContainer();
	void updateSelection(bool redraw);
};

#ifndef DISABLE_SAVELOADCHOOSER_GRID

class EditTextWidget;

class SavenameDialog : public Dialog {
public:
	SavenameDialog();

	void setDescription(const Common::U32String &desc);
	const Common::U32String &getDescription();

	void setTargetSlot(int slot) { _targetSlot = slot; }

	void open() override;
protected:
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
private:
	int _targetSlot;
	StaticTextWidget *_title;
	EditTextWidget *_description;
};

class SaveLoadChooserGrid : public SaveLoadChooserDialog {
public:
	SaveLoadChooserGrid(const Common::U32String &title, bool saveMode);
	~SaveLoadChooserGrid() override;

	const Common::U32String getResultString() const override;

	void open() override;

	void reflowLayout() override;

	SaveLoadChooserType getType() const override { return kSaveLoadDialogGrid; }

	void close() override;
protected:
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleMouseWheel(int x, int y, int direction) override;
	void updateSaveList(bool external) override;
private:
	int runIntern() override;

	uint _columns, _lines;
	uint _entriesPerPage;
	uint _curPage;

	ButtonWidget *_nextButton;
	ButtonWidget *_prevButton;

	StaticTextWidget *_pageTitle;
	StaticTextWidget *_pageDisplay;

	ContainerWidget *_newSaveContainer;
	int _nextFreeSaveSlot;

	SavenameDialog _savenameDialog;
	bool selectDescription();

	struct SlotButton {
		SlotButton() : container(nullptr), button(nullptr), description(nullptr) {}
		SlotButton(ContainerWidget *c, PicButtonWidget *b, StaticTextWidget *d) : container(c), button(b), description(d) {}

		ContainerWidget  *container;
		PicButtonWidget  *button;
		StaticTextWidget *description;

		void setVisible(bool state) {
			container->setVisible(state);
		}
	};
	typedef Common::Array<SlotButton> ButtonArray;
	ButtonArray _buttons;
	void destroyButtons();
	void hideButtons();
	void updateSaves();
};

#endif // !DISABLE_SAVELOADCHOOSER_GRID

} // End of namespace GUI

#endif
