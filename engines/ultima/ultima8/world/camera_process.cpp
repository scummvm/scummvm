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

#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/coord_utils.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(CameraProcess)

//
// Statics
//
CameraProcess *CameraProcess::_camera = nullptr;
int32 CameraProcess::_earthquake = 0;
int32 CameraProcess::_eqX = 0;
int32 CameraProcess::_eqY = 0;

CameraProcess::CameraProcess() : Process(), _s(),
	_e(), _time(0), _elapsed(0),
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

void CameraProcess::moveToLocation(int32 x, int32 y, int32 z) {
	moveToLocation(Point3(x, y, z));
}

void CameraProcess::moveToLocation(const Point3 &p) {
	if (_itemNum) {
		Item *item = getItem(_itemNum);
		if (item)
			item->clearExtFlag(Item::EXT_CAMERA);
		_itemNum = 0;
	}

	_s.x = _s.y = _s.z = _time = _elapsed = _lastFrameNum = 0;
	_eqX = _eqY = _earthquake = 0;
	_e = p;
	_s = GetCameraLocation();
}

Point3 CameraProcess::GetCameraLocation() {
	if (_camera) {
		return _camera->GetLerped(256, true);
	}

	World *world = World::get_instance();
	CurrentMap *map = world->getCurrentMap();
	int map_num = map->getNum();
	Actor *av = getControlledActor();
	Point3 pt;

	if (!av || av->getMapNum() != map_num) {
		pt.x = 8192;
		pt.y = 8192;
		pt.z = 64;
	} else {
		pt = av->getLocation();
	}

	if (_earthquake) {
		pt.x += 2 * _eqX + 4 * _eqY;
		pt.y += -2 * _eqX + 4 * _eqY;
	}
	return pt;
}

//
// Constructors
//

// Track item, do nothing
CameraProcess::CameraProcess(uint16 _itemnum) :
	_e(), _time(0), _elapsed(0), _itemNum(_itemnum), _lastFrameNum(0) {
	_s = GetCameraLocation();

	if (_itemNum) {
		Item *item = getItem(_itemNum);
		if (item) {
			item->setExtFlag(Item::EXT_CAMERA);
			_e = item->getLocation();
			_e.z += 20; //!!constant
		}
	} else {
		// No item
		_itemNum = 0;
		_e = _s;
	}
}

// Stay over point
CameraProcess::CameraProcess(const Point3 &p) :
	_e(p), _time(0), _elapsed(0), _itemNum(0), _lastFrameNum(0) {
	_s = GetCameraLocation();
}

// Scroll
CameraProcess::CameraProcess(const Point3 &p, int32 time) :
	_e(p), _time(time), _elapsed(0), _itemNum(0), _lastFrameNum(0) {
	_s = GetCameraLocation();
	debug(10, "Scrolling from (%d, %d,%d) to (%d, %d, %d) in %d frames",
		_s.x, _s.y, _s.z, _e.x, _e.y, _e.z,  _time);
}

void CameraProcess::terminate() {
	if (_itemNum) {
		Item *item = getItem(_itemNum);
		if (item)
			item->clearExtFlag(Item::EXT_CAMERA);
		_itemNum = 0;
	}

	Process::terminate();
}

void CameraProcess::run() {
	if (_earthquake) {
		Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();

		_eqX = rs.getRandomNumberRngSigned(-_earthquake, _earthquake);
		_eqY = rs.getRandomNumberRngSigned(-_earthquake, _earthquake);
	} else {
		_eqX = 0;
		_eqY = 0;
	}

	if (_time && _elapsed > _time) {
		_result = 0; // do we need this
		CameraProcess::SetCameraProcess(nullptr); // This will terminate us
		return;
	}

	_elapsed++;
}

void CameraProcess::itemMoved() {
	if (!_itemNum)
		return;

	Item *item = getItem(_itemNum);

	// We only update for now if lerping has been disabled
	if (!item || !item->hasExtFlags(Item::EXT_LERP_NOPREV))
		return;

	Point3 pt = item->getLocation();

	int32 maxdist = MAX(MAX(abs(_e.x - pt.z), abs(_e.y - pt.y)), abs(_e.z - pt.z));

	if (GAME_IS_U8 || (GAME_IS_CRUSADER && maxdist > 0x40)) {
		_s.x = _e.x = pt.x;
		_s.y = _e.y = pt.y;
		_e.z = pt.z;
		_s.z = _e.z += 20;
		World::get_instance()->getCurrentMap()->updateFastArea(_s, _e);
	}
}

