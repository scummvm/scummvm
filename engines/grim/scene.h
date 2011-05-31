/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 */

#ifndef GRIM_SCENE_H
#define GRIM_SCENE_H

#include "engines/grim/color.h"
#include "engines/grim/walkplane.h"
#include "engines/grim/objectstate.h"

namespace Common {
	class MemoryReadStream;
}
namespace Grim {

class SaveGame;
class CMap;

class Scene {
public:
	Scene(const Common::String &name, const char *buf, int len);
	Scene();
	~Scene();

	int32 getId();

	void loadText(TextSplitter &ts);
	void loadBinary(Common::MemoryReadStream *ms);

	void saveState(SaveGame *savedState) const;
	bool restoreState(SaveGame *savedState);

	int _minVolume;
	int _maxVolume;

	void drawBackground() const;
	void drawBitmaps(ObjectState::Position stage);
	void setupCamera() {
		_currSetup->setupCamera();
	}

	void setupLights();

	void setSoundPosition(const char *soundName, Graphics::Vector3d pos);
	void setSoundPosition(const char *soundName, Graphics::Vector3d pos, int minVol, int maxVol);
	void setSoundParameters(int minVolume, int maxVolume);
	void getSoundParameters(int *minVolume, int *maxVolume);

	const Common::String &getName() const { return _name; }

	void setLightEnableState(bool state) {
		_enableLights = state;
		_lightsConfigured = false;
	}
	void setLightsDirty();
	void setLightIntensity(const char *light, float intensity);
	void setLightIntensity(int light, float intensity);
	void setLightPosition(const char *light, const Graphics::Vector3d &pos);
	void setLightPosition(int light, const Graphics::Vector3d &pos);

	void setSetup(int num);
	int getSetup() const { return _currSetup - _setups; }

	// Sector access functions
	int getSectorCount() { return _numSectors; }

	Sector *getSectorBase(int id);

	Sector *findPointSector(const Graphics::Vector3d &p, Sector::SectorType type);
	void findClosestSector(const Graphics::Vector3d &p, Sector **sect, Graphics::Vector3d *closestPt);

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
		void loadBinary(Common::MemoryReadStream *ms);
		void setupCamera() const;
		Common::String _name;
		Bitmap *_bkgndBm, *_bkgndZBm;
		Graphics::Vector3d _pos, _interest;
		float _roll, _fov, _nclip, _fclip;
	};

	struct Light {		// Scene lighting data
		void load(TextSplitter &ts);
		void loadBinary(Common::MemoryReadStream *ms);
		Common::String _name;
		Common::String _type;
		Graphics::Vector3d _pos, _dir;
		Color _color;
		float _intensity, _umbraangle, _penumbraangle;
	};

	CMap *getCMap() {
		if (!_cmaps || ! _numCmaps)
			return NULL;
		return _cmaps[0];
	};

	Setup *getCurrSetup() { return _currSetup; }

private:
	bool _locked;
	void setId(int32 id);
	Common::String _name;
	int _numCmaps;
	ObjectPtr<CMap> *_cmaps;
	int _numSetups, _numLights, _numSectors, _numObjectStates;
	bool _enableLights;
	Sector **_sectors;
	Light *_lights;
	Setup *_setups;
	bool _lightsConfigured;

	Setup *_currSetup;
	typedef Common::List<ObjectState*> StateList;
	StateList _states;

	int _id;
	static int s_id;

	friend class GrimEngine;
};

} // end of namespace Grim

#endif
