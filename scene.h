// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef SCENE_H
#define SCENE_H

#include "vector3d.h"
#include "bitmap.h"
#include "color.h"
#include "debug.h"
#include "walkplane.h"
#include "objectstate.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <string>

class CMap;
class TextSplitter;

// The Lua code calls this a "set".

class Scene {
public:
	Scene(const char *name, const char *buf, int len);
	~Scene();

	void drawBackground() const {
		if (_currSetup->_bkgndZBm != NULL) // Some screens have no zbuffer mask (eg, Alley)
			_currSetup->_bkgndZBm->draw();

		if (_currSetup->_bkgndBm == NULL) {
			error("Null background for setup %s in %s", _currSetup->_name.c_str(), _name.c_str());
			return;
		} 
		_currSetup->_bkgndBm->draw();
	}
	void drawBitmaps(ObjectState::Position stage);
	void setupCamera() {
		_currSetup->setupCamera();
	}

	const char *name() const { return _name.c_str(); }

	void setSetup(int num);
	int setup() const { return _currSetup - _setups; }

	// Sector access functions
	int getSectorCount() { return _numSectors; }
	Sector *getSectorBase(int id) { 
		if ((_numSectors >= 0) && (id < _numSectors))
			return &_sectors[id];
		else
			return NULL;
	}
	Sector *findPointSector(Vector3d p, int flags);
	void findClosestSector(Vector3d p, Sector **sect, Vector3d *closestPt);

	void addObjectState(ObjectState *s) {
		_states.push_back(s);
	}
	ObjectState *findState(const char *filename);

private:
	struct Setup {		// Camera setup data
		void load(TextSplitter &ts);
		void setupCamera() const;
		std::string _name;
		ResPtr<Bitmap> _bkgndBm, _bkgndZBm;
		Vector3d _pos, _interest;
		float _roll, _fov, _nclip, _fclip;
	};

	struct Light {		// Scene lighting data
		void load(TextSplitter &ts);
		std::string _name;
		std::string _type;
		Vector3d _pos, _dir;
		Color _color;
		float _intensity, _umbraangle, _penumbraangle;
	};

	std::string _name;
	int _numCmaps;
	ResPtr<CMap> *_cmaps;
	int _numSetups, _numLights, _numSectors;
	Sector *_sectors;
	Light *_lights;
	Setup *_setups;
	Setup *_currSetup;

	typedef std::list<ObjectState*> StateList;
	StateList _states;
};

#endif
