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

#include "common/foreach.h"

#include "engines/grim/debug.h"
#include "engines/grim/set.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/colormap.h"
#include "engines/grim/grim.h"
#include "engines/grim/savegame.h"
#include "engines/grim/resource.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/gfx_base.h"

#include "engines/grim/sound.h"
#include "engines/grim/emi/sound/emisound.h"

#include "math/frustum.h"

namespace Grim {

Set::Set(const Common::String &sceneName, Common::SeekableReadStream *data) :
		_locked(false), _name(sceneName), _enableLights(false) {

	char header[7];
	data->read(header, 7);
	data->seek(0, SEEK_SET);
	if (memcmp(header, "section", 7) == 0) {
		TextSplitter ts(_name, data);
		loadText(ts);
	} else {
		loadBinary(data);
	}
	setupOverworldLights();
}

Set::Set() :
		_cmaps(nullptr), _locked(false), _enableLights(false), _numSetups(0),
		_numLights(0), _numSectors(0), _numObjectStates(0), _minVolume(0),
		_maxVolume(0), _numCmaps(0), _numShadows(0), _currSetup(nullptr),
		_setups(nullptr), _lights(nullptr), _sectors(nullptr), _shadows(nullptr) {

	setupOverworldLights();
}

Set::~Set() {
	if (_cmaps || g_grim->getGameType() == GType_MONKEY4) {
		delete[] _cmaps;
		for (int i = 0; i < _numSetups; ++i) {
			delete _setups[i]._bkgndBm;
			delete _setups[i]._bkgndZBm;
		}
		delete[] _setups;
		turnOffLights();
		delete[] _lights;
		for (int i = 0; i < _numSectors; ++i) {
			delete _sectors[i];
		}
		delete[] _sectors;
		while (!_states.empty()) {
			ObjectState *s = _states.front();
			_states.pop_front();
			delete s;
		}
		delete[] _shadows;
	}
	foreach (Light *l, _overworldLightsList) {
		delete l;
	}
}

void Set::setupOverworldLights() {
	Light *l;

	l = new Light();
	l->_name = "Overworld Light 1";
	l->_enabled = true;
	l->_type = Light::Ambient;
	l->_pos = Math::Vector3d(0, 0, 0);
	l->_dir = Math::Vector3d(0, 0, 0);
	l->_color = Color(255, 255, 255);
	l->setIntensity(0.5f);
	_overworldLightsList.push_back(l);

	l = new Light();
	l->_name = "Overworld Light 2";
	l->_enabled = true;
	l->_type = Light::Direct;
	l->_pos = Math::Vector3d(0, 0, 0);
	l->_dir = Math::Vector3d(0, 0, -1);
	l->_color = Color(255, 255, 255);
	l->setIntensity(0.6f);
	_overworldLightsList.push_back(l);
}

void Set::loadText(TextSplitter &ts) {
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
		_setups[i].load(this, i, ts);
	_currSetup = _setups;

	_numShadows = 0;
	_numSectors = -1;
	_numLights = -1;
	_lights = nullptr;
	_sectors = nullptr;
	_shadows = nullptr;

	_minVolume = 0;
	_maxVolume = 0;

	// Lights are optional
	if (ts.isEof())
		return;

	ts.expectString("section: lights");
	ts.scanString(" numlights %d", 1, &_numLights);
	_lights = new Light[_numLights];
	for (int i = 0; i < _numLights; i++) {
		_lights[i].load(ts);
		_lights[i]._id = i;
		_lightsList.push_back(&_lights[i]);
	}

	// Calculate the number of sectors
	ts.expectString("section: sectors");
	if (ts.isEof()) // Sectors are optional, but section: doesn't seem to be
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

void Set::loadBinary(Common::SeekableReadStream *data) {
	// yes, an array of size 0
	_cmaps = nullptr;//new CMapPtr[0];
	_numCmaps = 0;
	_numObjectStates = 0;


	_numSetups = data->readUint32LE();
	_setups = new Setup[_numSetups];
	for (int i = 0; i < _numSetups; i++)
		_setups[i].loadBinary(data);
	_currSetup = _setups;

	_numSectors = 0;
	_numLights = 0;
	_lights = nullptr;
	_sectors = nullptr;
	_shadows = nullptr;

	_minVolume = 0;
	_maxVolume = 0;

	// the rest may or may not be optional. Might be a good idea to check if there is no more data.

	_numLights = data->readUint32LE();
	_lights = new Light[_numLights];
	for (int i = 0; i < _numLights; i++) {
		_lights[i].loadBinary(data);
		_lights[i]._id = i;
		_lightsList.push_back(&_lights[i]);
	}

	_numSectors = data->readUint32LE();
	// Allocate and fill an array of sector info
	_sectors = new Sector*[_numSectors];
	for (int i = 0; i < _numSectors; i++) {
		_sectors[i] = new Sector();
		_sectors[i]->loadBinary(data);
	}

	_numShadows = data->readUint32LE();
	_shadows = new SetShadow[_numShadows];

	for (int i = 0; i < _numShadows; ++i) {
		_shadows[i].loadBinary(data, this);
	}

	// Enable lights by default
	_enableLights = true;
}

void Set::saveState(SaveGame *savedState) const {
	savedState->writeString(_name);
	if (g_grim->getGameType() == GType_GRIM) {
		savedState->writeLESint32(_numCmaps);
		for (int i = 0; i < _numCmaps; ++i) {
			savedState->writeString(_cmaps[i]->getFilename());
		}
	}
	savedState->writeLEUint32((uint32)(_currSetup - _setups)); // current setup id
	savedState->writeBool(_locked);
	savedState->writeBool(_enableLights);
	savedState->writeLESint32(_minVolume);
	savedState->writeLESint32(_maxVolume);

	savedState->writeLEUint32(_states.size());
	for (StateList::const_iterator i = _states.begin(); i != _states.end(); ++i) {
		savedState->writeLESint32((*i)->getId());
	}

	//Setups
	savedState->writeLESint32(_numSetups);
	for (int i = 0; i < _numSetups; ++i) {
		_setups[i].saveState(savedState);
	}

	//Sectors
	savedState->writeLESint32(_numSectors);
	for (int i = 0; i < _numSectors; ++i) {
		_sectors[i]->saveState(savedState);
	}

	//Lights
	savedState->writeLESint32(_numLights);
	for (int i = 0; i < _numLights; ++i) {
		_lights[i].saveState(savedState);
	}

	//Shadows
	savedState->writeLESint32(_numShadows);
	for (int i = 0; i < _numShadows; ++i) {
		_shadows[i].saveState(savedState);
	}
}

bool Set::restoreState(SaveGame *savedState) {
	_name = savedState->readString();
	if (g_grim->getGameType() == GType_GRIM) {
		_numCmaps = savedState->readLESint32();
		_cmaps = new CMapPtr[_numCmaps];
		for (int i = 0; i < _numCmaps; ++i) {
			Common::String str = savedState->readString();
			_cmaps[i] = g_resourceloader->getColormap(str);
		}
	}

	int32 currSetupId = savedState->readLEUint32();
	_locked           = savedState->readBool();
	_enableLights     = savedState->readBool();
	_minVolume        = savedState->readLESint32();
	_maxVolume        = savedState->readLESint32();

	_numObjectStates = savedState->readLESint32();
	_states.clear();
	for (int i = 0; i < _numObjectStates; ++i) {
		int32 id = savedState->readLESint32();
		ObjectState *o = ObjectState::getPool().getObject(id);
		_states.push_back(o);
	}

	//Setups
	_numSetups = savedState->readLESint32();
	_setups = new Setup[_numSetups];
	_currSetup = _setups + currSetupId;
	for (int i = 0; i < _numSetups; ++i) {
		_setups[i].restoreState(savedState);
	}

	//Sectors
	_numSectors = savedState->readLESint32();
	if (_numSectors > 0) {
		_sectors = new Sector*[_numSectors];
		for (int i = 0; i < _numSectors; ++i) {
			_sectors[i] = new Sector();
			_sectors[i]->restoreState(savedState);
		}
	} else {
		_sectors = nullptr;
	}

	_numLights = savedState->readLESint32();
	_lights = new Light[_numLights];
	for (int i = 0; i < _numLights; i++) {
		_lights[i].restoreState(savedState);
		_lights[i]._id = i;
		_lightsList.push_back(&_lights[i]);
	}

	if (savedState->saveMinorVersion() >= 19) {
		_numShadows = savedState->readLESint32();
		_shadows = new SetShadow[_numShadows];
		for (int i = 0; i < _numShadows; ++i) {
			_shadows[i].restoreState(savedState);
		}
	}

	return true;
}

void Set::Setup::load(Set *set, int id, TextSplitter &ts) {
	char buf[256];

	ts.scanString(" setup %256s", 1, buf);
	_name = buf;

	ts.scanString(" background %256s", 1, buf);
	_bkgndBm = loadBackground(buf);

	// ZBuffer is optional
	_bkgndZBm = nullptr;
	if (ts.checkString("zbuffer")) {
		ts.scanString(" zbuffer %256s", 1, buf);
		// Don't even try to load if it's the "none" bitmap
		if (strcmp(buf, "<none>.lbm") != 0) {
			_bkgndZBm = Bitmap::create(buf);
			Debug::debug(Debug::Bitmaps | Debug::Sets,
						 "Loading scene z-buffer bitmap: %s\n", buf);
		}
	}

	ts.scanString(" position %f %f %f", 3, &_pos.x(), &_pos.y(), &_pos.z());
	ts.scanString(" interest %f %f %f", 3, &_interest.x(), &_interest.y(), &_interest.z());
	ts.scanString(" roll %f", 1, &_roll);
	ts.scanString(" fov %f", 1, &_fov);
	ts.scanString(" nclip %f", 1, &_nclip);
	ts.scanString(" fclip %f", 1, &_fclip);
	for (;;) {
		char name[256], zname[256];
		char bitmap[256], zbitmap[256];
		zbitmap[0] = '\0';
		if (ts.checkString("object_art"))
			ts.scanString(" object_art %256s %256s", 2, name, bitmap);
		else
			break;
		if (ts.checkString("object_z"))
			ts.scanString(" object_z %256s %256s", 2, zname, zbitmap);

		if (zbitmap[0] == '\0' || strcmp(name, zname) == 0) {
			set->addObjectState(id, ObjectState::OBJSTATE_BACKGROUND, bitmap, zbitmap, true);
		}
	}
}

void Set::Setup::loadBinary(Common::SeekableReadStream *data) {
	char name[128];
	data->read(name, 128);
	_name = Common::String(name);

	// Skip an unknown number (this is the stringlength of the following string)
	int fNameLen = 0;
	fNameLen = data->readUint32LE();

	char *fileName = new char[fNameLen];
	data->read(fileName, fNameLen);

	_bkgndZBm = nullptr;
	_bkgndBm = loadBackground(fileName);

	_pos.readFromStream(data);

	Math::Quaternion q;
	q.readFromStream(data);
	q.toMatrix(_rot);

	_fov   = data->readFloatLE();
	_nclip = data->readFloatLE();
	_fclip = data->readFloatLE();

	delete[] fileName;
}

void Set::Setup::saveState(SaveGame *savedState) const {
	//name
	savedState->writeString(_name);

	//bkgndBm
	if (_bkgndBm) {
		savedState->writeLESint32(_bkgndBm->getId());
	} else {
		savedState->writeLESint32(0);
	}

	//bkgndZBm
	if (_bkgndZBm) {
		savedState->writeLESint32(_bkgndZBm->getId());
	} else {
		savedState->writeLESint32(0);
	}

	savedState->writeVector3d(_pos);
	if (g_grim->getGameType() == GType_MONKEY4) {
		// Get the rotation matrix as a quaternion and write it out
		Math::Quaternion q(_rot);
		savedState->writeFloat(q.x());
		savedState->writeFloat(q.y());
		savedState->writeFloat(q.z());
		savedState->writeFloat(q.w());
	} else {
		savedState->writeVector3d(_interest);
		savedState->writeFloat(_roll);
	}
	savedState->writeFloat(_fov);
	savedState->writeFloat(_nclip);
	savedState->writeFloat(_fclip);
}

bool Set::Setup::restoreState(SaveGame *savedState) {
	_name = savedState->readString();

	_bkgndBm = Bitmap::getPool().getObject(savedState->readLESint32());
	_bkgndZBm = Bitmap::getPool().getObject(savedState->readLESint32());

	_pos      = savedState->readVector3d();
	if (g_grim->getGameType() == GType_MONKEY4) {
		float x = savedState->readFloat();
		float y = savedState->readFloat();
		float z = savedState->readFloat();
		float w = savedState->readFloat();
		Math::Quaternion q(x, y, z, w);
		_rot = q.toMatrix();
	} else {
		_interest = savedState->readVector3d();
		_roll     = savedState->readFloat();
	}
	_fov      = savedState->readFloat();
	_nclip    = savedState->readFloat();
	_fclip    = savedState->readFloat();

	return true;
}

Light::Light() : _falloffNear(0.0f), _falloffFar(0.0f), _enabled(false), _id(0) {
	setIntensity(0.0f);
	setUmbra(0.0f);
	setPenumbra(0.0f);
}

void Set::Setup::getRotation(float *x, float *y, float *z) {
	Math::Angle aX, aY, aZ;
	if (g_grim->getGameType() == GType_MONKEY4)
		_rot.getEuler(&aX, &aY, &aZ, Math::EO_ZYX);
	else
		_rot.getEuler(&aX, &aY, &aZ, Math::EO_ZXY);

	if (x != nullptr)
		*x = aX.getDegrees();
	if (y != nullptr)
		*y = aY.getDegrees();
	if (z != nullptr)
		*z = aZ.getDegrees();
}

void Set::Setup::setPitch(Math::Angle pitch) {
	Math::Angle oldYaw, oldRoll;
	if (g_grim->getGameType() == GType_MONKEY4) {
		_rot.getEuler(&oldRoll, &oldYaw, nullptr, Math::EO_ZYX);
		_rot.buildFromEuler(oldRoll, oldYaw, pitch, Math::EO_ZYX);
	} else {
		_rot.getEuler(&oldYaw, nullptr, &oldRoll, Math::EO_ZXY);
		_rot.buildFromEuler(oldYaw, pitch, oldRoll, Math::EO_ZXY);
	}
}

void Set::Setup::setYaw(Math::Angle yaw) {
	Math::Angle oldPitch, oldRoll;
	if (g_grim->getGameType() == GType_MONKEY4) {
		_rot.getEuler(&oldRoll, nullptr, &oldPitch, Math::EO_ZYX);
		_rot.buildFromEuler(oldRoll, yaw, oldPitch, Math::EO_ZYX);
	} else {
		_rot.getEuler(nullptr, &oldPitch, &oldRoll, Math::EO_ZXY);
		_rot.buildFromEuler(yaw, oldPitch, oldRoll, Math::EO_ZXY);
	}
}

void Set::Setup::setRoll(Math::Angle roll) {
	Math::Angle oldPitch, oldYaw;
	if (g_grim->getGameType() == GType_MONKEY4) {
		_rot.getEuler(nullptr, &oldYaw, &oldPitch, Math::EO_ZYX);
		_rot.buildFromEuler(roll, oldYaw, oldPitch, Math::EO_ZYX);
	} else {
		_rot.getEuler(&oldYaw, &oldPitch, nullptr, Math::EO_ZXY);
		_rot.buildFromEuler(oldYaw, oldPitch, roll, Math::EO_ZXY);
	}
}

void Light::setUmbra(float angle) {
	_umbraangle = angle;
	_cosumbraangle = cosf(angle * (float)M_PI / 180.0f);
}

void Light::setPenumbra(float angle) {
	_penumbraangle = angle;
	_cospenumbraangle = cosf(angle * (float)M_PI / 180.0f);
}

void Light::setIntensity(float intensity) {
	_intensity = intensity;
	if (g_grim->getGameType() == GType_MONKEY4) {
		_scaledintensity = intensity / 255;
	} else {
		_scaledintensity = intensity / 15;
	}
}

void Light::load(TextSplitter &ts) {
	char buf[256];
	float tmp;

	// Light names can be null, but ts doesn't seem flexible enough to allow this
	if (strlen(ts.getCurrentLine()) > strlen(" light"))
		ts.scanString(" light %256s", 1, buf);
	else {
		ts.nextLine();
		strcpy(buf, "");
	}
	_name = buf;

	ts.scanString(" type %256s", 1, buf);
	Common::String type = buf;
	if (type == "spot") {
		_type = Spot;
	} else if (type == "omni") {
		_type = Omni;
	} else if (type == "direct") {
		_type = Direct;
	} else {
		error("Light::load() Unknown type of light: %s", buf);
	}

	ts.scanString(" position %f %f %f", 3, &_pos.x(), &_pos.y(), &_pos.z());
	ts.scanString(" direction %f %f %f", 3, &_dir.x(), &_dir.y(), &_dir.z());
	ts.scanString(" intensity %f", 1, &tmp);
	setIntensity(tmp);
	ts.scanString(" umbraangle %f", 1, &tmp);
	setUmbra(tmp);
	ts.scanString(" penumbraangle %f", 1, &tmp);
	setPenumbra(tmp);

	int r, g, b;
	ts.scanString(" color %d %d %d", 3, &r, &g, &b);
	_color.getRed() = r;
	_color.getGreen() = g;
	_color.getBlue() = b;

	_enabled = true;
}

void Light::loadBinary(Common::SeekableReadStream *data) {
	char name[32];
	data->read(name, 32);
	_name = name;

	_pos.readFromStream(data);

	Math::Quaternion quat;
	quat.readFromStream(data);

	_dir.set(0, 0, -1);
	Math::Matrix4 rot = quat.toMatrix();
	rot.transform(&_dir, false);

	// This relies on the order of the LightType enum.
	_type = (LightType)data->readSint32LE();

	setIntensity(data->readFloatLE());

	int j = data->readSint32LE();
	// This always seems to be 0
	if (j != 0) {
		warning("Light::loadBinary j != 0");
	}

	_color.getRed() = data->readSint32LE();
	_color.getGreen() = data->readSint32LE();
	_color.getBlue() = data->readSint32LE();

	_falloffNear = data->readFloatLE();
	_falloffFar = data->readFloatLE();
	setUmbra(data->readFloatLE());
	setPenumbra(data->readFloatLE());

	_enabled = true;
}

void Light::saveState(SaveGame *savedState) const {
	//name
	savedState->writeString(_name);
	savedState->writeBool(_enabled);

	//type
	savedState->writeLEUint32(_type);

	savedState->writeVector3d(_pos);
	savedState->writeVector3d(_dir);

	savedState->writeColor(_color);

	savedState->writeFloat(_intensity);
	savedState->writeFloat(_umbraangle);
	savedState->writeFloat(_penumbraangle);

	savedState->writeFloat(_falloffNear);
	savedState->writeFloat(_falloffFar);
}

bool Light::restoreState(SaveGame *savedState) {
	_name = savedState->readString();
	_enabled = savedState->readBool();
	if (savedState->saveMinorVersion() > 7) {
		if (savedState->saveMinorVersion() >= 12) {
			_type = (LightType)savedState->readLEUint32();
		} else {
			int type = savedState->readLEUint32();
			if (type == 1) {
				_type = Spot;
			} else if (type == 2) {
				_type = Direct;
			} else if (type == 3) {
				_type = Omni;
			} else if (type == 4) {
				_type = Ambient;
			}
		}
	} else {
		Common::String type = savedState->readString();
		if (type == "spot") {
			_type = Spot;
		} else if (type == "omni") {
			_type = Omni;
		} else if (type == "direct") {
			_type = Direct;
		}
	}

	_pos           = savedState->readVector3d();
	_dir           = savedState->readVector3d();

	_color         = savedState->readColor();

	setIntensity(    savedState->readFloat());
	setUmbra(        savedState->readFloat());
	setPenumbra(     savedState->readFloat());

	if (savedState->saveMinorVersion() >= 20) {
		_falloffNear = savedState->readFloat();
		_falloffFar = savedState->readFloat();
	}

	return true;
}

SetShadow::SetShadow() : _numSectors(0) {
}

void SetShadow::loadBinary(Common::SeekableReadStream *data, Set *set) {
	uint32 nameLen = data->readUint32LE();
	char *name = new char[nameLen];
	data->read(name, nameLen);
	_name = Common::String(name);

	int lightNameLen = data->readSint32LE();
	char *lightName = new char[lightNameLen];
	data->read(lightName, lightNameLen);

	_shadowPoint.readFromStream(data);

	if (lightNameLen > 0) {
		for (Common::List<Light *>::const_iterator it = set->getLights(false).begin(); it != set->getLights(false).end(); ++it) {
			if ((*it)->_name.equals(lightName)) {
				_shadowPoint = (*it)->_pos;
				break;
			}
		}
	}

	int numSectors = data->readSint32LE();
	for (int i = 0; i < numSectors; ++i) {
		uint32 sectorNameLen = data->readUint32LE();
		char *sectorName = new char[sectorNameLen];
		data->read(sectorName, sectorNameLen);
		_sectorNames.push_back(sectorName);
		delete[] sectorName;
	}

	data->skip(4); // Unknown
	_color._vals[0] = (byte)data->readSint32LE();
	_color._vals[1] = (byte)data->readSint32LE();
	_color._vals[2] = (byte)data->readSint32LE();
	delete[] lightName;
	delete[] name;
}

void SetShadow::saveState(SaveGame *savedState) const {
	savedState->writeString(_name);
	savedState->writeVector3d(_shadowPoint);
	savedState->writeLESint32(_numSectors);
	savedState->writeLEUint32(_sectorNames.size());
	for (Common::List<Common::String>::const_iterator it = _sectorNames.begin(); it != _sectorNames.end(); ++it) {
		savedState->writeString(*it);
	}
	savedState->writeColor(_color);
}

void SetShadow::restoreState(SaveGame *savedState) {
	_name = savedState->readString();
	_shadowPoint = savedState->readVector3d();
	_numSectors = savedState->readLESint32();
	uint numSectors = savedState->readLEUint32();
	for (uint i = 0; i < numSectors; ++i) {
		_sectorNames.push_back(savedState->readString());
	}
	_color = savedState->readColor();
}

void Set::Setup::setupCamera() const {
	// Ignore nclip_ and fclip_ for now in Grim.  This fixes:
	// (a) Nothing was being displayed in the Land of the Living
	// diner because lr.set set nclip to 0.
	// (b) The zbuffers for setups with different nclip or
	// fclip values.  If it turns out that the clipping planes
	// are important at some point, we'll need to modify the
	// zbuffer transformation in bitmap.cpp to take nclip_ and
	// fclip_ into account.
	if (g_grim->getGameType() == GType_GRIM) {
		g_driver->setupCameraFrustum(_fov, 0.01f, 3276.8f);
		g_driver->positionCamera(_pos, _interest, _roll);
	} else {
		g_driver->setupCameraFrustum(_fov, _nclip, _fclip);
		g_driver->positionCamera(_pos, _rot);
	}
}

class Sorter {
public:
	Sorter(const Math::Vector3d &pos) {
		_pos = pos;
	}

	bool operator()(Light *l1, Light *l2) const {
		float d1 = (l1->_pos - _pos).getSquareMagnitude();
		float d2 = (l2->_pos - _pos).getSquareMagnitude();
		if (d1 == d2) {
			return l1->_id < l2->_id;
		}

		return d1 < d2;
	}

	Math::Vector3d _pos;
};

void Set::setupLights(const Math::Vector3d &pos, bool inOverworld) {
	if (g_grim->getGameType() == GType_MONKEY4 && !g_driver->supportsShaders()) {
		// If shaders are not available, we do lighting in software for EMI.
		g_driver->disableLights();
		return;
	}

	if (!_enableLights) {
		g_driver->disableLights();
		return;
	}

	// Sort the ligths from the nearest to the farthest to the pos.
	Sorter sorter(pos);
	Common::List<Light *>* lightsList = inOverworld ? &_overworldLightsList : &_lightsList;
	Common::sort(lightsList->begin(), lightsList->end(), sorter);

	int count = 0;
	foreach (Light *l, *lightsList) {
		if (l->_enabled) {
			g_driver->setupLight(l, count);
			++count;
		}
	}
}

void Set::turnOffLights() {
	_enableLights = false;
	int count = 0;
	for (int i = 0; i < _numLights; i++) {
		Light *l = &_lights[i];
		if (l->_enabled) {
			g_driver->turnOffLight(count);
			++count;
		}
	}
}

void Set::setSetup(int num) {
	// Looks like num is zero-based so >= should work to find values
	// that are out of the range of valid setups

	// Quite weird, but this is what the original does when the setup id is above
	// the upper bound.
	if (num >= _numSetups)
		num %= _numSetups;

	if (num < 0) {
		error("Failed to change scene setup, value out of range");
		return;
	}
	_currSetup = _setups + num;
	g_grim->flagRefreshShadowMask(true);
	if (g_emiSound) {
		g_emiSound->updateSoundPositions();
	}
}

Bitmap::Ptr Set::loadBackground(const char *fileName) {
	Bitmap::Ptr bg = Bitmap::create(fileName);
	if (!bg) {
		Debug::warning(Debug::Bitmaps | Debug::Sets,
					   "Unable to load scene bitmap: %s, loading dfltroom instead", fileName);
		if (g_grim->getGameType() == GType_MONKEY4) {
			bg = Bitmap::create("dfltroom.til");
		} else {
			bg = Bitmap::create("dfltroom.bm");
		}
		if (!bg) {
			Debug::error(Debug::Bitmaps | Debug::Sets, "Unable to load dfltroom");
		}
	} else {
		Debug::debug(Debug::Bitmaps | Debug::Sets,
					 "Loaded scene bitmap: %s", fileName);
	}
	return bg;
}

void Set::drawBackground() const {
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

void Set::drawBitmaps(ObjectState::Position stage) {
	for (StateList::iterator i = _states.reverse_begin(); i != _states.end(); --i) {
		if ((*i)->getPos() == stage && _currSetup == _setups + (*i)->getSetupID())
			(*i)->draw();
	}
}

void Set::setupCamera() {
	_currSetup->setupCamera();
	_frustum.setup(g_driver->getProjection() * g_driver->getModelView());
}

Sector *Set::findPointSector(const Math::Vector3d &p, Sector::SectorType type) {
	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors[i];
		if (sector && (sector->getType() & type) && sector->isVisible() && sector->isPointInSector(p))
			return sector;
	}
	return nullptr;
}

int Set::findSectorSortOrder(const Math::Vector3d &p, Sector::SectorType type) {
	int setup = getSetup();
	int sortOrder = 0;
	float minDist = 0.01f;

	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors[i];
		if (!sector || (sector->getType() & type) == 0 || !sector->isVisible() || setup >= sector->getNumSortplanes())
			continue;

		Math::Vector3d closestPt = sector->getClosestPoint(p);
		float thisDist = (closestPt - p).getMagnitude();
		if (thisDist < minDist) {
			minDist = thisDist;
			sortOrder = sector->getSortplane(setup);
		}
	}
	return sortOrder;
}

void Set::findClosestSector(const Math::Vector3d &p, Sector **sect, Math::Vector3d *closestPoint) {
	Sector *resultSect = nullptr;
	Math::Vector3d resultPt = p;
	float minDist = 0.0;

	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors[i];
		if ((sector->getType() & Sector::WalkType) == 0 || !sector->isVisible())
			continue;
		Math::Vector3d closestPt = sector->getClosestPoint(p);
		float thisDist = (closestPt - p).getMagnitude();
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

void Set::shrinkBoxes(float radius) {
	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors[i];
		sector->shrink(radius);
	}
}

void Set::unshrinkBoxes() {
	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors[i];
		sector->unshrink();
	}
}

