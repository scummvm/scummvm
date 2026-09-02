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

#include "mohawk/zoombini_dialogs.h"

#include "common/array.h"
#include "common/compression/deflate.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/language.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/util.h"

#include "engines/engine.h"

#include "gui/ThemeEval.h"
#include "gui/browser.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/scrollcontainer.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_state.h"
#include "mohawk/zoombini_text.h"

namespace Mohawk {

namespace {

class ZoombiniSeparatorWidget : public GUI::Widget {
public:
	ZoombiniSeparatorWidget(GUI::GuiObject *boss, const Common::String &name) : GUI::Widget(boss, name) {
		setFlags(GUI::WIDGET_ENABLED | GUI::WIDGET_CLEARBG);
	}

protected:
	void drawWidget() override {
		g_gui.theme()->drawLineSeparator(Common::Rect(_x, _y, _x + _w, _y + _h));
	}
};

class ZoombiniSaveListWidget : public GUI::ScrollContainerWidget {
public:
	ZoombiniSaveListWidget(GUI::GuiObject *boss, int x, int y, int w, int h, const Common::U32String &corruptedTooltip)
		: GUI::ScrollContainerWidget(boss, x, y, w, h), _corruptedTooltip(corruptedTooltip) {
		setFlags(GUI::WIDGET_DYN_TOOLTIP);
	}

	void setRowState(uint row, GUI::Widget *widget, bool corrupted) {
		if (_rows.size() <= row)
			_rows.resize(row + 1);
		_rows[row]._widget = widget;
		_rows[row]._corrupted = corrupted;
	}

	void handleTooltipUpdate(int x, int y) override {
		(void)x;
		updateTooltip(y);
	}

private:
	void updateTooltip(int y) {
		setTooltip(Common::U32String());
		const int absoluteY = getAbsY() + y;
		for (uint row = 0; row < _rows.size(); row++) {
			const RowState &state = _rows[row];
			if (!state._widget || !state._widget->isVisible())
				continue;
			const int rowTop = state._widget->getAbsY();
			const int rowBottom = rowTop + state._widget->getHeight();
			if (rowTop <= absoluteY && absoluteY < rowBottom) {
				if (state._corrupted)
					setTooltip(_corruptedTooltip);
				return;
			}
		}
	}

	struct RowState {
		RowState() : _widget(nullptr), _corrupted(false) {
		}

		GUI::Widget *_widget;
		bool _corrupted;
	};

