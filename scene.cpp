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

#include "stdafx.h"
#include "scene.h"
#include "textsplit.h"
#include "resource.h"
#include "debug.h"
#include "bitmap.h"
#include "colormap.h"
#include "vector3d.h"
#include "driver.h"

#include "imuse/imuse.h"

#include <SDL.h>
#include "tinygl/gl.h"
#include <cmath>

Scene::Scene(const char *name, const char *buf, int len) :
		_name(name) {
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
	if (ts.eof()) 	// Sectors are optional, but section: doesn't seem to be
		return;

	// Sector NAMES can be null, but ts doesn't seem flexible enough to allow this
	if (strlen(ts.currentLine()) > strlen(" sector"))
		ts.scanString(" sector %256s", 1, tempBuf);
	else {
		ts.nextLine();
		strcpy(tempBuf, "");
	}

	ts.scanString(" id %d", 1, &_numSectors);
	_numSectors++;
	_sectors = new Sector[_numSectors];
	// FIXME: This would be nicer if we could rewind the textsplitter
	// stream
	_sectors[0].load0(ts, tempBuf, _numSectors);
	for (int i = 1; i < _numSectors; i++)
		_sectors[i].load(ts);
}

Scene::~Scene() {
	delete [] _cmaps;
	delete [] _setups;
	if (_lights)
		delete [] _lights;
	if (_sectors)
		delete [] _sectors;
	for (StateList::iterator i = _states.begin(); i != _states.end(); i++)
		delete (*i);
}

void Scene::Setup::load(TextSplitter &ts) {
	char buf[256];

	ts.scanString(" setup %256s", 1, buf);
	_name = buf;

	ts.scanString(" background %256s", 1, buf);
	_bkgndBm = g_resourceloader->loadBitmap(buf);

	// ZBuffer is optional
	if (!ts.checkString("zbuffer")) {
		_bkgndZBm = NULL;
	} else {
		ts.scanString(" zbuffer %256s", 1, buf);
		_bkgndZBm = g_resourceloader->loadBitmap(buf);
	}

	ts.scanString(" position %f %f %f", 3, &_pos.x(), &_pos.y(), &_pos.z());
	ts.scanString(" interest %f %f %f", 3, &_interest.x(), &_interest.y(), &_interest.z());
	ts.scanString(" roll %f", 1, &_roll);
	ts.scanString(" fov %f", 1, &_fov);
	ts.scanString(" nclip %f", 1, &_nclip);
	ts.scanString(" fclip %f", 1, &_fclip);
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
	ts.scanString(" color %d %d %d", 3, &_color.red(), &_color.green(), &_color.blue());
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
/*	glMatrixMode(GL_MODELVIEW);
	if (!_enableLights) {
		glDisable(GL_LIGHTING);
		return;
	}

	glEnable(GL_LIGHTING);

	for (int i = 0; i < _numLights; i++) {
		assert(i < GL_MAX_LIGHTS);
		Light *light = &_lights[i];
		GLfloat ambientLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat diffuseLight[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		GLfloat specularLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat lightPos[4];
		GLfloat lightDir[4];
		lightPos[0] = light->_pos._coords[0];
		lightPos[1] = light->_pos._coords[1];
		lightPos[2] = light->_pos._coords[2];
		lightPos[3] = 1.0f;
		diffuseLight[0] = light->_color.red / 256.0f;
		diffuseLight[1] = light->_color.blue / 256.0f;
		diffuseLight[2] = light->_color.green / 256.0f;
		diffuseLight[3] = 1.0f;

		if (strcmp(light->_type.c_str(), "omni") == 0) {
//			glLightfv(GL_LIGHT0 + i, GL_AMBIENT, ambientLight);
			glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, diffuseLight);
			glLightfv(GL_LIGHT0 + i, GL_SPECULAR, specularLight);
			glLightfv(GL_LIGHT0 + i, GL_POSITION, lightPos);
			glEnable(GL_LIGHT0 + i);
		} else if (strcmp(light->_type.c_str(), "direct") == 0) {
			lightDir[0] = light->_dir._coords[0];
			lightDir[1] = light->_dir._coords[1];
			lightDir[2] = light->_dir._coords[2];
			lightDir[3] = 1.0f;
//			glLightfv(GL_LIGHT0 + i, GL_AMBIENT, ambientLight);
			glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, diffuseLight);
			glLightfv(GL_LIGHT0 + i, GL_SPECULAR, specularLight);
			glLightfv(GL_LIGHT0 + i, GL_POSITION, lightPos);
			glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, lightDir);
			glEnable(GL_LIGHT0 + i);
		} else {
			error("Scene::setupLights() Unknown type of light: %s", light->_type);
		}
	}
	glEnable(GL_COLOR_MATERIAL);

	GLfloat materialLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, materialLight);
*/
}

void Scene::setSetup(int num) {
	_currSetup = _setups + num;
}

void Scene::drawBitmaps(ObjectState::Position stage) {
	for (StateList::iterator i = _states.begin(); i != _states.end(); i++) {
		if ((*i)->pos() == stage && _currSetup == _setups + (*i)->setupID())
			(*i)->draw();
	}
}

Sector *Scene::findPointSector(Vector3d p, int flags) {
	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors + i;
		if ((sector->type() & flags) && sector->visible() &&
		    sector->isPointInSector(p))
			return sector;
	}
	return NULL;
}

void Scene::findClosestSector(Vector3d p, Sector **sect, Vector3d *closestPt) {
	Sector *resultSect = NULL;
	Vector3d resultPt = p;
	float minDist;

	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors + i;
		if ((sector->type() & 0x1000) == 0 || !sector->visible())
			continue;
		Vector3d closestPt = sector->closestPoint(p);
		float thisDist = (closestPt - p).magnitude();
		if (resultSect == NULL || thisDist < minDist) {
			resultSect = sector;
			resultPt = closestPt;
			minDist = thisDist;
		}
	}

	if (sect != NULL)
		*sect = resultSect;

	if (closestPt != NULL)
		*closestPt = resultPt;
}

ObjectState *Scene::findState(const char *filename) {
	for (StateList::iterator i = _states.begin(); i != _states.end(); i++) {
		if (strcmp((*i)->bitmapFilename(), filename) == 0)
			return *i;
	}
	return NULL;
}

void Scene::setSoundPosition(const char *soundName, Vector3d pos) {
	Vector3d cameraPos = _currSetup->_pos;
	Vector3d vector, vector2;
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