void Set::setLightIntensity(const char *light, float intensity) {
	for (int i = 0; i < _numLights; ++i) {
		Light &l = _lights[i];
		if (l._name == light) {
			l.setIntensity(intensity);
			return;
		}
	}
}

void Set::setLightIntensity(int light, float intensity) {
	Light &l = _lights[light];
	l.setIntensity(intensity);
}

void Set::setLightEnabled(const char *light, bool enabled) {
	for (int i = 0; i < _numLights; ++i) {
		Light &l = _lights[i];
		if (l._name == light) {
			l._enabled = enabled;
			return;
		}
	}
}

void Set::setLightEnabled(int light, bool enabled) {
	Light &l = _lights[light];
	l._enabled = enabled;
}

void Set::setLightPosition(const char *light, const Math::Vector3d &pos) {
	for (int i = 0; i < _numLights; ++i) {
		Light &l = _lights[i];
		if (l._name == light) {
			l._pos = pos;
			return;
		}
	}
}

void Set::setLightPosition(int light, const Math::Vector3d &pos) {
	Light &l = _lights[light];
	l._pos = pos;
}

void Set::setSoundPosition(const char *soundName, const Math::Vector3d &pos) {
	setSoundPosition(soundName, pos, _minVolume, _maxVolume);
}

void Set::setSoundPosition(const char *soundName, const Math::Vector3d &pos, int minVol, int maxVol) {
	int newBalance, newVolume;
	calculateSoundPosition(pos, minVol, maxVol, newVolume, newBalance);
	g_sound->setVolume(soundName, newVolume);
	g_sound->setPan(soundName, newBalance);
}

