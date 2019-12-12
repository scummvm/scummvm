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

#ifndef ULTIMA8_WORLD_CAMERAPROCESS_H
#define ULTIMA8_WORLD_CAMERAPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

//
// The camera process. This works in 3 ways
//
// It can be set to stay where it currently is
// It can be set to follow an item.
// It can be set to scroll to an item
// It can be set to stay at a location
//

class CameraProcess : public Process {
public:
	CameraProcess();
	CameraProcess(uint16 itemnum);                          // Follow item/Do nothing
	CameraProcess(int32 x, int32 y, int32 z);               // Goto location
	CameraProcess(int32 x, int32 y, int32 z, int32 time);   // Scroll to location

	virtual ~CameraProcess();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	virtual void run();

	// You will notice that this isn't the same as how Item::GetLerped works
	void GetLerped(int32 &x, int32 &y, int32 &z, int32 factor, bool noupdate = false);

	//! Find the roof above the camera.
	//! \param factor Interpolation factor for this frame
	//! \return 0 if no roof found, objid of roof if found
	uint16 FindRoof(int32 factor);

	INTRINSIC(I_setCenterOn);
	INTRINSIC(I_move_to);
	INTRINSIC(I_scrollTo);
	INTRINSIC(I_startQuake);
	INTRINSIC(I_stopQuake);

	static void             GetCameraLocation(int32 &x, int32 &y, int32 &z);
	static CameraProcess   *GetCameraProcess() {
		return camera;
	}
	static uint16           SetCameraProcess(CameraProcess *);  // Set the current camera process. Adds process. Return PID
	static void             ResetCameraProcess();

	static void             SetEarthquake(int32 e) {
		earthquake = e;
		if (!e)  eq_x = eq_y = 0;
	}

	void                    ItemMoved();

	virtual void terminate();   // Terminate NOW!

	bool loadData(IDataSource *ids, uint32 version);
private:
	virtual void saveData(ODataSource *ods);

	int32 sx, sy, sz;
	int32 ex, ey, ez;
	int32 _time;
	int32 elapsed;
	uint16 itemnum;

	int32 last_framenum;

	static CameraProcess    *camera;
	static int32 earthquake;
	static int32 eq_x, eq_y;
};

} // End of namespace Ultima8

#endif
