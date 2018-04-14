/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "common/endian.h"

#include "engines/grim/debug.h"
#include "engines/grim/costume.h"
#include "engines/grim/grim.h"
#include "engines/grim/resource.h"
#include "engines/grim/actor.h"
#include "engines/grim/emi/costumeemi.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/emi/skeleton.h"
#include "engines/grim/emi/costume/emihead.h"
#include "engines/grim/emi/costume/emianim_component.h"
#include "engines/grim/emi/costume/emiluavar_component.h"
#include "engines/grim/emi/costume/emiluacode_component.h"
#include "engines/grim/emi/costume/emimesh_component.h"
#include "engines/grim/emi/costume/emiskel_component.h"
#include "engines/grim/emi/costume/emisprite_component.h"
#include "engines/grim/emi/costume/emitexi_component.h"

namespace Grim {

EMICostume::EMICostume(const Common::String &fname, Actor *owner, Costume *prevCost) :
		Costume(fname, owner, prevCost), _wearChore(nullptr), _emiSkel(nullptr) {
}

void EMICostume::load(Common::SeekableReadStream *data) {
	Common::Array<Component *> components;

	_numChores = data->readUint32LE();
	_chores = new Chore *[_numChores];
	for (int i = 0; i < _numChores; i++) {
		uint32 nameLength;
		Component *prevComponent = nullptr;
		nameLength = data->readUint32LE();
		assert(nameLength < 32);

		char name[32];
		data->read(name, nameLength);
		float length = data->readFloatLE();
		int numTracks = data->readUint32LE();

		if (length == 1000)
			length = -1.0f;
		else
			length *= 1000;

		EMIChore *chore = new EMIChore(name, i, this, (int)length, numTracks);
		_chores[i] = chore;

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
					prevComponent = nullptr;
			}
			// Actually load the appropriate component
			Component *component = loadEMIComponent(parentID < 0 ? nullptr : components[parentID], parentID, componentName, prevComponent);
			if (component) {
				component->setCostume(this);
				component->init();
				chore->addComponent(component);
			}

			components.push_back(component);

			ChoreTrack &track = chore->_tracks[k];
			track.numKeys = data->readUint32LE();
			track.keys = new TrackKey[track.numKeys];
			track.component = component;
			track.compID = -1; // -1 means "look at .component"

			for (int j = 0; j < track.numKeys; j++) {
				float time, value;
				time = data->readFloatLE();
				value = data->readFloatLE();
				track.keys[j].time = (int)(time * 1000);
				length = MAX(length, time * 1000);
				track.keys[j].value = (int)value;
			}
			delete[] componentName;
		}
	}

	_numComponents = components.size();
	_components = new Component *[_numComponents];
	for (int i = 0; i < _numComponents; ++i) {
		_components[i] = components[i];
	}

	_head = new EMIHead(this);
}

void EMICostume::playChore(int num, uint msecs) {
	// FIXME: Original EMI can play multiple instances of a chore at the same time.
	EMIChore *chore = static_cast<EMIChore *>(_chores[num]);
	if (chore->isWearChore()) {
		setWearChore(chore);
	}
	Costume::playChore(num, msecs);
}

void EMICostume::playChoreLooping(int num, uint msecs) {
	// FIXME: Original EMI can play multiple instances of a chore at the same time.
	EMIChore *chore = static_cast<EMIChore *>(_chores[num]);
	if (chore->isWearChore()) {
		setWearChore(chore);
	}
	Costume::playChoreLooping(num, msecs);
}

Component *EMICostume::loadEMIComponent(Component *parent, int parentID, const char *name, Component *prevComponent) {
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
		return new EMIMeshComponent(parent, parentID, name, prevComponent, tag, this);
	} else if (tag == MKTAG('s', 'k', 'e', 'l')) {
		return new EMISkelComponent(parent, parentID, name, prevComponent, tag);
	} else if (tag == MKTAG('t', 'e', 'x', 'i')) {
		return new EMITexiComponent(parent, parentID, name, prevComponent, tag);
	} else if (tag == MKTAG('a', 'n', 'i', 'm')) {
		return new EMIAnimComponent(parent, parentID, name, prevComponent, tag);
	} else if (tag == MKTAG('l', 'u', 'a', 'c')) {
		return new EMILuaCodeComponent(parent, parentID, name, prevComponent, tag);
	} else if (tag == MKTAG('l', 'u', 'a', 'v')) {
		return new EMILuaVarComponent(parent, parentID, name, prevComponent, tag);
	} else if (tag == MKTAG('s', 'p', 'r', 't')) {
		return new EMISpriteComponent(parent, parentID, name, prevComponent, tag);
	} else if (tag == MKTAG('s', 'h', 'a', 'd')) {
		Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement SHAD-handling: %s" , name);
	} else if (tag == MKTAG('a', 'w', 'g', 't')) {
		Debug::warning(Debug::Costumes, "Actor::loadComponentEMI Implement AWGT-handling: %s" , name);
	} else if (tag == MKTAG('s', 'n', 'd', '2')) {
		// ignore, this is a leftover from an earlier engine.
	} else {
		error("Actor::loadComponentEMI missing tag: %s for %s", name, type);
	}

	return nullptr;
}

