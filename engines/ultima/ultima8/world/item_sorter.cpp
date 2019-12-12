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

namespace Ultima8 {

using Pentagram::Rect;

// This does NOT need to be in the header
struct SortItem {
	SortItem(SortItem *n) : next(n), prev(0), item_num(0), shape(0), order(-1), depends() { }

	SortItem                *next;
	SortItem                *prev;

	uint16                  item_num;   // Owner item number

	Shape                   *shape;
	uint32                  shape_num;
	uint32                  frame;
	uint32                  flags;      // Item flags
	uint32                  ext_flags;  // Item extended flags

	int                     sx, sx2;    // Screenspace X coords
	int                     sy, sy2;    // Screenspace Y coords

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
	 5     |     6   7 = Right Near Bot LNB ++-
	   \   |   /     8 = Left  Far  Bot LFB --- (not shown)
	     \ | /
	       7

	*/

	int32   x, xleft;   // Worldspace bounding box x (xright = x)
	int32   y, yfar;    // Worldspace bounding box y (ynear = y)
	int32   z, ztop;    // Worldspace bounding box z (ztop = z)

	int32   sxleft;     // Screenspace bounding box left extent    (LNT x coord)
	int32   sxright;    // Screenspace bounding box right extent   (RFT x coord)

	int32   sxtop;      // Screenspace bounding box top x coord    (LFT x coord)
	int32   sytop;      // Screenspace bounding box top extent     (LFT y coord)

	int32   sxbot;      // Screenspace bounding box bottom x coord (RNB x coord) ss origin
	int32   sybot;      // Screenspace bounding box bottom extent  (RNB y coord) ss origin

	bool    f32x32 : 1;         // Needs 1 bit  0
	bool    flat : 1;           // Needs 1 bit  1
	bool    occl : 1;           // Needs 1 bit  2
	bool    solid : 1;          // Needs 1 bit  3
	bool    draw : 1;           // Needs 1 bit  4
	bool    roof : 1;           // Needs 1 bit  5
	bool    noisy : 1;          // Needs 1 bit  6
	bool    anim : 1;           // Needs 1 bit  7
	bool    trans : 1;          // Needs 1 bit  8
	bool    fixed : 1;
	bool    land : 1;

	bool    occluded : 1;       // Set true if occluded

	int16   clipped;            // Clipped to RenderSurface

	int32   order;      // Rendering order. -1 is not yet drawn

	// Note that std::priority_queue could be used here, BUT there is no guarentee that it's implementation
	// will be friendly to insertions
	// Alternatively i could use std::list, BUT there is no guarentee that it will keep wont delete
	// the unused nodes after doing a clear
	// So the only reasonable solution is to write my own list
	struct DependsList {
		struct Node {
			Node        *next;
			Node        *prev;
			SortItem    *val;
			Node() : next(0), prev(0), val(0) { }
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
				n = n->next;
				return *this;
			}
			bool operator != (const iterator &o) {
				return n != o.n;
			}
		};

		iterator begin() {
			return iterator(list);
		}
		iterator end() {
			return iterator(0);
		}

		void clear() {
			if (tail) {
				tail->next = unused;
				unused = list;
				tail = 0;
				list = 0;
			}
		}

		void push_back(SortItem *other) {
			if (!unused) unused = new Node();
			Node *nn = unused;
			unused = unused->next;
			nn->val = other;

			// Put it at the end
			if (tail) tail->next = nn;
			if (!list) list = nn;
			nn->next = 0;
			nn->prev = tail;
			tail = nn;
		}

		void insert_sorted(SortItem *other) {
			if (!unused) unused = new Node();
			Node *nn = unused;
			unused = unused->next;
			nn->val = other;

			for (Node *n = list; n != 0; n = n->next) {
				// Get the insert point... which is before the first item that has higher z than us
				if (other->ListLessThan(n->val)) {
					nn->next = n;
					nn->prev = n->prev;
					n->prev = nn;
					if (nn->prev) nn->prev->next = nn;
					else list = nn;
					return;
				}
			}

			// No suitable, so put at end
			if (tail) tail->next = nn;
			if (!list) list = nn;
			nn->next = 0;
			nn->prev = tail;
			tail = nn;
		}

		DependsList() : list(0), tail(0), unused(0) { }

		~DependsList() {
			clear();
			while (unused)  {
				Node *n = unused->next;
				delete unused;
				unused = n;
			}
		}
	};

	//std::vector<SortItem *>   depends;    // All this Items dependencies (i.e. all objects behind)
	//std::list<SortItem *> depends;    // All this Items dependencies (i.e. all objects behind)
	DependsList depends;

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
		return z < other->z ||
		       (z == other->z && x < other->x) ||
		       (z == other->z && x == other->x && y < other->y);
	}

};

