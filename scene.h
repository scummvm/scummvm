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
#include "tinygl/gl.h"
#include <string>

class CMap;
class TextSplitter;

// The Lua code calls this a "set".

class Scene {
public:
	Scene(const char *name, const char *buf, int len);
	~Scene();

	int _minVolume;
	int _maxVolume;

	void drawBackground() const {
		if (_currSetup->_bkgndZBm != NULL) // Some screens have no zbuffer mask (eg, Alley)
			_currSetup->_bkgndZBm->draw();

		if (_currSetup->_bkgndBm == NULL) {
			// This should fail softly, for some reason jumping to the signpost (sg) will load
			// the scene in such a way that the background isn't immediately available
			warning("Background hasn't loaded yet for setup %s in %s!", _currSetup->_name.c_str(), _name.c_str());
		} else {
			_currSetup->_bkgndBm->draw();
		}
	}
	void drawBitmaps(ObjectState::Position stage);
	void setupCamera() {
		_currSetup->setupCamera();
	}

	void setupLights();

	void setSoundPosition(const char *soundName, Vector3d pos);
	void setSoundParameters(int minVolume, int maxVolume);
	void getSoundParameters(int *minVolume, int *maxVolume);

	const char *name() const { return _name.c_str(); }

	void setLightEnableState(bool state) {
		_enableLights = state;
	}

	void setSetup(int num);
	int setup() const { return _currSetup - _setups; }

	// Sector access functions
	int getSectorCount() {
		// TODO: Find where this is called before we're initialized
		if (this == NULL)
			return 0;
		return _numSectors;
	}
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

	void deleteObjectState(ObjectState *s) {
		_states.remove(s);
	}

	void moveObjectStateToFirst(ObjectState *s);
	void moveObjectStateToLast(ObjectState *s);

	ObjectState *findState(const char *filename);

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

	bool locked;

private:

	std::string _name;
	int _numCmaps;
	ResPtr<CMap> *_cmaps;
	int _numSetups, _numLights, _numSectors;
	bool _enableLights;
	Sector *_sectors;
	Light *_lights;
	Setup *_setups;
public:
	Setup *_currSetup;
private:
	typedef std::list<ObjectState*> StateList;
	StateList _states;
};

#endif
