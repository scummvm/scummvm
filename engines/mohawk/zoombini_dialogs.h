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

#ifndef MOHAWK_ZOOMBINI_DIALOGS_H
#define MOHAWK_ZOOMBINI_DIALOGS_H

#include "common/str.h"
#include "gui/dialog.h"
#include "gui/widget.h"

namespace GUI {
class ButtonWidget;
class CheckboxWidget;
class CommandSender;
class ContainerWidget;
class EditTextWidget;
class RadiobuttonGroup;
class RadiobuttonWidget;
class ScrollContainerWidget;
class StaticTextWidget;
class ThemeEval;
} // namespace GUI

namespace Mohawk {

/** Modal dialog for entering or renaming a Zoombini save name. */
class ZoombiniSaveNameDialog : public GUI::Dialog {
public:
	/** Create a save-name dialog with the target's save-name code page. */
	ZoombiniSaveNameDialog(const Common::U32String &initialName, bool rename, Common::CodePage codePage);

	/** Return the text entered by the user. */
	Common::U32String getSaveName() const;

	void reflowLayout() override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	GUI::EditTextWidget *_edit;
};

/** Modal ScummVM dialog for managing Logical Journey of the Zoombini saves. */
class ZoombiniSaveManagementDialog : public GUI::Dialog {
public:
	/** Create the save-management dialog for a configuration target. */
	ZoombiniSaveManagementDialog(const Common::String &domain);
	/** Release save-management widgets. */
	~ZoombiniSaveManagementDialog() override;

	void open() override;
	void reflowLayout() override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	static constexpr int kZmbSaveMoveButtonWidth = 22;
	static constexpr int kZmbSaveCommonButtonWidth = 61;
	static constexpr int kZmbSaveButtonGap = 2;
	static constexpr int kZmbSaveSelectionX = 0;
	static constexpr int kZmbSaveSelectionWidth = 24;
	static constexpr int kZmbSaveSlotX = kZmbSaveSelectionX + kZmbSaveSelectionWidth;
	static constexpr int kZmbSaveSlotWidth = 32;
	static constexpr int kZmbSaveNameX = kZmbSaveSlotX + kZmbSaveSlotWidth;
	static constexpr int kZmbSaveNameWidth = 88;
	static constexpr int kZmbSaveVersionX = kZmbSaveNameX + kZmbSaveNameWidth;
	static constexpr int kZmbSaveVersionWidth = 50;
	static constexpr int kZmbSaveIsleX = kZmbSaveVersionX + kZmbSaveVersionWidth;
	static constexpr int kZmbSaveShelterWidth = 50;
	static constexpr int kZmbSaveBasecamp1X = kZmbSaveIsleX + kZmbSaveShelterWidth;
	static constexpr int kZmbSaveBasecamp2X = kZmbSaveBasecamp1X + kZmbSaveShelterWidth;
	static constexpr int kZmbSaveTownX = kZmbSaveBasecamp2X + kZmbSaveShelterWidth;
	static constexpr int kZmbSaveActivePackX = kZmbSaveTownX + kZmbSaveShelterWidth;
	static constexpr int kZmbSaveActivePackWidth = 50;
	static constexpr int kZmbSaveTableWidth = kZmbSaveActivePackX + kZmbSaveActivePackWidth;
	static constexpr int kZmbSaveTableRowHeight = 24;
	static constexpr int kZmbSaveDialogWidthMargin = 12;
	static constexpr int kZmbSaveDialogDescriptionHeight = 24;
	static constexpr int kZmbSaveDialogActionButtonsTop = 34;
	static constexpr int kZmbSaveDialogHeaderTop = 68;
	static constexpr int kZmbSaveDialogListTop = 92;
	static constexpr int kZmbSaveDialogListHeight = 196;
	static constexpr int kZmbSaveDialogButtonsTop = 300;
	static constexpr int kZmbSaveDialogButtonHeight = 28;
	static constexpr int kZmbSaveDialogHeight = 344;

	static constexpr int kMaxSaveRows = 50;

