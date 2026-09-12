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

#ifndef HOPKINS_BASE_ENHANCEMENTS_AUTOPLAY_H
#define HOPKINS_BASE_ENHANCEMENTS_AUTOPLAY_H

#include "common/array.h"
#include "common/scummsys.h"

namespace Hopkins {

class BaseData;
class BaseEngine;
struct BaseInputState;

/** Drive WBASE by producing ordinary player input from live edge-grid routes. */
class WBASEEnhancementsAutoplay {
public:
	WBASEEnhancementsAutoplay();

	void reset();
	bool active() const { return _active; }

	void clearMenuPointer();
	void selectPrevious();
	void selectNext();
	void updateMenuPointer(int x, int y);
	bool selectMenuPointer(int x, int y);
	bool startSelected(const BaseEngine &engine);
	bool startDestination(const BaseEngine &engine, int destination);
	void cancel();

	/** Return the forced destination room, or -1, after updating input. */
	int update(const BaseEngine &engine, BaseInputState &input);
	void renderMenu(const BaseData &data, int returnRoomId, byte *framebuffer,
			bool forcedMode) const;
	void renderStatus(const BaseData &data, byte *framebuffer, bool forcedMode) const;

	static int destinationCount();
	static int destinationRoomId(int index);
	static int destinationExitMapPos(int index);
	static const char *destinationLabel(int index);
	static const char *destinationMapLabel(int index);
	static int destinationIndexForExitMapPos(int mapPos);

private:
	bool rebuildRoute(const BaseEngine &engine, bool avoidBlockingObjects);
	int findVisibleGuard(const BaseEngine &engine) const;
	bool aimAt(const BaseEngine &engine, int targetX, int targetY, BaseInputState &input) const;
	void updateStuckState(const BaseEngine &engine, bool triedToMove);
	void beginRecovery();
	void updateRecovery(const BaseEngine &engine, BaseInputState &input);

	bool _active;
	int _selectedDestination;
	int _hoveredDestination;
	int _destination;
	Common::Array<int> _route;
	uint _routeIndex;
	int _lastPlayerX;
	int _lastPlayerY;
	int _stuckTicks;
	bool _routeInvalidated;
	int _blockedRouteCell;
	int _routeBlocker;
	int _movementBlocker;
	int _activeTicks;
	int _recoveryPhase;
	int _recoveryTicks;
	int _recoveryAttempts;
	bool _recoveryTurnRight;
};

} // End of namespace Hopkins

#endif // HOPKINS_BASE_ENHANCEMENTS_AUTOPLAY_H
