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
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(CameraProcess)

//
// Statics
//
CameraProcess *CameraProcess::_camera = nullptr;
int32 CameraProcess::_earthquake = 0;
int32 CameraProcess::_eqX = 0;
int32 CameraProcess::_eqY = 0;

CameraProcess::CameraProcess() : Process(), _sx(0), _sy(0), _sz(0),
	_ex(0), _ey(0), _ez(0), _time(0), _elapsed(0),
	_itemNum(0), _lastFrameNum(0) {
}

CameraProcess::~CameraProcess() {
	if (_camera == this)
		_camera = nullptr;
}

uint16 CameraProcess::SetCameraProcess(CameraProcess *cam) {
	if (!cam) cam = new CameraProcess(0);
	if (_camera) _camera->terminate();
	_camera = cam;
	return Kernel::get_instance()->addProcess(_camera);
}

void CameraProcess::ResetCameraProcess() {
	if (_camera) _camera->terminate();
	_camera = nullptr;
}

void CameraProcess::GetCameraLocation(int32 &x, int32 &y, int32 &z) {
	if (!_camera) {
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

		if (_earthquake) {
			x += 2 * _eqX + 4 * _eqY;
			y += -2 * _eqX + 4 * _eqY;
		}
	} else {
		_camera->GetLerped(x, y, z, 256, true);
	}
}

//
// Constructors
//

// Track item, do nothing
CameraProcess::CameraProcess(uint16 _itemnum) :
	_time(0), _elapsed(0), _itemNum(_itemnum), _lastFrameNum(0) {
	GetCameraLocation(_sx, _sy, _sz);

	if (_itemNum) {
		Item *item = getItem(_itemNum);
		if (item) {
			item->setExtFlag(Item::EXT_CAMERA);
			item->getLocation(_ex, _ey, _ez);
			_ez += 20; //!!constant
		} else {
			_ex = 0;
			_ey = 0;
			_ez = 0;
		}
		return;
	}

	// No item
	_itemNum = 0;
	_ex = _sx;
	_ey = _sy;
	_ez = _sz;
}

// Stay over point
CameraProcess::CameraProcess(int32 x, int32 y, int32 z) :
	_ex(x), _ey(y), _ez(z), _time(0), _elapsed(0), _itemNum(0), _lastFrameNum(0) {
	GetCameraLocation(_sx, _sy, _sz);
}

// Scroll
CameraProcess::CameraProcess(int32 x, int32 y, int32 z, int32 time) :
	_ex(x), _ey(y), _ez(z), _time(time), _elapsed(0), _itemNum(0), _lastFrameNum(0) {
	GetCameraLocation(_sx, _sy, _sz);
	//pout << "Scrolling from (" << sx << "," << sy << "," << sz << ") to (" <<
	//  ex << "," << ey << "," << ez << ") in " << _time << " frames" << Std::endl;
}

void CameraProcess::terminate() {
	if (_itemNum) {
		Item *item = getItem(_itemNum);
		if (item) item->clearExtFlag(Item::EXT_CAMERA);
	}

	Process::terminate();
}

void CameraProcess::run() {
	if (_earthquake) {
		_eqX = (getRandom() % (_earthquake * 2 + 1)) - _earthquake;
		_eqY = (getRandom() % (_earthquake * 2 + 1)) - _earthquake;
	} else {
		_eqX = 0;
		_eqY = 0;
	}

	if (_time && _elapsed > _time) {
		_result = 0; // do we need this
		CameraProcess::SetCameraProcess(0); // This will terminate us
		return;
	}

	_elapsed++;
}

void CameraProcess::ItemMoved() {
	if (!_itemNum)
		return;

	Item *item = getItem(_itemNum);

	// We only update for now if lerping has been disabled
	if (!item || !item->hasExtFlags(Item::EXT_LERP_NOPREV))
		return;

	int32 ix, iy, iz;
	item->getLocation(ix, iy, iz);

	int32 maxdist = MAX(MAX(abs(_ex - iz), abs(_ey - iy)), abs(_ez - iz));

	if (GAME_IS_U8 || (GAME_IS_CRUSADER && maxdist > 0x40)) {
		_sx = _ex = ix;
		_sy = _ey = iy;
		_ez = iz;
		_sz = _ez += 20;
		World::get_instance()->getCurrentMap()->updateFastArea(_sx, _sy, _sz, _ex, _ey, _ez);
	}
}

