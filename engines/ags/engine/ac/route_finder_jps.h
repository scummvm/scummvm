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

#include "ags/lib/std/queue.h"
#include "ags/lib/std/vector.h"
#include "ags/lib/std/algorithm.h"
#include "ags/lib/std/functional.h"
#include "ags/lib/std/xutility.h"

// Not all platforms define INFINITY
#ifndef INFINITY
#define INFINITY   ((float)(1e+300 * 1e+300)) // This must overflow
#endif

namespace AGS3 {

// TODO: this could be cleaned up/simplified ...

// further optimizations possible:
//    - forward refinement should use binary search

class Navigation {
public:
	Navigation();

	void Resize(int width, int height);

	enum NavResult {
		// unreachable
		NAV_UNREACHABLE,
		// straight line exists
		NAV_STRAIGHT,
		// path used
		NAV_PATH
	};

	// ncpath = navpoint-compressed path
	// opath = path composed of individual grid elements
	NavResult NavigateRefined(int sx, int sy, int ex, int ey, std::vector<int> &opath,
	                          std::vector<int> &ncpath);

	NavResult Navigate(int sx, int sy, int ex, int ey, std::vector<int> &opath);

	bool TraceLine(int srcx, int srcy, int targx, int targy, int &lastValidX, int &lastValidY) const;
	bool TraceLine(int srcx, int srcy, int targx, int targy, std::vector<int> *rpath = nullptr) const;

	inline void SetMapRow(int y, const unsigned char *row) {
		map[y] = row;
	}

	static int PackSquare(int x, int y);
	static void UnpackSquare(int sq, int &x, int &y);

private:
	// priority queue entry
	struct Entry {
		float cost;
		int index;

		inline Entry() {}

		inline Entry(float ncost, int nindex)
			: cost(ncost)
			, index(nindex) {
		}

		inline bool operator <(const Entry &b) const {
			return cost < b.cost;
		}

		inline bool operator >(const Entry &b) const {
			return cost > b.cost;
		}
	};

	int mapWidth;
	int mapHeight;
	std::vector<const unsigned char *> map;

	typedef unsigned short tFrameId;
	typedef int tPrev;

	struct NodeInfo {
		// quantized min distance from origin
		unsigned short dist;
		// frame id (counter to detect new search)
		tFrameId frameId;
		// previous node index (packed, relative to current node)
		tPrev prev;

		inline NodeInfo()
			: dist(0)
			, frameId(0)
			, prev(-1) {
		}
	};

	static const float DIST_SCALE_PACK;
	static const float DIST_SCALE_UNPACK;

	std::vector<NodeInfo> mapNodes;
	tFrameId frameId;

	std::priority_queue<Entry, std::vector<Entry>, Common::Greater<Entry> > pq;

	// temporary buffers:
	mutable std::vector<int> fpath;
	std::vector<int> ncpathIndex;
	std::vector<int> rayPath, orayPath;

	// temps for routing towards unreachable areas
	int cnode;
	int closest;

	// orthogonal only (this should correspond to what AGS is doing)
	bool nodiag;

	bool navLock;

	void IncFrameId();

	// outside map test
	inline bool Outside(int x, int y) const;
	// stronger inside test
	bool Passable(int x, int y) const;
	// plain access, unchecked
	inline bool Walkable(int x, int y) const;

	void AddPruned(int *buf, int &bcount, int x, int y) const;
	bool HasForcedNeighbor(int x, int y, int dx, int dy) const;
	int FindJump(int x, int y, int dx, int dy, int ex, int ey);
	int FindOrthoJump(int x, int y, int dx, int dy, int ex, int ey);

	// neighbor reachable (nodiag only)
	bool Reachable(int x0, int y0, int x1, int y1) const;

	static inline int sign(int n) {
		return n < 0 ? -1 : (n > 0 ? 1 : 0);
	}

	static inline int iabs(int n) {
		return n < 0 ? -n : n;
	}

	static inline int iclamp(int v, int min, int max) {
		return v < min ? min : (v > max ? max : v);
	}

	static inline int ClosestDist(int dx, int dy) {
		return dx * dx + dy * dy;
		// Manhattan?
		//return iabs(dx) + iabs(dy);
	}
};

inline int Navigation::PackSquare(int x, int y) {
	return (y << 16) + x;
}

inline void Navigation::UnpackSquare(int sq, int &x, int &y) {
	y = sq >> 16;
	x = sq & ((1 << 16) - 1);
}

inline bool Navigation::Outside(int x, int y) const {
	return
	    (unsigned)x >= (unsigned)mapWidth ||
	    (unsigned)y >= (unsigned)mapHeight;
}

inline bool Navigation::Walkable(int x, int y) const {
	// invert condition because of AGS
	return map[y][x] != 0;
}

} // namespace AGS3
