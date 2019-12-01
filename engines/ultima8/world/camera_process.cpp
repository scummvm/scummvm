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

#include "ultima8/misc/pent_include.h"

#include "ultima8/world/camera_process.h"
#include "ultima8/world/world.h"
#include "ultima8/world/current_map.h"
#include "ultima8/world/item.h"
#include "ultima8/world/actors/actor.h"
#include "ultima8/usecode/uc_machine.h"
#include "ultima8/graphics/shape_info.h"
#include "ultima8/kernel/kernel.h"
#include "ultima8/kernel/core_app.h"
#include "ultima8/world/get_object.h"
#inc
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(CameraProcess, Process)

//
// Statics
//
CameraProcess *CameraProcess::camera = 0;
int32 CameraProcess::earthquake = 0;
int32 CameraProcess::eq_x = 0;
int32 CameraProcess::eq_y = 0;

CameraProcess::CameraProcess() : Process() {

}

CameraProcess::~CameraProcess() {
	if (camera == this)
		camera = 0;
}

uint16 CameraProcess::SetCameraProcess(CameraProcess *cam) {
	if (!cam) cam = new CameraProcess(0);
	if (camera) camera->terminate();
	camera = cam;
	return Kernel::get_instance()->addProcess(camera);
}

void CameraProcess::ResetCameraProcess() {
	if (camera) camera->terminate();
	camera = 0;
}

void CameraProcess::GetCameraLocation(int32 &x, int32 &y, int32 &z) {
	if (!camera) {
		World *world = World::get_instance();
		CurrentMap *map = world->getCurrentMap();
		int map_num = map->getNum();
		Actor *av = getActor(1);

		if (!av || av->getMapNum() != map_num) {
			x = 8192;
			y = 8192;
			z = 64;
		} else
			av->getLocation(x, y, z);

		if (earthquake) {
			x += 2 * eq_x + 4 * eq_y;
			y += -2 * eq_x + 4 * eq_y;
		}
	} else {
		camera->GetLerped(x, y, z, 256, true);
	}
}

//
// Constructors
//

// Track item, do nothing
CameraProcess::CameraProcess(uint16 _itemnum) :
	time(0), elapsed(0), itemnum(_itemnum), last_framenum(0) {
	GetCameraLocation(sx, sy, sz);

	if (itemnum) {
		Item *item = getItem(itemnum);

		// Got it
		if (item) {
			item->setExtFlag(Item::EXT_CAMERA);
			item->getLocation(ex, ey, ez);
			ez += 20; //!!constant
		}
		return;
	}

	// No item
	itemnum = 0;
	ex = sx;
	ey = sy;
	ez = sz;
}

// Stay over point
CameraProcess::CameraProcess(int32 _x, int32 _y, int32 _z) :
	ex(_x), ey(_y), ez(_z), time(0), elapsed(0), itemnum(0), last_framenum(0) {
	GetCameraLocation(sx, sy, sz);
}

// Scroll
CameraProcess::CameraProcess(int32 _x, int32 _y, int32 _z, int32 _time) :
	ex(_x), ey(_y), ez(_z), time(_time), elapsed(0), itemnum(0), last_framenum(0) {
	GetCameraLocation(sx, sy, sz);
	//pout << "Scrolling from (" << sx << "," << sy << "," << sz << ") to (" <<
	//  ex << "," << ey << "," << ez << ") in " << time << " frames" << std::endl;
}

void CameraProcess::terminate() {
	if (itemnum) {
		Item *item = getItem(itemnum);
		if (item) item->clearExtFlag(Item::EXT_CAMERA);
	}

	Process::terminate();
}

void CameraProcess::run() {
	if (earthquake) {
		eq_x = (std::rand() % (earthquake * 2 + 1)) - earthquake;
		eq_y = (std::rand() % (earthquake * 2 + 1)) - earthquake;
	} else {
		eq_x = 0;
		eq_y = 0;
	}

	if (time && elapsed > time) {
		result = 0; // do we need this
		CameraProcess::SetCameraProcess(0); // This will terminate us
		return;
	}

	elapsed++;
}

void CameraProcess::ItemMoved() {
	if (itemnum) {
		Item *item = getItem(itemnum);

		// We only update for now if lerping has been disabled
		if (item && (item->getExtFlags() & Item::EXT_LERP_NOPREV)) {
			item->getLocation(ex, ey, ez);
			sx = ex;
			sy = ey;
			sz = ez += 20;

			World::get_instance()->getCurrentMap()->updateFastArea(sx, sy, sz, ex, ey, ez);
		}
	}
}

