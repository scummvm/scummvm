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

#include "gui/saveload-dialog.h"

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
#include "backends/cloud/cloudmanager.h"
#include "backends/cloud/savessyncrequest.h"
#include "backends/networking/curl/connectionmanager.h"
#endif

#include "common/translation.h"
#include "common/config-manager.h"

#include "gui/message.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widgets/edittext.h"

#include "graphics/scaler.h"
#include "common/savefile.h"
#include "engines/engine.h"

namespace GUI {

#define SCALEVALUE(val) ((val) * g_gui.getScaleFactor())

#if defined(USE_CLOUD) && defined(USE_LIBCURL)

enum {
	kCancelSyncCmd = 'PDCS',
	kBackgroundSyncCmd = 'PDBS'
};

SaveLoadCloudSyncProgressDialog::SaveLoadCloudSyncProgressDialog(bool canRunInBackground, SaveLoadChooserDialog *parent)
	: Dialog("SaveLoadCloudSyncProgress"), _close(false), _pollFrame(0), _parent(parent)
{
	_label = new StaticTextWidget(this, "SaveLoadCloudSyncProgress.TitleText", _("Downloading saves..."));
	uint32 progress = (uint32)(100 * CloudMan.getSyncDownloadingProgress());
	_progressBar = new SliderWidget(this, "SaveLoadCloudSyncProgress.ProgressBar");
	_progressBar->setMinValue(0);
	_progressBar->setMaxValue(100);
	_progressBar->setValue(progress);
	_progressBar->setEnabled(false);
	_percentLabel = new StaticTextWidget(this, "SaveLoadCloudSyncProgress.PercentText", Common::String::format("%u %%", progress));
	new ButtonWidget(this, "SaveLoadCloudSyncProgress.Cancel", _("Cancel"), Common::U32String(), kCancelSyncCmd, Common::ASCII_ESCAPE);	// Cancel dialog
	ButtonWidget *backgroundButton = new ButtonWidget(this, "SaveLoadCloudSyncProgress.Background", _("Run in background"), Common::U32String(), kBackgroundSyncCmd, Common::ASCII_RETURN);	// Confirm dialog
	backgroundButton->setEnabled(canRunInBackground);
}

SaveLoadCloudSyncProgressDialog::~SaveLoadCloudSyncProgressDialog() {
}

void SaveLoadCloudSyncProgressDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch(cmd) {
	case kCancelSyncCmd:
		setResult(kCancelSyncCmd);
		close();
		break;

	case kBackgroundSyncCmd:
		_close = true;
		break;

	default:
		break;
	}

	Dialog::handleCommand(sender, cmd, data);
}

void SaveLoadCloudSyncProgressDialog::handleTickle() {
	pollCloudMan();

	if (_close) {
		setResult(kBackgroundSyncCmd);
		close();
	}

	Dialog::handleTickle();
}

void SaveLoadCloudSyncProgressDialog::pollCloudMan() {
	_pollFrame = (_pollFrame + 1) % 60;
	if (_pollFrame != 1)
		return;

	const bool syncing = CloudMan.isSyncing();
	const uint32 progress = (uint32)(100 * CloudMan.getSyncDownloadingProgress());

	if (!syncing || progress == 100) {
		_close = true;
	}

	Cloud::Storage::SyncDownloadingInfo info;
	CloudMan.getSyncDownloadingInfo(info);

	const char *downloadedUnits, *totalUnits;
	Common::String downloaded = Common::getHumanReadableBytes(info.bytesDownloaded, downloadedUnits);
	Common::String total = Common::getHumanReadableBytes(info.bytesToDownload, totalUnits);

	Common::String progressPercent = Common::String::format("%u %%", progress);
	Common::String filesDownloaded = Common::String::format("%llu", (unsigned long long)info.filesDownloaded);
	Common::String filesToDownload = Common::String::format("%llu", (unsigned long long)info.filesToDownload);

	_percentLabel->setLabel(
		Common::U32String::format(
			_("%s (%s %S / %s %S, %s / %s files)"),
			progressPercent.c_str(),
			downloaded.c_str(), _(downloadedUnits).c_str(),
			total.c_str(), _(totalUnits).c_str(),
			filesDownloaded.c_str(), filesToDownload.c_str()
		)
	);
	_progressBar->setValue(progress);
	_progressBar->markAsDirty();

	if (_parent) {
		_parent->updateSaveList();
		_parent->reflowLayout();
	}
}
#endif

#ifndef DISABLE_SAVELOADCHOOSER_GRID
SaveLoadChooserType getRequestedSaveLoadDialog(const MetaEngine *metaEngine) {
	const Common::String &userConfig = ConfMan.get("gui_saveload_chooser", Common::ConfigManager::kApplicationDomain);

	// Check (and update if necessary) the theme config here. This catches
	// resolution changes, which happened after the GUI was closed. This
	// should assure that the correct GUI width/height are returned below and
	// prevent the logic from picking the grid dialog, even though it is not
	// possible to use it.
	g_gui.checkScreenChange();

	if (g_gui.getGUIWidth() >= 640 && g_gui.getGUIHeight() >= 400
		&& metaEngine->hasFeature(MetaEngine::kSavesSupportMetaInfo)
		&& metaEngine->hasFeature(MetaEngine::kSavesSupportThumbnail)
		&& userConfig.equalsIgnoreCase("grid")) {
		// In case we are 640x400 or higher, this dialog is not in save mode,
		// the user requested the grid dialog and the engines supports it we
		// try to set it up.
		return kSaveLoadDialogGrid;
	} else {
		// In all other cases we want to use the list dialog.
		return kSaveLoadDialogList;
	}
}

enum {
	kListSwitchCmd = 'LIST',
	kGridSwitchCmd = 'GRID'
};
#endif // !DISABLE_SAVELOADCHOOSER_GRID