// Check to see if we overlap si2
inline bool SortItem::overlap(const SortItem &si2) const {
	const int point_top_diff[2] = { sxtop - si2.sxbot, sytop - si2.sybot };
	const int point_bot_diff[2] = { sxbot - si2.sxtop, sybot - si2.sytop };

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

	const bool right_clear = sxright <= si2.sxleft;
	const bool left_clear = sxleft >= si2.sxright;
	const bool top_left_clear = dot_top_left >= 0;
	const bool top_right_clear = dot_top_right >= 0;
	const bool bot_left_clear = dot_bot_left >= 0;
	const bool bot_right_clear = dot_bot_right >= 0;

	const bool clear = right_clear | left_clear |
	                   bot_right_clear | bot_left_clear |
	                   top_right_clear | top_left_clear;

	return !clear;
}

// Check to see if we occlude si2
inline bool SortItem::occludes(const SortItem &si2) const {
	const int point_top_diff[2] = { sxtop - si2.sxtop, sytop - si2.sytop };
	const int point_bot_diff[2] = { sxbot - si2.sxbot, sybot - si2.sybot };

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


	const bool right_res = sxright >= si2.sxright;
	const bool left_res = sxleft <= si2.sxleft;
	const bool top_left_res = dot_top_left <= 0;
	const bool top_right_res = dot_top_right <= 0;
	const bool bot_left_res = dot_bot_left <= 0;
	const bool bot_right_res = dot_bot_right <= 0;

	return right_res & left_res & bot_right_res & bot_left_res &
		top_right_res & top_left_res;
}

inline bool SortItem::operator<(const SortItem &si2) const {
	const SortItem &si1 = *this;

	// Specialist z flat handling
	if (si1.flat && si2.flat) {
		// Differing z is easy for flats
		if (si1.ztop != si2.ztop) return si1.ztop < si2.ztop;

		// Equal z

		// Animated always gets drawn after
		if (si1.anim != si2.anim) return si1.anim < si2.anim;

		// Trans always gets drawn after
		if (si1.trans != si2.trans) return si1.trans < si2.trans;

		// Draw always gets drawn first
		if (si1.draw != si2.draw) return si1.draw > si2.draw;

		// Solid always gets drawn first
		if (si1.solid != si2.solid) return si1.solid > si2.solid;

		// Occludes always get drawn first
		if (si1.occl != si2.occl) return si1.occl > si2.occl;

		// 32x32 flats get drawn first
		if (si1.f32x32 != si2.f32x32) return si1.f32x32 > si2.f32x32;
	}
	// Mixed, or non flat
	else {

		// Clearly X, Y and Z (useful?)
		//if (si1.x <= si2.xleft && si1.y <= si2.yfar && si1.ztop <= si2.z) return true;
		//else if (si1.xleft >= si2.x && si1.yfar >= si2.y && si1.z >= si2.ztop) return false;

		//int front1 = si1.x + si1.y;
		//int rear1 = si1.xleft + si1.yfar;
		//int front2 = si2.x + si2.y;
		//int rear2 = si2.xleft + si2.yfar;

		// Rear of object is infront of other's front
		//if (front1 <= rear2) return true;
		//else if (rear1 >= front2) return false;

		// Clearly in z
		if (si1.ztop <= si2.z) return true;
		else if (si1.z >= si2.ztop) return false;

		// Partial in z
		//if (si1.ztop != si2.ztop) return si1.ztop < si2.ztop;
	}

	// Clearly in x and y? (useful?)
	//if (si1.x <= si2.xleft && si1.y <= si2.yfar) return true;
	//else if (si1.xleft >= si2.x && si1.yfar >= si2.y) return false;

	// Clearly in x?
	if (si1.x <= si2.xleft) return true;
	else if (si1.xleft >= si2.x) return false;

	// Clearly in y?
	if (si1.y <= si2.yfar) return true;
	else if (si1.yfar >= si2.y) return false;

	// Are overlapping in all 3 dimentions if we come here

	// Overlapping z-bottom check
	// If an object's base (z-bottom) is higher another's, it should be rendered after.
	// This check must be on the z-bottom and not the z-top because two objects with the
	// same z-position may have different heights (think of a mouse sorting vs the Avatar).
	if (si1.z < si2.z) return true;
	else if (si1.z > si2.z) return false;

	// Biased Clearly in z
	if ((si1.ztop + si1.z) / 2 <= si2.z) return true;
	else if (si1.z >= (si2.ztop + si2.z) / 2) return false;

	// Biased Clearly X
	if ((si1.x + si1.xleft) / 2 <= si2.xleft) return true;
	else if (si1.xleft >= (si2.x + si2.xleft) / 2) return false;

	// Biased Clearly Y
	if ((si1.y + si1.yfar) / 2 <= si2.yfar) return true;
	else if (si1.yfar >= (si2.y + si2.yfar) / 2) return false;

	// Partial in X + Y front
	if (si1.x + si1.y != si2.x + si2.y) return (si1.x + si1.y < si2.x + si2.y);

	// Partial in X + Y back
	if (si1.xleft + si1.yfar != si2.xleft + si2.yfar) return (si1.xleft + si1.yfar < si2.xleft + si2.yfar);

	// Partial in x?
	if (si1.x != si2.x) return si1.x < si2.x;

	// Partial in y?
	if (si1.y != si2.y) return si1.y < si2.y;

	// Just sort by shape number - not a number any more (is a pointer)
	if (si1.shape_num != si2.shape_num) return si1.shape_num < si2.shape_num;

	// And then by frame
	return si1.frame < si2.frame;
}

