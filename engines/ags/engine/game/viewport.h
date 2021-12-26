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

//=============================================================================
//
// Definition for the game viewports and cameras.
//
//=============================================================================

#ifndef AGS_ENGINE_GAME_VIEWPORT_H
#define AGS_ENGINE_GAME_VIEWPORT_H

#include "ags/lib/std/memory.h"
#include "ags/lib/std/vector.h"
#include "ags/shared/util/geometry.h"
#include "ags/shared/util/scaling.h"

namespace AGS3 {

class Camera;
class Viewport;

typedef std::shared_ptr<Camera> PCamera;
typedef std::shared_ptr<Viewport> PViewport;
typedef std::weak_ptr<Camera> CameraRef;
typedef std::weak_ptr<Viewport> ViewportRef;

// TODO: move to utility header
// From https://stackoverflow.com/questions/45507041/how-to-check-if-weak-ptr-is-empty-non-assigned
// Tests that weak_ptr is empty (was not initialized with valid reference)
template <typename T>
bool is_uninitialized(std::weak_ptr<T> const &weak) {
	using wt = std::weak_ptr<T>;
	return !weak.owner_before(wt{}) &&!wt{} .owner_before(weak);
}


// Camera defines a "looking eye" inside the room, its position and size.
// It does not render anywhere on its own, instead it is linked to a viewport
// and latter draws what that camera sees.
// One camera may be linked to multiple viewports.
// Camera does not move on its own, this is performed by separate behavior
// algorithm. But it provides "lock" property that tells if its position is
// currently owned by user script.
class Camera {
public:
	// Gets camera ID (serves as an index)
	inline int GetID() const {
		return _id;
	}
	// Sets new camera ID
	void SetID(int id);
	// Returns Room camera position and size inside the room (in room coordinates)
	const Rect &GetRect() const;
	// Sets explicit room camera's orthographic size
	void SetSize(const Size sz);
	// Puts room camera to the new location in the room
	void SetAt(int x, int y);
	// Tells if camera is currently locked at custom position
	bool IsLocked() const;
	// Locks room camera at its current position
	void Lock();
	// Similar to SetAt, but also locks camera preventing it from following player character
	void LockAt(int x, int y);
	// Releases camera lock, letting it follow player character
	void Release();

	// Link this camera to a new viewport; this does not unlink any linked ones
	void LinkToViewport(ViewportRef viewport);
	// Unlinks this camera from a given viewport; does nothing if link did not exist
	void UnlinkFromViewport(int id);
	// Get the array of linked viewport references
	const std::vector<ViewportRef> &GetLinkedViewports() const;

	// Tell if this camera has changed recently
	inline bool HasChangedPosition() const {
		return _hasChangedPosition;
	}
	inline bool HasChangedSize() const {
		return _hasChangedSize;
	}
	// Clears the changed flags
	void ClearChangedFlags() {
		_hasChangedPosition = false;
		_hasChangedSize = false;
	}

private:
	int _id = -1;
	// Actual position and orthographic size
	Rect _position;
	// Locked or following player automatically
	bool _locked = false;
	// Linked viewport refs, used to notify viewports of camera changes
	std::vector<ViewportRef> _viewportRefs;
	// Flags that tell whether this camera's position on screen has changed recently
	bool _hasChangedPosition = false;
	bool _hasChangedSize = false;
};


// A result of coordinate conversion between screen and the room,
// tells which viewport was used to pass the "touch" through.
typedef std::pair<Point, int> VpPoint;


// Viewport class defines a rectangular area on game screen where the contents
// of a Camera are rendered.
// Viewport may have one linked camera at a time.
class Viewport {
public:
	// Gets viewport ID (serves as an index)
	inline int GetID() const {
		return _id;
	}
	// Sets new viewport ID
	void SetID(int id);
	// Returns viewport's position on screen
	inline const Rect &GetRect() const {
		return _position;
	}
	// Returns viewport's room-to-screen transformation
	inline const AGS::Shared::PlaneScaling &GetTransform() const {
		return _transform;
	}
	// Set viewport's rectangle on screen
	void SetRect(const Rect &rc);
	// Sets viewport size
	void SetSize(const Size sz);
	// Sets viewport's position on screen
	void SetAt(int x, int y);

	// Tells whether viewport content is rendered on screen
	bool IsVisible() const {
		return _visible;
	}
	// Changes viewport visibility
	void SetVisible(bool on);
	// Gets the order viewport is displayed on screen
	int GetZOrder() const {
		return _zorder;
	}
	// Sets the viewport's z-order on screen
	void SetZOrder(int zorder);

	// Calculates room-to-viewport coordinate conversion.
	void AdjustTransformation();
	// Returns linked camera
	PCamera GetCamera() const;
	// Links new camera to this viewport, overriding existing link;
	// pass nullptr to leave viewport without a camera link
	void LinkCamera(PCamera cam);

	// TODO: provide a Transform object here that does these conversions instead
	// Converts room coordinates to the game screen coordinates through this viewport;
	// if clipping is on, the function will fail for room coordinates outside of camera
	VpPoint RoomToScreen(int roomx, int roomy, bool clip = false) const;
	// Converts game screen coordinates to the room coordinates through this viewport;
	// if clipping is on, the function will fail for screen coordinates outside of viewport;
	// convert_cam_to_data parameter converts camera "game" coordinates to "data" units (legacy mode)
	VpPoint ScreenToRoom(int scrx, int scry, bool clip = false, bool convert_cam_to_data = false) const;

	// Following functions tell if this viewport has changed recently
	inline bool HasChangedPosition() const {
		return _hasChangedPosition;
	}
	inline bool HasChangedSize() const {
		return _hasChangedSize;
	}
	inline bool HasChangedVisible() const {
		return _hasChangedVisible;
	}
	inline void SetChangedVisible() {
		_hasChangedVisible = true;
	}
	// Clears the changed flags
	inline void ClearChangedFlags() {
		_hasChangedPosition = false;
		_hasChangedSize = false;
		_hasChangedVisible = false;
	}

private:
	// Parameterized implementation of screen-to-room coordinate conversion
	VpPoint ScreenToRoomImpl(int scrx, int scry, bool clip, bool convert_cam_to_data);

	int _id = -1;
	// Position of the viewport on screen
	Rect _position;
	// TODO: Camera reference (when supporting multiple cameras)
	// Coordinate tranform between camera and viewport
	// TODO: need to add rotate conversion to let script API support that;
	// (maybe use full 3D matrix for that)
	AGS::Shared::PlaneScaling _transform;
	// Linked camera reference
	CameraRef _camera;
	bool _visible = true;
	int _zorder = 0;
	// Flags that tell whether this viewport's position on screen has changed recently
	bool _hasChangedPosition = false;
	bool _hasChangedOffscreen = false;
	bool _hasChangedSize = false;
	bool _hasChangedVisible = false;
};

} // namespace AGS3

#endif // AGS_ENGINE_AC_VIEWPORT_H