SaveLoadChooserDialog::SaveLoadChooserDialog(const Common::String &dialogName, const bool saveMode)
	: Dialog(dialogName), _metaEngine(nullptr), _delSupport(false), _metaInfoSupport(false),
	_thumbnailSupport(false), _saveDateSupport(false), _playTimeSupport(false), _saveMode(saveMode),
	_dialogWasShown(false)
#ifndef DISABLE_SAVELOADCHOOSER_GRID
	, _listButton(nullptr), _gridButton(nullptr)
#endif // !DISABLE_SAVELOADCHOOSER_GRID
#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	, _pollFrame(0), _didUpdateAfterSync(true)
#endif
	{
#ifndef DISABLE_SAVELOADCHOOSER_GRID
	addChooserButtons();
#endif // !DISABLE_SAVELOADCHOOSER_GRID
}

SaveLoadChooserDialog::SaveLoadChooserDialog(int x, int y, int w, int h, const bool saveMode)
	: Dialog(x, y, w, h), _metaEngine(nullptr), _delSupport(false), _metaInfoSupport(false),
	_thumbnailSupport(false), _saveDateSupport(false), _playTimeSupport(false), _saveMode(saveMode),
	_dialogWasShown(false)
#ifndef DISABLE_SAVELOADCHOOSER_GRID
	, _listButton(nullptr), _gridButton(nullptr)
#endif // !DISABLE_SAVELOADCHOOSER_GRID
#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	, _pollFrame(0), _didUpdateAfterSync(true)
#endif
	{
#ifndef DISABLE_SAVELOADCHOOSER_GRID
	addChooserButtons();
#endif // !DISABLE_SAVELOADCHOOSER_GRID
}

SaveLoadChooserDialog::~SaveLoadChooserDialog() {
}

void SaveLoadChooserDialog::open() {
	Dialog::open();

	// So that quitting ScummVM will not cause the dialog result to say a
	// saved game was selected.
	setResult(-1);

	_dialogWasShown = false;
}

void SaveLoadChooserDialog::close() {
	Dialog::close();
}

int SaveLoadChooserDialog::run(const Common::String &target, const MetaEngine *metaEngine) {
	_metaEngine = metaEngine;
	_target = target;
	_delSupport = _metaEngine->hasFeature(MetaEngine::kSupportsDeleteSave);
	_metaInfoSupport = _metaEngine->hasFeature(MetaEngine::kSavesSupportMetaInfo);
	_thumbnailSupport = _metaInfoSupport && _metaEngine->hasFeature(MetaEngine::kSavesSupportThumbnail);
	_saveDateSupport = _metaInfoSupport && _metaEngine->hasFeature(MetaEngine::kSavesSupportCreationDate);
	_playTimeSupport = _metaInfoSupport && _metaEngine->hasFeature(MetaEngine::kSavesSupportPlayTime);

	return runIntern();
}

void SaveLoadChooserDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
#ifndef DISABLE_SAVELOADCHOOSER_GRID
	switch (cmd) {
	case kListSwitchCmd:
		setResult(kSwitchSaveLoadDialog);
		// We save the requested dialog type here to avoid the setting to be
		// overwritten when our reflowLayout logic selects a different dialog
		// type.
		ConfMan.set("gui_saveload_chooser", "list", Common::ConfigManager::kApplicationDomain);
		close();
		break;

	case kGridSwitchCmd:
		setResult(kSwitchSaveLoadDialog);
		// See above.
		ConfMan.set("gui_saveload_chooser", "grid", Common::ConfigManager::kApplicationDomain);
		close();
		break;

	default:
		break;
	}
#endif // !DISABLE_SAVELOADCHOOSER_GRID

	return Dialog::handleCommand(sender, cmd, data);
}

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
void SaveLoadChooserDialog::runSaveSync(bool hasSavepathOverride) {
	if (!CloudMan.isSyncing()) {
		if (hasSavepathOverride) {
			CloudMan.showCloudDisabledIcon();
		} else {
			CloudMan.syncSaves();
		}
	}
}
#endif

void SaveLoadChooserDialog::handleTickle() {
#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	if (!_dialogWasShown && CloudMan.isSyncing()) {
		Common::Array<Common::String> files = CloudMan.getSyncingFiles();
		if (!files.empty()) {
			{
				SaveLoadCloudSyncProgressDialog dialog(_metaEngine ? _metaEngine->hasFeature(MetaEngine::kSimpleSavesNames) : false, this);
				int result = dialog.runModal();
				if (result == kCancelSyncCmd) {
					CloudMan.cancelSync();
				}
			}
			_dialogWasShown = true;
			updateSaveList();
		}
	}

	pollCloudMan();
#endif
	Dialog::handleTickle();
}

void SaveLoadChooserDialog::reflowLayout() {
#ifndef DISABLE_SAVELOADCHOOSER_GRID
	const SaveLoadChooserType currentType = getType();
	const SaveLoadChooserType requestedType = getRequestedSaveLoadDialog(_metaEngine);

	addChooserButtons();
	if (currentType == kSaveLoadDialogList) {
		_listButton->setEnabled(false);
	}
	if (currentType == kSaveLoadDialogGrid) {
		_gridButton->setEnabled(false);
	}

	// Change the dialog type if there is any need for it.
	if (requestedType != currentType) {
		setResult(kSwitchSaveLoadDialog);
		close();
	}
#endif // !DISABLE_SAVELOADCHOOSER_GRID

	Dialog::reflowLayout();
}

void SaveLoadChooserDialog::updateSaveList() {
#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	Common::Array<Common::String> files = CloudMan.getSyncingFiles(); //returns empty array if not syncing
	g_system->getSavefileManager()->updateSavefilesList(files);
#endif
	listSaves();
}