void CameraProcess::GetLerped(int32 &x, int32 &y, int32 &z, int32 factor, bool noupdate) {
	if (_time == 0) {
		if (!noupdate) {

			bool inBetween = true;

			if (_lastFrameNum != _elapsed) {
				// No lerping if we missed a frame
				if ((_elapsed - _lastFrameNum) > 1) factor = 256;
				_lastFrameNum = _elapsed;
				inBetween = false;
			}

			if (!inBetween) {
				_sx = _ex;
				_sy = _ey;
				_sz = _ez;

				if (_itemNum) {
					Item *item = getItem(_itemNum);
					// Got it
					if (item) {
						_sx = _ex;
						_sy = _ey;
						_sz = _ez;
						item->getLocation(_ex, _ey, _ez);
						_ez += 20; //!!constant
					}
				}
				// Update the fast area
				World::get_instance()->getCurrentMap()->updateFastArea(_sx, _sy, _sz, _ex, _ey, _ez);
			}
		}

		if (factor == 256) {
			x = _ex;
			y = _ey;
			z = _ez;
		} else if (factor == 0) {
			x = _sx;
			y = _sy;
			z = _sz;
		} else {
			// This way while possibly slower is more accurate
			x = ((_sx * (256 - factor) + _ex * factor) >> 8);
			y = ((_sy * (256 - factor) + _ey * factor) >> 8);
			z = ((_sz * (256 - factor) + _ez * factor) >> 8);
		}
	} else {
		// Do a quadratic interpolation here of velocity (maybe), but not yet
		int32 sfactor = _elapsed;
		int32 efactor = _elapsed + 1;

		if (sfactor > _time) sfactor = _time;
		if (efactor > _time) efactor = _time;

		int32 lsx = ((_sx * (_time - sfactor) + _ex * sfactor) / _time);
		int32 lsy = ((_sy * (_time - sfactor) + _ey * sfactor) / _time);
		int32 lsz = ((_sz * (_time - sfactor) + _ez * sfactor) / _time);

		int32 lex = ((_sx * (_time - efactor) + _ex * efactor) / _time);
		int32 ley = ((_sy * (_time - efactor) + _ey * efactor) / _time);
		int32 lez = ((_sz * (_time - efactor) + _ez * efactor) / _time);

		// Update the fast area
		if (!noupdate) World::get_instance()->getCurrentMap()->updateFastArea(lsx, lsy, lsz, lex, ley, lez);

		// This way while possibly slower is more accurate
		x = ((lsx * (256 - factor) + lex * factor) >> 8);
		y = ((lsy * (256 - factor) + ley * factor) >> 8);
		z = ((lsz * (256 - factor) + lez * factor) >> 8);
	}

	if (_earthquake) {
		x += 2 * _eqX + 4 * _eqY;
		y += -2 * _eqX + 4 * _eqY;
	}
}

uint16 CameraProcess::FindRoof(int32 factor) {
	int32 x, y, z;
	int32 earthquake_old = _earthquake;
	_earthquake = 0;
	GetLerped(x, y, z, factor);
	_earthquake = earthquake_old;
	Item *avatar = getItem(1);
	assert(avatar);
	int32 dx, dy, dz;
	avatar->getFootpadWorld(dx, dy, dz);
	uint16 roofid;
	World::get_instance()->getCurrentMap()->isValidPosition(x, y, z - 10, dx / 2, dy / 2, dz / 2, 0, 1, 0, &roofid);
	return roofid;
}

void CameraProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_sx));
	ws->writeUint32LE(static_cast<uint32>(_sy));
	ws->writeUint32LE(static_cast<uint32>(_sz));
	ws->writeUint32LE(static_cast<uint32>(_ex));
	ws->writeUint32LE(static_cast<uint32>(_ey));
	ws->writeUint32LE(static_cast<uint32>(_ez));
	ws->writeUint32LE(static_cast<uint32>(_time));
	ws->writeUint32LE(static_cast<uint32>(_elapsed));
	ws->writeUint16LE(_itemNum);
	ws->writeUint32LE(_lastFrameNum);
	ws->writeUint32LE(static_cast<uint32>(_earthquake));
	ws->writeUint32LE(static_cast<uint32>(_eqX));
	ws->writeUint32LE(static_cast<uint32>(_eqY));
}

bool CameraProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_sx = static_cast<int32>(rs->readUint32LE());
	_sy = static_cast<int32>(rs->readUint32LE());
	_sz = static_cast<int32>(rs->readUint32LE());
	_ex = static_cast<int32>(rs->readUint32LE());
	_ey = static_cast<int32>(rs->readUint32LE());
	_ez = static_cast<int32>(rs->readUint32LE());
	_time = static_cast<int32>(rs->readUint32LE());
	_elapsed = static_cast<int32>(rs->readUint32LE());
	_itemNum = rs->readUint16LE();
	_lastFrameNum = rs->readUint32LE();
	_earthquake = static_cast<int32>(rs->readUint32LE()); //static
	_eqX = static_cast<int32>(rs->readUint32LE()); //static
	_eqY = static_cast<int32>(rs->readUint32LE()); //static

	_camera = this; //static

	return true;
}

//	"Camera::move_to(uword, uword, ubyte, word)",
uint32 CameraProcess::I_moveTo(const uint8 *args, unsigned int argsize) {
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT8(z);
	if (argsize > 6) {
		ARG_SINT16(unk);
	}

	if (GAME_IS_CRUSADER) {
		x *= 2;
		y *= 2;
	}
	CameraProcess::SetCameraProcess(new CameraProcess(x, y, z));
	return 0;
}

//	"Camera::setCenterOn(uword)",
uint32 CameraProcess::I_setCenterOn(const uint8 *args, unsigned int /*argsize*/) {
	ARG_OBJID(itemNum);
	CameraProcess::SetCameraProcess(new CameraProcess(itemNum));
	return 0;
}

//	Camera::scrollTo(uword, uword, ubyte, word)
uint32 CameraProcess::I_scrollTo(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT8(z);
	ARG_SINT16(unk);

	if (GAME_IS_CRUSADER) {
		x *= 2;
		y *= 2;
	}

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

uint32 CameraProcess::I_getCameraX(const uint8 *args, unsigned int argsize) {
	int32 x, y, z;
	assert(GAME_IS_CRUSADER);
	GetCameraLocation(x, y, z);
	return x / 2;
}

uint32 CameraProcess::I_getCameraY(const uint8 *args, unsigned int argsize) {
	int32 x, y, z;
	assert(GAME_IS_CRUSADER);
	GetCameraLocation(x, y, z);
	return y / 2;
}

uint32 CameraProcess::I_getCameraZ(const uint8 *args, unsigned int argsize) {
	int32 x, y, z;
	GetCameraLocation(x, y, z);
	return z;
}

} // End of namespace Ultima8
} // End of namespace Ultima
