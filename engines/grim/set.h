/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_SET_H
#define GRIM_SET_H

#include "engines/grim/pool.h"
#include "engines/grim/object.h"
#include "engines/grim/color.h"
#include "engines/grim/sector.h"
#include "engines/grim/objectstate.h"

namespace Common {
	class SeekableReadStream;
}
namespace Grim {

class SaveGame;
class CMap;
class Light;

class Set : public PoolObject<Set> {
public:
	Set(const Common::String &name, Common::SeekableReadStream *data);
	Set();
	~Set();

	static int32 getStaticTag() { return MKTAG('S', 'E', 'T', ' '); }

	void loadText(TextSplitter &ts);
	void loadBinary(Common::SeekableReadStream *data);

	void saveState(SaveGame *savedState) const;
	bool restoreState(SaveGame *savedState);

	int _minVolume;
	int _maxVolume;

	void drawBackground() const;
	void drawBitmaps(ObjectState::Position stage);
	void setupCamera() {
		_currSetup->setupCamera();
	}

	void setupLights(const Math::Vector3d &pos);

	void setSoundPosition(const char *soundName, const Math::Vector3d &pos);
	void setSoundPosition(const char *soundName, const Math::Vector3d &pos, int minVol, int maxVol);
	void setSoundParameters(int minVolume, int maxVolume);
	void getSoundParameters(int *minVolume, int *maxVolume);

	const Common::String &getName() const { return _name; }

	void setLightEnableState(bool state) {
		_enableLights = state;
	}
	void setLightIntensity(const char *light, float intensity);
	void setLightIntensity(int light, float intensity);
	void setLightPosition(const char *light, const Math::Vector3d &pos);
	void setLightPosition(int light, const Math::Vector3d &pos);
	void setLightEnabled(const char *light, bool enabled);
	void setLightEnabled(int light, bool enabled);
	void turnOffLights();

	void setSetup(int num);
	int getSetup() const { return _currSetup - _setups; }

	// Sector access functions
	int getSectorCount() { return _numSectors; }

	Sector *getSectorBase(int id);
	Sector *getSector(const Common::String &name);
	Sector *getSector(const Common::String &name, const Math::Vector3d &pos);

	Sector *findPointSector(const Math::Vector3d &p, Sector::SectorType type);
	void findClosestSector(const Math::Vector3d &p, Sector **sect, Math::Vector3d *closestPt);
	void shrinkBoxes(float radius);
	void unshrinkBoxes();

	void addObjectState(const ObjectState::Ptr &s);
	void deleteObjectState(const ObjectState::Ptr &s) {
		_states.remove(s);
	}

	void moveObjectStateToFront(const ObjectState::Ptr &s);
	void moveObjectStateToBack(const ObjectState::Ptr &s);

	ObjectState *addObjectState(int setupID, ObjectState::Position pos, const char *bitmap, const char *zbitmap, bool transparency);
	ObjectState *findState(const Common::String &filename);

	struct Setup {		// Camera setup data
		void load(TextSplitter &ts);
		void loadBinary(Common::SeekableReadStream *data);
		void setupCamera() const;
		void saveState(SaveGame *savedState) const;
		bool restoreState(SaveGame *savedState);

		Common::String _name;
		Bitmap::Ptr _bkgndBm, _bkgndZBm;
		Math::Vector3d _pos, _interest;
		float _roll, _fov, _nclip, _fclip;
	};

	CMap *getCMap() {
		if (!_cmaps || ! _numCmaps)
			return NULL;
		return _cmaps[0];
	};

	Setup *getCurrSetup() { return _currSetup; }

private:
	bool _locked;
	Common::String _name;
	int _numCmaps;
	ObjectPtr<CMap> *_cmaps;
	int _numSetups, _numLights, _numSectors, _numObjectStates;
	bool _enableLights;
	Sector **_sectors;
	Light *_lights;
	Common::List<Light *> _lightsList;
	Setup *_setups;

	Setup *_currSetup;
	typedef Common::List<ObjectState::Ptr> StateList;
	StateList _states;

	friend class GrimEngine;
};

class Light {		// Set lighting data
public:
	void load(TextSplitter &ts);
	void loadBinary(Common::SeekableReadStream *data);
	void saveState(SaveGame *savedState) const;
	bool restoreState(SaveGame *savedState);

	Common::String _name;
	Common::String _type;
	Math::Vector3d _pos, _dir;
	Color _color;
	float _intensity, _umbraangle, _penumbraangle;
	bool _enabled;
};

} // end of namespace Grim

#endif