void SaveLoadChooserDialog::listSaves() {
	if (!_metaEngine) return; //very strange
	_saveList = _metaEngine->listSaves(_target.c_str(), _saveMode);

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	//if there is Cloud support, add currently synced files as "locked" saves in the list
	if (_metaEngine->hasFeature(MetaEngine::kSimpleSavesNames)) {
		Common::String pattern = _target + ".###";
		Common::Array<Common::String> files = CloudMan.getSyncingFiles(); //returns empty array if not syncing
		for (uint32 i = 0; i < files.size(); ++i) {
			if (!files[i].matchString(pattern, true))
				continue;

			//make up some slot number
			int slotNum = 0;
			for (uint32 j = (files[i].size() > 3 ? files[i].size() - 3 : 0); j < files[i].size(); ++j) { //3 last chars
				char c = files[i][j];
				if (c < '0' || c > '9')
					continue;
				slotNum = slotNum * 10 + (c - '0');
			}

			SaveStateDescriptor slot(_metaEngine, slotNum, files[i]);
			slot.setLocked(true);
			_saveList.push_back(slot);
		}

		Common::sort(_saveList.begin(), _saveList.end(), SaveStateDescriptorSlotComparator());
	}
#endif
}

void SaveLoadChooserDialog::activate(int slot, const Common::U32String &description) {
	if (!_saveList.empty() && slot < int(_saveList.size())) {
		const SaveStateDescriptor &desc = _saveList[slot];
		if (_saveMode)
			_resultString = description.empty() ? desc.getDescription() : description;
		setResult(desc.getSaveSlot());
	}
	close();
}

#ifndef DISABLE_SAVELOADCHOOSER_GRID
void SaveLoadChooserDialog::addChooserButtons() {
	if (_listButton) {
		removeWidget(_listButton);
		delete _listButton;
	}

	if (_gridButton) {
		removeWidget(_gridButton);
		delete _gridButton;
	}

	_listButton = createSwitchButton("SaveLoadChooser.ListSwitch", Common::U32String("L"), _("List view"), ThemeEngine::kImageList, kListSwitchCmd);
	_gridButton = createSwitchButton("SaveLoadChooser.GridSwitch", Common::U32String("G"), _("Grid view"), ThemeEngine::kImageGrid, kGridSwitchCmd);
	if (!_metaInfoSupport || !_thumbnailSupport || !(g_gui.getGUIWidth() >= 640 && g_gui.getGUIHeight() >= 400)) {
		_gridButton->setEnabled(false);
		_listButton->setEnabled(false);
	}
}

ButtonWidget *SaveLoadChooserDialog::createSwitchButton(const Common::String &name, const Common::U32String &desc, const Common::U32String &tooltip, const char *image, uint32 cmd) {
	ButtonWidget *button;

#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowChooserPics") == 1 && g_gui.theme()->supportsImages()) {
		button = new PicButtonWidget(this, name, tooltip, cmd);
		((PicButtonWidget *)button)->setGfxFromTheme(image, kPicButtonStateEnabled, false);
	} else
#endif
		button = new ButtonWidget(this, name, desc, tooltip, cmd);

	return button;
}
#endif // !DISABLE_SAVELOADCHOOSER_GRID

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
void SaveLoadChooserDialog::pollCloudMan() {
	_pollFrame = (_pollFrame + 1) % 60;
	if (_pollFrame != 1)
		return;

	const bool syncing = CloudMan.isSyncing();
	const uint32 progress = (uint32)(100 * CloudMan.getSyncDownloadingProgress());

	bool update = false;
	if (syncing && progress < 100) {
		update = true;
		_didUpdateAfterSync = false;
	} else {
		if (!_didUpdateAfterSync) { // do one more update when sync is over
			update = true;
			_didUpdateAfterSync = true;
		}
	}

	if (update)
		updateSaveList();
}
#endif

// SaveLoadChooserSimple implementation

enum {
	kChooseCmd = 'CHOS',
	kDelCmd = 'DEL '
};

SaveLoadChooserSimple::SaveLoadChooserSimple(const Common::U32String &title, const Common::U32String &buttonLabel, bool saveMode)
	: SaveLoadChooserDialog("SaveLoadChooser", saveMode), _list(nullptr), _chooseButton(nullptr), _deleteButton(nullptr), _gfxWidget(nullptr),
	_container(nullptr) {
	_backgroundType = ThemeEngine::kDialogBackgroundSpecial;

	_pageTitle = new StaticTextWidget(this, "SaveLoadChooser.Title", title);

	// Add choice list
	_list = new ListWidget(this, "SaveLoadChooser.List");
	_list->setNumberingMode(kListNumberingZero);
	_list->setEditable(saveMode);
	_list->enableQuickSelect(false); // quick select is only useful on sorted list

	_gfxWidget = new GraphicsWidget(this, 0, 0, 10, 10);

	_date = new StaticTextWidget(this, 0, 0, 10, 10, _("No date saved"), Graphics::kTextAlignCenter);
	_time = new StaticTextWidget(this, 0, 0, 10, 10, _("No time saved"), Graphics::kTextAlignCenter);
	_playtime = new StaticTextWidget(this, 0, 0, 10, 10, _("No playtime saved"), Graphics::kTextAlignCenter);

	// Buttons
	new ButtonWidget(this, "SaveLoadChooser.Cancel", _("Cancel"), Common::U32String(), kCloseCmd);
	_chooseButton = new ButtonWidget(this, "SaveLoadChooser.Choose", buttonLabel, Common::U32String(), kChooseCmd);
	_chooseButton->setEnabled(false);

	_deleteButton = new ButtonWidget(this, "SaveLoadChooser.Delete", _("Delete"), Common::U32String(), kDelCmd);
	_deleteButton->setEnabled(false);

	_delSupport = _metaInfoSupport = _thumbnailSupport = false;

	addThumbnailContainer();
}