#define COMPARISON_RETURN(val,op,tab)                               \
	pout << tab"if (si1."#val" != si2."#val") -> ("             \
	     << si1.val << " != " << si2.val << ") -> "          \
	     << (si1.val != si2.val) << std::endl;               \
	pout << tab"{" << std::endl;                                \
	if (si1.val != si2.val)                                     \
	{                                                           \
		pout << tab"\treturn si1."#val" "#op" si2."#val"; -> (" \
		     << si1.val << " "#op" " << si2.val << ") -> "\
		     << (si1.val op si2.val) << std::endl;       \
		return si1.val op si2.val;                              \
	}                                                           \
	pout << tab"}" << std::endl;

#define COMPARISON_RETURN_EX(val1,op,val2,tab)                      \
	pout << tab"if ("#val1" != "#val2") -> ("                   \
	     << val1 << " != " << val2 << ") -> "                \
	     << (val1 != val2) << std::endl;                     \
	pout << tab"{" << std::endl;                                \
	if (val1 != val2)                                           \
	{                                                           \
		pout << tab"\treturn "#val1" "#op" "#val2"; -> ("       \
		     << val1 << " "#op" " << val2 << ") -> "     \
		     << (val1 op val2) << std::endl;             \
		return val1 op val2;                                    \
	}                                                           \
	pout << tab"}" << std::endl;

#define COMPARISON_RETURN_TF(val1,op,val2,tf,tab)                   \
	pout << tab "if ("#val1" "#op" "#val2") -> ("                   \
	     << val1 << " "#op" " << val2 << ") -> "             \
	     << (val1 op val2) << std::endl;                     \
	pout << tab"{" << std::endl;                                \
	if (val1 op val2)                                           \
	{                                                           \
		pout << tab"\treturn " << tf << std::endl;              \
		return tf;                                              \
	}                                                           \
	pout << tab"}" << std::endl;

