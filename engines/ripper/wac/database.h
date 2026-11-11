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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_WAC_DATABASE_H
#define RIPPER_WAC_DATABASE_H

#include "common/array.h"
#include "common/rect.h"

#include "ripper/resources.h"
#include "ripper/controls.h"
#include "ripper/input.h"
#include "ripper/modal/modal_dialog.h"
#include "ripper/wac/database_catalog.h"

namespace Ripper {

class WacDatabaseMediaCallback;
class WacManager;
class WacStillImageViewer;
class WacVoiceLockRenderer;
class RipperEngine;

class WacDatabaseSession {
public:
	explicit WacDatabaseSession(WacManager *wac);

	uint16 run();

private:
	friend class WacDatabaseMediaCallback;
	friend class WacStillImageViewer;
	friend class WacVoiceLockRenderer;

	struct DatabaseEntry {
		Common::String label;
		const WacDatabaseCatalogEntry *catalog;

		byte originalIndex() const { return catalog->originalIndex; }
	};

	enum class DatabaseInputMode {
		kPlayback,
		kTextPanel,
		kViewer
	};
	enum class DatabaseInputOutcome {
		kContinue,
		kCloseEntry,
		kExitWac,
		kSelectEntry,
		kRedrawText,
		kRefreshPalette
	};

	struct TextScrollState {
		uint firstVisible = 0;
		uint maximumFirstVisible = 0;
		uint visibleRows = 0;
		ModalDialogManager::TextPanelScrollControl control = ModalDialogManager::kTextPanelScrollNone;
		int dragOffset = 0;
		bool dragging = false;
	};

	struct DatabaseInputState {
		DatabaseInputMode mode;
		TextScrollState text;

		explicit DatabaseInputState(DatabaseInputMode mode_) : mode(mode_) {}
	};

	struct DatabaseInputResult {
		DatabaseInputOutcome outcome;
		MouseState mouse;

		explicit DatabaseInputResult(DatabaseInputOutcome outcome_ = DatabaseInputOutcome::kContinue) :
			outcome(outcome_) {}
		bool closesEntry() const {
			return outcome == DatabaseInputOutcome::kCloseEntry ||
				outcome == DatabaseInputOutcome::kExitWac ||
				outcome == DatabaseInputOutcome::kSelectEntry;
		}
	};

	void serviceDatabaseCornerAnimation(bool textPanelActive = false);
	void buildDatabaseEntries();
	void drawDatabase() const;
	void clearDatabaseMediaViewport();
	DatabaseInputResult serviceDatabaseMediaInput(byte activeEntryIndex, DatabaseInputState &input);
	DatabaseInputOutcome routeDatabaseAction(uint16 action);
	bool navigateDatabase(uint16 command, bool allowTab);
	bool resolveDatabaseRow(const Common::Point &point, uint &index) const;
	bool scrollDatabaseTextKey(TextScrollState &scroll, uint16 command, bool &redraw);
	bool scrollDatabaseTextMouse(TextScrollState &scroll, const MouseState &mouse);
	bool drawDatabaseTextPanel(uint bodyResourceId, const Common::Rect &bounds,
		TextScrollState &scroll);
	uint16 runDatabaseTextPanel(DatabaseEntry &entry, uint bodyResourceId);
	uint16 runJournal(byte entryIndex, const Common::String &entryLabel);
	bool drawJournalTextPanel(const Common::Array<Common::String> &lines, uint progress,
		TextScrollState &scroll);
	bool drawJournalTextPanelLine(const Common::Array<Common::String> &lines,
		uint firstVisible, uint visibleRows, uint lineIndex);
	uint16 runVoiceLock(byte entryIndex, const Common::String &entryLabel);
	uint16 dispatchDatabaseEntry(DatabaseEntry &entry);
	RipperEngine *engine() const;
	const Common::String &resourceString(uint resourceId) const;
	uint measureText(const Common::String &text) const;
	void wrapText(const Common::String &text, uint maxWidth,
		Common::Array<Common::String> &lines) const;
	void drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const;
	void drawBitmap(const BitmapAssetFrame &bitmap, int x, int y) const;
	void drawAnimatedCorner(int x, int y) const;
	void serviceIdleEffects();
	bool persistentControlHovered() const;
	const Common::Array<BitmapAssetFrame> &databaseScrollArrows() const;

	WacManager *_wac;
	Common::Array<DatabaseEntry> _databaseEntries;
	ChooserModel _databaseChooser;
	uint32 _databaseCornerLastMillis;
	bool _databaseCornerAlternate;
};

} // End of namespace Ripper

#endif // RIPPER_WAC_DATABASE_H