void SaveLoadChooserSimple::addThumbnailContainer() {
	// When switching layouts, create / remove the thumbnail container as needed
	if (g_gui.xmlEval()->getVar("Globals.SaveLoadChooser.ExtInfo.Visible") == 1 && !_container) {
		_container = new ContainerWidget(this, "SaveLoadChooser.Thumbnail");
	} else if (g_gui.xmlEval()->getVar("Globals.SaveLoadChooser.ExtInfo.Visible") == 0 && _container) {
		removeWidget(_container);
		delete _container;
		_container = nullptr;
	}
}

int SaveLoadChooserSimple::runIntern() {
	if (_gfxWidget)
		_gfxWidget->setGfx((Graphics::ManagedSurface *)nullptr);

	_resultString.clear();
	reflowLayout();
	updateSaveList();

	return Dialog::runModal();
}

const Common::U32String SaveLoadChooserSimple::getResultString() const {
	int selItem = _list->getSelected();
	return (selItem >= 0) ? _list->getSelectedString() : _resultString;
}

void SaveLoadChooserSimple::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int selItem = _list->getSelected();

	switch (cmd) {
	case kListItemSingleClickedCmd:
		// This command is sent even if an item is clicked while in edit mode,
		// but that's okay because startEditMode() does nothing when editing.
		if (_list->isEditable() && _chooseButton->isEnabled()) {
			_list->startEditMode();
		}
		break;
	case kListItemEditModeStartedCmd:
		if (_list->getSelectedString() == _("Untitled saved game")) {
			_list->setEditString(Common::U32String());
			_list->setEditColor(ThemeEngine::kFontColorNormal);
		}
		break;
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd:
		if (selItem >= 0 && _chooseButton->isEnabled()) {
			if (_list->isEditable() || !_list->getSelectedString().empty()) {
				_list->endEditMode();
				Common::U32String description;
				if (!_saveList.empty())
					description = _list->getSelectedString();
				activate(selItem, description);
			}
		}
		break;
	case kChooseCmd:
		_list->endEditMode();
		if (selItem >= 0) {
			Common::U32String description;
			if (!_saveList.empty())
				description = _list->getSelectedString();
			activate(selItem, description);
		}
		break;
	case kListSelectionChangedCmd:
		updateSelection(true);
		break;
	case kDelCmd:
	case kListItemRemovalRequestCmd:
		if (_deleteButton->isEnabled()) {
			MessageDialog alert(_("Do you really want to delete this saved game?"),
								_("Delete"), _("Cancel"));
			if (alert.runModal() == kMessageOK) {
				int saveSlot = _saveList[selItem].getSaveSlot();
				if (_metaEngine->removeSaveState(_target.c_str(), saveSlot)) {
					setResult(-1);
					int scrollPos = _list->getCurrentScrollPos();
					updateSaveList(); // resets scroll pos
					_list->scrollTo(scrollPos);
					updateSelection(true);
				} else {
					// Delete failed. SavefileManager may contain an error description.
					Common::String errorDesc;
					if (g_system->getSavefileManager()->getError().getCode() != Common::kNoError) {
						errorDesc = ", " + g_system->getSavefileManager()->getErrorDesc();
					}
					warning("Error deleting %s save slot %d%s", _target.c_str(), saveSlot, errorDesc.c_str());

					GUI::MessageDialog errorDialog(_("Error deleting saved game"));
					errorDialog.runModal();
				}
			}
		}
		break;
	case kCloseCmd:
		setResult(-1);
		// Fall through
	default:
		SaveLoadChooserDialog::handleCommand(sender, cmd, data);
	}
}

void SaveLoadChooserSimple::reflowLayout() {
	addThumbnailContainer();

	SaveLoadChooserDialog::reflowLayout();

	if (g_gui.xmlEval()->getVar("Globals.SaveLoadChooser.ExtInfo.Visible") == 1 && (_thumbnailSupport || _saveDateSupport || _playTimeSupport)) {
		int16 x, y;
		int16 w, h;

		if (!g_gui.xmlEval()->getWidgetData("SaveLoadChooser.Thumbnail", x, y, w, h))
			error("Error when loading position data for Save/Load Thumbnails");

		// Even if there is no thumbnail support, getWidgetData() will provide default thumbnail values
		int thumbW = SCALEVALUE(kThumbnailWidth);
		int thumbH = SCALEVALUE(kThumbnailHeight2);
		int thumbX = x + (w >> 1) - (thumbW >> 1);
		int thumbY = y + kLineHeight;

		int textLines = 0;
		if (_saveDateSupport)
			textLines += 2;
		if (_playTimeSupport)
			textLines++;
		if (textLines > 0)
			textLines++; // add a line of padding at the bottom

		if (_thumbnailSupport) {
			_gfxWidget->resize(thumbX, thumbY, thumbW, thumbH, false);
			_gfxWidget->setVisible(true);
		} else {
			// choose sensible values for displaying playtime and date/time when a thumbnail is not being used
			thumbH = 0;
			thumbY = y;
			h = kLineHeight;
			_gfxWidget->setVisible(false);
		}

		int height = thumbY + thumbH + kLineHeight;

		if (_saveDateSupport) {
			_date->resize(thumbX, height, thumbW, kLineHeight, false);
			height += kLineHeight;
			_time->resize(thumbX, height, thumbW, kLineHeight, false);
			height += kLineHeight;
			_date->setVisible(_saveDateSupport);
			_time->setVisible(_saveDateSupport);
		} else {
			_date->setVisible(false);
			_time->setVisible(false);
		}

		if (_playTimeSupport) {
			_playtime->resize(thumbX, height, thumbW, kLineHeight, false);
			_playtime->setVisible(_playTimeSupport);
		} else {
			_playtime->setVisible(false);
		}

		_container->resize(x, y, w, h + (kLineHeight * textLines), false);
		_container->setVisible(true);

		updateSelection(false);
	} else {
		if (_container) _container->setVisible(false);
		_gfxWidget->setVisible(false);
		_date->setVisible(false);
		_time->setVisible(false);
		_playtime->setVisible(false);
	}
}