Point3 CameraProcess::GetLerped(int32 factor, bool noupdate) {
	Point3 pt;
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
				_s = _e;

				if (_itemNum) {
					Item *item = getItem(_itemNum);
					// Got it
					if (item) {
						item->setExtFlag(Item::EXT_CAMERA);
						_s = _e;
						_e = item->getLocation();
						_e.z += 20; //!!constant
					}
				}
				// Update the fast area
				World::get_instance()->getCurrentMap()->updateFastArea(_s, _e);
			}
		}

		if (factor == 256) {
			pt = _e;
		} else if (factor == 0) {
			pt = _s;
		} else {
			// This way while possibly slower is more accurate
			pt.x = ((_s.x * (256 - factor) + _e.x * factor) >> 8);
			pt.y = ((_s.y * (256 - factor) + _e.y * factor) >> 8);
			pt.z = ((_s.z * (256 - factor) + _e.z * factor) >> 8);
		}
	} else {
		// Do a quadratic interpolation here of velocity (maybe), but not yet
		int32 sfactor = _elapsed;
		int32 efactor = _elapsed + 1;

		if (sfactor > _time) sfactor = _time;
		if (efactor > _time) efactor = _time;

		Point3 ls;
		ls.x = ((_s.x * (_time - sfactor) + _e.x * sfactor) / _time);
		ls.y = ((_s.y * (_time - sfactor) + _e.y * sfactor) / _time);
		ls.z = ((_s.z * (_time - sfactor) + _e.z * sfactor) / _time);

		Point3 le;
		le.x = ((_s.x * (_time - efactor) + _e.x * efactor) / _time);
		le.y = ((_s.y * (_time - efactor) + _e.y * efactor) / _time);
		le.z = ((_s.z * (_time - efactor) + _e.z * efactor) / _time);

		// Update the fast area
		if (!noupdate)
			World::get_instance()->getCurrentMap()->updateFastArea(ls, le);

		// This way while possibly slower is more accurate
		pt.x = ((ls.x * (256 - factor) + le.x * factor) >> 8);
		pt.y = ((ls.y * (256 - factor) + le.y * factor) >> 8);
		pt.z = ((ls.z * (256 - factor) + le.z * factor) >> 8);
	}

	if (_earthquake) {
		pt.x += 2 * _eqX + 4 * _eqY;
		pt.y += -2 * _eqX + 4 * _eqY;
	}
	return pt;
}

uint16 CameraProcess::findRoof(int32 factor) {
	int32 earthquake_old = _earthquake;
	_earthquake = 0;
	Point3 pt = GetLerped(factor);
	_earthquake = earthquake_old;

	// Camera box based on 2x2x2 footpad to avoid floor detected as roof
	Box target(pt.x, pt.y, pt.z, 64, 64, 16);

	PositionInfo info = World::get_instance()->getCurrentMap()->getPositionInfo(target, target, 0, kMainActorId);
	return info.roof ? info.roof->getObjId() : 0;
}

void CameraProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_s.x));
	ws->writeUint32LE(static_cast<uint32>(_s.y));
	ws->writeUint32LE(static_cast<uint32>(_s.z));
	ws->writeUint32LE(static_cast<uint32>(_e.x));
	ws->writeUint32LE(static_cast<uint32>(_e.y));
	ws->writeUint32LE(static_cast<uint32>(_e.z));
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

	_s.x = static_cast<int32>(rs->readUint32LE());
	_s.y = static_cast<int32>(rs->readUint32LE());
	_s.z = static_cast<int32>(rs->readUint32LE());
	_e.x = static_cast<int32>(rs->readUint32LE());
	_e.y = static_cast<int32>(rs->readUint32LE());
	_e.z = static_cast<int32>(rs->readUint32LE());
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
		ARG_NULL16(); // sint16? what is this?
	}

	World_FromUsecodeXY(x, y);
	Point3 pt(x, y, z);
	CameraProcess::SetCameraProcess(new CameraProcess(pt));
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
	ARG_NULL16(); // some uint16?

	World_FromUsecodeXY(x, y);
	Point3 pt(x, y, z);
	return CameraProcess::SetCameraProcess(new CameraProcess(pt, 25));
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
	assert(GAME_IS_CRUSADER);
	Point3 pt = GetCameraLocation();
	return World_ToUsecodeCoord(pt.x);
}

uint32 CameraProcess::I_getCameraY(const uint8 *args, unsigned int argsize) {
	assert(GAME_IS_CRUSADER);
	Point3 pt = GetCameraLocation();
	return World_ToUsecodeCoord(pt.y);
}

uint32 CameraProcess::I_getCameraZ(const uint8 *args, unsigned int argsize) {
	Point3 pt = GetCameraLocation();
	return pt.z;
}

} // End of namespace Ultima8
} // End of namespace Ultima
