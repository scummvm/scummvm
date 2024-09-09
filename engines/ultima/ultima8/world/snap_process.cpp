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

#include "common/config-manager.h"

#include "ultima/ultima8/world/snap_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {


SnapProcess *SnapProcess::_instance = nullptr;

DEFINE_RUNTIME_CLASSTYPE_CODE(SnapProcess)

SnapProcess::SnapProcess() : Process(), _currentSnapEgg(0) {
	_instance = this;
	_type = 1; // persistent
}

SnapProcess::~SnapProcess() {
	if (_instance == this)
		_instance = nullptr;
}

void SnapProcess::run() {
	bool snap_to_player = ConfMan.getBool("camera_on_player");
	bool in_stasis = Ultima8Engine::get_instance()->isAvatarInStasis();

	// For cut scenes and remote cameras (when in stasis),
	// we should snap to the right egg - not to the player.
	// Snapping to player resumes once stasis is done.
	if (snap_to_player && !in_stasis) {
		const Actor *controlled = getControlledActor();
		if (controlled) {
			Point3 pt = controlled->getCentre();
			if (pt.x > 0 || pt.y > 0) {
				_currentSnapEgg = 0;
				CameraProcess *camera = CameraProcess::GetCameraProcess();
				if (camera->getItemNum() != controlled->getObjId())
					CameraProcess::SetCameraProcess(new CameraProcess(pt));
			}
		}
	} else {
		if (!_currentSnapEgg || !isNpcInRangeOfCurrentEgg()) {
			updateCurrentEgg();
		}
	}
}

void SnapProcess::addEgg(Item *item) {
	assert(item);
	ObjId id = item->getObjId();
	for (Std::list<ObjId>::const_iterator iter = _snapEggs.begin();
		 iter != _snapEggs.end(); iter++) {
		if (*iter == id)
			return;
	}
	_snapEggs.push_back(id);
}

void SnapProcess::updateCurrentEgg() {
	// Check that we have something to snap to..
	if (!_currentSnapEgg && !_snapEggs.size())
		return;

	const Actor *a = getControlledActor();

	if (!a)
		return;

	int32 axd, ayd, azd;
	Point3 pta = a->getLocation();
	a->getFootpadWorld(axd, ayd, azd);
	Rect arect(pta.x, pta.y, pta.x + axd, pta.y + ayd);

	for (Std::list<ObjId>::const_iterator iter = _snapEggs.begin();
		 iter != _snapEggs.end(); iter++) {
		const Item *egg = getItem(*iter);
		if (!egg)
			continue;
		Rect r;
		Point3 pte = egg->getLocation();
		getSnapEggRange(egg, r);
		if (r.intersects(arect) && (pta.z <= pte.z + 0x30 && pta.z >= pte.z - 0x30)) {
			_currentSnapEgg = *iter;
			_currentSnapEggRange = r;
			CameraProcess::SetCameraProcess(new CameraProcess(_currentSnapEgg));
		}
	}
}

void SnapProcess::removeEgg(Item *item) {
	assert(item);
	ObjId id = item->getObjId();
	for (Std::list<ObjId>::iterator iter = _snapEggs.begin();
		 iter != _snapEggs.end(); iter++) {
		if (*iter == id) {
			iter = _snapEggs.erase(iter);
		}
	}
	if (id == _currentSnapEgg) {
		_currentSnapEgg = 0;
		_currentSnapEggRange = Rect();
	}
}

void SnapProcess::clearEggs() {
	_snapEggs.clear();
	_currentSnapEgg = 0;
	_currentSnapEggRange = Rect();
}


bool SnapProcess::isNpcInRangeOfCurrentEgg() const {
	if (!_currentSnapEgg)
		return false;

	const Actor *a = getControlledActor();
	const Item *currentegg = getItem(_currentSnapEgg);

	if (!a || !currentegg)
		return false;

	int32 axd, ayd, azd;
	Point3 pta = a->getLocation();
	a->getFootpadWorld(axd, ayd, azd);
	Point3 pte = currentegg->getLocation();

	Rect arect(pta.x, pta.y, pta.x + axd, pta.y + ayd);

	if (!_currentSnapEggRange.intersects(arect))
		return false;
	if (pta.z > pte.z + 0x30 || pta.z < pte.z - 0x30)
		return false;

	return true;
}

void SnapProcess::getSnapEggRange(const Item *item, Rect &rect) const {
	assert(item);
	uint16 qhi = ((item->getQuality() >> 8) & 0xff);
	// Interpreting the values as *signed* char here is deliberate.
	int32 xoff = static_cast<int8>(item->getMapNum()) * 0x0020;
	int32 yoff = static_cast<int8>(item->getNpcNum()) * 0x0020;
	int32 xrange = (qhi >> 4) * 0x20;
	int32 yrange = (qhi & 0xf) * 0x20;

	Point3 pt = item->getLocation();

	rect.left = pt.x - xrange + xoff;
	rect.top = pt.y - yrange + yoff;
	rect.setWidth(xrange * 2);
	rect.setHeight(yrange * 2);
}

void SnapProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint16LE(_currentSnapEgg);
	ws->writeUint16LE(_snapEggs.size());
	for (Std::list<ObjId>::const_iterator iter = _snapEggs.begin();
		 iter != _snapEggs.end(); iter++) {
		ws->writeUint16LE(*iter);
	}
}

bool SnapProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_currentSnapEgg = rs->readUint16LE();
	uint16 numeggs = rs->readUint16LE();
	for (int i = 0; i < numeggs; i++) {
		_snapEggs.push_back(rs->readUint16LE());
	}

	_type = 1; // should be persistent but older savegames may not know that.

	return true;
}

SnapProcess *SnapProcess::get_instance() {
	return _instance;
}


} // End of namespace Ultima8
} // End of namespace Ultima