inline bool SortItem::operator<<(const SortItem &si2) const {
	const SortItem &si1 = *this;

	if (si2.overlap(si1)) pout << "Overlaping" << std::endl;
	else {
		pout << "Not Overlaping" << std::endl;
		return false;
	}

	// Specialist z flat handling
	pout << "if (si1.flat && si2.flat) -> if ("
	     << si1.flat << " && " << si2.flat << ") -> "
	     << (si1.flat && si2.flat) << std::endl;
	pout << "{" << std::endl;

	if (si1.flat && si2.flat) {
		// Differing z is easy for flats
		//if (si1.ztop != si2.ztop) return si1.ztop < si2.ztop;
		COMPARISON_RETURN(ztop, <, "\t");

		// Equal z

		// Animated always gets drawn after
		//if (si1.anim != si2.anim) return si1.anim < si2.anim;
		COMPARISON_RETURN(anim, <, "\t");

		// Trans always gets drawn after
		//if (si1.trans != si2.trans) return si1.trans < si2.trans;
		COMPARISON_RETURN(trans, <, "\t");

		// Draw always gets drawn first
		//if (si1.draw != si2.draw) return si1.draw > si2.draw;
		COMPARISON_RETURN(draw, >, "\t");

		// Solid always gets drawn first
		//if (si1.solid != si2.solid) return si1.solid > si2.solid;
		COMPARISON_RETURN(solid, >, "\t");

		// Occludes always get drawn first
		//if (si1.occl != si2.occl) return si1.occl > si2.occl;
		COMPARISON_RETURN(occl, >, "\t");

		// 32x32 flats get drawn first
		//if (si1.f32x32 != si2.f32x32) return si1.f32x32 > si2.f32x32;
		COMPARISON_RETURN(f32x32, >, "\t");
	}
	// Mixed, or non flat
	else {
		pout << "}" << std::endl;
		pout << "else" << std::endl;
		pout << "{" << std::endl;

		// Clearly X, Y and Z  (useful?)
		//if (si1.x <= si2.xleft && si1.y <= si2.yfar && si1.ztop <= si2.z) return true;
		//else if (si1.xleft >= si2.x && si1.yfar >= si2.y && si1.z >= si2.ztop) return false;

		//int front1 = si1.x + si1.y;
		//int rear1 = si1.xleft + si1.yfar;
		//int front2 = si2.x + si2.y;
		//int rear2 = si2.xleft + si2.yfar;

		// Rear of object is infront of other's front
		//if (front1 <= rear2) return true;
		//else if (rear1 >= front2) return false;
		//COMPARISON_RETURN_TF(front1,<=,rear2,true,"\t");
		//COMPARISON_RETURN_TF(rear1,>=,front2,false,"\t");

		// Clearly in z
		//if (si1.ztop <= si2.z) return true;
		//else if (si1.z >= si2.ztop) return false;
		COMPARISON_RETURN_TF(si1.ztop, <=, si2.z, true, "\t");
		COMPARISON_RETURN_TF(si1.z, >=, si2.ztop, false, "\t");

		// Partial in z
		//if (si1.ztop != si2.ztop) return si1.ztop < si2.ztop;
	}
	pout << "}" << std::endl;

	// Clearly in x and y? (useful?)
	//if (si1.x <= si2.xleft && si1.y <= si2.yfar) return true;
	//else if (si1.xleft >= si2.x && si1.yfar >= si2.y) return false;

	// Clearly X
	//if (si1.x <= si2.xleft) return true;
	//else if (si1.xleft >= si2.x) return false;
	COMPARISON_RETURN_TF(si1.x, <=, si2.xleft, true, "\t");
	COMPARISON_RETURN_TF(si1.xleft, >=, si2.x, false, "\t");

	// Clearly Y
	//if (si1.y <= si2.yfar) return true;
	//else if (si1.yfar >= si2.y) return false;
	COMPARISON_RETURN_TF(si1.y, <=, si2.yfar, true, "\t");
	COMPARISON_RETURN_TF(si1.yfar, >=, si2.y, false, "\t");

	// Z base
	COMPARISON_RETURN_TF(si1.z, <, si2.z, true, "");
	COMPARISON_RETURN_TF(si1.z, >, si2.z, false, "");

	// Biased Clearly in z
	//if ((si1.ztop+si1.z)/2 <= si2.z) return true;
	//else if (si1.z >= (si2.ztop+si2.z)/2) return false;
	COMPARISON_RETURN_TF((si1.ztop + si1.z) / 2, <=, si2.z, true, "");
	COMPARISON_RETURN_TF(si1.z, >=, (si2.ztop + si2.z) / 2, false, "");

	// Biased Clearly X
	//if ((si1.x+si1.xleft)/2 <= si2.xleft) return true;
	//else if (si1.xleft >= (si2.x+si2.xleft)/2) return false;
	COMPARISON_RETURN_TF((si1.x + si1.xleft) / 2, <=, si2.xleft, true, "");
	COMPARISON_RETURN_TF(si1.xleft, >=, (si2.x + si2.xleft) / 2, false, "");

	// Biased Clearly Y
	//if ((si1.y+si1.yfar)/2 <= si2.yfar) return true;
	//else if (si1.yfar >= (si2.y+si2.yfar)/2) return false;
	COMPARISON_RETURN_TF((si1.y + si1.yfar) / 2, <=, si2.yfar, true, "");
	COMPARISON_RETURN_TF(si1.yfar, >=, (si2.y + si2.yfar) / 2, false, "");

	// Partial in X + Y front
	//if (si1.x + si1.y != si2.x + si2.y) return (si1.x + si1.y < si2.x + si2.y);
	COMPARISON_RETURN_EX(si1.x + si1.y, <, si2.x + si2.y, "");

	// Partial in X + Y back
	//if (si1.xleft + si1.yfar != si2.xleft + si2.yfar) return (si1.xleft + si1.yfar < si2.xleft + si2.yfar);
	COMPARISON_RETURN_EX(si1.xleft + si1.yfar, <, si2.xleft + si2.yfar, "");

	// Partial in x?
	//if (si1.x != si2.x) return si1.x < si2.x;
	COMPARISON_RETURN(x, <, "");

	// Partial in y?
	//if (si1.y != si2.y) return si1.y < si2.y;
	COMPARISON_RETURN(y, <, "");

	// Just sort by shape number
//	if (si1.shape_num != si2.shape_num) return si1.shape_num < si2.shape_num;
	COMPARISON_RETURN(shape_num, <, "");

	// And then by frame
	//return si1.frame < si2.frame;
	COMPARISON_RETURN(frame, <, "");
	return 0;
}


//
// ItemSorter
//

ItemSorter::ItemSorter() :
	shapes(0), surf(0), items(0), items_tail(0), items_unused(0), sort_limit(0) {
	int i = 2048;
	while (i--) items_unused = new SortItem(items_unused);
}

ItemSorter::~ItemSorter() {
	//
	if (items_tail) {
		items_tail->next = items_unused;
		items_unused = items;
	}
	items = 0;
	items_tail = 0;

	while (items_unused) {
		SortItem *next = items_unused->next;
		delete items_unused;
		items_unused = next;
	}

	delete [] items;
}

void ItemSorter::BeginDisplayList(RenderSurface *rs,
                                  int32 camx, int32 camy, int32 camz) {
	// Get the shapes, if required
	if (!shapes) shapes = GameData::get_instance()->getMainShapes();

	//
	if (items_tail) {
		items_tail->next = items_unused;
		items_unused = items;
	}
	items = 0;
	items_tail = 0;

	// Set the RenderSurface, and reset the item list
	surf = rs;
	order_counter = 0;

	// Screenspace bounding box bottom x coord (RNB x coord)
	cam_sx = (camx - camy) / 4;
	// Screenspace bounding box bottom extent  (RNB y coord)
	cam_sy = (camx + camy) / 8 - camz;
}