void SaveLoadChooserSimple::updateSelection(bool redraw) {
	int selItem = _list->getSelected();

	bool isDeletable = _delSupport;
	bool isWriteProtected = false;
	bool isLocked = false;

	// We used to support letting the themes specify the fill color with our
	// initial theme based GUI. But this support was dropped.
	_gfxWidget->setGfx(-1, -1, 0, 0, 0);
	_date->setLabel(_("No date saved"));
	_time->setLabel(_("No time saved"));
	_playtime->setLabel(_("No playtime saved"));

	if (selItem >= 0 && _metaInfoSupport) {
		SaveStateDescriptor desc = (_saveList[selItem].getLocked() ? _saveList[selItem] : _metaEngine->querySaveMetaInfos(_target.c_str(), _saveList[selItem].getSaveSlot()));
		if (!_saveList[selItem].getLocked() && desc.getSaveSlot() >= 0 && !desc.getDescription().empty())
			_saveList[selItem] = desc;

		isDeletable = _saveList[selItem].getDeletableFlag() && _delSupport;
		isWriteProtected = desc.getWriteProtectedFlag() ||
			_saveList[selItem].getWriteProtectedFlag();
		isLocked = desc.getLocked();

		if (_thumbnailSupport) {
			const Graphics::Surface *thumb = desc.getThumbnail();
			if (thumb && _gfxWidget->isVisible())
				_gfxWidget->setGfx(thumb, true);
		}

		if (_saveDateSupport) {
			const Common::U32String &saveDate = desc.getSaveDate();
			if (!saveDate.empty())
				_date->setLabel(_("Date: ") + saveDate);

			const Common::U32String &saveTime = desc.getSaveTime();
			if (!saveTime.empty())
				_time->setLabel(_("Time: ") + saveTime);
		}

		if (_playTimeSupport) {
			const Common::U32String &playTime = desc.getPlayTime();
			if (!playTime.empty())
				_playtime->setLabel(_("Playtime: ") + playTime);
		}
	}

	if (_list->isEditable()) {
		// Disable the save button if slot is locked, nothing is selected,
		// or if the selected game is write protected
		_chooseButton->setEnabled(!isLocked && selItem >= 0 && !isWriteProtected);
	} else {
		// Disable the load button if slot is locked, nothing is selected,
		// or if an empty list item is selected.
		_chooseButton->setEnabled(!isLocked && selItem >= 0 && !_list->getSelectedString().empty());
	}

	// Delete will always be disabled if the engine doesn't support it.
	_deleteButton->setEnabled(isDeletable && !isLocked && (selItem >= 0) && (!_list->getSelectedString().empty()));

	if (redraw) {
		_gfxWidget->markAsDirty();
		_date->markAsDirty();
		_time->markAsDirty();
		_playtime->markAsDirty();
		_chooseButton->markAsDirty();
		_deleteButton->markAsDirty();

		g_gui.scheduleTopDialogRedraw();
	}
}

void SaveLoadChooserSimple::open() {
	SaveLoadChooserDialog::open();

	// Scroll the list to the last used entry.
	_list->scrollTo(ConfMan.getInt("gui_saveload_last_pos"));
}

void SaveLoadChooserSimple::close() {
	// Save the current scroll position/used entry.
	const int result = getResult();
	if (result >= 0) {
		ConfMan.setInt("gui_saveload_last_pos", result);
	} else {
		// Use the current scroll position here.
		// TODO: This means we canceled the dialog (or switch to the grid). Do
		// we want to save this position here? Does the user want that?
		// TODO: Do we want to save the current scroll position or the
		// currently selected item here? The scroll position is what the user
		// currently sees and seems to make more sense.
		ConfMan.setInt("gui_saveload_last_pos", _list->getCurrentScrollPos());
	}

	_metaEngine = nullptr;
	_target.clear();
	_saveList.clear();
	_list->setList(Common::U32StringArray());

	SaveLoadChooserDialog::close();
}

void SaveLoadChooserSimple::updateSaveList() {
	SaveLoadChooserDialog::updateSaveList();

	int curSlot = 0;
	int saveSlot = 0;
	Common::U32StringArray saveNames;
	ThemeEngine::FontColor color = ThemeEngine::kFontColorNormal;
	Common::U32String emptyDesc;
	for (SaveStateList::const_iterator x = _saveList.begin(); x != _saveList.end(); ++x) {
		// Handle gaps in the list of save games
		saveSlot = x->getSaveSlot();
		if (curSlot < saveSlot) {
			while (curSlot < saveSlot) {
				SaveStateDescriptor dummySave(_metaEngine, curSlot, "");
				_saveList.insert_at(curSlot, dummySave);
				saveNames.push_back(emptyDesc);
				curSlot++;
			}

			// Sync the save list iterator
			for (x = _saveList.begin(); x != _saveList.end(); ++x) {
				if (x->getSaveSlot() == saveSlot)
					break;
			}
		}

		// Show "Untitled saved game" for empty/whitespace saved game descriptions
		Common::U32String description = x->getDescription();
		Common::U32String trimmedDescription = description;
		trimmedDescription.trim();
		if (trimmedDescription.empty()) {
			description = _("Untitled saved game");
			color = ThemeEngine::kFontColorAlternate;
		} else {
			color = x->getLocked() ? ThemeEngine::kFontColorAlternate : ThemeEngine::kFontColorNormal;
		}

		saveNames.push_back(GUI::ListWidget::getThemeColor(color) + description);
		curSlot++;
	}

	// Fill the rest of the save slots with empty saves

	int maximumSaveSlots = _metaEngine->getMaximumSaveSlot();

#ifdef __DS__
	// Low memory on the DS means too many save slots are impractical, so limit
	// the maximum here.
	if (maximumSaveSlots > 99) {
		maximumSaveSlots = 99;
	}
#endif

	for (int i = curSlot; i <= maximumSaveSlots; i++) {
		saveNames.push_back(emptyDesc);
		SaveStateDescriptor dummySave(_metaEngine, i, "");
		_saveList.push_back(dummySave);
	}

	int selected = _list->getSelected();
	_list->setList(saveNames);
	if (selected >= 0 && selected < (int)saveNames.size())
		_list->setSelected(selected);
	else
		_chooseButton->setEnabled(false);

	g_gui.scheduleTopDialogRedraw();
}

