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

#ifndef ULTIMA8_WORLD_CAMERAPROCESS_H
#define ULTIMA8_WORLD_CAMERAPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/classtype.h"
#include "ultima/ultima8/misc/point3.h"

namespace Ultima {
namespace Ultima8 {

/**
* The camera process. This works in 4 ways:
*
* It can be set to stay where it currently is
* It can be set to follow an item.
* It can be set to scroll to an item
* It can be set to stay at a location
*/
class CameraProcess : public Process {
public:
	CameraProcess();
	CameraProcess(uint16 itemnum);                // Follow item/Do nothing
	CameraProcess(const Point3 &p);               // Goto location
	CameraProcess(const Point3 &p, int32 time);   // Scroll to location

	~CameraProcess() override;

	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;

	// You will notice that this isn't the same as how Item::GetLerped works
	Point3 GetLerped(int32 factor, bool noupdate = false);

	//! Find the roof above the camera.
	//! \param factor Interpolation factor for this frame
	//! \return 0 if no roof found, objid of roof if found
	uint16 findRoof(int32 factor);

	/**
	 * Move the existing camera process to a new location.  If the current process is focused on
	 * an item, remove that focus.
	 *
	 * This is not the same as setting a new process, because execution order will not change,
	 * so other pending events will all happen before the fast area is updated
	 */
	void moveToLocation(int32 x, int32 y, int32 z);
	void moveToLocation(const Point3 &p);

	INTRINSIC(I_setCenterOn);
	INTRINSIC(I_moveTo);
	INTRINSIC(I_scrollTo);
	INTRINSIC(I_startQuake);
	INTRINSIC(I_stopQuake);
	INTRINSIC(I_getCameraX);
	INTRINSIC(I_getCameraY);
	INTRINSIC(I_getCameraZ);

	static Point3           GetCameraLocation();
	static CameraProcess   *GetCameraProcess() {
		return _camera;
	}

	/**
	 * Set the current camera process. Adds process and returns PID.
	 * The new process will go on the front of the process queue, so the fast area
	 * will be updated before any other pending actions occur.
	 */
	static uint16           SetCameraProcess(CameraProcess *);
	static void             ResetCameraProcess();

	static void             SetEarthquake(int32 e) {
		_earthquake = e;
		if (!e)  _eqX = _eqY = 0;
	}

	/** Notify the Camera that the target item has moved */
	void itemMoved();

	void terminate() override;   // Terminate NOW!

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	uint16 getTrackedItem() const {
		return _itemNum;
	}

private:
	Point3 _s;
	Point3 _e;
	int32 _time;
	int32 _elapsed;
	uint16 _itemNum;

	int32 _lastFrameNum;

	static CameraProcess *_camera;
	static int32 _earthquake;
	static int32 _eqX, _eqY;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
