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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#ifndef ANDROID
#include "engines/grim/gfx_opengl.h"
#endif

#include "engines/grim/bitmap.h"
#include "common/foreach.h"
#include "engines/grim/hotspot.h"
#include "engines/grim/primitives.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/grim.h"
#include "engines/grim/actor.h"
#include "engines/grim/set.h"
#include "engines/grim/inputdialog.h"
#include "engines/grim/cursor.h"
#include "engines/grim/lua.h"
#include "engines/grim/resource.h"
#include "engines/grim/lua/lua.h"
#include "graphics/pixelbuffer.h"
#include "common/array.h"
#include "common/system.h"

namespace Grim {

inline int16 max(int16 a, int16 b) { return a>b ? a : b; }
inline int16 min(int16 a, int16 b) { return a<b ? a : b; }

void Polygon::draw(const Color& col) {
	PrimitiveObject line;
	for (size_t i=0; i<_pnts.size(); i++) {
		line.createLine(_pnts[i],_pnts[(i+1)%_pnts.size()],col);
		line.draw();
	}
}

bool Polygon::contains(const Common::Point& pos) {
	bool result = false;
	for (int i = 0, j = _pnts.size() - 1; i < (int)_pnts.size(); j = i++) {
		if ((_pnts[i].y > pos.y) != (_pnts[j].y > pos.y) &&
			(pos.x < (_pnts[j].x - _pnts[i].x) * (pos.y - _pnts[i].y) / (_pnts[j].y-_pnts[i].y) + _pnts[i].x)) {
			result = !result;
		}
	}
	return result;
}

Common::Point Polygon::center() {
	Common::Point p(0,0);
	for (size_t i=0; i<_pnts.size(); i++)
		p += _pnts[i];
	return Common::Point (p.x/_pnts.size(), p.y/_pnts.size());
}

void Polygon::move(const Common::Point& c) {
	Common::Point p(0,0);
	for (size_t i=0; i<_pnts.size(); i++)
		p += _pnts[i];
	Common::Point offset(c.x - p.x/_pnts.size(), c.y - p.y/_pnts.size());
	for (size_t i=0; i<_pnts.size(); i++)
		_pnts[i] += offset;
}

void Polygon::fixBorders() {
	const int FIX_BORDER = 6;
	Common::Point c = center();
	for (size_t i=0; i<_pnts.size(); i++) {
		Common::Point& p = _pnts[i];
		if (p.x <= FIX_BORDER && p.x < c.x) p.x = 0;
		if (p.x >= 640-FIX_BORDER && p.x > c.x) p.x = 640;
		if (p.y <= FIX_BORDER && p.y < c.y) p.y = 0;
		if (p.y >= 480-FIX_BORDER && p.y > c.y) p.y = 480;
	}
#ifdef ANDROID
	const int WIDEN_BORDERS = 22;
	for (size_t i=0; i<_pnts.size(); i++) {
		Common::Point& p = _pnts[i];
		if (c.x <= WIDEN_BORDERS && p.x > c.x) p.x = max(p.x, c.x+WIDEN_BORDERS);
		if (c.x >= 640-WIDEN_BORDERS && p.x < c.x) p.x = min(p.x, c.x-WIDEN_BORDERS);
		if (c.y <= WIDEN_BORDERS && p.y > c.y) p.y = max(p.y, c.y+WIDEN_BORDERS);
		if (c.y >= 480-WIDEN_BORDERS && p.y < c.y) p.y = min(p.y, c.y-WIDEN_BORDERS);
	}
#endif
}

inline Actor* getManny() {
	foreach (Actor *a, g_grim->getActiveActors()) {
		if (a->getName() == "Manny")
			return a;
	}
	return 0;
}

HotspotMan::HotspotMan() :
	_selectMode (0), _initialized(false), _ctrlMode(0),_cutScene(0),
	_flashHS(false), _activeHS(nullptr) {
}

HotspotMan::~HotspotMan() {
}

Common::String readString(Common::SeekableReadStream* data) {
	int len = data->readSint32LE();
	char* buf = new char[len+1];
	data->read(buf,len);
	buf[len] = 0;
	Common::String s(buf);
	delete[] buf;
	return s;
}

void HotspotMan::initialize() {
	_hotspots.clear();
	Common::SeekableReadStream *data = g_resourceloader->openNewStreamFile("set.bin");
	int numSets = data->readSint32LE();
	for (int i=0; i<numSets; i++) {
		Common::String setID = readString(data);
		_hotspots[setID] = Common::Array<Hotspot>();
		int numHS = data->readSint32LE();
		for (int j=0; j<numHS; j++) {
			Hotspot hs;
			hs._id=readString(data);
			hs._desc=readString(data);
			hs._setup=data->readSint32LE();
			hs._type=data->readSint32LE();
			hs._objId=-1;
			int numPath = data->readSint32LE();
			for (int k=0; k<numPath; k++) {
				float pos[3];
				data->read(&pos, 3*sizeof(float));
				hs._path.push_back(Math::Vector3d(pos[0],pos[1],pos[2]));
			}
			int numPoly = data->readSint32LE();
			for (int k=0; k<numPoly; k++) {
				int x = data->readSint32LE(), y = data->readSint32LE();
				hs._region._pnts.push_back(Common::Point(x,y));
			}
			hs._region.fixBorders();
			_hotspots[setID].push_back(hs);
		}
	}
	loadFlashBitmaps();
	_initialized = true;
}

void HotspotMan::loadFlashBitmaps() {
	for(int i=0; i<8; i++) {
		Common::String fn = Common::String::format("cursor%d_hl.tga",i);
		_flashBitmaps[i] = Bitmap::create(fn.c_str());
		_flashBitmaps[i]->_data->load();
		_flashBitmaps[i]->_data->_hasTransparency = true;
	}
}

int HotspotMan::addHotspot(const Common::String& name, const Math::Vector3d& pos, const Common::String& scene) {
	HotObject hs;
	hs._pos = pos;
	hs._active = false;
	_hotobject.push_back(hs);

	int num = _hotobject.size() - 1;
	renameHotspot(num, name);
	return num;
}

void HotspotMan::disableAll() {
	for (size_t i=0; i<_hotobject.size(); i++)
		_hotobject[i]._active = false;
}

void HotspotMan::flashHotspots() {
	if (_cutScene > 0) return;
	_flashStart = g_system->getMillis();
	_flashHS = true;
	update();
}

void HotspotMan::drawActive(int debugMode) {
	if (_flashHS) {
		unsigned int curTime = g_system->getMillis();
		unsigned int delta = curTime - _flashStart;
		if (delta >= 1700) _flashHS = false;
		if ((delta % 900) < 450) {
			int setup = g_grim->getCurrSet()->getSetup();
			Common::Array<Hotspot>& hotspots = _hotspots[active_set()];
			for (size_t i=0; i<hotspots.size(); i++) {
				Hotspot& hs = hotspots[i];
				if (hs._setup == setup && (hs._objId<=0 || _hotobject[hs._objId]._active)) {
					if ((_ctrlMode == Dialog && hs._type >= 20) ||
						(_ctrlMode == Special && hs._type >= 10 && hs._type < 20) ||
						(_ctrlMode == Linear && hs._type >= 10 && hs._type < 20) ||
						((_ctrlMode == Normal || _ctrlMode == NoWalk) && hs._type < 10)) {
						int cu = hs._type % 10;
						if (hs._type == 3) cu = 2;
						if (hs._type == 4) cu = 1;
						Common::Point p = hs._region.center();
						_flashBitmaps[cu]->draw(p.x-15,p.y-15);
					}
				}
			}
		}
	}

	if (_ctrlMode == Dialog) {
		/*for (int i=0; i<_rows; i++) {
			PrimitiveObject x;
			x.createRectangle(Common::Point(_x0,_y0+i*_h),Common::Point(_x0+_w,_y0+(i+1)*_h),Color(200,200,200),false);
			x.draw();
		}*/
		//return;
	}
	if (_ctrlMode == Inventory) {
		const int dx = 4;//, dx2 = 6;
		int num_rows = (_inventory.size()-1)/_cols+1;
		int x1 = _x0+_w*_cols, y1 =_y0+_h*num_rows;
		g_driver->blackbox(0,0,640,480,0.4);
		g_driver->blackbox(_x0-dx,_y0-dx,x1+dx,y1+dx,0.9);
		PrimitiveObject rect;
		Color col(195,195,100);
		rect.createRectangle(Common::Point(_x0-dx,_y0-dx),Common::Point(x1+dx,y1+dx), col, false);
		rect.draw();
		//rect.createRectangle(Common::Point(_x0-dx2,_y0-dx2),Common::Point(x1+dx2,y1+dx2), col, false);
		//rect.draw();

		for (int j=0,idx=0; j<_rows; j++) {
			for (int i=0; i<_cols; i++,idx++) {
				if (idx >= (int)_inventory.size()) break;
				_inventory[idx]._bmp->draw(_x0+i*_w,_y0+j*_h);
			}
		}
	}
	if (debugMode > 0) {
		PrimitiveObject prim;
		for (size_t i=0; i<_hotobject.size(); i++) {
			if (_hotobject[i]._active) {
				Common::Point p1(_hotobject[i]._rect.left, _hotobject[i]._rect.top);
				Common::Point p2(_hotobject[i]._rect.right, _hotobject[i]._rect.bottom);
				prim.createRectangle(p1,p2,Color(200,0,200),false);
				prim.draw();

			}
		}
		Common::Array<Hotspot>& hotspots = _hotspots[active_set()];
		int setup = g_grim->getCurrSet()->getSetup();
		for (size_t i=0; i<hotspots.size(); i++) {
			if (hotspots[i]._setup == setup || _ctrlMode == Options)
				hotspots[i]._region.draw(Color(0,200,200));
		}
		if (_selectMode > 0)
			_selectPoly.draw(Color(200,200,200));
	}
	if(debugMode==2) {
		g_grim->getCurrSet()->setupCamera();
		Set *set = g_grim->getCurrSet();
		for (int s=0; s<set->getSectorCount(); s++) {
			Sector* sector = set->getSectorBase(s);
			if ((sector->getType() & Sector::WalkType) != Sector::WalkType &&
				(sector->getType() & Sector::HotType) != Sector::HotType)
				continue;
			int num = sector->getNumVertices();
			for (int j=0; j<num; j++) {
				int d0=j, d1=(j+1)%num;
				int x0,x1,y0,y1;
				Math::Vector3d v0 = sector->getVertices()[d0];
				Math::Vector3d v1 = sector->getVertices()[d1];
				g_driver->worldToScreen(v0, x0, y0);
				g_driver->worldToScreen(v1, x1, y1);
				PrimitiveObject p;
				Color col = Color(0,200,0);
				if ((sector->getType() & Sector::HotType) == Sector::HotType)
					col = Color(200,0,0);
				p.createLine(Common::Point(x0,y0),Common::Point(x1,y1),col);
				p.draw();
			}
		}
	}
}

void HotspotMan::updatePerspective() {
	//warning("Entering region %d", g_grim->getCurrSet()->getSetup());
	g_grim->getCurrSet()->setupCamera();
	for (size_t i=0; i<_hotobject.size(); i++) {
		int x=0,y=0;
		g_driver->worldToScreen(_hotobject[i]._pos,x,y);
		_hotobject[i]._rect = Common::Rect(x-10,y-10,x+10,y+10);
	}
	restoreCursor();
}

void HotspotMan::notifyWalk(int id) {
	/*if (_selectMode < 0)
		okKey(false);*/

#ifdef ANDROID
	Common::Array<Hotspot>& hotspots = _hotspots[active_set()];
	int setup = g_grim->getCurrSet()->getSetup();
	for (size_t i=0; i<hotspots.size(); i++) {
		Hotspot& hs = hotspots[i];
		if (hs._setup == setup && hs._objId == id) {
			Common::Point pnt = hs._region.center();
			g_grim->getCursor()->setPersistent(1, 1, pnt.x, pnt.y);
			_activeHS = &hs;
			return;
		}
	}
#endif
}

void HotspotMan::debug(int num) {
	for (size_t i=0; i<_hotobject.size(); i++) {
		if (!_hotobject[i]._active) continue;
		warning("hobj %c %s",_hotobject[i]._active ? 'A':'D',(_hotobject[i]._id + "/" + _hotobject[i]._desc).c_str());
	}
	Common::Array<Hotspot>& hotspots = _hotspots[active_set()];
	for (size_t i=0; i<hotspots.size(); i++) {
		char c = (hotspots[i]._objId>=0 && _hotobject[hotspots[i]._objId]._active) ? 'A' : 'D';
		warning("hspt %c %s",c,(hotspots[i]._id + "/" + hotspots[i]._desc).c_str());
	}
	warning("manny at %g,%g,%g",getManny()->getPos().x(), getManny()->getPos().y(), getManny()->getPos().z());
}

/*
void HotspotMan::reload(bool always) {
	_initialized = true;
	Common::String scene = g_grim->getCurrSet()->getName();
	if (_curScene != scene || always) {
		_curScene = scene;
		_hotspot.clear();

		char fname[256];
		sprintf(fname, "/Users/tpfaff/code/residualvm/hs/%s.hot",scene.c_str());
		FILE *fp = fopen(fname, "r");
		if (!fp) return;

		int setup, type, num, x, y;
		char id[256], name[256];
		while(fscanf(fp,"%d %s \"%100[^\"]\" %d %d", &setup, id, name, &type, &num) > 0) {
			Hotspot hs;
			hs._id = id;
			hs._setup = setup;
			hs._desc = name;
			hs._type = type;
			hs._objId = -1;
			for (int i=0; i<num; i++) {
				float p[3];
				fscanf(fp,"%g %g %g",&p[0],&p[1],&p[2]);
				Math::Vector3d v(p[0],p[1],p[2]);
				hs._path.push_back(v);
			}
			fscanf(fp,"%d",&num);
			for (int i=0; i<num; i++) {
				fscanf(fp,"%d %d", &x, &y);
				hs._region._pnts.push_back(Common::Point(x,y));
			}
			for (size_t j=0; j<_hotobject.size(); j++) {
				if (_hotobject[j]._id == hs._id)
					hs._objId = j;
			}
			_hotspot.push_back(hs);
		}
		fclose(fp);
	}
}*/

inline int min(int a, int b) { return a<b ? a : b; }
inline int max(int a, int b) { return a>b ? a : b; }

void HotspotMan::cancel() {
	_selectMode = 0;
	_selectPoly._pnts.clear();
}

Common::String get_scene_id() {
	Common::String scene = g_grim->getCurrSet()->getName();
	for(int i=0; i<4; i++) scene.deleteLastChar();
	return scene;
}

Common::String HotspotMan::active_set() {
	return (_curOption.empty() || _ctrlMode != Options) ?
				g_grim->getCurrSet()->getName() : _curOption;
}

void split_string(const Common::String& str, char split, Common::String& left, Common::String& right) {
	left = ""; right = "";
	bool useleft = true;
	for (size_t i=0; i<str.size(); i++) {
		if (str[i]==split)
			useleft = false;
		else if (useleft)
			left += str[i];
		else
			right += str[i];
	}
}

void HotspotMan::append_hotspot(const Common::String& id, const Common::String& name, int type) {
	Common::String scene = get_scene_id();
	char fname[256];
	sprintf(fname, "/Users/tpfaff/code/residualvm/hs/%s.set.hot",scene.c_str());
	warning("saving to %s",fname);
	FILE *fp = fopen(fname,"a");
	if (!fp) error("cant open hostpot file");
	fprintf(fp, "%d %s \"%s\" %d %d",_lastSetup, id.c_str(), name.c_str(),type,_selectPath.size());
	for (size_t i=0; i<_selectPath.size();i++) {
		Math::Vector3d v = _selectPath[i];
		fprintf(fp, " %g %g %g", v.x(), v.y(), v.z());
	}
	fprintf(fp, " %d", _selectPoly._pnts.size()-1);
	for (size_t i=0; i<_selectPoly._pnts.size()-1; i++)
		fprintf(fp," %d %d",_selectPoly._pnts[i].x, _selectPoly._pnts[i].y);
	fprintf(fp,"\n");
	fclose(fp);
}

void HotspotMan::okKey(bool shift) {
	if (_selectMode > 0) {
		Common::String defaultText = "", desc="";

		// find possible object defaults
		for (size_t i=0; i<_hotobject.size(); i++) {
			if (!_hotobject[i]._active) continue;
			Common::Point p ((_hotobject[i]._rect.left + _hotobject[i]._rect.right)/2,
							 (_hotobject[i]._rect.top + _hotobject[i]._rect.bottom)/2);
			if (_selectPoly.contains(p)) {
				defaultText = _hotobject[i]._id;
				desc = "[default: " + _hotobject[i]._desc + " ]";
				defaultText.erase(0,3);
				break;
			}
		}

		// object id
		InputDialog dlg("Object ID " +desc, defaultText);
		int res = dlg.runModal();
		Common::String id = dlg.getString();

		// The KeyUp event for CTRL has been eat by the gui loop, so we
		// need to reset it manually.
		g_grim->clearEventQueue();
		if (res && !id.empty()) {
			_lastSetup = g_grim->getCurrSet()->getSetup();
			_selectPath.clear();
			if (id[0] == 's' && id[1] == 'x') {
				int type = 10;
				Common::String code,name;
				split_string(id,'_',code,name);
				if (name == "star") type = 11;
				if (name == "go") type = 12;
				if (name == "right") type = 13;
				if (name == "left") type = 14;
				if (name == "up") type = 15;
				if (name == "down") type = 16;
				_selectPath.push_back(Math::Vector3d(0,0,0));
				append_hotspot(id,name, type);
			} else if (id[0] == 't' && id[1] == 'x') {
				Common::String name = "_";
				int type = 1;
				id = "/" + get_scene_id() + id;
				for (size_t i=0; i<_hotobject.size(); i++) {
					if (_hotobject[i]._id == id)
						name = _hotobject[i]._desc;
				}
				if (name=="door")
					type = 2;
				_selectPath.push_back(Math::Vector3d(0,0,0));
				append_hotspot(id,name, type);
			} else {
				_selectMode = -1;
				_lastName = id;
				warning("ok, walk to position and hit ENTER / Shift+ENTER");
				return;
			}
		}
		_selectMode = 0;
		_selectPoly._pnts.clear();
	} else if (_selectMode < 0) {
		warning("ok, saving position");
		Actor* manny = getManny();
		if (manny) {
			_selectPath.push_back(manny->getPos());
			if (!shift) {
				append_hotspot("to",_lastName, 3);
				_selectMode = 0;
				_selectPoly._pnts.clear();
			}
		}
	}
}

// get min distance s for x0+s*x1, y0+t*y1
double line_line_dist(const Math::Vector3d& x0, const Math::Vector3d& x1,
					  const Math::Vector3d& y0, const Math::Vector3d& y1) {
	double a = x1.dotProduct(x1), b = -x1.dotProduct(y1), c = y1.dotProduct(y1);
	double d = -x1.dotProduct(x0-y0), e = y1.dotProduct(x0-y0);
	return (c*d-b*e)/(c*a-b*b);
}

void HotspotMan::event(const Common::Point& cursor, const Common::Event& ev, int debugMode, bool doubleClick) {
	bool climbing = LuaBase::instance()->queryVariable("system.currentActor.is_climbing", false) != 0;

	int button = 0;
	if (ev.type == Common::EVENT_LBUTTONDOWN)
		button = 1;
	else if (ev.type == Common::EVENT_RBUTTONDOWN)
		button = 2;
	else if (ev.type == Common::EVENT_MBUTTONDOWN)
		button = 3;
	else if (ev.type == Common::EVENT_SCROLL_UP)
		button = 4;
	else if (ev.type == Common::EVENT_SCROLL)
		button = 4;
	if (ev.kbd.hasFlags(Common::KBD_ALT))
		button = 2;
	if (ev.kbd.hasFlags(Common::KBD_CTRL))
		button = 3;
	update();

	// --- first, try active hotspot
	if (_activeHS) {
		Common::Point delta = cursor - _activeHS->_region.center();
		if (abs(delta.x) < 40 && abs(delta.y) < 40) {
			LuaObjects objects;
			objects.add(1);
			objects.add(_activeHS->_objId);
			objects.add(0);
			LuaBase::instance()->callback("mouseCommand", objects);
			_activeHS = nullptr;
			return;
		}
	}

	_lastCursor = cursor;
#ifdef ANDROID
	restoreCursor();
	if (_cutScene == 0 && _ctrlMode == Normal && ev.type != Common::EVENT_SCROLL) {
		g_grim->getCursor()->setPersistent(0, 8, cursor.x, cursor.y);
	}
#endif

	if (_ctrlMode == Dialog && button > 0) {
		// dialog mode
		int dialog = inBox(cursor), click = 1;
		if (button==2) click = 2;
		LuaObjects objects;
		objects.add(dialog);
		objects.add(click);
		LuaBase::instance()->callback("dialogSelect", objects);
	} else if (_ctrlMode == Inventory && button > 0) {
		for (int j=0,idx=0; j<_rows; j++) {
			for (int i=0; i<_cols; i++,idx++) {
				if (idx >= (int)_inventory.size()) break;
				if (cursor.x >= _x0+i*_w && cursor.x <_x0+(i+1)*_w &&
					cursor.y >= _y0+j*_h && cursor.y <_y0+(j+1)*_h) {
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

	if (debugMode > 0 && button == 1 && _selectMode >=0) {
		Common::Point pnt(cursor.x, cursor.y);
		if (_selectMode == 0)
			_selectPoly._pnts.push_back(pnt);
		else
			_selectPoly._pnts[_selectMode] = pnt;
		_selectPoly._pnts.push_back(pnt);
		_selectMode++;
	}

	if (debugMode==0) {

		// ------- click on hot spots ------------
		Common::Array<Hotspot>& hotspots = _hotspots[active_set()];
		int setup = g_grim->getCurrSet()->getSetup();
		for (size_t i=0; i<hotspots.size(); i++) {
			Hotspot& hs = hotspots[i];
			if (_ctrlMode == Options && hs._region.contains(cursor)) {
				LuaObjects objects;
				objects.add(active_set().c_str());
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
					if (button==0 && _ctrlMode != Special) continue;
					LuaObjects objects;
					objects.add(hs._id.c_str());
					objects.add(button);
					LuaBase::instance()->callback("mouseSpecial", objects);
					return;
				}
				// normal mode
				int gid = hs._objId;
				if ((_ctrlMode != Normal && _ctrlMode != NoWalk) ||
					hs._type >= 10 || hs._type == 3 || button==0) continue;
				if (gid<=0 || !_hotobject[gid]._active) continue;
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

			for (size_t i=0; i<hotspots.size(); i++) {
				Hotspot& hs = hotspots[i];
				if (hs._setup == setup && hs._region.contains(cursor) && !climbing) {
					// normal mode walkbox
					int gid = hs._objId;
					if (hs._type == 3 && (gid <= 0 || _hotobject[gid]._active)) {
						float* buf = new float[hs._path.size()*3];
						LuaObjects objects;
						objects.add(button);
						objects.add(doubleClick ? 1 : 0);
						for (int k=0; k<(int)hs._path.size(); k++) {
							buf[3*k] = hs._path[k].x();
							buf[3*k+1] = hs._path[k].y();
							buf[3*k+2] = hs._path[k].z();
						}
						objects.add(buf, hs._path.size()*3);
						LuaBase::instance()->callback("mouseWalk", objects);
						delete[] buf;
						return;
					}
				}
			}

			if (_ctrlMode == Normal)
				freeClick(cursor,button,doubleClick,climbing);
			else if (_ctrlMode == NoWalk) {
				LuaObjects objects;
				objects.add(button);
				LuaBase::instance()->callback("mouseNowalk", objects);
			}
		}
	}

	if (_ctrlMode == Linear && button > 0) {
		// linear mode
		Actor* manny = getManny();
		if (!manny) return;
		Math::Vector3d r0, r1;
		g_grim->getCurrSet()->setupCamera();
		g_driver->raycast(cursor.x, cursor.y, r0, r1);
		Math::Vector3d p0 = manny->getPos(), p1 = _axis;
		r0 -= _axis * _offset;
		r1 -= _axis * _offset;
		float s = line_line_dist(p0,p1,r0,r1);

		LuaObjects objects;
		objects.add(p0.dotProduct(_axis) + s);
		objects.add(s);
		objects.add(button);
		LuaBase::instance()->callback("linearMove", objects);
		return;
	}
}
void HotspotMan::getName(const Common::Point& pos) {
	for (size_t i=0; i<_hotobject.size(); i++) {
		if (_hotobject[i]._active && _hotobject[i]._rect.contains(pos)) {
			//cursor->setCursor(1);
			warning("current obj %s %s",_hotobject[i]._id.c_str(),_hotobject[i]._desc.c_str());
		}
	}
}

void HotspotMan::hover(const Common::Point& pos) {
	if (g_grim->getCurrSet() == NULL) return;
	update();
	_lastCursor = pos;
	Cursor* cursor = g_grim->getCursor();
	cursor->setPersistent(0, -1);
	cursor->setCursor(0);

	if (_cutScene > 0 && _ctrlMode != Dialog && _ctrlMode != Options) {
#ifdef ANDROID
		cursor->setPersistent(0, 7, 320, 240);
#else
		cursor->setCursor(7);
#endif
		return;
	}

	int setup = g_grim->getCurrSet()->getSetup();
	Common::Array<Hotspot>& hotspots = _hotspots[active_set()];

	int select = 0;
	for (size_t i=0; i<hotspots.size(); i++) {
		if (_activeHS) continue;
		if ((hotspots[i]._setup == setup || _ctrlMode == Options) &&
			hotspots[i]._region.contains(pos)) {
			if (hotspots[i]._objId>=0 && !_hotobject[hotspots[i]._objId]._active)
				continue;
			int type = hotspots[i]._type;
			if (_ctrlMode == Normal || _ctrlMode == NoWalk) {
				if (type>=10) continue;
				cursor->setCursor((type==2 ||type==3) ? 2 : 1);
			} else if (_ctrlMode == Dialog) {
				if (type<20 || type >= 30) continue;
				cursor->setCursor(type-20);
			} else if (_ctrlMode == Special || _ctrlMode == Linear) {
				if (type<10 || type >= 20) continue;
				cursor->setCursor(type-10);
			} else if (_ctrlMode == Options) {
				if (type<30 || type >= 40) continue;
				cursor->setCursor(type-30);
				select = hotspots[i]._setup;
			}
		}
	}
	if (_selectMode > 0)
		_selectPoly._pnts[_selectMode] = Common::Point(pos.x, pos.y);

	if (_ctrlMode == Dialog) {
		// dialog mode
		int dialog = inBox(pos), click = 0;
		LuaObjects objects;
		objects.add(dialog);
		objects.add(click);
		LuaBase::instance()->callback("dialogSelect", objects);
	} else if (_ctrlMode == Inventory) {
		for (int j=0,idx=0; j<_rows; j++) {
			for (int i=0; i<_cols; i++,idx++) {
				if (idx >= (int)_inventory.size()) break;
				if (pos.x >= _x0+i*_w && pos.x <_x0+(i+1)*_w &&
					pos.y >= _y0+j*_h && pos.y <_y0+(j+1)*_h) {
					cursor->setCursor(1);
				}
			}
		}
	} else if (_ctrlMode == Options) {
		LuaObjects objects;
		objects.add(active_set().c_str());
		objects.add(select);
		LuaBase::instance()->callback("mouseHover", objects);
		return;
	}
}

Common::Point HotspotMan::mannyPos2D(float zOffset) {
	int x=0, y=0;
	Actor* manny = getManny();
	if (g_grim->getCurrSet() && manny) {
		Math::Vector3d p = manny->getPos();
		p += Math::Vector3d(0,0,zOffset);
		g_grim->getCurrSet()->setupCamera();
		g_driver->worldToScreen(p,x,y);
	}
	return Common::Point(x,y);
}

void HotspotMan::freeClick(const Common::Point& cursor, int button, bool doubleClick, bool climbing) {
	Math::Vector3d r0, r1;
	g_grim->getCurrSet()->setupCamera();
	g_driver->raycast(cursor.x, cursor.y, r0, r1);

	// climbing
	if (climbing) {
		Actor* manny = getManny();
		if (!manny) return;
		r0 += Math::Vector3d(0,0,-0.25);
		r1 += Math::Vector3d(0,0,-0.25); // center on mannys body, not feet
		Math::Vector3d p0 = manny->getPos(), p1 = Math::Vector3d(0,0,1);
		float s = line_line_dist(p0,p1,r0,r1);

		LuaObjects objects;
		objects.add(p0.z() + s);
		objects.add(s);
		LuaBase::instance()->callback("climbTowards", objects);
		return;
	}

	// walking
	for (int i=0; i<50; i++) {
		Set *set = g_grim->getCurrSet();
		for (int s=0; s<set->getSectorCount(); s++) {
			Sector* sector = set->getSectorBase(s);
			if ((sector->getType() & Sector::WalkType) != Sector::WalkType)
				continue;
			Math::Vector3d v = sector->raycast(r0,r1);
			if (sector->isPointInSector(v)) {
				LuaObjects objects;
				objects.add(button);
				objects.add(doubleClick ? 1 : 0);
				float p[3] = {v.x(), v.y(), v.z()};
				objects.add(p,3);
				LuaBase::instance()->callback("mouseWalk", objects);
				return;
			}
		}
		r0.z() -= 0.03; r1.z() -= 0.03;
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
	for (int i=0; i<num; i++) {
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
	for (int i=0; i<numSets; i++) {
		Common::String setname = savedState->readString();
		int numSpots = savedState->readLESint32();
		_hotspots[setname].clear();
		for (int j=0; j<numSpots; j++) {
			Hotspot hs;
			hs._id = savedState->readString();
			hs._desc = savedState->readString();
			int pathlen = savedState->readLESint32();
			for (int k=0; k<pathlen; k++)
				hs._path.push_back(savedState->readVector3d());
			int polylen = savedState->readLESint32();
			for (int k=0; k<polylen; k++) {
				int x = savedState->readLEUint16();
				int y = savedState->readLEUint16();
				hs._region._pnts.push_back(Common::Point(x,y));
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
	for (size_t i=0; i<_hotobject.size(); i++) {
		HotObject& hs = _hotobject[i];
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
	for(HotDict::iterator it=_hotspots.begin(); it != _hotspots.end(); ++it) {
		savedState->writeString(it->_key);
		Common::Array<Hotspot>& hotspots = it->_value;
		savedState->writeLESint32(hotspots.size());
		for (size_t i=0; i< hotspots.size(); i++) {
			Hotspot& hs = hotspots[i];
			savedState->writeString(hs._id);
			savedState->writeString(hs._desc);
			savedState->writeLESint32(hs._path.size());
			for (size_t j=0; j<hs._path.size(); j++)
				savedState->writeVector3d(hs._path[j]);
			savedState->writeLESint32(hs._region._pnts.size());
			for (size_t j=0; j<hs._region._pnts.size(); j++) {
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
int HotspotMan::inBox(const Common::Point& p) {
	for (int j=0; j<_rows; j++) {
		for (int i=0; i<_cols; i++) {
			if (p.x >= _x0+i*_w && p.x < _x0+(i+1)*_w &&
				p.y >= _y0+j*_h && p.y < _y0+(j+1)*_h)
				return j*(_cols)+i+1;
		}
	}
	return 0;
}

void HotspotMan::update() {
	Common::Array<Hotspot>& hotspots = _hotspots[active_set()];
	for (size_t i=0; i<hotspots.size(); i++) {
		if (hotspots[i]._type != 4) continue;
		foreach(Actor* a,g_grim->getActiveActors()) {
			if (a->getName()==hotspots[i]._desc && hotspots[i]._path.size()==1) {
				g_grim->getCurrSet()->setupCamera();
				int x=0,y=0;
				Math::Vector3d trans = hotspots[i]._path[0];
				Math::Quaternion quat = a->getRotationQuat();
				Math::Quaternion quatInv(-quat.x(),-quat.y(),-quat.z(),quat.w());
				quatInv.toMatrix().transform(&trans, true);
				Math::Vector3d pos = a->getPos() + trans;
				g_driver->worldToScreen(pos,x,y);
				Common::Point p(x,y);
				hotspots[i]._region.move(p);
			}
		}
	}
}

void HotspotMan::updateHotspot(const Common::String& id, const Math::Vector3d& pos, int vis) {
	Common::Array<Hotspot>& hotspots = _hotspots[active_set()];
	for (size_t i=0; i<hotspots.size(); i++) {
		if (hotspots[i]._id == id) {
			int x=(int)pos.x(), y=(int)pos.y();
			if (vis>0) {
				g_grim->getCurrSet()->setupCamera();
				g_driver->worldToScreen(pos,x,y);
			}
			Common::Point p(x,y);
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
	_activeHS = nullptr;
	g_grim->getCursor()->setPersistent(1, -1);
#ifdef ANDROID
	g_grim->getCursor()->setCursor(-1);
	if (_cutScene > 0 && _ctrlMode != Dialog && _ctrlMode != Options)
		g_grim->getCursor()->setPersistent(0, 7, 320, 240);
	else
		g_grim->getCursor()->setPersistent(0, -1);
#else
	hover(_lastCursor);
#endif
}

void HotspotMan::renameHotspot(int num, const Common::String& name) {
	Common::String id = name, desc = name;
	for (size_t i=1;i<id.size();i++) {
		if (id[i]=='/') {
			id.erase(i);
			desc.erase(0,i+1);
			break;
		}
	}

	int n=0;
	for (size_t i=0; i<_hotobject.size(); i++) {
		if (_hotobject[i]._id.hasPrefix(id))
			n++;
	}
	if (n>0)
		id += Common::String::format("_%d",n);
	_hotobject[num]._id = id;
	_hotobject[num]._desc = desc;
	//warning("renaming to %s / %s",id.c_str(),desc.c_str() );

	// re-link to hotspots
	for (HotDict::iterator it=_hotspots.begin(); it != _hotspots.end(); ++it) {
		//Common::String s = it->_key;
		Common::Array<Hotspot>& hotspots = it->_value;
		for (size_t i=0; i<hotspots.size(); i++) {
			if (hotspots[i]._id == id)
				hotspots[i]._objId = num;
		}
	}
}

void HotspotMan::addInventory(const Common::String& id, const Common::String& pic) {
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

} /* namespace */
