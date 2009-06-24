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

#include "engines/grim/scene.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/colormap.h"
#include "engines/grim/grim.h"
#include "engines/grim/savegame.h"

#include "engines/grim/imuse/imuse.h"

namespace Grim {

Scene::Scene(const char *name, const char *buf, int len) :
		_locked(false), _name(name), _enableLights(false) {
	TextSplitter ts(buf, len);
	char tempBuf[256];

	ts.expectString("section: colormaps");
	ts.scanString(" numcolormaps %d", 1, &_numCmaps);
	_cmaps = new ResPtr<CMap>[_numCmaps];
	char cmap_name[256];
	for (int i = 0; i < _numCmaps; i++) {
		ts.scanString(" colormap %256s", 1, cmap_name);
		_cmaps[i] = g_resourceloader->loadColormap(cmap_name);
	}

	if (g_grim->getGameFlags() & GF_DEMO) {
		ts.expectString("section: objectstates");
		ts.scanString(" tot_objects %d", 1, &_numObjectStates);
		char object_name[256];
		ts.scanString(" object %256s", 1, object_name);
	} else {
		_numObjectStates = 0;
	}

	ts.expectString("section: setups");
	ts.scanString(" numsetups %d", 1, &_numSetups);
	_setups = new Setup[_numSetups];
	for (int i = 0; i < _numSetups; i++)
		_setups[i].load(ts);
	_currSetup = _setups;

	_numSectors = -1;
	_numLights = -1;
	_lights = NULL;
	_sectors = NULL;

	_minVolume = 0;
	_maxVolume = 0;

	// Lights are optional
	if (ts.eof())
		return;

	ts.expectString("section: lights");
	ts.scanString(" numlights %d", 1, &_numLights);
	_lights = new Light[_numLights];
	for (int i = 0; i < _numLights; i++)
		_lights[i].load(ts);

	// Calculate the number of sectors
	ts.expectString("section: sectors");
	if (ts.eof())	// Sectors are optional, but section: doesn't seem to be
		return;

	int sectorStart = ts.getLineNumber();
	_numSectors = 0;
	// Find the number of sectors (while the sectors usually
	// count down from the highest number there are a few
	// cases where they count up, see hh.set for example)
	while (!ts.eof()) {
		ts.scanString(" %s", 1, tempBuf);
		if (!strcasecmp(tempBuf, "sector"))
			_numSectors++;
	}
	// Allocate and fill an array of sector info
	_sectors = new Sector[_numSectors];
	ts.setLineNumber(sectorStart);
	for (int i = 0; i < _numSectors; i++)
		_sectors[i].load(ts);
}

Scene::~Scene() {
	delete[] _cmaps;
	delete[] _setups;
	if (_lights)
		delete[] _lights;
	if (_sectors)
		delete[] _sectors;
	for (StateList::iterator i = _states.begin(); i != _states.end(); i++)
		delete (*i);
}

void Scene::saveState(SaveGame *savedState) {
	savedState->writeLESint32(_name.size());
	savedState->write(_name.c_str(), _name.size());
	savedState->writeLEUint32(_currSetup - _setups); // current setup id
	savedState->writeLEUint32(_locked);
	savedState->writeLEUint32(_enableLights);
	savedState->writeLEUint32(_minVolume);
	savedState->writeLEUint32(_maxVolume);

	savedState->writeLEUint32(_numObjectStates);
	for (StateList::iterator i = _states.begin(); i != _states.end(); i++) {
		ObjectState *s = *i;
	}
}

void Scene::Setup::load(TextSplitter &ts) {
	char buf[256];

	ts.scanString(" setup %256s", 1, buf);
	_name = buf;

	ts.scanString(" background %256s", 1, buf);
	_bkgndBm = g_resourceloader->loadBitmap(buf);
	if (!_bkgndBm) {
		if (gDebugLevel == DEBUG_BITMAPS || gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
			printf("Unable to load scene bitmap: %s\n", buf);
	} else {
		if (gDebugLevel == DEBUG_BITMAPS || gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL)
			printf("Loaded scene bitmap: %s\n", buf);
	}

	// ZBuffer is optional
	if (!ts.checkString("zbuffer")) {
		_bkgndZBm = NULL;
	} else {
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
	if (ts.checkString("object_art"))
		ts.scanString(" object_art %256s", 1, buf);
}

void Scene::Light::load(TextSplitter &ts) {
	char buf[256];

	// Light names can be null, but ts doesn't seem flexible enough to allow this
	if (strlen(ts.currentLine()) > strlen(" light"))
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
	_color.red() = r;
	_color.green() = g;
	_color.blue() = b;
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
	if (!_enableLights) {
		g_driver->disableLights();
		return;
	}

	for (int i = 0; i < _numLights; i++) {
		g_driver->setupLight(&_lights[i], i);
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
}

void Scene::drawBitmaps(ObjectState::Position stage) {
	for (StateList::iterator i = _states.begin(); i != _states.end(); i++) {
		if ((*i)->pos() == stage && _currSetup == _setups + (*i)->setupID())
			(*i)->draw();
	}
}

Sector *Scene::findPointSector(Graphics::Vector3d p, int flags) {
	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors + i;
		if ((sector->type() & flags) && sector->visible() && sector->isPointInSector(p))
			return sector;
	}
	return NULL;
}

void Scene::findClosestSector(Graphics::Vector3d p, Sector **sect, Graphics::Vector3d *closestPt) {
	Sector *resultSect = NULL;
	Graphics::Vector3d resultPt = p;
	float minDist = 0.0;

	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors + i;
		if ((sector->type() & 0x1000) == 0 || !sector->visible())
			continue;
		Graphics::Vector3d closestPt = sector->closestPoint(p);
		float thisDist = (closestPt - p).magnitude();
		if (!resultSect || thisDist < minDist) {
			resultSect = sector;
			resultPt = closestPt;
			minDist = thisDist;
		}
	}

	if (sect)
		*sect = resultSect;

	if (closestPt)
		*closestPt = resultPt;
}

ObjectState *Scene::findState(const char *filename) {
	// Check the different state objects for the bitmap
	for (StateList::iterator i = _states.begin(); i != _states.end(); i++) {
		const char *file = (*i)->bitmapFilename();

		if (strcmp(file, filename) == 0)
			return *i;
		if (strcasecmp(file, filename) == 0) {
			if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
				warning("State object request '%s' matches object '%s' but is the wrong case", filename, file);
			return *i;
		}
	}
	return NULL;
}

void Scene::setSoundPosition(const char *soundName, Graphics::Vector3d pos) {
	Graphics::Vector3d cameraPos = _currSetup->_pos;
	Graphics::Vector3d vector, vector2;
	vector.set(fabs(cameraPos.x() - pos.x()), fabs(cameraPos.y() - pos.y()), fabs(cameraPos.z() - pos.z()));
	float distance = vector.magnitude();
	float maxDistance = 8.0f;
	int diffVolume = _maxVolume - _minVolume;
	int newVolume = (int)(diffVolume * (1.0 - (distance / maxDistance)));
	newVolume += _minVolume;
	g_imuse->setVolume(soundName, newVolume);

	//TODO
	//g_imuse->setPan(soundName, pan);
}

void Scene::setSoundParameters(int minVolume, int maxVolume) {
	_minVolume = minVolume;
	_maxVolume = maxVolume;
}

void Scene::getSoundParameters(int *minVolume, int *maxVolume) {
	*minVolume = _minVolume;
	*maxVolume = _maxVolume;
}

void Scene::moveObjectStateToFirst(ObjectState *s) {
	_states.remove(s);
	_states.push_front(s);
}

void Scene::moveObjectStateToLast(ObjectState *s) {
	_states.remove(s);
	_states.push_back(s);
}

} // end of namespace Grim
