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
 *
 */

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/world/item_sorter.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/misc/rect.h"
#include "ultima/ultima8/games/game_data.h"

// temp
#include "ultima/ultima8/world/actors/weapon_overlay.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/get_object.h"
// --

namespace Ultima {
namespace Ultima8 {

// This does NOT need to be in the header
struct SortItem {
	SortItem(SortItem *n) : _next(n), _prev(nullptr), _itemNum(0),
			_shape(nullptr), _order(-1), _depends(), _shapeNum(0),
			_frame(0), _flags(0), _extFlags(0), _sx(0), _sy(0),
			_sx2(0), _sy2(0), _x(0), _y(0), _z(0), _xLeft(0),
			_yFar(0), _zTop(0), _sxLeft(0), _sxRight(0), _sxTop(0),
			_syTop(0), _sxBot(0), _syBot(0),_f32x32(false), _flat(false),
			_occl(false), _solid(false), _draw(false), _roof(false),
			_noisy(false), _anim(false), _trans(false), _fixed(false),
			_land(false), _occluded(false), _clipped(0) { }

	SortItem                *_next;
	SortItem                *_prev;

	uint16                  _itemNum;   // Owner item number

	const Shape             *_shape;
	uint32                  _shapeNum;
	uint32                  _frame;
	uint32                  _flags;     // Item flags
	uint32                  _extFlags;  // Item extended flags

	int                     _sx, _sx2;  // Screenspace X coords
	int                     _sy, _sy2;  // Screenspace Y coords

	/*
	            Bounding Box layout

	       1
	     /   \
	   /       \     1 = Left  Far  Top LFT --+
	 2           3   2 = Left  Near Top LNT -++
	 | \       / |   3 = Right Far  Top RFT +-+
	 |   \   /   |   4 = Right Near Top RNT +++
	 |     4     |   5 = Left  Near Bot LNB -+-
	 |     |     |   6 = Right Far  Bot RFB +--
	 5     |     6   7 = Right Near Bot RNB ++-
	   \   |   /     8 = Left  Far  Bot LFB --- (not shown)
	     \ | /
	       7

	*/

	int32   _x, _xLeft;   // Worldspace bounding box x (xright = x)
	int32   _y, _yFar;    // Worldspace bounding box y (ynear = y)
	int32   _z, _zTop;    // Worldspace bounding box z (_zTop = z)

	int32   _sxLeft;     // Screenspace bounding box left extent    (LNT x coord)
	int32   _sxRight;    // Screenspace bounding box right extent   (RFT x coord)

	int32   _sxTop;      // Screenspace bounding box top x coord    (LFT x coord)
	int32   _syTop;      // Screenspace bounding box top extent     (LFT y coord)

	int32   _sxBot;      // Screenspace bounding box bottom x coord (RNB x coord) ss origin
	int32   _syBot;      // Screenspace bounding box bottom extent  (RNB y coord) ss origin

	bool    _f32x32 : 1;         // Needs 1 bit  0
	bool    _flat : 1;           // Needs 1 bit  1
	bool    _occl : 1;           // Needs 1 bit  2
	bool    _solid : 1;          // Needs 1 bit  3
	bool    _draw : 1;           // Needs 1 bit  4
	bool    _roof : 1;           // Needs 1 bit  5
	bool    _noisy : 1;          // Needs 1 bit  6
	bool    _anim : 1;           // Needs 1 bit  7
	bool    _trans : 1;          // Needs 1 bit  8
	bool    _fixed : 1;
	bool    _land : 1;

	bool    _occluded : 1;       // Set true if occluded

	int16   _clipped;            // Clipped to RenderSurface

	int32   _order;      // Rendering _order. -1 is not yet drawn

	// Note that Std::priority_queue could be used here, BUT there is no guarentee that it's implementation
	// will be friendly to insertions
	// Alternatively i could use Std::list, BUT there is no guarentee that it will keep wont delete
	// the unused nodes after doing a clear
	// So the only reasonable solution is to write my own list
	struct DependsList {
		struct Node {
			Node        *_next;
			Node        *_prev;
			SortItem    *val;
			Node() : _next(nullptr), _prev(nullptr), val(nullptr) { }
		};

		Node *list;
		Node *tail;
		Node *unused;

		struct iterator {
			Node *n;
			SortItem *&operator *() {
				return n->val;
			}
			iterator(Node *node) : n(node) { }
			iterator &operator++() {
				n = n->_next;
				return *this;
			}
			bool operator != (const iterator &o) {
				return n != o.n;
			}
		};

		iterator begin() const {
			return iterator(list);
		}
		iterator end() const {
			return iterator(nullptr);
		}

		void clear() {
			if (tail) {
				tail->_next = unused;
				unused = list;
				tail = nullptr;
				list = nullptr;
			}
		}