void ItemSorter::AddItem(int32 x, int32 y, int32 z, uint32 shape_num, uint32 frame_num, uint32 flags, uint32 ext_flags, uint16 item_num) {
	//if (z > skip_lift) return;
	//if (Application::tgwds && shape == 538) return;

	// First thing, get a SortItem to use (first of unused)
	if (!items_unused) items_unused = new SortItem(0);
	SortItem *si = items_unused;

	si->item_num = item_num;
	si->shape = shapes->getShape(shape_num);
	si->shape_num = shape_num;
	si->frame = frame_num;
	ShapeFrame *frame = si->shape->getFrame(si->frame);
	if (!frame) {
		perr << "Invalid shape: " << si->shape_num << "," << si->frame
		     << std::endl;
		return;
	}

	ShapeInfo *info = shapes->getShapeInfo(shape_num);

	//if (info->is_editor && !show_editor_items) return;
	//if (info->z > shape_max_height) return;

	// Dimensions
	int32 xd, yd, zd;
	si->flags = flags;
	si->ext_flags = ext_flags;

	// X and Y are flipped
	if (si->flags & Item::FLG_FLIPPED) {
		xd = info->y * 32;  // Multiply by 32 to get actual world size
		yd = info->x * 32;  // Multiply by 32 to get actual world size
	} else {
		xd = info->x * 32;  // Multiply by 32 to get actual world size
		yd = info->y * 32;  // Multiply by 32 to get actual world size
	}

	zd = info->z * 8;   // Multiply by 8 to get actual world size

	// Worldspace bounding box
	si->x = x;
	si->y = y;
	si->z = z;
	si->xleft = si->x - xd;
	si->yfar = si->y - yd;
	si->ztop = si->z + zd;

	// Screenspace bounding box left extent    (LNT x coord)
	si->sxleft = si->xleft / 4 - si->y / 4 - cam_sx;
	// Screenspace bounding box right extent   (RFT x coord)
	si->sxright = si->x / 4 - si->yfar / 4 - cam_sx;

	// Screenspace bounding box top x coord    (LFT x coord)
	si->sxtop = si->xleft / 4 - si->yfar / 4 - cam_sx;
	// Screenspace bounding box top extent     (LFT y coord)
	si->sytop = si->xleft / 8 + si->yfar / 8 - si->ztop - cam_sy;

	// Screenspace bounding box bottom x coord (RNB x coord)
	si->sxbot = si->x / 4 - si->y / 4 - cam_sx;
	// Screenspace bounding box bottom extent  (RNB y coord)
	si->sybot = si->x / 8 + si->y / 8 - si->z - cam_sy;

//	si->sxleft += swo2;
//	si->sxright += swo2;
//	si->sxbot += swo2;
//	si->sxtop += swo2;

//	si->sytop += sho2;
//	si->sybot += sho2;

	// Real Screenspace coords
	si->sx = si->sxbot - frame->xoff;   // Left
	si->sy = si->sybot - frame->yoff;   // Top
	si->sx2 = si->sx + frame->width;    // Right
	si->sy2 = si->sy + frame->height;   // Bottom

	// Do Clipping here
	si->clipped = surf->CheckClipped(Rect(si->sx, si->sy, frame->width, frame->height));
	if (si->clipped < 0) return;

	// These help out with sorting. We calc them now, so it will be faster
	si->f32x32 = xd == 128 && yd == 128;
	si->flat = zd == 0;

	/*
	if (Application::tgwds) {
	    si->draw = false;
	    si->solid = false;
	    si->occl = false;
	    si->roof = false;
	    si->noisy = false;
	    si->anim = false;
	    si->trans = false;
	}
	else
	*/
	{
		si->draw = info->is_draw();
		si->solid = info->is_solid();
		si->occl = info->is_occl() && !(si->flags & Item::FLG_INVISIBLE) &&
		           !(si->ext_flags & Item::EXT_TRANSPARENT);
		si->roof = info->is_roof();
		si->noisy = info->is_noisy();
		si->anim = info->animtype != 0;
		si->trans = info->is_translucent();
		si->fixed = info->is_fixed();
		si->land = info->is_land();
	}

	si->occluded = false;
	si->order = -1;

	// We will clear all the vector memory
	// Stictly speaking the vector will sort of leak memory, since they
	// are never deleted
	si->depends.clear();
	//si->depends.erase(si->depends.begin(), si->depends.end());    // MSVC.Netism

	// Iterate the list and compare shapes

	// Ok,
	SortItem *addpoint = 0;
	for (SortItem *si2 = items; si2 != 0; si2 = si2->next) {
		// Get the insert point... which is before the first item that has higher z than us
		if (!addpoint && si->ListLessThan(si2)) addpoint = si2;

		// Doesn't overlap
		if (si2->occluded || !si->overlap(*si2)) continue;

		// Attempt to find which is infront
		if (*si < *si2) {
			// si2 occludes si (us)
			if (si2->occl && si2->occludes(*si)) {
				// No need to do any more checks, this isn't visible
				si->occluded = true;
				break;
			}

			// si1 is behind si2, so add it to si2's dependency list
			si2->depends.insert_sorted(si);
		} else {
			// ss occludes si2. Sadly, we can't remove it from the list.
			if (si->occl && si->occludes(*si2)) si2->occluded = true;
			// si2 is behind si1, so add it to si1's dependency list
			else si->depends.push_back(si2);
		}
	}

	// Add it to the list
	items_unused = items_unused->next;

	// have a position
	//addpoint = 0;
	if (addpoint) {
		si->next = addpoint;
		si->prev = addpoint->prev;
		addpoint->prev = si;
		if (si->prev) si->prev->next = si;
		else items = si;
	}
	// Add it to the end of the list
	else {
		if (items_tail) items_tail->next = si;
		if (!items) items = si;
		si->next = 0;
		si->prev = items_tail;
		items_tail = si;
	}
}

