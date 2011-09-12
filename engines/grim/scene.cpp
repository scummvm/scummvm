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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "common/memstream.h"

#include "engines/grim/debug.h"
#include "engines/grim/scene.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/colormap.h"
#include "engines/grim/grim.h"
#include "engines/grim/savegame.h"
#include "engines/grim/resource.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/gfx_base.h"

#include "engines/grim/imuse/imuse.h"

namespace Grim {

Scene::Scene(const Common::String &sceneName, const char *buf, int len) :
		PoolObject<Scene, MKTAG('S', 'E', 'T', ' ')>(), _locked(false), _name(sceneName), _enableLights(false),
		_lightsConfigured(false) {

	if (len >= 7 && memcmp(buf, "section", 7) == 0) {
		TextSplitter ts(buf, len);
		loadText(ts);
	} else {
		Common::MemoryReadStream ms((const byte *)buf, len);
		loadBinary(&ms);
	}
}

Scene::Scene() :
	PoolObject<Scene, MKTAG('S', 'E', 'T', ' ')>(), _cmaps(NULL) {

}

Scene::~Scene() {
	if (_cmaps) {
		delete[] _cmaps;
		for (int i = 0; i < _numSetups; ++i) {
			delete _setups[i]._bkgndBm;
			delete _setups[i]._bkgndZBm;
		}
		delete[] _setups;
		delete[] _lights;
		for (int i = 0; i < _numSectors; ++i) {
			delete _sectors[i];
		}
		delete[] _sectors;
		for (StateList::iterator i = _states.begin(); i != _states.end(); ++i)
			delete (*i);
	}
}

void Scene::resetInternalData() {
	for (int i = 0; i < _numSetups; ++i) {
		_setups[i]._bkgndBm = NULL;
		_setups[i]._bkgndZBm = NULL;
	}
}

void Scene::loadText(TextSplitter &ts){
	char tempBuf[256];

	ts.expectString("section: colormaps");
	ts.scanString(" numcolormaps %d", 1, &_numCmaps);
	_cmaps = new ObjectPtr<CMap>[_numCmaps];
	char cmap_name[256];
	for (int i = 0; i < _numCmaps; i++) {
		ts.scanString(" colormap %256s", 1, cmap_name);
		_cmaps[i] = g_resourceloader->getColormap(cmap_name);
	}

	if (ts.checkString("section: objectstates") || ts.checkString("sections: object_states")) {
		ts.nextLine();
		ts.scanString(" tot_objects %d", 1, &_numObjectStates);
		char object_name[256];
		for (int l = 0; l < _numObjectStates; l++) {
			ts.scanString(" object %256s", 1, object_name);
		}
	} else {
		_numObjectStates = 0;
	}

	ts.expectString("section: setups");
	ts.scanString(" numsetups %d", 1, &_numSetups);
	_setups = new Setup[_numSetups];
	for (int i = 0; i < _numSetups; i++)
		_setups[i].load(ts);
	_currSetup = _setups;

	_lightsConfigured = false;
	_numSectors = -1;
	_numLights = -1;
	_lights = NULL;
	_sectors = NULL;

	_minVolume = 0;
	_maxVolume = 0;

	// Lights are optional
	if (ts.isEof())
		return;

	ts.expectString("section: lights");
	ts.scanString(" numlights %d", 1, &_numLights);
	_lights = new Light[_numLights];
	for (int i = 0; i < _numLights; i++)
		_lights[i].load(ts);

	// Calculate the number of sectors
	ts.expectString("section: sectors");
	if (ts.isEof())	// Sectors are optional, but section: doesn't seem to be
		return;

	int sectorStart = ts.getLineNumber();
	_numSectors = 0;
	// Find the number of sectors (while the sectors usually
	// count down from the highest number there are a few
	// cases where they count up, see hh.set for example)
	while (!ts.isEof()) {
		ts.scanString(" %s", 1, tempBuf);
		if (!scumm_stricmp(tempBuf, "sector"))
			_numSectors++;
	}
	// Allocate and fill an array of sector info
	_sectors = new Sector*[_numSectors];
	ts.setLineNumber(sectorStart);
	for (int i = 0; i < _numSectors; i++) {
		// Use the ids as index for the sector in the array.
		// This way when looping they are checked from the id 0 sto the last,
		// which seems important for sets with overlapping camera sectors, like ga.set.
		Sector *s = new Sector();
		s->load(ts);
		_sectors[s->getSectorId()] = s;
	}
}

void Scene::loadBinary(Common::MemoryReadStream *ms)
{
	// yes, an array of size 0
	_cmaps = NULL;//new CMapPtr[0];


	_numSetups = ms->readUint32LE();
	_setups = new Setup[_numSetups];
	for (int i = 0; i < _numSetups; i++)
		_setups[i].loadBinary(ms);
	_currSetup = _setups;

	_numSectors = 0;
	_numLights = 0;
	_lights = NULL;
	_sectors = NULL;

	_minVolume = 0;
	_maxVolume = 0;

	// the rest may or may not be optional. Might be a good idea to check if there is no more data.

	_numLights = ms->readUint32LE();
	_lights = new Light[_numLights];
	for (int i = 0; i < _numLights; i++)
		_lights[i].loadBinary(ms);

	// bypass light stuff for now
	_numLights = 0;

	_numSectors = ms->readUint32LE();
	// Allocate and fill an array of sector info
	_sectors = new Sector*[_numSectors];
	for (int i = 0; i < _numSectors; i++) {
		_sectors[i] = new Sector();
		_sectors[i]->loadBinary(ms);
	}
}


void Scene::saveState(SaveGame *savedState) const {
	savedState->writeString(_name);
	savedState->writeLESint32(_numCmaps);
	for (int i = 0; i < _numCmaps; ++i) {
		savedState->writeString(_cmaps[i]->getFilename());
	}
	savedState->writeLEUint32(_currSetup - _setups); // current setup id
	savedState->writeLEUint32(_locked);
	savedState->writeLEUint32(_enableLights);
	savedState->writeLEUint32(_minVolume);
	savedState->writeLEUint32(_maxVolume);

	savedState->writeLEUint32(_states.size());
	for (StateList::const_iterator i = _states.begin(); i != _states.end(); ++i) {
		savedState->writeLEUint32((*i)->getId());
	}

	//Setups
	savedState->writeLEUint32(_numSetups);
	for (int i = 0; i < _numSetups; ++i) {
		Setup &set = _setups[i];

		//name
		savedState->writeString(set._name);

		//bkgndBm
		if (set._bkgndBm) {
			savedState->writeLEUint32(set._bkgndBm->getId());
		} else {
			savedState->writeLEUint32(0);
		}

		//bkgndZBm
		if (set._bkgndZBm) {
			savedState->writeLEUint32(set._bkgndZBm->getId());
		} else {
			savedState->writeLEUint32(0);
		}

		savedState->writeVector3d(set._pos);
		savedState->writeVector3d(set._interest);
		savedState->writeFloat(set._roll);
		savedState->writeFloat(set._fov);
		savedState->writeFloat(set._nclip);
		savedState->writeFloat(set._fclip);
	}

	//Sectors
	savedState->writeLEUint32(_numSectors);
	for (int i = 0; i < _numSectors; ++i) {
		_sectors[i]->saveState(savedState);
	}

	//Lights
	savedState->writeLEUint32(_numLights);
	for (int i = 0; i < _numLights; ++i) {
		Light &l = _lights[i];

		//name
		savedState->writeString(l._name);
		savedState->writeLEBool(l._enabled);

		//type
		savedState->writeString(l._type);

		savedState->writeVector3d(l._pos);
		savedState->writeVector3d(l._dir);

		savedState->writeColor(l._color);

		savedState->writeFloat(l._intensity);
		savedState->writeFloat(l._umbraangle);
		savedState->writeFloat(l._penumbraangle);
    }
}

bool Scene::restoreState(SaveGame *savedState) {
	_name = savedState->readString();
	_numCmaps = savedState->readLESint32();
	_cmaps = new CMapPtr[_numCmaps];
	for (int i = 0; i < _numCmaps; ++i) {
		Common::String str = savedState->readString();
		_cmaps[i] = g_resourceloader->getColormap(str);
	}

	int32 currSetupId = savedState->readLEUint32();
	_locked           = savedState->readLEUint32();
	_enableLights     = savedState->readLEUint32();
	_minVolume        = savedState->readLEUint32();
	_maxVolume        = savedState->readLEUint32();

	_numObjectStates = savedState->readLEUint32();
	_states.clear();
	for (int i = 0; i < _numObjectStates; ++i) {
		int32 id = savedState->readLEUint32();
		ObjectState *o = ObjectState::getPool()->getObject(id);
		_states.push_back(o);
	}

	//Setups
	_numSetups = savedState->readLEUint32();
	_setups = new Setup[_numSetups];
	_currSetup = _setups + currSetupId;
	for (int i = 0; i < _numSetups; ++i) {
		Setup &set = _setups[i];

		set._name = savedState->readString();

		set._bkgndBm = Bitmap::getPool()->getObject(savedState->readLEUint32());
		set._bkgndZBm = Bitmap::getPool()->getObject(savedState->readLEUint32());

		set._pos      = savedState->readVector3d();
		set._interest = savedState->readVector3d();
		set._roll     = savedState->readFloat();
		set._fov      = savedState->readFloat();
		set._nclip    = savedState->readFloat();
		set._fclip    = savedState->readFloat();
	}

    //Sectors
	_numSectors = savedState->readLEUint32();
	if (_numSectors > 0) {
		_sectors = new Sector*[_numSectors];
		for (int i = 0; i < _numSectors; ++i) {
			_sectors[i] = new Sector();
			_sectors[i]->restoreState(savedState);
		}
	} else {
		_sectors = NULL;
	}

	_numLights = savedState->readLEUint32();
	_lights = new Light[_numLights];
	for (int i = 0; i < _numLights; ++i) {
		Light &l = _lights[i];

		l._name = savedState->readString();
		l._enabled = savedState->readLEBool();
		l._type = savedState->readString();

		l._pos           = savedState->readVector3d();
		l._dir           = savedState->readVector3d();

		l._color         = savedState->readColor();

		l._intensity     = savedState->readFloat();
		l._umbraangle    = savedState->readFloat();
		l._penumbraangle = savedState->readFloat();
	}

	_lightsConfigured = false;

	return true;
}

void Scene::Setup::load(TextSplitter &ts) {
	char buf[256];

	ts.scanString(" setup %256s", 1, buf);
	_name = buf;

	ts.scanString(" background %256s", 1, buf);
	_bkgndBm = g_resourceloader->loadBitmap(buf);
	if (!_bkgndBm) {
		if (gDebugLevel == DEBUG_BITMAPS || gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
			warning("Unable to load scene bitmap: %s\n", buf);
	} else {
		if (gDebugLevel == DEBUG_BITMAPS || gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL)
			warning("Loaded scene bitmap: %s\n", buf);
	}

	// ZBuffer is optional
	_bkgndZBm = NULL;
	if (ts.checkString("zbuffer")) {
		ts.scanString(" zbuffer %256s", 1, buf);
		// Don't even try to load if it's the "none" bitmap
		if (strcmp(buf, "<none>.lbm") != 0) {
			_bkgndZBm = g_resourceloader->loadBitmap(buf);
			if (gDebugLevel == DEBUG_BITMAPS || gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL)
				printf("Loading scene z-buffer bitmap: %s\n", buf);
		}
	}

	ts.scanString(" position %f %f %f", 3, &_pos.x(), &_pos.y(), &_pos.z());
	ts.scanString(" interest %f %f %f", 3, &_interest.x(), &_interest.y(), &_interest.z());
	ts.scanString(" roll %f", 1, &_roll);
	ts.scanString(" fov %f", 1, &_fov);
	ts.scanString(" nclip %f", 1, &_nclip);
	ts.scanString(" fclip %f", 1, &_fclip);
	for (;;) {
		if (ts.checkString("object_art"))
			ts.scanString(" object_art %256s", 1, buf);
		else
			break;
		if (ts.checkString("object_z"))
			ts.scanString(" object_z %256s", 1, buf);
	}
}

void Scene::Setup::loadBinary(Common::MemoryReadStream *ms) {
	char name[128];
	ms->read(name, 128);
	_name = Common::String(name);

	// Skip an unknown number (this is the stringlength of the following string)
	int fNameLen = 0;
	fNameLen = ms->readUint32LE();

	char* fileName = new char[fNameLen];
	ms->read(fileName,fNameLen);

	_bkgndZBm = NULL;
	_bkgndBm = g_resourceloader->loadBitmap(fileName);


	ms->read(_pos._coords, 12);

	ms->read(_interest._coords, 12);

	ms->read(&_roll, 4);
	ms->read(&_fov, 4);
	ms->read(&_nclip, 4);
	ms->read(&_fclip, 4);

}

void Light::load(TextSplitter &ts) {
	char buf[256];

	// Light names can be null, but ts doesn't seem flexible enough to allow this
	if (strlen(ts.getCurrentLine()) > strlen(" light"))
		ts.scanString(" light %256s", 1, buf);
	else {
		ts.nextLine();
		strcpy(buf, "");
	}
	_name = buf;

	ts.scanString(" type %256s", 1, buf);
	_type = buf;

	ts.scanString(" position %f %f %f", 3, &_pos.x(), &_pos.y(), &_pos.z());
	ts.scanString(" direction %f %f %f", 3, &_dir.x(), &_dir.y(), &_dir.z());
	ts.scanString(" intensity %f", 1, &_intensity);
	ts.scanString(" umbraangle %f", 1, &_umbraangle);
	ts.scanString(" penumbraangle %f", 1, &_penumbraangle);

	int r, g, b;
	ts.scanString(" color %d %d %d", 3, &r, &g, &b);
	_color.getRed() = r;
	_color.getGreen() = g;
	_color.getBlue() = b;

	_enabled = true;
}

void Light::loadBinary(Common::MemoryReadStream *ms) {
	// skip lights for now
	ms->seek(100, SEEK_CUR);
}

void Scene::Setup::setupCamera() const {
	// Ignore nclip_ and fclip_ for now.  This fixes:
	// (a) Nothing was being displayed in the Land of the Living
	// diner because lr.set set nclip to 0.
	// (b) The zbuffers for setups with different nclip or
	// fclip values.  If it turns out that the clipping planes
	// are important at some point, we'll need to modify the
	// zbuffer transformation in bitmap.cpp to take nclip_ and
	// fclip_ into account.
	g_driver->setupCamera(_fov, 0.01f, 3276.8f, _roll);
	g_driver->positionCamera(_pos, _interest);
}

void Scene::setupLights() {
	if (_lightsConfigured)
		return;
	_lightsConfigured = true;
	if (!_enableLights) {
		g_driver->disableLights();
		return;
	}

	int count = 0;
	for (int i = 0; i < _numLights; i++) {
		Light *l = &_lights[i];
		if (l->_enabled) {
			g_driver->setupLight(l, count);
			++count;
		}
	}
}

void Scene::setSetup(int num) {
	// Looks like num is zero-based so >= should work to find values
	// that are out of the range of valid setups
	if (num >= _numSetups || num < 0) {
		error("Failed to change scene setup, value out of range");
		return;
	}
	_currSetup = _setups + num;
	g_grim->flagRefreshShadowMask(true);
	_lightsConfigured = false;
}

void Scene::drawBackground() const {
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

void Scene::drawBitmaps(ObjectState::Position stage) {
	for (StateList::iterator i = _states.reverse_begin(); i != _states.end(); --i) {
		if ((*i)->getPos() == stage && _currSetup == _setups + (*i)->getSetupID())
			(*i)->draw();
	}
}

Sector *Scene::findPointSector(const Graphics::Vector3d &p, Sector::SectorType type) {
	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors[i];
		if (sector && (sector->getType() & type) && sector->isVisible() && sector->isPointInSector(p))
			return sector;
	}
	return NULL;
}

void Scene::findClosestSector(const Graphics::Vector3d &p, Sector **sect, Graphics::Vector3d *closestPoint) {
	Sector *resultSect = NULL;
	Graphics::Vector3d resultPt = p;
	float minDist = 0.0;

	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors[i];
		if ((sector->getType() & Sector::WalkType) == 0 || !sector->isVisible())
			continue;
		Graphics::Vector3d closestPt = sector->getClosestPoint(p);
		float thisDist = (closestPt - p).magnitude();
		if (!resultSect || thisDist < minDist) {
			resultSect = sector;
			resultPt = closestPt;
			minDist = thisDist;
		}
	}

	if (sect)
		*sect = resultSect;

	if (closestPoint)
		*closestPoint = resultPt;
}

void Scene::shrinkBoxes(float radius) {
	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors[i];
		sector->shrink(radius);
	}
}

