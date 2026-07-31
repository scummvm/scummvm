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
class WacJournalPuzzle;
class WacManager;
class WacStillImageViewer;
class WacVoiceLockPuzzle;
class WacVoiceLockRenderer;
class RipperEngine;
struct MouseState;

class WacDatabaseSession {
public:
	explicit WacDatabaseSession(WacManager *wac);

	uint16 run();

private:
	friend class WacDatabaseMediaCallback;
	friend class WacJournalPuzzle;
	friend class WacStillImageViewer;
	friend class WacVoiceLockPuzzle;
	friend class WacVoiceLockRenderer;

	struct DatabaseEntry {
		Common::String label;
		byte originalIndex;
	};

	void serviceDatabaseCornerAnimation(bool textPanelActive = false);
	void buildDatabaseEntries();
	void drawDatabase() const;
	void clearDatabaseMediaViewport();
	uint16 serviceDatabaseMediaInput(byte activeEntryIndex,
		uint *textFirstVisible = nullptr, uint textMaximumFirstVisible = 0,
		uint textPageRows = 0, MouseState *publishedMouse = nullptr,
		bool deferCursorUpdate = false);
	bool drawDatabaseTextPanel(uint bodyResourceId, const Common::Rect &bounds,
		uint firstVisible, uint &maximumFirstVisible, uint &visibleRows);
	uint16 runDatabaseTextPanel(DatabaseEntry &entry, uint bodyResourceId);
	uint16 dispatchDatabaseEntry(DatabaseEntry &entry);
	RipperEngine *engine() const;
	const Common::String &resourceString(uint resourceId) const;
	uint measureText(const Common::String &text) const;
	void drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const;
	void drawBitmap(const BitmapAssetFrame &bitmap, int x, int y) const;
	void drawAnimatedCorner(int x, int y) const;
	void serviceIdleEffects();
	bool persistentControlHovered() const;
	const Common::Array<BitmapAssetFrame> &databaseScrollArrows() const;

	WacManager *_wac;
	Common::Array<DatabaseEntry> _databaseEntries;
	uint _databaseSelection;
	uint _databaseFirstVisible;
	int _databaseTextScrollControl;
	int _databaseTextScrollDragOffset;
	uint32 _databaseCornerLastMillis;
	bool _databaseCornerAlternate;
	bool _databaseTextScrollDragging;
};

} // End of namespace Ripper

#endif // RIPPER_WAC_DATABASE_H
