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

#ifndef TETRAEDGE_TE_TE_SCENE_WARP_H
#define TETRAEDGE_TE_TE_SCENE_WARP_H

#include "tetraedge/te/te_camera.h"
#include "tetraedge/te/te_warp.h"
#include "tetraedge/te/te_warp_bloc.h"

namespace Tetraedge {

class TePickMesh;

class TeSceneWarp {
	friend class TeSceneWarpXmlParser;

public:
	class Animation {
	public:
		Animation() : _fps(0) {}
		Common::String _name;
		float _fps;
	};
	class Exit {
	public:
		Exit() : _nbWarpBlock(0) {}
		Common::String _name;
		Common::String _linkedWarp;
		unsigned long _markerId;
		int _nbWarpBlock;
		float _camAngleX;
		float _camAngleY;
		Common::List<TeWarpBloc> _warpBlocs;
		Common::List<Common::String> _strList;
	};
	class Object {
	public:
		Common::String _name;
		Common::String _str2;
		unsigned long _markerId;
		Common::List<Common::String> _strList;
	};
	class WarpEvent {
	public:
		Common::String _name;
		Common::List<Common::String> _strList;
	};

	TeSceneWarp();
	~TeSceneWarp();

	//void activeMarker(const Common::String &s1, const Common::String &s2) // unused
	//Common::String addMarkerToAnimation(const Common::String &animName); // unused
	void close();
	//void createExit(); // unused
	const Animation *getAnimation(const Common::String &name);
	const Exit *getExit(const Common::String &linkedWarp);
	const Object *getObject(const Common::String &name);
	const WarpEvent *getWarpEvent(const Common::String &name); // TODO: what's the right return type?
	bool load(const Common::String &name, TeWarp *warp, bool flag);
	//void moveIndicator(const Common::String &, const Common::String &, const TeVector2s32 &)); // unused
	//void popBlockToExit(const Common::String &name, TePickMesh *mesh); // unused
	//void pushBlockToExit(const Common::String &name, TePickMesh *mesh); // unused 
	//void removeExit(const Common::String &name); // unused
	void render(const TeCamera &cam1, const TeCamera &cam2, const Common::String &str1, const Common::String &str2);
	//void save(); // unused

	void addObject(const Object &obj) { _objects.push_back(obj); }
	void addExit(const Exit &exit) { _exits.push_back(exit); }
	void addAnimation(const Animation &anim) { _animations.push_back(anim); }
	void addWarpEvent(const WarpEvent &event) { _warpEvents.push_back(event); }

private:
	Common::List<Object> _objects;
	Common::List<Exit> _exits;
	Common::List<Animation> _animations;
	Common::List<WarpEvent> _warpEvents;
	Common::String _name;
	TeWarp *_warp;
	int _numExitsCreated;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_SCENE_WARP_H