	enum {
		kSaveSelectionChangedCmd = 'zSel',
		kMoveOneSaveUpCmd = 'zUp1',
		kMoveOneSaveDownCmd = 'zDn1',
		kRenameSaveCmd = 'zRen',
		kDeleteSaveCmd = 'zDel',
		kExportOneSaveCmd = 'zEx1',
		kImportOneSaveCmd = 'zIm1',
		kRecoverSaveCmd = 'zRec',
		kCompactSlotsCmd = 'zCmp',
	};

	/** Refresh save names and metadata shown in the table. */
	void refreshSaveList();
	/** Recalculate the save-table columns and row geometry. */
	void updateSaveTableLayout();
	/** Delete the selected save slot after command confirmation. */
	void deleteSave(int slot);
	/** Rename the selected save slot after editing its display name. */
	void renameSave(int slot);
	/** Export the selected save slot as an original-engine TXT file. */
	void exportOneSave(int slot);
	/** Move one save entry to an adjacent roster slot. */
	void moveSave(int slot, int destinationSlot);
	/** Recover the selected corrupted state file. */
	void recoverSave(int slot);
	/** Repair a corrupted roster, or compact valid save-file stems. */
	void compactSaves();
	/** Update common save-management buttons for the current row selection. */
	void updateActionButtonState();

	/** Configuration manager domain whose saves are being managed. */
	Common::String _domain;
	/** Exclusive selection group for the save-slot cells. */
	GUI::RadiobuttonGroup _saveSelectionGroup;
	/** Zero-based slot selected for the bottom move controls, or -1. */
	int _selectedSaveSlot;
	/** Dialog description shown above the save table. */
	GUI::StaticTextWidget *_description;
	/** Fixed-column save table header. */
	GUI::ContainerWidget *_saveHeader;
	/** Scroll container holding save rows. */
	GUI::ScrollContainerWidget *_saveList;
	/** Container holding save-row widgets. */
	GUI::ContainerWidget *_saveTable;
	/** Number of save rows currently populated in the table. */
	int _saveRowCount;
	/** Whether Slot Compact is available and would change the current target. */
	bool _saveCompactionNeeded = false;
	/** Structural problems that make the current target's roster recoverable. */
	uint32 _rosterIssueFlags = 0;
	/** Common button that moves the selected save one slot upward. */
	GUI::ButtonWidget *_moveUpButton;
	/** Common button that moves the selected save one slot downward. */
	GUI::ButtonWidget *_moveDownButton;
	/** Common button that renames the selected save. */
	GUI::ButtonWidget *_renameButton;
	/** Common button that imports one save. */
	GUI::ButtonWidget *_importButton;
	/** Common button that exports the selected save. */
	GUI::ButtonWidget *_exportButton;
	/** Common button that deletes the selected save. */
	GUI::ButtonWidget *_deleteButton;
	/** Common button that recovers the selected corrupted save. */
	GUI::ButtonWidget *_recoverButton;
	/** Common button that repairs a roster or compacts valid save-file stems. */
	GUI::ButtonWidget *_compactButton;
	/** Uncaptioned save-row selection controls. */
	GUI::RadiobuttonWidget *_saveSelectionButtons[kMaxSaveRows];
	/** Save-file number labels. */
	GUI::StaticTextWidget *_saveSlotLabels[kMaxSaveRows];
	/** Save-name labels. */
	GUI::StaticTextWidget *_saveNameLabels[kMaxSaveRows];
	/** Serialized save-format labels. */
	GUI::StaticTextWidget *_saveVersionLabels[kMaxSaveRows];
	/** Isle progress labels. */
	GUI::StaticTextWidget *_saveIsleLabels[kMaxSaveRows];
	/** Basecamp 1 progress labels. */
	GUI::StaticTextWidget *_saveBasecamp1Labels[kMaxSaveRows];
	/** Basecamp 2 progress labels. */
	GUI::StaticTextWidget *_saveBasecamp2Labels[kMaxSaveRows];
	/** Town progress labels. */
	GUI::StaticTextWidget *_saveTownLabels[kMaxSaveRows];
	/** Active-pack count labels. */
	GUI::StaticTextWidget *_saveActivePackLabels[kMaxSaveRows];
	/** Target-scoped state filenames used to describe recovery backups. */
	Common::String _saveFileNames[kMaxSaveRows];
	/** Whether each visible corrupted row has a recoverable state payload. */
	bool _saveRecoverableRows[kMaxSaveRows];
	/** Primary validation problem shown before recovering each row. */
	uint8 _saveIssueRows[kMaxSaveRows];
};

/** ScummVM options dialog for Logical Journey of the Zoombinis */
class ZoombiniOptionsWidget : public GUI::OptionsContainerWidget {
public:
	/** Create the Zoombini options widget. */
	ZoombiniOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain);
	/** Release option controls. */
	~ZoombiniOptionsWidget() override;