	Common::Array<RowState> _rows;
	Common::U32String _corruptedTooltip;
};

static int scaleSaveDialogValue(const int value) {
	return value > 0 ? static_cast<int>(value * g_gui.getScaleFactor()) : value;
}

static Common::U32String describeSaveIssue(ZoombiniGameState::ZmbSaveIssue issue) {
	switch (issue) {
	case ZoombiniGameState::ZmbSaveIssue::kMagic:
		return _("The state-file signature is invalid.");
	case ZoombiniGameState::ZmbSaveIssue::kPage:
		return _("The saved current-page information is invalid.");
	case ZoombiniGameState::ZmbSaveIssue::kScalarFields:
		return _("One or more saved scalar fields are outside their valid ranges.");
	case ZoombiniGameState::ZmbSaveIssue::kBasecamp1Storage:
		return _("The Basecamp 1 storage data contains invalid fields.");
	case ZoombiniGameState::ZmbSaveIssue::kBasecamp2Storage:
		return _("The Basecamp 2 storage data contains invalid fields.");
	case ZoombiniGameState::ZmbSaveIssue::kTownStorage:
		return _("The Zoombiniville storage data contains invalid fields.");
	case ZoombiniGameState::ZmbSaveIssue::kIslePack:
		return _("The Isle pack contains invalid entries or counters.");
	case ZoombiniGameState::ZmbSaveIssue::kBasecamp1Pack:
		return _("The Basecamp 1 handoff pack contains invalid entries or counters.");
	case ZoombiniGameState::ZmbSaveIssue::kBasecamp2Pack:
		return _("The Basecamp 2 handoff pack contains invalid entries or counters.");
	case ZoombiniGameState::ZmbSaveIssue::kActivePack:
		return _("The active traveling pack contains invalid entries or counters.");
	case ZoombiniGameState::ZmbSaveIssue::kPuzzlePack:
		return _("The saved puzzle party contains too many Zoombinis.");
	case ZoombiniGameState::ZmbSaveIssue::kProgressFields:
		return _("The saved page or Zoombiniville progress fields are invalid.");
	case ZoombiniGameState::ZmbSaveIssue::kRouteLevel:
		return _("A saved route difficulty level is invalid.");
	case ZoombiniGameState::ZmbSaveIssue::kRodMapProgress:
		return _("The saved journey-map progress is contradictory or out of range.");
	case ZoombiniGameState::ZmbSaveIssue::kPerfectCounter:
		return _("A saved route-perfect counter is invalid.");
	case ZoombiniGameState::ZmbSaveIssue::kLogicalLocationCount:
		return _("A logical Basecamp or Zoombiniville population count is invalid.");
	case ZoombiniGameState::ZmbSaveIssue::kIsleActiveCount:
		return _("The Isle and active-pack counts cannot form a valid 625-Zoombini total.");
	case ZoombiniGameState::ZmbSaveIssue::kPhysicalLocationCount:
		return _("The logical location totals do not match the populated storage and shelter snapshots.");
	case ZoombiniGameState::ZmbSaveIssue::kNone:
	default:
		return _("The state file failed structural validation.");
	}
}

static void appendRosterIssue(Common::U32String &description, const Common::U32String &issue) {
	if (!description.empty())
		description += Common::U32String("\n");
	description += issue;
}

static Common::U32String describeRosterIssues(uint32 issues) {
	Common::U32String description;
	if (issues & ZoombiniGameState::kRosterIssueMissing)
		appendRosterIssue(description, _("The roster file is missing although state files exist."));
	if (issues & ZoombiniGameState::kRosterIssueSize)
		appendRosterIssue(description, _("The roster file is not exactly 1606 bytes."));
	if (issues & ZoombiniGameState::kRosterIssueRead)
		appendRosterIssue(description, _("The roster header or available entries could not be read safely."));
	if (issues & ZoombiniGameState::kRosterIssueMagic)
		appendRosterIssue(description, _("The roster-file signature is invalid."));
	if (issues & ZoombiniGameState::kRosterIssueEntryCount)
		appendRosterIssue(description, _("The roster entry count exceeds the complete 32-byte entries available in the file."));
	if (issues & ZoombiniGameState::kRosterIssueNextCounter)
		appendRosterIssue(description, _("The roster's next save-file number is invalid."));
	if (issues & ZoombiniGameState::kRosterIssueSaveName)
		appendRosterIssue(description, _("A roster save name is empty or is not terminated inside its fixed-width field."));
	if (issues & ZoombiniGameState::kRosterIssueFileStem)
		appendRosterIssue(description, _("A roster entry has an invalid ZOOM#### state-file name."));
	if (issues & ZoombiniGameState::kRosterIssueDuplicateFileStem)
		appendRosterIssue(description, _("Multiple roster entries refer to the same state file."));
	if (issues & ZoombiniGameState::kRosterIssueMissingState)
		appendRosterIssue(description, _("A state file referenced by the roster is missing."));
	if (issues & ZoombiniGameState::kRosterIssueOrphanState)
		appendRosterIssue(description, _("One or more state files are not listed in the roster."));
	if (description.empty())
		description = _("The roster file failed structural validation.");
	return description;
}

static void appendBackupStorageDescription(Common::U32String &description,
										   const Common::String &backupFilenamePattern) {
	const Common::Path savePath = ConfMan.getPath("savepath");
	const Common::U32String filenamePattern(backupFilenamePattern, Common::kUtf8);
	if (!savePath.empty() && savePath != "None") {
		const Common::U32String path(savePath.toString(Common::Path::kNativeSeparator), Common::kUtf8);
		description += Common::U32String::format(_("\n\nBackup location: %S\nBackup filename pattern: %S"), path.c_str(), filenamePattern.c_str());
	} else {
		description += Common::U32String::format(_("\n\nBackup location: ScummVM save storage\nBackup filename pattern: %S"), filenamePattern.c_str());
	}
	description += _("\nN is replaced with the first available non-negative number.");
}

} // End of anonymous namespace

ZoombiniSaveNameDialog::ZoombiniSaveNameDialog(const Common::U32String &initialName, bool rename, Common::CodePage codePage)
	: GUI::Dialog(0, 0, 360, 168), _edit(nullptr) {
	Common::U32String title;
	if (rename)
		title = _c("Rename savefile", "zoombini-options");
	else
		title = _c("Save name", "zoombini-options");
	new GUI::StaticTextWidget(this, 12, 10, _w - 24, 24, title, Graphics::kTextAlignStart);
	new GUI::StaticTextWidget(this, 12, 38, _w - 24, 20, false,
							  _c("Save names are limited to 22 bytes.", "zoombini-options"), Graphics::kTextAlignStart,
							  Common::U32String(), GUI::ThemeEngine::kFontStyleNormal, Common::UNK_LANG, false);
	const Common::String codePageName = ZoombiniText::getCodePageName(codePage);
	new GUI::StaticTextWidget(this, 12, 58, _w - 24, 20, false,
							  Common::U32String::format(_c("Only characters representable in %s are allowed.", "zoombini-options"), codePageName.c_str()),
							  Graphics::kTextAlignStart, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal, Common::UNK_LANG, false);
	_edit = new GUI::EditTextWidget(this, 12, 84, _w - 24, 28, false, initialName);
	new GUI::ButtonWidget(this, 12, 126, 150, 28, false, _("OK"), Common::U32String(), GUI::kOKCmd);
	new GUI::ButtonWidget(this, _w - 162, 126, 150, 28, false, _("Cancel"), Common::U32String(), GUI::kCloseCmd);
}

Common::U32String ZoombiniSaveNameDialog::getSaveName() const {
	return _edit->getEditString();
}

void ZoombiniSaveNameDialog::reflowLayout() {
	const int screenWidth = g_system->getOverlayWidth();
	const int screenHeight = g_system->getOverlayHeight();
	_x = MAX<int>(0, (screenWidth - _w) / 2);
	_y = MAX<int>(0, (screenHeight - _h) / 2);
	GUI::Dialog::reflowLayout();
}

void ZoombiniSaveNameDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == GUI::kOKCmd) {
		setResult(GUI::kOKCmd);
		close();
	} else if (cmd == GUI::kCloseCmd) {
		setResult(GUI::kCloseCmd);
		close();
	} else {
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

bool ZoombiniSaveTransfer::importOneSave(const Common::String &target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();

	// I18N: Title of a file chooser for importing one Zoombini save.
	GUI::BrowserDialog browser(_("Select one .TXT or .ZMB save file"), false);
	if (browser.runModal() <= 0)
		return false;

	const Common::FSNode source = browser.getResult();
	if (!source.exists() || source.isDirectory()) {
		GUI::MessageDialog msg(_("The selected item is not a save file."));
		msg.runModal();
		return false;
	}

	const Common::String sourceName = source.getName();
	const size_t extensionPos = sourceName.findLastOf('.');
	if (extensionPos == Common::String::npos) {
		GUI::MessageDialog msg(_("Select a .TXT or .ZMB save file."));
		msg.runModal();
		return false;
	}

	const Common::String extension = sourceName.substr(extensionPos);
	if (!extension.equalsIgnoreCase(".TXT") && !extension.equalsIgnoreCase(".ZMB")) {
		GUI::MessageDialog msg(_("Select a .TXT or .ZMB save file."));
		msg.runModal();
		return false;
	}

	Common::Array<byte> stateBytes;
	Common::SeekableReadStream *rawSource = source.createReadStream();
	Common::SeekableReadStream *sourceStream = Common::wrapCompressedReadStream(rawSource, DisposeAfterUse::YES);
	const bool read = zmbReadAll(sourceStream, stateBytes, static_cast<uint32>(ZoombiniGameState::kStateFileSizeTlcV20));
	delete sourceStream;
	if (!read || !zmbIsValidStateSize(stateBytes.size())) {
		GUI::MessageDialog msg(_("The selected file is not a valid Zoombini save."));
		msg.runModal();
		return false;
	}

	ZmbRosterFile roster;
	Common::InSaveFile *rosterFile = saveFileMan->openForLoading(target + ".WHO");
	if (rosterFile) {
		Common::Array<byte> rosterBytes;
		const bool rosterRead = zmbReadAll(rosterFile, rosterBytes, ZoombiniGameState::kZmbRosterFileSize);
		delete rosterFile;
		if (!rosterRead || !ZoombiniGameState::parseRoster(rosterBytes, roster) || !roster.isSerializedStateValid()) {
			GUI::MessageDialog msg(_("The target has an invalid save roster."));
			msg.runModal();
			return false;
		}
	}
	if (!roster.isSerializedStateValid()) {
		GUI::MessageDialog msg(_("The target has an invalid save roster."));
		msg.runModal();
		return false;
	}

	if (roster.getEntryCapacity() <= roster.getEntryCount()) {
		GUI::MessageDialog msg(_("There is no empty save slot available."));
		msg.runModal();
		return false;
	}

	Common::String suggestedName = sourceName.substr(0, extensionPos);
	if (suggestedName.empty())
		suggestedName = "Imported Save";
	const Common::CodePage codePage = ZoombiniGameState::getSaveNameCodePage(target);
	ZoombiniSaveNameDialog nameDialog(Common::U32String(suggestedName, codePage), false, codePage);
	if (nameDialog.runModal() != GUI::kOKCmd)
		return false;

	const Common::U32String saveName = ZoombiniGameState::trimSaveName(nameDialog.getSaveName());
	byte encodedSaveName[23];
	if (!ZoombiniGameState::encodeRosterSaveName(saveName, codePage, encodedSaveName, ARRAYSIZE(encodedSaveName))) {
		GUI::MessageDialog msg(_("The save name is empty, too long, or cannot be represented in the target code page."));
		msg.runModal();
		return false;
	}

	for (int i = 0; i < roster.getEntryCount(); i++) {
		const Common::U32String existingName = roster.getEntry(i).getSaveName(codePage);
		if (existingName.equals(saveName) ||
			memcmp(roster.getEntry(i)._saveName, encodedSaveName, ARRAYSIZE(encodedSaveName)) == 0) {
			GUI::MessageDialog msg(_("A save with that name already exists."));
			msg.runModal();
			return false;
		}
	}

	const uint16 oldCounter = roster._nextSaveFileNameCounter;
	ZmbRosterEntry entry;
	memcpy(entry._saveName, encodedSaveName, ARRAYSIZE(entry._saveName));
	const int slot = roster.getEntryCount();
	if (!ZoombiniGameState::appendNewSaveEntryForTarget(saveFileMan, target, roster, entry)) {
		roster._nextSaveFileNameCounter = oldCounter;
		GUI::MessageDialog msg(_("Unable to add the imported save to the save roster."));
		msg.runModal();
		return false;
	}
	const Common::String destination = ZoombiniGameState::makeSaveFilename(target, roster.getEntry(slot)._fileName);
	if (destination.empty()) {
		roster.removeEntryAt(slot);
		roster._nextSaveFileNameCounter = oldCounter;
		GUI::MessageDialog msg(_("Unable to add the imported save to the save roster."));
		msg.runModal();
		return false;
	}
	if (!zmbWriteToSaveFile(saveFileMan, destination, stateBytes)) {
		saveFileMan->removeSavefile(destination);
		roster.removeEntryAt(slot);
		roster._nextSaveFileNameCounter = oldCounter;
		GUI::MessageDialog msg(_("Unable to write the imported save file."));
		msg.runModal();
		return false;
	}

	if (!ZoombiniGameState::saveRosterFile(saveFileMan, target + ".WHO", roster)) {
		saveFileMan->removeSavefile(destination);
		roster.removeEntryAt(slot);
		roster._nextSaveFileNameCounter = oldCounter;
		GUI::MessageDialog msg(_("Unable to update the save roster."));
		msg.runModal();
		return false;
	}

	GUI::MessageDialog done(_("The save file was imported."));
	done.runModal();
	return true;
}

ZoombiniSaveManagementDialog::ZoombiniSaveManagementDialog(const Common::String &domain) : GUI::Dialog(0, 0, 1, 1),
																						   _domain(domain),
																						   _saveSelectionGroup(this, kSaveSelectionChangedCmd),
																						   _selectedSaveSlot(-1),
																						   _description(nullptr),
																						   _saveHeader(nullptr),
																						   _saveList(nullptr),
																						   _saveTable(nullptr),
																						   _saveRowCount(0),
																						   _moveUpButton(nullptr),
																						   _moveDownButton(nullptr),
																						   _renameButton(nullptr),
																						   _importButton(nullptr),
																						   _exportButton(nullptr),
																						   _deleteButton(nullptr),
																						   _recoverButton(nullptr),
																						   _compactButton(nullptr) {
	for (int i = 0; i < kMaxSaveRows; i++) {
		_saveSelectionButtons[i] = nullptr;
		_saveSlotLabels[i] = nullptr;
		_saveNameLabels[i] = nullptr;
		_saveVersionLabels[i] = nullptr;
		_saveIsleLabels[i] = nullptr;
		_saveBasecamp1Labels[i] = nullptr;
		_saveBasecamp2Labels[i] = nullptr;
		_saveTownLabels[i] = nullptr;
		_saveActivePackLabels[i] = nullptr;
		_saveFileNames[i].clear();
		_saveRecoverableRows[i] = false;
		_saveIssueRows[i] = static_cast<uint8>(ZoombiniGameState::ZmbSaveIssue::kNone);
	}

	const int margin = kZmbSaveDialogWidthMargin;

	_description = new GUI::StaticTextWidget(this, margin, 8, kZmbSaveTableWidth, kZmbSaveDialogDescriptionHeight,
											 true, _c("Manage savefiles", "zoombini-options"), Graphics::kTextAlignStart);

	_saveHeader = new GUI::ContainerWidget(this, margin, kZmbSaveDialogHeaderTop, kZmbSaveTableWidth, kZmbSaveTableRowHeight, true);
	_saveHeader->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);
	new GUI::StaticTextWidget(_saveHeader, kZmbSaveSlotX, 0, kZmbSaveSlotWidth, kZmbSaveTableRowHeight, true,
							  _c("Slot", "zoombini-options"), Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(_saveHeader, kZmbSaveNameX, 0, kZmbSaveNameWidth, kZmbSaveTableRowHeight, true,
							  _c("Name", "zoombini-options"), Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(_saveHeader, kZmbSaveVersionX, 0, kZmbSaveVersionWidth, kZmbSaveTableRowHeight, true,
							  _c("Ver", "zoombini-options"), Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(_saveHeader, kZmbSaveIsleX, 0, kZmbSaveShelterWidth, kZmbSaveTableRowHeight, true,
							  _c("Isle", "zoombini-options"), Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(_saveHeader, kZmbSaveBasecamp1X, 0, kZmbSaveShelterWidth, kZmbSaveTableRowHeight, true,
							  _c("BC1", "zoombini-options"), Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(_saveHeader, kZmbSaveBasecamp2X, 0, kZmbSaveShelterWidth, kZmbSaveTableRowHeight, true,
							  _c("BC2", "zoombini-options"), Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(_saveHeader, kZmbSaveTownX, 0, kZmbSaveShelterWidth, kZmbSaveTableRowHeight, true,
							  _c("Town", "zoombini-options"), Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(_saveHeader, kZmbSaveActivePackX, 0, kZmbSaveActivePackWidth, kZmbSaveTableRowHeight, true,
							  _c("Active", "zoombini-options"), Graphics::kTextAlignCenter);

	_saveList = new ZoombiniSaveListWidget(this, scaleSaveDialogValue(margin), scaleSaveDialogValue(kZmbSaveDialogListTop),
										   scaleSaveDialogValue(kZmbSaveTableWidth), scaleSaveDialogValue(kZmbSaveDialogListHeight),
										   _c("Corrupted save", "zoombini-options"));
	_saveTable = new GUI::ContainerWidget(_saveList, 0, 0, kZmbSaveTableWidth, 0, true);
	const int fontHeight = MAX<int>(1, static_cast<int>(g_gui.getFontHeight() / g_gui.getScaleFactor()));
	const int selectionHeight = MIN<int>(kZmbSaveTableRowHeight, fontHeight);
	const int selectionOffset = (kZmbSaveTableRowHeight - selectionHeight) / 2;
	for (int i = 0; i < kMaxSaveRows; i++) {
		const int y = i * kZmbSaveTableRowHeight;
		_saveSelectionButtons[i] = new GUI::RadiobuttonWidget(_saveTable, kZmbSaveSelectionX, y + selectionOffset, kZmbSaveSelectionWidth, selectionHeight, true,
															  &_saveSelectionGroup, i, Common::U32String(), Common::U32String());
		_saveSlotLabels[i] = new GUI::StaticTextWidget(_saveTable, kZmbSaveSlotX, y, kZmbSaveSlotWidth, kZmbSaveTableRowHeight, true,
													   Common::U32String(), Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_saveNameLabels[i] = new GUI::StaticTextWidget(_saveTable, kZmbSaveNameX, y, kZmbSaveNameWidth, kZmbSaveTableRowHeight, true,
													   Common::U32String(), Graphics::kTextAlignLeft, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_saveVersionLabels[i] = new GUI::StaticTextWidget(_saveTable, kZmbSaveVersionX, y, kZmbSaveVersionWidth, kZmbSaveTableRowHeight, true,
														  Common::U32String(), Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_saveIsleLabels[i] = new GUI::StaticTextWidget(_saveTable, kZmbSaveIsleX, y, kZmbSaveShelterWidth, kZmbSaveTableRowHeight, true,
													   Common::U32String(), Graphics::kTextAlignLeft, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_saveBasecamp1Labels[i] = new GUI::StaticTextWidget(_saveTable, kZmbSaveBasecamp1X, y, kZmbSaveShelterWidth, kZmbSaveTableRowHeight, true,
															Common::U32String(), Graphics::kTextAlignLeft, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_saveBasecamp2Labels[i] = new GUI::StaticTextWidget(_saveTable, kZmbSaveBasecamp2X, y, kZmbSaveShelterWidth, kZmbSaveTableRowHeight, true,
															Common::U32String(), Graphics::kTextAlignLeft, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_saveTownLabels[i] = new GUI::StaticTextWidget(_saveTable, kZmbSaveTownX, y, kZmbSaveShelterWidth, kZmbSaveTableRowHeight, true,
													   Common::U32String(), Graphics::kTextAlignLeft, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_saveActivePackLabels[i] = new GUI::StaticTextWidget(_saveTable, kZmbSaveActivePackX, y, kZmbSaveActivePackWidth, kZmbSaveTableRowHeight, true,
															 Common::U32String(), Graphics::kTextAlignLeft, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	}

	int actionX = margin;
	_moveUpButton = new GUI::ButtonWidget(this, actionX, kZmbSaveDialogActionButtonsTop,
										  kZmbSaveMoveButtonWidth, kZmbSaveDialogButtonHeight, true,
										  Common::U32String("^"), Common::U32String(), kMoveOneSaveUpCmd);
	_moveUpButton->setTarget(this);
	actionX += kZmbSaveMoveButtonWidth + kZmbSaveButtonGap;
	_moveDownButton = new GUI::ButtonWidget(this, actionX, kZmbSaveDialogActionButtonsTop,
											kZmbSaveMoveButtonWidth, kZmbSaveDialogButtonHeight, true,
											Common::U32String("v"), Common::U32String(), kMoveOneSaveDownCmd);
	_moveDownButton->setTarget(this);
	actionX += kZmbSaveMoveButtonWidth + kZmbSaveButtonGap;
	_renameButton = new GUI::ButtonWidget(this, actionX, kZmbSaveDialogActionButtonsTop,
										  kZmbSaveCommonButtonWidth, kZmbSaveDialogButtonHeight, true,
										  _c("Rename", "zoombini-options"), Common::U32String(), kRenameSaveCmd);
	_renameButton->setTarget(this);
	actionX += kZmbSaveCommonButtonWidth + kZmbSaveButtonGap;
	_importButton = new GUI::ButtonWidget(this, actionX, kZmbSaveDialogActionButtonsTop,
										  kZmbSaveCommonButtonWidth, kZmbSaveDialogButtonHeight, true,
										  _c("Import", "zoombini-options"), Common::U32String(), kImportOneSaveCmd);
	_importButton->setTarget(this);
	actionX += kZmbSaveCommonButtonWidth + kZmbSaveButtonGap;
	_exportButton = new GUI::ButtonWidget(this, actionX, kZmbSaveDialogActionButtonsTop,
										  kZmbSaveCommonButtonWidth, kZmbSaveDialogButtonHeight, true,
										  _c("Export", "zoombini-options"), Common::U32String(), kExportOneSaveCmd);
	_exportButton->setTarget(this);
	actionX += kZmbSaveCommonButtonWidth + kZmbSaveButtonGap;
	_deleteButton = new GUI::ButtonWidget(this, actionX, kZmbSaveDialogActionButtonsTop,
										  kZmbSaveCommonButtonWidth, kZmbSaveDialogButtonHeight, true,
										  _c("Delete", "zoombini-options"), Common::U32String(), kDeleteSaveCmd);
	_deleteButton->setTarget(this);
	actionX += kZmbSaveCommonButtonWidth + kZmbSaveButtonGap;
	_recoverButton = new GUI::ButtonWidget(this, actionX, kZmbSaveDialogActionButtonsTop,
										   kZmbSaveCommonButtonWidth, kZmbSaveDialogButtonHeight, true,
										   _c("Recover", "zoombini-options"), Common::U32String(), kRecoverSaveCmd);
	_recoverButton->setTarget(this);
	actionX += kZmbSaveCommonButtonWidth + kZmbSaveButtonGap;
	_compactButton = new GUI::ButtonWidget(this, actionX, kZmbSaveDialogActionButtonsTop,
										   kZmbSaveTableWidth - actionX, kZmbSaveDialogButtonHeight, true,
										   _c("Slot Compact", "zoombini-options"), Common::U32String(), kCompactSlotsCmd);
	_compactButton->setTarget(this);

	new GUI::ButtonWidget(this, margin + (kZmbSaveTableWidth - 120) / 2, kZmbSaveDialogButtonsTop,
						  120, kZmbSaveDialogButtonHeight, true,
						  _("Close"), Common::U32String(), GUI::kCloseCmd);
}

ZoombiniSaveManagementDialog::~ZoombiniSaveManagementDialog() {
}

void ZoombiniSaveManagementDialog::open() {
	GUI::Dialog::open();
	refreshSaveList();
	_saveList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void ZoombiniSaveManagementDialog::reflowLayout() {
	const int scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 16);
	const int width = scaleSaveDialogValue(kZmbSaveTableWidth + (2 * kZmbSaveDialogWidthMargin)) + scrollbarWidth;
	const int height = scaleSaveDialogValue(kZmbSaveDialogHeight);
	const int screenWidth = g_system->getOverlayWidth();
	const int screenHeight = g_system->getOverlayHeight();

	_x = MAX<int>(0, (screenWidth - width) / 2);
	_y = MAX<int>(0, (screenHeight - height) / 2);
	_w = width;
	_h = height;

	GUI::Dialog::reflowLayout();
}

void ZoombiniSaveManagementDialog::refreshSaveList() {
	const Common::Array<ZoombiniSaveSummary> summaries = ZoombiniGameState::listSaveSummaries(_domain);
	ZoombiniSaveListWidget *saveList = static_cast<ZoombiniSaveListWidget *>(_saveList);
	_rosterIssueFlags = ZoombiniGameState::getRosterIssueFlagsForTarget(_domain);
	_saveCompactionNeeded = _rosterIssueFlags == ZoombiniGameState::kRosterIssueNone &&
							ZoombiniGameState::isSaveCompactionNeededForTarget(_domain);
	Common::U32String compactButtonLabel;
	if (_rosterIssueFlags != ZoombiniGameState::kRosterIssueNone)
		compactButtonLabel = _c("Roster Repair", "zoombini-options");
	else
		compactButtonLabel = _c("Slot Compact", "zoombini-options");
	_compactButton->setLabel(compactButtonLabel);
	_compactButton->markAsDirty();
	_saveRowCount = MIN<int>(summaries.size(), kMaxSaveRows);
	if (_saveRowCount == 0)
		_selectedSaveSlot = -1;
	else
		_selectedSaveSlot = CLIP<int>(_selectedSaveSlot, 0, _saveRowCount - 1);

	// Save names follow the configured language's executable code page.
	const Common::Language language = Common::parseLanguage(ConfMan.get("language", _domain));
	const Common::CodePage saveNameCodePage = ZoombiniText::getExeCodePage(language);

	for (int i = 0; i < kMaxSaveRows; i++) {
		const bool visible = i < _saveRowCount;
		_saveFileNames[i].clear();
		_saveRecoverableRows[i] = false;
		_saveIssueRows[i] = static_cast<uint8>(ZoombiniGameState::ZmbSaveIssue::kNone);
		_saveSelectionButtons[i]->setVisible(visible);
		_saveSlotLabels[i]->setVisible(visible);
		_saveNameLabels[i]->setVisible(visible);
		_saveVersionLabels[i]->setVisible(visible);
		_saveIsleLabels[i]->setVisible(visible);
		_saveBasecamp1Labels[i]->setVisible(visible);
		_saveBasecamp2Labels[i]->setVisible(visible);
		_saveTownLabels[i]->setVisible(visible);
		_saveActivePackLabels[i]->setVisible(visible);
		if (!visible) {
			saveList->setRowState(i, _saveSlotLabels[i], false);
			continue;
		}

		const ZoombiniSaveSummary &summary = summaries[i];
		_saveFileNames[i] = summary._fileName;
		_saveRecoverableRows[i] = summary._stateRecoverable;
		_saveIssueRows[i] = static_cast<uint8>(summary._stateIssue);
		const Common::U32String saveName(summary._saveName, saveNameCodePage);
		Common::U32String slotName = _("?");
		if (!summary._saveFileNumber.empty()) {
			Common::String displayedNumber = summary._saveFileNumber;
			uint strippedZeroCount = 0;
			while (strippedZeroCount < 2 && strippedZeroCount < displayedNumber.size() && displayedNumber[strippedZeroCount] == '0')
				strippedZeroCount += 1;
			if (0 < strippedZeroCount)
				displayedNumber = displayedNumber.substr(strippedZeroCount);
			slotName = Common::U32String(displayedNumber.c_str());
		}

		_saveSlotLabels[i]->setLabel(slotName);
		_saveNameLabels[i]->setLabel(saveName);
		_saveVersionLabels[i]->setLabel(Common::U32String(ZoombiniGameState::getSaveFormatName(summary._saveFormat)));
		if (summary._stateValid) {
			_saveIsleLabels[i]->setLabel(Common::U32String::format("%d", summary._isleCount));
			_saveBasecamp1Labels[i]->setLabel(Common::U32String::format("%d", summary._basecamp1Count));
			_saveBasecamp2Labels[i]->setLabel(Common::U32String::format("%d", summary._basecamp2Count));
			_saveTownLabels[i]->setLabel(Common::U32String::format("%d", summary._townCount));
			_saveActivePackLabels[i]->setLabel(Common::U32String::format("%d", summary._activePackCount));
		} else {
			_saveIsleLabels[i]->setLabel(_("?"));
			_saveBasecamp1Labels[i]->setLabel(_("?"));
			_saveBasecamp2Labels[i]->setLabel(_("?"));
			_saveTownLabels[i]->setLabel(_("?"));
			_saveActivePackLabels[i]->setLabel(_("?"));
		}

		GUI::ThemeEngine::FontColor rowFontColor = GUI::ThemeEngine::kFontColorNormal;
		if (!summary._stateValid)
			rowFontColor = GUI::ThemeEngine::kFontColorOverride;
		saveList->setRowState(i, _saveSlotLabels[i], !summary._stateValid);
		GUI::StaticTextWidget *rowLabels[] = {
			_saveSlotLabels[i],
			_saveNameLabels[i],
			_saveVersionLabels[i],
			_saveIsleLabels[i],
			_saveBasecamp1Labels[i],
			_saveBasecamp2Labels[i],
			_saveTownLabels[i],
			_saveActivePackLabels[i],
		};
		for (uint labelIdx = 0; labelIdx < ARRAYSIZE(rowLabels); labelIdx++) {
			rowLabels[labelIdx]->setFontColor(rowFontColor);
			rowLabels[labelIdx]->markAsDirty();
		}
	}

	_saveSelectionGroup.setValue(_selectedSaveSlot);
	updateSaveTableLayout();
	updateActionButtonState();
}

void ZoombiniSaveManagementDialog::updateSaveTableLayout() {
	_saveTable->setSize(scaleSaveDialogValue(kZmbSaveTableWidth), scaleSaveDialogValue(_saveRowCount * kZmbSaveTableRowHeight));
}

void ZoombiniSaveManagementDialog::deleteSave(int slot) {
	if (slot < 0 || _saveRowCount <= slot)
		return;

	GUI::MessageDialog confirm(_("Delete this saved game?"), _("Yes"), _("No"));
	if (confirm.runModal() != GUI::kMessageOK)
		return;

	bool deleted = false;
	MohawkEngine_Zoombini *vm = dynamic_cast<MohawkEngine_Zoombini *>(g_engine);
	if (vm && ConfMan.getActiveDomainName() == _domain && vm->_state)
		deleted = vm->_state->deleteGameAndShiftRoster(slot);
	else
		deleted = ZoombiniGameState::deleteSaveForTarget(_domain, slot);

	if (!deleted) {
		GUI::MessageDialog failure(_("Unable to delete the selected saved game."));
		failure.runModal();
		return;
	}

	if (_selectedSaveSlot == slot) {
		if (_saveRowCount <= 1)
			_selectedSaveSlot = -1;
		else if (slot + 1 < _saveRowCount)
			_selectedSaveSlot = slot;
		else
			_selectedSaveSlot = slot - 1;
	} else if (slot < _selectedSaveSlot) {
		_selectedSaveSlot -= 1;
	}

	refreshSaveList();
	_saveList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void ZoombiniSaveManagementDialog::renameSave(int slot) {
	if (slot < 0 || _saveRowCount <= slot)
		return;

	const Common::CodePage codePage = ZoombiniGameState::getSaveNameCodePage(_domain);
	ZoombiniSaveNameDialog dialog(_saveNameLabels[slot]->getLabel(), true, codePage);
	if (dialog.runModal() != GUI::kOKCmd)
		return;

	const Common::U32String saveName = ZoombiniGameState::trimSaveName(dialog.getSaveName());
	if (saveName.empty()) {
		GUI::MessageDialog failure(_("The save name cannot be empty."));
		failure.runModal();
		return;
	}

	bool renamed = false;
	MohawkEngine_Zoombini *vm = dynamic_cast<MohawkEngine_Zoombini *>(g_engine);
	if (vm && ConfMan.getActiveDomainName() == _domain && vm->_state)
		renamed = vm->_state->renameGame(slot, saveName);
	else
		renamed = ZoombiniGameState::renameSaveForTarget(_domain, slot, saveName);

	if (!renamed) {
		GUI::MessageDialog failure(_("Unable to rename the selected saved game."));
		failure.runModal();
		return;
	}

	refreshSaveList();
	_saveList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void ZoombiniSaveManagementDialog::exportOneSave(int slot) {
	ZoombiniSaveTransfer::exportOneSave(_domain, slot);
}

void ZoombiniSaveManagementDialog::moveSave(int slot, int destinationSlot) {
	if (slot < 0 || _saveRowCount <= slot || destinationSlot < 0 || _saveRowCount <= destinationSlot ||
		(slot + 1 != destinationSlot && destinationSlot + 1 != slot))
		return;

	bool moved = false;
	MohawkEngine_Zoombini *vm = dynamic_cast<MohawkEngine_Zoombini *>(g_engine);
	if (vm && ConfMan.getActiveDomainName() == _domain && vm->_state)
		moved = vm->_state->moveGame(slot, destinationSlot);
	else
		moved = ZoombiniGameState::moveSaveForTarget(_domain, slot, destinationSlot);

	if (!moved) // NO-OP
		return;

	_selectedSaveSlot = destinationSlot;
	refreshSaveList();
	_saveList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void ZoombiniSaveManagementDialog::recoverSave(int slot) {
	if (slot < 0 || _saveRowCount <= slot || !_saveRecoverableRows[slot])
		return;

	Common::U32String prompt = _("The selected save is corrupted.\n\nDetected problem:\n");
	prompt += describeSaveIssue(static_cast<ZoombiniGameState::ZmbSaveIssue>(_saveIssueRows[slot]));
	prompt += _("\n\nRecovery is experimental and best-effort. Some progress may be changed or lost even if the recovered save "
				"loads successfully. A backup of the original save will be retained.");
	Common::String backupFilenamePattern = _saveFileNames[slot];
	if (backupFilenamePattern.empty())
		backupFilenamePattern = Common::String::format("%s-####.ZMB", _domain.c_str());
	backupFilenamePattern += "-recovery-N.bak";
	appendBackupStorageDescription(prompt, backupFilenamePattern);
	prompt += _("\n\nContinue?");
	GUI::MessageDialog confirm(prompt, _("Recover"), _("Cancel"));
	if (confirm.runModal() != GUI::kMessageOK)
		return;

	const ZoombiniGameState::ZmbSaveRecoverResult result = ZoombiniGameState::recoverSaveForTarget(_domain, slot);
	if (result == ZoombiniGameState::ZmbSaveRecoverResult::kSuccess) {
		refreshSaveList();
		_saveList->reflowLayout();
		g_gui.scheduleTopDialogRedraw();
		return;
	}

	Common::U32String message = _("Unable to recover the selected saved game.");
	if (result == ZoombiniGameState::ZmbSaveRecoverResult::kRollbackFailed)
		message = _("Recovery failed, and the original saved game could not be restored. A backup file was retained.");
	GUI::MessageDialog failure(message);
	failure.runModal();
}

void ZoombiniSaveManagementDialog::compactSaves() {
	if (_rosterIssueFlags != ZoombiniGameState::kRosterIssueNone) {
		Common::U32String prompt = _("The save roster is corrupted.\n\nDetected problems:\n");
		prompt += describeRosterIssues(_rosterIssueFlags);
		prompt += _("\n\nRoster recovery is experimental and best-effort. Save names or ordering may change, and state files that "
					"cannot be matched will receive recovered names. A backup of the original roster will be retained when it exists.");
		appendBackupStorageDescription(prompt, Common::String::format("%s-roster-recovery-N.bak", _domain.c_str()));
		prompt += _("\n\nContinue?");
		GUI::MessageDialog confirm(prompt, _c("Roster Repair", "zoombini-options"), _("Cancel"));
		if (confirm.runModal() != GUI::kMessageOK)
			return;

		MohawkEngine_Zoombini *vm = dynamic_cast<MohawkEngine_Zoombini *>(g_engine);
		const ZoombiniGameState::ZmbRosterRecoverResult result = ZoombiniGameState::recoverRosterForTarget(_domain);
		Common::U32String message;
		switch (result) {
		case ZoombiniGameState::ZmbRosterRecoverResult::kSuccess:
			if (vm && ConfMan.getActiveDomainName() == _domain && vm->_state)
				vm->_state->loadRoster();
			refreshSaveList();
			_saveList->reflowLayout();
			g_gui.scheduleTopDialogRedraw();
			message = _("The save roster was recovered. State files were not modified.");
			break;
		case ZoombiniGameState::ZmbRosterRecoverResult::kNotNeeded:
			refreshSaveList();
			return;
		case ZoombiniGameState::ZmbRosterRecoverResult::kBackupFailed:
			message = _("Unable to back up the corrupted save roster. No recovery was performed.");
			break;
		case ZoombiniGameState::ZmbRosterRecoverResult::kWriteFailed:
			message = _("Unable to write a recovered save roster. The original roster was restored when possible.");
			break;
		case ZoombiniGameState::ZmbRosterRecoverResult::kRollbackFailed:
			message = _("Roster recovery failed, and the original roster could not be restored. A backup file was retained.");
			break;
		}

		GUI::MessageDialog status(message);
		status.runModal();
		return;
	}

	GUI::MessageDialog confirm(_("This will remove gaps, repair duplicate save file numbers using the "
								 "surviving save data, and reset the next file number. Continue?"),
							   _("Yes"), _("No"));
	if (confirm.runModal() != GUI::kMessageOK)
		return;

	MohawkEngine_Zoombini *vm = dynamic_cast<MohawkEngine_Zoombini *>(g_engine);
	const ZoombiniGameState::ZmbSaveCompactResult result = ZoombiniGameState::compactSaveFilesForTarget(_domain);

	Common::U32String message;
	switch (result) {
	case ZoombiniGameState::ZmbSaveCompactResult::kSuccess:
		if (vm && ConfMan.getActiveDomainName() == _domain && vm->_state)
			vm->_state->loadRoster();
		refreshSaveList();
		_saveList->reflowLayout();
		g_gui.scheduleTopDialogRedraw();
		message = _("The save file numbers and roster were repaired.");
		break;
	case ZoombiniGameState::ZmbSaveCompactResult::kNoChanges:
		return;
	case ZoombiniGameState::ZmbSaveCompactResult::kInvalidRoster:
		message = _("The target has an invalid save roster.");
		break;
	case ZoombiniGameState::ZmbSaveCompactResult::kMissingSaveFile:
		message = _("A save file listed in the roster is missing.");
		break;
	case ZoombiniGameState::ZmbSaveCompactResult::kFileNameConflict:
		message = _("A save file outside the roster uses a compacted file number.");
		break;
	case ZoombiniGameState::ZmbSaveCompactResult::kFileMoveFailed:
		message = _("Unable to renumber the save files.");
		break;
	case ZoombiniGameState::ZmbSaveCompactResult::kRosterWriteFailed:
		message = _("Unable to update the save roster.");
		break;
	case ZoombiniGameState::ZmbSaveCompactResult::kRollbackFailed:
		message = _("Compaction failed, and the original save file names could not be fully restored.");
		break;
	}

	GUI::MessageDialog status(message);
	status.runModal();
}

void ZoombiniSaveManagementDialog::updateActionButtonState() {
	if (!_moveUpButton || !_moveDownButton || !_renameButton || !_importButton || !_exportButton || !_deleteButton || !_recoverButton || !_compactButton)
		return;

	const bool hasSelection = 0 <= _selectedSaveSlot && _selectedSaveSlot < _saveRowCount;
	_moveUpButton->setEnabled(hasSelection && 0 < _selectedSaveSlot);
	_moveDownButton->setEnabled(hasSelection && _selectedSaveSlot + 1 < _saveRowCount);
	_renameButton->setEnabled(hasSelection);
	_importButton->setEnabled(true);
	_exportButton->setEnabled(hasSelection);
	_deleteButton->setEnabled(hasSelection);
	_recoverButton->setEnabled(hasSelection && _saveRecoverableRows[_selectedSaveSlot]);
	_compactButton->setEnabled(_rosterIssueFlags != ZoombiniGameState::kRosterIssueNone || _saveCompactionNeeded);
}

void ZoombiniSaveManagementDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kSaveSelectionChangedCmd:
		if (data < static_cast<uint32>(_saveRowCount))
			_selectedSaveSlot = static_cast<int>(data);
		else
			_selectedSaveSlot = -1;
		updateActionButtonState();
		break;
	case kMoveOneSaveUpCmd:
		if (0 < _selectedSaveSlot)
			moveSave(_selectedSaveSlot, _selectedSaveSlot - 1);
		break;
	case kMoveOneSaveDownCmd:
		if (0 <= _selectedSaveSlot && _selectedSaveSlot + 1 < _saveRowCount)
			moveSave(_selectedSaveSlot, _selectedSaveSlot + 1);
		break;
	case kDeleteSaveCmd:
		deleteSave(_selectedSaveSlot);
		break;
	case kRenameSaveCmd:
		renameSave(_selectedSaveSlot);
		break;
	case kExportOneSaveCmd:
		exportOneSave(_selectedSaveSlot);
		break;
	case kImportOneSaveCmd: {
		const int selectedSaveSlot = _selectedSaveSlot;
		const bool hadSelection = 0 <= selectedSaveSlot && selectedSaveSlot < _saveRowCount;
		if (ZoombiniSaveTransfer::importOneSave(_domain)) {
			MohawkEngine_Zoombini *vm = dynamic_cast<MohawkEngine_Zoombini *>(g_engine);
			if (vm && ConfMan.getActiveDomainName() == _domain && vm->_state)
				vm->_state->loadRoster();
			if (hadSelection)
				_selectedSaveSlot = selectedSaveSlot;
			refreshSaveList();
			_saveList->reflowLayout();
			g_gui.scheduleTopDialogRedraw();
		}
		break;
	}
	case kRecoverSaveCmd:
		recoverSave(_selectedSaveSlot);
		break;
	case kCompactSlotsCmd:
		compactSaves();
		break;
	default:
		GUI::Dialog::handleCommand(sender, cmd, data);
		break;
	}
}

ZoombiniOptionsWidget::ZoombiniOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain) : OptionsContainerWidget(boss, name, "ZoombiniEngineOptionsDialog", domain),
																															   _saveFilesHeader(nullptr),
																															   _importSavesButton(nullptr),
																															   _exportSavesButton(nullptr),
																															   _manageSavesButton(nullptr),
																															   _resetButton(nullptr) {
	// The v1.x-only MIDI settings are meaningless for the TLC v2.0 rebuild,
	// which contains no MIDI resources. Only the v2.0 detection entry carries
	// GAMEOPTION_ZMB_V20, so treat every other (older, unflagged) target as v1.x.
	Common::String guiOptions = ConfMan.get("guioptions", _domain);
	_isV1x = !checkGameGUIOption(GAMEOPTION_ZMB_V20, guiOptions);

	_useMacMidiCheckbox = nullptr;
	_fixHotelMidiHaltBugCheckbox = nullptr;
	_fixCavesL4MidiSilentBugCheckbox = nullptr;

	GUI::StaticTextWidget *text;

	new ZoombiniSeparatorWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.SaveFilesSeparator");
	_saveFilesHeader = new GUI::StaticTextWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.SaveFilesHeader",
												 _c("Manage savefiles", "zoombini-options"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	_saveFilesHeader->setAlign(Graphics::TextAlign::kTextAlignStart);

	_importSavesButton = new GUI::ButtonWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.ImportSaves",
											   _c("Import Saves", "zoombini-options"),
											   _("Copy the original game's ZOOMBINI.WHO roster and ZOOM####.TXT saves from a directory into this ScummVM game."),
											   kImportSavesCmd);
	_importSavesButton->setTarget(this);

	_exportSavesButton = new GUI::ButtonWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.ExportSaves",
											   _c("Export Saves", "zoombini-options"),
											   _("Copy this ScummVM game's saves into the original game's ZOOMBINI.WHO / ZOOM####.TXT layout."),
											   kExportSavesCmd);
	_exportSavesButton->setTarget(this);

	_manageSavesButton = new GUI::ButtonWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.ManageSaves",
											   _("Manage"),
											   Common::U32String(),
											   kManageSavesCmd);
	_manageSavesButton->setTarget(this);

	new ZoombiniSeparatorWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.BugFixesSeparator");
	text = new GUI::StaticTextWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.BugFixes",
									 _c("Bug fixes", "zoombini-options"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	text->setAlign(Graphics::TextAlign::kTextAlignStart);

	_audioPopFixCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.AudioDiscontinuityFix",
												   _("Fix audio pops/clicks"),
												   _("Reduces audible pops at the end of some sound effects."));

	_fixFleensTreeDescendFeetBugCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.FixFleensTreeDescendFeetBug",
																   _("Fix 'Fleens!' tree-descending Zoombini feet display bug"),
																   _("Corrects the malformed one-frame roller-skate pose when a Zoombini descends from the tree. Turn this off to reproduce the original behavior."));

	if (_isV1x) {
		_fixHotelMidiHaltBugCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.FixHotelMidiHaltBug",
															   _("Fix 'Hotel Dimensia' MIDI background music halt bug"),
															   _("Keeps MIDI music playing when a Zoombini is dropped in 'Hotel Dimensia'. Turn this off to reproduce the original halt."));

		_fixCavesL4MidiSilentBugCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.FixCavesL4MidiSilentBug",
																   _("Fix 'The Lion's Lair' missing Level 4 background MIDI bug"),
																   _("Plays the level 4 MIDI resource that the original engine left unused. Turn this off to reproduce the original silent level 4 behavior."));
	}

	new ZoombiniSeparatorWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.EnhancementsSeparator");
	text = new GUI::StaticTextWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.Enhancements",
									 _c("Enhancements", "zoombini-options"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	text->setAlign(Graphics::TextAlign::kTextAlignStart);

	_useAccurate60FPSCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.UseAccurate60FPS",
														_("Use accurate 60FPS tick rate"),
														_("Uses exact 60FPS timing. Turn this off to use the original engine's 17ms integer tick."));

	_enhancedKbdShortcutsCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.EnhancedKbdShortcuts",
															_("Enable enhanced keyboard shortcuts"),
															_("Enables some ScummVM-only keyboard shortcuts for quality of life improvements."));

	_showRemappedOptionDialogShortcutsCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.ShowRemappedOptionDialogShortcuts",
																		 _("Show remapped shortcuts in the in-game Options dialog"),
																		 _("Shows the current ScummVM key mappings in the in-game Options dialog instead of the original shortcut labels."));

	new ZoombiniSeparatorWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.AdjustmentsSeparator");
	text = new GUI::StaticTextWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.Adjustments",
									 _c("Adjustments", "zoombini-options"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	text->setAlign(Graphics::TextAlign::kTextAlignStart);

	_brightenPaletteCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.BrightenPalette",
													   _("Brighten palette (original behavior)"),
													   _("Applies the brightness adjustment to palettes as the original engine does."));

	_originalPrngCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.OriginalPRNG",
													_("Use original PRNG (requires restart)"),
													_("Uses the original engine's pseudo-random number generator instead of ScummVM's default. Changes take effect after restarting the game."));

	_colorBlindModeCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.ColorBlindMode",
													  _("Color Blind Mode (Experimental)"), _("Experimental color blind assistance"));

	_alwaysMazePlayCelebrationSfxCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.MazeAlwaysPlayCelebrationSfx",
																	_("Bubblewonder Abyss: always play Zoombini celebration SFX after a landing"),
																	_("Always plays a Zoombini's celebration SFX after a landing, which was played only on some special scenarios."));

	_alwaysTownPlayMemorialSfxCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.TownAlwaysPlayMemorialSfx",
																 _("Zoombiniville: always play memorial card SFX"),
																 _("Immediately plays button SFX when opening a memorial card. Turning this off makes the BGM or narration suppress it."));

	_mazeRestoreUnusedL4LayoutCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.MazeRestoreUnusedL4Layout",
																 _("Bubblewonder Abyss: restore unused level-4 layout"),
																 _("Restores the unused level 4 maze layout which was unused in original engine to the layout rotation. Warning: some runs on this layout cannot save all 16 Zoombinis."));

	_mazeRandomizeInitialLayoutCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.MazeRandomizeInitialLayout",
																  _("Bubblewonder Abyss: randomize initial maze layout (requires restart)"),
																  _("Randomly chooses the first maze layout when starting the game. Levels 1-3 use either the base or alternative layout. Level 4 also includes the restored layout when its option is enabled. Changes take effect after restarting the game."));

	_ferryHighlightTraitMatchCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.FerryHighlightTraitMatch",
																_("Ferryboat: Restore unused zoombini trait match highlight"),
																_("Force-enables the dormant trait-match highlight from the original engine. When enabled, matching traits are highlighted after accepted placements."));

	if (_isV1x) {
		// v1.x hybrid Mac/PC discs ship two device-profile MIDI archives with the
		// same tMID IDs: MIDIMPC.MHK (the Windows/MPC soundtrack the original engine
		// used) and MIDIMAC.MHK (the Macintosh soundtrack). When enabled, v1.x pages
		// load MIDIMAC.MHK instead. Ignored by the TLC v2.0 rebuild (no MIDI).
		_useMacMidiCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.UseMacMidi",
													  _("Use Macintosh MIDI soundtrack"),
													  _("Plays the Macintosh-authored MIDI songs instead of the Windows songs. Both are the same tunes arranged for different synthesizers."));
	}

	new ZoombiniSeparatorWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.ResetOptionsSeparator");
	text = new GUI::StaticTextWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.ResetOptions",
									 _c("Reset options", "zoombini-options"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	text->setAlign(Graphics::TextAlign::kTextAlignStart);

	_resetButton = new GUI::ButtonWidget(widgetsBoss(), "ZoombiniEngineOptionsDialog.Reset",
										 _("Reset"), Common::U32String(), kResetOptionsCmd);
	_resetButton->setTarget(this);
}

ZoombiniOptionsWidget::~ZoombiniOptionsWidget() {
}

void ZoombiniOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical)
		.addPadding(0, 0, 0, 0)
		.addSpace(10)
		.addWidget("SaveFilesSeparator", "", -1, 2)
		.addWidget("SaveFilesHeader", "OptionsLabel")
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
		.addPadding(0, 0, 0, 0)
		.addWidget("ImportSaves", "Button")
		.addWidget("ExportSaves", "Button")
		.addWidget("ManageSaves", "Button")
		.closeLayout()
		.addSpace(10)
		.addWidget("BugFixesSeparator", "", -1, 2)
		.addWidget("BugFixes", "OptionsLabel")
		.addWidget("AudioDiscontinuityFix", "Checkbox")
		.addWidget("FixFleensTreeDescendFeetBug", "Checkbox");

	if (_isV1x) {
		layouts
			.addWidget("FixHotelMidiHaltBug", "Checkbox")
			.addWidget("FixCavesL4MidiSilentBug", "Checkbox");
	}

	layouts
		.addSpace(10)
		.addWidget("EnhancementsSeparator", "", -1, 2)
		.addWidget("Enhancements", "OptionsLabel")
		.addWidget("UseAccurate60FPS", "Checkbox")
		.addWidget("EnhancedKbdShortcuts", "Checkbox")
		.addWidget("ShowRemappedOptionDialogShortcuts", "Checkbox")
		.addSpace(10)
		.addWidget("AdjustmentsSeparator", "", -1, 2)
		.addWidget("Adjustments", "OptionsLabel")
		.addWidget("BrightenPalette", "Checkbox")
		.addWidget("OriginalPRNG", "Checkbox")
		.addWidget("ColorBlindMode", "Checkbox")
		.addWidget("MazeAlwaysPlayCelebrationSfx", "Checkbox")
		.addWidget("TownAlwaysPlayMemorialSfx", "Checkbox")
		.addWidget("MazeRestoreUnusedL4Layout", "Checkbox")
		.addWidget("MazeRandomizeInitialLayout", "Checkbox")
		.addWidget("FerryHighlightTraitMatch", "Checkbox");

	if (_isV1x) {
		layouts.addWidget("UseMacMidi", "Checkbox");
	}

	layouts
		.addSpace(10)
		.addWidget("ResetOptionsSeparator", "", -1, 2)
		.addWidget("ResetOptions", "OptionsLabel")
		.addWidget("Reset", "Button")
		.closeLayout()
		.closeDialog();
}

