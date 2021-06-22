/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/grim/hotspot.h"
#include "common/array.h"
#include "common/foreach.h"
#include "common/system.h"
#include "engines/grim/actor.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/cursor.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/grim.h"
#include "engines/grim/inputdialog.h"
#include "engines/grim/lua.h"
#include "engines/grim/lua/lua.h"
#include "engines/grim/primitives.h"
#include "engines/grim/resource.h"
#include "engines/grim/set.h"
#include "graphics/pixelbuffer.h"

namespace Grim {

void Polygon::draw(const Color &col) {
	PrimitiveObject line;
	for (size_t i = 0, total = _pnts.size(); i < total; ++i) {
		line.createLine(_pnts[i], _pnts[(i + 1) % total], col);
		line.draw();
	}
}

bool Polygon::contains(const Common::Point &pos) {
	bool result = false;
	for (int i = 0, j = _pnts.size() - 1; i < (int)_pnts.size(); j = i++) {
		if ((_pnts[i].y > pos.y) != (_pnts[j].y > pos.y) &&
			(pos.x < (_pnts[j].x - _pnts[i].x) * (pos.y - _pnts[i].y) / (_pnts[j].y - _pnts[i].y) + _pnts[i].x)) {
			result = !result;
		}
	}
	return result;
}

Common::Point Polygon::center() {
	Common::Point p(0, 0);
	size_t total = _pnts.size();
	for (size_t i = 0; i < total; ++i)
		p += _pnts[i];
	return Common::Point(p.x / total, p.y / total);
}

void Polygon::move(const Common::Point &c) {
	Common::Point p(0, 0);
	size_t total = _pnts.size();
	for (size_t i = 0; i < total; ++i)
		p += _pnts[i];
	Common::Point offset(c.x - p.x / total, c.y - p.y / total);
	for (size_t i = 0; i < total; ++i)
		_pnts[i] += offset;
}

static Actor *getManny() {
	foreach (Actor *a, g_grim->getActiveActors()) {
		if (a->getName() == "Manny")
			return a;
	}
	return nullptr;
}

HotspotMan::HotspotMan() : _initialized(false), _ctrlMode(0), _cutScene(0),
						   _flashHS(false) {
}

HotspotMan::~HotspotMan() {
}

static Common::String readString(Common::SeekableReadStream *data) {
	int len = data->readSint32LE();
	char *buf = new char[len + 1];
	data->read(buf, len);
	buf[len] = 0;
	Common::String s(buf);
	delete[] buf;
	return s;
}

void HotspotMan::initialize() {
	_hotspots.clear();
	Common::SeekableReadStream *data = g_resourceloader->openNewStreamFile("set.bin");
	if (!data)
		return;
	int numSets = data->readSint32LE();
	for (int i = 0; i < numSets; i++) {
		Common::String setID = readString(data);
		_hotspots[setID] = Common::Array<Hotspot>();
		int numHS = data->readSint32LE();
		for (int j = 0; j < numHS; ++j) {
			Hotspot hs;
			hs._id = readString(data);
			hs._desc = readString(data);
			hs._setup = data->readSint32LE();
			hs._type = data->readSint32LE();
			hs._objId = -1;
			int numPath = data->readSint32LE();
			for (int k = 0; k < numPath; ++k) {
				float pos[3];
				data->read(&pos, 3 * sizeof(float));
				hs._path.push_back(Math::Vector3d(pos[0], pos[1], pos[2]));
			}
			int numPoly = data->readSint32LE();
			for (int k = 0; k < numPoly; ++k) {
				int x = data->readSint32LE(), y = data->readSint32LE();
				hs._region._pnts.push_back(Common::Point(x, y));
			}
			_hotspots[setID].push_back(hs);
		}
	}
	loadFlashBitmaps();
	_initialized = true;
}

void HotspotMan::loadFlashBitmaps() {
	for (int i = 0; i < 8; ++i) {
		_flashBitmaps[i] = Bitmap::create(Common::String::format("cursor%d_hl.tga", i));
		_flashBitmaps[i]->_data->load();
		_flashBitmaps[i]->_data->_hasTransparency = true;
	}
}

int HotspotMan::addHotspot(const Common::String &name, const Math::Vector3d &pos, const Common::String &scene) {
	HotObject hs;
	hs._pos = pos;
	hs._active = false;
	_hotobject.push_back(hs);

	int num = _hotobject.size() - 1;
	renameHotspot(num, name);
	return num;
}

void HotspotMan::disableAll() {
	for (size_t i = 0, total = _hotobject.size(); i < total; ++i)
		_hotobject[i]._active = false;
}

void HotspotMan::flashHotspots() {
	if (_cutScene > 0)
		return;
	_flashStart = g_system->getMillis();
	_flashHS = true;
	update();
}

void HotspotMan::drawActive() {
	if (_flashHS) {
		uint32 curTime = g_system->getMillis();
		uint32 delta = curTime - _flashStart;
		if (delta >= 1700)
			_flashHS = false;
		if ((delta % 900) < 450) {
			int setup = g_grim->getCurrSet()->getSetup();
			Common::Array<Hotspot> &hotspots = _hotspots[activeSet()];
			for (size_t i = 0, total = hotspots.size(); i < total; ++i) {
				Hotspot &hs = hotspots[i];
				if (hs._setup == setup && (hs._objId <= 0 || _hotobject[hs._objId]._active)) {
					if ((_ctrlMode == Dialog && hs._type >= 20) ||
						(_ctrlMode == Special && hs._type >= 10 && hs._type < 20) ||
						(_ctrlMode == Linear && hs._type >= 10 && hs._type < 20) ||
						((_ctrlMode == Normal || _ctrlMode == NoWalk) && hs._type < 10)) {
						int cu = hs._type % 10;
						if (hs._type == 3)
							cu = 2;
						if (hs._type == 4)
							cu = 1;
						Common::Point p = hs._region.center();
						_flashBitmaps[cu]->draw(p.x - 15, p.y - 15);
					}
				}
			}
		}
	}

	if (_ctrlMode == Inventory) {
		const int dx = 4;
		const int numRows = (_inventory.size() - 1) / _cols + 1;
		int x1 = _x0 + _w * _cols, y1 = _y0 + _h * numRows;
		g_driver->blackbox(0, 0, 640, 480, 0.4);
		g_driver->blackbox(_x0 - dx, _y0 - dx, x1 + dx, y1 + dx, 0.9);
		PrimitiveObject rect;
		Color col(195, 195, 100);
		rect.createRectangle(Common::Point(_x0 - dx, _y0 - dx), Common::Point(x1 + dx, y1 + dx), col, false);
		rect.draw();

		for (int j = 0, idx = 0; j < _rows; j++) {
			for (int i = 0; i < _cols; ++i, ++idx) {
				if (idx >= (int)_inventory.size())
					break;
				_inventory[idx]._bmp->draw(_x0 + i * _w, _y0 + j * _h);
			}
		}
	}
}

void HotspotMan::updatePerspective() {
	g_grim->getCurrSet()->setupCamera();
	for (size_t i = 0; i < _hotobject.size(); ++i) {
		int x = 0, y = 0;
		g_driver->worldToScreen(_hotobject[i]._pos, x, y);
		_hotobject[i]._rect = Common::Rect(x - 10, y - 10, x + 10, y + 10);
	}
	restoreCursor();
}

Common::String HotspotMan::activeSet() {
	return (_curOption.empty() || _ctrlMode != Options) ? g_grim->getCurrSet()->getName() : _curOption;
}

// get min distance s for x0 + s * x1, y0 + t * y1
static double lineLineDist(
	const Math::Vector3d &x0, const Math::Vector3d &x1,
	const Math::Vector3d &y0, const Math::Vector3d &y1) {
	double a = x1.dotProduct(x1), b = -x1.dotProduct(y1), c = y1.dotProduct(y1);
	double d = -x1.dotProduct(x0 - y0), e = y1.dotProduct(x0 - y0);
	return (c * d - b * e) / (c * a - b * b);
}

void HotspotMan::event(const Common::Point &cursor, const Common::Event &ev, bool doubleClick) {
	bool climbing = LuaBase::instance()->queryVariable("system.currentActor.is_climbing", false) != 0;
	_lastCursor = cursor;

	int button = 0;
	if (ev.type == Common::EVENT_LBUTTONDOWN)
		button = 1;
	else if (ev.type == Common::EVENT_RBUTTONDOWN)
		button = 2;
	else if (ev.type == Common::EVENT_MBUTTONDOWN)
		button = 3;
	if (ev.kbd.hasFlags(Common::KBD_ALT))
		button = 2;
	if (ev.kbd.hasFlags(Common::KBD_CTRL))
		button = 3;
	update();

	if (_ctrlMode == Dialog && button > 0) {
		// dialog mode
		int dialog = inBox(cursor), click = 1;
		if (button == 2)
			click = 2;
		LuaObjects objects;
		objects.add(dialog);
		objects.add(click);
		LuaBase::instance()->callback("dialogSelect", objects);
	} else if (_ctrlMode == Inventory && button > 0) {
		for (int j = 0, idx = 0; j < _rows; ++j) {
			for (int i = 0; i < _cols; ++i, ++idx) {
				if (idx >= (int)_inventory.size())
					break;
				if (cursor.x >= _x0 + i * _w && cursor.x < _x0 + (i + 1) * _w &&
					cursor.y >= _y0 + j * _h && cursor.y < _y0 + (j + 1) * _h) {
					LuaObjects objects;
					objects.add(_inventory[idx]._id.c_str());
					objects.add(button);
					LuaBase::instance()->callback("mouseInventory", objects);
					return;
				}
			}
		}
		LuaObjects objects;
		objects.add("outside");
		objects.add(button);
		LuaBase::instance()->callback("mouseInventory", objects);
		return;
	}

	// ------- click on hot spots ------------
	Common::Array<Hotspot> &hotspots = _hotspots[activeSet()];
	int setup = g_grim->getCurrSet()->getSetup();
	for (size_t i = 0, total = hotspots.size(); i < total; ++i) {
		Hotspot &hs = hotspots[i];
		if (_ctrlMode == Options && hs._region.contains(cursor)) {
			LuaObjects objects;
			objects.add(activeSet().c_str());
			objects.add(hs._id.c_str());
			objects.add(hs._setup);
			objects.add(button);
			LuaBase::instance()->callback("mouseOpt", objects);
			return;
		}

		if (hs._setup == setup && hs._region.contains(cursor) && !climbing) {
			// special mode calls
			if ((_ctrlMode == Dialog && hs._type >= 20) ||
				(_ctrlMode == Special && hs._type >= 10 && hs._type < 20) ||
				(_ctrlMode == Linear && hs._type >= 10 && hs._type < 20)) {
				if (button == 0 && _ctrlMode != Special)
					continue;
				LuaObjects objects;
				objects.add(hs._id.c_str());
				objects.add(button);
				LuaBase::instance()->callback("mouseSpecial", objects);
				return;
			}
			// normal mode
			int gid = hs._objId;
			if ((_ctrlMode != Normal && _ctrlMode != NoWalk) ||
				hs._type >= 10 || hs._type == 3 || button == 0) {
				continue;
			}
			if (gid <= 0 || !_hotobject[gid]._active)
				continue;
			LuaObjects objects;
			objects.add(button);
			objects.add(gid);
			int walk = doubleClick ? 2 : 1;
			objects.add(walk);
			LuaBase::instance()->callback("mouseCommand", objects);
			return;
		}
	}
	if (_ctrlMode == Normal || _ctrlMode == NoWalk) {

		for (size_t i = 0, total = hotspots.size(); i < total; ++i) {
			Hotspot &hs = hotspots[i];
			if (hs._setup == setup && hs._region.contains(cursor) && !climbing) {
				// normal mode walkbox
				int gid = hs._objId;
				if (hs._type == 3 && (gid <= 0 || _hotobject[gid]._active)) {
					float *buf = new float[hs._path.size() * 3];
					LuaObjects objects;
					objects.add(button);
					objects.add(doubleClick ? 1 : 0);
					for (int k = 0, pathSize = hs._path.size(); k < pathSize; ++k) {
						buf[3 * k] = hs._path[k].x();
						buf[3 * k + 1] = hs._path[k].y();
						buf[3 * k + 2] = hs._path[k].z();
					}
					objects.add(buf, hs._path.size() * 3);
					LuaBase::instance()->callback("mouseWalk", objects);
					delete[] buf;
					return;
				}
			}
		}

		if (_ctrlMode == Normal)
			freeClick(cursor, button, doubleClick, climbing);
		else if (_ctrlMode == NoWalk) {
			LuaObjects objects;
			objects.add(button);
			LuaBase::instance()->callback("mouseNowalk", objects);
		}
	}

	if (_ctrlMode == Linear && button > 0) {
		// linear mode
		Actor *manny = getManny();
		if (!manny)
			return;
		Math::Vector3d r0, r1;
		g_grim->getCurrSet()->setupCamera();
		g_driver->raycast(cursor.x, cursor.y, r0, r1);
		Math::Vector3d p0 = manny->getPos(), p1 = _axis;
		r0 -= _axis * _offset;
		r1 -= _axis * _offset;
		float s = lineLineDist(p0, p1, r0, r1);

		LuaObjects objects;
		objects.add(p0.dotProduct(_axis) + s);
		objects.add(s);
		objects.add(button);
		LuaBase::instance()->callback("linearMove", objects);
		return;
	}
}

void HotspotMan::hover(const Common::Point &pos) {
	if (g_grim->getCurrSet() == nullptr)
		return;
	update();
	_lastCursor = pos;
	Cursor *cursor = g_grim->getCursor();
	cursor->setPersistent(-1);
	cursor->setCursor(0);

	if (_cutScene > 0 && _ctrlMode != Dialog && _ctrlMode != Options) {
		cursor->setCursor(7);
		return;
	}

	int setup = g_grim->getCurrSet()->getSetup();
	Common::Array<Hotspot> &hotspots = _hotspots[activeSet()];

	int select = 0;
	for (size_t i = 0, total = hotspots.size(); i < total; ++i) {
		if ((hotspots[i]._setup == setup || _ctrlMode == Options) &&
			hotspots[i]._region.contains(pos)) {
			if (hotspots[i]._objId >= 0 && !_hotobject[hotspots[i]._objId]._active)
				continue;
			int type = hotspots[i]._type;
			if (_ctrlMode == Normal || _ctrlMode == NoWalk) {
				if (type >= 10)
					continue;
				cursor->setCursor((type == 2 || type == 3) ? 2 : 1);
			} else if (_ctrlMode == Dialog) {
				if (type < 20 || type >= 30)
					continue;
				cursor->setCursor(type - 20);
			} else if (_ctrlMode == Special || _ctrlMode == Linear) {
				if (type < 10 || type >= 20)
					continue;
				cursor->setCursor(type - 10);
			} else if (_ctrlMode == Options) {
				if (type < 30 || type >= 40)
					continue;
				cursor->setCursor(type - 30);
				select = hotspots[i]._setup;
			}
		}
	}
	if (_ctrlMode == Dialog) {
		// dialog mode
		int dialog = inBox(pos), click = 0;
		LuaObjects objects;
		objects.add(dialog);
		objects.add(click);
		LuaBase::instance()->callback("dialogSelect", objects);
	} else if (_ctrlMode == Inventory) {
		for (int j = 0, idx = 0; j < _rows; ++j) {
			for (int i = 0; i < _cols; ++i, ++idx) {
				if (idx >= (int)_inventory.size())
					break;
				if (pos.x >= _x0 + i * _w && pos.x < _x0 + (i + 1) * _w &&
					pos.y >= _y0 + j * _h && pos.y < _y0 + (j + 1) * _h) {
					cursor->setCursor(1);
				}
			}
		}
	} else if (_ctrlMode == Options) {
		LuaObjects objects;
		objects.add(activeSet().c_str());
		objects.add(select);
		LuaBase::instance()->callback("mouseHover", objects);
		return;
	}
}

void HotspotMan::freeClick(const Common::Point &cursor, int button, bool doubleClick, bool climbing) {
	Math::Vector3d r0, r1;
	g_grim->getCurrSet()->setupCamera();
	g_driver->raycast(cursor.x, cursor.y, r0, r1);

	// climbing
	if (climbing) {
		Actor *manny = getManny();
		if (!manny)
			return;
		r0 += Math::Vector3d(0, 0, -0.25);
		r1 += Math::Vector3d(0, 0, -0.25); // center on mannys body, not feet
		Math::Vector3d p0 = manny->getPos(), p1 = Math::Vector3d(0, 0, 1);
		float s = lineLineDist(p0, p1, r0, r1);

		LuaObjects objects;
		objects.add(p0.z() + s);
		objects.add(s);
		LuaBase::instance()->callback("climbTowards", objects);
		return;
	}

	// walking
	for (int i = 0; i < 50; ++i) {
		Set *set = g_grim->getCurrSet();
		for (int s = 0, total = set->getSectorCount(); s < total; ++s) {
			Sector *sector = set->getSectorBase(s);
			if ((sector->getType() & Sector::WalkType) != Sector::WalkType)
				continue;
			Math::Vector3d v = sector->raycast(r0, r1);
			if (sector->isPointInSector(v)) {
				LuaObjects objects;
				objects.add(button);
				objects.add(doubleClick ? 1 : 0);
				float p[3] = {v.x(), v.y(), v.z()};
				objects.add(p, 3);
				LuaBase::instance()->callback("mouseWalk", objects);
				return;
			}
		}
		r0.z() -= 0.03;
		r1.z() -= 0.03;
	}
	LuaObjects objects;
	objects.add(button);
	objects.add(doubleClick ? 1 : 0);
	LuaBase::instance()->callback("mouseUseInHand", objects);
}

bool HotspotMan::restoreState(SaveGame *savedState) {
	savedState->beginSection('HOTM');
	_ctrlMode = savedState->readLESint32();
	_rows = savedState->readLESint32();
	_cols = savedState->readLESint32();
	_x0 = savedState->readLESint32();
	_y0 = savedState->readLESint32();
	_w = savedState->readLESint32();
	_h = savedState->readLESint32();
	_axis = savedState->readVector3d();
	_offset = savedState->readFloat();
	_curScene = savedState->readString();

	int num = savedState->readLESint32();
	_hotobject.clear();
	for (int i = 0; i < num; ++i) {
		HotObject hs;
		hs._id = savedState->readString();
		hs._desc = savedState->readString();
		hs._pos = savedState->readVector3d();
		hs._rect.top = savedState->readLEUint16();
		hs._rect.left = savedState->readLEUint16();
		hs._rect.bottom = savedState->readLEUint16();
		hs._rect.right = savedState->readLEUint16();
		hs._active = savedState->readBool();
		_hotobject.push_back(hs);
	}
	int numSets = savedState->readLESint32();
	for (int i = 0; i < numSets; ++i) {
		Common::String setname = savedState->readString();
		int numSpots = savedState->readLESint32();
		_hotspots[setname].clear();
		for (int j = 0; j < numSpots; ++j) {
			Hotspot hs;
			hs._id = savedState->readString();
			hs._desc = savedState->readString();
			int pathlen = savedState->readLESint32();
			for (int k = 0; k < pathlen; ++k)
				hs._path.push_back(savedState->readVector3d());
			int polylen = savedState->readLESint32();
			for (int k = 0; k < polylen; ++k) {
				int x = savedState->readLEUint16();
				int y = savedState->readLEUint16();
				hs._region._pnts.push_back(Common::Point(x, y));
			}
			hs._setup = savedState->readLESint32();
			hs._type = savedState->readLESint32();
			hs._objId = savedState->readLESint32();
			_hotspots[setname].push_back(hs);
		}
	}
	savedState->endSection();
	_icons.clear();
	loadFlashBitmaps();
	return true;
}

void HotspotMan::saveState(SaveGame *savedState) {
	savedState->beginSection('HOTM');
	savedState->writeLESint32(_ctrlMode);
	savedState->writeLESint32(_rows);
	savedState->writeLESint32(_cols);
	savedState->writeLESint32(_x0);
	savedState->writeLESint32(_y0);
	savedState->writeLESint32(_w);
	savedState->writeLESint32(_h);
	savedState->writeVector3d(_axis);
	savedState->writeFloat(_offset);
	savedState->writeString(_curScene);

	savedState->writeLESint32(_hotobject.size());
	for (size_t i = 0, total = _hotobject.size(); i < total; ++i) {
		HotObject &hs = _hotobject[i];
		savedState->writeString(hs._id);
		savedState->writeString(hs._desc);
		savedState->writeVector3d(hs._pos);
		savedState->writeLEUint16(hs._rect.top);
		savedState->writeLEUint16(hs._rect.left);
		savedState->writeLEUint16(hs._rect.bottom);
		savedState->writeLEUint16(hs._rect.right);
		savedState->writeBool(hs._active);
	}
	savedState->writeLESint32(_hotspots.size());
	for (HotDict::iterator it = _hotspots.begin(), end = _hotspots.end(); it != end; ++it) {
		savedState->writeString(it->_key);
		Common::Array<Hotspot> &hotspots = it->_value;
		size_t total = hotspots.size();
		savedState->writeLESint32(total);
		for (size_t i = 0; i < total; ++i) {
			Hotspot &hs = hotspots[i];
			savedState->writeString(hs._id);
			savedState->writeString(hs._desc);
			savedState->writeLESint32(hs._path.size());
			for (size_t j = 0, count = hs._path.size(); j < count; ++j)
				savedState->writeVector3d(hs._path[j]);
			savedState->writeLESint32(hs._region._pnts.size());
			for (size_t j = 0, points = hs._region._pnts.size(); j < points; ++j) {
				savedState->writeLEUint16(hs._region._pnts[j].x);
				savedState->writeLEUint16(hs._region._pnts[j].y);
			}
			savedState->writeLESint32(hs._setup);
			savedState->writeLESint32(hs._type);
			savedState->writeLESint32(hs._objId);
		}
	}
	savedState->endSection();
}

int HotspotMan::inBox(const Common::Point &p) {
	for (int j = 0; j < _rows; ++j) {
		for (int i = 0; i < _cols; ++i) {
			if (p.x >= _x0 + i * _w && p.x < _x0 + (i + 1) * _w &&
				p.y >= _y0 + j * _h && p.y < _y0 + (j + 1) * _h)
				return j * _cols + i + 1;
		}
	}
	return 0;
}

void HotspotMan::update() {
	Common::Array<Hotspot> &hotspots = _hotspots[activeSet()];
	for (size_t i = 0, total = hotspots.size(); i < total; ++i) {
		if (hotspots[i]._type != 4)
			continue;
		foreach (Actor *a, g_grim->getActiveActors()) {
			if (a->getName() == hotspots[i]._desc && hotspots[i]._path.size() == 1) {
				g_grim->getCurrSet()->setupCamera();
				int x = 0, y = 0;
				Math::Vector3d trans = hotspots[i]._path[0];
				Math::Quaternion quat = a->getRotationQuat();
				Math::Quaternion quatInv(-quat.x(), -quat.y(), -quat.z(), quat.w());
				quatInv.toMatrix().transform(&trans, true);
				Math::Vector3d pos = a->getPos() + trans;
				g_driver->worldToScreen(pos, x, y);
				Common::Point p(x, y);
				hotspots[i]._region.move(p);
			}
		}
	}
}

void HotspotMan::updateHotspot(const Common::String &id, const Math::Vector3d &pos, int vis) {
	Common::Array<Hotspot> &hotspots = _hotspots[activeSet()];
	for (size_t i = 0, total = hotspots.size(); i < total; ++i) {
		if (hotspots[i]._id == id) {
			int x = pos.x(), y = pos.y();
			if (vis > 0) {
				g_grim->getCurrSet()->setupCamera();
				g_driver->worldToScreen(pos, x, y);
			}
			Common::Point p(x, y);
			hotspots[i]._region.move(p);
		}
	}
	restoreCursor();
}

void HotspotMan::resetInventory() {
	_inventory.clear();
}

void HotspotMan::cutSceneMode(int mode) {
	if (_cutScene != mode) {
		_cutScene = mode;
		restoreCursor();
	}
}

void HotspotMan::restoreCursor() {
	hover(_lastCursor);
}

void HotspotMan::renameHotspot(int num, const Common::String &name) {
	Common::String id = name, desc = name;
	for (size_t i = 1, total = id.size(); i < total; ++i) {
		if (id[i] == '/') {
			id.erase(i);
			desc.erase(0, i + 1);
			break;
		}
	}

	int n = 0;
	for (size_t i = 0, total = _hotobject.size(); i < total; ++i) {
		if (_hotobject[i]._id.hasPrefix(id))
			++n;
	}
	if (n > 0)
		id += Common::String::format("_%d", n);
	_hotobject[num]._id = id;
	_hotobject[num]._desc = desc;

	// re-link to hotspots
	for (HotDict::iterator it = _hotspots.begin(), end = _hotspots.end(); it != end; ++it) {
		Common::Array<Hotspot> &hotspots = it->_value;
		for (size_t i = 0, total = hotspots.size(); i < total; ++i) {
			if (hotspots[i]._id == id)
				hotspots[i]._objId = num;
		}
	}
}

void HotspotMan::addInventory(const Common::String &id, const Common::String &pic) {
	InventoryItem it;
	it._id = id;
	it._bmp = _icons[pic];
	if (!it._bmp) {
		it._bmp = Bitmap::create(pic + ".tga");
		it._bmp->_data->load();
		_icons[pic] = it._bmp;
	}
	_inventory.push_back(it);
}

} // namespace Grim
