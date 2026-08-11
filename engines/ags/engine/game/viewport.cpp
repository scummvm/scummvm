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

#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/game/room_struct.h"
#include "ags/engine/game/viewport.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

void Camera::SetID(int id) {
	_id = id;
}

// Returns Room camera position and size inside the room (in room coordinates)
const Rect &Camera::GetRect() const {
	return _position;
}

// Sets explicit room camera's orthographic size
void Camera::SetSize(const Size cam_size) {
	// TODO: currently we don't support having camera larger than room background
	// (or rather - looking outside of the room background); look into this later
	const Size real_room_sz = (_G(displayed_room) >= 0 && (_GP(thisroom).Width > 0 && _GP(thisroom).Height > 0)) ?
							  Size(data_to_game_coord(_GP(thisroom).Width), data_to_game_coord(_GP(thisroom).Height)) :
							  Size(INT32_MAX, INT32_MAX);

	const Size real_size = Size::Clamp(cam_size, Size(1, 1), real_room_sz);
	if (_position.GetWidth() == real_size.Width && _position.GetHeight() == real_size.Height)
		return;

	_position.SetWidth(real_size.Width);
	_position.SetHeight(real_size.Height);
	SetAt(_position.Left, _position.Top); // readjust in case went off-room after size changed
	for (auto vp = _viewportRefs.begin(); vp != _viewportRefs.end(); ++vp) {
		auto locked_vp = vp->lock();
		if (locked_vp)
			locked_vp->AdjustTransformation();
	}
	_hasChangedSize = true;
}

// Puts room camera to the new location in the room
void Camera::SetAt(int x, int y) {
	int cw = _position.GetWidth();
	int ch = _position.GetHeight();
	int room_width = data_to_game_coord(_GP(thisroom).Width);
	int room_height = data_to_game_coord(_GP(thisroom).Height);
	x = Math::Clamp(x, 0, room_width - cw);
	y = Math::Clamp(y, 0, room_height - ch);
	if (_position.Left == x && _position.Top == y)
		return;
	_position.MoveTo(Point(x, y));
	_hasChangedPosition = true;
}

// Tells if camera is currently locked at custom position
bool Camera::IsLocked() const {
	return _locked;
}

// Locks room camera at its current position
void Camera::Lock() {
	debug_script_log("Room camera locked");
	_locked = true;
}

// Similar to SetAt, but also locks camera preventing it from following player character
void Camera::LockAt(int x, int y) {
	debug_script_log("Room camera locked to %d,%d", x, y);
	SetAt(x, y);
	_locked = true;
}

// Releases camera lock, letting it follow player character
void Camera::Release() {
	_locked = false;
	debug_script_log("Room camera released back to engine control");
}

// Link this camera to a new viewport; this does not unlink any linked ones
void Camera::LinkToViewport(ViewportRef viewport) {
	auto new_locked = viewport.lock();
	if (!new_locked)
		return;
	for (auto vp = _viewportRefs.begin(); vp != _viewportRefs.end(); ++vp) {
		auto old_locked = vp->lock();
		if (old_locked->GetID() == new_locked->GetID())
			return;
	}
	_viewportRefs.push_back(viewport);
}

// Unlinks this camera from a given viewport; does nothing if link did not exist
void Camera::UnlinkFromViewport(int id) {
	for (auto vp = _viewportRefs.begin(); vp != _viewportRefs.end(); ++vp) {
		auto locked = vp->lock();
		if (locked && locked->GetID() == id) {
			_viewportRefs.erase(vp);
			return;
		}
	}
}

const std::vector<ViewportRef> &Camera::GetLinkedViewports() const {
	return _viewportRefs;
}

void Viewport::SetID(int id) {
	_id = id;
}

void Viewport::SetRect(const Rect &rc) {
	// TODO: consider allowing size 0,0, in which case viewport is considered not visible
	Size fix_size = rc.GetSize().IsNull() ? Size(1, 1) : rc.GetSize();
	Rect new_pos = RectWH(rc.Left, rc.Top, fix_size.Width, fix_size.Height);
	if (new_pos == _position)
		return;
	_position = new_pos;
	AdjustTransformation();
	_hasChangedPosition = true;
	_hasChangedSize = true;
}

void Viewport::SetSize(const Size sz) {
	// TODO: consider allowing size 0,0, in which case viewport is considered not visible
	Size fix_size = sz.IsNull() ? Size(1, 1) : sz;
	if (_position.GetWidth() == fix_size.Width && _position.GetHeight() == fix_size.Height)
		return;
	_position = RectWH(_position.Left, _position.Top, fix_size.Width, fix_size.Height);
	AdjustTransformation();
	_hasChangedSize = true;
}

void Viewport::SetAt(int x, int y) {
	if (_position.Left == x && _position.Top == y)
		return;
	_position.MoveTo(Point(x, y));
	AdjustTransformation();
	_hasChangedPosition = true;
}

void Viewport::SetVisible(bool on) {
	_visible = on;
	_hasChangedVisible = true;
}

void Viewport::SetZOrder(int zorder) {
	_zorder = zorder;
	_hasChangedVisible = true;
}

void Viewport::AdjustTransformation() {
	auto locked_cam = _camera.lock();
	if (locked_cam)
		_transform.Init(locked_cam->GetRect().GetSize(), _position);
}

PCamera Viewport::GetCamera() const {
	return _camera.lock();
}

void Viewport::LinkCamera(PCamera cam) {
	_camera = cam;
	AdjustTransformation();
}

VpPoint Viewport::RoomToScreen(int roomx, int roomy, bool clip) const {
	auto cam = _camera.lock();
	if (!cam)
		return std::make_pair(Point(), -1);
	const Rect &camr = cam->GetRect();
	Point screen_pt = _transform.Scale(Point(roomx - camr.Left, roomy - camr.Top));
	if (clip && !_position.IsInside(screen_pt))
		return std::make_pair(Point(), -1);
	return std::make_pair(screen_pt, _id);
}

VpPoint Viewport::ScreenToRoom(int scrx, int scry, bool clip, bool convert_cam_to_data) const {
	Point screen_pt(scrx, scry);
	if (clip && !_position.IsInside(screen_pt))
		return std::make_pair(Point(), -1);
	auto cam = _camera.lock();
	if (!cam)
		return std::make_pair(Point(), -1);

	const Rect &camr = cam->GetRect();
	Point p = _transform.UnScale(screen_pt);
	if (convert_cam_to_data) {
		p.X += game_to_data_coord(camr.Left);
		p.Y += game_to_data_coord(camr.Top);
	} else {
		p.X += camr.Left;
		p.Y += camr.Top;
	}
	return std::make_pair(p, _id);
}

} // namespace AGS3