		void push_back(SortItem *other) {
			if (!unused) unused = new Node();
			Node *nn = unused;
			unused = unused->_next;
			nn->val = other;

			// Put it at the end
			if (tail) tail->_next = nn;
			if (!list) list = nn;
			nn->_next = nullptr;
			nn->_prev = tail;
			tail = nn;
		}

		void insert_sorted(SortItem *other) {
			if (!unused) unused = new Node();
			Node *nn = unused;
			unused = unused->_next;
			nn->val = other;

			for (Node *n = list; n != nullptr; n = n->_next) {
				// Get the insert point... which is before the first item that has higher z than us
				if (other->ListLessThan(n->val)) {
					nn->_next = n;
					nn->_prev = n->_prev;
					n->_prev = nn;
					if (nn->_prev) nn->_prev->_next = nn;
					else list = nn;
					return;
				}
			}

			// No suitable, so put at end
			if (tail) tail->_next = nn;
			if (!list) list = nn;
			nn->_next = nullptr;
			nn->_prev = tail;
			tail = nn;
		}

		DependsList() : list(nullptr), tail(nullptr), unused(nullptr) { }

		~DependsList() {
			clear();
			while (unused)  {
				Node *n = unused->_next;
				delete unused;
				unused = n;
			}
		}
	};

	//Std::vector<SortItem *>   _depends;    // All this Items dependencies (i.e. all objects behind)
	//Std::list<SortItem *> _depends;    // All this Items dependencies (i.e. all objects behind)
	DependsList _depends;

	// Functions

	// Check to see if we overlap si2
	inline bool overlap(const SortItem &si2) const;

	// Check to see if we occlude si2
	inline bool occludes(const SortItem &si2) const;

	// Operator less than
	inline bool operator<(const SortItem &si2) const;

	// Operator left shift (ok, what this does it output the comparisons)
	inline bool operator<<(const SortItem &si2) const;