// SaveLoadChooserGrid implementation

#ifndef DISABLE_SAVELOADCHOOSER_GRID

enum {
	kNextCmd = 'NEXT',
	kPrevCmd = 'PREV',
	kNewSaveCmd = 'SAVE'
};

SaveLoadChooserGrid::SaveLoadChooserGrid(const Common::U32String &title, bool saveMode)
	: SaveLoadChooserDialog("SaveLoadChooser", saveMode), _lines(0), _columns(0), _entriesPerPage(0),
	_curPage(0), _newSaveContainer(nullptr), _nextFreeSaveSlot(0), _buttons() {
	_backgroundType = ThemeEngine::kDialogBackgroundSpecial;

	_pageTitle = new StaticTextWidget(this, "SaveLoadChooser.Title", title);

	// The list widget needs to be bound so it takes space in the layout
	ContainerWidget *list = new ContainerWidget(this, "SaveLoadChooser.List");
	list->setBackgroundType(ThemeEngine::kWidgetBackgroundNo);

	// Buttons
	new ButtonWidget(this, "SaveLoadChooser.Delete", _("Cancel"), Common::U32String(), kCloseCmd);
	_nextButton = new ButtonWidget(this, "SaveLoadChooser.Choose", _("Next"), Common::U32String(), kNextCmd);
	_nextButton->setEnabled(false);

	_prevButton = new ButtonWidget(this, "SaveLoadChooser.Cancel", _("Prev"), Common::U32String(), kPrevCmd);
	_prevButton->setEnabled(false);

	// Page display
	_pageDisplay = new StaticTextWidget(this, "SaveLoadChooser.PageDisplay", Common::U32String());
	_pageDisplay->setAlign(Graphics::kTextAlignEnd);
}

SaveLoadChooserGrid::~SaveLoadChooserGrid() {
	removeWidget(_pageTitle);
	delete _pageTitle;

	removeWidget(_pageDisplay);
	delete _pageDisplay;
}

const Common::U32String SaveLoadChooserGrid::getResultString() const {
	return _resultString;
}

void SaveLoadChooserGrid::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	const int slot = cmd + _curPage * _entriesPerPage - 1;
	if (cmd <= _entriesPerPage && slot < (int)_saveList.size()) {
		activate(slot, Common::U32String());
	}

	switch (cmd) {
	case kNextCmd:
		++_curPage;
		updateSaves();
		g_gui.scheduleTopDialogRedraw();
		break;

	case kPrevCmd:
		--_curPage;
		updateSaves();
		g_gui.scheduleTopDialogRedraw();
		break;

	case kNewSaveCmd:
		setResult(_nextFreeSaveSlot);
		close();
		break;

	case kCloseCmd:
		setResult(-1);
		// Fall through
	default:
		SaveLoadChooserDialog::handleCommand(sender, cmd, data);
	}
}

void SaveLoadChooserGrid::handleMouseWheel(int x, int y, int direction) {
	if (direction > 0) {
		if (_nextButton->isEnabled()) {
			++_curPage;
			updateSaves();
			g_gui.scheduleTopDialogRedraw();
		}
	} else {
		if (_prevButton->isEnabled()) {
			--_curPage;
			updateSaves();
			g_gui.scheduleTopDialogRedraw();
		}
	}
}

void SaveLoadChooserGrid::updateSaveList() {
	SaveLoadChooserDialog::updateSaveList();
	updateSaves();
	g_gui.scheduleTopDialogRedraw();
}

void SaveLoadChooserGrid::open() {
	SaveLoadChooserDialog::open();

	listSaves();
	_resultString.clear();

	// Load information to restore the last page the user had open.
	assert(_entriesPerPage != 0);
	const uint lastPos = ConfMan.getInt("gui_saveload_last_pos");
	const uint listSize = _saveList.size();
	uint bestMatch = 0;
	uint diff = 0xFFFFFFFF;

	// We look for the nearest available slot, since a slot might be missing
	// due to the user deleting it via the list based chooser, by deleting
	// it by hand, etc.
	for (uint i = 0; i < listSize; ++i) {
		uint curDiff = ABS(_saveList[i].getSaveSlot() - (int)lastPos);
		if (curDiff < diff) {
			diff = curDiff;
			bestMatch = i;
		}
	}

	_curPage = bestMatch / _entriesPerPage;

	// Determine the next free save slot for save mode
	if (_saveMode) {
		int lastSlot = -1;
		_nextFreeSaveSlot = -1;
		for (SaveStateList::const_iterator x = _saveList.begin(); x != _saveList.end(); ++x) {
			const int curSlot = x->getSaveSlot();

			// In case there was a gap found use the slot.
			if (lastSlot + 1 < curSlot) {
				// Check that the save slot can be used for user saves.
				SaveStateDescriptor desc = _metaEngine->querySaveMetaInfos(_target.c_str(), lastSlot + 1);
				if (!desc.getWriteProtectedFlag()) {
					_nextFreeSaveSlot = lastSlot + 1;
					break;
				}
			}

			lastSlot = curSlot;
		}

		// Use the next available slot otherwise.
		const int maxSlot = _metaEngine->getMaximumSaveSlot();
		for (int i = lastSlot; _nextFreeSaveSlot == -1 && i < maxSlot; ++i) {
			// Check that the save slot can be used for user saves.
			SaveStateDescriptor desc = _metaEngine->querySaveMetaInfos(_target.c_str(), i + 1);
			if (!desc.getWriteProtectedFlag()) {
				_nextFreeSaveSlot = i + 1;
			}
		}

		// Do not allow more slots than available
		if (_nextFreeSaveSlot > maxSlot)
			_nextFreeSaveSlot = -1;
	}

	updateSaves();
}

