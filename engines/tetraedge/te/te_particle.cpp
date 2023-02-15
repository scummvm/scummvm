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

TeParticle::TeParticle(TeScene *scene) : _scene(scene), _size(0),
_colorTime(0), _time(0), _period(0), _particlePerPeriod(0),
_enabled(false), _startLoop(0), _gravity(0), _randomDir(false) {
	indexedParticles()->push_back(this);
}

TeParticle::~TeParticle() {
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