void Set::calculateSoundPosition(const Math::Vector3d &pos, int minVol, int maxVol, int &volume, int &balance) {
	// TODO: The volume and pan needs to be updated when the setup changes.
	// Note: This is only used in Grim. See SoundTrack::updatePosition for the corresponding implementation in EMI.

	/* distance calculation */
	Math::Vector3d cameraPos = _currSetup->_pos;
	Math::Vector3d vector = pos - cameraPos;
	float distance = vector.getMagnitude();
	float diffVolume = maxVol - minVol;
	//This 8.f is a guess, so it may need some adjusting
	int newVolume = (int)(8.f * diffVolume / distance);
	newVolume += minVol;
	if (newVolume > _maxVolume)
		newVolume = _maxVolume;
	volume = newVolume;
	float angle;

	Math::Vector3d cameraVector = _currSetup->_interest - _currSetup->_pos;
	Math::Vector3d up(0, 0, 1);
	Math::Vector3d right;
	cameraVector.normalize();
	float roll = -_currSetup->_roll * (float)M_PI / 180.f;
	float cosr = cos(roll);
	// Rotate the up vector by roll.
	up = up * cosr + Math::Vector3d::crossProduct(cameraVector, up) * sin(roll) +
			cameraVector * Math::Vector3d::dotProduct(cameraVector, up) * (1 - cosr);
	right = Math::Vector3d::crossProduct(cameraVector, up);
	right.normalize();
	angle = atan2(Math::Vector3d::dotProduct(vector, right),
						Math::Vector3d::dotProduct(vector, cameraVector));

	float pan = sin(angle);
	balance = (int)((pan + 1.f) / 2.f * 127.f + 0.5f);
}

