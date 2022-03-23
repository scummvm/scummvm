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

#include "ags/shared/util/geometry.h"
#include "ags/lib/std/algorithm.h"

namespace AGS3 {

bool AreRectsIntersecting(const Rect &r1, const Rect &r2) { // NOTE: remember that in AGS Y axis is pointed downwards
	return r1.Left <= r2.Right && r1.Right >= r2.Left &&
	       r1.Top <= r2.Bottom && r1.Bottom >= r2.Top;
}

bool IsRectInsideRect(const Rect &place, const Rect &item) {
	return item.Left >= place.Left && item.Right <= place.Right &&
	       item.Top >= place.Top && item.Bottom <= place.Bottom;
}

float DistanceBetween(const Rect &r1, const Rect &r2) {
	// https://gamedev.stackexchange.com/a/154040
	Rect rect_outer(
	    std::min(r1.Left, r2.Left),
	    std::min(r1.Top, r2.Top),
	    std::max(r1.Right, r2.Right),
	    std::max(r1.Bottom, r2.Bottom)
	);
	int inner_width = std::max(0, rect_outer.GetWidth() - r1.GetWidth() - r2.GetWidth());
	int inner_height = std::max(0, rect_outer.GetHeight() - r1.GetHeight() - r2.GetHeight());
	return static_cast<float>(std::sqrt((inner_width ^ 2) + (inner_height ^ 2)));
}

Size ProportionalStretch(int dest_w, int dest_h, int item_w, int item_h) {
	int width = item_w ? dest_w : 0;
	int height = item_w ? (dest_w * item_h / item_w) : 0;
	if (height > dest_h) {
		width = item_h ? (dest_h * item_w / item_h) : 0;
		height = dest_h;
	}
	return Size(width, height);
}

Size ProportionalStretch(const Size &dest, const Size &item) {
	return ProportionalStretch(dest.Width, dest.Height, item.Width, item.Height);
}

int AlignInHRange(int x1, int x2, int off_x, int width, FrameAlignment align) {
	if (align & kMAlignRight)
		return off_x + x2 - width;
	else if (align & kMAlignHCenter)
		return off_x + x1 + ((x2 - x1 + 1) >> 1) - (width >> 1);
	return off_x + x1; // kAlignLeft is default
}

int AlignInVRange(int y1, int y2, int off_y, int height, FrameAlignment align) {
	if (align & kMAlignBottom)
		return off_y + y2 - height;
	else if (align & kMAlignVCenter)
		return off_y + y1 + ((y2 - y1 + 1) >> 1) - (height >> 1);
	return off_y + y1; // kAlignTop is default
}

Rect AlignInRect(const Rect &frame, const Rect &item, FrameAlignment align) {
	int x = AlignInHRange(frame.Left, frame.Right, item.Left, item.GetWidth(), align);
	int y = AlignInVRange(frame.Top, frame.Bottom, item.Top, item.GetHeight(), align);

	Rect dst_item = item;
	dst_item.MoveTo(Point(x, y));
	return dst_item;
}

Rect OffsetRect(const Rect &r, const Point off) {
	return Rect(r.Left + off.X, r.Top + off.Y, r.Right + off.X, r.Bottom + off.Y);
}

Rect CenterInRect(const Rect &place, const Rect &item) {
	return RectWH((place.GetWidth() >> 1) - (item.GetWidth() >> 1),
	              (place.GetHeight() >> 1) - (item.GetHeight() >> 1),
	              item.GetWidth(), item.GetHeight());
}

Rect ClampToRect(const Rect &place, const Rect &item) {
	return Rect(
	           AGSMath::Clamp(item.Left, place.Left, place.Right),
	           AGSMath::Clamp(item.Top, place.Top, place.Bottom),
	           AGSMath::Clamp(item.Right, place.Left, place.Right),
	           AGSMath::Clamp(item.Bottom, place.Top, place.Bottom)
	       );
}

Rect PlaceInRect(const Rect &place, const Rect &item, const RectPlacement &placement) {
	switch (placement) {
	case kPlaceCenter:
		return CenterInRect(place, item);
	case kPlaceStretch:
		return place;
	case kPlaceStretchProportional:
		return CenterInRect(place,
		                    RectWH(ProportionalStretch(place.GetWidth(), place.GetHeight(), item.GetWidth(), item.GetHeight())));
	default:
		return RectWH(place.Left + item.Left, place.Top + item.Top, item.GetWidth(), item.GetHeight());
	}
}

} // namespace AGS3