void SaveLoadChooserGrid::reflowLayout() {
	// HACK: The page display is not available in low resolution layout. We
	// remove and readd the widget here to avoid our GUI from erroring out.
	removeWidget(_pageDisplay);
	if (g_gui.xmlEval()->getVar("Globals.ShowChooserPageDisplay") == 1) {
		_pageDisplay->init();
	}

	SaveLoadChooserDialog::reflowLayout();
	destroyButtons();

	// HACK: The whole code below really works around the fact, that we have
	// no easy way to dynamically layout widgets.
	const uint16 availableWidth = getWidth() - 20;
	int16 availableHeight;

	int16 x, y;
	int16 w;
	if (!g_gui.xmlEval()->getWidgetData("SaveLoadChooser.List", x, y, w, availableHeight))
		error("Could not load widget position for 'SaveLoadChooser.List'");

	const int16 buttonWidth = SCALEVALUE(kThumbnailWidth + 6);
	const int16 buttonHeight = SCALEVALUE(kThumbnailHeight2 + 6);

	const int16 containerFrameWidthAdd = SCALEVALUE(10);
	const int16 containerFrameHeightAdd = 0;
	const int16 containerWidth = buttonWidth + containerFrameWidthAdd;
	const int16 containerHeight = buttonHeight + kLineHeight + containerFrameHeightAdd;

	const int16 defaultSpacingHorizontal = SCALEVALUE(4);
	const int16 defaultSpacingVertical = SCALEVALUE(8);
	const int16 slotAreaWidth = containerWidth + defaultSpacingHorizontal;
	const int16 slotAreaHeight = containerHeight + defaultSpacingVertical;

	const uint oldEntriesPerPage = _entriesPerPage;
	_columns = MAX<uint>(1, availableWidth / slotAreaWidth);
	_lines = MAX<uint>(1, availableHeight / slotAreaHeight);
	_entriesPerPage = _columns * _lines;

	// In save mode the first button is always "New Save", thus we need to
	// adjust the entries per page here.
	if (_saveMode) {
		--_entriesPerPage;
	}

	// Recalculate the page number
	if (!_saveList.empty() && oldEntriesPerPage != 0) {
		if (_entriesPerPage != 0) {
			_curPage = (_curPage * oldEntriesPerPage) / _entriesPerPage;
		} else {
			_curPage = 0;
		}
	}

	const uint addX = _columns > 1 ? (availableWidth % slotAreaWidth) / (_columns - 1) : 0;
	//const uint addY = _lines > 1 ? (availableHeight % slotAreaHeight) / (_lines - 1) : 0;

	_buttons.reserve(_lines * _columns);
	y += defaultSpacingVertical / 2;
	for (uint curLine = 0; curLine < _lines; ++curLine, y += slotAreaHeight/* + addY*/) {
		for (uint curColumn = 0, curX = x + defaultSpacingHorizontal / 2; curColumn < _columns; ++curColumn, curX += slotAreaWidth + addX) {
			int dstY = containerFrameHeightAdd / 2;
			int dstX = containerFrameWidthAdd / 2;

			// In the save mode we will always create a new save button as the first button.
			if (_saveMode && curLine == 0 && curColumn == 0) {
				_newSaveContainer = new ContainerWidget(this, curX, y, containerWidth, containerHeight);
				ButtonWidget *newSave = new ButtonWidget(_newSaveContainer, dstX, dstY, buttonWidth, buttonHeight, _("New Save"), _("Create a new saved game"), kNewSaveCmd);
				// In case no more slots are free, we will disable the new save button
				if (_nextFreeSaveSlot == -1) {
					newSave->setEnabled(false);
				}
				continue;
			}

			ContainerWidget *container = new ContainerWidget(this, curX, y, containerWidth, containerHeight);
			container->setVisible(false);

			// Command 0 cannot be used, since it won't be send. Thus we will adjust
			// command number here, if required. This is only the case for load mode
			// since for save mode, the first button used is index 1 anyway.
			uint buttonCmd = curLine * _columns + curColumn;
			if (!_saveMode) {
				buttonCmd += 1;
			}

			PicButtonWidget *button = new PicButtonWidget(container, dstX, dstY, buttonWidth, buttonHeight, Common::U32String(), buttonCmd);
			dstY += buttonHeight;

			StaticTextWidget *description = new StaticTextWidget(container, dstX, dstY, buttonWidth, kLineHeight, Common::String(), Graphics::kTextAlignStart);

			_buttons.push_back(SlotButton(container, button, description));
		}
	}

	if (!_target.empty())
		updateSaves();
}

void SaveLoadChooserGrid::close() {
	// Save the current page.
	const int result = getResult();
	if (result >= 0 && result != _nextFreeSaveSlot) {
		// If the user selected a slot we use that one. We ignore new slots
		// here, since otherwise the dialog would reset to page 0 when the
		// user cancels the savename dialog.
		ConfMan.setInt("gui_saveload_last_pos", result);
	} else {
		// Otherwise save the first entry on the current page.
		// This is less precise than the solution above, since the number of
		// entries shown differs between save and load version of the dialog,
		// thus it might wrap to a different page than expected.
		// Similar things happen on resolution changes.
		// TODO: Should we ignore this here? Is the user likely to be
		// interested in having this page restored when he canceled?
		ConfMan.setInt("gui_saveload_last_pos", !_saveList.empty() ? _saveList[_curPage * _entriesPerPage].getSaveSlot() : 0);
	}

	SaveLoadChooserDialog::close();
	hideButtons();
}