void ZoombiniOptionsWidget::resetToDefaults() {
	_audioPopFixCheckbox->setState(true);
	_fixFleensTreeDescendFeetBugCheckbox->setState(true);
	if (_fixHotelMidiHaltBugCheckbox)
		_fixHotelMidiHaltBugCheckbox->setState(true);
	if (_fixCavesL4MidiSilentBugCheckbox)
		_fixCavesL4MidiSilentBugCheckbox->setState(true);
	_useAccurate60FPSCheckbox->setState(true);
	_enhancedKbdShortcutsCheckbox->setState(true);
	_showRemappedOptionDialogShortcutsCheckbox->setState(true);
	_brightenPaletteCheckbox->setState(true);
	_originalPrngCheckbox->setState(true);
	_colorBlindModeCheckbox->setState(false);
	_alwaysMazePlayCelebrationSfxCheckbox->setState(false);
	_alwaysTownPlayMemorialSfxCheckbox->setState(true);
	_mazeRestoreUnusedL4LayoutCheckbox->setState(false);
	_mazeRandomizeInitialLayoutCheckbox->setState(false);
	_ferryHighlightTraitMatchCheckbox->setState(false);
	if (_useMacMidiCheckbox)
		_useMacMidiCheckbox->setState(false);
}

void ZoombiniOptionsWidget::load() {
	_audioPopFixCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionFixAudioPops, _domain));
	_fixFleensTreeDescendFeetBugCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionFixFleensTreeDescendFeetBug, _domain));
	if (_fixHotelMidiHaltBugCheckbox)
		_fixHotelMidiHaltBugCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionFixHotelMidiHaltBug, _domain));
	if (_fixCavesL4MidiSilentBugCheckbox)
		_fixCavesL4MidiSilentBugCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionFixCavesL4MidiSilentBug, _domain));
	_useAccurate60FPSCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionUseAccurate60FPS, _domain));
	_enhancedKbdShortcutsCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionEnhancedKbdShortcuts, _domain));
	_showRemappedOptionDialogShortcutsCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionShowRemappedOptionDialogShortcuts, _domain));
	_brightenPaletteCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionBrightenPalette, _domain));
	_originalPrngCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionOriginalPRNG, _domain));
	_colorBlindModeCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionColorBlindMode, _domain));
	_alwaysMazePlayCelebrationSfxCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionMazeAlwaysPlayCelebrationSfx, _domain));
	_alwaysTownPlayMemorialSfxCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionTownAlwaysPlayMemorialSfx, _domain));
	_mazeRestoreUnusedL4LayoutCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionMazeRestoreUnusedL4Layout, _domain));
	_mazeRandomizeInitialLayoutCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionMazeRandomizeInitialLayout, _domain));
	_ferryHighlightTraitMatchCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionFerryHighlightTraitMatch, _domain));
	if (_useMacMidiCheckbox)
		_useMacMidiCheckbox->setState(ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionUseMacMidi, _domain));
}

