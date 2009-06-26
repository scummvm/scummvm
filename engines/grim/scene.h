/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GRIM_SCENE_H
#define GRIM_SCENE_H

#include "engines/grim/color.h"
#include "engines/grim/walkplane.h"
#include "engines/grim/objectstate.h"

namespace Grim {

class SaveGame;

class Scene {
public:
	Scene(const char *name, const char *buf, int len);
	~Scene();

	void saveState(SaveGame *savedState);

	int _minVolume;
	int _maxVolume;

	void drawBackground() const {
		if (_currSetup->_bkgndZBm) // Some screens have no zbuffer mask (eg, Alley)
			_currSetup->_bkgndZBm->draw();

		if (!_currSetup->_bkgndBm) {
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

	void setSoundPosition(const char *soundName, Graphics::Vector3d pos);
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
		return _numSectors;
	}
	Sector *getSectorBase(int id) {
		if ((_numSectors >= 0) && (id < _numSectors))
			return &_sectors[id];
		else
			return NULL;
	}
	Sector *findPointSector(Graphics::Vector3d p, int flags);
	void findClosestSector(Graphics::Vector3d p, Sector **sect, Graphics::Vector3d *closestPt);

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
		Common::String _name;
		Bitmap *_bkgndBm, *_bkgndZBm;
		Graphics::Vector3d _pos, _interest;
		float _roll, _fov, _nclip, _fclip;
	};

	struct Light {		// Scene lighting data
		void load(TextSplitter &ts);
		Common::String _name;
		Common::String _type;
		Graphics::Vector3d _pos, _dir;
		Color _color;
		float _intensity, _umbraangle, _penumbraangle;
	};

	bool _locked;

private:

	Common::String _name;
	int _numCmaps;
	CMap **_cmaps;
	int _numSetups, _numLights, _numSectors, _numObjectStates;
	bool _enableLights;
	Sector *_sectors;
	Light *_lights;
	Setup *_setups;
public:
	Setup *_currSetup;
private:
	typedef Common::List<ObjectState*> StateList;
	StateList _states;
};

} // end of namespace Grim

#endif