void ItemSorter::AddItem(Item *add) {
#if 0

	int32 x, y, z;
	add->getLerped(x, y, z);
	AddItem(x, y, z, add->getShape(), add->getFrame(), add->getFlags(), add->getObjId());

#else

	//if (add->iz > skip_lift) return;
	//if (Application::tgwds && shape == 538) return;

	// First thing, get a SortItem to use
	if (!items_unused) items_unused = new SortItem(0);
	SortItem *si = items_unused;

	si->item_num = add->getObjId();
	si->shape = add->getShapeObject();
	si->shape_num = add->getShape();
	si->frame = add->getFrame();
	ShapeFrame *frame = si->shape->getFrame(si->frame);
	if (!frame) {
		perr << "Invalid shape: " << si->shape_num << "," << si->frame
		     << std::endl;
		return;
	}

	ShapeInfo *info = add->getShapeInfo();

	//if (info->is_editor && !show_editor_items) return;
	//if (info->z > shape_max_height) return;

	// Dimensions
	int32 xd, yd, zd;
	si->flags = add->getFlags();
	si->ext_flags = add->getExtFlags();

	// X and Y are flipped
	if (si->flags & Item::FLG_FLIPPED) {
		xd = info->y * 32;  // Multiply by 32 to get actual world size
		yd = info->x * 32;  // Multiply by 32 to get actual world size
	} else {
		xd = info->x * 32;  // Multiply by 32 to get actual world size
		yd = info->y * 32;  // Multiply by 32 to get actual world size
	}

	zd = info->z * 8;   // Multiply by 8 to get actual world size

	// Worldspace bounding box
	add->getLerped(si->x, si->y, si->z);
	si->xleft = si->x - xd;
	si->yfar = si->y - yd;
	si->ztop = si->z + zd;

	// Screenspace bounding box left extent    (LNT x coord)
	si->sxleft = si->xleft / 4 - si->y / 4 - cam_sx;
	// Screenspace bounding box right extent   (RFT x coord)
	si->sxright = si->x / 4 - si->yfar / 4 - cam_sx;

	// Screenspace bounding box top x coord    (LFT x coord)
	si->sxtop = si->xleft / 4 - si->yfar / 4 - cam_sx;
	// Screenspace bounding box top extent     (LFT y coord)
	si->sytop = si->xleft / 8 + si->yfar / 8 - si->ztop - cam_sy;

	// Screenspace bounding box bottom x coord (RNB x coord)
	si->sxbot = si->x / 4 - si->y / 4 - cam_sx;
	// Screenspace bounding box bottom extent  (RNB y coord)
	si->sybot = si->x / 8 + si->y / 8 - si->z - cam_sy;

//	si->sxleft += swo2;
//	si->sxright += swo2;
//	si->sxbot += swo2;
//	si->sxtop += swo2;

//	si->sytop += sho2;
//	si->sybot += sho2;

	// Real Screenspace coords
	si->sx = si->sxbot - frame->xoff;   // Left
	si->sy = si->sybot - frame->yoff;   // Top
	si->sx2 = si->sx + frame->width;    // Right
	si->sy2 = si->sy + frame->height;   // Bottom

	// Do Clipping here
	si->clipped = surf->CheckClipped(Rect(si->sx, si->sy, frame->width, frame->height));
	if (si->clipped < 0) return;

	// These help out with sorting. We calc them now, so it will be faster
	si->f32x32 = xd == 128 && yd == 128;
	si->flat = zd == 0;

	/*
	if (Application::tgwds) {
	    si->draw = false;
	    si->solid = false;
	    si->occl = false;
	    si->roof = false;
	    si->noisy = false;
	    si->anim = false;
	    si->trans = false;
	}
	else
	*/
	{
		si->draw = info->is_draw();
		si->solid = info->is_solid();
		si->occl = info->is_occl() && !(si->flags & Item::FLG_INVISIBLE) &&
		           !(si->ext_flags & Item::EXT_TRANSPARENT);
		si->roof = info->is_roof();
		si->noisy = info->is_noisy();
		si->anim = info->animtype != 0;
		si->trans = info->is_translucent();
		si->fixed = info->is_fixed();
		si->land = info->is_land();
	}

	si->occluded = false;
	si->order = -1;

	// We will clear all the vector memory
	// Stictly speaking the vector will sort of leak memory, since they
	// are never deleted
	si->depends.clear();
	//si->depends.erase(si->depends.begin(), si->depends.end());    // MSVC.Netism

	// Iterate the list and compare shapes

	// Ok,
	SortItem *addpoint = 0;
	for (SortItem *si2 = items; si2 != 0; si2 = si2->next) {
		// Get the insert point... which is before the first item that has higher z than us
		if (!addpoint && si->ListLessThan(si2)) addpoint = si2;

		// Doesn't overlap
		if (si2->occluded || !si->overlap(*si2)) continue;

		// Attempt to find which is infront
		if (*si < *si2) {
			// si2 occludes ss
			if (si2->occl && si2->occludes(*si)) {
				// No need to do any more checks, this isn't visible
				si->occluded = true;
				break;
			}

			// si1 is behind si2, so add it to si2's dependency list
			si2->depends.insert_sorted(si);
		} else {
			// ss occludes si2. Sadly, we can't remove it from the list.
			if (si->occl && si->occludes(*si2)) si2->occluded = true;
			// si2 is behind si1, so add it to si1's dependency list
			else si->depends.push_back(si2);
		}
	}

	// Add it to the list
	items_unused = items_unused->next;

	// have a position
	//addpoint = 0;
	if (addpoint) {
		si->next = addpoint;
		si->prev = addpoint->prev;
		addpoint->prev = si;
		if (si->prev) si->prev->next = si;
		else items = si;
	}
	// Add it to the end of the list
	else {
		if (items_tail) items_tail->next = si;
		if (!items) items = si;
		si->next = 0;
		si->prev = items_tail;
		items_tail = si;
	}
#endif
}