bool ZoombiniOptionsWidget::save() {
	bool originalPrngChanged = ConfMan.getBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionOriginalPRNG, _domain) != _originalPrngCheckbox->getState();

	ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionFixAudioPops, _audioPopFixCheckbox->getState(), _domain);
	ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionFixFleensTreeDescendFeetBug, _fixFleensTreeDescendFeetBugCheckbox->getState(), _domain);
	if (_fixHotelMidiHaltBugCheckbox)
		ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionFixHotelMidiHaltBug, _fixHotelMidiHaltBugCheckbox->getState(), _domain);
	if (_fixCavesL4MidiSilentBugCheckbox)
		ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionFixCavesL4MidiSilentBug, _fixCavesL4MidiSilentBugCheckbox->getState(), _domain);
	ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionUseAccurate60FPS, _useAccurate60FPSCheckbox->getState(), _domain);
	ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionEnhancedKbdShortcuts, _enhancedKbdShortcutsCheckbox->getState(), _domain);
	ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionShowRemappedOptionDialogShortcuts,
					_showRemappedOptionDialogShortcutsCheckbox->getState(), _domain);
	ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionBrightenPalette, _brightenPaletteCheckbox->getState(), _domain);
	ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionOriginalPRNG, _originalPrngCheckbox->getState(), _domain);
	ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionColorBlindMode, _colorBlindModeCheckbox->getState(), _domain);
	ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionMazeAlwaysPlayCelebrationSfx, _alwaysMazePlayCelebrationSfxCheckbox->getState(), _domain);
	ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionTownAlwaysPlayMemorialSfx, _alwaysTownPlayMemorialSfxCheckbox->getState(), _domain);
	ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionMazeRestoreUnusedL4Layout, _mazeRestoreUnusedL4LayoutCheckbox->getState(), _domain);
	ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionMazeRandomizeInitialLayout, _mazeRandomizeInitialLayoutCheckbox->getState(), _domain);
	ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionFerryHighlightTraitMatch, _ferryHighlightTraitMatchCheckbox->getState(), _domain);
	if (_useMacMidiCheckbox)
		ConfMan.setBool(Mohawk::MohawkMetaEngine_Zoombini::kOptionUseMacMidi, _useMacMidiCheckbox->getState(), _domain);
	if (originalPrngChanged && g_engine) {
		GUI::MessageDialog dialog(_("The PRNG option change will take effect after restarting the game."));
		dialog.runModal();
	}
	return true;
}

void ZoombiniOptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kImportSavesCmd:
		ZoombiniSaveTransfer::importFromOriginalFolder(_domain);
		break;
	case kExportSavesCmd:
		ZoombiniSaveTransfer::exportToOriginalFolder(_domain);
		break;
	case kManageSavesCmd: {
		ZoombiniSaveManagementDialog dialog(_domain);
		dialog.runModal();
		break;
	}
	case kResetOptionsCmd:
		resetToDefaults();
		break;
	default:
		GUI::OptionsContainerWidget::handleCommand(sender, cmd, data);
		break;
	}
}

} // End of namespace Mohawk