Sector *Set::getSectorBase(int id) {
	if ((_numSectors >= 0) && (id < _numSectors))
		return _sectors[id];
	else
		return nullptr;
}

Sector *Set::getSectorByName(const Common::String &name) {
	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors[i];
		if (sector->getName() == name) {
			return sector;
		}
	}
	return nullptr;
}

Sector *Set::getSectorBySubstring(const Common::String &str) {
	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors[i];
		if (strstr(sector->getName().c_str(), str.c_str())) {
			return sector;
		}
	}
	return nullptr;
}

Sector *Set::getSectorBySubstring(const Common::String &str, const Math::Vector3d &pos) {
	for (int i = 0; i < _numSectors; i++) {
		Sector *sector = _sectors[i];
		if (strstr(sector->getName().c_str(), str.c_str()) && sector->isPointInSector(pos)) {
			return sector;
		}
	}
	return nullptr;
}

void Set::setSoundParameters(int minVolume, int maxVolume) {
	_minVolume = minVolume;
	_maxVolume = maxVolume;
}

void Set::getSoundParameters(int *minVolume, int *maxVolume) {
	*minVolume = _minVolume;
	*maxVolume = _maxVolume;
}

void Set::addObjectState(const ObjectState::Ptr &s) {
	_states.push_front(s);
}

