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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRIM_SET_H
#define GRIM_SET_H

#include "engines/grim/pool.h"
#include "engines/grim/object.h"
#include "engines/grim/color.h"
#include "engines/grim/sector.h"
#include "engines/grim/objectstate.h"
#include "math/quat.h"
#include "math/frustum.h"

namespace Common {
	class SeekableReadStream;
}
namespace Grim {

class SaveGame;
class CMap;
struct Light;
struct SetShadow;

class Set : public PoolObject<Set> {
public:
	Set(const Common::String &name, Common::SeekableReadStream *data);
	Set();
	~Set();

	static int32 getStaticTag() { return MKTAG('S', 'E', 'T', ' '); }

	void loadText(TextSplitter &ts);
	void loadBinary(Common::SeekableReadStream *data);
	void setupOverworldLights();

	void saveState(SaveGame *savedState) const;
	bool restoreState(SaveGame *savedState);

	int _minVolume;
	int _maxVolume;

	static Bitmap::Ptr loadBackground(const char *fileName);
	void drawBackground() const;
	void drawBitmaps(ObjectState::Position stage);
	void setupCamera();

	void setupLights(const Math::Vector3d &pos, bool inOverworld);

	void setSoundPosition(const char *soundName, const Math::Vector3d &pos);
	void setSoundPosition(const char *soundName, const Math::Vector3d &pos, int minVol, int maxVol);
	void calculateSoundPosition(const Math::Vector3d &pos, int minVol, int maxVol, int &volume, int &balance);
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
	inline int getNumSetups() const { return _numSetups; }

	// Sector access functions
	int getSectorCount() { return _numSectors; }

	Sector *getSectorBase(int id);
	Sector *getSectorByName(const Common::String &name);
	Sector *getSectorBySubstring(const Common::String &str);
	Sector *getSectorBySubstring(const Common::String &str, const Math::Vector3d &pos);

	Sector *findPointSector(const Math::Vector3d &p, Sector::SectorType type);
	int findSectorSortOrder(const Math::Vector3d &p, Sector::SectorType type);
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

	// Setups contain the camera information and background for all views in a Set
	struct Setup {      // Camera setup data
		void load(Set *set, int id, TextSplitter &ts);
		void loadBinary(Common::SeekableReadStream *data);
		void setupCamera() const;
		void saveState(SaveGame *savedState) const;
		bool restoreState(SaveGame *savedState);

		void getRotation(float *x, float *y, float *z);
		Math::Matrix4 getRotation() { return _rot; }
		void setPitch(Math::Angle p);
		void setYaw(Math::Angle y);
		void setRoll(Math::Angle r);

		Common::String _name;
		Bitmap::Ptr _bkgndBm, _bkgndZBm;

		// Camera settings
		Math::Vector3d _pos, _interest;
		Math::Matrix4 _rot;
		float _roll, _fov, _nclip, _fclip;
	};

	CMap *getCMap() {
		if (!_cmaps || ! _numCmaps)
			return NULL;
		return _cmaps[0];
	};

	Setup *getCurrSetup() { return _currSetup; }
	const Common::List<Light *> &getLights(bool inOverworld) { return (inOverworld ? _overworldLightsList : _lightsList); }
	const Math::Frustum &getFrustum() { return _frustum; }

	int getShadowCount() const { return _numShadows; }
	SetShadow *getShadow(int i);
	SetShadow *getShadowByName(const Common::String &name);

private:
	bool _locked;
	Common::String _name;
	int _numCmaps;
	ObjectPtr<CMap> *_cmaps;
	int _numSetups, _numLights, _numSectors, _numObjectStates, _numShadows;
	bool _enableLights;
	Sector **_sectors;
	Light *_lights;
	Common::List<Light *> _lightsList;
	Common::List<Light *> _overworldLightsList;
	Setup *_setups;
	SetShadow *_shadows;

	Setup *_currSetup;
	typedef Common::List<ObjectState::Ptr> StateList;
	StateList _states;

	Math::Frustum _frustum;

	friend class GrimEngine;
};

/**
* \struct Light
* Set lighting data
*/
struct Light {
	Light();
	void load(TextSplitter &ts);
	void loadBinary(Common::SeekableReadStream *data);
	void saveState(SaveGame *savedState) const;
	bool restoreState(SaveGame *savedState);
	void setIntensity(float intensity);
	void setUmbra(float angle);
	void setPenumbra(float angle);

	enum LightType {
		Omni = 1,
		Spot = 2,
		Direct = 3,
		Ambient = 4
	};

	Common::String _name;
	LightType _type;
	Math::Vector3d _pos, _dir;
	Color _color;
	float _intensity, _umbraangle, _penumbraangle, _falloffNear, _falloffFar;
	float _scaledintensity, _cosumbraangle, _cospenumbraangle;
	bool _enabled;
	// there may be more lights with the same position, so this is used to make the sort stable
	int _id;
};

/**
* \struct SetShadow
* Set shadow data (EMI)
*/
struct SetShadow {
	SetShadow();
	void loadBinary(Common::SeekableReadStream *data, Set *set);
	void saveState(SaveGame *savedState) const;
	void restoreState(SaveGame *savedState);

	Common::String _name;
	Math::Vector3d _shadowPoint;
	int _numSectors;
	Common::List<Common::String> _sectorNames;
	Color _color;
};

} // end of namespace Grim

#endif