void Scene::unshrinkBoxes() {
	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors[i];
		sector->unshrink();
	}
}

ObjectState *Scene::findState(const char *filename) {
	// Check the different state objects for the bitmap
	for (StateList::iterator i = _states.begin(); i != _states.end(); ++i) {
		const Common::String &file = (*i)->getBitmapFilename();

		if (file == filename)
			return *i;
		if (file.compareToIgnoreCase(filename) == 0) {
			if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
				warning("State object request '%s' matches object '%s' but is the wrong case", filename, file.c_str());
			return *i;
		}
	}
	return NULL;
}

void Scene::setLightsDirty() {
	_lightsConfigured = false;
}

void Scene::setLightIntensity(const char *light, float intensity) {
	for (int i = 0; i < _numLights; ++i) {
		Light &l = _lights[i];
		if (l._name == light) {
			l._intensity = intensity;
			_lightsConfigured = false;
			return;
		}
	}
}

void Scene::setLightIntensity(int light, float intensity) {
	Light &l = _lights[light];
	l._intensity = intensity;
	_lightsConfigured = false;
}

void Scene::setLightEnabled(const char *light, bool enabled) {
	for (int i = 0; i < _numLights; ++i) {
		Light &l = _lights[i];
		if (l._name == light) {
			l._enabled = enabled;
			_lightsConfigured = false;
			return;
		}
	}
}

