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

namespace Ripper {

class WacDatabaseMediaCallback;
class WacManager;
struct MouseState;

class WacDatabaseSession {
public:
	explicit WacDatabaseSession(WacManager *wac);

	uint16 run();

private:
	friend class WacDatabaseMediaCallback;

	struct DatabaseEntry {
		Common::String label;
		byte originalIndex;
	};

	void serviceDatabaseCornerAnimation(bool textPanelActive = false);
	void buildDatabaseEntries();
	void drawDatabase() const;
	void clearDatabaseMediaViewport();
	bool loadDatabaseStillImage(const Common::String &path);
	void drawDatabaseStillImage() const;
	void drawDatabaseScrollControls() const;
	int findDatabaseScrollControl(const Common::Point &point) const;
	void scrollDatabaseStillImage(int delta);
	uint16 serviceDatabaseMediaInput(byte activeEntryIndex,
		uint *textFirstVisible = nullptr, uint textMaximumFirstVisible = 0,
		uint textPageRows = 0, MouseState *publishedMouse = nullptr,
		bool deferCursorUpdate = false);
	bool drawDatabaseTextPanel(uint bodyResourceId, const Common::Rect &bounds,
		uint firstVisible, uint &maximumFirstVisible, uint &visibleRows);
	void wrapJournalText(const Common::String &text, uint maximumWidth,
		Common::Array<Common::String> &lines) const;
	bool drawJournalTextPanel(const Common::Array<Common::String> &lines, uint progress,
		uint firstVisible, uint &maximumFirstVisible, uint &visibleRows);
	bool drawJournalTextPanelLine(const Common::Array<Common::String> &lines,
		uint firstVisible, uint visibleRows, uint lineIndex);
	uint16 runJournalRevealScene(DatabaseEntry &entry);
	uint16 runVoiceLockPuzzle(DatabaseEntry &entry);
	uint16 runDatabaseTextPanel(DatabaseEntry &entry, uint bodyResourceId);
	uint16 dispatchDatabaseEntry(DatabaseEntry &entry);

	WacManager *_wac;
	BitmapAssetFrame _databaseStillImage;
	Common::Array<DatabaseEntry> _databaseEntries;
	uint _databaseSelection;
	uint _databaseFirstVisible;
	uint _databaseStillImageScroll;
	int _databaseScrollControl;
	int _databaseTextScrollControl;
	int _databaseTextScrollDragOffset;
	uint32 _databaseCornerLastMillis;
	bool _databaseCornerAlternate;
	bool _databaseTextScrollDragging;
};

} // End of namespace Ripper

#endif // RIPPER_WAC_DATABASE_H
