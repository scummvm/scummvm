/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tetraedge/te/te_particle.h"
#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_core.h"

namespace Tetraedge {

static const char TE_PARTICLE_RANDOM_TABLE[] = "http://www.arkham-development.com/";


TeParticle::TeParticle(TeScene *scene) : _scene(scene), _size(0),
_colorTime(0), _time(0), _period(0), _particlePerPeriod(0),
_enabled(false), _startLoop(0), _gravity(0), _randomDir(false) {
	indexedParticles()->push_back(this);
	_elementsPending.reserve(32);
}

TeParticle::~TeParticle() {
	_elements.clear();
	_elementsPending.clear();
	Common::Array<TeParticle *> *parts = indexedParticles();
	for (uint i = 0; i < parts->size(); i++) {
		if ((*parts)[i] == this) {
			parts->remove_at(i);
			break;
		}
	}
}

bool TeParticle::loadTexture(const Common::String &filename) {
	// Path for these textures includes '/' so convert to Path object first.
	const Common::Path path(filename);
	_texture = Te3DTexture::makeInstance();
	TeCore *core = g_engine->getCore();
	Common::FSNode texnode = core->findFile(path);
	return _texture->load(texnode);
}

void TeParticle::setOrientation(const TeVector3f32 &orientation) {
	// Divergence from original.. orientation is only ever used
	// to calculate this matrix during update(), so just do it now.
	_orientMatrix = TeMatrix4x4();
	_orientMatrix.rotate(TeQuaternion::fromAxisAndAngle(TeVector3f32(1,  0, 0),
							orientation.x() * M_PI / 180));
	_orientMatrix.rotate(TeQuaternion::fromAxisAndAngle(TeVector3f32(0, -1, 0),
							orientation.y() * M_PI / 180));
	_orientMatrix.rotate(TeQuaternion::fromAxisAndAngle(TeVector3f32(0,  0, 1),
							orientation.z() * M_PI / 180));
}

static const double PART_EPSILON = 4.656613e-10;

// A kind of weird randomizer, but that's what the original does..
static double _particleRandom() {
	int i = g_engine->getRandomNumber(INT_MAX);
	byte b = TE_PARTICLE_RANDOM_TABLE[g_engine->getRandomNumber(0x21)];
	return (float)(i ^ b) * -2 * PART_EPSILON + 1;
}

void TeParticle::update(int val) {
	if (val <= 0) {
		_realTimer.timeElapsed();
		return;
	}

	for (int i = 0; i < val; i++) {
		float timeOffset = (val < 2 ? _realTimer.timeFromLastTimeElapsed() / 1000.0f : _period);

		_lastTime += timeOffset;
		_lastTime = CLIP(_lastTime, 0, _period * 10);
		if (_enabled && _period <= _lastTime) {
			// Diverge from original.. precalculate these values to avoid
			// redoing the math on every particle.
			Common::Array<TeVector3f32> quad(4);
			quad[0] = _matrix.mult3x3(TeVector3f32(-1, -1, 0));
			quad[1] = _matrix.mult3x3(TeVector3f32( 1, -1, 0));
			quad[2] = _matrix.mult3x3(TeVector3f32( 1,  1, 0));
			quad[3] = _matrix.mult3x3(TeVector3f32(-1,  1, 0));
			const TeQuaternion norot = TeQuaternion::fromEuler(TeVector3f32(0, 0, 0));
			_lastTime = fmod(_lastTime, _period);
			for (int p = 0; p < _particlePerPeriod; p++) {
				TeIntrusivePtr<TeElement> elem;
				if (_elementsPending.empty()) {
					elem = new TeElement();
					_elements.push_back(elem);
				} else {
					elem = _elementsPending.back();
					_elements.push_back(elem);
					_elementsPending.pop_back();
					elem->_elapsedTime = 0;
					elem->_yOffset = 0;
				}

				const TeVector3f32 posoffset(_particleRandom() * _volumeSize.x(),
								_particleRandom() * _volumeSize.y(),
								_particleRandom() * _volumeSize.z());
				elem->setPositionFast(_position + (_orientMatrix * posoffset));

				if (!_randomDir) {
					elem->_direction = TeVector3f32(0, 0, 0);
				} else {
					const TeVector3f32 dirmod(_particleRandom(), _particleRandom(), _particleRandom());
					if (_direction.length() >= 0.001f) {
						elem->_direction = dirmod * _direction.length() / 2;
					} else {
						elem->_direction = dirmod / 4;
					}
				}

				elem->setQuad(_texture, quad, TeColor(255, 0, 0, 0));
				elem->setRotation(norot);
				float sz = _size / 10;
				elem->setScale(TeVector3f32(sz, sz, sz));
				elem->_elapsedTime = 0;
				_scene->models().push_back(elem.get());

				if (_colorTime == 0) {
					elem->setColor(_startColor);
				} else {
					elem->setColor(TeColor(0, 0, 0, 0));
				}
			}
		}

		for (auto eiter = _elements.begin(); eiter != _elements.end(); eiter++) {
			TeIntrusivePtr<TeElement> elem = *eiter;
			TeVector3f32 newpos = elem->position() + (_direction + elem->_direction) * timeOffset / 1000;
			elem->setPositionFast(newpos);
			float newTime = elem->_elapsedTime + timeOffset;
			elem->_elapsedTime = newTime;
			if (newTime <= _time) {
				float alpha = _startColor.a();
				if (newTime < _colorTime) {
					alpha = (alpha * newTime) / _colorTime;
				}
				float endScale = newTime / _time;
				float startScale = 1.0 - endScale;
				elem->setColor(TeColor(_endColor.r() * endScale + _startColor.r() * startScale,
								 _endColor.g() * endScale + _startColor.g() * startScale,
								 _endColor.b() * endScale + _startColor.b() * startScale,
								 _endColor.a() * endScale + alpha * startScale));
				for (int j = -1; j < timeOffset / 100; j++) {
					elem->_yOffset += _gravity / 1000;
					elem->setPositionFast(elem->position() + TeVector3f32(0, elem->_yOffset, 0));
				}
			} else {
				// This element is done.
				_elementsPending.push_back(elem);
				eiter = _elements.erase(eiter);
				for (uint j = 0; j < _scene->models().size(); j++) {
					if (_scene->models()[j].get() == elem.get()) {
						_scene->models().remove_at(j);
						break;
					}
				}
			}
		}
	}

	//debug("Updated particle texture %s, %d active %d pending",
	//	_texture->getAccessName().c_str(), _elements.size(),
	//	_elementsPending.size());

	_realTimer.timeElapsed();
}

/*static*/
Common::Array<TeParticle *> *TeParticle::_indexedParticles = nullptr;

/*static*/
int TeParticle::getIndex(const Common::String &name) {
	int retval = -1;
	Common::Array<TeParticle *> *parts = indexedParticles();
	for (uint i = 0; i < parts->size(); i++) {
		if ((*parts)[i]->_name == name) {
			retval = i;
			break;
		}
	}
	return retval;
}

/*static*/
TeParticle *TeParticle::getIndexedParticle(int idx) {
	Common::Array<TeParticle *> *parts = indexedParticles();
	if (idx >= (int)(parts->size()))
		error("Invalid particle %d requested (of %d)", idx, parts->size());
	return (*parts)[idx];
}

/*static*/
void TeParticle::deleteAll() {
	// Divergence from original - we keep auto pointers
	// to the particles in the scene so don't delete
	// them here.
	Common::Array<TeParticle *> *parts = indexedParticles();
	parts->clear();
}

/*static*/
void TeParticle::updateAll(int val) {
	Common::Array<TeParticle *> *parts = indexedParticles();
	for (uint i = 0; i < parts->size(); i++)
		(*parts)[i]->update(val);
}

/*static*/
Common::Array<TeParticle *> *TeParticle::indexedParticles() {
	if (_indexedParticles == nullptr)
		_indexedParticles = new Common::Array<TeParticle *>();
	return _indexedParticles;
}

/*static*/
void TeParticle::cleanup() {
	delete _indexedParticles;
	_indexedParticles = nullptr;
}

} // end namespace Tetraedge