void Scene::setLightEnabled(int light, bool enabled) {
	Light &l = _lights[light];
	l._enabled = enabled;
	_lightsConfigured = false;
}

void Scene::setLightPosition(const char *light, const Graphics::Vector3d &pos) {
	for (int i = 0; i < _numLights; ++i) {
		Light &l = _lights[i];
		if (l._name == light) {
			l._pos = pos;
			_lightsConfigured = false;
			return;
		}
	}
}

void Scene::setLightPosition(int light, const Graphics::Vector3d &pos) {
	Light &l = _lights[light];
	l._pos = pos;
	_lightsConfigured = false;
}

void Scene::setSoundPosition(const char *soundName, Graphics::Vector3d pos) {
	setSoundPosition(soundName, pos, _minVolume, _maxVolume);
}

void Scene::setSoundPosition(const char *soundName, Graphics::Vector3d pos, int minVol, int maxVol) {
	// TODO: The volume and pan needs to be updated when the setup changes.
	Graphics::Vector3d cameraPos = _currSetup->_pos;
	Graphics::Vector3d vector = pos - cameraPos;
	float distance = vector.magnitude();
	float diffVolume = maxVol - minVol;
	//This 8.f is a guess, so it may need some adjusting
	int newVolume = (int)(8.f * diffVolume / distance);
	newVolume += minVol;
	if (newVolume > _maxVolume)
		newVolume = _maxVolume;
	g_imuse->setVolume(soundName, newVolume);

	Graphics::Vector3d cameraVector =_currSetup->_interest - _currSetup->_pos;
	Graphics::Vector3d up(0,0,1);
	Graphics::Vector3d right;
	cameraVector.normalize();
	float roll = -_currSetup->_roll * LOCAL_PI / 180.f;
	float cosr = cos(roll);
	// Rotate the up vector by roll.
	up = up * cosr + Graphics::cross(cameraVector, up) * sin(roll) +
		cameraVector * Graphics::dot(cameraVector, up) * (1 - cosr);
	right = Graphics::cross(cameraVector, up);
	right.normalize();
	float angle = atan2(Graphics::dot(vector, right), Graphics::dot(vector, cameraVector));
	float pan = sin(angle);
	g_imuse->setPan(soundName, (int)((pan + 1.f) / 2.f * 127.f + 0.5f));
}

Sector *Scene::getSectorBase(int id) {
	if ((_numSectors >= 0) && (id < _numSectors))
		return _sectors[id];
	else
		return NULL;
}

void Scene::setSoundParameters(int minVolume, int maxVolume) {
	_minVolume = minVolume;
	_maxVolume = maxVolume;
}

void Scene::getSoundParameters(int *minVolume, int *maxVolume) {
	*minVolume = _minVolume;
	*maxVolume = _maxVolume;
}

void Scene::addObjectState(ObjectState *s) {
	_states.push_front(s);
}

void Scene::moveObjectStateToFront(ObjectState *s) {
	_states.remove(s);
	_states.push_front(s);
	// Make the state invisible. This hides the deadbolt when brennis closes the switcher door
	// in the server room (tu), and therefore fixes https://github.com/residual/residual/issues/24
	s->setNumber(0);
}

void Scene::moveObjectStateToBack(ObjectState *s) {
	_states.remove(s);
	_states.push_back(s);
}

} // end of namespace Grim