void EMICostume::draw() {
	bool drewMesh = false;
	for (Common::List<Chore*>::iterator it = _playingChores.begin(); it != _playingChores.end(); ++it) {
		Chore *c = (*it);
		if (!c->_playing)
			continue;
		for (int i = 0; i < c->_numTracks; ++i) {
			if (c->_tracks[i].component) {
				c->_tracks[i].component->draw();
				if (c->_tracks[i].component->isComponentType('m', 'e', 's', 'h'))
					drewMesh = true;
			}
		}
	}

	if (_wearChore && !drewMesh) {
		_wearChore->getMesh()->draw();
	}
}

int EMICostume::update(uint time) {
	for (Common::List<Chore*>::iterator i = _playingChores.begin(); i != _playingChores.end(); ++i) {
		Chore *c = *i;
		c->update(time);

		for (int t = 0; t < c->_numTracks; ++t) {
			if (c->_tracks[t].component) {
				c->_tracks[t].component->update(time);
			}
		}

		if (!c->isPlaying()) {
			i = _playingChores.erase(i);
			--i;
		}
	}

	return 0;
}

void EMICostume::saveState(SaveGame *state) const {
	Costume::saveState(state);

	for (int i = 0; i < _numChores; ++i) {
		EMIChore *chore = (EMIChore *)_chores[i];
		state->writeLESint32(chore->getId());
	}

	state->writeLESint32(_wearChore ? _wearChore->getChoreId() : -1);
}

bool EMICostume::restoreState(SaveGame *state) {
	bool ret = Costume::restoreState(state);
	if (ret) {
		if (state->saveMinorVersion() >= 11) {
			EMIChore::Pool &pool = EMIChore::getPool();
			for (int i = 0; i < _numChores; ++i) {
				EMIChore *chore = (EMIChore *)_chores[i];
				int id = state->readLESint32();
				pool.removeObject(chore->getId());
				EMIChore* oldChore = pool.getObject(id);
				if (oldChore) {
					pool.removeObject(id);
					oldChore->setId(chore->getId());
					pool.addObject(oldChore);
				}
				chore->setId(id);
				pool.addObject(chore);
			}
		}

		if (state->saveMinorVersion() < 13) {
			// Used to be active texture IDs for materials. Materials are now
			// managed by the owner Actor of this Costume.
			for (uint i = 0; i < _materials.size(); ++i) {
				state->readLESint32();
			}
		}

		int id = state->readLESint32();
		if (id >= 0) {
			EMIChore *chore = static_cast<EMIChore *>(_chores[id]);
			setWearChore(chore);
		}
	}
	return ret;
}

Material *EMICostume::findMaterial(const Common::String &name) {
	return _owner->findMaterial(name);
}

Material *EMICostume::loadMaterial(const Common::String &name, bool clamp) {
	MaterialPtr mat = _owner->loadMaterial(name, clamp);
	if (mat) {
		// Save a reference to the material, so it will not be freed during the
		// lifetime of this costume.
		if (Common::find(_materials.begin(), _materials.end(), mat) == _materials.end())
			_materials.push_back(mat);
	}
	return mat;
}

void EMICostume::setWearChore(EMIChore *chore) {
	if (chore != _wearChore) {
		_wearChore = chore;

		if (_emiSkel) {
			_emiSkel->reset();
		}
		_emiSkel = chore->getSkeleton();
	}
}

void EMICostume::setHead(const char *joint, const Math::Vector3d &offset) {
	static_cast<EMIHead *>(_head)->setJoint(joint, offset);
}

void EMICostume::setHeadLimits(float yawRange, float maxPitch, float minPitch) {
	static_cast<EMIHead *>(_head)->setLimits(yawRange, maxPitch, minPitch);
}

EMIModel *EMICostume::getEMIModel() const {
	if (!_wearChore)
		return nullptr;
	return _wearChore->getMesh()->_obj;
}

EMIModel *EMICostume::getEMIModel(int num) const {
	if (num >= _numChores) {
		return nullptr;
	}
	EMIChore *chore = static_cast<EMIChore *>(_chores[num]);
	if (chore == nullptr) {
		return nullptr;
	}
	EMIMeshComponent *mesh = chore->getMesh();
	if (mesh == nullptr) {
		return nullptr;
	}
	return mesh->_obj;
}

} // end of namespace Grim