SortItem *prev = 0;

void ItemSorter::PaintDisplayList(bool item_highlight) {
	prev = 0;
	SortItem *it = items;
	SortItem *end = 0;
	order_counter = 0;  // Reset the order_counter
	while (it != end) {
		if (it->order == -1) if (PaintSortItem(it)) return;
		it = it->next;
	}

	// Item highlighting. We redraw each 'item' transparent
	if (item_highlight) {
		it = items;
		while (it != end) {
			if (!(it->flags & (Item::FLG_DISPOSABLE | Item::FLG_FAST_ONLY)) && !it->fixed) {
				surf->PaintHighlightInvis(it->shape,
				                          it->frame,
				                          it->sxbot,
				                          it->sybot,
				                          it->trans,
				                          (it->flags & Item::FLG_FLIPPED) != 0, 0x1f00ffff);
			}

			it = it->next;
		}

	}
}

bool ItemSorter::PaintSortItem(SortItem *si) {
	// Don't paint this, or dependencies if occluded
	if (si->occluded) return false;

	// Resursion, detection
	si->order = -2;

	// Iterate through our dependancies, and paint them, if possible
	SortItem::DependsList::iterator it = si->depends.begin();
	SortItem::DependsList::iterator end = si->depends.end();
	while (it != end) {
		// Well, it can't. Implies infinite recursive sorting.
		//if ((*it)->order == -2) CANT_HAPPEN_MSG("Detected cycle in the dependency graph");

		if ((*it)->order == -1) if (PaintSortItem((*it))) return true;

		++it;
	}

	// Set our painting order
	si->order = order_counter;
	order_counter++;

	// Now paint us!

//	if (wire) si->info->draw_box_back(s, dispx, dispy, 255);

	if (si->ext_flags & Item::EXT_HIGHLIGHT && si->ext_flags & Item::EXT_TRANSPARENT)
		surf->PaintHighlightInvis(si->shape, si->frame, si->sxbot, si->sybot, si->trans, (si->flags & Item::FLG_FLIPPED) != 0, 0x7F00007F);
	if (si->ext_flags & Item::EXT_HIGHLIGHT)
		surf->PaintHighlight(si->shape, si->frame, si->sxbot, si->sybot, si->trans, (si->flags & Item::FLG_FLIPPED) != 0, 0x7F00007F);
	else if (si->ext_flags & Item::EXT_TRANSPARENT)
		surf->PaintInvisible(si->shape, si->frame, si->sxbot, si->sybot, si->trans, (si->flags & Item::FLG_FLIPPED) != 0);
	else if (si->flags & Item::FLG_FLIPPED)
		surf->PaintMirrored(si->shape, si->frame, si->sxbot, si->sybot, si->trans);
	else if (si->trans)
		surf->PaintTranslucent(si->shape, si->frame, si->sxbot, si->sybot);
	else if (!si->clipped)
		surf->PaintNoClip(si->shape, si->frame, si->sxbot, si->sybot);
	else
		surf->Paint(si->shape, si->frame, si->sxbot, si->sybot);

//	if (wire) si->info->draw_box_front(s, dispx, dispy, 255);

	// weapon overlay
	// FIXME: use highlight/invisibility, also add to Trace() ?
	if (si->shape_num == 1 && si->item_num == 1) {
		MainActor *av = getMainActor();
		const WeaponOverlayFrame *wo_frame = 0;
		uint32 wo_shapenum;
		av->getWeaponOverlay(wo_frame, wo_shapenum);
		if (wo_frame) {
			Shape *wo_shape = GameData::get_instance()->getMainShapes()->getShape(wo_shapenum);
			surf->Paint(wo_shape, wo_frame->frame,
			            si->sxbot + wo_frame->xoff,
			            si->sybot + wo_frame->yoff);
		}
	}

	if (sort_limit) {
		if (order_counter == sort_limit) {
			static uint32 previt = 0;
			int x1 = si->xleft;
			int y1 = si->yfar;
			int z2 = si->ztop;
			if (!previt || previt != si->item_num) {
				previt = si->item_num;
				pout << si->shape_num << ":" << si->frame << " (" << x1 << "," << y1 << "," << si->z << ") (" << si->x << "," << si->y << "," << z2 << ")" << std::endl;
				//  ss->info->print();
				if (prev) *prev << *si;
			}
			return true;
		}
		prev = si;
	}

	return false;
}

