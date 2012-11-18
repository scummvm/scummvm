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

#include "common/endian.h"

#include "engines/grim/debug.h"
#include "engines/grim/costume.h"
#include "engines/grim/grim.h"
#include "engines/grim/resource.h"
#include "engines/grim/emi/costumeemi.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/costume/chore.h"
#include "engines/grim/costume/head.h"
#include "engines/grim/emi/costume/emianim_component.h"
#include "engines/grim/emi/costume/emiluavar_component.h"
#include "engines/grim/emi/costume/emiluacode_component.h"
#include "engines/grim/emi/costume/emimesh_component.h"
#include "engines/grim/emi/costume/emiskel_component.h"
#include "engines/grim/emi/costume/emisprite_component.h"
#include "engines/grim/emi/costume/emitexi_component.h"
#include "engines/grim/costume/main_model_component.h"

namespace Grim {

EMICostume::EMICostume(const Common::String &fname, Costume *prevCost) :
		Costume(fname, prevCost), _emiSkel(NULL), _emiMesh(NULL) {
}

void EMICostume::load(Common::SeekableReadStream *data) {
	Common::Array<Component *> components;

	_numChores = data->readUint32LE();
	_chores = new Chore *[_numChores];
	for (int i = 0; i < _numChores; i++) {
		uint32 nameLength;
		Component *prevComponent = NULL;
		nameLength = data->readUint32LE();
		assert(nameLength < 32);

		char name[32];
		data->read(name, nameLength);
		float length;
		data->read(&length, 4);
		int numTracks = data->readUint32LE();

		if (length < 1000)
			length *= 1000;

		_chores[i] = new PoolChore(name, i, this, (int)length, numTracks);

		for (int k = 0; k < numTracks; k++) {
			int componentNameLength = data->readUint32LE();

			char *componentName = new char[componentNameLength];
			data->read(componentName, componentNameLength);

			data->readUint32LE();
			int parentID = data->readUint32LE();
			if (parentID == -1 && _prevCostume) {
				// However, only the first item can actually share the
				// node hierarchy with the previous costume, so flag
				// that component so it knows what to do
				if (i == 0)
					parentID = -2;
				prevComponent = _prevCostume->getComponent(0);
				// Make sure that the component is valid
				if (!prevComponent->isComponentType('M', 'M', 'D', 'L'))
					prevComponent = NULL;
			}
			// Actually load the appropriate component
			Component *component = loadComponent(parentID < 0 ? NULL : components[parentID], parentID, componentName, prevComponent);
			if (component) {
				component->setCostume(this);
				component->init();

				if (strcmp(_chores[i]->getName(), "wear_default") == 0) {
					if (component->isComponentType('m', 'e', 's', 'h')) {
						_emiMesh = static_cast<EMIMeshComponent *>(component);
						if (_emiSkel) {
							_emiMesh->_obj->setSkeleton(_emiSkel->_obj);
						}
						for (unsigned int z = 0; z < _emiMesh->_obj->_numTextures; ++z)
							_materials.push_back(_emiMesh->_obj->_mats[z]);
					} else if (component->isComponentType('s', 'k', 'e', 'l')) {
						_emiSkel = static_cast<EMISkelComponent *>(component);
						if (_emiMesh) {
							_emiMesh->_obj->setSkeleton(_emiSkel->_obj);
						}
					}
				
				}
			}

			components.push_back(component);

			ChoreTrack &track = _chores[i]->_tracks[k];
			track.numKeys = data->readUint32LE();
			track.keys = new TrackKey[track.numKeys];
			track.component = component;
			track.compID = -1; // -1 means "look at .component"

			for (int j = 0; j < track.numKeys; j++) {
				float time, value;
				data->read(&time, 4);
				data->read(&value, 4);
				track.keys[j].time = (int)(time * 1000);
				track.keys[j].value = (int)value;
			}
			delete[] componentName;
		}
	}

	_numComponents = components.size();
	_components = new Component *[_numComponents];
	for (int i = 0; i < _numComponents; ++i) {
		_components[i] = components[i];
		if (!_components[i])
			continue;

	}
}

Component *EMICostume::loadComponent(Component *parent, int parentID, const char *name, Component *prevComponent) {
	// some have an exclimation mark, this could mean something.
	// for now, return 0 otherwise it will just crash in some other part.
	//return 0;

	assert(name[0] == '!');
	++name;

	char type[5];
	tag32 tag = 0;
	memcpy(&tag, name, 4);
	memcpy(&type, name, 4);
	type[4] = 0;
	tag = FROM_BE_32(tag);

	name += 4;

	if (tag == MKTAG('m', 'e', 's', 'h')) {
		//Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement MESH-handling: %s" , name);
		return new EMIMeshComponent(parent, parentID, name, prevComponent, tag);
	} else if (tag == MKTAG('s', 'k', 'e', 'l')) {
		//Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement SKEL-handling: %s" , name);
		return new EMISkelComponent(parent, parentID, name, prevComponent, tag);
	} else if (tag == MKTAG('t', 'e', 'x', 'i')) {
//    Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement TEXI-handling: %s" , name);
		return new EMITexiComponent(parent, parentID, name, prevComponent, tag);
	} else if (tag == MKTAG('a', 'n', 'i', 'm')) {
		//Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement ANIM-handling: %s" , name);
		return new EMIAnimComponent(parent, parentID, name, prevComponent, tag);
	} else if (tag == MKTAG('l', 'u', 'a', 'c')) {
//    Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement LUAC-handling: %s" , name);
		return new EMILuaCodeComponent(parent, parentID, name, prevComponent, tag);
	} else if (tag == MKTAG('l', 'u', 'a', 'v')) {
//    Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement LUAV-handling: %s" , name);
		return new EMILuaVarComponent(parent, parentID, name, prevComponent, tag);
	} else if (tag == MKTAG('s', 'p', 'r', 't')) {
		//Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement SPRT-handling: %s" , name);
		return new EMISpriteComponent(parent, parentID, name, prevComponent, tag);
	} else if (tag == MKTAG('s', 'h', 'a', 'd')) {
		Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement SHAD-handling: %s" , name);
	} else if (tag == MKTAG('a', 'w', 'g', 't')) {
		Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement AWGT-handling: %s" , name);
	} else {
		error("Actor::loadComponentEMI missing tag: %s for %s", name, type);
	}
	/*
	char t[4];
	memcpy(t, &tag, sizeof(tag32));
	warning("loadComponent: Unknown tag '%c%c%c%c', name '%s'", t[0], t[1], t[2], t[3], name);*/
	return NULL;
}

void EMICostume::draw() {
	bool drewMesh = false;
	for (Common::List<Chore*>::iterator it = _playingChores.begin(); it != _playingChores.end(); ++it) {
		Chore *c = (*it);
		for (int i = 0; i < c->_numTracks; ++i) {
			if (c->_tracks[i].component) {
				c->_tracks[i].component->draw();
				if (c->_tracks[i].component->isComponentType('m', 'e', 's', 'h'))
					drewMesh = true;
			}
		}
	}

	if (_emiMesh && !drewMesh) {
		_emiMesh->draw();
	}
}

int EMICostume::update(uint time) {
	for (Common::List<Chore*>::iterator i = _playingChores.begin(); i != _playingChores.end(); ++i) {
		(*i)->update(time);
		if (!(*i)->isPlaying()) {
			i = _playingChores.erase(i);
			--i;
		}
	}

	int marker = 0;
	for (int i = 0; i < _numComponents; i++) {
		if (_components[i]) {
			_components[i]->setMatrix(_matrix);
			int m = _components[i]->update(time);
			if (m > 0) {
				marker = m;
			}
		}
	}

	return marker;
}

Material * EMICostume::findSharedMaterial(const Common::String &name) {
	Common::List<Material *>::iterator it = _materials.begin();
	for (; it != _materials.end(); ++it)
		if ((*it)->getFilename() == name)
			return *it;

	Material * mat = g_resourceloader->loadMaterial(name.c_str(), NULL);
	_materials.push_back(mat);
	return mat;
}


void EMICostume::saveState(SaveGame *state) const {
	// TODO
	return;
}

bool EMICostume::restoreState(SaveGame *state) {
	// TODO
	return true;
}

} // end of namespace Grim