void CameraProcess::GetLerped(int32 &x, int32 &y, int32 &z, int32 factor, bool noupdate) {
	if (time == 0) {
		if (!noupdate) {

			bool inBetween = true;

			if (last_framenum != elapsed) {
				// No lerping if we missed a frame
				if ((elapsed - last_framenum) > 1) factor = 256;
				last_framenum = elapsed;
				inBetween = false;
			}

			if (!inBetween) {
				sx = ex;
				sy = ey;
				sz = ez;

				if (itemnum) {
					Item *item = getItem(itemnum);
					// Got it
					if (item) {
						sx = ex;
						sy = ey;
						sz = ez;
						item->getLocation(ex, ey, ez);
						ez += 20; //!!constant
					}
				}
				// Update the fast area
				World::get_instance()->getCurrentMap()->updateFastArea(sx, sy, sz, ex, ey, ez);
			}
		}

		if (factor == 256) {
			x = ex;
			y = ey;
			z = ez;
		} else if (factor == 0) {
			x = sx;
			y = sy;
			z = sz;
		} else {
			// This way while possibly slower is more accurate
			x = ((sx * (256 - factor) + ex * factor) >> 8);
			y = ((sy * (256 - factor) + ey * factor) >> 8);
			z = ((sz * (256 - factor) + ez * factor) >> 8);
		}
	} else {
		// Do a quadratic interpolation here of velocity (maybe), but not yet
		int32 sfactor = elapsed;
		int32 efactor = elapsed + 1;

		if (sfactor > time) sfactor = time;
		if (efactor > time) efactor = time;

		int32 lsx = ((sx * (time - sfactor) + ex * sfactor) / time);
		int32 lsy = ((sy * (time - sfactor) + ey * sfactor) / time);
		int32 lsz = ((sz * (time - sfactor) + ez * sfactor) / time);

		int32 lex = ((sx * (time - efactor) + ex * efactor) / time);
		int32 ley = ((sy * (time - efactor) + ey * efactor) / time);
		int32 lez = ((sz * (time - efactor) + ez * efactor) / time);

		// Update the fast area
		if (!noupdate) World::get_instance()->getCurrentMap()->updateFastArea(lsx, lsy, lsz, lex, ley, lez);

		// This way while possibly slower is more accurate
		x = ((lsx * (256 - factor) + lex * factor) >> 8);
		y = ((lsy * (256 - factor) + ley * factor) >> 8);
		z = ((lsz * (256 - factor) + lez * factor) >> 8);
	}

	if (earthquake) {
		x += 2 * eq_x + 4 * eq_y;
		y += -2 * eq_x + 4 * eq_y;
	}
}

uint16 CameraProcess::FindRoof(int32 factor) {
	int32 x, y, z;
	int32 earthquake_old = earthquake;
	earthquake = 0;
	GetLerped(x, y, z, factor);
	earthquake = earthquake_old;
	Item *avatar = getItem(1);
	int32 dx, dy, dz;
	avatar->getFootpadWorld(dx, dy, dz);
	uint16 roofid;
	World::get_instance()->getCurrentMap()->isValidPosition(x, y, z - 10, dx / 2, dy / 2, dz / 2, 0, 1, 0, &roofid);
	return roofid;
}

void CameraProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write4(static_cast<uint32>(sx));
	ods->write4(static_cast<uint32>(sy));
	ods->write4(static_cast<uint32>(sz));
	ods->write4(static_cast<uint32>(ex));
	ods->write4(static_cast<uint32>(ey));
	ods->write4(static_cast<uint32>(ez));
	ods->write4(static_cast<uint32>(time));
	ods->write4(static_cast<uint32>(elapsed));
	ods->write2(itemnum);
	ods->write4(last_framenum);
	ods->write4(static_cast<uint32>(earthquake));
	ods->write4(static_cast<uint32>(eq_x));
	ods->write4(static_cast<uint32>(eq_y));
}

bool CameraProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	sx = static_cast<int32>(ids->read4());
	sy = static_cast<int32>(ids->read4());
	sz = static_cast<int32>(ids->read4());
	ex = static_cast<int32>(ids->read4());
	ey = static_cast<int32>(ids->read4());
	ez = static_cast<int32>(ids->read4());
	time = static_cast<int32>(ids->read4());
	elapsed = static_cast<int32>(ids->read4());
	itemnum = ids->read2();
	last_framenum = ids->read4();
	earthquake = static_cast<int32>(ids->read4()); //static
	eq_x = static_cast<int32>(ids->read4()); //static
	eq_y = static_cast<int32>(ids->read4()); //static

	camera = this; //static

	return true;
}

//	"Camera::move_to(uword, uword, ubyte, word)",
uint32 CameraProcess::I_move_to(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT8(z);
	ARG_SINT16(unk);
	CameraProcess::SetCameraProcess(new CameraProcess(x, y, z));
	return 0;
}

//	"Camera::setCenterOn(uword)",
uint32 CameraProcess::I_setCenterOn(const uint8 *args, unsigned int /*argsize*/) {
	ARG_OBJID(itemnum);
	CameraProcess::SetCameraProcess(new CameraProcess(itemnum));
	return 0;
}

//	Camera::scrollTo(uword, uword, ubyte, word)
uint32 CameraProcess::I_scrollTo(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT8(z);
	ARG_SINT16(unk);
	return CameraProcess::SetCameraProcess(new CameraProcess(x, y, z, 25));
}

//	Camera::startQuake(word)
uint32 CameraProcess::I_startQuake(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(strength);
	SetEarthquake(strength);
	return 0;
}

//	Camera::stopQuake()
uint32 CameraProcess::I_stopQuake(const uint8 * /*args*/, unsigned int /*argsize*/) {
	SetEarthquake(0);
	return 0;
}

} // End of namespace Ultima8