bool ItemSorter::NullPaintSortItem(SortItem *si) {
	// Don't paint this, or dependencies if occluded
	if (si->occluded) return false;

	// Resursion, detection
	si->order = -2;

	// Iterate through our dependancies, and paint them, if possible
	SortItem::DependsList::iterator it = si->depends.begin();
	SortItem::DependsList::iterator end = si->depends.end();
	while (it != end) {
		// Well, it can't. Implies recursive sorting. Can happen though so
		// you had best leave this commented out
		//if ((*it)->order == -2) CANT_HAPPEN_MSG("Recursive item sorting");

		if ((*it)->order == -1) if (NullPaintSortItem((*it))) return true;

		++it;
	}

	// Set our painting/sorting order
	si->order = order_counter;
	order_counter++;

	return false;
}

uint16 ItemSorter::Trace(int32 x, int32 y, HitFace *face, bool item_highlight) {
	SortItem *it;
	SortItem *selected;

	if (!order_counter) { // If no order_counter we need to sort the items
		it = items;
		order_counter = 0;  // Reset the order_counter
		while (it != 0) {
			if (it->order == -1) if (NullPaintSortItem(it)) break;

			it = it->next;
		}
	}

	// Firstly, we check for highlighted items
	selected = 0;

	if (item_highlight) {
		it = items_tail;
		selected = 0;

		for (it = items_tail; it != 0; it = it->prev) {
			if (!(it->flags & (Item::FLG_DISPOSABLE | Item::FLG_FAST_ONLY)) && !it->fixed) {

				if (!it->item_num) continue;

				// Doesn't Overlap
				if (x < it->sx || x >= it->sx2 || y < it->sy || y >= it->sy2) continue;

				// Now check the frame itself
				ShapeFrame *frame = it->shape->getFrame(it->frame);
				assert(frame); // invalid frames shouldn't have been added to the list

				// Nope, doesn't have a point
				if (it->flags & Item::FLG_FLIPPED) {
					if (!frame->hasPoint(it->sxbot - x, y - it->sybot)) continue;
				} else {
					if (!frame->hasPoint(x - it->sxbot, y - it->sybot)) continue;
				}

				// Ok now check against selected
				selected = it;
			}
		}

	}

	// Ok, this is all pretty simple. We iterate all the items.
	// We then check to see if the item has a point where the trace goes.
	// Finally we then set the selected SortItem if it's 'order' is highest

	if (!selected) for (it = items; it != 0; it = it->next) {
			if (!it->item_num) continue;

			// Doesn't Overlap
			if (x < it->sx || x >= it->sx2 || y < it->sy || y >= it->sy2) continue;

			// Now check the frame itself
			ShapeFrame *frame = it->shape->getFrame(it->frame);
			assert(frame); // invalid frames shouldn't have been added to the list

			// Nope, doesn't have a point
			if (it->flags & Item::FLG_FLIPPED) {
				if (!frame->hasPoint(it->sxbot - x, y - it->sybot)) continue;
			} else {
				if (!frame->hasPoint(x - it->sxbot, y - it->sybot)) continue;
			}

			// Ok now check against selected
			if (!selected || (it->order > selected->order)) selected = it;
		}

	if (selected) {

		if (face) {
			// shortcut for zero-height items
			if (selected->ztop == selected->z) {
				*face = Z_FACE;
			} else {
				// determine face that was hit

				// RNT coordinates
				int32 RNTx = selected->sxbot;
				int32 RNTy = selected->sybot - selected->ztop + selected->z;

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

		return selected->item_num;
	}

	return 0;
}

} // End of namespace Ultima8