int SaveLoadChooserGrid::runIntern() {
	int slot;
	do {
		const SaveLoadChooserType currentType = getType();
		const SaveLoadChooserType requestedType = getRequestedSaveLoadDialog(_metaEngine);

		// Catch resolution changes when the save name dialog was open.
		if (currentType != requestedType) {
			setResult(kSwitchSaveLoadDialog);
			return kSwitchSaveLoadDialog;
		}

		slot = runModal();
	} while (_saveMode && slot >= 0 && !selectDescription());

	// Special case for new save games. We need to handle this here, since
	// we cannot handle it in close() without problems.
	if (slot == _nextFreeSaveSlot) {
		ConfMan.setInt("gui_saveload_last_pos", slot);
	}

	return slot;
}

bool SaveLoadChooserGrid::selectDescription() {
	_savenameDialog.setDescription(_resultString);
	_savenameDialog.setTargetSlot(getResult());
	if (_savenameDialog.runModal() == 0) {
		_resultString = _savenameDialog.getDescription();
		return true;
	} else {
		return false;
	}
}

void SaveLoadChooserGrid::destroyButtons() {
	if (_newSaveContainer) {
		removeWidget(_newSaveContainer);
		delete _newSaveContainer;
		_newSaveContainer = nullptr;
	}

	for (ButtonArray::iterator i = _buttons.begin(), end = _buttons.end(); i != end; ++i) {
		removeWidget(i->container);
		delete i->container;
	}

	_buttons.clear();
}

void SaveLoadChooserGrid::hideButtons() {
	for (ButtonArray::iterator i = _buttons.begin(), end = _buttons.end(); i != end; ++i) {
		i->button->setGfx((Graphics::ManagedSurface *)nullptr);
		i->setVisible(false);
	}
}

void SaveLoadChooserGrid::updateSaves() {
	hideButtons();

	bool isWriteProtected = false;

	for (uint i = _curPage * _entriesPerPage, curNum = 0; i < _saveList.size() && curNum < _entriesPerPage; ++i, ++curNum) {
		const uint saveSlot = _saveList[i].getSaveSlot();

		SaveStateDescriptor desc =  (_saveList[i].getLocked() ? _saveList[i] : _metaEngine->querySaveMetaInfos(_target.c_str(), saveSlot));
		if (!_saveList[i].getLocked() && desc.getSaveSlot() >= 0 && !desc.getDescription().empty())
			_saveList[i] = desc;
		SlotButton &curButton = _buttons[curNum];
		curButton.setVisible(true);
		const Graphics::Surface *thumbnail = desc.getThumbnail();
		if (thumbnail) {
			curButton.button->setGfx(desc.getThumbnail());
		} else {
			curButton.button->setGfx(kThumbnailWidth, kThumbnailHeight2, 0, 0, 0);
		}
		curButton.description->setLabel(Common::U32String(Common::String::format("%d. ", saveSlot)) + _saveList[i].getDescription());

		Common::U32String tooltip(_("Name: "));
		tooltip += _saveList[i].getDescription();

		if (_saveDateSupport) {
			const Common::U32String &saveDate = desc.getSaveDate();
			if (!saveDate.empty()) {
				tooltip += Common::U32String("\n");
				tooltip +=  _("Date: ") + saveDate;
			}

			const Common::U32String &saveTime = desc.getSaveTime();
			if (!saveTime.empty()) {
				tooltip += Common::U32String("\n");
				tooltip += _("Time: ") + saveTime;
			}
		}

		if (_playTimeSupport) {
			const Common::U32String &playTime = desc.getPlayTime();
			if (!playTime.empty()) {
				tooltip += Common::U32String("\n");
				tooltip += _("Playtime: ") + playTime;
			}
		}

		curButton.button->setTooltip(tooltip);

		// In save mode we disable the button, when it's write protected.
		// TODO: Maybe we should not display it at all then?
		// We also disable and description the button if slot is locked
		isWriteProtected = desc.getWriteProtectedFlag() ||
			_saveList[i].getWriteProtectedFlag();
		if ((_saveMode && isWriteProtected) || desc.getLocked()) {
			curButton.button->setEnabled(false);
		} else {
			curButton.button->setEnabled(true);
		}
		curButton.description->setEnabled(!desc.getLocked());
	}

	const uint numPages = (_entriesPerPage != 0 && !_saveList.empty()) ? ((_saveList.size() + _entriesPerPage - 1) / _entriesPerPage) : 1;
	_pageDisplay->setLabel(Common::String::format("%u/%u", _curPage + 1, numPages));

	if (_curPage > 0)
		_prevButton->setEnabled(true);
	else
		_prevButton->setEnabled(false);

	if ((_curPage + 1) * _entriesPerPage < _saveList.size())
		_nextButton->setEnabled(true);
	else
		_nextButton->setEnabled(false);
}

SavenameDialog::SavenameDialog()
	: Dialog("SavenameDialog") {
	_title = new StaticTextWidget(this, "SavenameDialog.DescriptionText", Common::String());

	new ButtonWidget(this, "SavenameDialog.Cancel", _("Cancel"), Common::U32String(), kCloseCmd);
	new ButtonWidget(this, "SavenameDialog.Ok", _("OK"), Common::U32String(), kOKCmd);

	_description = new EditTextWidget(this, "SavenameDialog.Description", Common::U32String(), Common::U32String(), 0, kOKCmd);

	_targetSlot = 0;
}

void SavenameDialog::setDescription(const Common::U32String &desc) {
	_description->setEditString(desc);
}

const Common::U32String &SavenameDialog::getDescription() {
	return _description->getEditString();
}

void SavenameDialog::open() {
	Dialog::open();
	setResult(-1);

	_title->setLabel(Common::U32String::format(_("Enter a description for slot %d:"), _targetSlot));
}

void SavenameDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kOKCmd:
		setResult(0);
		close();
		break;

	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

#endif // !DISABLE_SAVELOADCHOOSER_GRID

} // End of namespace GUI