	// Comparison for the sorted lists
	inline bool ListLessThan(const SortItem *other) const {
		return _z < other->_z || (_z == other->_z && _flat);
	}

};

// Check to see if we overlap si2
inline bool SortItem::overlap(const SortItem &si2) const {
	const int point_top_diff[2] = { _sxTop - si2._sxBot, _syTop - si2._syBot };
	const int point_bot_diff[2] = { _sxBot - si2._sxTop, _syBot - si2._syTop };

	// This function is a bit of a hack. It uses dot products between
	// points and the lines. Nothing is normalized since that isn't
	// important

	// 'normal' of top  left line ( 2,-1) of the bounding box
	const int32 dot_top_left = point_top_diff[0] + point_top_diff[1] * 2;

	// 'normal' of top right line ( 2, 1) of the bounding box
	const int32 dot_top_right = -point_top_diff[0] + point_top_diff[1] * 2;

	// 'normal' of bot  left line (-2,-1) of the bounding box
	const int32 dot_bot_left =  point_bot_diff[0] - point_bot_diff[1] * 2;

	// 'normal' of bot right line (-2, 1) of the bounding box
	const int32 dot_bot_right = -point_bot_diff[0] - point_bot_diff[1] * 2;

	const bool right_clear = _sxRight <= si2._sxLeft;
	const bool left_clear = _sxLeft >= si2._sxRight;
	const bool top_left_clear = dot_top_left >= 0;
	const bool top_right_clear = dot_top_right >= 0;
	const bool bot_left_clear = dot_bot_left >= 0;
	const bool bot_right_clear = dot_bot_right >= 0;

	const bool clear = right_clear || left_clear ||
	                   (bot_right_clear || bot_left_clear) ||
	                   (top_right_clear || top_left_clear);

	return !clear;
}

// Check to see if we occlude si2
inline bool SortItem::occludes(const SortItem &si2) const {
	const int point_top_diff[2] = { _sxTop - si2._sxTop, _syTop - si2._syTop };
	const int point_bot_diff[2] = { _sxBot - si2._sxBot, _syBot - si2._syBot };

	// This function is a bit of a hack. It uses dot products between
	// points and the lines. Nothing is normalized since that isn't
	// important

	// 'normal' of top left line ( 2, -1) of the bounding box
	const int32 dot_top_left = point_top_diff[0] + point_top_diff[1] * 2;

	// 'normal' of top right line ( 2, 1) of the bounding box
	const int32 dot_top_right = -point_top_diff[0] + point_top_diff[1] * 2;

	// 'normal' of bot  left line (-2,-1) of the bounding box
	const int32 dot_bot_left =  point_bot_diff[0] - point_bot_diff[1] * 2;

	// 'normal' of bot right line (-2, 1) of the bounding box
	const int32 dot_bot_right = -point_bot_diff[0] - point_bot_diff[1] * 2;


	const bool right_res = _sxRight >= si2._sxRight;
	const bool left_res = _sxLeft <= si2._sxLeft;
	const bool top_left_res = dot_top_left <= 0;
	const bool top_right_res = dot_top_right <= 0;
	const bool bot_left_res = dot_bot_left <= 0;
	const bool bot_right_res = dot_bot_right <= 0;

	return right_res && left_res && bot_right_res && bot_left_res &&
		top_right_res && top_left_res;
}

inline bool SortItem::operator<(const SortItem &si2) const {
	const SortItem &si1 = *this;

	// Specialist z flat handling
	if (si1._flat && si2._flat) {
		// Differing z is easy for flats
		if (si1._zTop != si2._zTop) return si1._zTop < si2._zTop;

		// Equal z

		// Animated always gets drawn after
		if (si1._anim != si2._anim) return si1._anim < si2._anim;

		// Trans always gets drawn after
		if (si1._trans != si2._trans) return si1._trans < si2._trans;

		// Draw always gets drawn first
		if (si1._draw != si2._draw) return si1._draw > si2._draw;

		// Solid always gets drawn first
		if (si1._solid != si2._solid) return si1._solid > si2._solid;

		// Occludes always get drawn first
		if (si1._occl != si2._occl) return si1._occl > si2._occl;

		// 32x32 flats get drawn first
		if (si1._f32x32 != si2._f32x32) return si1._f32x32 > si2._f32x32;
	}
	// Mixed, or non flat
	else {
		// Clearly in z
		if (si1._zTop <= si2._z)
			return true;
		else if (si1._z >= si2._zTop)
			return false;
	}

	// Clearly in x?
	if (si1._x <= si2._xLeft) return true;
	else if (si1._xLeft >= si2._x) return false;

	// Clearly in y?
	if (si1._y <= si2._yFar) return true;
	else if (si1._yFar >= si2._y) return false;

	// Are overlapping in all 3 dimentions if we come here

	// Overlapping z-bottom check
	// If an object's base (z-bottom) is higher another's, it should be rendered after.
	// This check must be on the z-bottom and not the z-top because two objects with the
	// same z-position may have different heights (think of a mouse sorting vs the Avatar).
	if (si1._z < si2._z) return true;
	else if (si1._z > si2._z) return false;

	// Biased Clearly in z
	if ((si1._zTop + si1._z) / 2 <= si2._z) return true;
	else if (si1._z >= (si2._zTop + si2._z) / 2) return false;

	// Biased Clearly X
	if ((si1._x + si1._xLeft) / 2 <= si2._xLeft) return true;
	else if (si1._xLeft >= (si2._x + si2._xLeft) / 2) return false;

	// Biased Clearly Y
	if ((si1._y + si1._yFar) / 2 <= si2._yFar) return true;
	else if (si1._yFar >= (si2._y + si2._yFar) / 2) return false;

	// Partial in X + Y front
	if (si1._x + si1._y != si2._x + si2._y) return (si1._x + si1._y < si2._x + si2._y);

	// Partial in X + Y back
	if (si1._xLeft + si1._yFar != si2._xLeft + si2._yFar) return (si1._xLeft + si1._yFar < si2._xLeft + si2._yFar);

	// Partial in x?
	if (si1._x != si2._x) return si1._x < si2._x;

	// Partial in y?
	if (si1._y != si2._y) return si1._y < si2._y;

	// Just sort by shape number
	if (si1._shapeNum != si2._shapeNum) return si1._shapeNum < si2._shapeNum;

	// And then by _frame
	return si1._frame < si2._frame;
}

#define COMPARISON_RETURN(val,op,tab)                               \
	pout << tab"if (si1."#val" != si2."#val") -> ("             \
	     << si1.val << " != " << si2.val << ") -> "          \
	     << (si1.val != si2.val) << Std::endl;               \
	pout << tab"{" << Std::endl;                                \
	if (si1.val != si2.val)                                     \
	{                                                           \
		pout << tab"\treturn si1."#val" "#op" si2."#val"; -> (" \
		     << si1.val << " "#op" " << si2.val << ") -> "\
		     << (si1.val op si2.val) << Std::endl;       \
		return si1.val op si2.val;                              \
	}                                                           \
	pout << tab"}" << Std::endl;

#define COMPARISON_RETURN_EX(val1,op,val2,tab)                      \
	pout << tab"if ("#val1" != "#val2") -> ("                   \
	     << val1 << " != " << val2 << ") -> "                \
	     << (val1 != val2) << Std::endl;                     \
	pout << tab"{" << Std::endl;                                \
	if (val1 != val2)                                           \
	{                                                           \
		pout << tab"\treturn "#val1" "#op" "#val2"; -> ("       \
		     << val1 << " "#op" " << val2 << ") -> "     \
		     << (val1 op val2) << Std::endl;             \
		return val1 op val2;                                    \
	}                                                           \
	pout << tab"}" << Std::endl;

#define COMPARISON_RETURN_TF(val1,op,val2,tf,tab)                   \
	pout << tab "if ("#val1" "#op" "#val2") -> ("                   \
	     << val1 << " "#op" " << val2 << ") -> "             \
	     << (val1 op val2) << Std::endl;                     \
	pout << tab"{" << Std::endl;                                \
	if (val1 op val2)                                           \
	{                                                           \
		pout << tab"\treturn " << tf << Std::endl;              \
		return tf;                                              \
	}                                                           \
	pout << tab"}" << Std::endl;

inline bool SortItem::operator<<(const SortItem &si2) const {
	const SortItem &si1 = *this;

	if (si2.overlap(si1)) pout << "Overlaping" << Std::endl;
	else {
		pout << "Not Overlaping" << Std::endl;
		return false;
	}

	// Specialist z flat handling
	pout << "if (si1._flat && si2._flat) -> if ("
	     << si1._flat << " && " << si2._flat << ") -> "
	     << (si1._flat && si2._flat) << Std::endl;
	pout << "{" << Std::endl;

	if (si1._flat && si2._flat) {
		// Differing z is easy for flats
		//if (si1._zTop != si2._zTop) return si1._zTop < si2._zTop;
		COMPARISON_RETURN(_zTop, <, "\t");

		// Equal z

		// Animated always gets drawn after
		//if (si1._anim != si2._anim) return si1._anim < si2._anim;
		COMPARISON_RETURN(_anim, <, "\t");

		// Trans always gets drawn after
		//if (si1._trans != si2._trans) return si1._trans < si2._trans;
		COMPARISON_RETURN(_trans, <, "\t");

		// Draw always gets drawn first
		//if (si1._draw != si2._draw) return si1._draw > si2._draw;
		COMPARISON_RETURN(_draw, >, "\t");

		// Solid always gets drawn first
		//if (si1._solid != si2._solid) return si1._solid > si2._solid;
		COMPARISON_RETURN(_solid, >, "\t");

		// Occludes always get drawn first
		//if (si1._occl != si2._occl) return si1._occl > si2._occl;
		COMPARISON_RETURN(_occl, >, "\t");

		// 32x32 flats get drawn first
		//if (si1._f32x32 != si2._f32x32) return si1._f32x32 > si2._f32x32;
		COMPARISON_RETURN(_f32x32, >, "\t");
	}
	// Mixed, or non flat
	else {
		pout << "}" << Std::endl;
		pout << "else" << Std::endl;
		pout << "{" << Std::endl;

		// Clearly X, Y and Z  (useful?)
		//if (si1._x <= si2._xLeft && si1._y <= si2._yFar && si1._zTop <= si2._z) return true;
		//else if (si1._xLeft >= si2._x && si1._yFar >= si2._y && si1._z >= si2._zTop) return false;

		//int front1 = si1._x + si1._y;
		//int rear1 = si1._xLeft + si1._yFar;
		//int front2 = si2._x + si2._y;
		//int rear2 = si2._xLeft + si2._yFar;

		// Rear of object is infront of other's front
		//if (front1 <= rear2) return true;
		//else if (rear1 >= front2) return false;
		//COMPARISON_RETURN_TF(front1,<=,rear2,true,"\t");
		//COMPARISON_RETURN_TF(rear1,>=,front2,false,"\t");

		// Clearly in z
		//if (si1._zTop <= si2._z) return true;
		//else if (si1._z >= si2._zTop) return false;
		COMPARISON_RETURN_TF(si1._zTop, <=, si2._z, true, "\t");
		COMPARISON_RETURN_TF(si1._z, >=, si2._zTop, false, "\t");

		// Partial in z
		//if (si1._zTop != si2._zTop) return si1._zTop < si2._zTop;
	}
	pout << "}" << Std::endl;

	// Clearly in x and y? (useful?)
	//if (si1._x <= si2._xLeft && si1._y <= si2._yFar) return true;
	//else if (si1._xLeft >= si2._x && si1._yFar >= si2._y) return false;

	// Clearly X
	//if (si1._x <= si2._xLeft) return true;
	//else if (si1._xLeft >= si2._x) return false;
	COMPARISON_RETURN_TF(si1._x, <=, si2._xLeft, true, "\t");
	COMPARISON_RETURN_TF(si1._xLeft, >=, si2._x, false, "\t");

	// Clearly Y
	//if (si1._y <= si2._yFar) return true;
	//else if (si1._yFar >= si2._y) return false;
	COMPARISON_RETURN_TF(si1._y, <=, si2._yFar, true, "\t");
	COMPARISON_RETURN_TF(si1._yFar, >=, si2._y, false, "\t");

	// Z base
	COMPARISON_RETURN_TF(si1._z, <, si2._z, true, "");
	COMPARISON_RETURN_TF(si1._z, >, si2._z, false, "");

	// Biased Clearly in z
	//if ((si1._zTop+si1._z)/2 <= si2._z) return true;
	//else if (si1._z >= (si2._zTop+si2._z)/2) return false;
	COMPARISON_RETURN_TF((si1._zTop + si1._z) / 2, <=, si2._z, true, "");
	COMPARISON_RETURN_TF(si1._z, >=, (si2._zTop + si2._z) / 2, false, "");

	// Biased Clearly X
	//if ((si1._x+si1._xLeft)/2 <= si2._xLeft) return true;
	//else if (si1._xLeft >= (si2._x+si2._xLeft)/2) return false;
	COMPARISON_RETURN_TF((si1._x + si1._xLeft) / 2, <=, si2._xLeft, true, "");
	COMPARISON_RETURN_TF(si1._xLeft, >=, (si2._x + si2._xLeft) / 2, false, "");

	// Biased Clearly Y
	//if ((si1._y+si1._yFar)/2 <= si2._yFar) return true;
	//else if (si1._yFar >= (si2._y+si2._yFar)/2) return false;
	COMPARISON_RETURN_TF((si1._y + si1._yFar) / 2, <=, si2._yFar, true, "");
	COMPARISON_RETURN_TF(si1._yFar, >=, (si2._y + si2._yFar) / 2, false, "");

	// Partial in X + Y front
	//if (si1._x + si1._y != si2._x + si2._y) return (si1._x + si1._y < si2._x + si2._y);
	COMPARISON_RETURN_EX(si1._x + si1._y, <, si2._x + si2._y, "");

	// Partial in X + Y back
	//if (si1._xLeft + si1._yFar != si2._xLeft + si2._yFar) return (si1._xLeft + si1._yFar < si2._xLeft + si2._yFar);
	COMPARISON_RETURN_EX(si1._xLeft + si1._yFar, <, si2._xLeft + si2._yFar, "");

	// Partial in x?
	//if (si1._x != si2._x) return si1._x < si2._x;
	COMPARISON_RETURN(_x, <, "");

	// Partial in y?
	//if (si1._y != si2._y) return si1._y < si2._y;
	COMPARISON_RETURN(_y, <, "");

	// Just sort by _shape number
//	if (si1._shapeNum != si2._shapeNum) return si1._shapeNum < si2._shapeNum;
	COMPARISON_RETURN(_shapeNum, <, "");

	// And then by _frame
	//return si1._frame < si2._frame;
	COMPARISON_RETURN(_frame, <, "");
	return 0;
}


//
// ItemSorter
//

ItemSorter::ItemSorter() :
	_shapes(nullptr), _surf(nullptr), _items(nullptr), _itemsTail(nullptr),
	_itemsUnused(nullptr), _sortLimit(0), _camSx(0), _camSy(0), _orderCounter(0) {
	int i = 2048;
	while (i--) _itemsUnused = new SortItem(_itemsUnused);
}

ItemSorter::~ItemSorter() {
	//
	if (_itemsTail) {
		_itemsTail->_next = _itemsUnused;
		_itemsUnused = _items;
	}
	_items = nullptr;
	_itemsTail = nullptr;

	while (_itemsUnused) {
		SortItem *_next = _itemsUnused->_next;
		delete _itemsUnused;
		_itemsUnused = _next;
	}

	delete [] _items;
}

void ItemSorter::BeginDisplayList(RenderSurface *rs,
                                  int32 camx, int32 camy, int32 camz) {
	// Get the _shapes, if required
	if (!_shapes) _shapes = GameData::get_instance()->getMainShapes();

	//
	if (_itemsTail) {
		_itemsTail->_next = _itemsUnused;
		_itemsUnused = _items;
	}
	_items = nullptr;
	_itemsTail = nullptr;

	// Set the RenderSurface, and reset the item list
	_surf = rs;
	_orderCounter = 0;

	// Screenspace bounding box bottom x coord (RNB x coord)
	_camSx = (camx - camy) / 4;
	// Screenspace bounding box bottom extent  (RNB y coord)
	_camSy = (camx + camy) / 8 - camz;
}

void ItemSorter::AddItem(int32 x, int32 y, int32 z, uint32 shapeNum, uint32 frame_num, uint32 flags, uint32 ext_flags, uint16 itemNum) {

	// First thing, get a SortItem to use (first of unused)
	if (!_itemsUnused)
		_itemsUnused = new SortItem(0);
	SortItem *si = _itemsUnused;

	si->_itemNum = itemNum;
	si->_shape = _shapes->getShape(shapeNum);
	si->_shapeNum = shapeNum;
	si->_frame = frame_num;
	const ShapeFrame *_frame = si->_shape->getFrame(si->_frame);
	if (!_frame) {
		perr << "Invalid shape: " << si->_shapeNum << "," << si->_frame
		     << Std::endl;
		return;
	}

	si->_flags = flags;
	si->_extFlags = ext_flags;

	const ShapeInfo *info = _shapes->getShapeInfo(shapeNum);
	// Dimensions
	int32 xd, yd, zd;
	info->getFootpadWorld(xd, yd, zd, flags & Item::FLG_FLIPPED);

	// Worldspace bounding box
	si->_x = x;
	si->_y = y;
	si->_z = z;
	si->_xLeft = si->_x - xd;
	si->_yFar = si->_y - yd;
	si->_zTop = si->_z + zd;

	// Screenspace bounding box left extent    (LNT x coord)
	si->_sxLeft = si->_xLeft / 4 - si->_y / 4 - _camSx;
	// Screenspace bounding box right extent   (RFT x coord)
	si->_sxRight = si->_x / 4 - si->_yFar / 4 - _camSx;

	// Screenspace bounding box top x coord    (LFT x coord)
	si->_sxTop = si->_xLeft / 4 - si->_yFar / 4 - _camSx;
	// Screenspace bounding box top extent     (LFT y coord)
	si->_syTop = si->_xLeft / 8 + si->_yFar / 8 - si->_zTop - _camSy;

	// Screenspace bounding box bottom x coord (RNB x coord)
	si->_sxBot = si->_x / 4 - si->_y / 4 - _camSx;
	// Screenspace bounding box bottom extent  (RNB y coord)
	si->_syBot = si->_x / 8 + si->_y / 8 - si->_z - _camSy;

	// Real Screenspace coords
	si->_sx = si->_sxBot - _frame->_xoff;   // Left
	si->_sy = si->_syBot - _frame->_yoff;   // Top
	si->_sx2 = si->_sx + _frame->_width;    // Right
	si->_sy2 = si->_sy + _frame->_height;   // Bottom

	// Do Clipping here
	si->_clipped = _surf->CheckClipped(Rect(si->_sx, si->_sy, si->_sx + _frame->_width, si->_sy + _frame->_height));
	if (si->_clipped < 0)
		// Clipped away entirely - don't add to the list.
		return;

	// These help out with sorting. We calc them now, so it will be faster
	si->_f32x32 = xd == 128 && yd == 128;
	si->_flat = zd == 0;

	si->_draw = info->is_draw();
	si->_solid = info->is_solid();
	si->_occl = info->is_occl() && !(si->_flags & Item::FLG_INVISIBLE) &&
			   !(si->_extFlags & Item::EXT_TRANSPARENT);
	si->_roof = info->is_roof();
	si->_noisy = info->is_noisy();
	si->_anim = info->_animType != 0;
	si->_trans = info->is_translucent();
	si->_fixed = info->is_fixed();
	si->_land = info->is_land();

	si->_occluded = false;
	si->_order = -1;

	// We will clear all the vector memory
	// Stictly speaking the vector will sort of leak memory, since they
	// are never deleted
	si->_depends.clear();

	// Iterate the list and compare _shapes

	// Ok,
	SortItem *addpoint = nullptr;
	for (SortItem *si2 = _items; si2 != nullptr; si2 = si2->_next) {
		// Get the insert point... which is before the first item that has higher z than us
		if (!addpoint && si->ListLessThan(si2))
			addpoint = si2;

		// Doesn't overlap
		if (si2->_occluded || !si->overlap(*si2))
			continue;

		// Attempt to find which is infront
		if (*si < *si2) {
			// si2 occludes si (us)
			if (si2->_occl && si2->occludes(*si)) {
				// No need to do any more checks, this isn't visible
				si->_occluded = true;
				break;
			}

			// si1 is behind si2, so add it to si2's dependency list
			si2->_depends.insert_sorted(si);
		} else {
			// ss occludes si2. Sadly, we can't remove it from the list.
			if (si->_occl && si->occludes(*si2))
				si2->_occluded = true;
			// si2 is behind si1, so add it to si1's dependency list
			else
				si->_depends.push_back(si2);
		}
	}

	// Add it to the list
	_itemsUnused = _itemsUnused->_next;

	// have a position
	//addpoint = 0;
	if (addpoint) {
		si->_next = addpoint;
		si->_prev = addpoint->_prev;
		addpoint->_prev = si;
		if (si->_prev)
			si->_prev->_next = si;
		else
			_items = si;
	}
	// Add it to the end of the list
	else {
		if (_itemsTail)
			_itemsTail->_next = si;
		if (!_items)
			_items = si;
		si->_next = nullptr;
		si->_prev = _itemsTail;
		_itemsTail = si;
	}
}

void ItemSorter::AddItem(const Item *add) {
	int32 x, y, z;
	add->getLerped(x, y, z);
	AddItem(x, y, z, add->getShape(), add->getFrame(),
			add->getFlags(), add->getExtFlags(), add->getObjId());
}

SortItem *_prev = 0;

void ItemSorter::PaintDisplayList(bool item_highlight) {
	_prev = nullptr;
	SortItem *it = _items;
	SortItem *end = nullptr;
	_orderCounter = 0;  // Reset the _orderCounter
	while (it != end) {
		if (it->_order == -1) if (PaintSortItem(it)) return;
		it = it->_next;
	}

	// Item highlighting. We redraw each 'item' transparent
	if (item_highlight) {
		it = _items;
		while (it != end) {
			if (!(it->_flags & (Item::FLG_DISPOSABLE | Item::FLG_FAST_ONLY)) && !it->_fixed) {
				_surf->PaintHighlightInvis(it->_shape,
				                          it->_frame,
				                          it->_sxBot,
				                          it->_syBot,
				                          it->_trans,
				                          (it->_flags & Item::FLG_FLIPPED) != 0, 0x1f00ffff);
			}

			it = it->_next;
		}

	}
}

/**
 * Recursively paint this item and all its dependencies.
 * Returns true if recursion should stop.
 */
bool ItemSorter::PaintSortItem(SortItem *si) {
	// Don't paint this, or dependencies (yet) if occluded
	if (si->_occluded)
		return false;

	// Resursion detection
	si->_order = -2;

	// Iterate through our dependancies, and paint them, if possible
	SortItem::DependsList::iterator it = si->_depends.begin();
	SortItem::DependsList::iterator end = si->_depends.end();
	while (it != end) {
		if ((*it)->_order == -2) {
			//warning("cycle in paint dependency graph %d -> %d -> ... -> %d",
			//		si->_shapeNum, (*it)->_shapeNum, si->_shapeNum);
			break;
		}
		else if ((*it)->_order == -1) {
			if (PaintSortItem((*it)))
				return true;
		}
		++it;
	}

	// Set our painting _order
	si->_order = _orderCounter;
	_orderCounter++;

	// Now paint us!

//	if (wire) si->info->draw_box_back(s, dispx, dispy, 255);

	if (si->_extFlags & Item::EXT_HIGHLIGHT && si->_extFlags & Item::EXT_TRANSPARENT)
		_surf->PaintHighlightInvis(si->_shape, si->_frame, si->_sxBot, si->_syBot, si->_trans, (si->_flags & Item::FLG_FLIPPED) != 0, 0x7F00007F);
	if (si->_extFlags & Item::EXT_HIGHLIGHT)
		_surf->PaintHighlight(si->_shape, si->_frame, si->_sxBot, si->_syBot, si->_trans, (si->_flags & Item::FLG_FLIPPED) != 0, 0x7F00007F);
	else if (si->_extFlags & Item::EXT_TRANSPARENT)
		_surf->PaintInvisible(si->_shape, si->_frame, si->_sxBot, si->_syBot, si->_trans, (si->_flags & Item::FLG_FLIPPED) != 0);
	else if (si->_flags & Item::FLG_FLIPPED)
		_surf->PaintMirrored(si->_shape, si->_frame, si->_sxBot, si->_syBot, si->_trans);
	else if (si->_trans)
		_surf->PaintTranslucent(si->_shape, si->_frame, si->_sxBot, si->_syBot);
	else if (!si->_clipped)
		_surf->PaintNoClip(si->_shape, si->_frame, si->_sxBot, si->_syBot);
	else
		_surf->Paint(si->_shape, si->_frame, si->_sxBot, si->_syBot);

//	if (wire) si->info->draw_box_front(s, dispx, dispy, 255);

	// weapon overlay
	// FIXME: use highlight/invisibility, also add to Trace() ?
	if (si->_shapeNum == 1 && si->_itemNum == 1) {
		MainActor *av = getMainActor();
		const WeaponOverlayFrame *wo_frame = nullptr;
		uint32 wo_shapenum;
		av->getWeaponOverlay(wo_frame, wo_shapenum);
		if (wo_frame) {
			const Shape *wo_shape = GameData::get_instance()->getMainShapes()->getShape(wo_shapenum);
			_surf->Paint(wo_shape, wo_frame->_frame,
			            si->_sxBot + wo_frame->_xOff,
			            si->_syBot + wo_frame->_yOff);
		}
	}

	if (_sortLimit) {
		if (_orderCounter == _sortLimit) {
			static uint32 previt = 0;
			int x1 = si->_xLeft;
			int y1 = si->_yFar;
			int z2 = si->_zTop;
			if (!previt || previt != si->_itemNum) {
				previt = si->_itemNum;
				pout << si->_shapeNum << ":" << si->_frame << " (" << x1 << "," << y1 << "," << si->_z << ") (" << si->_x << "," << si->_y << "," << z2 << ")" << Std::endl;
				//  ss->info->print();
				if (_prev) *_prev << *si;
			}
			return true;
		}
		_prev = si;
	}

	return false;
}

bool ItemSorter::NullPaintSortItem(SortItem *si) {
	// Don't paint this, or dependencies if occluded
	if (si->_occluded) return false;

	// Resursion, detection
	si->_order = -2;

	// Iterate through our dependancies, and paint them, if possible
	SortItem::DependsList::iterator it = si->_depends.begin();
	SortItem::DependsList::iterator end = si->_depends.end();
	while (it != end) {
		// Well, it can't. Implies recursive sorting. Can happen though so
		// you had best leave this commented out
		//if ((*it)->_order == -2) CANT_HAPPEN_MSG("Recursive item sorting");

		if ((*it)->_order == -1) if (NullPaintSortItem((*it))) return true;

		++it;
	}

	// Set our painting/sorting _order
	si->_order = _orderCounter;
	_orderCounter++;

	return false;
}

uint16 ItemSorter::Trace(int32 x, int32 y, HitFace *face, bool item_highlight) {
	SortItem *it;
	SortItem *selected;

	if (!_orderCounter) { // If no _orderCounter we need to sort the _items
		it = _items;
		_orderCounter = 0;  // Reset the _orderCounter
		while (it != nullptr) {
			if (it->_order == -1) if (NullPaintSortItem(it)) break;

			it = it->_next;
		}
	}

	// Firstly, we check for highlighted _items
	selected = nullptr;

	if (item_highlight) {
		selected = nullptr;

		for (it = _itemsTail; it != nullptr; it = it->_prev) {
			if (!(it->_flags & (Item::FLG_DISPOSABLE | Item::FLG_FAST_ONLY)) && !it->_fixed) {

				if (!it->_itemNum) continue;

				// Doesn't Overlap
				if (x < it->_sx || x >= it->_sx2 || y < it->_sy || y >= it->_sy2) continue;

				// Now check the _frame itself
				const ShapeFrame *_frame = it->_shape->getFrame(it->_frame);
				assert(_frame); // invalid frames shouldn't have been added to the list

				// Nope, doesn't have a point
				if (it->_flags & Item::FLG_FLIPPED) {
					if (!_frame->hasPoint(it->_sxBot - x, y - it->_syBot)) continue;
				} else {
					if (!_frame->hasPoint(x - it->_sxBot, y - it->_syBot)) continue;
				}

				// Ok now check against selected
				selected = it;
			}
		}

	}

	// Ok, this is all pretty simple. We iterate all the _items.
	// We then check to see if the item has a point where the trace goes.
	// Finally we then set the selected SortItem if it's '_order' is highest

	if (!selected) for (it = _items; it != nullptr; it = it->_next) {
			if (!it->_itemNum) continue;

			// Doesn't Overlap
			if (x < it->_sx || x >= it->_sx2 || y < it->_sy || y >= it->_sy2) continue;

			// Now check the _frame itself
			const ShapeFrame *_frame = it->_shape->getFrame(it->_frame);
			assert(_frame); // invalid frames shouldn't have been added to the list

			// Nope, doesn't have a point
			if (it->_flags & Item::FLG_FLIPPED) {
				if (!_frame->hasPoint(it->_sxBot - x, y - it->_syBot)) continue;
			} else {
				if (!_frame->hasPoint(x - it->_sxBot, y - it->_syBot)) continue;
			}

			// Ok now check against selected
			if (!selected || (it->_order > selected->_order)) selected = it;
		}

	if (selected) {

		if (face) {
			// shortcut for zero-height _items
			if (selected->_zTop == selected->_z) {
				*face = Z_FACE;
			} else {
				// determine face that was hit

				// RNT coordinates
				int32 RNTx = selected->_sxBot;
				int32 RNTy = selected->_syBot - selected->_zTop + selected->_z;

				/*
				            Bounding Box layout (top part)

				       1
				     /   \
				   /       \     1 = Left  Far  Top LFT --+
				 2  Z-face   3   2 = Left  Near Top LNT -++
				 | \       / |   3 = Right Far  Top RFT +-+
				 |   \   /   |   4 = Right Near Top RNT +++
				 | Y   4  X  |
				 |face |face |

				*/

				if (2 * (y - RNTy) <= (x - RNTx) && // if above/on line 4-3
				        2 * (y - RNTy) < (RNTx - x)) // and above/on line 4-2
					*face = Z_FACE;
				else if (x > RNTx)
					*face = X_FACE;
				else
					*face = Y_FACE;
			}
		}

		return selected->_itemNum;
	}

	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