ObjectState *Set::addObjectState(int setupID, ObjectState::Position pos, const char *bitmap, const char *zbitmap, bool transparency) {
	ObjectState *state = findState(bitmap);

	if (state) {
		return state;
	}

	state = new ObjectState(setupID, pos, bitmap, zbitmap, transparency);
	addObjectState(state);

	return state;
}

ObjectState *Set::findState(const Common::String &filename) {
	// Check the different state objects for the bitmap
	for (StateList::iterator i = _states.begin(); i != _states.end(); ++i) {
		const Common::String &file = (*i)->getBitmapFilename();

		if (file == filename)
			return *i;
		if (file.compareToIgnoreCase(filename) == 0) {
			Debug::warning(Debug::Sets, "State object request '%s' matches object '%s' but is the wrong case", filename.c_str(), file.c_str());
			return *i;
		}
	}
	return nullptr;
}

void Set::moveObjectStateToFront(const ObjectState::Ptr &s) {
	_states.remove(s);
	_states.push_front(s);
	// Make the state invisible. This hides the deadbolt when brennis closes the switcher door
	// in the server room (tu), and therefore fixes https://github.com/residualvm/residualvm/issues/24
	s->setActiveImage(0);
}

void Set::moveObjectStateToBack(const ObjectState::Ptr &s) {
	_states.remove(s);
	_states.push_back(s);
}

SetShadow *Set::getShadow(int i) {
	return &_shadows[i];
}

SetShadow *Set::getShadowByName(const Common::String &name) {
	for (int i = 0; i < _numShadows; ++i) {
		SetShadow *shadow = &_shadows[i];
		if (shadow->_name.equalsIgnoreCase(name))
			return shadow;
	}
	return nullptr;
}

} // end of namespace Grim