	// OptionsContainerWidget API
	/** Load persisted option values into the controls. */
	void load() override;
	/** Save control values and return whether the operation succeeded. */
	bool save() override;
	/** Handle save import, export, and management commands. */
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	enum {
		kImportSavesCmd = 'zImp',
		kExportSavesCmd = 'zExp',
		kManageSavesCmd = 'zMgm',
		kResetOptionsCmd = 'zRst'
	};

	// OptionsContainerWidget API
	/** Define the options layout from the active GUI theme. */
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
	/** Set all option controls to their registered default values. */
	void resetToDefaults();

	// True for Broderbund v1.x releases; false for the TLC v2.0 rebuild. Gates
	// the v1.x-only MIDI settings, which are meaningless for v2.0 (no MIDI).
	/** Whether the selected release family exposes the v1.x MIDI settings. */
	bool _isV1x;

	/** Save-file section header. */
	GUI::StaticTextWidget *_saveFilesHeader;
	/** Button that imports original-format save files. */
	GUI::ButtonWidget *_importSavesButton;
	/** Button that exports save files to the original format. */
	GUI::ButtonWidget *_exportSavesButton;
	/** Button that opens the separate save-management modal. */
	GUI::ButtonWidget *_manageSavesButton;
	/** Audio-pop compatibility option control. */
	GUI::CheckboxWidget *_audioPopFixCheckbox;
	/** Fleens tree-descending feet compatibility option control. */
	GUI::CheckboxWidget *_fixFleensTreeDescendFeetBugCheckbox;
	/** Hotel MIDI-halt compatibility option control. */
	GUI::CheckboxWidget *_fixHotelMidiHaltBugCheckbox;
	/** Caves level 4 MIDI compatibility option control. */
	GUI::CheckboxWidget *_fixCavesL4MidiSilentBugCheckbox;
	/** Accurate 60 FPS option control. */
	GUI::CheckboxWidget *_useAccurate60FPSCheckbox;
	/** Enhanced keyboard shortcut option control. */
	GUI::CheckboxWidget *_enhancedKbdShortcutsCheckbox;
	/** Remapped OptionDialog shortcut display option control. */
	GUI::CheckboxWidget *_showRemappedOptionDialogShortcutsCheckbox;
	/** Brighten-palette option control. */
	GUI::CheckboxWidget *_brightenPaletteCheckbox;
	/** Original PRNG option control. */
	GUI::CheckboxWidget *_originalPrngCheckbox;
	/** Color-blind mode option control. */
	GUI::CheckboxWidget *_colorBlindModeCheckbox;
	/** Maze celebration SFX option control. */
	GUI::CheckboxWidget *_alwaysMazePlayCelebrationSfxCheckbox;
	/** Town memorial SFX option control. */
	GUI::CheckboxWidget *_alwaysTownPlayMemorialSfxCheckbox;
	/** Maze unused level-4 layout option control. */
	GUI::CheckboxWidget *_mazeRestoreUnusedL4LayoutCheckbox;
	/** Maze initial-layout randomization option control. */
	GUI::CheckboxWidget *_mazeRandomizeInitialLayoutCheckbox;
	/** Ferry trait-match highlight option control. */
	GUI::CheckboxWidget *_ferryHighlightTraitMatchCheckbox;
	/** Macintosh MIDI option control. */
	GUI::CheckboxWidget *_useMacMidiCheckbox;
	/** Button that restores all engine options to their defaults. */
	GUI::ButtonWidget *_resetButton;
};

} // End of namespace Mohawk

#endif
